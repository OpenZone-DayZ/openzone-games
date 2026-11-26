// The board games' table in the window (boards and dice design §7): the field
// the way the seat sees it, the pieces standing on it, a click that lights a
// piece's moves and a second that plays one, the last move and a check marked,
// the pawn's promotion asked, the pieces taken beside the field. The plates,
// the buttons and the log are the later card games' table, which a board
// game's view also is (OZG_BPublic extends OZG_GPublic); the lines of the
// table's middle stand right of the field.
modded class OZG_GameMenu
{
    protected static const string LAYOUT_BPIC = "OpenZone_Games/gui/layouts/ozg_bpic.layout";
    protected static const string LAYOUT_BLIGHT = "OpenZone_Games/gui/layouts/ozg_blight.layout";
    // The field as White reads it and as Black does (models/src/make_textures.py).
    protected static const string BOARD_WHITE = "OpenZone_Games/gui/boards/ozg_board_win_white_co.paa";
    protected static const string BOARD_BLACK = "OpenZone_Games/gui/boards/ozg_board_win_black_co.paa";
    protected static const string PIECES = "set:ozg_pieces_classic image:";
    // The board's picture in units of the window, and the field inside it: the
    // picture is 320 mm of board, the field 280 of them, 20 the frame.
    protected static const float BOARD_SIDE = 420.0;
    protected static const float BOARD_MIDDLE_X = 602.0;
    protected static const float BOARD_MIDDLE_Y = 500.0;
    // The column right of the field the table's lines stand in.
    protected static const float BOARD_TEXT_X = 824.0;
    protected static const float BOARD_TEXT_W = 334.0;
    protected static const float BOARD_TEXT_Y = 410.0;
    // A piece stands with its base this part of its square down; the base lies
    // this part of its picture above the picture's bottom (models/src/sprites.py).
    protected static const float PIECE_FOOT = 0.64;
    protected static const float PIECE_BASE = 0.055;

    protected ref OZG_BPublic m_BPub;
    protected ref OZG_BPrivate m_BOwn;
    protected ref array<Widget> m_BoardDrawn;
    // The board's own lines: why a game ended, the promotion's question.
    protected ref array<Widget> m_BTexts;
    // The table's lines and where the look put them (four numbers each, in
    // units), while a board game has them aside.
    protected ref array<Widget> m_BLines;
    protected ref array<float> m_BLinesAt;
    protected bool m_BLinesAside;
    // The squares picked so far of the move being made: the piece, then
    // where it lands, one square a click.
    protected ref array<int> m_PickPath;
    protected int m_PromoFrom = -1;
    protected int m_PromoTo = -1;
    // The field: its top-left corner and a square's side, in units; turned for
    // black.
    protected float m_BX;
    protected float m_BY;
    protected float m_BS;
    protected bool m_BFlip;

    protected bool BoardGame()
    {
        if (!m_V || m_V.GameId == "")
            return false;
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (!sample)
            return false;
        return sample.SetKind() == OZG_Const.SET_BOARD;
    }

    protected void BoardRead()
    {
        m_BPub = null;
        m_BOwn = null;
        if (!BoardGame() || m_V.St == OZG_Const.ST_LOBBY)
            return;
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
        string err;
        if (view.Shared != "")
            JsonFileLoader<OZG_BPublic>.LoadData(view.Shared, m_BPub, err);
        if (view.Own != "")
            JsonFileLoader<OZG_BPrivate>.LoadData(view.Own, m_BOwn, err);
    }

    override void GenPaintTable()
    {
        super.GenPaintTable();
        BoardPaint();
    }

    override void GenHideTable()
    {
        super.GenHideTable();
        BoardForget();
        BoardLinesHome();
        if (m_BTexts)
            HideAll(m_BTexts);
    }

    // The look is built anew: its widgets went with the old window, so they
    // are forgotten, not unlinked.
    override void GenForget()
    {
        super.GenForget();
        if (m_BoardDrawn)
            m_BoardDrawn.Clear();
        if (m_BTexts)
            m_BTexts.Clear();
        if (m_BLines)
        {
            m_BLines.Clear();
            m_BLinesAt.Clear();
        }
        m_BLinesAside = false;
        PickClear();
    }

    override void ClickTable()
    {
        if (BoardGame())
        {
            float x;
            float y;
            PointerAt(x, y);
            BoardClick(x, y);
            return;
        }
        super.ClickTable();
    }

    protected void BoardForget()
    {
        if (!m_BoardDrawn)
            m_BoardDrawn = new array<Widget>();
        for (int i = 0; i < m_BoardDrawn.Count(); i++)
        {
            if (m_BoardDrawn[i])
                m_BoardDrawn[i].Unlink();
        }
        m_BoardDrawn.Clear();
    }

    // ---- the table's lines -----------------------------------------------------

    // The result, the status, the hint and what was said, where the look put
    // them: read before a board game first moves them.
    protected void BoardLinesKeep()
    {
        if (!m_BLines)
        {
            m_BLines = new array<Widget>();
            m_BLinesAt = new array<float>();
        }
        if (m_BLines.Count() > 0)
            return;
        m_BLines.Insert(m_Result);
        m_BLines.Insert(m_Status);
        m_BLines.Insert(m_Hint);
        m_BLines.Insert(m_Said);
        for (int i = 0; i < m_BLines.Count(); i++)
        {
            float x = 0;
            float y = 0;
            float w = 0;
            float h = 0;
            if (m_BLines[i])
            {
                m_BLines[i].GetPos(x, y);
                m_BLines[i].GetSize(w, h);
            }
            m_BLinesAt.Insert(x / m_K);
            m_BLinesAt.Insert(y / m_K);
            m_BLinesAt.Insert(w / m_K);
            m_BLinesAt.Insert(h / m_K);
        }
    }

    protected void BoardLinesHome()
    {
        if (!m_BLinesAside || !m_BLines)
            return;
        for (int i = 0; i < m_BLines.Count(); i++)
        {
            int at = i * 4;
            BoardSet(m_BLines[i], m_BLinesAt[at], m_BLinesAt[at + 1], m_BLinesAt[at + 2], m_BLinesAt[at + 3]);
        }
        m_BLinesAside = false;
    }

    protected void BoardLinesAside()
    {
        BoardLinesKeep();
        float x = BOARD_TEXT_X;
        float w = BOARD_TEXT_W;
        float y = BOARD_TEXT_Y;
        BoardSet(m_Result, x, y, w, 56);
        BoardSet(m_Status, x, y + 88, w, 30);
        BoardSet(m_Hint, x, y + 120, w, 22);
        BoardSet(m_Said, x, y + 146, w, 22);
        m_BLinesAside = true;
    }

    // A widget moved and sized in units; whether it shows is left as it was.
    protected void BoardSet(Widget w, float x, float y, float width, float height)
    {
        if (!w)
            return;
        w.SetPos(x * m_K, y * m_K);
        w.SetSize(width * m_K, height * m_K);
    }

    // ---- drawing ---------------------------------------------------------------

    protected ImageWidget BoardPic(string picture, float x, float y, float w, float h, int prio)
    {
        ImageWidget pic = ImageWidget.Cast(MakeLayout(LAYOUT_BPIC, m_TableBox, "OZG_BPic"));
        if (!pic)
            return null;
        pic.LoadImageFile(0, picture);
        pic.SetImage(0);
        pic.SetSort(prio);
        PutAt(pic, x, y, w, h);
        m_BoardDrawn.Insert(pic);
        return pic;
    }

    protected void BoardLight(int sq, int colour, float inset)
    {
        Widget light = MakeLayout(LAYOUT_BLIGHT, m_TableBox, "OZG_BLight");
        if (!light)
            return;
        float x;
        float y;
        SquareCorner(sq, x, y);
        light.SetColor(colour);
        light.SetSort(2);
        PutAt(light, x + inset, y + inset, m_BS - 2 * inset, m_BS - 2 * inset);
        m_BoardDrawn.Insert(light);
    }

    // The square drawn in a row (from the top) and a column (from the left),
    // the way this seat sees the field.
    protected int SquareOfCell(int row, int col)
    {
        if (m_BFlip)
            return row * 8 + (7 - col);
        return (7 - row) * 8 + col;
    }

    // The top-left corner of a square, in units, the way this seat sees it.
    protected void SquareCorner(int sq, out float x, out float y)
    {
        int file = sq % 8;
        int rank = sq / 8;
        if (m_BFlip)
        {
            x = m_BX + (7 - file) * m_BS;
            y = m_BY + rank * m_BS;
            return;
        }
        x = m_BX + file * m_BS;
        y = m_BY + (7 - rank) * m_BS;
    }

    // The square under (px, py), -1 for none.
    protected int SquareAt(float px, float py)
    {
        if (m_BS <= 0)
            return -1;
        float fx = (px - m_BX) / m_BS;
        float fy = (py - m_BY) / m_BS;
        if (fx < 0 || fy < 0 || fx >= 8 || fy >= 8)
            return -1;
        int col = fx;
        int row = fy;
        return SquareOfCell(row, col);
    }

    // Draughts and nardy draw checkers; chess does not.
    protected bool BoardDiscs()
    {
        return m_V && m_V.GameId != OZG_ChessConst.GAME_ID;
    }

    // The picture of a piece: chess writes White's in capitals and Black's in
    // small letters; the checkers are W and B.
    protected string PieceImage(string letter)
    {
        if (BoardDiscs())
        {
            if (letter == "W" || letter == "w")
                return PIECES + "wD";
            return PIECES + "bD";
        }
        string up = letter;
        up.ToUpper();
        if (up == letter)
            return PIECES + "w" + up;
        return PIECES + "b" + up;
    }

    // How big a piece's picture is drawn, in boxes. The pictures keep the
    // pieces' sizes, a king twice a pawn, which leaves the small ones too small
    // on a square; these bring them closer.
    protected float PieceScale(string up)
    {
        if (up == "P")
            return 1.6;
        if (up == "R")
            return 1.55;
        if (up == "N")
            return 1.4;
        if (up == "B")
            return 1.38;
        return 1.3;
    }

    // A piece's picture standing in a box `side` units wide at (x, y): its base
    // `foot` of the box down, its size the kind's; a checker lies in the box,
    // a king of draughts as two, one on the other.
    protected void PieceAt(string letter, float x, float y, float side, float foot, int prio)
    {
        if (BoardDiscs())
        {
            BoardPic(PieceImage(letter), x, y, side, side, prio);
            if (letter == "W" || letter == "B")
                BoardPic(PieceImage(letter), x, y - side * 0.16, side, side, prio);
            return;
        }
        string up = letter;
        up.ToUpper();
        float s = side * PieceScale(up);
        float baseY = y + side * foot;
        BoardPic(PieceImage(letter), x + (side - s) * 0.5, baseY + s * PIECE_BASE - s, s, s, prio);
    }

    // Is the letter a white piece's? Chess writes White in capitals; the
    // checkers are w and W.
    protected bool WhiteLetter(string letter)
    {
        if (BoardDiscs())
            return letter == "w" || letter == "W";
        string up = letter;
        up.ToUpper();
        return up == letter;
    }

    protected void BoardPaint()
    {
        BoardForget();
        if (!m_BTexts)
            m_BTexts = new array<Widget>();
        HideAll(m_BTexts);
        if (!BoardGame() || m_V.St == OZG_Const.ST_LOBBY)
        {
            PickClear();
            return;
        }
        BoardRead();
        // Nardy draws no 8 x 8 field (its table waits for the mockups): the
        // generic lines and buttons are its table for now.
        if (!m_BPub || m_BPub.Board.Length() != 64)
            return;
        BoardLinesAside();
        float frame = BOARD_SIDE * 20.0 / 320.0;
        float left = BOARD_MIDDLE_X - BOARD_SIDE * 0.5;
        float top = BOARD_MIDDLE_Y - BOARD_SIDE * 0.5;
        m_BX = left + frame;
        m_BY = top + frame;
        m_BS = (BOARD_SIDE - 2 * frame) / 8.0;
        m_BFlip = false;
        if (m_BPub && m_V.Mine >= 0 && m_V.Mine == m_BPub.Black)
            m_BFlip = true;
        string picture = BOARD_WHITE;
        if (m_BFlip)
            picture = BOARD_BLACK;
        BoardPic(picture, left, top, BOARD_SIDE, BOARD_SIDE, 1);
        if (!m_BPub || m_BPub.Board.Length() != 64)
            return;
        BoardPaintEnd();
        BoardPaintLights();
        BoardPaintPieces();
        BoardPaintTaken(left, top);
        if (m_PromoTo >= 0)
            BoardPaintPromotion();
    }

    // The end of a game: the score where the result stands, why under it.
    protected void BoardPaintEnd()
    {
        if (m_BPub.Ended != 1 || m_BPub.Final == "")
            return;
        if (m_Result)
        {
            m_Result.SetText(Caps(m_BPub.Final));
            m_Result.Show(true);
        }
        TextWidget why = GenText(m_BTexts, 0);
        if (!why || m_BPub.Reason == "")
            return;
        why.SetText(Caps(Tr(m_BPub.Reason)));
        why.SetSort(25);
        PutAt(why, BOARD_TEXT_X, BOARD_TEXT_Y + 58, BOARD_TEXT_W, 24);
    }

    // The last move, a check, the squares picked and where the move may go on:
    // a square it would take on lit whole, an empty one marked in its middle;
    // a square the move may end on and go on from, marked brighter.
    protected void BoardPaintLights()
    {
        array<int> last = new array<int>();
        string tail;
        BoardPath(m_BPub.Last, last, tail);
        int i;
        for (i = 0; i < last.Count(); i++)
        {
            int alpha = 80;
            if (i == last.Count() - 1)
                alpha = 110;
            if (last[i] >= 0)
                BoardLight(last[i], ARGB(alpha, 255, 214, 80), 0);
        }
        if (m_BPub.Check >= 0)
            BoardLight(m_BPub.Check, ARGB(150, 220, 40, 30), 0);
        // A sleight just seen: its squares in red a while.
        if (m_SawSquares && GetGame().GetTime() < m_SawUntil)
        {
            for (int sw = 0; sw < m_SawSquares.Count(); sw++)
                BoardLight(m_SawSquares[sw], ARGB(150, 230, 60, 60), 0);
        }
        array<int> next = new array<int>();
        PickNext(next);
        if (Picked().Count() > 0 && next.Count() == 0 && PickExact().Count() == 0)
            PickClear();
        array<int> picked = Picked();
        for (i = 0; i < picked.Count(); i++)
            BoardLight(picked[i], ARGB(120, 70, 190, 70), 0);
        if (picked.Count() >= 2 && PickExact().Count() > 0)
            BoardLight(picked[picked.Count() - 1], ARGB(200, 120, 230, 120), m_BS * 0.2);
        for (i = 0; i < next.Count(); i++)
        {
            if (m_BPub.Board.Get(next[i]) != ".")
                BoardLight(next[i], ARGB(100, 70, 190, 70), 0);
            else
                BoardLight(next[i], ARGB(140, 70, 190, 70), m_BS * 0.34);
        }
    }

    // The pieces, the far rows first: a piece stands over the square behind it.
    protected void BoardPaintPieces()
    {
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                int sq = SquareOfCell(row, col);
                string letter = m_BPub.Board.Get(sq);
                if (letter == ".")
                    continue;
                float x;
                float y;
                SquareCorner(sq, x, y);
                PieceAt(letter, x, y, m_BS, PIECE_FOOT, 10 + row);
            }
        }
    }

    // The pieces taken, left of the field: what the side at the top took goes
    // down from the top, what the side at the bottom took up from the bottom.
    protected void BoardPaintTaken(float left, float top)
    {
        float small = m_BS * 0.7;
        float x = left - small - 4;
        float step = small * 0.55;
        int byTop = 0;
        int byBottom = 0;
        for (int k = 0; k < m_BPub.Taken.Length(); k++)
        {
            string taken = m_BPub.Taken.Get(k);
            bool white = WhiteLetter(taken);
            // Black sits at the top of a field not turned, and takes White's.
            bool tookByTop = white != m_BFlip;
            if (tookByTop)
            {
                PieceAt(taken, x, top + 8 + byTop * step, small, 0.9, 10 + byTop);
                byTop++;
            }
            else
            {
                PieceAt(taken, x, top + BOARD_SIDE - 8 - small - byBottom * step, small, 0.9, 30 - byBottom);
                byBottom++;
            }
        }
    }

    // The row the promotion's four pieces stand in: by the square the pawn
    // goes to, inside the field.
    protected void PromoRow(out float x, out float y)
    {
        float sx;
        float sy;
        SquareCorner(m_PromoTo, sx, sy);
        x = sx - m_BS * 1.5;
        if (x < m_BX)
            x = m_BX;
        if (x > m_BX + 4 * m_BS)
            x = m_BX + 4 * m_BS;
        y = sy + m_BS * 1.15;
        if (y + m_BS > m_BY + 8 * m_BS)
            y = sy - m_BS * 1.15;
    }

    // The promotion asked: four pieces in a row, the question under them.
    protected void BoardPaintPromotion()
    {
        array<int> pair = new array<int>();
        pair.Insert(m_PromoFrom);
        pair.Insert(m_PromoTo);
        array<string> between = new array<string>();
        MovesOfPath(pair, between);
        if (between.Count() < 2)
        {
            m_PromoFrom = -1;
            m_PromoTo = -1;
            return;
        }
        float rowX;
        float rowY;
        PromoRow(rowX, rowY);
        string kinds = "QRBN";
        bool white = m_V.Mine == m_BPub.White;
        for (int i = 0; i < 4; i++)
        {
            string letter = kinds.Get(i);
            if (!white)
                letter.ToLower();
            float px = rowX + i * m_BS;
            Widget back = MakeLayout(LAYOUT_BLIGHT, m_TableBox, "OZG_BLight");
            if (back)
            {
                back.SetColor(ARGB(235, 30, 36, 30));
                back.SetSort(40);
                PutAt(back, px, rowY, m_BS, m_BS);
                m_BoardDrawn.Insert(back);
            }
            PieceAt(letter, px, rowY, m_BS, 0.86, 41);
        }
        TextWidget ask = GenText(m_BTexts, 1);
        if (!ask)
            return;
        ask.SetText(Caps(Tr("STR_OZG_CHESS_PROMOTE")));
        ask.SetSort(25);
        PutAt(ask, rowX - m_BS, rowY + m_BS + 2, m_BS * 6, 22);
    }

    // ---- a sleight seen ------------------------------------------------------------

    protected ref array<int> m_SawSquares;
    protected int m_SawUntil;

    // A sleight on the board seen (§9.1): said, and its squares lit a while.
    void BoardSaw(OZG_WBoardSaw saw)
    {
        if (!m_SawSquares)
            m_SawSquares = new array<int>();
        m_SawSquares.Clear();
        int a = OZG_ChessBoard.SqOf(saw.A);
        int b = OZG_ChessBoard.SqOf(saw.B);
        if (a >= 0)
            m_SawSquares.Insert(a);
        if (b >= 0)
            m_SawSquares.Insert(b);
        m_SawUntil = GetGame().GetTime() + 8000;
        string key = "STR_OZG_BSAW_" + saw.Kind;
        key.ToUpper();
        string who = SeatName(saw.SeatNo);
        string line = Tr(key);
        line.Replace("%1", who);
        line.Replace("%2", saw.A);
        line.Replace("%3", saw.B);
        Said(line);
        BoardPaint();
    }

    // Words with their arguments in this client's language, for what the
    // server says outside a view (a notice).
    string WordsOf(string words)
    {
        return Words(words);
    }

    // ---- moves -----------------------------------------------------------------

    protected array<int> Picked()
    {
        if (!m_PickPath)
            m_PickPath = new array<int>();
        return m_PickPath;
    }

    protected void PickClear()
    {
        Picked().Clear();
        m_PromoFrom = -1;
        m_PromoTo = -1;
    }

    // The squares of a move as it goes on the wire: "e2e4" and "e7e8q" (the
    // letter left over in `tail`), "c3-d4", "c3:e5:c7".
    protected void BoardPath(string move, array<int> into, out string tail)
    {
        into.Clear();
        tail = "";
        if (move.IndexOf("-") >= 0 || move.IndexOf(":") >= 0)
        {
            OZG_DraughtsBoard.PathOf(move, into);
            return;
        }
        if (move.Length() < 4)
            return;
        into.Insert(OZG_ChessBoard.SqOf(move.Substring(0, 2)));
        into.Insert(OZG_ChessBoard.SqOf(move.Substring(2, 2)));
        if (move.Length() > 4)
            tail = move.Substring(4, move.Length() - 4);
    }

    // Does the path of `move` begin with `prefix`? Its length in `count`.
    protected bool PathBegins(string move, array<int> prefix, out int count)
    {
        array<int> path = new array<int>();
        string tail;
        BoardPath(move, path, tail);
        count = path.Count();
        if (path.Count() < prefix.Count() || prefix.Count() == 0)
            return false;
        for (int i = 0; i < prefix.Count(); i++)
        {
            if (path[i] != prefix[i])
                return false;
        }
        return true;
    }

    // This seat's moves whose path is exactly `path` (more than one for a
    // pawn that becomes a piece).
    protected void MovesOfPath(array<int> path, array<string> into)
    {
        into.Clear();
        if (!m_BOwn || !m_BOwn.CanPlay)
            return;
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (PathBegins(m_BOwn.CanPlay[i], path, count) && count == path.Count())
                into.Insert(m_BOwn.CanPlay[i]);
        }
    }

    protected array<string> PickExact()
    {
        array<string> exact = new array<string>();
        if (Picked().Count() >= 2)
            MovesOfPath(Picked(), exact);
        return exact;
    }

    // The squares the move being picked may go to next.
    protected void PickNext(array<int> into)
    {
        into.Clear();
        if (!m_BOwn || !m_BOwn.CanPlay || Picked().Count() == 0)
            return;
        int at = Picked().Count();
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (!PathBegins(m_BOwn.CanPlay[i], Picked(), count) || count <= at)
                continue;
            array<int> path = new array<int>();
            string tail;
            BoardPath(m_BOwn.CanPlay[i], path, tail);
            if (into.Find(path[at]) < 0)
                into.Insert(path[at]);
        }
    }

    // Does any move of this seat begin on `sq`?
    protected bool MovesFrom(int sq)
    {
        if (!m_BOwn || !m_BOwn.CanPlay)
            return false;
        array<int> start = new array<int>();
        start.Insert(sq);
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (PathBegins(m_BOwn.CanPlay[i], start, count) && count >= 2)
                return true;
        }
        return false;
    }

    // A click on the field: the move is picked a square a click, played when
    // only one move is left that ends there; a click on the last square picked
    // again ends a move that could go on (a chain of jumps in corners).
    protected void BoardClick(float px, float py)
    {
        if (!m_BPub)
            return;
        if (m_PromoTo >= 0)
        {
            string chosen = PromotionAt(px, py);
            int promoFrom = m_PromoFrom;
            int promoTo = m_PromoTo;
            PickClear();
            if (chosen != "")
            {
                Play(OZG_ChessBoard.SqName(promoFrom) + OZG_ChessBoard.SqName(promoTo) + chosen);
                return;
            }
            BoardPaint();
            return;
        }
        int sq = SquareAt(px, py);
        if (sq < 0)
        {
            PickClear();
            BoardPaint();
            return;
        }
        array<int> path = Picked();
        int picked = path.Count();
        if (picked >= 2 && sq == path[picked - 1])
        {
            array<string> here = PickExact();
            if (here.Count() == 1)
            {
                PickClear();
                Play(here[0]);
                return;
            }
        }
        if (picked > 0)
        {
            array<int> next = new array<int>();
            PickNext(next);
            if (next.Find(sq) >= 0)
            {
                Picked().Insert(sq);
                array<int> further = new array<int>();
                PickNext(further);
                array<string> exact = PickExact();
                if (further.Count() == 0 && exact.Count() == 1)
                {
                    PickClear();
                    Play(exact[0]);
                    return;
                }
                if (further.Count() == 0 && exact.Count() > 1)
                {
                    m_PromoFrom = path[0];
                    m_PromoTo = sq;
                }
                BoardPaint();
                return;
            }
        }
        Picked().Clear();
        if (MovesFrom(sq))
            Picked().Insert(sq);
        BoardPaint();
    }

    // The letter of the piece clicked among the four of a promotion, "" for
    // none.
    protected string PromotionAt(float px, float py)
    {
        float x;
        float y;
        PromoRow(x, y);
        if (py < y || py > y + m_BS || px < x || px >= x + 4 * m_BS)
            return "";
        int i = (px - x) / m_BS;
        if (i < 0 || i > 3)
            return "";
        string kinds = "qrbn";
        return kinds.Get(i);
    }
}
