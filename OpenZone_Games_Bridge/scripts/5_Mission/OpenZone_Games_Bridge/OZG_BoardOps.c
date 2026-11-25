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
}
