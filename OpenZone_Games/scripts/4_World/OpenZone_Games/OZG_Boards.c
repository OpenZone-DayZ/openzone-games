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
