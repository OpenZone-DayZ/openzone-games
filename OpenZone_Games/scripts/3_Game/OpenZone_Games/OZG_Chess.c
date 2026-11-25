// Chess by the FIDE Laws, without a clock (boards and dice design §5.1).
//
// The board is 64 squares, a1 = 0, h1 = 7, a8 = 56. A piece is its kind
// (PAWN .. KING) plus BLACK for a black one; 0 is an empty square. A move is
// one int: from | to << 6 | promotion kind << 12.
class OZG_ChessConst
{
    static const string GAME_ID = "chess";
    static const string LABEL = "STR_OZG_GAME_CHESS";

    static const int PAWN = 1;
    static const int KNIGHT = 2;
    static const int BISHOP = 3;
    static const int ROOK = 4;
    static const int QUEEN = 5;
    static const int KING = 6;
    static const int BLACK = 8;
    // A FEN letter by kind; lower case for black.
    static const string LETTERS = ".PNBRQK";
    static const string FILES = "abcdefgh";

    static const string START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // The moves that are not on the board.
    static const string M_RESIGN = "resign";
    static const string M_DRAW = "draw";
    static const string M_ACCEPT = "accept";
    static const string M_DECLINE = "decline";

    // Why a game ended (keys of the string table).
    static const string END_MATE = "STR_OZG_CHESS_END_MATE";
    static const string END_STALEMATE = "STR_OZG_CHESS_END_STALEMATE";
    static const string END_RESIGN = "STR_OZG_BOARD_END_RESIGN";
    static const string END_AGREED = "STR_OZG_BOARD_END_AGREED";
    static const string END_REPETITION = "STR_OZG_BOARD_END_REPETITION";
    static const string END_FIFTY = "STR_OZG_CHESS_END_FIFTY";
    static const string END_MATERIAL = "STR_OZG_CHESS_END_MATERIAL";
    static const string END_LEFT = "STR_OZG_BOARD_END_LEFT";

    // The refusal when the board lacks a piece the game needs.
    static const string NO_PIECES = "STR_OZG_BOARD_MISSING";

    // What the next game is told: the seat that had white.
    static const string HINT_WHITE = "white";
}

// What a move changed, to take it back.
class OZG_ChessUndo
{
    int Moved;
    int Cap;
    int CapSq;
    int Castle;
    int Ep;
    int Half;
    int Full;
}

// A position and the rules of moving in it; no seats, no players.
class OZG_ChessBoard
{
    int Sq[64];
    // 0: white to move; BLACK: black.
    int Side;
    // 1 white short, 2 white long, 4 black short, 8 black long.
    int Castle;
    // The square a pawn's double step passed over, -1 for none.
    int Ep;
    // Half-moves since the last capture or pawn move; the move number.
    int Half;
    int Full;
    // Where the kings stand.
    int KingW;
    int KingB;

    protected ref array<int> m_KnF;
    protected ref array<int> m_KnR;
    protected ref array<int> m_KgF;
    protected ref array<int> m_KgR;
    protected ref array<ref array<int>> m_PlyMoves;
    protected ref array<ref OZG_ChessUndo> m_PlyUndo;

    void OZG_ChessBoard()
    {
        m_KnF = Ints("1,2,2,1,-1,-2,-2,-1");
        m_KnR = Ints("2,1,-1,-2,-2,-1,1,2");
        m_KgF = Ints("1,1,0,-1,-1,-1,0,1");
        m_KgR = Ints("0,1,1,1,0,-1,-1,-1");
        m_PlyMoves = new array<ref array<int>>();
        m_PlyUndo = new array<ref OZG_ChessUndo>();
        Ep = -1;
        Full = 1;
        KingW = -1;
        KingB = -1;
    }

    static array<int> Ints(string csv)
    {
        array<string> parts = new array<string>();
        csv.Split(",", parts);
        array<int> into = new array<int>();
        for (int i = 0; i < parts.Count(); i++)
            into.Insert(parts[i].ToInt());
        return into;
    }

    // ---- moves as ints ---------------------------------------------------------

    static int Mv(int from, int to, int promo)
    {
        return from | (to << 6) | (promo << 12);
    }

    static int FromOf(int mv)
    {
        return mv & 63;
    }

    static int ToOf(int mv)
    {
        return (mv >> 6) & 63;
    }

    static int PromoOf(int mv)
    {
        return (mv >> 12) & 7;
    }

    // ---- squares and letters ---------------------------------------------------

    static string SqName(int sq)
    {
        int file = sq % 8;
        int rank = sq / 8;
        string f = OZG_ChessConst.FILES.Get(file);
        int shown = rank + 1;
        return f + shown.ToString();
    }

    // -1 when the text is no square.
    static int SqOf(string name)
    {
        if (name.Length() != 2)
            return -1;
        int file = OZG_ChessConst.FILES.IndexOf(name.Get(0));
        int rank = name.Get(1).ToInt() - 1;
        if (file < 0 || rank < 0 || rank > 7)
            return -1;
        return rank * 8 + file;
    }

    static string LetterOf(int piece)
    {
        int kind = piece & 7;
        if (kind == 0 || kind > 6)
            return ".";
        string l = OZG_ChessConst.LETTERS.Get(kind);
        if ((piece & OZG_ChessConst.BLACK) != 0)
            l.ToLower();
        return l;
    }

    static int PieceOf(string letter)
    {
        string up = letter;
        up.ToUpper();
        int kind = OZG_ChessConst.LETTERS.IndexOf(up);
        if (kind <= 0)
            return 0;
        if (up != letter)
            return kind | OZG_ChessConst.BLACK;
        return kind;
    }

    // ---- FEN ---------------------------------------------------------------------

    bool FromFen(string fen)
    {
        array<string> parts = new array<string>();
        fen.Split(" ", parts);
        if (parts.Count() < 2)
            return false;
        int i;
        for (i = 0; i < 64; i++)
            Sq[i] = 0;
        KingW = -1;
        KingB = -1;
        string rows = parts[0];
        int rank = 7;
        int file = 0;
        for (i = 0; i < rows.Length(); i++)
        {
            string c = rows.Get(i);
            if (c == "/")
            {
                rank--;
                file = 0;
                continue;
            }
            int empties = c.ToInt();
            if (empties > 0)
            {
                file += empties;
                continue;
            }
            int piece = PieceOf(c);
            if (piece == 0 || rank < 0 || file > 7)
                return false;
            int at = rank * 8 + file;
            Sq[at] = piece;
            if (piece == OZG_ChessConst.KING)
                KingW = at;
            if (piece == (OZG_ChessConst.KING | OZG_ChessConst.BLACK))
                KingB = at;
            file++;
        }
        Side = 0;
        if (parts[1] == "b")
            Side = OZG_ChessConst.BLACK;
        Castle = 0;
        if (parts.Count() > 2)
        {
            string rights = parts[2];
            if (rights.IndexOf("K") >= 0)
                Castle = Castle | 1;
            if (rights.IndexOf("Q") >= 0)
                Castle = Castle | 2;
            if (rights.IndexOf("k") >= 0)
                Castle = Castle | 4;
            if (rights.IndexOf("q") >= 0)
                Castle = Castle | 8;
        }
        Ep = -1;
        if (parts.Count() > 3 && parts[3] != "-")
            Ep = SqOf(parts[3]);
        Half = 0;
        if (parts.Count() > 4)
            Half = parts[4].ToInt();
        Full = 1;
        if (parts.Count() > 5)
            Full = parts[5].ToInt();
        if (Full < 1)
            Full = 1;
        return KingW >= 0 && KingB >= 0;
    }

    string ToFen()
    {
        string rows = "";
        for (int rank = 7; rank >= 0; rank--)
        {
            int empties = 0;
            for (int file = 0; file < 8; file++)
            {
                int p = Sq[rank * 8 + file];
                if (p == 0)
                {
                    empties++;
                    continue;
                }
                if (empties > 0)
                    rows = rows + empties.ToString();
                empties = 0;
                rows = rows + LetterOf(p);
            }
            if (empties > 0)
                rows = rows + empties.ToString();
            if (rank > 0)
                rows = rows + "/";
        }
        string side = "w";
        if (Side != 0)
            side = "b";
        string rights = "";
        if ((Castle & 1) != 0)
            rights = rights + "K";
        if ((Castle & 2) != 0)
            rights = rights + "Q";
        if ((Castle & 4) != 0)
            rights = rights + "k";
        if ((Castle & 8) != 0)
            rights = rights + "q";
        if (rights == "")
            rights = "-";
        string ep = "-";
        if (Ep >= 0)
            ep = SqName(Ep);
        string fen = rows + " " + side;
        fen = fen + " " + rights;
        fen = fen + " " + ep;
        fen = fen + " " + Half.ToString();
        fen = fen + " " + Full.ToString();
        return fen;
    }

    // The 64 squares as the window gets them.
    string Squares()
    {
        string s = "";
        for (int i = 0; i < 64; i++)
            s = s + LetterOf(Sq[i]);
        return s;
    }

    // What makes two positions the same for repetition: the squares, the
    // side, the castling rights, and the en passant square when a pawn can
    // take there.
    string ChKey()
    {
        string k = Squares();
        k = k + Side.ToString();
        k = k + ":" + Castle.ToString();
        if (Ep >= 0 && EpTakeable())
            k = k + ":" + Ep.ToString();
        return k;
    }

    protected bool EpTakeable()
    {
        int pawn = OZG_ChessConst.PAWN | Side;
        int file = Ep % 8;
        int behind = Ep - 8;
        if (Side != 0)
            behind = Ep + 8;
        if (behind < 0 || behind > 63)
            return false;
        if (file > 0 && Sq[behind - 1] == pawn)
            return true;
        if (file < 7 && Sq[behind + 1] == pawn)
            return true;
        return false;
    }

    // ---- attacks -------------------------------------------------------------------

    // Is the square attacked by the side `by` (0 or BLACK)?
    bool ChAttacked(int sq, int by)
    {
        int file = sq % 8;
        int rank = sq / 8;
        int i;
        int nf;
        int nr;
        int t;
        // Pawns: a pawn of `by` one rank behind, a file aside.
        int pr = rank - 1;
        if (by != 0)
            pr = rank + 1;
        int pawn = OZG_ChessConst.PAWN | by;
        if (pr >= 0 && pr <= 7)
        {
            if (file > 0 && Sq[pr * 8 + file - 1] == pawn)
                return true;
            if (file < 7 && Sq[pr * 8 + file + 1] == pawn)
                return true;
        }
        int knight = OZG_ChessConst.KNIGHT | by;
        int king = OZG_ChessConst.KING | by;
        for (i = 0; i < 8; i++)
        {
            nf = file + m_KnF[i];
            nr = rank + m_KnR[i];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7 && Sq[nr * 8 + nf] == knight)
                return true;
            nf = file + m_KgF[i];
            nr = rank + m_KgR[i];
            if (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7 && Sq[nr * 8 + nf] == king)
                return true;
        }
        int rook = OZG_ChessConst.ROOK | by;
        int bishop = OZG_ChessConst.BISHOP | by;
        int queen = OZG_ChessConst.QUEEN | by;
        // The eight rays: the even ones along ranks and files, the odd ones
        // along diagonals (m_KgF, m_KgR).
        for (i = 0; i < 8; i++)
        {
            int df = m_KgF[i];
            int dr = m_KgR[i];
            bool straight = df == 0 || dr == 0;
            nf = file + df;
            nr = rank + dr;
            while (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7)
            {
                t = Sq[nr * 8 + nf];
                if (t != 0)
                {
                    if (t == queen)
                        return true;
                    if (straight && t == rook)
                        return true;
                    if (!straight && t == bishop)
                        return true;
                    break;
                }
                nf += df;
                nr += dr;
            }
        }
        return false;
    }

    int KingOf(int side)
    {
        if (side == 0)
            return KingW;
        return KingB;
    }

    // Is the side to move in check?
    bool InCheck()
    {
        return ChAttacked(KingOf(Side), OZG_ChessConst.BLACK - Side);
    }

    // ---- moves ---------------------------------------------------------------------

    protected void AddPawn(int from, int to, int lastRank, array<int> into)
    {
        if (to / 8 == lastRank)
        {
            into.Insert(Mv(from, to, OZG_ChessConst.QUEEN));
            into.Insert(Mv(from, to, OZG_ChessConst.ROOK));
            into.Insert(Mv(from, to, OZG_ChessConst.BISHOP));
            into.Insert(Mv(from, to, OZG_ChessConst.KNIGHT));
            return;
        }
        into.Insert(Mv(from, to, 0));
    }

    // Every move of the side to move that the pieces allow, a king left in
    // check or not.
    void ChPseudo(array<int> into)
    {
        int us = Side;
        int them = OZG_ChessConst.BLACK - us;
        int fwd = 8;
        int startRank = 1;
        int lastRank = 7;
        if (us != 0)
        {
            fwd = -8;
            startRank = 6;
            lastRank = 0;
        }
        for (int sq = 0; sq < 64; sq++)
        {
            int p = Sq[sq];
            if (p == 0 || (p & OZG_ChessConst.BLACK) != us)
                continue;
            int kind = p & 7;
            int file = sq % 8;
            int rank = sq / 8;
            int i;
            int nf;
            int nr;
            int t;
            if (kind == OZG_ChessConst.PAWN)
            {
                t = sq + fwd;
                if (t >= 0 && t < 64 && Sq[t] == 0)
                {
                    AddPawn(sq, t, lastRank, into);
                    if (rank == startRank && Sq[t + fwd] == 0)
                        into.Insert(Mv(sq, t + fwd, 0));
                }
                for (i = -1; i <= 1; i += 2)
                {
                    nf = file + i;
                    if (nf < 0 || nf > 7)
                        continue;
                    t = sq + fwd + i;
                    if (t < 0 || t > 63)
                        continue;
                    if (Sq[t] != 0 && (Sq[t] & OZG_ChessConst.BLACK) == them)
                        AddPawn(sq, t, lastRank, into);
                    else if (t == Ep && Sq[t] == 0)
                        into.Insert(Mv(sq, t, 0));
                }
                continue;
            }
            if (kind == OZG_ChessConst.KNIGHT || kind == OZG_ChessConst.KING)
            {
                for (i = 0; i < 8; i++)
                {
                    if (kind == OZG_ChessConst.KNIGHT)
                    {
                        nf = file + m_KnF[i];
                        nr = rank + m_KnR[i];
                    }
                    else
                    {
                        nf = file + m_KgF[i];
                        nr = rank + m_KgR[i];
                    }
                    if (nf < 0 || nf > 7 || nr < 0 || nr > 7)
                        continue;
                    t = nr * 8 + nf;
                    if (Sq[t] == 0 || (Sq[t] & OZG_ChessConst.BLACK) == them)
                        into.Insert(Mv(sq, t, 0));
                }
                if (kind == OZG_ChessConst.KING)
                    AddCastles(sq, into);
                continue;
            }
            // Sliders: the rook along the even rays, the bishop along the odd
            // ones, the queen along all eight.
            for (i = 0; i < 8; i++)
            {
                int df = m_KgF[i];
                int dr = m_KgR[i];
                bool straight = df == 0 || dr == 0;
                if (kind == OZG_ChessConst.ROOK && !straight)
                    continue;
                if (kind == OZG_ChessConst.BISHOP && straight)
                    continue;
                nf = file + df;
                nr = rank + dr;
                while (nf >= 0 && nf <= 7 && nr >= 0 && nr <= 7)
                {
                    t = nr * 8 + nf;
                    if (Sq[t] == 0)
                    {
                        into.Insert(Mv(sq, t, 0));
                    }
                    else
                    {
                        if ((Sq[t] & OZG_ChessConst.BLACK) == them)
                            into.Insert(Mv(sq, t, 0));
                        break;
                    }
                    nf += df;
                    nr += dr;
                }
            }
        }
    }

    // Castling: the right kept, the squares between empty, the rook home, the
    // king not in check and not passing an attacked square (the square it
    // lands on is the legal filter's).
    protected void AddCastles(int sq, array<int> into)
    {
        int them = OZG_ChessConst.BLACK - Side;
        int rook = OZG_ChessConst.ROOK | Side;
        if (Side == 0 && sq == 4)
        {
            if ((Castle & 1) != 0 && Sq[5] == 0 && Sq[6] == 0 && Sq[7] == rook && !ChAttacked(4, them) && !ChAttacked(5, them))
                into.Insert(Mv(4, 6, 0));
            if ((Castle & 2) != 0 && Sq[3] == 0 && Sq[2] == 0 && Sq[1] == 0 && Sq[0] == rook && !ChAttacked(4, them) && !ChAttacked(3, them))
                into.Insert(Mv(4, 2, 0));
        }
        if (Side != 0 && sq == 60)
        {
            if ((Castle & 4) != 0 && Sq[61] == 0 && Sq[62] == 0 && Sq[63] == rook && !ChAttacked(60, them) && !ChAttacked(61, them))
                into.Insert(Mv(60, 62, 0));
            if ((Castle & 8) != 0 && Sq[59] == 0 && Sq[58] == 0 && Sq[57] == 0 && Sq[56] == rook && !ChAttacked(60, them) && !ChAttacked(59, them))
                into.Insert(Mv(60, 58, 0));
        }
    }

    // The rights a move from or onto the square leaves: a king or a rook
    // moving from home, or a rook taken at home, ends them.
    protected static int CastleKeep(int sq)
    {
        if (sq == 0)
            return 13;
        if (sq == 7)
            return 14;
        if (sq == 4)
            return 12;
        if (sq == 56)
            return 7;
        if (sq == 63)
            return 11;
        if (sq == 60)
            return 3;
        return 15;
    }

    void ChDo(int mv, OZG_ChessUndo u)
    {
        int from = FromOf(mv);
        int to = ToOf(mv);
        int promo = PromoOf(mv);
        int p = Sq[from];
        int kind = p & 7;
        int us = p & OZG_ChessConst.BLACK;
        u.Moved = p;
        u.Cap = Sq[to];
        u.CapSq = to;
        u.Castle = Castle;
        u.Ep = Ep;
        u.Half = Half;
        u.Full = Full;
        if (kind == OZG_ChessConst.PAWN && to == Ep && Sq[to] == 0)
        {
            int behind = to - 8;
            if (us != 0)
                behind = to + 8;
            u.Cap = Sq[behind];
            u.CapSq = behind;
            Sq[behind] = 0;
        }
        Sq[to] = p;
        Sq[from] = 0;
        if (promo != 0)
            Sq[to] = promo | us;
        if (kind == OZG_ChessConst.KING)
        {
            if (us == 0)
                KingW = to;
            else
                KingB = to;
            if (to - from == 2)
            {
                Sq[from + 1] = Sq[from + 3];
                Sq[from + 3] = 0;
            }
            else if (from - to == 2)
            {
                Sq[from - 1] = Sq[from - 4];
                Sq[from - 4] = 0;
            }
        }
        Ep = -1;
        if (kind == OZG_ChessConst.PAWN && Math.AbsInt(to - from) == 16)
            Ep = (from + to) / 2;
        Castle = Castle & CastleKeep(from) & CastleKeep(to);
        if (kind == OZG_ChessConst.PAWN || u.Cap != 0)
            Half = 0;
        else
            Half = Half + 1;
        if (us != 0)
            Full = Full + 1;
        Side = OZG_ChessConst.BLACK - Side;
    }

    void ChUndo(int mv, OZG_ChessUndo u)
    {
        Side = OZG_ChessConst.BLACK - Side;
        int from = FromOf(mv);
        int to = ToOf(mv);
        int p = u.Moved;
        int kind = p & 7;
        int us = p & OZG_ChessConst.BLACK;
        Sq[from] = p;
        Sq[to] = 0;
        Sq[u.CapSq] = u.Cap;
        if (kind == OZG_ChessConst.KING)
        {
            if (us == 0)
                KingW = from;
            else
                KingB = from;
            if (to - from == 2)
            {
                Sq[from + 3] = Sq[from + 1];
                Sq[from + 1] = 0;
            }
            else if (from - to == 2)
            {
                Sq[from - 4] = Sq[from - 1];
                Sq[from - 1] = 0;
            }
        }
        Castle = u.Castle;
        Ep = u.Ep;
        Half = u.Half;
        Full = u.Full;
    }

    // The legal moves of the side to move.
    void ChLegal(array<int> into)
    {
        into.Clear();
        array<int> all = new array<int>();
        ChPseudo(all);
        OZG_ChessUndo u = new OZG_ChessUndo();
        int us = Side;
        for (int i = 0; i < all.Count(); i++)
        {
            ChDo(all[i], u);
            if (!ChAttacked(KingOf(us), Side))
                into.Insert(all[i]);
            ChUndo(all[i], u);
        }
    }

    // ---- perft: the count of move paths, against the published numbers ---------

    int ChPerft(int depth)
    {
        return PerftAt(depth, 0);
    }

    protected int PerftAt(int depth, int ply)
    {
        if (depth == 0)
            return 1;
        while (m_PlyMoves.Count() <= ply)
        {
            m_PlyMoves.Insert(new array<int>());
            m_PlyUndo.Insert(new OZG_ChessUndo());
        }
        array<int> moves = m_PlyMoves[ply];
        moves.Clear();
        ChPseudo(moves);
        OZG_ChessUndo u = m_PlyUndo[ply];
        int us = Side;
        int n = 0;
        for (int i = 0; i < moves.Count(); i++)
        {
            int mv = moves[i];
            ChDo(mv, u);
            if (!ChAttacked(KingOf(us), Side))
                n += PerftAt(depth - 1, ply + 1);
            ChUndo(mv, u);
        }
        return n;
    }

    // ---- the end ---------------------------------------------------------------

    // Too little to mate with: kings alone, a king and one minor piece
    // against a king, or bishops of one colour of square alone.
    bool Insufficient()
    {
        int minors = 0;
        int bishopsLight = 0;
        int bishopsDark = 0;
        int knights = 0;
        for (int i = 0; i < 64; i++)
        {
            int kind = Sq[i] & 7;
            if (kind == 0 || kind == OZG_ChessConst.KING)
                continue;
            if (kind == OZG_ChessConst.PAWN || kind == OZG_ChessConst.ROOK || kind == OZG_ChessConst.QUEEN)
                return false;
            if (kind == OZG_ChessConst.KNIGHT)
            {
                knights++;
                continue;
            }
            int shade = ((i % 8) + (i / 8)) % 2;
            if (shade == 0)
                bishopsDark++;
            else
                bishopsLight++;
        }
        minors = knights + bishopsLight + bishopsDark;
        if (minors <= 1)
            return true;
        if (knights == 0 && (bishopsLight == 0 || bishopsDark == 0))
            return true;
        return false;
    }

    // ---- words -------------------------------------------------------------------

    static string Wire(int mv)
    {
        string w = SqName(FromOf(mv)) + SqName(ToOf(mv));
        int promo = PromoOf(mv);
        if (promo != 0)
        {
            string l = OZG_ChessConst.LETTERS.Get(promo);
            l.ToLower();
            w = w + l;
        }
        return w;
    }

    // The legal move written so, or -1.
    int FromWire(string wire)
    {
        string w = wire;
        w.ToLower();
        array<int> legal = new array<int>();
        ChLegal(legal);
        for (int i = 0; i < legal.Count(); i++)
        {
            if (Wire(legal[i]) == w)
                return legal[i];
        }
        return -1;
    }

    // The move in long notation, before it is made: "Ng1-f3", "e5xd6 e.p.",
    // "e7-e8=Q", "O-O"; the check sign is the caller's.
    string LongOf(int mv)
    {
        int from = FromOf(mv);
        int to = ToOf(mv);
        int p = Sq[from];
        int kind = p & 7;
        if (kind == OZG_ChessConst.KING && to - from == 2)
            return "O-O";
        if (kind == OZG_ChessConst.KING && from - to == 2)
            return "O-O-O";
        string s = "";
        if (kind != OZG_ChessConst.PAWN)
            s = OZG_ChessConst.LETTERS.Get(kind);
        bool takes = Sq[to] != 0;
        bool ep = kind == OZG_ChessConst.PAWN && to == Ep && Sq[to] == 0;
        s = s + SqName(from);
        if (takes || ep)
            s = s + "x";
        else
            s = s + "-";
        s = s + SqName(to);
        int promo = PromoOf(mv);
        if (promo != 0)
            s = s + "=" + OZG_ChessConst.LETTERS.Get(promo);
        if (ep)
            s = s + " e.p.";
        return s;
    }
}

// The game: two seats, white and black, around one OZG_ChessBoard.
class OZG_Chess extends OZG_Game
{
    protected ref OZG_ChessBoard m_Pos;
    protected string m_StartFen;
    protected ref array<string> m_Played;
    protected ref array<string> m_Hist;
    protected ref array<string> m_Keys;
    protected string m_Taken;
    protected int m_White = -1;
    protected int m_Black = -1;
    protected int m_Offer = -1;
    protected bool m_Over;
    protected string m_Result;
    protected string m_Reason;
    protected string m_Last;
    // The pieces the board holds, by face, when the session handed them.
    protected ref map<string, int> m_Have;

    void OZG_Chess()
    {
        m_Pos = new OZG_ChessBoard();
        m_Played = new array<string>();
        m_Hist = new array<string>();
        m_Keys = new array<string>();
        m_StartFen = OZG_ChessConst.START;
    }

    override string GameId()
    {
        return OZG_ChessConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_ChessConst.LABEL;
    }

    override string SetKind()
    {
        return OZG_Const.SET_BOARD;
    }

    override int MinPlayers()
    {
        return 2;
    }

    override int MaxPlayers()
    {
        return 2;
    }

    // The faces of the board's pieces the game plays with.
    override void CardsUsed(array<string> into)
    {
        into.Clear();
        array<string> kinds = new array<string>();
        Needs(kinds, null);
        into.Copy(kinds);
    }

    // The 32 pieces: the faces and how many of each.
    static void Needs(array<string> faces, array<int> counts)
    {
        string kinds = "KQRBNP";
        array<int> per = OZG_ChessBoard.Ints("1,1,2,2,2,8");
        for (int c = 0; c < 2; c++)
        {
            string colour = "w";
            if (c == 1)
                colour = "b";
            for (int k = 0; k < 6; k++)
            {
                faces.Insert(colour + kinds.Get(k));
                if (counts)
                    counts.Insert(per[k]);
            }
        }
    }

    override void TakePieces(array<string> faces)
    {
        m_Have = new map<string, int>();
        for (int i = 0; i < faces.Count(); i++)
        {
            int had = 0;
            m_Have.Find(faces[i], had);
            m_Have.Set(faces[i], had + 1);
        }
    }

    // The faces the board lacks, as many times as each is missing.
    void Missing(array<string> into)
    {
        into.Clear();
        if (!m_Have)
            return;
        array<string> faces = new array<string>();
        array<int> counts = new array<int>();
        Needs(faces, counts);
        for (int i = 0; i < faces.Count(); i++)
        {
            int had = 0;
            m_Have.Find(faces[i], had);
            for (int n = had; n < counts[i]; n++)
                into.Insert(faces[i]);
        }
    }

    override OZG_Game Fresh()
    {
        return new OZG_Chess();
    }

    // A game from another position: the stand's checks.
    void StartFrom(string fen)
    {
        m_StartFen = fen;
    }

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        if (seats.Count() != 2)
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        array<string> lack = new array<string>();
        Missing(lack);
        if (lack.Count() > 0)
        {
            refusal = OZG_ChessConst.NO_PIECES;
            return false;
        }
        // White: the seat that had black last game; or the host; or the first.
        int was = HintInt(hint, OZG_ChessConst.HINT_WHITE, -1);
        int host = HintInt(hint, OZG_Const.HINT_HOST, -1);
        m_White = seats[0];
        m_Black = seats[1];
        if (was >= 0 && seats.Find(was) >= 0)
        {
            if (was == seats[0])
            {
                m_White = seats[1];
                m_Black = seats[0];
            }
        }
        else if (host == seats[1])
        {
            m_White = seats[1];
            m_Black = seats[0];
        }
        if (!m_Pos.FromFen(m_StartFen))
        {
            refusal = OZG_Const.NO_GAME;
            return false;
        }
        m_Played.Clear();
        m_Hist.Clear();
        m_Keys.Clear();
        m_Keys.Insert(m_Pos.ChKey());
        m_Taken = "";
        m_Offer = -1;
        m_Over = false;
        m_Result = "";
        m_Reason = "";
        m_Last = "";
        return true;
    }

    // The seat whose move it is; -1 when over.
    int SeatToMove()
    {
        if (m_Over)
            return -1;
        if (m_Pos.Side == 0)
            return m_White;
        return m_Black;
    }

    protected int OtherOf(int seat)
    {
        if (seat == m_White)
            return m_Black;
        return m_White;
    }

    protected void End(string result, string reason)
    {
        m_Over = true;
        m_Result = result;
        m_Reason = reason;
        m_Offer = -1;
    }

    protected string WinOf(int seat)
    {
        if (seat == m_White)
            return "1-0";
        return "0-1";
    }

    override bool Act(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (m_Over || (seat != m_White && seat != m_Black))
            return false;
        int other = OtherOf(seat);
        if (move == OZG_ChessConst.M_RESIGN)
        {
            End(WinOf(other), OZG_ChessConst.END_RESIGN);
            refusal = "";
            return true;
        }
        if (move == OZG_ChessConst.M_DRAW || move == OZG_ChessConst.M_ACCEPT)
        {
            if (m_Offer == other)
            {
                End("1/2", OZG_ChessConst.END_AGREED);
                refusal = "";
                return true;
            }
            if (move == OZG_ChessConst.M_ACCEPT || m_Offer == seat)
                return false;
            m_Offer = seat;
            refusal = "";
            return true;
        }
        if (move == OZG_ChessConst.M_DECLINE)
        {
            if (m_Offer != other)
                return false;
            m_Offer = -1;
            refusal = "";
            return true;
        }
        if (seat != SeatToMove())
            return false;
        int mv = m_Pos.FromWire(move);
        if (mv < 0)
            return false;
        Play(mv);
        // A move answers the other side's offer with no.
        if (m_Offer == other)
            m_Offer = -1;
        refusal = "";
        return true;
    }

    // A legal move made, written down, and the end looked for.
    protected void Play(int mv)
    {
        string text = m_Pos.LongOf(mv);
        int to = OZG_ChessBoard.ToOf(mv);
        int captured = m_Pos.Sq[to];
        int kind = m_Pos.Sq[OZG_ChessBoard.FromOf(mv)] & 7;
        if (kind == OZG_ChessConst.PAWN && to == m_Pos.Ep && captured == 0)
            captured = OZG_ChessConst.PAWN | (OZG_ChessConst.BLACK - m_Pos.Side);
        OZG_ChessUndo u = new OZG_ChessUndo();
        int mover = SeatToMove();
        m_Pos.ChDo(mv, u);
        if (captured != 0)
            m_Taken = m_Taken + OZG_ChessBoard.LetterOf(captured);
        string wire = OZG_ChessBoard.Wire(mv);
        m_Played.Insert(wire);
        m_Last = wire;
        string key = m_Pos.ChKey();
        m_Keys.Insert(key);

        array<int> legal = new array<int>();
        m_Pos.ChLegal(legal);
        bool check = m_Pos.InCheck();
        if (legal.Count() == 0)
        {
            if (check)
            {
                text = text + "#";
                End(WinOf(mover), OZG_ChessConst.END_MATE);
            }
            else
            {
                End("1/2", OZG_ChessConst.END_STALEMATE);
            }
        }
        else
        {
            if (check)
                text = text + "+";
            if (Repeats(key) >= 3)
                End("1/2", OZG_ChessConst.END_REPETITION);
            else if (m_Pos.Half >= 100)
                End("1/2", OZG_ChessConst.END_FIFTY);
            else if (m_Pos.Insufficient())
                End("1/2", OZG_ChessConst.END_MATERIAL);
        }
        m_Hist.Insert(text);
    }

    protected int Repeats(string key)
    {
        int n = 0;
        for (int i = 0; i < m_Keys.Count(); i++)
        {
            if (m_Keys[i] == key)
                n++;
        }
        return n;
    }

    override bool Waits(int seat)
    {
        return !m_Over && seat == SeatToMove();
    }

    override void DropSeat(int seat)
    {
        if (m_Over)
            return;
        if (seat != m_White && seat != m_Black)
            return;
        End(WinOf(OtherOf(seat)), OZG_ChessConst.END_LEFT);
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || (seat != m_White && seat != m_Black))
            return;
        if (seat == SeatToMove())
        {
            array<int> legal = new array<int>();
            m_Pos.ChLegal(legal);
            for (int i = 0; i < legal.Count(); i++)
                into.Insert(OZG_ChessBoard.Wire(legal[i]));
        }
        into.Insert(OZG_ChessConst.M_RESIGN);
        int other = OtherOf(seat);
        if (m_Offer == other)
        {
            into.Insert(OZG_ChessConst.M_ACCEPT);
            into.Insert(OZG_ChessConst.M_DECLINE);
        }
        else if (m_Offer != seat)
        {
            into.Insert(OZG_ChessConst.M_DRAW);
        }
    }

    override string PublicView()
    {
        OZG_BPublic v = new OZG_BPublic();
        v.GameId = OZG_ChessConst.GAME_ID;
        v.Trump = "";
        v.Board = m_Pos.Squares();
        v.Turn = SeatToMove();
        v.White = m_White;
        v.Black = m_Black;
        v.Last = m_Last;
        v.Check = -1;
        bool inCheck = m_Pos.InCheck();
        if (inCheck && (!m_Over || m_Reason == OZG_ChessConst.END_MATE))
            v.Check = m_Pos.KingOf(m_Pos.Side);
        v.History.Copy(m_Hist);
        v.Taken = m_Taken;
        v.Offer = m_Offer;
        v.Final = m_Result;
        v.Reason = m_Reason;
        // The plates: each side's colour, the offer of a draw.
        AddSeat(v, m_White, "STR_OZG_BOARD_WHITE");
        AddSeat(v, m_Black, "STR_OZG_BOARD_BLACK");
        if (m_Over)
        {
            v.Ended = 1;
            v.Result = OZG_Words.W2("STR_OZG_BOARD_RESULT", OZG_Words.KeyArg(m_Reason), m_Result);
            v.Status = v.Result;
        }
        else
        {
            int turn = SeatToMove();
            v.Awaited.Insert(turn);
            string key = "STR_OZG_BOARD_STATUS_MOVE";
            if (inCheck)
                key = "STR_OZG_CHESS_STATUS_CHECK";
            v.Status = OZG_Words.W1(key, OZG_Words.SeatArg(turn));
        }
        return OZG_BPublic.Text(v);
    }

    protected void AddSeat(OZG_BPublic v, int seat, string colourKey)
    {
        if (seat < 0)
            return;
        OZG_GSeat g = new OZG_GSeat();
        g.SeatNo = seat;
        g.Role = OZG_Words.W0(colourKey);
        if (m_Offer == seat)
            g.Line = OZG_Words.W0("STR_OZG_BOARD_OFFERS");
        v.SeatList.Insert(g);
    }

    override string SeatView(int seat)
    {
        OZG_BPrivate v = new OZG_BPrivate();
        array<string> moves = new array<string>();
        Moves(seat, moves);
        int col = 0;
        for (int i = 0; i < moves.Count(); i++)
        {
            string m = moves[i];
            if (m == OZG_ChessConst.M_RESIGN || m == OZG_ChessConst.M_DRAW || m == OZG_ChessConst.M_ACCEPT || m == OZG_ChessConst.M_DECLINE)
            {
                string label = "STR_OZG_BOARD_BTN_" + m;
                label.ToUpper();
                OZG_GButton b = OZG_GButton.Make(m, OZG_Words.W0(label), -1, col);
                if (m == OZG_ChessConst.M_ACCEPT)
                    b.Hot = 1;
                v.Buttons.Insert(b);
                col++;
                continue;
            }
            v.CanPlay.Insert(m);
        }
        if (seat == SeatToMove())
            v.Status = OZG_Words.W0("STR_OZG_BOARD_YOUR_MOVE");
        return OZG_BPrivate.Text(v);
    }

    // The log's words of a move: the seat's name comes first.
    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move == OZG_ChessConst.M_RESIGN || move == OZG_ChessConst.M_DRAW || move == OZG_ChessConst.M_ACCEPT || move == OZG_ChessConst.M_DECLINE)
        {
            string key = "STR_OZG_BOARD_LOG_" + move;
            key.ToUpper();
            into.Insert(key);
            return;
        }
        if (move.Length() < 4)
            return;
        string shown = move.Substring(0, 2) + "-" + move.Substring(2, 2);
        if (move.Length() > 4)
        {
            string promo = move.Substring(4, 1);
            promo.ToUpper();
            shown = shown + "=" + promo;
        }
        into.Insert("STR_OZG_BOARD_LOG_MOVE");
        into.Insert(shown);
    }

    override bool IsOver()
    {
        return m_Over;
    }

    override string Outcome()
    {
        if (m_White < 0)
            return "";
        return OZG_ChessConst.HINT_WHITE + "=" + m_White.ToString();
    }

    // The game as text: the seats and the end, the start position and every
    // move; loading plays the moves again. No field is ever empty: Split
    // drops empty ones (measured 2026-11-26), so "-" stands for nothing.
    override string SaveState()
    {
        string over = "0";
        if (m_Over)
            over = "1";
        string s = "c1;" + m_White.ToString();
        s = s + ";" + m_Black.ToString();
        s = s + ";" + m_Offer.ToString();
        s = s + ";" + over;
        s = s + ";" + Dash(m_Result);
        s = s + ";" + Dash(m_Reason);
        s = s + ";" + m_StartFen;
        string moves = "";
        for (int i = 0; i < m_Played.Count(); i++)
        {
            if (i > 0)
                moves = moves + " ";
            moves = moves + m_Played[i];
        }
        s = s + ";" + Dash(moves);
        return s;
    }

    static string Dash(string s)
    {
        if (s == "")
            return "-";
        return s;
    }

    static string Undash(string s)
    {
        if (s == "-")
            return "";
        return s;
    }

    override bool LoadState(string data)
    {
        array<string> parts = new array<string>();
        data.Split(";", parts);
        if (parts.Count() < 9 || parts[0] != "c1")
            return false;
        OZG_Chess again = new OZG_Chess();
        again.m_StartFen = parts[7];
        if (!again.m_Pos.FromFen(again.m_StartFen))
            return false;
        again.m_White = parts[1].ToInt();
        again.m_Black = parts[2].ToInt();
        again.m_Keys.Insert(again.m_Pos.ChKey());
        again.m_Taken = "";
        array<string> moves = new array<string>();
        Undash(parts[8]).Split(" ", moves);
        for (int i = 0; i < moves.Count(); i++)
        {
            if (moves[i] == "")
                continue;
            if (again.m_Over)
                return false;
            int mv = again.m_Pos.FromWire(moves[i]);
            if (mv < 0)
                return false;
            again.Play(mv);
        }
        m_Pos = again.m_Pos;
        m_StartFen = again.m_StartFen;
        m_Played = again.m_Played;
        m_Hist = again.m_Hist;
        m_Keys = again.m_Keys;
        m_Taken = again.m_Taken;
        m_Last = again.m_Last;
        m_White = again.m_White;
        m_Black = again.m_Black;
        m_Offer = parts[3].ToInt();
        bool over = parts[4] == "1";
        m_Over = over;
        m_Result = Undash(parts[5]);
        m_Reason = Undash(parts[6]);
        return true;
    }

    override bool Sound(out string why)
    {
        why = "";
        int kings = 0;
        for (int i = 0; i < 64; i++)
        {
            if ((m_Pos.Sq[i] & 7) == OZG_ChessConst.KING)
                kings++;
        }
        if (kings != 2)
        {
            why = "kings " + kings.ToString();
            return false;
        }
        return true;
    }

    override string Brief()
    {
        string s = "chess white=" + m_White.ToString();
        s = s + " black=" + m_Black.ToString();
        s = s + " ply=" + m_Played.Count().ToString();
        s = s + " last=" + m_Last;
        if (m_Over)
            s = s + " over=" + m_Result + " " + m_Reason;
        return s;
    }

    override string Everything()
    {
        return Brief() + " fen=" + m_Pos.ToFen();
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (seat != SeatToMove())
            return "";
        array<int> legal = new array<int>();
        m_Pos.ChLegal(legal);
        if (legal.Count() == 0)
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        // A capture or a promotion first, when there is one.
        array<int> sharp = new array<int>();
        for (int i = 0; i < legal.Count(); i++)
        {
            int mv = legal[i];
            if (m_Pos.Sq[OZG_ChessBoard.ToOf(mv)] != 0 || OZG_ChessBoard.PromoOf(mv) != 0)
                sharp.Insert(mv);
        }
        if (sharp.Count() > 0)
            return OZG_ChessBoard.Wire(sharp[dice.Below(sharp.Count())]);
        return OZG_ChessBoard.Wire(legal[dice.Below(legal.Count())]);
    }

    // For the stand: the position as FEN.
    string Fen()
    {
        return m_Pos.ToFen();
    }
}
