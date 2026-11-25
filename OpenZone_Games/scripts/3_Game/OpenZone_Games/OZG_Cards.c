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
// of its back, its mark and who sees it) is kept
// beside the name, a record a card (OZG_DealCard below): two aces of spades
// may wear one back or two, be marked or clean, and both ways they are two
// cards.
//
// THE JOKERS: `Xr`, the red one, and `Xb`, the black one. A deck whose config
// says ozgJokers = 1 is filled with them besides the fifty-two. A joker is a
// card (IsCard, FaceOf) with no rank and no suit (RankOf -1, SuitOf ""), and
// no game of this mod deals one: a game is handed the cards of its own set
// (OZG_Game.CardsUsed), and no set names a joker.
class OZG_Cards
{
    static const string RANKS = "23456789TJQKA";
    static const string SUITS = "shdc";
    // A joker's first letter, and its colours: red, black.
    static const string JOKER = "X";
    static const string JOKER_COLOURS = "rb";
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
        string first = card.Substring(0, 1);
        string second = card.Substring(1, 1);
        string ranks = RANKS;
        string suits = SUITS;
        string colours = JOKER_COLOURS;
        bool plain = ranks.IndexOf(first) >= 0 && suits.IndexOf(second) >= 0;
        bool joker = first == JOKER && colours.IndexOf(second) >= 0;
        if (!plain && !joker)
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

    // 0 for a two .. 12 for an ace; -1 for what is not a card, and for a
    // joker, which has no rank.
    static int RankOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return -1;
        string ranks = RANKS;
        return ranks.IndexOf(card.Substring(0, 1));
    }

    // "s", "h", "d" or "c"; "" for what is not a card, and for a joker.
    static string SuitOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return "";
        if (card.Substring(0, 1) == JOKER)
            return "";
        return card.Substring(1, 1);
    }

    // Whether a card is a joker, the first of its face or a further one.
    static bool IsJoker(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return false;
        return card.Substring(0, 1) == JOKER;
    }

    // The two jokers, the red one first.
    static void Jokers(array<string> into)
    {
        string colours = JOKER_COLOURS;
        for (int i = 0; i < colours.Length(); i++)
            into.Insert(JOKER + colours.Substring(i, 1));
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
// every other card of the deal; the rest is what the card is like: the look
// of its back and its mark as it was when the deal began (design §3.4, §4.1).
// Server only: no record goes into a view.
class OZG_DealCard
{
    // A face, or for a further card of that face the face, a dot and its
    // number: `As`, `As.1`, `As.2` (OZG_Cards.CopyNamed).
    string Name;
    // The look of its back (OZG_Decks.StyleAt).
    int Look;
    // 0: a clean card; 1..99 the sign of its mark.
    int Sign;
    // The ids that read it and that notice it.
    ref array<string> Readers;
    ref array<string> Noticers;

    static OZG_DealCard Make(string name, int look)
    {
        OZG_DealCard c = new OZG_DealCard();
        c.Name = name;
        c.Look = look;
        c.Sign = 0;
        c.Readers = new array<string>();
        c.Noticers = new array<string>();
        return c;
    }

    // A record made by script from one the JSON loader filled, or from one
    // a game was handed: every member of a record written since VERSION 3.
    OZG_DealCard Copy()
    {
        OZG_DealCard c = Make(Name, Look);
        c.Sign = Sign;
        if (Readers)
            c.Readers.Copy(Readers);
        if (Noticers)
            c.Noticers.Copy(Noticers);
        return c;
    }

    bool ReadBy(string uid)
    {
        if (Sign <= 0 || uid == "" || !Readers)
            return false;
        return Readers.Find(uid) >= 0;
    }
}
