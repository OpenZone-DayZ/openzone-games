// Corners (boards and dice design §5.4): each side's discs in its corner, 9
// (3 x 3) by default or 12 (4 x 3) as the host's option; White's corner from a1,
// Black's from h8. A move is one step to a free square beside, along a rank or a
// file, or a chain of jumps over a disc beside (of either colour) onto the free
// square beyond, as long as the player likes. The first to fill the other's
// corner wins; if Black fills it on the move right after White, it is a draw. A
// disc still in its own corner after its side's 40th move loses the game for
// that side. A side with no move passes.
//
// Squares as chess's; steps on the wire as "a1-a2", jumps as "a1:a3:c3".
class OZG_CornersConst
{
    static const string GAME_ID = "corners";
    static const string LABEL = "STR_OZG_GAME_CORNERS";
    static const string OPT_SIZE = "size";
    static const string SIZE_9 = "9";
    static const string SIZE_12 = "12";
    static const int EMPTY = 0;
    static const int WHITE = 1;
    static const int BLACK = 3;
    static const string LETTERS = ".w.b";
    static const int LEAVE_BY = 40;
    static const string M_PASS = "pass";
    static const string END_FILLED = "STR_OZG_CORNERS_END_FILLED";
    static const string END_BOTH = "STR_OZG_CORNERS_END_BOTH";
    static const string END_STAYED = "STR_OZG_CORNERS_END_STAYED";
    static const string END_STUCK = "STR_OZG_CORNERS_END_STUCK";
}

class OZG_CornersBoard
{
    int Sq[64];
    int Side;
    // The corner's width (files) and height (ranks): 3 x 3 or 4 x 3.
    int Wide = 3;
    int High = 3;

    void SetUp(int wide, int high)
    {
        Wide = wide;
        High = high;
        for (int i = 0; i < 64; i++)
        {
            Sq[i] = OZG_CornersConst.EMPTY;
            if (InCorner(i, 0))
                Sq[i] = OZG_CornersConst.WHITE;
            else if (InCorner(i, 1))
                Sq[i] = OZG_CornersConst.BLACK;
        }
        Side = 0;
    }

    // A position from text: 64 letters of ".wb" and the side, w or b.
    bool FromText(string text)
    {
        if (text.Length() != 65)
            return false;
        for (int i = 0; i < 64; i++)
        {
            string c = text.Get(i);
            if (c == ".")
                Sq[i] = OZG_CornersConst.EMPTY;
            else if (c == "w")
                Sq[i] = OZG_CornersConst.WHITE;
            else if (c == "b")
                Sq[i] = OZG_CornersConst.BLACK;
            else
                return false;
        }
        Side = 0;
        if (text.Get(64) == "b")
            Side = 1;
        return true;
    }

    void CopyFrom(OZG_CornersBoard other)
    {
        for (int i = 0; i < 64; i++)
            Sq[i] = other.Sq[i];
        Side = other.Side;
        Wide = other.Wide;
        High = other.High;
    }

    // Is `sq` in the corner a side starts from? White's is at a1, Black's at h8.
    bool InCorner(int sq, int side)
    {
        int f = sq % 8;
        int r = sq / 8;
        if (side == 0)
            return f < Wide && r < High;
        return f >= 8 - Wide && r >= 8 - High;
    }

    static int Piece(int side)
    {
        if (side == 0)
            return OZG_CornersConst.WHITE;
        return OZG_CornersConst.BLACK;
    }

    // The square one step from `sq` along a rank or file, -1 off the board.
    // Directions: 0 up, 1 down, 2 right, 3 left.
    static int CoNext(int sq, int dir)
    {
        int f = sq % 8;
        int r = sq / 8;
        if (dir == 0)
            r++;
        else if (dir == 1)
            r--;
        else if (dir == 2)
            f++;
        else
            f--;
        if (f < 0 || f > 7 || r < 0 || r > 7)
            return -1;
        return r * 8 + f;
    }

    string Squares()
    {
        string s = "";
        string letters = OZG_CornersConst.LETTERS;
        for (int i = 0; i < 64; i++)
            s = s + letters.Get(Sq[i]);
        return s;
    }

    // Has `side` filled the other's corner?
    bool Filled(int side)
    {
        int mine = Piece(side);
        for (int i = 0; i < 64; i++)
        {
            if (InCorner(i, 1 - side) && Sq[i] != mine)
                return false;
        }
        return true;
    }

    // Is a disc of `side` still in its own corner?
    bool Stayed(int side)
    {
        int mine = Piece(side);
        for (int i = 0; i < 64; i++)
        {
            if (InCorner(i, side) && Sq[i] == mine)
                return true;
        }
        return false;
    }

    int Count(int side)
    {
        int mine = Piece(side);
        int n = 0;
        for (int i = 0; i < 64; i++)
        {
            if (Sq[i] == mine)
                n++;
        }
        return n;
    }

    void CoLegal(array<string> into)
    {
        into.Clear();
        int mine = Piece(Side);
        for (int sq = 0; sq < 64; sq++)
        {
            if (Sq[sq] != mine)
                continue;
            for (int dir = 0; dir < 4; dir++)
            {
                int to = CoNext(sq, dir);
                if (to >= 0 && Sq[to] == OZG_CornersConst.EMPTY)
                    into.Insert(OZG_ChessBoard.SqName(sq) + "-" + OZG_ChessBoard.SqName(to));
            }
            array<int> path = new array<int>();
            path.Insert(sq);
            Sq[sq] = OZG_CornersConst.EMPTY;
            CoJumps(sq, path, into);
            Sq[sq] = mine;
        }
    }

    // Every chain of jumps on from `at`; each square landed on ends a move.
    protected void CoJumps(int at, array<int> path, array<string> into)
    {
        for (int dir = 0; dir < 4; dir++)
        {
            int over = CoNext(at, dir);
            if (over < 0 || Sq[over] == OZG_CornersConst.EMPTY)
                continue;
            int land = CoNext(over, dir);
            if (land < 0 || Sq[land] != OZG_CornersConst.EMPTY || path.Find(land) >= 0)
                continue;
            path.Insert(land);
            string move = OZG_ChessBoard.SqName(path[0]);
            for (int i = 1; i < path.Count(); i++)
                move = move + ":" + OZG_ChessBoard.SqName(path[i]);
            if (into.Find(move) < 0)
                into.Insert(move);
            CoJumps(land, path, into);
            path.Remove(path.Count() - 1);
        }
    }

    void CoDo(string move)
    {
        array<int> path = new array<int>();
        OZG_DraughtsBoard.PathOf(move, path);
        int from = path[0];
        int to = path[path.Count() - 1];
        int p = Sq[from];
        Sq[from] = OZG_CornersConst.EMPTY;
        Sq[to] = p;
        Side = 1 - Side;
    }
}

class OZG_Corners extends OZG_BoardDuel
{
    protected ref OZG_CornersBoard m_Pos;
    protected int m_Size = 9;
    // The ply White filled Black's corner on, -1 before.
    protected int m_WhiteFilled = -1;
    // Moves made by each side.
    protected int m_WhiteMoves;
    protected int m_BlackMoves;

    void OZG_Corners()
    {
        m_Pos = new OZG_CornersBoard();
    }

    override string GameId()
    {
        return OZG_CornersConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_CornersConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Corners();
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        OZG_Option o = new OZG_Option();
        o.OptionId = OZG_CornersConst.OPT_SIZE;
        o.LabelKey = "STR_OZG_CORNERS_SIZE";
        o.ValueIds.Insert(OZG_CornersConst.SIZE_9);
        o.ValueKeys.Insert("STR_OZG_CORNERS_9");
        o.ValueIds.Insert(OZG_CornersConst.SIZE_12);
        o.ValueKeys.Insert("STR_OZG_CORNERS_12");
        o.DefaultId = OZG_CornersConst.SIZE_9;
        into.Insert(o);
    }

    override protected void DuelOptions(map<string, string> options)
    {
        m_Size = 9;
        if (OptionOf(options, OZG_CornersConst.OPT_SIZE, OZG_CornersConst.SIZE_9) == OZG_CornersConst.SIZE_12)
            m_Size = 12;
    }

    override protected void DuelNeeds(array<string> faces, array<int> counts)
    {
        faces.Insert(OZG_DuelConst.WHITE_DISC);
        counts.Insert(m_Size);
        faces.Insert(OZG_DuelConst.BLACK_DISC);
        counts.Insert(m_Size);
    }

    override protected void DuelStart()
    {
        int wide = 3;
        if (m_Size == 12)
            wide = 4;
        m_Pos.SetUp(wide, 3);
        if (m_From != "")
            m_Pos.FromText(m_From);
        m_WhiteFilled = -1;
        m_WhiteMoves = 0;
        m_BlackMoves = 0;
    }

    override protected int DuelSide()
    {
        return m_Pos.Side;
    }

    override protected void DuelLegal(array<string> into)
    {
        m_Pos.CoLegal(into);
    }

    override protected string DuelKey()
    {
        return m_Pos.Squares() + m_Pos.Side.ToString();
    }

    override protected string DuelSquares()
    {
        return m_Pos.Squares();
    }

    override protected void DuelApply(string move)
    {
        int mover = m_Pos.Side;
        m_Pos.CoDo(move);
        m_Played.Insert(move);
        m_Hist.Insert(move);
        m_Last = move;
        KeepKey();
        int made;
        if (mover == 0)
        {
            m_WhiteMoves++;
            made = m_WhiteMoves;
        }
        else
        {
            m_BlackMoves++;
            made = m_BlackMoves;
        }

        if (mover == 0)
        {
            // White has filled: Black has one move to fill as well.
            if (m_Pos.Filled(0) && m_WhiteFilled < 0)
                m_WhiteFilled = m_Played.Count();
        }
        else
        {
            bool blackFilled = m_Pos.Filled(1);
            if (m_WhiteFilled >= 0 && blackFilled)
            {
                End(OZG_DuelConst.DRAWN, OZG_CornersConst.END_BOTH);
                return;
            }
            if (m_WhiteFilled >= 0)
            {
                End(OZG_DuelConst.WIN_WHITE, OZG_CornersConst.END_FILLED);
                return;
            }
            if (blackFilled)
            {
                End(OZG_DuelConst.WIN_BLACK, OZG_CornersConst.END_FILLED);
                return;
            }
        }
        if (made == OZG_CornersConst.LEAVE_BY && m_Pos.Stayed(mover))
        {
            End(WinOfSide(1 - mover), OZG_CornersConst.END_STAYED);
            return;
        }
        // The side to move with no move passes; with neither able to, the
        // game stops drawn.
        array<string> next = new array<string>();
        m_Pos.CoLegal(next);
        if (next.Count() > 0)
            return;
        m_Pos.Side = mover;
        array<string> again = new array<string>();
        m_Pos.CoLegal(again);
        if (again.Count() == 0)
        {
            End(OZG_DuelConst.DRAWN, OZG_CornersConst.END_STUCK);
            return;
        }
        m_Hist.Insert(OZG_CornersConst.M_PASS);
    }

    override protected void DuelTake(OZG_BoardDuel from)
    {
        OZG_Corners same = OZG_Corners.Cast(from);
        if (!same)
            return;
        m_Pos = same.m_Pos;
        m_Size = same.m_Size;
        m_WhiteFilled = same.m_WhiteFilled;
        m_WhiteMoves = same.m_WhiteMoves;
        m_BlackMoves = same.m_BlackMoves;
    }

    // A disc moved, swapped, put back or taken off by hand.
    override protected bool DuelSleight(string kind, string a, string b, out string refusal)
    {
        refusal = OZG_DuelConst.NO_SLEIGHT;
        int from = OZG_ChessBoard.SqOf(a);
        int to = OZG_ChessBoard.SqOf(b);
        string letters = OZG_CornersConst.LETTERS;
        if (kind == OZG_DuelConst.S_MOVE)
        {
            if (from < 0 || to < 0 || m_Pos.Sq[from] == OZG_CornersConst.EMPTY || m_Pos.Sq[to] != OZG_CornersConst.EMPTY)
                return false;
            m_Pos.Sq[to] = m_Pos.Sq[from];
            m_Pos.Sq[from] = OZG_CornersConst.EMPTY;
        }
        else if (kind == OZG_DuelConst.S_SWAP)
        {
            if (from < 0 || to < 0 || from == to)
                return false;
            if (m_Pos.Sq[from] == OZG_CornersConst.EMPTY || m_Pos.Sq[to] == OZG_CornersConst.EMPTY)
                return false;
            int keep = m_Pos.Sq[to];
            m_Pos.Sq[to] = m_Pos.Sq[from];
            m_Pos.Sq[from] = keep;
        }
        else if (kind == OZG_DuelConst.S_OFF)
        {
            if (from < 0 || m_Pos.Sq[from] == OZG_CornersConst.EMPTY)
                return false;
            m_Taken = m_Taken + letters.Get(m_Pos.Sq[from]);
            m_Pos.Sq[from] = OZG_CornersConst.EMPTY;
        }
        else if (kind == OZG_DuelConst.S_BACK)
        {
            int piece = letters.IndexOf(a);
            if ((a != "w" && a != "b") || to < 0 || m_Taken.IndexOf(a) < 0)
                return false;
            if (m_Pos.Sq[to] != OZG_CornersConst.EMPTY)
                m_Taken = m_Taken + letters.Get(m_Pos.Sq[to]);
            TakeBack(a);
            m_Pos.Sq[to] = piece;
        }
        else
        {
            return false;
        }
        refusal = "";
        return true;
    }

    protected int TakenOf(string letter)
    {
        int n = 0;
        for (int i = 0; i < m_Taken.Length(); i++)
        {
            if (m_Taken.Get(i) == letter)
                n++;
        }
        return n;
    }

    override protected bool DuelSound(out string why)
    {
        why = "";
        int white = m_Pos.Count(0) + TakenOf("w");
        int black = m_Pos.Count(1) + TakenOf("b");
        if (white != m_Size || black != m_Size)
        {
            why = "discs " + white.ToString() + "/" + black.ToString();
            return false;
        }
        return true;
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (seat != SeatToMove())
            return "";
        array<string> legal = new array<string>();
        DuelLegal(legal);
        if (legal.Count() == 0)
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        // A move that ends nearer the other's corner first, when there is one.
        int side = m_Pos.Side;
        array<string> forward = new array<string>();
        for (int i = 0; i < legal.Count(); i++)
        {
            array<int> path = new array<int>();
            OZG_DraughtsBoard.PathOf(legal[i], path);
            int from = path[0];
            int to = path[path.Count() - 1];
            int gain = (to % 8 + to / 8) - (from % 8 + from / 8);
            if (side == 1)
                gain = -gain;
            if (gain > 0)
                forward.Insert(legal[i]);
        }
        if (forward.Count() > 0)
            return forward[dice.Below(forward.Count())];
        return legal[dice.Below(legal.Count())];
    }

    string PositionText()
    {
        return m_Pos.Squares() + m_Pos.Side.ToString();
    }
}
