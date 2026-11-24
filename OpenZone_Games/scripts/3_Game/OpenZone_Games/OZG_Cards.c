// Cards. A card is a rank of RANKS and a suit of SUITS: `6s` is the six of
// spades, `Th` the ten of hearts (design §8).
//
// A DECK MAY HOLD ANY NUMBER OF ONE FACE. Cards are things, and a deck can be
// put together of aces alone. In a deal the first card of a face is named by
// the face, `As`, and every further one by the face, a dot and its number:
// `As.1`, `As.2`, with no end to it. So A CARD'S NAME HAS NO FIXED WIDTH:
// nothing may cut it by position. The rules ask RankOf and SuitOf; whoever
// needs the face for a picture or a word asks FaceOf; whoever takes a card
// out of a move asks OfMove.
//
// THE NAME ONLY TELLS CARDS APART. What a card is besides its face (the look
// of its back, and from the next phase its mark and who sees it) is kept
// beside the name, a record a card (OZG_DealCard below): two aces of spades
// may wear one back or two, be marked or clean, and both ways they are two
// cards.
class OZG_Cards
{
    static const string RANKS = "23456789TJQKA";
    static const string SUITS = "shdc";
    // Between a face and the number of a further card of that face.
    static const string COPY_MARK = ".";

    // Whether a text is a card's name, and which card of its face it names:
    // 0 for the first, 1 and up for the others.
    protected static bool WellMade(string card, out int copy)
    {
        copy = 0;
        int n = card.Length();
        if (n < 2)
            return false;
        string ranks = RANKS;
        if (ranks.IndexOf(card.Substring(0, 1)) < 0)
            return false;
        string suits = SUITS;
        if (suits.IndexOf(card.Substring(1, 1)) < 0)
            return false;
        if (n == 2)
            return true;
        // A further card: the mark and a number from 1 up, and nothing else.
        if (n < 4 || card.Substring(2, 1) != COPY_MARK)
            return false;
        string digits = card.Substring(3, n - 3);
        int number = digits.ToInt();
        if (number < 1 || number.ToString() != digits)
            return false;
        copy = number;
        return true;
    }

    // 0 for a two .. 12 for an ace; -1 for what is not a card.
    static int RankOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return -1;
        string ranks = RANKS;
        return ranks.IndexOf(card.Substring(0, 1));
    }

    // "s", "h", "d" or "c"; "" for what is not a card.
    static string SuitOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return "";
        return card.Substring(1, 1);
    }

    // 0 for the first card of a face, 1 and up for the others; -1 for what is
    // not a card.
    static int CopyOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return -1;
        return copy;
    }

    // The face: the name of the first card of its kind. `As.3` is `As`.
    static string FaceOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return "";
        return card.Substring(0, 2);
    }

    // The name of the n-th card of a face; the face itself for 0.
    static string CopyNamed(string face, int copy)
    {
        string plain = FaceOf(face);
        if (plain == "" || copy < 0)
            return "";
        if (copy == 0)
            return plain;
        return plain + COPY_MARK + copy.ToString();
    }

    static bool IsCard(string card)
    {
        int copy;
        return WellMade(card, copy);
    }

    // The card a move names: `a:<card>`, `d:<card>:<n>`, `t:<card>`. "" for a
    // move that names none (`take`, `pass`).
    static string OfMove(string move)
    {
        int first = move.IndexOf(":");
        if (first < 0)
            return "";
        string named = move.Substring(first + 1, move.Length() - first - 1);
        int second = named.IndexOf(":");
        if (second >= 0)
            named = named.Substring(0, second);
        if (!IsCard(named))
            return "";
        return named;
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

// One card of a deal, as a game is handed it. The name tells it apart from
// every other card of the deal; the rest is what the card is like. The next
// phases add what else a card carries into a game: the sign of its mark, and
// which seats read it and which notice it (design §4).
class OZG_DealCard
{
    // A face, or for a further card of that face the face, a dot and its
    // number: `As`, `As.1`, `As.2` (OZG_Cards.CopyNamed).
    string Name;
    // The look of its back (OZG_Decks.StyleAt).
    int Look;

    static OZG_DealCard Make(string name, int look)
    {
        OZG_DealCard c = new OZG_DealCard();
        c.Name = name;
        c.Look = look;
        return c;
    }
}
