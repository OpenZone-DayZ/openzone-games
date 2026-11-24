// The things of OpenZone Games in the world: the deck a player carries, the
// place it becomes on a surface, what the hologram shows, and a seat's mark.

// The decks there are. A deck is a config class that inherits OZG_Deck_Base
// and its art; no script is needed to add one. Both sides read the same
// config in the same order, so a deck's number means the same on a client and
// on the server.
class OZG_Decks
{
    protected static ref array<string> s_Classes;

    protected static void Load()
    {
        s_Classes = new array<string>();
        int n = GetGame().ConfigGetChildrenCount("CfgVehicles");
        string name;
        for (int i = 0; i < n; i++)
        {
            GetGame().ConfigGetChildName("CfgVehicles", i, name);
            if (!GetGame().IsKindOf(name, "OZG_Deck_Base"))
                continue;
            if (GetGame().ConfigGetInt("CfgVehicles " + name + " scope") != 2)
                continue;
            s_Classes.Insert(name);
        }
        OZG_Log.Dbg("decks: " + s_Classes.Count().ToString() + " in config");
    }

    static int NumberOf(string deckClass)
    {
        if (!s_Classes)
            Load();
        return s_Classes.Find(deckClass);
    }

    static string ClassOf(int number)
    {
        if (!s_Classes)
            Load();
        if (number < 0 || number >= s_Classes.Count())
            return "";
        return s_Classes[number];
    }

    static string FirstClass()
    {
        return ClassOf(0);
    }

    // The style a deck draws its cards in: the imageset is ozg_cards_<style>.
    static string StyleOf(string deckClass)
    {
        string style;
        if (deckClass != "")
            style = GetGame().ConfigGetTextOut("CfgVehicles " + deckClass + " ozgStyle");
        if (style == "")
            style = "classic";
        return style;
    }

    // The texture of a deck's back on the 3D things.
    static string SheetOf(string deckClass)
    {
        if (deckClass == "")
            return "";
        return GetGame().ConfigGetTextOut("CfgVehicles " + deckClass + " ozgSheet");
    }

    static void Reset()
    {
        s_Classes = null;
    }
}

// The deck. A kit in the game's sense: "choose placement", the hologram,
// "deploy" held; the action deletes the kit when the place is made.
class OZG_Deck_Base extends ItemBase
{
    override bool IsBasebuildingKit()
    {
        return true;
    }

    override bool IsDeployable()
    {
        return true;
    }

    override float GetDeployTime()
    {
        return 2.0;
    }

    override void SetActions()
    {
        super.SetActions();
        AddAction(ActionTogglePlaceObject);
        AddAction(OZG_ActionLayOut);
    }

    override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
    {
        super.OnPlacementComplete(player, position, orientation);
        if (!GetGame().IsServer())
            return;
        OZG_Sessions.LayOut(PlayerBase.Cast(player), GetType(), position, orientation);
    }
}

// What the hologram shows. It carries the place's look and nothing of its
// logic: on the server the game makes one as a real, networked thing for as
// long as the deploy lasts.
class OZG_PlacePlacing extends ItemBase
{
    override void SetActions()
    {
    }

    override bool IsInventoryVisible()
    {
        return false;
    }

    override bool DisableVicinityIcon()
    {
        return true;
    }
}

// The laid-out game: a mat with the deck on it. Not takeable, saved with the
// world; the anchor of the session. In its synchronised variables there is
// only a pulse: the state, the seats, the revision. It words the actions, and
// a client whose window is open on an older revision asks again.
class OZG_Place extends ItemBase
{
    protected static ref array<OZG_Place> s_All = new array<OZG_Place>();

    protected int m_OZG_IdA;
    protected int m_OZG_IdB;
    protected int m_OZG_St;
    protected int m_OZG_Taken;
    protected int m_OZG_Busy;
    protected int m_OZG_Total;
    protected int m_OZG_Rev;
    protected int m_OZG_DeckNo;

    // Server only.
    protected string m_OZG_DeckClass;
    protected int m_OZG_Mode;
    protected bool m_OZG_Loaded;

    // Client only.
    protected int m_OZG_LookOf;

    void OZG_Place()
    {
        m_OZG_DeckNo = -1;
        m_OZG_LookOf = -2;
        RegisterNetSyncVariableInt("m_OZG_IdA");
        RegisterNetSyncVariableInt("m_OZG_IdB");
        RegisterNetSyncVariableInt("m_OZG_St", 0, 3);
        RegisterNetSyncVariableInt("m_OZG_Taken", 0, 15);
        RegisterNetSyncVariableInt("m_OZG_Busy", 0, 15);
        RegisterNetSyncVariableInt("m_OZG_Total", 0, 15);
        RegisterNetSyncVariableInt("m_OZG_Rev");
        RegisterNetSyncVariableInt("m_OZG_DeckNo", -1, 126);
    }

    // ------------------------------------------------------------ identity

    string OZG_Id()
    {
        if (m_OZG_IdA == 0)
            return "";
        return m_OZG_IdA.ToString() + "-" + m_OZG_IdB.ToString();
    }

    string OZG_DeckClass()
    {
        if (m_OZG_DeckClass != "")
            return m_OZG_DeckClass;
        return OZG_Decks.ClassOf(m_OZG_DeckNo);
    }

    int OZG_Mode()
    {
        return m_OZG_Mode;
    }

    int OZG_State()
    {
        return m_OZG_St;
    }

    int OZG_Taken()
    {
        return m_OZG_Taken;
    }

    int OZG_Busy()
    {
        return m_OZG_Busy;
    }

    int OZG_Total()
    {
        return m_OZG_Total;
    }

    int OZG_Rev()
    {
        return m_OZG_Rev;
    }

    // Server: what a new place is told once.
    void OZG_Setup(string deckClass, int mode)
    {
        m_OZG_IdA = Math.RandomInt(1, 2147483647);
        m_OZG_IdB = Math.RandomInt(1, 2147483647);
        m_OZG_DeckClass = deckClass;
        m_OZG_DeckNo = OZG_Decks.NumberOf(deckClass);
        m_OZG_Mode = mode;
        SetSynchDirty();
    }

    // Server: the pulse.
    void OZG_Pulse(int state, int taken, int busy, int total, int rev)
    {
        bool same = m_OZG_St == state && m_OZG_Taken == taken && m_OZG_Busy == busy;
        if (same && m_OZG_Total == total && m_OZG_Rev == rev)
            return;
        m_OZG_St = state;
        m_OZG_Taken = taken;
        m_OZG_Busy = busy;
        m_OZG_Total = total;
        m_OZG_Rev = rev;
        SetSynchDirty();
    }

    // ------------------------------------------------------------ every place

    static array<OZG_Place> All()
    {
        return s_All;
    }

    // Another place closer than `spacing` to a point.
    static bool NearOther(vector at, float spacing, OZG_Place self)
    {
        for (int i = 0; i < s_All.Count(); i++)
        {
            OZG_Place other = s_All[i];
            if (!other || other == self)
                continue;
            if (vector.Distance(at, other.GetPosition()) < spacing)
                return true;
        }
        return false;
    }

    override void EEInit()
    {
        super.EEInit();
        if (s_All.Find(this) < 0)
            s_All.Insert(this);
        if (GetGame() && GetGame().IsServer())
        {
            SetTakeable(false);
            SetLifetime(OZG_Const.PLACE_LIFETIME);
        }
    }

    override void EEDelete(EntityAI parent)
    {
        s_All.RemoveItem(this);
        if (GetGame() && GetGame().IsServer())
            OZG_Sessions.PlaceGone(this);
        super.EEDelete(parent);
    }

    // ------------------------------------------------------------ the world's storage

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        OZG_Log.Dbg("place " + OZG_Id() + " is written to the world's storage");
        ctx.Write(OZG_Const.SAVE_VERSION);
        ctx.Write(m_OZG_IdA);
        ctx.Write(m_OZG_IdB);
        ctx.Write(m_OZG_DeckClass);
        ctx.Write(m_OZG_Mode);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
        int v;
        if (!ctx.Read(v))
            return false;
        int a;
        if (!ctx.Read(a))
            return false;
        int b;
        if (!ctx.Read(b))
            return false;
        string deck;
        if (!ctx.Read(deck))
            return false;
        int mode;
        if (!ctx.Read(mode))
            return false;
        m_OZG_IdA = a;
        m_OZG_IdB = b;
        m_OZG_DeckClass = deck;
        m_OZG_Mode = mode;
        m_OZG_Loaded = true;
        OZG_Log.Dbg("place " + OZG_Id() + " is read from the world's storage, deck " + deck);
        return true;
    }

    override void EEOnAfterLoad()
    {
        super.EEOnAfterLoad();
        if (!GetGame() || !GetGame().IsServer())
            return;
        SetLifetime(OZG_Const.PLACE_LIFETIME);
        m_OZG_DeckNo = OZG_Decks.NumberOf(m_OZG_DeckClass);
        SetSynchDirty();
        if (m_OZG_Loaded)
            OZG_Sessions.Restore(this);
    }

    // ------------------------------------------------------------ not an item

    // None of an item's own actions: this is not something to take, craft with
    // or attach.
    override void SetActions()
    {
        AddAction(OZG_ActionSit);
        AddAction(OZG_ActionWatch);
        AddAction(OZG_ActionGather);
    }

    override bool IsInventoryVisible()
    {
        return false;
    }

    override bool DisableVicinityIcon()
    {
        return true;
    }

    override bool IsTakeable()
    {
        return false;
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        return false;
    }

    override bool CanPutIntoHands(EntityAI parent)
    {
        return false;
    }

    // ------------------------------------------------------------ the look

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Look();
        OZG_PlaceWatch.Moved(this);
    }

    // The deck on the mat wears the back of the deck the place was laid out
    // from.
    protected void OZG_Look()
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (m_OZG_LookOf == m_OZG_DeckNo)
            return;
        m_OZG_LookOf = m_OZG_DeckNo;
        string sheet = OZG_Decks.SheetOf(OZG_Decks.ClassOf(m_OZG_DeckNo));
        if (sheet == "")
            return;
        int at = GetHiddenSelectionIndex("deck");
        if (at >= 0)
            SetObjectTexture(at, sheet);
    }
}

// What a client hears when a place's pulse moves: the window, if it is open on
// this place, compares revisions. The window lives a tier up, so it listens
// here.
class OZG_PlaceWatch
{
    static ref ScriptInvoker OnMoved = new ScriptInvoker();

    static void Moved(OZG_Place place)
    {
        if (GetGame().IsDedicatedServer())
            return;
        OnMoved.Invoke(place);
    }
}

// A seat's mark. It exists on one client only, for a few seconds, and is
// nothing a cursor or a body should meet.
class OZG_SeatMark extends ItemBase
{
    override void SetActions()
    {
    }

    override bool IsInventoryVisible()
    {
        return false;
    }

    override bool DisableVicinityIcon()
    {
        return true;
    }

    override bool IsTakeable()
    {
        return false;
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        return false;
    }

    override bool CanPutIntoHands(EntityAI parent)
    {
        return false;
    }

    override bool CanBeActionTarget()
    {
        return false;
    }
}
