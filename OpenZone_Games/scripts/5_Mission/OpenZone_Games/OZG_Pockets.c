// What the window keeps of the card sharp's phase 4 (design §6, §10) on the
// client: the cards its player carries, and the flights of his own sleights
// waiting for the server's word.

// A card of the player's pockets as the strip draws it: its item's key (what
// a sleight names it by, OZG_Keys), its face and the look of its back. A card
// planned out of the deck before a deal is drawn there too, a back when it is
// to be one by chance.
class OZG_PocketCard
{
    string Key;
    string Face;
    int    Look;
    bool   Planned;

    static OZG_PocketCard Make(string key, string face, int look, bool planned)
    {
        OZG_PocketCard c = new OZG_PocketCard();
        c.Key = key;
        c.Face = face;
        c.Look = look;
        c.Planned = planned;
        return c;
    }
}

// The pockets of this client's player, read from his own inventory (design
// §9: the client's own inventory is the client's to read).
class OZG_PocketsRead
{
    static void Into(array<ref OZG_PocketCard> into)
    {
        into.Clear();
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        array<OZG_Card_Base> cards = new array<OZG_Card_Base>();
        OZG_DeckRead.PocketCards(player, cards);
        for (int i = 0; i < cards.Count(); i++)
        {
            OZG_Card_Base c = cards[i];
            into.Insert(OZG_PocketCard.Make(OZG_Keys.Of(c), c.OZG_Face(), c.OZG_Look(), false));
        }
    }

    // One line of the keys, to tell a change.
    static string Line(array<ref OZG_PocketCard> cards)
    {
        string line = "";
        for (int i = 0; i < cards.Count(); i++)
            line = line + cards[i].Key + ";";
        return line;
    }
}

// A sleight of this player's own, asked for and not yet answered: what his
// window flies when the server says it was done (OZG_GameMenu.SleightDone).
class OZG_SleightFly
{
    string Kind;
    // Where the card of the pockets was drawn, and its picture.
    ref OZG_CardSpot PocketSpot;
    string PocketPicture;
    // Where the card of the hand was drawn, and its picture.
    ref OZG_CardSpot HandSpot;
    string HandPicture;
    int    HandLook;
}
