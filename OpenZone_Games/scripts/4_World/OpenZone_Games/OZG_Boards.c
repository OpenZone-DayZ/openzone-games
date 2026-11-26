// The "3 in 1" board and its pieces (boards and dice design §3).

// A piece: a chess piece, a checker, a die. Its face (`ozgFace` in its config:
// "wK" .. "bP", "wD", "bD", "Di") is what a game is handed, as a card's face is.
class OZG_Piece_Base extends ItemBase
{
    string OZG_Face()
    {
        return GetGame().ConfigGetTextOut("CfgVehicles " + GetType() + " ozgFace");
    }
}

// The board is a set as a deck is one: laid out, kept on its place and given
// back as a deck is, so in script it is a deck (OZG_Deck_Base). What is its own
// is what it holds: a piece in each slot of its insert, and nothing else.
class OZG_Board_Base extends OZG_Deck_Base
{
    override bool OZG_Takes(EntityAI thing)
    {
        return thing.IsInherited(OZG_Piece_Base);
    }

    // Server: every piece of the insert, once, for a board that is new. A
    // piece's class names the slots of its kind, so creating it puts it into
    // the first free one.
    override void OZG_FillNow()
    {
        if (m_OZG_Filled)
            return;
        m_OZG_Filled = true;
        if (!GetGame().IsServer())
            return;
        if (GetInventory().AttachmentCount() > 0)
            return;
        array<string> classes = new array<string>();
        OZG_BoardRead.PiecesOf(GetType(), classes);
        int made = 0;
        for (int i = 0; i < classes.Count(); i++)
        {
            if (GetInventory().CreateAttachment(classes[i]))
                made++;
        }
        if (made != classes.Count())
            OZG_Log.Warn("a board " + GetType() + " took " + made.ToString() + " of its " + classes.Count().ToString() + " pieces");
    }
}

// The board laid out (boards and dice design §4): the chess field up, or the
// nardy field when a nardy game is chosen. The field is the picture of the
// model's one selection; both pictures share the frame, so the sides look the
// same either way.
class OZG_BoardPlace extends OZG_Place
{
    protected int m_OZG_Side;
    // Client and server: the side painted now, -1 for none yet.
    protected int m_OZG_SideShown = -1;

    void OZG_BoardPlace()
    {
        RegisterNetSyncVariableInt("m_OZG_Side", 0, 1);
    }

    override void OZG_ShowGame(string gameId)
    {
        int side = 0;
        if (gameId == OZG_NardyConst.LONG_ID || gameId == OZG_NardyConst.SHORT_ID)
            side = 1;
        if (side == m_OZG_Side)
            return;
        m_OZG_Side = side;
        SetSynchDirty();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_PaintSide();
    }

    override void EEInit()
    {
        super.EEInit();
        if (!GetGame().IsDedicatedServer())
            OZG_PaintSide();
    }

    protected void OZG_PaintSide()
    {
        if (m_OZG_Side == m_OZG_SideShown)
            return;
        m_OZG_SideShown = m_OZG_Side;
        string key = "ozgFieldChess";
        if (m_OZG_Side == 1)
            key = "ozgFieldNardy";
        string picture = GetGame().ConfigGetTextOut("CfgVehicles " + GetType() + " " + key);
        if (picture != "")
            SetObjectTexture(0, picture);
    }
}

// A die (boards and dice design §10): one for the board and the can alike. It
// may be loaded with the card sharp's kit -- a face that comes up more often,
// and how much -- and keeps who knows of it: its maker, and whoever studied it
// closely, noticed it or opened it in the kit's window since.
class OZG_Die_Base extends OZG_Piece_Base
{
    protected int m_OZG_LoadFace;
    protected int m_OZG_LoadWeight;
    protected string m_OZG_LoadBy;
    protected ref array<string> m_OZG_Knowers;

    void OZG_Die_Base()
    {
        m_OZG_Knowers = new array<string>();
    }

    int OZG_LoadFace()
    {
        return m_OZG_LoadFace;
    }

    int OZG_LoadWeight()
    {
        return m_OZG_LoadWeight;
    }

    string OZG_LoadBy()
    {
        return m_OZG_LoadBy;
    }

    bool OZG_Loaded()
    {
        return m_OZG_LoadFace > 0 && m_OZG_LoadWeight > 0;
    }

    bool OZG_Knows(string uid)
    {
        return m_OZG_Knowers.Find(uid) >= 0;
    }

    // Studied closely in the hands (OZG_ActionStudy), as a card is.
    override void SetActions()
    {
        super.SetActions();
        AddAction(OZG_ActionStudy);
    }

    void OZG_AddKnower(string uid)
    {
        if (uid != "" && m_OZG_Knowers.Find(uid) < 0)
            m_OZG_Knowers.Insert(uid);
    }

    // Server: loaded anew by `uid`: the old load and those who knew it gone.
    void OZG_Load(int face, int weight, string uid)
    {
        m_OZG_LoadFace = face;
        m_OZG_LoadWeight = weight;
        m_OZG_LoadBy = uid;
        m_OZG_Knowers.Clear();
        OZG_AddKnower(uid);
    }

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        ctx.Write(OZG_Const.DIE_SAVE);
        ctx.Write(m_OZG_LoadFace);
        ctx.Write(m_OZG_LoadWeight);
        ctx.Write(m_OZG_LoadBy);
        int n = m_OZG_Knowers.Count();
        ctx.Write(n);
        for (int i = 0; i < n; i++)
            ctx.Write(m_OZG_Knowers[i]);
        if (m_OZG_LoadFace > 0)
            OZG_Log.Dbg("die " + OZG_Keys.Of(this) + " stored, loaded " + m_OZG_LoadFace.ToString() + " at " + m_OZG_LoadWeight.ToString());
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
        int v;
        if (!ctx.Read(v))
            return false;
        int face;
        int weight;
        string by;
        int n;
        if (!ctx.Read(face) || !ctx.Read(weight) || !ctx.Read(by) || !ctx.Read(n))
            return false;
        if (n < 0 || n > 1000)
            return false;
        m_OZG_Knowers.Clear();
        for (int i = 0; i < n; i++)
        {
            string one;
            if (!ctx.Read(one))
                return false;
            m_OZG_Knowers.Insert(one);
        }
        m_OZG_LoadFace = face;
        m_OZG_LoadWeight = weight;
        m_OZG_LoadBy = by;
        if (face > 0)
            OZG_Log.Dbg("die read back, loaded " + face.ToString() + " at " + weight.ToString());
        return true;
    }
}

// The can of dice (boards and dice design §3.3): a set as the board is one,
// five dice in slots of its own.
class OZG_DiceCan_Base extends OZG_Deck_Base
{
    static const int DICE = 5;

    override bool OZG_Takes(EntityAI thing)
    {
        return thing.IsKindOf("OZG_Die_Base");
    }

    // Server: five dice for a can that is new.
    override void OZG_FillNow()
    {
        if (m_OZG_Filled)
            return;
        m_OZG_Filled = true;
        if (!GetGame().IsServer())
            return;
        if (GetInventory().AttachmentCount() > 0)
            return;
        for (int i = 0; i < DICE; i++)
            GetInventory().CreateAttachment("OZG_Die");
    }
}

// The dice a seat throws (boards and dice design §8).
class OZG_DiceRead
{
    // The dice of the set a place holds (the board's two, the can's five), in
    // their slots' order.
    static void OfPlace(OZG_Place place, array<EntityAI> into)
    {
        into.Clear();
        if (!place)
            return;
        OZG_Deck_Base held = place.OZG_Deck();
        if (!held)
            return;
        OfThing(held, into);
    }

    // The dice a board or a can holds, in the order of its die slots (Die1,
    // Die2, ..): the order attachments are listed in is not kept across a
    // restart, the slots are, and a swapped die takes the slot of the one it
    // replaced.
    static void OfThing(EntityAI holder, array<EntityAI> into)
    {
        TStringArray slots = new TStringArray;
        GetGame().ConfigGetTextArray("CfgVehicles " + holder.GetType() + " attachments", slots);
        for (int i = 0; i < slots.Count(); i++)
        {
            if (slots[i].IndexOf("Die") < 0)
                continue;
            int id = InventorySlots.GetSlotIdFromString(slots[i]);
            EntityAI a = holder.GetInventory().FindAttachment(id);
            if (a && a.IsKindOf("OZG_Die_Base"))
                into.Insert(a);
        }
    }

    // A can the player carries holding `need` dice or more: its first `need`.
    static bool OwnCan(PlayerBase player, int need, array<EntityAI> into)
    {
        into.Clear();
        if (!player)
            return false;
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI can = items[i];
            if (!can || !can.IsKindOf("OZG_DiceCan_Base"))
                continue;
            array<EntityAI> dice = new array<EntityAI>();
            OfThing(can, dice);
            if (dice.Count() < need)
                continue;
            for (int k = 0; k < need; k++)
                into.Insert(dice[k]);
            return true;
        }
        return false;
    }

    // The loads of dice: the face (0 for none) and the weight of each.
    static void Loads(array<EntityAI> dice, array<int> faces, array<int> weights)
    {
        faces.Clear();
        weights.Clear();
        for (int i = 0; i < dice.Count(); i++)
        {
            OZG_Die_Base die = OZG_Die_Base.Cast(dice[i]);
            int face = 0;
            int weight = 0;
            if (die && die.OZG_Loaded())
            {
                face = die.OZG_LoadFace();
                weight = die.OZG_LoadWeight();
            }
            faces.Insert(face);
            weights.Insert(weight);
        }
    }
}

class OZG_BoardRead
{
    // The piece class that goes into each slot of a board's insert, in the
    // slots' order: for the slot OZG_Bd_<face><n> the class of that face in
    // the board's look ("Classic": OZG_Chess_Classic_wK, OZG_Disc_Classic_W,
    // OZG_Die).
    static void PiecesOf(string boardClass, array<string> into)
    {
        into.Clear();
        TStringArray slots = new TStringArray;
        GetGame().ConfigGetTextArray("CfgVehicles " + boardClass + " attachments", slots);
        string look = GetGame().ConfigGetTextOut("CfgVehicles " + boardClass + " ozgStyle");
        if (look == "")
            look = "classic";
        string up = look.Substring(0, 1);
        up.ToUpper();
        look = up + look.Substring(1, look.Length() - 1);
        for (int i = 0; i < slots.Count(); i++)
        {
            string face = FaceOfSlot(slots[i]);
            string cls = ClassOf(face, look);
            if (cls != "")
                into.Insert(cls);
        }
    }

    // "OZG_Bd_wR2" -> "wR"; "OZG_Bd_bD07" -> "bD"; "OZG_Bd_Die1" -> "Di".
    static string FaceOfSlot(string slot)
    {
        string prefix = "OZG_Bd_";
        if (slot.IndexOf(prefix) != 0)
            return "";
        string rest = slot.Substring(prefix.Length(), slot.Length() - prefix.Length());
        if (rest.IndexOf("Die") == 0)
            return "Di";
        if (rest.Length() < 2)
            return "";
        return rest.Substring(0, 2);
    }

    static string ClassOf(string face, string look)
    {
        if (face == "Di")
            return "OZG_Die";
        if (face.Length() != 2)
            return "";
        string kind = face.Substring(1, 1);
        if (kind == "D")
        {
            string side = face.Substring(0, 1);
            side.ToUpper();
            return "OZG_Disc_" + look + "_" + side;
        }
        return "OZG_Chess_" + look + "_" + face;
    }

    // The faces of every piece the board on a place holds, one entry a piece,
    // and of the pieces in the place's cargo beside it.
    static void FacesOf(OZG_Place place, array<string> into)
    {
        into.Clear();
        if (!place)
            return;
        OZG_Deck_Base board = place.OZG_Deck();
        int i;
        if (board)
        {
            int held = board.GetInventory().AttachmentCount();
            for (i = 0; i < held; i++)
            {
                OZG_Piece_Base one = OZG_Piece_Base.Cast(board.GetInventory().GetAttachmentFromIndex(i));
                if (one)
                    into.Insert(one.OZG_Face());
            }
        }
        CargoBase cargo = place.GetInventory().GetCargo();
        if (cargo)
        {
            for (i = 0; i < cargo.GetItemCount(); i++)
            {
                OZG_Piece_Base beside = OZG_Piece_Base.Cast(cargo.GetItem(i));
                if (beside)
                    into.Insert(beside.OZG_Face());
            }
        }
    }
}
