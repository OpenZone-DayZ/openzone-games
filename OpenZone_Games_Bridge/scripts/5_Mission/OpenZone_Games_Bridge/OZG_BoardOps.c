// The stand's ops for the board games (boards and dice design).
class OZG_BoardOps
{
    // split text=<text> by=<separator>: what string.Split makes of it -- the
    // count and the parts between brackets.
    static bool OpSplit(map<string, string> args, out string detail)
    {
        string text = OZG_VerbOps.Arg(args, "text", "a;;b;");
        string by = OZG_VerbOps.Arg(args, "by", ";");
        array<string> parts = new array<string>();
        text.Split(by, parts);
        detail = "split '" + text + "' by '" + by + "': " + parts.Count().ToString();
        for (int i = 0; i < parts.Count(); i++)
            detail = detail + " [" + parts[i] + "]";
        return true;
    }

    // splitlen n=<bytes> parts=<count>: `parts` parts of `n` bytes joined by
    // ';', split by string.Split and by OZG_Text.SplitLong: how many parts
    // each gives back and how long the first is.
    static bool OpSplitLen(map<string, string> args, out string detail)
    {
        int n = OZG_VerbOps.Arg(args, "n", "9000").ToInt();
        int count = OZG_VerbOps.Arg(args, "parts", "2").ToInt();
        string piece = "";
        for (int i = 0; i < n; i++)
            piece = piece + "x";
        string text = piece;
        for (int k = 1; k < count; k++)
            text = text + ";" + piece;
        array<string> a = new array<string>();
        text.Split(";", a);
        array<string> b = new array<string>();
        OZG_Text.SplitLong(text, ";", b);
        int aFirst = -1;
        if (a.Count() > 0)
            aFirst = a[0].Length();
        int bFirst = -1;
        if (b.Count() > 0)
            bFirst = b[0].Length();
        detail = "text " + text.Length().ToString() + " bytes; Split: " + a.Count().ToString() + " parts, the first " + aFirst.ToString();
        detail = detail + "; SplitLong: " + b.Count().ToString() + " parts, the first " + bFirst.ToString();
        return true;
    }

    // perft fen=<FEN> depth=<n>: how many move paths of that length the
    // position has, against the published numbers (the start position: 20,
    // 400, 8902, 197281).
    static bool OpPerft(map<string, string> args, out string detail)
    {
        string fen = OZG_VerbOps.Arg(args, "fen", OZG_ChessConst.START);
        int depth = OZG_VerbOps.Arg(args, "depth", "3").ToInt();
        if (depth < 1 || depth > 5)
        {
            detail = "depth 1..5";
            return false;
        }
        OZG_ChessBoard board = new OZG_ChessBoard();
        if (!board.FromFen(fen))
        {
            detail = "the FEN does not load: " + fen;
            return false;
        }
        int t0 = GetGame().GetTime();
        int paths = board.ChPerft(depth);
        int t1 = GetGame().GetTime();
        detail = "perft " + depth.ToString() + " = " + paths.ToString();
        detail = detail + " in " + (t1 - t0).ToString() + " ms; fen back " + board.ToFen();
        return true;
    }

    // dperft pos=<64 letters of .wWbB and w|b> depth=<n>: draughts' move
    // lines of that length, and the moves of the position with what each
    // takes. No pos: the setup.
    static bool OpDPerft(map<string, string> args, out string detail)
    {
        OZG_DraughtsBoard board = new OZG_DraughtsBoard();
        string pos = OZG_VerbOps.Arg(args, "pos", "");
        if (pos == "")
        {
            board.SetUp();
        }
        else if (!board.FromText(pos))
        {
            detail = "the position does not load: " + pos;
            return false;
        }
        int depth = OZG_VerbOps.Arg(args, "depth", "1").ToInt();
        if (depth < 1 || depth > 8)
        {
            detail = "depth 1..8";
            return false;
        }
        int t0 = GetGame().GetTime();
        int paths = board.DrPerft(depth);
        int t1 = GetGame().GetTime();
        detail = "dperft " + depth.ToString() + " = " + paths.ToString();
        detail = detail + " in " + (t1 - t0).ToString() + " ms; moves:";
        array<string> moves = new array<string>();
        array<string> takes = new array<string>();
        board.DrLegal(moves, takes);
        for (int i = 0; i < moves.Count() && i < 40; i++)
        {
            detail = detail + " " + moves[i];
            if (takes[i] != "")
                detail = detail + "(x" + takes[i] + ")";
        }
        return true;
    }
}
