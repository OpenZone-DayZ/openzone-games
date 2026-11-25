// The dealer's three (card sharp's design §6.5), as they ride in the deal:
// cards of his pockets put into the deck, cards taken out of it into his
// pockets, cards of the deck he reads dealt to him first. On the wire one
// text, "in=<key>,..;out=<key>|*,..;pick=<key>,.." (OZG_WAsk.Stack), a key
// being a card item's network id (OZG_Keys) and * a card of the deck taken
// by chance. The session checks it whole and tries the deal on paper before
// an item moves; what it found for each key is kept here.
class OZG_Stack
{
    ref array<string> InKeys;
    ref array<string> OutKeys;
    ref array<string> PickKeys;

    // What the keys stood for, once checked: the cards going in (from the
    // dealer's pockets), coming out (the place's, a card by chance among
    // them) and chosen (the place's).
    ref array<OZG_Card_Base> InCards;
    ref array<OZG_Card_Base> OutCards;
    ref array<OZG_Card_Base> PickCards;

    void OZG_Stack()
    {
        InKeys = new array<string>();
        OutKeys = new array<string>();
        PickKeys = new array<string>();
        InCards = new array<OZG_Card_Base>();
        OutCards = new array<OZG_Card_Base>();
        PickCards = new array<OZG_Card_Base>();
    }

    // The keys of one list: "k,k,k"; empty parts left out.
    protected static void Keys(string text, array<string> into)
    {
        into.Clear();
        array<string> parts = new array<string>();
        text.Split(",", parts);
        for (int i = 0; i < parts.Count(); i++)
        {
            string key = parts[i].Trim();
            if (key != "")
                into.Insert(key);
        }
    }

    void Read(string text)
    {
        InKeys.Clear();
        OutKeys.Clear();
        PickKeys.Clear();
        array<string> lists = new array<string>();
        text.Split(";", lists);
        for (int i = 0; i < lists.Count(); i++)
        {
            string list = lists[i];
            int eq = list.IndexOf("=");
            if (eq <= 0)
                continue;
            string name = list.Substring(0, eq).Trim();
            string keys = list.Substring(eq + 1, list.Length() - eq - 1);
            if (name == "in")
                Keys(keys, InKeys);
            else if (name == "out")
                Keys(keys, OutKeys);
            else if (name == "pick")
                Keys(keys, PickKeys);
        }
    }

    bool Empty()
    {
        return InKeys.Count() == 0 && OutKeys.Count() == 0 && PickKeys.Count() == 0;
    }

    // The sleights it makes (§6.5, §7): a card put in is one, a card taken
    // out one, the cards chosen for his hand one together.
    int Sleights()
    {
        int n = InKeys.Count() + OutKeys.Count();
        if (PickKeys.Count() > 0)
            n++;
        return n;
    }

    // "Kh,As" for the server's log.
    static string Faces(array<OZG_Card_Base> cards)
    {
        string line = "";
        for (int i = 0; i < cards.Count(); i++)
        {
            if (i > 0)
                line = line + ",";
            if (cards[i])
                line = line + cards[i].OZG_Face();
            else
                line = line + "?";
        }
        if (line == "")
            return "-";
        return line;
    }
}
