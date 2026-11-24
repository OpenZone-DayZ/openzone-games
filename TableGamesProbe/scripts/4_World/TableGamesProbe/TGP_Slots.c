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
