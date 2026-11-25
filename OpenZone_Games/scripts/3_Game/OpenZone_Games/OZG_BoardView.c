// What a board game tells the window (boards and dice design §7). It is the
// table view of the later card games (OZG_GPublic, OZG_GPrivate) and more: the
// seats' plates, whose move it is, the buttons and the log are drawn as theirs
// are; the board is drawn from what is added here. JSON on the wire.
class OZG_BPublic extends OZG_GPublic
{
    // The squares a1, b1 .. h1, a2 .. h8: a FEN letter a piece ("PNBRQK" white,
    // "pnbrqk" black; "W" and "B" a white and a black checker), "." an empty one.
    string Board;
    // The seat to move; -1 when the game is over.
    int Turn;
    // The seats of the two sides.
    int White;
    int Black;
    // The last move as on the wire ("e2e4"), its squares lit in the window.
    string Last;
    // The square of a king in check, -1 for none.
    int Check;
    // Every half-move in long notation, the piece's English letter first
    // ("Ng1-f3", "e7-e8=Q", "O-O"); the window puts its own letters on.
    ref array<string> History;
    // The pieces taken, as FEN letters.
    string Taken;
    // The seat that offers a draw, -1 for none.
    int Offer;
    // "1-0", "0-1", "1/2" when over; "" while it goes on.
    string Final;
    // Why it ended: a key of the string table.
    string Reason;

    void OZG_BPublic()
    {
        History = new array<string>();
        Turn = -1;
        White = -1;
        Black = -1;
        Check = -1;
        Offer = -1;
    }

    static string Text(OZG_BPublic v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_BPublic>.MakeData(v, text, err, false);
        return text;
    }
}

// A seat's own half: CanPlay holds its board moves ("e2e4", "e7e8q"); the
// moves that are not on the board ("resign", "draw", "accept", "decline") are
// its Buttons.
class OZG_BPrivate extends OZG_GPrivate
{
    static string Text(OZG_BPrivate v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_BPrivate>.MakeData(v, text, err, false);
        return text;
    }
}
