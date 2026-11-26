// What every two-sided board game shares (boards and dice design §5): White and
// Black on two seats, the moves one by one, resigning, a draw offered and
// accepted, the end with a score and its reason, the game kept as its moves.
// A kind of game (draughts, giveaway, corners) says what its position is, which
// moves it has and when it ends; chess keeps its own older copy of this.
class OZG_DuelConst
{
    static const string M_RESIGN = "resign";
    static const string M_DRAW = "draw";
    static const string M_ACCEPT = "accept";
    static const string M_DECLINE = "decline";
    static const string HINT_WHITE = "white";
    static const string WIN_WHITE = "1-0";
    static const string WIN_BLACK = "0-1";
    static const string DRAWN = "1/2";
    static const string END_RESIGN = "STR_OZG_BOARD_END_RESIGN";
    static const string END_AGREED = "STR_OZG_BOARD_END_AGREED";
    static const string END_REPETITION = "STR_OZG_BOARD_END_REPETITION";
    static const string END_LEFT = "STR_OZG_BOARD_END_LEFT";
    static const string NO_PIECES = "STR_OZG_BOARD_MISSING";
    static const string WHITE_DISC = "wD";
    static const string BLACK_DISC = "bD";
    // A sleight on the board (§9.1): its kinds, and how a game keeps it among
    // its moves ("!<kind>:<a>:<b>", "-" for nothing).
    static const string S_MOVE = "move";
    static const string S_SWAP = "swap";
    static const string S_BACK = "back";
    static const string S_OFF = "off";
    static const string SLEIGHT_MARK = "!";
    static const string NO_SLEIGHT = "STR_OZG_BSLEIGHT_NO";
    static const string BAD_SLEIGHT = "STR_OZG_BSLEIGHT_BAD";

    static string SleightText(string kind, string a, string b)
    {
        string s = SLEIGHT_MARK + kind;
        s = s + ":" + OZG_BoardDuel.Dash(a);
        s = s + ":" + OZG_BoardDuel.Dash(b);
        return s;
    }

    // A kept sleight's parts; false for a text that is not one.
    static bool SleightOf(string text, out string kind, out string a, out string b)
    {
        kind = "";
        a = "";
        b = "";
        if (text.IndexOf(SLEIGHT_MARK) != 0)
            return false;
        array<string> parts = new array<string>();
        text.Substring(1, text.Length() - 1).Split(":", parts);
        if (parts.Count() != 3)
            return false;
        kind = parts[0];
        a = OZG_BoardDuel.Undash(parts[1]);
        b = OZG_BoardDuel.Undash(parts[2]);
        return true;
    }
}

class OZG_BoardDuel extends OZG_Game
{
    protected int m_White = -1;
    protected int m_Black = -1;
    protected int m_Offer = -1;
    protected bool m_Over;
    protected string m_Result;
    protected string m_Reason;
    protected string m_Last;
    protected string m_Taken;
    // The moves as they went on the wire (what a saved game replays), and as
    // the move list shows them.
    protected ref array<string> m_Played;
    protected ref array<string> m_Hist;
    // The positions so far, for a draw by repetition.
    protected ref array<string> m_Keys;
    // The options the game began with, as `id=value` joined with `,`.
    protected string m_Opts;
    // The pieces the board holds, by face, when the session handed them.
    protected ref map<string, int> m_Have;
    // A position to begin from instead of the setup (the stand's checks), as
    // the kind of game writes it.
    protected string m_From;

    void OZG_BoardDuel()
    {
        m_Played = new array<string>();
        m_Hist = new array<string>();
        m_Keys = new array<string>();
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

    // ---- what a kind of game gives ----------------------------------------------

    // The faces it plays with and how many of each, for the options read.
    protected void DuelNeeds(array<string> faces, array<int> counts)
    {
    }

    // The options read (before the pieces are counted).
    protected void DuelOptions(map<string, string> options)
    {
    }

    // The start position.
    protected void DuelStart()
    {
    }

    // 0 when White is to move, 1 when Black.
    protected int DuelSide()
    {
        return 0;
    }

    // The legal moves of the side to move, as they go on the wire.
    protected void DuelLegal(array<string> into)
    {
    }

    // A legal move made: the position changed, m_Taken grown, the move list
    // written, the end looked for.
    protected void DuelApply(string move)
    {
    }

    // The position and the side to move, for a repetition.
    protected string DuelKey()
    {
        return "";
    }

    // The 64 squares from a1 to h8, a letter or '.' each: what the window draws.
    protected string DuelSquares()
    {
        return "";
    }

    // How many crowned pieces stand on the board (draughts' kings): a crown is heard
    // when there are more after a move.
    protected int DuelKings()
    {
        return 0;
    }

    // What the table hears of a move (boards and dice design §15): the piece set down,
    // or a piece taken; a piece crowned. The end is heard after them (End).
    protected void DuelHeard(string move, int takenBefore, int kingsBefore)
    {
        if (m_Taken.Length() > takenBefore)
            HearSound(OZG_SoundIds.CAPTURE);
        else
            HearSound(OZG_SoundIds.PIECE);
        if (DuelKings() > kingsBefore)
            HearSound(OZG_SoundIds.CROWN);
    }

    // Does the position add up?
    protected bool DuelSound(out string why)
    {
        why = "";
        return true;
    }

    // A game from another position: the stand's checks. Kept with the options,
    // so a saved game replays from it.
    void StartFrom(string text)
    {
        m_From = text;
    }

    // ---- the pieces ----------------------------------------------------------------

    override void CardsUsed(array<string> into)
    {
        into.Clear();
        array<string> faces = new array<string>();
        array<int> counts = new array<int>();
        DuelNeeds(faces, counts);
        into.Copy(faces);
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
        DuelNeeds(faces, counts);
        for (int i = 0; i < faces.Count(); i++)
        {
            int had = 0;
            m_Have.Find(faces[i], had);
            for (int n = had; n < counts[i]; n++)
                into.Insert(faces[i]);
        }
    }

    // ---- a game ----------------------------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        if (seats.Count() != 2)
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        m_Opts = OptsText(options);
        if (m_From != "")
        {
            if (m_Opts != "")
                m_Opts = m_Opts + ",";
            m_Opts = m_Opts + "from=" + m_From;
        }
        DuelOptions(options);
        array<string> lack = new array<string>();
        Missing(lack);
        if (lack.Count() > 0)
        {
            refusal = OZG_DuelConst.NO_PIECES;
            return false;
        }
        // White: the seat that had Black last game; or the host; or the first.
        int was = HintInt(hint, OZG_DuelConst.HINT_WHITE, -1);
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
        DuelFresh();
        return true;
    }

    // The game from its start, the seats and options kept.
    protected void DuelFresh()
    {
        DuelStart();
        m_Played.Clear();
        m_Hist.Clear();
        m_Keys.Clear();
        m_Keys.Insert(DuelKey());
        m_Taken = "";
        m_Offer = -1;
        m_Over = false;
        m_Result = "";
        m_Reason = "";
        m_Last = "";
    }

    static string OptsText(map<string, string> options)
    {
        string s = "";
        if (!options)
            return s;
        for (int i = 0; i < options.Count(); i++)
        {
            if (s != "")
                s = s + ",";
            s = s + options.GetKey(i) + "=" + options.GetElement(i);
        }
        return s;
    }

    static map<string, string> OptsOf(string text)
    {
        map<string, string> options = new map<string, string>();
        array<string> pairs = new array<string>();
        text.Split(",", pairs);
        for (int i = 0; i < pairs.Count(); i++)
        {
            int eq = pairs[i].IndexOf("=");
            if (eq <= 0)
                continue;
            string id = pairs[i].Substring(0, eq);
            string value = pairs[i].Substring(eq + 1, pairs[i].Length() - eq - 1);
            options.Set(id, value);
        }
        return options;
    }

    // The seat whose move it is; -1 when over.
    int SeatToMove()
    {
        if (m_Over)
            return -1;
        if (DuelSide() == 0)
            return m_White;
        return m_Black;
    }

    protected int OtherOf(int seat)
    {
        if (seat == m_White)
            return m_Black;
        return m_White;
    }

    protected int SeatOfSide(int side)
    {
        if (side == 0)
            return m_White;
        return m_Black;
    }

    protected void End(string result, string reason)
    {
        m_Over = true;
        m_Result = result;
        m_Reason = reason;
        m_Offer = -1;
        HearSound(OZG_SoundIds.OVER);
    }

    protected string WinOfSide(int side)
    {
        if (side == 0)
            return OZG_DuelConst.WIN_WHITE;
        return OZG_DuelConst.WIN_BLACK;
    }

    protected string WinOf(int seat)
    {
        if (seat == m_White)
            return OZG_DuelConst.WIN_WHITE;
        return OZG_DuelConst.WIN_BLACK;
    }

    protected bool IsTalk(string move)
    {
        if (move == OZG_DuelConst.M_RESIGN || move == OZG_DuelConst.M_DRAW)
            return true;
        return move == OZG_DuelConst.M_ACCEPT || move == OZG_DuelConst.M_DECLINE;
    }

    override bool Act(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (m_Over || (seat != m_White && seat != m_Black))
            return false;
        int other = OtherOf(seat);
        if (move == OZG_DuelConst.M_RESIGN)
        {
            End(WinOf(other), OZG_DuelConst.END_RESIGN);
            refusal = "";
            return true;
        }
        if (move == OZG_DuelConst.M_DRAW || move == OZG_DuelConst.M_ACCEPT)
        {
            if (m_Offer == other)
            {
                End(OZG_DuelConst.DRAWN, OZG_DuelConst.END_AGREED);
                refusal = "";
                return true;
            }
            if (move == OZG_DuelConst.M_ACCEPT || m_Offer == seat)
                return false;
            m_Offer = seat;
            refusal = "";
            return true;
        }
        if (move == OZG_DuelConst.M_DECLINE)
        {
            if (m_Offer != other)
                return false;
            m_Offer = -1;
            refusal = "";
            return true;
        }
        if (seat != SeatToMove())
            return false;
        array<string> legal = new array<string>();
        DuelLegal(legal);
        if (legal.Find(move) < 0)
            return false;
        int takenBefore = m_Taken.Length();
        int kingsBefore = DuelKings();
        DuelApply(move);
        DuelHeard(move, takenBefore, kingsBefore);
        // A move answers the other side's offer with no.
        if (m_Offer == other)
            m_Offer = -1;
        refusal = "";
        return true;
    }

    // A position written down after a move; true when it has now come three
    // times.
    protected bool KeepKey()
    {
        string key = DuelKey();
        m_Keys.Insert(key);
        int n = 0;
        for (int i = 0; i < m_Keys.Count(); i++)
        {
            if (m_Keys[i] == key)
                n++;
        }
        return n >= 3;
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
        End(WinOf(OtherOf(seat)), OZG_DuelConst.END_LEFT);
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || (seat != m_White && seat != m_Black))
            return;
        if (seat == SeatToMove())
            DuelLegal(into);
        into.Insert(OZG_DuelConst.M_RESIGN);
        int other = OtherOf(seat);
        if (m_Offer == other)
        {
            into.Insert(OZG_DuelConst.M_ACCEPT);
            into.Insert(OZG_DuelConst.M_DECLINE);
        }
        else if (m_Offer != seat)
        {
            into.Insert(OZG_DuelConst.M_DRAW);
        }
    }

    override string PublicView()
    {
        return OZG_BPublic.Text(DuelView());
    }

    // The public half as an object, for a kind of game to add to.
    protected OZG_BPublic DuelView()
    {
        OZG_BPublic v = new OZG_BPublic();
        v.GameId = GameId();
        v.Trump = "";
        v.Board = DuelSquares();
        v.Turn = SeatToMove();
        v.White = m_White;
        v.Black = m_Black;
        v.Last = m_Last;
        v.Check = -1;
        v.History.Copy(m_Hist);
        v.Taken = m_Taken;
        v.Offer = m_Offer;
        v.Final = m_Result;
        v.Reason = m_Reason;
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
            v.Status = OZG_Words.W1("STR_OZG_BOARD_STATUS_MOVE", OZG_Words.SeatArg(turn));
        }
        return v;
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
        return OZG_BPrivate.Text(DuelSeatView(seat));
    }

    protected OZG_BPrivate DuelSeatView(int seat)
    {
        OZG_BPrivate v = new OZG_BPrivate();
        array<string> moves = new array<string>();
        Moves(seat, moves);
        int col = 0;
        for (int i = 0; i < moves.Count(); i++)
        {
            string m = moves[i];
            if (IsTalk(m))
            {
                string label = "STR_OZG_BOARD_BTN_" + m;
                label.ToUpper();
                OZG_GButton b = OZG_GButton.Make(m, OZG_Words.W0(label), -1, col);
                if (m == OZG_DuelConst.M_ACCEPT)
                    b.Hot = 1;
                v.Buttons.Insert(b);
                col++;
                continue;
            }
            v.CanPlay.Insert(m);
        }
        if (seat == SeatToMove())
            v.Status = OZG_Words.W0("STR_OZG_BOARD_YOUR_MOVE");
        return v;
    }

    // The log's words of a move: the seat's name comes first.
    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (IsTalk(move))
        {
            string key = "STR_OZG_BOARD_LOG_" + move;
            key.ToUpper();
            into.Insert(key);
            return;
        }
        into.Insert("STR_OZG_BOARD_LOG_MOVE");
        into.Insert(move);
    }

    override bool IsOver()
    {
        return m_Over;
    }

    override string Outcome()
    {
        if (m_White < 0)
            return "";
        return OZG_DuelConst.HINT_WHITE + "=" + m_White.ToString();
    }

    // The game as text: the seats, the end, the options and every move;
    // loading plays the moves again. No field is ever empty: Split drops empty
    // ones, so "-" stands for nothing.
    override string SaveState()
    {
        string over = "0";
        if (m_Over)
            over = "1";
        string s = "d1;" + m_White.ToString();
        s = s + ";" + m_Black.ToString();
        s = s + ";" + m_Offer.ToString();
        s = s + ";" + over;
        s = s + ";" + Dash(m_Result);
        s = s + ";" + Dash(m_Reason);
        s = s + ";" + Dash(m_Opts);
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
        OZG_Text.SplitLong(data, ";", parts);
        if (parts.Count() < 9 || parts[0] != "d1")
            return false;
        OZG_BoardDuel again = OZG_BoardDuel.Cast(Fresh());
        if (!again)
            return false;
        again.m_White = parts[1].ToInt();
        again.m_Black = parts[2].ToInt();
        string opts = Undash(parts[7]);
        again.m_Opts = opts;
        map<string, string> chosen = OptsOf(opts);
        string from;
        if (chosen.Find("from", from))
            again.StartFrom(from);
        again.DuelOptions(chosen);
        again.DuelFresh();
        // The moves can pass 8191 bytes: read in place.
        array<string> moves = new array<string>();
        OZG_Text.FieldPieces(data, ";", 8, " ", moves);
        for (int i = 0; i < moves.Count(); i++)
        {
            if (moves[i] == "")
                continue;
            if (again.m_Over)
                return false;
            if (!again.DuelReplay(moves[i]))
                return false;
        }
        DuelTake(again);
        m_White = again.m_White;
        m_Black = again.m_Black;
        m_Opts = again.m_Opts;
        m_Played = again.m_Played;
        m_Hist = again.m_Hist;
        m_Keys = again.m_Keys;
        m_Taken = again.m_Taken;
        m_Last = again.m_Last;
        m_Offer = parts[3].ToInt();
        bool over = parts[4] == "1";
        m_Over = over;
        m_Result = Undash(parts[5]);
        m_Reason = Undash(parts[6]);
        return true;
    }

    // ---- sleights on the board (§9.1) ---------------------------------------

    override bool BoardSleight(int seat, string kind, string a, string b, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (m_Over || (seat != m_White && seat != m_Black))
            return false;
        if (!DuelSleight(kind, a, b, refusal))
            return false;
        // Kept among the moves, so a saved game has it; the move list shows
        // nothing of it.
        m_Played.Insert(OZG_DuelConst.SleightText(kind, a, b));
        DuelAfterSleight();
        refusal = "";
        return true;
    }

    // A kind of game's sleight on its position: checked against what it
    // forbids, the position as it was when refused.
    protected bool DuelSleight(string kind, string a, string b, out string refusal)
    {
        refusal = OZG_DuelConst.NO_SLEIGHT;
        return false;
    }

    // The end looked for after a sleight: a side left with no piece or no move.
    protected void DuelAfterSleight()
    {
    }

    // One letter less of the pieces taken off; false when none is there.
    protected bool TakeBack(string letter)
    {
        int at = m_Taken.IndexOf(letter);
        if (letter.Length() != 1 || at < 0)
            return false;
        string head = m_Taken.Substring(0, at);
        string tail = m_Taken.Substring(at + 1, m_Taken.Length() - at - 1);
        m_Taken = head + tail;
        return true;
    }

    // A saved move made again on load; false when it does not fit.
    protected bool DuelReplay(string move)
    {
        string kind;
        string a;
        string b;
        if (OZG_DuelConst.SleightOf(move, kind, a, b))
        {
            string refusal;
            if (!DuelSleight(kind, a, b, refusal))
                return false;
            m_Played.Insert(move);
            DuelAfterSleight();
            return true;
        }
        array<string> legal = new array<string>();
        DuelLegal(legal);
        if (legal.Find(move) < 0)
            return false;
        DuelApply(move);
        return true;
    }

    // A loaded game's position and counters taken over from `from`, a game of
    // the same kind.
    protected void DuelTake(OZG_BoardDuel from)
    {
    }

    override bool Sound(out string why)
    {
        return DuelSound(why);
    }

    override string Brief()
    {
        string s = GameId() + " white=" + m_White.ToString();
        s = s + " black=" + m_Black.ToString();
        s = s + " ply=" + m_Played.Count().ToString();
        s = s + " last=" + m_Last;
        if (m_Over)
            s = s + " over=" + m_Result + " " + m_Reason;
        return s;
    }

    override string Everything()
    {
        return Brief() + " board=" + DuelSquares();
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
        return legal[dice.Below(legal.Count())];
    }
}
