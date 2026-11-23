// Cards. A card is two characters, a rank of RANKS and a suit of SUITS: `6s` is
// the six of spades, `Th` the ten of hearts (design §8). A deck is always the
// 52; a game says which of them it plays with.
class OZG_Cards
{
    static const string RANKS = "23456789TJQKA";
    static const string SUITS = "shdc";

    // 0 for a two .. 12 for an ace; -1 for what is not a card.
    static int RankOf(string card)
    {
        if (card.Length() != 2)
            return -1;
        string ranks = RANKS;
        return ranks.IndexOf(card.Substring(0, 1));
    }

    // "s", "h", "d" or "c"; "" for what is not a card.
    static string SuitOf(string card)
    {
        if (card.Length() != 2)
            return "";
        string suits = SUITS;
        string suit = card.Substring(1, 1);
        if (suits.IndexOf(suit) < 0)
            return "";
        return suit;
    }

    static bool IsCard(string card)
    {
        if (RankOf(card) < 0)
            return false;
        return SuitOf(card) != "";
    }

    // The 52, in a fixed order: spades, hearts, diamonds, clubs, each from the
    // two up to the ace.
    static void Whole(array<string> into)
    {
        FromRank("2", into);
    }

    // The cards from this rank up, in the same order: "6" gives the 36.
    static void FromRank(string lowest, array<string> into)
    {
        string ranks = RANKS;
        string suits = SUITS;
        int first = ranks.IndexOf(lowest);
        if (first < 0)
            first = 0;
        for (int s = 0; s < suits.Length(); s++)
        {
            for (int r = first; r < ranks.Length(); r++)
                into.Insert(ranks.Substring(r, 1) + suits.Substring(s, 1));
        }
    }

    // "6s 7h Qd": for the log and the stand.
    static string Line(array<string> cards)
    {
        string text = "";
        for (int i = 0; i < cards.Count(); i++)
        {
            if (i > 0)
                text = text + " ";
            text = text + cards[i];
        }
        return text;
    }

    // The other way: cards parted by spaces or commas. What is not a card is
    // left out.
    static void Parse(string text, array<string> into)
    {
        string plain = text;
        plain.Replace(",", " ");
        array<string> parts = new array<string>();
        plain.Split(" ", parts);
        for (int i = 0; i < parts.Count(); i++)
        {
            if (IsCard(parts[i]))
                into.Insert(parts[i]);
        }
    }
}
