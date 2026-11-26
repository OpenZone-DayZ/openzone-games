// Draughts and giveaway (boards and dice design §5.2, §5.3): draughts-64, what
// is called Russian draughts elsewhere. 8 x 8, the dark squares, twelve men a
// side, White first. A man steps one square forward and takes forward and
// backward; taking is compulsory, any taking line may be chosen and is played to
// its end; a man reaching the far rank is crowned, in the middle of a taking
// too, and takes on as a king; a king goes any distance along a diagonal and
// lands on any free square beyond what it takes (on one from which the taking
// goes on, when there is one); the pieces taken come off when the move ends and
// none is jumped twice (the Turkish strike rule). Draws: agreement, threefold
// repetition, fifteen moves of kings alone with no taking. In draughts the side
// with no piece or no move loses; in giveaway it wins.
//
// Squares as chess's: a1 = 0 .. h8 = 63, a1 dark. Moves on the wire: "c3-d4",
// takings "c3:e5:c7" (every square landed on).
class OZG_DraughtsConst
{
    static const string GAME_ID = "draughts";
    static const string GIVEAWAY_ID = "giveaway";
    static const string LABEL = "STR_OZG_GAME_DRAUGHTS";
    static const string GIVEAWAY_LABEL = "STR_OZG_GAME_GIVEAWAY";
    static const int EMPTY = 0;
    static const int W_MAN = 1;
    static const int W_KING = 2;
    static const int B_MAN = 3;
    static const int B_KING = 4;
    static const string LETTERS = ".wWbB";
    static const int MEN = 12;
    // Fifteen moves a side of kings alone with nothing taken.
    static const int KING_PLIES = 30;
    static const string END_NO_PIECES = "STR_OZG_DRAUGHTS_END_NO_PIECES";
    static const string END_NO_MOVE = "STR_OZG_DRAUGHTS_END_NO_MOVE";
    static const string END_KINGS = "STR_OZG_DRAUGHTS_END_KINGS";
}

// A position and its moves.
class OZG_DraughtsBoard
{
    int Sq[64];
    int Side;
    // Plies in a row made by a king with nothing taken.
    int KingPlies;

    void Clear()
    {
        for (int i = 0; i < 64; i++)
            Sq[i] = OZG_DraughtsConst.EMPTY;
        Side = 0;
        KingPlies = 0;
    }

    void SetUp()
    {
        Clear();
        for (int sq = 0; sq < 64; sq++)
        {
            if (!Dark(sq))
                continue;
            int rank = sq / 8;
            if (rank <= 2)
                Sq[sq] = OZG_DraughtsConst.W_MAN;
            else if (rank >= 5)
                Sq[sq] = OZG_DraughtsConst.B_MAN;
        }
    }

    void CopyFrom(OZG_DraughtsBoard other)
    {
        for (int i = 0; i < 64; i++)
            Sq[i] = other.Sq[i];
        Side = other.Side;
        KingPlies = other.KingPlies;
    }

    static bool Dark(int sq)
    {
        int f = sq % 8;
        int r = sq / 8;
        return (f + r) % 2 == 0;
    }

    static bool WhiteOf(int p)
    {
        return p == OZG_DraughtsConst.W_MAN || p == OZG_DraughtsConst.W_KING;
    }

    static bool BlackOf(int p)
    {
        return p == OZG_DraughtsConst.B_MAN || p == OZG_DraughtsConst.B_KING;
    }

    static bool KingOf(int p)
    {
        return p == OZG_DraughtsConst.W_KING || p == OZG_DraughtsConst.B_KING;
    }

    static bool SideOf(int p, int side)
    {
        if (side == 0)
            return WhiteOf(p);
        return BlackOf(p);
    }

    // The far rank of a side: where its men are crowned.
    static bool FarRank(int sq, int side)
    {
        int r = sq / 8;
        if (side == 0)
            return r == 7;
        return r == 0;
    }

    // The square one diagonal step from `sq`, -1 off the board. Directions:
    // 0 up-right, 1 up-left, 2 down-right, 3 down-left (up: towards rank 8).
    static int DrNext(int sq, int dir)
    {
        int f = sq % 8;
        int r = sq / 8;
        if (dir == 0 || dir == 2)
            f++;
        else
            f--;
        if (dir == 0 || dir == 1)
            r++;
        else
            r--;
        if (f < 0 || f > 7 || r < 0 || r > 7)
            return -1;
        return r * 8 + f;
    }

    string Squares()
    {
        string s = "";
        string letters = OZG_DraughtsConst.LETTERS;
        for (int i = 0; i < 64; i++)
            s = s + letters.Get(Sq[i]);
        return s;
    }

    string DrKey()
    {
        return Squares() + Side.ToString();
    }

    int Count(int side)
    {
        int n = 0;
        for (int i = 0; i < 64; i++)
        {
            if (SideOf(Sq[i], side))
                n++;
        }
        return n;
    }

    // ---- moves -------------------------------------------------------------------

    // Every legal move of the side to move, and beside each the squares it
    // takes ("" for none, else the names joined with ':').
    void DrLegal(array<string> moves, array<string> takes)
    {
        moves.Clear();
        takes.Clear();
        int sq;
        for (sq = 0; sq < 64; sq++)
        {
            if (SideOf(Sq[sq], Side))
                DrTakings(sq, moves, takes);
        }
        if (moves.Count() > 0)
            return;
        for (sq = 0; sq < 64; sq++)
        {
            if (SideOf(Sq[sq], Side))
                DrQuiet(sq, moves, takes);
        }
    }

    protected void DrQuiet(int from, array<string> moves, array<string> takes)
    {
        int p = Sq[from];
        bool king = KingOf(p);
        for (int dir = 0; dir < 4; dir++)
        {
            if (!king)
            {
                // A man steps forward only.
                bool up = dir == 0 || dir == 1;
                if (up != (Side == 0))
                    continue;
                int to = DrNext(from, dir);
                if (to >= 0 && Sq[to] == OZG_DraughtsConst.EMPTY)
                {
                    moves.Insert(OZG_ChessBoard.SqName(from) + "-" + OZG_ChessBoard.SqName(to));
                    takes.Insert("");
                }
                continue;
            }
            int at = DrNext(from, dir);
            while (at >= 0 && Sq[at] == OZG_DraughtsConst.EMPTY)
            {
                moves.Insert(OZG_ChessBoard.SqName(from) + "-" + OZG_ChessBoard.SqName(at));
                takes.Insert("");
                at = DrNext(at, dir);
            }
        }
    }

    // Every taking line of the piece on `from`.
    protected void DrTakings(int from, array<string> moves, array<string> takes)
    {
        int p = Sq[from];
        array<int> path = new array<int>();
        array<int> taken = new array<int>();
        path.Insert(from);
        // The piece has left its square for the move.
        Sq[from] = OZG_DraughtsConst.EMPTY;
        DrJumps(from, KingOf(p), path, taken, moves, takes);
        Sq[from] = p;
    }

    // From `at` the moving piece (a king when `king`) takes on; every line
    // that can go no further is a move. True when any taking was found.
    protected bool DrJumps(int at, bool king, array<int> path, array<int> taken, array<string> moves, array<string> takes)
    {
        bool any = false;
        for (int dir = 0; dir < 4; dir++)
        {
            if (!king)
            {
                int over = DrNext(at, dir);
                if (over < 0)
                    continue;
                int land = DrNext(over, dir);
                if (land < 0)
                    continue;
                if (!SideOf(Sq[over], 1 - Side) || taken.Find(over) >= 0 || Sq[land] != OZG_DraughtsConst.EMPTY)
                    continue;
                any = true;
                bool crowned = FarRank(land, Side);
                path.Insert(land);
                taken.Insert(over);
                bool more = DrJumps(land, crowned, path, taken, moves, takes);
                if (!more)
                    DrRecord(path, taken, moves, takes);
                path.Remove(path.Count() - 1);
                taken.Remove(taken.Count() - 1);
                continue;
            }
            // A king slides to the first piece on the line.
            int hit = DrNext(at, dir);
            while (hit >= 0 && Sq[hit] == OZG_DraughtsConst.EMPTY)
                hit = DrNext(hit, dir);
            if (hit < 0 || !SideOf(Sq[hit], 1 - Side) || taken.Find(hit) >= 0)
                continue;
            array<int> lands = new array<int>();
            int beyond = DrNext(hit, dir);
            while (beyond >= 0 && Sq[beyond] == OZG_DraughtsConst.EMPTY)
            {
                lands.Insert(beyond);
                beyond = DrNext(beyond, dir);
            }
            if (lands.Count() == 0)
                continue;
            any = true;
            taken.Insert(hit);
            // Where the taking goes on, it must land there.
            array<int> going = new array<int>();
            int i;
            for (i = 0; i < lands.Count(); i++)
            {
                if (DrCanTake(lands[i], taken))
                    going.Insert(lands[i]);
            }
            if (going.Count() > 0)
            {
                for (i = 0; i < going.Count(); i++)
                {
                    int on = going[i];
                    path.Insert(on);
                    DrJumps(on, true, path, taken, moves, takes);
                    path.Remove(path.Count() - 1);
                }
            }
            else
            {
                for (i = 0; i < lands.Count(); i++)
                {
                    int end = lands[i];
                    path.Insert(end);
                    DrRecord(path, taken, moves, takes);
                    path.Remove(path.Count() - 1);
                }
            }
            taken.Remove(taken.Count() - 1);
        }
        return any;
    }

    // Could a king on `at` take anything, with `taken` already taken?
    protected bool DrCanTake(int at, array<int> taken)
    {
        for (int dir = 0; dir < 4; dir++)
        {
            int hit = DrNext(at, dir);
            while (hit >= 0 && Sq[hit] == OZG_DraughtsConst.EMPTY)
                hit = DrNext(hit, dir);
            if (hit < 0 || !SideOf(Sq[hit], 1 - Side) || taken.Find(hit) >= 0)
                continue;
            int beyond = DrNext(hit, dir);
            if (beyond >= 0 && Sq[beyond] == OZG_DraughtsConst.EMPTY)
                return true;
        }
        return false;
    }

    protected void DrRecord(array<int> path, array<int> taken, array<string> moves, array<string> takes)
    {
        string move = OZG_ChessBoard.SqName(path[0]);
        int i;
        for (i = 1; i < path.Count(); i++)
            move = move + ":" + OZG_ChessBoard.SqName(path[i]);
        if (moves.Find(move) >= 0)
            return;
        string off = "";
        for (i = 0; i < taken.Count(); i++)
        {
            if (i > 0)
                off = off + ":";
            off = off + OZG_ChessBoard.SqName(taken[i]);
        }
        moves.Insert(move);
        takes.Insert(off);
    }

    // A legal move made; the letters of the pieces it took, in the order
    // taken.
    string DrDo(string move, string off)
    {
        array<int> path = new array<int>();
        PathOf(move, path);
        int from = path[0];
        int to = path[path.Count() - 1];
        int p = Sq[from];
        bool wasKing = KingOf(p);
        Sq[from] = OZG_DraughtsConst.EMPTY;
        string letters = OZG_DraughtsConst.LETTERS;
        string gone = "";
        array<int> taken = new array<int>();
        PathOf(off, taken);
        for (int i = 0; i < taken.Count(); i++)
        {
            gone = gone + letters.Get(Sq[taken[i]]);
            Sq[taken[i]] = OZG_DraughtsConst.EMPTY;
        }
        // Crowned where a man landed on the far rank, in a taking's middle too.
        bool crown = false;
        for (int k = 1; k < path.Count(); k++)
        {
            if (FarRank(path[k], Side))
                crown = true;
        }
        if (crown && Side == 0)
            p = OZG_DraughtsConst.W_KING;
        else if (crown)
            p = OZG_DraughtsConst.B_KING;
        Sq[to] = p;
        if (wasKing && taken.Count() == 0)
            KingPlies++;
        else
            KingPlies = 0;
        Side = 1 - Side;
        return gone;
    }

    // The squares of a move or of what it takes, in order.
    static void PathOf(string text, array<int> into)
    {
        into.Clear();
        string s = text;
        s.Replace("-", ":");
        array<string> names = new array<string>();
        s.Split(":", names);
        for (int i = 0; i < names.Count(); i++)
        {
            int sq = OZG_ChessBoard.SqOf(names[i]);
            if (sq >= 0)
                into.Insert(sq);
        }
    }

    // Every man on its far rank a king: after a piece was put there by hand.
    void DrCrown()
    {
        for (int i = 0; i < 64; i++)
        {
            if (Sq[i] == OZG_DraughtsConst.W_MAN && FarRank(i, 0))
                Sq[i] = OZG_DraughtsConst.W_KING;
            else if (Sq[i] == OZG_DraughtsConst.B_MAN && FarRank(i, 1))
                Sq[i] = OZG_DraughtsConst.B_KING;
        }
    }

    // For the stand: the number of move lines `depth` plies deep.
    int DrPerft(int depth)
    {
        if (depth <= 0)
            return 1;
        array<string> moves = new array<string>();
        array<string> takes = new array<string>();
        DrLegal(moves, takes);
        if (depth == 1)
            return moves.Count();
        int n = 0;
        OZG_DraughtsBoard keep = new OZG_DraughtsBoard();
        keep.CopyFrom(this);
        for (int i = 0; i < moves.Count(); i++)
        {
            string move = moves[i];
            string off = takes[i];
            DrDo(move, off);
            n = n + DrPerft(depth - 1);
            CopyFrom(keep);
        }
        return n;
    }

    // A position set out from text: 64 letters of LETTERS and the side, e.g.
    // "........(...)w" with 'w' or 'b' last.
    bool FromText(string text)
    {
        if (text.Length() != 65)
            return false;
        string letters = OZG_DraughtsConst.LETTERS;
        for (int i = 0; i < 64; i++)
        {
            int p = letters.IndexOf(text.Get(i));
            if (p < 0)
                return false;
            Sq[i] = p;
        }
        Side = 0;
        if (text.Get(64) == "b")
            Side = 1;
        KingPlies = 0;
        return true;
    }
}

class OZG_Draughts extends OZG_BoardDuel
{
    protected ref OZG_DraughtsBoard m_Pos;

    void OZG_Draughts()
    {
        m_Pos = new OZG_DraughtsBoard();
    }

    override string GameId()
    {
        return OZG_DraughtsConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_DraughtsConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Draughts();
    }

    // Does the side with no piece or no move win (giveaway) or lose?
    protected bool LastWins()
    {
        return false;
    }

    override protected void DuelNeeds(array<string> faces, array<int> counts)
    {
        faces.Insert(OZG_DuelConst.WHITE_DISC);
        counts.Insert(OZG_DraughtsConst.MEN);
        faces.Insert(OZG_DuelConst.BLACK_DISC);
        counts.Insert(OZG_DraughtsConst.MEN);
    }

    override protected void DuelStart()
    {
        if (m_From != "" && m_Pos.FromText(m_From))
            return;
        m_Pos.SetUp();
    }

    override protected int DuelSide()
    {
        return m_Pos.Side;
    }

    override protected void DuelLegal(array<string> into)
    {
        array<string> takes = new array<string>();
        m_Pos.DrLegal(into, takes);
    }

    override protected string DuelKey()
    {
        return m_Pos.DrKey();
    }

    override protected string DuelSquares()
    {
        return m_Pos.Squares();
    }

    override protected void DuelApply(string move)
    {
        array<string> moves = new array<string>();
        array<string> takes = new array<string>();
        m_Pos.DrLegal(moves, takes);
        int at = moves.Find(move);
        if (at < 0)
            return;
        int mover = m_Pos.Side;
        string gone = m_Pos.DrDo(move, takes[at]);
        m_Taken = m_Taken + gone;
        m_Played.Insert(move);
        m_Hist.Insert(move);
        m_Last = move;
        bool thrice = KeepKey();
        if (DrEnded())
            return;
        if (thrice)
            End(OZG_DuelConst.DRAWN, OZG_DuelConst.END_REPETITION);
        else if (m_Pos.KingPlies >= OZG_DraughtsConst.KING_PLIES)
            End(OZG_DuelConst.DRAWN, OZG_DraughtsConst.END_KINGS);
    }

    // The side to move with no piece or no move: it has lost (won, in
    // giveaway). True when that ended the game.
    protected bool DrEnded()
    {
        int next = m_Pos.Side;
        array<string> after = new array<string>();
        array<string> afterTakes = new array<string>();
        m_Pos.DrLegal(after, afterTakes);
        string why = "";
        if (m_Pos.Count(next) == 0)
            why = OZG_DraughtsConst.END_NO_PIECES;
        else if (after.Count() == 0)
            why = OZG_DraughtsConst.END_NO_MOVE;
        if (why == "")
            return false;
        if (LastWins())
            End(WinOfSide(next), why);
        else
            End(WinOfSide(1 - next), why);
        return true;
    }

    // A disc moved, swapped, put back or taken off by hand: dark squares only,
    // a man on its far rank crowned.
    override protected bool DuelSleight(string kind, string a, string b, out string refusal)
    {
        refusal = OZG_DuelConst.NO_SLEIGHT;
        int from = OZG_ChessBoard.SqOf(a);
        int to = OZG_ChessBoard.SqOf(b);
        string letters = OZG_DraughtsConst.LETTERS;
        if (kind == OZG_DuelConst.S_MOVE)
        {
            if (from < 0 || to < 0 || !OZG_DraughtsBoard.Dark(to))
                return false;
            if (m_Pos.Sq[from] == OZG_DraughtsConst.EMPTY || m_Pos.Sq[to] != OZG_DraughtsConst.EMPTY)
                return false;
            m_Pos.Sq[to] = m_Pos.Sq[from];
            m_Pos.Sq[from] = OZG_DraughtsConst.EMPTY;
        }
        else if (kind == OZG_DuelConst.S_SWAP)
        {
            if (from < 0 || to < 0 || from == to)
                return false;
            if (m_Pos.Sq[from] == OZG_DraughtsConst.EMPTY || m_Pos.Sq[to] == OZG_DraughtsConst.EMPTY)
                return false;
            int keep = m_Pos.Sq[to];
            m_Pos.Sq[to] = m_Pos.Sq[from];
            m_Pos.Sq[from] = keep;
        }
        else if (kind == OZG_DuelConst.S_OFF)
        {
            if (from < 0 || m_Pos.Sq[from] == OZG_DraughtsConst.EMPTY)
                return false;
            m_Taken = m_Taken + letters.Get(m_Pos.Sq[from]);
            m_Pos.Sq[from] = OZG_DraughtsConst.EMPTY;
        }
        else if (kind == OZG_DuelConst.S_BACK)
        {
            int piece = letters.IndexOf(a);
            if (piece <= 0 || to < 0 || !OZG_DraughtsBoard.Dark(to) || m_Taken.IndexOf(a) < 0)
                return false;
            if (m_Pos.Sq[to] != OZG_DraughtsConst.EMPTY)
                m_Taken = m_Taken + letters.Get(m_Pos.Sq[to]);
            TakeBack(a);
            m_Pos.Sq[to] = piece;
        }
        else
        {
            return false;
        }
        m_Pos.DrCrown();
        m_Pos.KingPlies = 0;
        refusal = "";
        return true;
    }

    override protected void DuelAfterSleight()
    {
        DrEnded();
    }

    override protected void DuelTake(OZG_BoardDuel from)
    {
        OZG_Draughts same = OZG_Draughts.Cast(from);
        if (same)
            m_Pos = same.m_Pos;
    }

    override protected bool DuelSound(out string why)
    {
        why = "";
        int white = m_Pos.Count(0);
        int black = m_Pos.Count(1);
        if (white > OZG_DraughtsConst.MEN || black > OZG_DraughtsConst.MEN)
        {
            why = "men " + white.ToString() + "/" + black.ToString();
            return false;
        }
        for (int i = 0; i < 64; i++)
        {
            if (m_Pos.Sq[i] != OZG_DraughtsConst.EMPTY && !OZG_DraughtsBoard.Dark(i))
            {
                why = "a piece on a light square " + OZG_ChessBoard.SqName(i);
                return false;
            }
        }
        return true;
    }

    // For the stand.
    int Perft(int depth)
    {
        return m_Pos.DrPerft(depth);
    }

    string PositionText()
    {
        string side = "w";
        if (m_Pos.Side == 1)
            side = "b";
        return m_Pos.Squares() + side;
    }
}

class OZG_Giveaway extends OZG_Draughts
{
    override string GameId()
    {
        return OZG_DraughtsConst.GIVEAWAY_ID;
    }

    override string LabelKey()
    {
        return OZG_DraughtsConst.GIVEAWAY_LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Giveaway();
    }

    override protected bool LastWins()
    {
        return true;
    }
}
