// The things of OpenZone Games in the world: the deck a player carries, the
// place it becomes on a surface, what the hologram shows, and a seat's mark.

// The decks there are. A deck is a config class that inherits OZG_Deck_Base
// and its art; no script is needed to add one. Both sides read the same
// config in the same order, so a deck's number means the same on a client and
// on the server.
class OZG_Decks
{
    protected static ref array<string> s_Classes;
    // The looks there are: every style a deck names, in the config's order,
    // and the sheet of the first deck of each. A look's number is what a card
    // tells the clients; both sides read the same config in the same order.
    protected static ref array<string> s_Styles;
    protected static ref array<string> s_Sheets;
    // The pictures of a look's single cards: the path of their files up to
    // the face (the deck's ozgCards), which <face>_co.paa and back_co.paa end.
    protected static ref array<string> s_Cards;

    protected static void Load()
    {
        s_Classes = new array<string>();
        s_Styles = new array<string>();
        s_Sheets = new array<string>();
        s_Cards = new array<string>();
        int n = GetGame().ConfigGetChildrenCount("CfgVehicles");
        string name;
        for (int i = 0; i < n; i++)
        {
            GetGame().ConfigGetChildName("CfgVehicles", i, name);
            // A board is a set as a deck is one (boards and dice design §2).
            bool aSet = GetGame().IsKindOf(name, "OZG_Deck_Base") || GetGame().IsKindOf(name, "OZG_Board_Base");
            if (!aSet)
                continue;
            if (GetGame().ConfigGetInt("CfgVehicles " + name + " scope") != 2)
                continue;
            s_Classes.Insert(name);
            string style = StyleOf(name);
            if (s_Styles.Find(style) < 0)
            {
                s_Styles.Insert(style);
                s_Sheets.Insert(GetGame().ConfigGetTextOut("CfgVehicles " + name + " ozgSheet"));
                s_Cards.Insert(GetGame().ConfigGetTextOut("CfgVehicles " + name + " ozgCards"));
            }
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

    // What a set is (OZG_Const.SET_*): its config's ozgSetKind; a deck names
    // none and is cards.
    static string KindOf(string setClass)
    {
        string kind;
        if (setClass != "")
            kind = GetGame().ConfigGetTextOut("CfgVehicles " + setClass + " ozgSetKind");
        if (kind == "")
            kind = OZG_Const.SET_CARDS;
        return kind;
    }

    // The class of the place a set is laid out as: its config's ozgPlace; a
    // deck names none and lies on the mat.
    static string PlaceOf(string setClass)
    {
        string place;
        if (setClass != "")
            place = GetGame().ConfigGetTextOut("CfgVehicles " + setClass + " ozgPlace");
        if (place == "")
            place = "OZG_Place";
        return place;
    }

    // The texture of a deck's back on the 3D things.
    static string SheetOf(string deckClass)
    {
        if (deckClass == "")
            return "";
        return GetGame().ConfigGetTextOut("CfgVehicles " + deckClass + " ozgSheet");
    }

    // Whether a new deck of this class gets the two jokers besides the
    // fifty-two (its config's ozgJokers = 1).
    static bool HasJokers(string deckClass)
    {
        if (deckClass == "")
            return false;
        return GetGame().ConfigGetInt("CfgVehicles " + deckClass + " ozgJokers") == 1;
    }

    // The number of a look; 0 for a style no deck names.
    static int LookOfStyle(string style)
    {
        if (!s_Classes)
            Load();
        int at = s_Styles.Find(style);
        if (at < 0)
            return 0;
        return at;
    }

    // The look of a deck's own cards.
    static int LookOfDeck(string deckClass)
    {
        return LookOfStyle(StyleOf(deckClass));
    }

    static string StyleAt(int look)
    {
        if (!s_Classes)
            Load();
        if (look < 0 || look >= s_Styles.Count())
            return "classic";
        return s_Styles[look];
    }

    static string SheetAt(int look)
    {
        if (!s_Classes)
            Load();
        if (look < 0 || look >= s_Sheets.Count())
            return "";
        return s_Sheets[look];
    }

    // The picture of one card's face in a look, and of the look's back; ""
    // when the look names no single cards.
    static string CardFaceAt(int look, string face)
    {
        string path = CardsAt(look);
        if (path == "" || face == "")
            return "";
        return path + face + "_co.paa";
    }

    static string CardBackAt(int look)
    {
        string path = CardsAt(look);
        if (path == "")
            return "";
        return path + "back_co.paa";
    }

    protected static string CardsAt(int look)
    {
        if (!s_Classes)
            Load();
        if (look < 0 || look >= s_Cards.Count())
            return "";
        return s_Cards[look];
    }

    static void Reset()
    {
        s_Classes = null;
        s_Styles = null;
        s_Sheets = null;
        s_Cards = null;
    }
}

// The deck. A kit in the game's sense: "choose placement", the hologram,
// "deploy" held. A container of its cards as well: slots that take any card.
class OZG_Deck_Base extends ItemBase
{
    // Server: true only while the game's own action ends this deck's placing.
    // A kit that was placed is deleted there (actiondeployobject.c:230); a
    // deck is not spent by being laid out (OZG_ActionLayOut.OnEndServer).
    protected bool m_OZG_Spared;
    // Came back from the world's storage, or from a character's: what it held
    // then is what it holds, and it is never filled again. Nothing of the
    // deck's own is stored; that it WAS stored is what tells an old deck from
    // a new one.
    protected bool m_OZG_Stored;
    protected bool m_OZG_Filled;

    // The sound of the deck gathered up (design §17), in one synchronised
    // number as a place carries its game's (OZG_Sounds.PackLot): the place is
    // gone by then, the deck is where it lay.
    protected int m_OZG_Sound;
    protected ref OZG_SoundPulse m_OZG_Ear;

    void OZG_Deck_Base()
    {
        RegisterNetSyncVariableInt("m_OZG_Sound");
    }

    // Server: a lot of sounds, heard on every client near the deck.
    void OZG_Hear(array<int> ids)
    {
        if (!GetGame() || !GetGame().IsServer() || !ids || ids.Count() == 0)
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Sound = m_OZG_Ear.NextNumber(m_OZG_Sound, ids);
        SetSynchDirty();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_Quiet, OZG_Sounds.QUIET_MS, false);
        OZG_Log.Dbg("deck " + GetType() + " heard " + m_OZG_Ear.LastLot());
    }

    void OZG_HearOne(int id)
    {
        array<int> one = new array<int>();
        one.Insert(id);
        OZG_Hear(one);
    }

    // Server: the number keeps its count and carries no sounds.
    void OZG_Quiet()
    {
        m_OZG_Sound = OZG_Sounds.QuietOf(m_OZG_Sound);
        SetSynchDirty();
    }

    int OZG_SoundNumber()
    {
        return m_OZG_Sound;
    }

    string OZG_SoundLast()
    {
        if (!m_OZG_Ear)
            return "-";
        return m_OZG_Ear.LastLot();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        if (GetGame().IsDedicatedServer())
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Ear.HeardNumber(m_OZG_Sound, this, "deck");
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        super.EEDelete(parent);
    }

    override bool IsBasebuildingKit()
    {
        return !m_OZG_Spared;
    }

    void OZG_Spare(bool on)
    {
        m_OZG_Spared = on;
    }

    override bool IsDeployable()
    {
        return true;
    }

    // One second of progress. What a player holds for is longer: the body
    // kneels down first. Measured 2026-11-24 with a progress of two seconds:
    // a hold of five seconds placed nothing, six placed the game.
    override float GetDeployTime()
    {
        return 1.0;
    }

    override void EEInit()
    {
        super.EEInit();
        // Not at once: a deck that comes back from storage learns only after
        // this that it was stored.
        if (GetGame() && GetGame().IsServer())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_FillIfNew, 200, false);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        m_OZG_Stored = true;
        return super.OnStoreLoad(ctx, version);
    }

    void OZG_FillIfNew()
    {
        if (!m_OZG_Stored)
            OZG_FillNow();
    }

    // Server: the fifty-two cards of this deck's own look, and the two jokers
    // of a deck that has them, once.
    void OZG_FillNow()
    {
        if (m_OZG_Filled)
            return;
        m_OZG_Filled = true;
        if (!GetGame().IsServer())
            return;
        if (GetInventory().AttachmentCount() > 0)
            return;
        string style = OZG_Decks.StyleOf(GetType());
        array<string> faces = new array<string>();
        OZG_Cards.Whole(faces);
        if (OZG_Decks.HasJokers(GetType()))
            OZG_Cards.Jokers(faces);
        int made = 0;
        for (int i = 0; i < faces.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(GetInventory().CreateAttachment(OZG_Const.CARD_PREFIX + faces[i]));
            if (card)
            {
                card.OZG_SetStyle(style);
                made++;
            }
        }
        if (made != faces.Count())
            OZG_Log.Warn("a deck " + GetType() + " took " + made.ToString() + " of its cards");
    }

    // How many cards it has room for: the slots its class names.
    int OZG_Slots()
    {
        TStringArray names = new TStringArray;
        GetGame().ConfigGetTextArray("CfgVehicles " + GetType() + " attachments", names);
        return names.Count();
    }

    int OZG_Free()
    {
        return OZG_Slots() - GetInventory().AttachmentCount();
    }

    // A deck takes cards and nothing else; a board, its pieces (OZG_Takes).
    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (!attachment || !OZG_Takes(attachment))
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }

    bool OZG_Takes(EntityAI thing)
    {
        return thing.IsInherited(OZG_Card_Base);
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
        OZG_Sessions.LayOut(PlayerBase.Cast(player), this, position, orientation);
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
    // The sounds of the game (design §17): one synchronised number, a count
    // and up to three sounds (OZG_Sounds.PackLot), quiet again QUIET_MS after.
    protected int m_OZG_Sound;
    protected ref OZG_SoundPulse m_OZG_Ear;

    // Server only.
    protected string m_OZG_DeckClass;
    protected int m_OZG_Mode;
    protected bool m_OZG_Loaded;
    protected int m_OZG_OpenUntil;

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
        RegisterNetSyncVariableInt("m_OZG_Sound");
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

    // Server: the game chosen here, for a place that shows it (a board turns
    // its nardy field up). A deck's place shows nothing.
    void OZG_ShowGame(string gameId)
    {
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

    // Server: a lot of sounds of the game, heard on every client near the
    // place (design §17), whether its window is open or not.
    void OZG_Hear(array<int> ids)
    {
        if (!GetGame() || !GetGame().IsServer() || !ids || ids.Count() == 0)
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Sound = m_OZG_Ear.NextNumber(m_OZG_Sound, ids);
        SetSynchDirty();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_Quiet, OZG_Sounds.QUIET_MS, false);
        OZG_Log.Dbg("place " + OZG_Id() + " heard " + m_OZG_Ear.LastLot());
    }

    void OZG_HearOne(int id)
    {
        array<int> one = new array<int>();
        one.Insert(id);
        OZG_Hear(one);
    }

    // Server: the number keeps its count and carries no sounds.
    void OZG_Quiet()
    {
        m_OZG_Sound = OZG_Sounds.QuietOf(m_OZG_Sound);
        SetSynchDirty();
    }

    int OZG_SoundNumber()
    {
        return m_OZG_Sound;
    }

    string OZG_SoundLast()
    {
        if (!m_OZG_Ear)
            return "-";
        return m_OZG_Ear.LastLot();
    }

    // Client: what the number carries is played on the place.
    protected void OZG_Listen()
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Ear.HeardNumber(m_OZG_Sound, this, "place " + OZG_Id());
    }

    // ------------------------------------------------------------ every place

    static array<OZG_Place> All()
    {
        return s_All;
    }

    // The place of an id on this side, or null (a client has the places near
    // its player).
    static OZG_Place OZG_ById(string id)
    {
        if (id == "")
            return null;
        for (int i = 0; i < s_All.Count(); i++)
        {
            OZG_Place p = s_All[i];
            if (p && p.OZG_Id() == id)
                return p;
        }
        return null;
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
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        s_All.RemoveItem(this);
        if (GetGame() && GetGame().IsServer())
            OZG_Sessions.PlaceGone(this);
        // A client's marks of this place's seats are things of that client
        // alone: nobody else can take them away when the place goes.
        if (GetGame())
            OZG_PlaceWatch.Gone(this);
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

    // ------------------------------------------------------------ the deck it holds

    // Server: for a moment the session's own hand may put things in and take
    // them out.
    void OZG_Open()
    {
        m_OZG_OpenUntil = GetGame().GetTime() + OZG_Const.OPEN_MS;
    }

    // Whether a SERVER says no. A client never does: the deck comes out of
    // the placer's hands by an event his own client has to agree with
    // (hand_guards.c:302 asks LocationCanMoveEntity on both sides), and a
    // client that refused would go on believing the deck is in its hands.
    // What keeps players out on a client is that the place shows no
    // inventory at all (IsInventoryVisible, DisableVicinityIcon above); what
    // keeps a dishonest client out is this.
    protected bool OZG_Shut()
    {
        if (!GetGame() || !GetGame().IsServer())
            return false;
        return GetGame().GetTime() >= m_OZG_OpenUntil;
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (OZG_Shut())
            return false;
        if (!attachment || !attachment.IsInherited(OZG_Deck_Base))
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }

    override bool CanReleaseAttachment(EntityAI attachment)
    {
        if (OZG_Shut())
            return false;
        return super.CanReleaseAttachment(attachment);
    }

    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if (OZG_Shut())
            return false;
        if (!item || !item.IsInherited(OZG_Card_Base))
            return false;
        return super.CanReceiveItemIntoCargo(item);
    }

    override bool CanReleaseCargo(EntityAI cargo)
    {
        if (OZG_Shut())
            return false;
        return super.CanReleaseCargo(cargo);
    }

    OZG_Deck_Base OZG_Deck()
    {
        return OZG_Deck_Base.Cast(GetInventory().FindAttachmentByName(OZG_Const.SLOT_HOLD));
    }

    // Server: the deck put down on the mat is heard (design §17): laid out by
    // hand (it lands a tick or more after the place is made), by the stand,
    // or given to a place that lost its own. A place read from the world's
    // storage has no session yet when its deck comes back, and is not heard.
    override void EEItemAttached(EntityAI item, string slot_name)
    {
        super.EEItemAttached(item, slot_name);
        if (!GetGame() || !GetGame().IsServer())
            return;
        if (!OZG_Deck_Base.Cast(item) || !OZG_Sessions.Of(this))
            return;
        OZG_HearOne(OZG_SoundIds.PUTDOWN);
    }

    // Server: the deck is asked out of the placer's hands into the place, as
    // the game's indoor fireplace takes its first stick
    // (actioncreateindoorfireplace.c:95). The hands answer a tick or more
    // later, and the call's own answer says nothing (humaninventory.c:228):
    // whoever asks looks again (OZG_Session.AwaitDeck).
    void OZG_TakeDeck(PlayerBase from, OZG_Deck_Base deck)
    {
        if (!from || !deck)
            return;
        OZG_Open();
        from.ServerTakeEntityToTargetAttachmentEx(this, deck, InventorySlots.GetSlotIdFromString(OZG_Const.SLOT_HOLD));
    }

    // Server: a full deck of a class made inside the place: for a place made
    // by script, and for one that lost its deck.
    OZG_Deck_Base OZG_MakeDeck(string deckClass)
    {
        OZG_Deck_Base deck = OZG_Deck();
        if (deck)
            return deck;
        if (deckClass == "")
            deckClass = OZG_Decks.FirstClass();
        OZG_Open();
        deck = OZG_Deck_Base.Cast(GetInventory().CreateAttachment(deckClass));
        if (deck)
            deck.OZG_FillNow();
        else
            OZG_Log.Error("place " + OZG_Id() + " could not be given a deck " + deckClass);
        return deck;
    }

    // Server: the deck out of the place onto the spot where the place lies,
    // at once (inventory.c:1250, a move between two things that are no
    // player's hands). Null when the place holds none.
    OZG_Deck_Base OZG_Release()
    {
        OZG_Deck_Base deck = OZG_Deck();
        if (!deck)
            return null;
        OZG_Open();
        vector m4[4];
        GetTransform(m4);
        if (!GetInventory().DropEntityWithTransform(InventoryMode.SERVER, this, deck, m4))
        {
            OZG_Log.Error("the deck of place " + OZG_Id() + " did not leave it");
            return null;
        }
        return deck;
    }

    // Server: before the deck leaves. Cards the place kept beside the deck go
    // back into it. When there are more cards than slots, as many as do not
    // fit are left lying where the place lies, and WHICH is chance among all
    // of them, the deck's own included: the owner's word is "random extra
    // cards", not "the ones that were brought in". Returns how many were
    // left.
    int OZG_Tidy()
    {
        array<EntityAI> beside = new array<EntityAI>();
        CargoBase cargo = GetInventory().GetCargo();
        int i;
        if (cargo)
        {
            for (i = 0; i < cargo.GetItemCount(); i++)
                beside.Insert(cargo.GetItem(i));
        }
        if (beside.Count() == 0)
            return 0;

        OZG_Deck_Base deck = OZG_Deck();
        int room = 0;
        array<EntityAI> all = new array<EntityAI>();
        if (deck)
        {
            room = deck.OZG_Free();
            int held = deck.GetInventory().AttachmentCount();
            for (i = 0; i < held; i++)
                all.Insert(deck.GetInventory().GetAttachmentFromIndex(i));
        }
        for (i = 0; i < beside.Count(); i++)
            all.Insert(beside[i]);

        int over = beside.Count() - room;
        if (over < 0)
            over = 0;

        OZG_Open();
        vector m4[4];

        // Chance picks the ones that stay behind.
        for (i = 0; i < over; i++)
        {
            int pick = Math.RandomInt(0, all.Count());
            EntityAI stray = all[pick];
            all.Remove(pick);
            beside.RemoveItem(stray);
            if (!stray)
                continue;
            EntityAI holder = stray.GetHierarchyParent();
            if (!holder)
                continue;
            GetTransform(m4);
            float dx = Math.RandomFloat(-0.25, 0.25);
            float dz = Math.RandomFloat(-0.25, 0.25);
            m4[3] = m4[3] + Vector(dx, 0, dz);
            holder.GetInventory().DropEntityWithTransform(InventoryMode.SERVER, holder, stray, m4);
        }

        // Each of the others beside the deck now has a slot.
        for (i = 0; i < beside.Count(); i++)
        {
            EntityAI card = beside[i];
            if (!card || !deck)
                continue;
            InventoryLocation src = new InventoryLocation;
            InventoryLocation dst = new InventoryLocation;
            if (!card.GetInventory().GetCurrentInventoryLocation(src))
                continue;
            if (deck.GetInventory().FindFreeLocationFor(card, FindInventoryLocationType.ATTACHMENT, dst))
                GetInventory().TakeToDst(InventoryMode.SERVER, src, dst);
        }
        return over;
    }

    // ------------------------------------------------------------ the look

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Look();
        OZG_Listen();
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
    // A place left this client's world (gathered, folded, deleted, or out of
    // the client's bubble): its id.
    static ref ScriptInvoker OnGone = new ScriptInvoker();
    // This client's own player was hit.
    static ref ScriptInvoker OnHit = new ScriptInvoker();

    static void Moved(OZG_Place place)
    {
        if (GetGame().IsDedicatedServer())
            return;
        OnMoved.Invoke(place);
    }

    static void Gone(OZG_Place place)
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (!place)
            return;
        string id = place.OZG_Id();
        if (id != "")
            OnGone.Invoke(id);
    }

    static void Hit(PlayerBase player)
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (player && player == GetGame().GetPlayer())
            OnHit.Invoke();
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
