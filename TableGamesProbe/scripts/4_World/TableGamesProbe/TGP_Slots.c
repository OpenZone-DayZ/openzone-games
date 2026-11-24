// The slot probe (2026-11-24): can a thing have a slot for each of fifty-two
// cards, and what does it cost against the same cards in a grid.
//
// The question behind it: OpenZone Games wants a deck that is a container of
// its cards, every card with a place of its own. Slots are the engine's, one
// table for every mod a server loads, so their number is measured before it is
// designed with.
class TGP_Slots
{
    static const int CARDS = 52;

    // The hold probe (2026-11-24): can a kit go into what it has placed, with
    // what it holds, and come back. What the probe's deck said when it was
    // placed, when it asked the hands, and how long after that the mat had it
    // (-1: not yet).
    static string s_Moved;
    static int s_AskedAt;
    static int s_ArrivedMs = -1;

    protected static PlayerBase FirstPlayer()
    {
        array<Man> men = new array<Man>;
        GetGame().GetPlayers(men);
        for (int i = 0; i < men.Count(); i++)
        {
            PlayerBase p = PlayerBase.Cast(men.Get(i));
            if (p && p.GetIdentity())
                return p;
        }
        return null;
    }

    protected static EntityAI MatNear(vector at)
    {
        array<Object> found = new array<Object>;
        GetGame().GetObjectsAtPosition(at, 4.0, found, null);
        for (int i = 0; i < found.Count(); i++)
        {
            Object o = found.Get(i);
            if (o && o.GetType() == "TGP_Mat")
                return EntityAI.Cast(o);
        }
        return null;
    }

    // A probe deck with its fifty-two cards into the first player's hands.
    static string DeckFill()
    {
        PlayerBase p = FirstPlayer();
        if (!p)
            return "deckfill: nobody is connected";
        // What the hands hold goes: the probe wants them free, and the stand's
        // own delete does not reach a thing in hands. They are free a tick
        // later, so the deck is asked for again.
        EntityAI busy = p.GetHumanInventory().GetEntityInHands();
        if (busy)
        {
            string was = busy.GetType();
            GetGame().ObjectDelete(busy);
            return "deckfill: the hands held " + was + ", which is deleted; ask again";
        }
        EntityAI deck = p.GetHumanInventory().CreateInHands("TGP_Deck");
        if (!deck)
            return "deckfill: the hands did not take a deck";
        int made = 0;
        for (int i = 0; i < CARDS; i++)
        {
            int id = InventorySlots.GetSlotIdFromString("TGP_C" + Two(i));
            if (deck.GetInventory().CreateAttachmentEx("TGP_SlotCard_" + Two(i), id))
                made++;
        }
        return "deckfill: cards=" + made.ToString() + " held=" + deck.GetInventory().AttachmentCount().ToString();
    }

    // What the mat near a point holds, and what the first player's hands hold.
    static string Hold(vector at)
    {
        string s = "hold:";
        EntityAI mat = MatNear(at);
        if (!mat)
        {
            s = s + " no mat within 4 m";
        }
        else
        {
            s = s + " mat atts=" + mat.GetInventory().AttachmentCount().ToString();
            EntityAI deck = mat.GetInventory().FindAttachmentByName("TGP_Hold");
            if (deck)
                s = s + " deck=" + deck.GetType() + " cards=" + deck.GetInventory().AttachmentCount().ToString();
            else
                s = s + " deck=none";
        }
        string hands = "none";
        PlayerBase p = FirstPlayer();
        EntityAI held;
        if (p)
            held = p.GetHumanInventory().GetEntityInHands();
        if (held)
            hands = held.GetType() + "/" + held.GetInventory().AttachmentCount().ToString();
        s = s + " hands=" + hands + " ms=" + s_ArrivedMs.ToString() + "; " + s_Moved;
        return s;
    }

    // The deck out of the mat onto the ground where the mat lies, at once; the
    // mat is removed; free hands are asked to take the deck up.
    static string HoldBack(vector at)
    {
        EntityAI mat = MatNear(at);
        PlayerBase p = FirstPlayer();
        if (!mat || !p)
            return "holdback: no mat, or nobody connected";
        EntityAI deck = mat.GetInventory().FindAttachmentByName("TGP_Hold");
        if (!deck)
            return "holdback: the mat holds no deck";
        vector m4[4];
        mat.GetTransform(m4);
        bool dropped = mat.GetInventory().DropEntityWithTransform(InventoryMode.SERVER, mat, deck, m4);
        string parent = "none";
        if (deck.GetHierarchyParent())
            parent = deck.GetHierarchyParent().GetType();
        GetGame().ObjectDelete(mat);
        if (dropped && !p.GetHumanInventory().GetEntityInHands())
            p.ServerTakeEntityToHands(deck);
        string s = "holdback: dropped=" + TGP_Out.B(dropped) + " parentAfterDrop=" + parent;
        s = s + " cards=" + deck.GetInventory().AttachmentCount().ToString();
        return s;
    }

    protected static string Two(int i)
    {
        if (i < 10)
            return "0" + i.ToString();
        return i.ToString();
    }

    // How many slots the engine holds now, and how many of ours it knows.
    static string Count()
    {
        int known = 0;
        int firstBad = -1;
        int id;
        for (int i = 0; i < CARDS; i++)
        {
            id = InventorySlots.GetSlotIdFromString("TGP_C" + Two(i));
            if (id != InventorySlots.INVALID)
                known++;
            else if (firstBad < 0)
                firstBad = i;
        }
        int engine = InventorySlots.COUNT;
        int ours = CARDS;
        string s = "slots: engine COUNT=" + engine.ToString();
        s = s + " ours known=" + known.ToString() + "/" + ours.ToString();
        s = s + " firstUnknown=" + firstBad.ToString();
        int body = InventorySlots.GetSlotIdFromString("Body");
        s = s + " Body=" + body.ToString();
        return s;
    }

    // Both boxes beside a point, each filled with the fifty-two cards.
    static string Fill(vector at)
    {
        string s = "";

        EntityAI slotBox = EntityAI.Cast(GetGame().CreateObjectEx("TGP_SlotBox", at, ECE_PLACE_ON_SURFACE));
        if (!slotBox)
            return "fill: TGP_SlotBox was not made";
        int t0 = TickCount(0);
        int attached = 0;
        int id;
        EntityAI made;
        int i;
        for (i = 0; i < CARDS; i++)
        {
            id = InventorySlots.GetSlotIdFromString("TGP_C" + Two(i));
            made = slotBox.GetInventory().CreateAttachmentEx("TGP_SlotCard_" + Two(i), id);
            if (made)
                attached++;
        }
        float msSlots = TickCount(t0) / 10000.0;
        s = s + "slot box: made=" + attached.ToString() + " held=" + slotBox.GetInventory().AttachmentCount().ToString();
        s = s + " ms=" + TGP_Out.F2(msSlots);

        vector beside = at + "0.6 0 0";
        EntityAI gridBox = EntityAI.Cast(GetGame().CreateObjectEx("TGP_GridBox", beside, ECE_PLACE_ON_SURFACE));
        if (!gridBox)
            return s + "; TGP_GridBox was not made";
        t0 = TickCount(0);
        int stowed = 0;
        for (i = 0; i < CARDS; i++)
        {
            made = gridBox.GetInventory().CreateEntityInCargo("TGP_SlotCard_" + Two(i));
            if (made)
                stowed++;
        }
        float msGrid = TickCount(t0) / 10000.0;
        CargoBase cargo = gridBox.GetInventory().GetCargo();
        int inCargo = -1;
        if (cargo)
            inCargo = cargo.GetItemCount();
        s = s + "; grid box: made=" + stowed.ToString() + " held=" + inCargo.ToString();
        s = s + " ms=" + TGP_Out.F2(msGrid);
        return s;
    }

    // Every probe box within a few metres of a point is removed, with what it
    // holds.
    static string Clear(vector at)
    {
        array<Object> found = new array<Object>;
        GetGame().GetObjectsAtPosition(at, 6.0, found, null);
        int gone = 0;
        Object o;
        for (int i = 0; i < found.Count(); i++)
        {
            o = found.Get(i);
            if (!o)
                continue;
            if (o.IsKindOf("TGP_SlotBox") || o.IsKindOf("TGP_GridBox"))
            {
                GetGame().ObjectDelete(o);
                gone++;
            }
        }
        return "cleared " + gone.ToString() + " probe box(es)";
    }
}
