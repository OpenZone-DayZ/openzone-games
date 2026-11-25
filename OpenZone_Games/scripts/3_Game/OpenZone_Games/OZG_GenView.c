// The table view of the games after durak (stakes-and-games design §4): one
// shape every such game describes its table in, and one drawing of it in the
// window. Durak keeps its own view (OZG_DurakPublic) and its own drawing.
//
// EVERY TEXT IS WORDS: a key of the string table and its arguments, joined by
// `|` (OZG_Words). An argument `c:<card>` is worded as a card, `s:<seat>` as
// the name of that seat, `k:<key>` as another key; anything else stays as it
// is (a number). The server words nothing itself: each client in its language.

// A seat dealt in, as everybody sees it.
class OZG_GSeat
{
    int SeatNo;
    // The cards it holds face down, by the look of each back, the lowest
    // first: a fan of backs.
    ref array<int> Backs;
    // Its cards that lie face up for everybody (a dummy, a showdown, an open
    // whist), with the look of each face.
    ref array<string> Faces;
    ref array<int> FaceLooks;
    // What it does now, and its numbers (a stack and a bet, tricks): words.
    string Role;
    string Line;
    // 1: out of this deal (folded, busted, sitting out).
    int Out;
    // 1: the dealer, the button, the banker -- a mark on its plate.
    int Mark;

    void OZG_GSeat()
    {
        Backs = new array<int>();
        Faces = new array<string>();
        FaceLooks = new array<int>();
        SeatNo = -1;
    }
}

// A pile in the middle of the table: the board, the trick being played, the
// talon, the pot. Its face-up cards first, then the backs on it.
class OZG_GZone
{
    string Id;
    // Words above it and under it.
    string Label;
    string Line;
    ref array<string> Faces;
    ref array<int> Looks;
    // Beside Faces: the seat that laid each card, -1 for none.
    ref array<int> By;
    int Backs;
    int BackLook;

    void OZG_GZone()
    {
        Faces = new array<string>();
        Looks = new array<int>();
        By = new array<int>();
    }
}

// A move that names no card, as a button: a bid, "Hit", "Call 40".
class OZG_GButton
{
    string Move;
    // Its label: words.
    string Label;
    // Its place in the grid of bids; Row -1: the row of actions.
    int Row;
    int Col;
    // 1: drawn as the main choice.
    int Hot;

    static OZG_GButton Make(string move, string label, int row, int col)
    {
        OZG_GButton b = new OZG_GButton();
        b.Move = move;
        b.Label = label;
        b.Row = row;
        b.Col = col;
        b.Hot = 0;
        return b;
    }
}

// The public half of a view.
class OZG_GPublic
{
    string GameId;
    // Words for everybody: whose move it is, the contract.
    string Status;
    // "s", "h", "d", "c" while a suit is trump; "n" for none; "" for a game
    // that knows no trump.
    string Trump;
    ref array<ref OZG_GSeat> SeatList;
    ref array<ref OZG_GZone> Zones;
    // The seats the game stands still for.
    ref array<int> Awaited;
    int Ended;
    // Words: how the deal ended.
    string Result;
    // The score sheet: a line of words each.
    ref array<string> Score;

    void OZG_GPublic()
    {
        SeatList = new array<ref OZG_GSeat>();
        Zones = new array<ref OZG_GZone>();
        Awaited = new array<int>();
        Score = new array<string>();
    }
}

// A seat's own half: its hand, what it may do, an amount to type.
class OZG_GPrivate
{
    ref array<string> Held;
    ref array<int> HeldLooks;
    // Moves that name a card (OZG_Cards.OfMove): a click on that card -- in
    // the hand, or among the faces of a seat it plays for (the dummy) --
    // makes the move.
    ref array<string> CanPlay;
    ref array<ref OZG_GButton> Buttons;
    // Words for this seat alone: "your move: ...".
    string Status;
    // An amount to type: the move it makes is AmountMove followed by the
    // number, sent by a button labelled AmountLabel; the bounds and the
    // number the field starts with.
    string AmountMove;
    string AmountLabel;
    int AmountMin;
    int AmountMax;
    int AmountStart;

    void OZG_GPrivate()
    {
        Held = new array<string>();
        HeldLooks = new array<int>();
        CanPlay = new array<string>();
        Buttons = new array<ref OZG_GButton>();
    }
}

// Words: a key and its arguments in one string.
class OZG_Words
{
    static const string SEP = "|";

    static string W0(string key)
    {
        return key;
    }

    static string W1(string key, string a)
    {
        return key + SEP + a;
    }

    static string W2(string key, string a, string b)
    {
        string s = key + SEP + a;
        s = s + SEP + b;
        return s;
    }

    static string W3(string key, string a, string b, string c)
    {
        string s = key + SEP + a;
        s = s + SEP + b;
        s = s + SEP + c;
        return s;
    }

    static string W4(string key, string a, string b, string c, string d)
    {
        string s = key + SEP + a;
        s = s + SEP + b;
        s = s + SEP + c;
        s = s + SEP + d;
        return s;
    }

    static string CardArg(string card)
    {
        return "c:" + card;
    }

    static string SeatArg(int seat)
    {
        return "s:" + seat.ToString();
    }

    static string KeyArg(string key)
    {
        return "k:" + key;
    }

    static string NumArg(int n)
    {
        return n.ToString();
    }

    // The parts of words: [key, arg, arg ...].
    static void Parts(string words, array<string> into)
    {
        into.Clear();
        if (words == "")
            return;
        words.Split(SEP, into);
    }
}
