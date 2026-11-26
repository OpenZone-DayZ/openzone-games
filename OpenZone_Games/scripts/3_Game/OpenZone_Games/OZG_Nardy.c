// Long and short nardy (boards and dice design §5.5 to §5.7) on the board's
// other side: 24 points, fifteen discs a side, two dice.
//
// Points are numbered 1 to 24 the way White counts them; White moves down to 1
// and bears off from 1..6. In long nardy both sides go the same way round:
// White from its head on 24, Black from its head on 12 down to 1, over to 24
// and down to 13, bearing off from 13..18; a point with a disc of the other
// side is closed, one disc leaves the head a turn (two on a first throw of 6-6,
// 4-4 or 3-3), and no block of six points in a row stands unless a disc of the
// other side is ahead of it. Short nardy is backgammon without the doubling
// cube: the classic setup, Black going up from 1 to 24, a single disc of the
// other side hit to the bar, the bar entered first, single, gammon (mars) and
// backgammon (koks) scored 1, 2 and 3; long nardy scores a mars double.
//
// Every side counts its own path in positions: 24 the far end, 1 the last
// point before bearing off, 25 the bar. Both dice are used where they can be,
// the higher where only one can; doubles are played four times. Who begins:
// each side throws one die, the higher (in short nardy the two numbers are the
// first throw).
//
// Moves on the wire: "throw"; a step "m:<from>:<to>:<die>", from a point or
// "bar", to a point or "off", the points as White counts them.
class OZG_NardyConst
{
    static const string LONG_ID = "nardy_long";
    static const string SHORT_ID = "nardy_short";
    static const string LONG_LABEL = "STR_OZG_GAME_NARDY_LONG";
    static const string SHORT_LABEL = "STR_OZG_GAME_NARDY_SHORT";
    static const string M_THROW = "throw";
    static const string BAR = "bar";
    static const string OFF = "off";
    static const int DISCS = 15;
    static const int BAR_POS = 25;
    static const string END_WIN = "STR_OZG_NARDY_END_WIN";
    static const string END_MARS = "STR_OZG_NARDY_END_MARS";
    static const string END_KOKS = "STR_OZG_NARDY_END_KOKS";
}

class OZG_NardyBoard
{
    bool Long;
    // The 24 points, White's numbers 1..24 at 0..23: White's discs counted up,
    // Black's down.
    int Pts[24];
    int Bar[2];
    int Off[2];
    int Side;

    void SetUp(bool isLong)
    {
        Long = isLong;
        for (int i = 0; i < 24; i++)
            Pts[i] = 0;
        Bar[0] = 0;
        Bar[1] = 0;
        Off[0] = 0;
        Off[1] = 0;
        Side = 0;
        if (Long)
        {
            Pts[23] = OZG_NardyConst.DISCS;
            Pts[11] = -OZG_NardyConst.DISCS;
            return;
        }
        Pts[23] = 2;
        Pts[12] = 5;
        Pts[7] = 3;
        Pts[5] = 5;
        Pts[0] = -2;
        Pts[11] = -5;
        Pts[16] = -3;
        Pts[18] = -5;
    }

    void CopyFrom(OZG_NardyBoard other)
    {
        Long = other.Long;
        for (int i = 0; i < 24; i++)
            Pts[i] = other.Pts[i];
        Bar[0] = other.Bar[0];
        Bar[1] = other.Bar[1];
        Off[0] = other.Off[0];
        Off[1] = other.Off[1];
        Side = other.Side;
    }

    // The point (1..24) of a side's position (1..24).
    int PointOf(int side, int q)
    {
        if (side == 0)
            return q;
        if (Long)
            return ((q - 13) % 24 + 24) % 24 + 1;
        return 25 - q;
    }

    int PosOf(int side, int g)
    {
        if (side == 0)
            return g;
        if (Long)
            return (g + 11) % 24 + 1;
        return 25 - g;
    }

    // The discs of `side` on the point `g`.
    int Mine(int side, int g)
    {
        int c = Pts[g - 1];
        if (side == 0 && c > 0)
            return c;
        if (side == 1 && c < 0)
            return -c;
        return 0;
    }

    void Put(int side, int g, int n)
    {
        if (side == 0)
            Pts[g - 1] = Pts[g - 1] + n;
        else
            Pts[g - 1] = Pts[g - 1] - n;
    }

    // The discs of `side` on its position `q`.
    int AtPos(int side, int q)
    {
        return Mine(side, PointOf(side, q));
    }

    bool AllHome(int side)
    {
        if (Bar[side] > 0)
            return false;
        for (int q = 7; q <= 24; q++)
        {
            if (AtPos(side, q) > 0)
                return false;
        }
        return true;
    }

    // Would `side` hold six points in a row with no disc of the other side
    // ahead of them, the rows counted along the other side's way?
    bool BadBlock(int side)
    {
        int other = 1 - side;
        int lowest = 99;
        int q;
        for (q = 1; q <= 24; q++)
        {
            if (AtPos(other, q) > 0)
            {
                lowest = q;
                break;
            }
        }
        int run = 0;
        for (q = 1; q <= 24; q++)
        {
            if (Mine(side, PointOf(other, q)) > 0)
            {
                run++;
                if (run >= 6 && lowest > q)
                    return true;
            }
            else
            {
                run = 0;
            }
        }
        return false;
    }

    // The steps `side` may take with a die `d`, as positions: from (25 the
    // bar) and to (0 off). `headLeft`: how many more discs may leave the head
    // this turn (long nardy).
    void NaSteps(int side, int d, int headLeft, array<int> froms, array<int> tos)
    {
        froms.Clear();
        tos.Clear();
        int other = 1 - side;
        if (!Long && Bar[side] > 0)
        {
            int enter = OZG_NardyConst.BAR_POS - d;
            if (Mine(other, PointOf(side, enter)) <= 1)
            {
                froms.Insert(OZG_NardyConst.BAR_POS);
                tos.Insert(enter);
            }
            return;
        }
        bool home = AllHome(side);
        for (int qf = 24; qf >= 1; qf--)
        {
            if (AtPos(side, qf) == 0)
                continue;
            if (Long && qf == 24 && headLeft <= 0)
                continue;
            int qt = qf - d;
            if (qt >= 1)
            {
                int g = PointOf(side, qt);
                int theirs = Mine(other, g);
                if (Long && theirs > 0)
                    continue;
                if (!Long && theirs > 1)
                    continue;
                if (Long && NaBlocks(side, qf, qt))
                    continue;
                froms.Insert(qf);
                tos.Insert(qt);
                continue;
            }
            if (!home)
                continue;
            // Off: exactly, or from below the die when nothing stands higher.
            if (qt < 0)
            {
                bool higher = false;
                for (int q = qf + 1; q <= 6; q++)
                {
                    if (AtPos(side, q) > 0)
                        higher = true;
                }
                if (higher)
                    continue;
            }
            froms.Insert(qf);
            tos.Insert(0);
        }
    }

    // Would the step make a block that may not stand?
    protected bool NaBlocks(int side, int qf, int qt)
    {
        OZG_NardyBoard after = new OZG_NardyBoard();
        after.CopyFrom(this);
        after.Put(side, PointOf(side, qf), -1);
        after.Put(side, PointOf(side, qt), 1);
        return after.BadBlock(side);
    }

    // A step made; true when it hit a disc (short nardy).
    bool NaDo(int side, int qf, int qt)
    {
        if (qf == OZG_NardyConst.BAR_POS)
            Bar[side] = Bar[side] - 1;
        else
            Put(side, PointOf(side, qf), -1);
        if (qt <= 0)
        {
            Off[side] = Off[side] + 1;
            return false;
        }
        int g = PointOf(side, qt);
        int other = 1 - side;
        bool hit = false;
        if (!Long && Mine(other, g) == 1)
        {
            Put(other, g, -1);
            Bar[other] = Bar[other] + 1;
            hit = true;
        }
        Put(side, g, 1);
        return hit;
    }

    int Count(int side)
    {
        int n = Bar[side] + Off[side];
        for (int g = 1; g <= 24; g++)
            n = n + Mine(side, g);
        return n;
    }

    // The board as text: the 24 points' counts (White up, Black down), then
    // the bar and the discs off of each side.
    string Text()
    {
        string s = "";
        for (int i = 0; i < 24; i++)
        {
            if (i > 0)
                s = s + ",";
            s = s + Pts[i].ToString();
        }
        s = s + "|" + Bar[0].ToString() + "," + Bar[1].ToString();
        s = s + "|" + Off[0].ToString() + "," + Off[1].ToString();
        return s;
    }

    // A step as it goes on the wire, the points as White counts them.
    string Wire(int side, int qf, int qt, int d)
    {
        string from = OZG_NardyConst.BAR;
        if (qf != OZG_NardyConst.BAR_POS)
            from = PointOf(side, qf).ToString();
        string to = OZG_NardyConst.OFF;
        if (qt > 0)
            to = PointOf(side, qt).ToString();
        return "m:" + from + ":" + to + ":" + d.ToString();
    }
}

class OZG_Nardy extends OZG_BoardDuel
{
    protected ref OZG_NardyBoard m_Pos;
    protected bool m_Long;
    protected string m_Whose = OZG_DiceConst.WHOSE_HOST;
    protected ref OZG_Random m_Chance;
    protected ref OZG_DiceCups m_Cups;
    // The dice left to play this turn, and the throw as it fell.
    protected ref array<int> m_Dice;
    protected ref array<int> m_Roll;
    protected bool m_Thrown;
    // Long nardy: how many discs may still leave the head this turn; whether
    // each side's first turn is still to come.
    protected int m_HeadLeft = 1;
    protected bool m_FirstW = true;
    protected bool m_FirstB = true;

    void OZG_Nardy()
    {
        m_Long = true;
        m_Pos = new OZG_NardyBoard();
        m_Cups = new OZG_DiceCups();
        m_Dice = new array<int>();
        m_Roll = new array<int>();
    }

    override string GameId()
    {
        return OZG_NardyConst.LONG_ID;
    }

    override string LabelKey()
    {
        return OZG_NardyConst.LONG_LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Nardy();
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        into.Insert(OZG_DiceOptions.Whose(false));
    }

    override int DiceNeeded()
    {
        return 2;
    }

    override void TakeDice(int seat, array<int> faces, array<int> weights)
    {
        m_Cups.Set(seat, faces, weights);
    }

    override protected void DuelOptions(map<string, string> options)
    {
        m_Whose = OptionOf(options, OZG_DiceConst.OPT_WHOSE, OZG_DiceConst.WHOSE_HOST);
    }

    override protected void DuelNeeds(array<string> faces, array<int> counts)
    {
        faces.Insert(OZG_DuelConst.WHITE_DISC);
        counts.Insert(OZG_NardyConst.DISCS);
        faces.Insert(OZG_DuelConst.BLACK_DISC);
        counts.Insert(OZG_NardyConst.DISCS);
        if (m_Whose != OZG_DiceConst.WHOSE_OWN)
        {
            faces.Insert(OZG_DiceConst.DIE_FACE);
            counts.Insert(2);
        }
    }

    override protected void DuelStart()
    {
        m_Pos.SetUp(m_Long);
        m_Dice.Clear();
        m_Roll.Clear();
        m_Thrown = false;
        m_HeadLeft = 1;
        m_FirstW = true;
        m_FirstB = true;
    }

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        m_Chance = chance;
        if (!super.Begin(seats, options, chance, hint, refusal))
            return false;
        // Who begins: a die each, thrown again while they are equal.
        int a = 1;
        int b = 1;
        OZG_Random dice = Chance();
        for (int tries = 0; tries < 100 && a == b; tries++)
        {
            a = m_Cups.Of(m_White, 2).Throw(0, dice);
            b = m_Cups.Of(m_Black, 2).Throw(0, dice);
        }
        if (a == b)
            b = a % 6 + 1;
        NaOpen(a, b);
        return true;
    }

    protected OZG_Random Chance()
    {
        if (!m_Chance)
            m_Chance = new OZG_Random();
        return m_Chance;
    }

    // The opening throw: White's die `a`, Black's `b`; the higher begins.
    protected void NaOpen(int a, int b)
    {
        m_Played.Insert("open:" + a.ToString() + ":" + b.ToString());
        m_Hist.Insert("open " + a.ToString() + " " + b.ToString());
        m_Pos.Side = 0;
        if (b > a)
            m_Pos.Side = 1;
        if (m_Long)
            return;
        // Short nardy: the two numbers are the first throw.
        NaThrown(a, b, false);
    }

    protected bool First(int side)
    {
        if (side == 0)
            return m_FirstW;
        return m_FirstB;
    }

    // A throw fallen: the dice to play; with no step at all the turn passes.
    protected void NaThrown(int a, int b, bool record)
    {
        if (record)
        {
            m_Played.Insert("throw:" + a.ToString() + ":" + b.ToString());
            m_Hist.Insert("throw " + a.ToString() + " " + b.ToString());
        }
        m_Roll.Clear();
        m_Roll.Insert(a);
        m_Roll.Insert(b);
        m_Dice.Clear();
        m_Dice.Insert(a);
        m_Dice.Insert(b);
        if (a == b)
        {
            m_Dice.Insert(a);
            m_Dice.Insert(a);
        }
        m_Thrown = true;
        m_HeadLeft = 1;
        bool big = a == b && (a == 6 || a == 4 || a == 3);
        if (m_Long && big && First(m_Pos.Side))
            m_HeadLeft = 2;
        array<string> steps = new array<string>();
        NaLegal(steps);
        if (steps.Count() == 0)
        {
            m_Hist.Insert("pass");
            NaTurnOver();
        }
    }

    protected void NaTurnOver()
    {
        if (m_Pos.Side == 0)
            m_FirstW = false;
        else
            m_FirstB = false;
        m_Pos.Side = 1 - m_Pos.Side;
        m_Thrown = false;
        m_Dice.Clear();
        m_HeadLeft = 1;
    }

    override protected int DuelSide()
    {
        return m_Pos.Side;
    }

    override protected string DuelKey()
    {
        return m_Pos.Text() + m_Pos.Side.ToString();
    }

    // The window draws no 8 x 8 field for nardy: its board is in Line.
    override protected string DuelSquares()
    {
        return "";
    }

    override protected void DuelLegal(array<string> into)
    {
        into.Clear();
        if (!m_Thrown)
        {
            into.Insert(OZG_NardyConst.M_THROW);
            return;
        }
        NaLegal(into);
    }

    // The steps that begin a turn's best use of the dice left.
    protected void NaLegal(array<string> into)
    {
        into.Clear();
        int side = m_Pos.Side;
        int most = NaMost(m_Pos, m_Dice, m_HeadLeft, side);
        if (most == 0)
            return;
        bool two = m_Dice.Count() == 2 && m_Dice[0] != m_Dice[1];
        int high = 0;
        if (two)
        {
            high = m_Dice[0];
            if (m_Dice[1] > high)
                high = m_Dice[1];
        }
        array<string> highOnes = new array<string>();
        array<int> tried = new array<int>();
        for (int i = 0; i < m_Dice.Count(); i++)
        {
            int d = m_Dice[i];
            if (tried.Find(d) >= 0)
                continue;
            tried.Insert(d);
            array<int> froms = new array<int>();
            array<int> tos = new array<int>();
            m_Pos.NaSteps(side, d, m_HeadLeft, froms, tos);
            for (int k = 0; k < froms.Count(); k++)
            {
                int qf = froms[k];
                int qt = tos[k];
                OZG_NardyBoard after = new OZG_NardyBoard();
                after.CopyFrom(m_Pos);
                after.NaDo(side, qf, qt);
                array<int> rest = new array<int>();
                rest.Copy(m_Dice);
                rest.Remove(rest.Find(d));
                int left = m_HeadLeft;
                if (m_Long && qf == 24)
                    left--;
                int reach = 1 + NaMost(after, rest, left, side);
                if (reach < most)
                    continue;
                string wire = m_Pos.Wire(side, qf, qt, d);
                if (into.Find(wire) >= 0)
                    continue;
                into.Insert(wire);
                if (d == high)
                    highOnes.Insert(wire);
            }
        }
        // Where only one die can be used, the higher.
        if (two && most == 1 && highOnes.Count() > 0)
            into.Copy(highOnes);
    }

    // The most dice of `dice` `side` can play from `pos`.
    protected int NaMost(OZG_NardyBoard pos, array<int> dice, int headLeft, int side)
    {
        if (dice.Count() == 0)
            return 0;
        int best = 0;
        array<int> tried = new array<int>();
        for (int i = 0; i < dice.Count(); i++)
        {
            int d = dice[i];
            if (tried.Find(d) >= 0)
                continue;
            tried.Insert(d);
            array<int> froms = new array<int>();
            array<int> tos = new array<int>();
            pos.NaSteps(side, d, headLeft, froms, tos);
            for (int k = 0; k < froms.Count(); k++)
            {
                int qf = froms[k];
                int qt = tos[k];
                OZG_NardyBoard after = new OZG_NardyBoard();
                after.CopyFrom(pos);
                after.NaDo(side, qf, qt);
                array<int> rest = new array<int>();
                rest.Copy(dice);
                rest.Remove(rest.Find(d));
                int left = headLeft;
                if (pos.Long && qf == 24)
                    left--;
                int n = 1 + NaMost(after, rest, left, side);
                if (n > best)
                    best = n;
                if (best == dice.Count())
                    return best;
            }
        }
        return best;
    }

    override protected void DuelApply(string move)
    {
        if (move == OZG_NardyConst.M_THROW)
        {
            int seat = SeatToMove();
            OZG_DiceCup cup = m_Cups.Of(seat, 2);
            int a = cup.Throw(0, Chance());
            int b = cup.Throw(1, Chance());
            NaThrown(a, b, true);
            m_Last = "";
            return;
        }
        NaStep(move);
    }

    override protected bool DuelReplay(string move)
    {
        array<string> parts = new array<string>();
        move.Split(":", parts);
        if (parts.Count() == 3 && parts[0] == "open")
        {
            NaOpen(parts[1].ToInt(), parts[2].ToInt());
            return true;
        }
        if (parts.Count() == 3 && parts[0] == "throw")
        {
            if (m_Thrown)
                return false;
            NaThrown(parts[1].ToInt(), parts[2].ToInt(), true);
            return true;
        }
        return super.DuelReplay(move);
    }

    // A step of the wire made; the turn over when no die is left to play.
    protected void NaStep(string move)
    {
        array<string> parts = new array<string>();
        move.Split(":", parts);
        if (parts.Count() != 4)
            return;
        int side = m_Pos.Side;
        int d = parts[3].ToInt();
        int qf = OZG_NardyConst.BAR_POS;
        if (parts[1] != OZG_NardyConst.BAR)
            qf = m_Pos.PosOf(side, parts[1].ToInt());
        int qt = 0;
        if (parts[2] != OZG_NardyConst.OFF)
            qt = m_Pos.PosOf(side, parts[2].ToInt());
        bool hit = m_Pos.NaDo(side, qf, qt);
        if (m_Long && qf == 24)
            m_HeadLeft--;
        int at = m_Dice.Find(d);
        if (at >= 0)
            m_Dice.Remove(at);
        m_Played.Insert(move);
        string shown = parts[1] + "-" + parts[2];
        if (hit)
            shown = shown + "*";
        m_Hist.Insert(shown);
        m_Last = move;
        if (m_Pos.Off[side] >= OZG_NardyConst.DISCS)
        {
            NaWon(side);
            return;
        }
        array<string> steps = new array<string>();
        NaLegal(steps);
        if (m_Dice.Count() == 0 || steps.Count() == 0)
            NaTurnOver();
    }

    // `side` has borne off its last disc: a mars when the other bore off none,
    // in short nardy a koks when the other still has a disc on the bar or in
    // the winner's home.
    protected void NaWon(int side)
    {
        int other = 1 - side;
        int points = 1;
        string why = OZG_NardyConst.END_WIN;
        if (m_Pos.Off[other] == 0)
        {
            points = 2;
            why = OZG_NardyConst.END_MARS;
            if (!m_Long)
            {
                bool deep = m_Pos.Bar[other] > 0;
                for (int q = 19; q <= 24; q++)
                {
                    if (m_Pos.AtPos(other, q) > 0)
                        deep = true;
                }
                if (deep)
                {
                    points = 3;
                    why = OZG_NardyConst.END_KOKS;
                }
            }
        }
        string score = points.ToString() + "-0";
        if (side == 1)
            score = "0-" + points.ToString();
        End(score, why);
    }

    override protected void DuelTake(OZG_BoardDuel from)
    {
        OZG_Nardy same = OZG_Nardy.Cast(from);
        if (!same)
            return;
        m_Pos = same.m_Pos;
        m_Whose = same.m_Whose;
        m_Dice = same.m_Dice;
        m_Roll = same.m_Roll;
        m_Thrown = same.m_Thrown;
        m_HeadLeft = same.m_HeadLeft;
        m_FirstW = same.m_FirstW;
        m_FirstB = same.m_FirstB;
    }

    override protected bool DuelSound(out string why)
    {
        why = "";
        int white = m_Pos.Count(0);
        int black = m_Pos.Count(1);
        if (white != OZG_NardyConst.DISCS || black != OZG_NardyConst.DISCS)
        {
            why = "discs " + white.ToString() + "/" + black.ToString();
            return false;
        }
        return true;
    }

    // The view: the board as text, the dice; every step a button.
    override protected OZG_BPublic DuelView()
    {
        OZG_BPublic v = super.DuelView();
        v.Points = m_Pos.Text();
        v.Dice = OZG_DiceText.Line(m_Roll);
        v.DiceLeft = OZG_DiceText.Line(m_Dice);
        OZG_GZone zone = new OZG_GZone();
        zone.Id = "dice";
        zone.Label = OZG_Words.W0("STR_OZG_NARDY_DICE");
        zone.Line = OZG_Words.W1("STR_OZG_NARDY_DICE_LINE", OZG_DiceText.Line(m_Dice));
        v.Zones.Insert(zone);
        for (int i = 0; i < v.SeatList.Count(); i++)
        {
            OZG_GSeat g = v.SeatList[i];
            int side = 0;
            if (g.SeatNo == m_Black)
                side = 1;
            g.Line = OZG_Words.W2("STR_OZG_NARDY_SEAT_LINE", m_Pos.Off[side].ToString(), m_Pos.Bar[side].ToString());
        }
        return v;
    }

    override protected OZG_BPrivate DuelSeatView(int seat)
    {
        OZG_BPrivate v = super.DuelSeatView(seat);
        // The steps and the throw as buttons, before resigning and the draw.
        array<ref OZG_GButton> talk = new array<ref OZG_GButton>();
        int i;
        for (i = 0; i < v.Buttons.Count(); i++)
            talk.Insert(v.Buttons[i]);
        v.Buttons.Clear();
        int col = 0;
        for (i = 0; i < v.CanPlay.Count(); i++)
        {
            string m = v.CanPlay[i];
            OZG_GButton b;
            if (m == OZG_NardyConst.M_THROW)
            {
                b = OZG_GButton.Make(m, OZG_Words.W0("STR_OZG_NARDY_BTN_THROW"), -1, col);
                b.Hot = 1;
            }
            else
            {
                b = OZG_GButton.Make(m, NaStepWords(m), 0, col);
            }
            v.Buttons.Insert(b);
            col++;
        }
        for (i = 0; i < talk.Count(); i++)
        {
            talk[i].Col = col;
            v.Buttons.Insert(talk[i]);
            col++;
        }
        v.CanPlay.Clear();
        return v;
    }

    protected string NaStepWords(string move)
    {
        array<string> parts = new array<string>();
        move.Split(":", parts);
        if (parts.Count() != 4)
            return OZG_Words.W0(move);
        string from = parts[1];
        if (from == OZG_NardyConst.BAR)
            from = OZG_Words.KeyArg("STR_OZG_NARDY_BAR");
        string to = parts[2];
        if (to == OZG_NardyConst.OFF)
            to = OZG_Words.KeyArg("STR_OZG_NARDY_OFF");
        return OZG_Words.W3("STR_OZG_NARDY_BTN_STEP", from, to, parts[3]);
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move == OZG_NardyConst.M_THROW)
        {
            into.Insert("STR_OZG_NARDY_LOG_THROW");
            return;
        }
        array<string> parts = new array<string>();
        move.Split(":", parts);
        if (parts.Count() == 4)
        {
            array<string> words = new array<string>();
            OZG_Words.Parts(NaStepWords(move), words);
            into.Insert("STR_OZG_NARDY_LOG_STEP");
            for (int i = 1; i < words.Count(); i++)
                into.Insert(words[i]);
            return;
        }
        super.MoveWords(move, into);
    }

    override string Everything()
    {
        string s = Brief() + " points=" + m_Pos.Text();
        s = s + " side=" + m_Pos.Side.ToString();
        s = s + " dice=" + OZG_DiceText.Line(m_Dice);
        string thrown = "0";
        if (m_Thrown)
            thrown = "1";
        s = s + " thrown=" + thrown;
        return s;
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
        // Bearing off first, then the step that goes furthest.
        for (int i = 0; i < legal.Count(); i++)
        {
            if (legal[i].IndexOf(":off:") >= 0)
                return legal[i];
        }
        return legal[dice.Below(legal.Count())];
    }
}

class OZG_NardyShort extends OZG_Nardy
{
    void OZG_NardyShort()
    {
        m_Long = false;
    }

    override string GameId()
    {
        return OZG_NardyConst.SHORT_ID;
    }

    override string LabelKey()
    {
        return OZG_NardyConst.SHORT_LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_NardyShort();
    }
}
