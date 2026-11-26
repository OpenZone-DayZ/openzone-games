// Thousand (boards and dice design §6.1): five dice; 1 scores 10, 5 scores 5,
// three of a kind its face x 10 (three ones 100), four of a kind double three,
// five of a kind ten times three (five ones: 1000, the game), 1-2-3-4-5 125 and
// 2-3-4-5-6 250 (all five of one throw). Scoring dice are put aside and the rest
// thrown again, or the turn's points written down; a throw that scores nothing
// loses the turn's points (a bolt); five dice put aside are all thrown again.
// The first to 1000 wins. The host's house rules, all on by default: entry (the
// first points need 50 in one turn), pits (a total may not stay in 200-300 or
// 600-700), the barrel (from 880 a turn counts only when it reaches 1000; three
// that do not fall back 120), overtaking (the one overtaken loses 50), three
// bolts in a row lose 100.
//
// Moves: "throw"; "set:<positions>" (1-based, of the dice of the last throw);
// "write".
class OZG_ThousandConst
{
    static const string GAME_ID = "thousand";
    static const string LABEL = "STR_OZG_GAME_THOUSAND";
    static const int GOAL = 1000;
    static const string M_SET = "set:";
    static const string M_WRITE = "write";
    static const string OPT_ENTRY = "entry";
    static const string OPT_PITS = "pits";
    static const string OPT_BARREL = "barrel";
    static const string OPT_OVERTAKE = "overtake";
    static const string OPT_BOLTS = "bolts";
    static const int ENTRY = 50;
    static const int BARREL = 880;
    static const int BARREL_TRIES = 3;
    static const int BARREL_FALL = 120;
    static const int OVERTAKE_LOSS = 50;
    static const int BOLTS = 3;
    static const int BOLTS_LOSS = 100;
}

class OZG_Thousand extends OZG_DiceGame
{
    protected ref array<int> m_Total;
    protected ref array<int> m_Bolts;
    protected ref array<int> m_BarrelTries;
    protected ref array<int> m_Entered;
    protected int m_TurnPoints;
    // Dice to throw next (5 at a turn's start), the last throw, and the phase:
    // 0 throw, 1 put aside, 2 throw the rest or write.
    protected int m_Left = 5;
    protected ref array<int> m_Throw;
    protected ref array<int> m_Aside;
    protected int m_Phase;
    // Which of the seat's five dice (their places among them) are thrown next,
    // and which fell in the last throw: the dice left on the table are the ones
    // thrown again, so a loaded die weighs on its own throws only.
    protected ref array<int> m_Slots;
    protected ref array<int> m_ThrowSlots;
    protected int m_Entry = 50;
    protected bool m_Pits = true;
    protected bool m_Barrel = true;
    protected bool m_Overtake = true;
    protected bool m_BoltRule = true;

    void OZG_Thousand()
    {
        m_Total = new array<int>();
        m_Bolts = new array<int>();
        m_BarrelTries = new array<int>();
        m_Entered = new array<int>();
        m_Throw = new array<int>();
        m_Aside = new array<int>();
        m_Slots = new array<int>();
        m_ThrowSlots = new array<int>();
    }

    override string GameId()
    {
        return OZG_ThousandConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_ThousandConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Thousand();
    }

    override protected void DgOptions(array<ref OZG_Option> into)
    {
        OZG_Option entry = new OZG_Option();
        entry.OptionId = OZG_ThousandConst.OPT_ENTRY;
        entry.LabelKey = "STR_OZG_THOUSAND_ENTRY";
        entry.ValueIds.Insert("50");
        entry.ValueKeys.Insert("STR_OZG_THOUSAND_ENTRY_50");
        entry.ValueIds.Insert("0");
        entry.ValueKeys.Insert("STR_OZG_DICE_RULE_OFF");
        entry.DefaultId = "50";
        into.Insert(entry);
        into.Insert(Rule(OZG_ThousandConst.OPT_PITS, "STR_OZG_THOUSAND_PITS"));
        into.Insert(Rule(OZG_ThousandConst.OPT_BARREL, "STR_OZG_THOUSAND_BARREL"));
        into.Insert(Rule(OZG_ThousandConst.OPT_OVERTAKE, "STR_OZG_THOUSAND_OVERTAKE"));
        into.Insert(Rule(OZG_ThousandConst.OPT_BOLTS, "STR_OZG_THOUSAND_BOLTS"));
    }

    protected OZG_Option Rule(string id, string labelKey)
    {
        OZG_Option o = new OZG_Option();
        o.OptionId = id;
        o.LabelKey = labelKey;
        o.ValueIds.Insert("on");
        o.ValueKeys.Insert("STR_OZG_DICE_RULE_ON");
        o.ValueIds.Insert("off");
        o.ValueKeys.Insert("STR_OZG_DICE_RULE_OFF");
        o.DefaultId = "on";
        return o;
    }

    override protected void DgRead(map<string, string> options)
    {
        m_Entry = OptionOf(options, OZG_ThousandConst.OPT_ENTRY, "50").ToInt();
        // A comparison goes into a local first, never straight into a field.
        bool pits = OptionOf(options, OZG_ThousandConst.OPT_PITS, "on") == "on";
        bool barrel = OptionOf(options, OZG_ThousandConst.OPT_BARREL, "on") == "on";
        bool overtake = OptionOf(options, OZG_ThousandConst.OPT_OVERTAKE, "on") == "on";
        bool bolts = OptionOf(options, OZG_ThousandConst.OPT_BOLTS, "on") == "on";
        m_Pits = pits;
        m_Barrel = barrel;
        m_Overtake = overtake;
        m_BoltRule = bolts;
    }

    override protected bool DgBegin(out string refusal)
    {
        refusal = "";
        m_Total.Clear();
        m_Bolts.Clear();
        m_BarrelTries.Clear();
        m_Entered.Clear();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            m_Total.Insert(0);
            m_Bolts.Insert(0);
            m_BarrelTries.Insert(0);
            m_Entered.Insert(0);
        }
        ThNewTurn();
        return true;
    }

    protected void ThNewTurn()
    {
        m_TurnPoints = 0;
        m_Left = 5;
        m_Throw.Clear();
        m_Aside.Clear();
        m_Phase = 0;
        ThAllSlots();
        m_ThrowSlots.Clear();
    }

    protected void ThAllSlots()
    {
        m_Slots.Clear();
        for (int i = 0; i < 5; i++)
            m_Slots.Insert(i);
    }

    override protected void DgPassTurn()
    {
        ThNewTurn();
        m_Turn = DgNextIndex(m_Turn);
    }

    // The points of dice put aside together; -1 when one of them scores
    // nothing.
    static int ThPoints(array<int> dice)
    {
        int n = dice.Count();
        if (n == 0)
            return -1;
        int c[7];
        int f;
        for (f = 0; f < 7; f++)
            c[f] = 0;
        for (int i = 0; i < n; i++)
            c[dice[i]] = c[dice[i]] + 1;
        if (n == 5)
        {
            bool low = true;
            bool high = true;
            for (f = 1; f <= 5; f++)
            {
                if (c[f] != 1)
                    low = false;
                if (c[f + 1] != 1)
                    high = false;
            }
            if (low)
                return 125;
            if (high)
                return 250;
        }
        int points = 0;
        for (f = 1; f <= 6; f++)
        {
            int k = c[f];
            if (k == 0)
                continue;
            int three = f * 10;
            if (f == 1)
                three = 100;
            if (k == 3)
            {
                points = points + three;
                continue;
            }
            if (k == 4)
            {
                points = points + 2 * three;
                continue;
            }
            if (k == 5)
            {
                points = points + 10 * three;
                continue;
            }
            if (f == 1)
            {
                points = points + 10 * k;
                continue;
            }
            if (f == 5)
            {
                points = points + 5 * k;
                continue;
            }
            return -1;
        }
        return points;
    }

    // Every way to put aside scoring dice of the last throw, one a set of
    // values: "set:<positions>".
    protected void ThSets(array<string> into)
    {
        into.Clear();
        int n = m_Throw.Count();
        array<string> seen = new array<string>();
        int masks = 1 << n;
        for (int mask = 1; mask < masks; mask++)
        {
            array<int> values = new array<int>();
            string positions = "";
            for (int b = 0; b < n; b++)
            {
                if ((mask & (1 << b)) == 0)
                    continue;
                values.Insert(m_Throw[b]);
                if (positions != "")
                    positions = positions + ",";
                positions = positions + (b + 1).ToString();
            }
            if (ThPoints(values) <= 0)
                continue;
            values.Sort();
            string key = Ints(values, "");
            if (seen.Find(key) >= 0)
                continue;
            seen.Insert(key);
            into.Insert(OZG_ThousandConst.M_SET + positions);
        }
    }

    // The values a "set:" move puts aside; false when it names a die twice or
    // none of the throw.
    protected bool ThValues(string move, array<int> into)
    {
        into.Clear();
        string list = move.Substring(OZG_ThousandConst.M_SET.Length(), move.Length() - OZG_ThousandConst.M_SET.Length());
        array<int> positions = new array<int>();
        IntsOf(list, ",", positions);
        array<int> used = new array<int>();
        for (int i = 0; i < positions.Count(); i++)
        {
            int p = positions[i] - 1;
            if (p < 0 || p >= m_Throw.Count() || used.Find(p) >= 0)
                return false;
            used.Insert(p);
            into.Insert(m_Throw[p]);
        }
        return into.Count() > 0;
    }

    // The places in the last throw (from 0) a "set:" move names.
    protected void ThPositions(string move, array<int> into)
    {
        into.Clear();
        string list = move.Substring(OZG_ThousandConst.M_SET.Length(), move.Length() - OZG_ThousandConst.M_SET.Length());
        array<int> positions = new array<int>();
        IntsOf(list, ",", positions);
        for (int i = 0; i < positions.Count(); i++)
            into.Insert(positions[i] - 1);
    }

    protected bool OnBarrel(int i)
    {
        return m_Barrel && m_Total[i] >= OZG_ThousandConst.BARREL;
    }

    protected bool MayWrite()
    {
        if (m_Phase != 2)
            return false;
        if (m_Entry > 0 && m_Entered[m_Turn] == 0 && m_TurnPoints < m_Entry)
            return false;
        return true;
    }

    override protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (seat != DgCurrent())
            return false;
        int i = m_Turn;
        if (move == OZG_DiceGameConst.M_THROW && (m_Phase == 0 || m_Phase == 2))
        {
            if (m_Slots.Count() == 0)
                ThAllSlots();
            DgRollSlots(seat, m_Slots, m_Throw);
            m_ThrowSlots.Copy(m_Slots);
            array<string> sets = new array<string>();
            ThSets(sets);
            if (sets.Count() > 0)
            {
                m_Phase = 1;
                return true;
            }
            ThBolt(i);
            return true;
        }
        if (move.IndexOf(OZG_ThousandConst.M_SET) == 0 && m_Phase == 1)
        {
            array<int> values = new array<int>();
            if (!ThValues(move, values))
                return false;
            int points = ThPoints(values);
            if (points <= 0)
                return false;
            // Five ones: the game.
            if (points == 1000 && values.Count() == 5)
            {
                m_Total[i] = m_Total[i] + 1000;
                DgWonBy(i, "STR_OZG_THOUSAND_END_ONES");
                return true;
            }
            m_TurnPoints = m_TurnPoints + points;
            for (int k = 0; k < values.Count(); k++)
                m_Aside.Insert(values[k]);
            // The dice left on the table are thrown next; all five put aside,
            // all five again.
            array<int> taken = new array<int>();
            ThPositions(move, taken);
            array<int> rest = new array<int>();
            for (int r = 0; r < m_ThrowSlots.Count(); r++)
            {
                if (taken.Find(r) < 0)
                    rest.Insert(m_ThrowSlots[r]);
            }
            m_Slots.Copy(rest);
            if (m_Slots.Count() == 0)
                ThAllSlots();
            m_Left = m_Slots.Count();
            m_Throw.Clear();
            m_ThrowSlots.Clear();
            m_Phase = 2;
            return true;
        }
        if (move == OZG_ThousandConst.M_WRITE && MayWrite())
        {
            ThWrite(i);
            return true;
        }
        return false;
    }

    // A throw that scored nothing: the turn's points lost.
    protected void ThBolt(int i)
    {
        m_Hist.Insert(m_Seats[i].ToString() + " bolt");
        if (m_BoltRule)
        {
            m_Bolts[i] = m_Bolts[i] + 1;
            if (m_Bolts[i] >= OZG_ThousandConst.BOLTS)
            {
                m_Bolts[i] = 0;
                m_Total[i] = m_Total[i] - OZG_ThousandConst.BOLTS_LOSS;
            }
        }
        if (OnBarrel(i))
            ThBarrelMiss(i);
        DgPassTurn();
    }

    protected void ThBarrelMiss(int i)
    {
        m_BarrelTries[i] = m_BarrelTries[i] + 1;
        if (m_BarrelTries[i] < OZG_ThousandConst.BARREL_TRIES)
            return;
        m_BarrelTries[i] = 0;
        m_Total[i] = m_Total[i] - OZG_ThousandConst.BARREL_FALL;
    }

    protected void ThWrite(int i)
    {
        int was = m_Total[i];
        int now = was + m_TurnPoints;
        m_Bolts[i] = 0;
        if (OnBarrel(i))
        {
            if (now >= OZG_ThousandConst.GOAL)
            {
                m_Total[i] = now;
                DgWonBy(i, "STR_OZG_THOUSAND_END");
                return;
            }
            ThBarrelMiss(i);
            DgPassTurn();
            return;
        }
        if (now >= OZG_ThousandConst.GOAL)
        {
            m_Total[i] = now;
            DgWonBy(i, "STR_OZG_THOUSAND_END");
            return;
        }
        bool pit = (now >= 200 && now <= 300) || (now >= 600 && now <= 700);
        if (m_Pits && pit)
        {
            m_Hist.Insert(m_Seats[i].ToString() + " pit");
            DgPassTurn();
            return;
        }
        if (m_Overtake)
        {
            for (int j = 0; j < m_Seats.Count(); j++)
            {
                if (j == i || m_Gone.Find(j) >= 0)
                    continue;
                if (m_Total[j] > was && m_Total[j] < now)
                    m_Total[j] = m_Total[j] - OZG_ThousandConst.OVERTAKE_LOSS;
            }
        }
        m_Total[i] = now;
        m_Entered[i] = 1;
        m_BarrelTries[i] = 0;
        DgPassTurn();
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || seat != DgCurrent())
            return;
        if (m_Phase == 0)
        {
            into.Insert(OZG_DiceGameConst.M_THROW);
            return;
        }
        if (m_Phase == 1)
        {
            ThSets(into);
            return;
        }
        into.Insert(OZG_DiceGameConst.M_THROW);
        if (MayWrite())
            into.Insert(OZG_ThousandConst.M_WRITE);
    }

    override string PublicView()
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = GameId();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            string key = "STR_OZG_THOUSAND_SEAT";
            if (OnBarrel(i))
                key = "STR_OZG_THOUSAND_SEAT_BARREL";
            string line = OZG_Words.W2(key, m_Total[i].ToString(), m_Bolts[i].ToString());
            OZG_GSeat g = DgPlate(m_Seats[i], "", line);
            if (m_Gone.Find(i) >= 0)
                g.Out = 1;
            v.SeatList.Insert(g);
        }
        v.Zones.Insert(DgZone("throw", OZG_Words.W0("STR_OZG_THOUSAND_THROW"), m_Throw));
        // the canvas: the dice put aside drawn as dice, the turn's points in a box of
        // their own
        v.Zones.Insert(DgZone("aside", OZG_Words.W0("STR_OZG_THOUSAND_ASIDE"), m_Aside));
        OZG_GZone turn = new OZG_GZone();
        turn.Id = "turn";
        turn.Label = OZG_Words.W0("STR_OZG_THOUSAND_TURN");
        turn.Line = OZG_Words.W1("STR_OZG_DWIN_NUMBER", m_TurnPoints.ToString());
        v.Zones.Insert(turn);
        if (m_Over)
        {
            v.Ended = 1;
            v.Result = m_End;
            v.Status = m_End;
        }
        else
        {
            v.Awaited.Insert(DgCurrent());
            v.Status = OZG_Words.W1("STR_OZG_DICE_TURN", OZG_Words.SeatArg(DgCurrent()));
        }
        return DgPublicText(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int i = 0; i < can.Count(); i++)
        {
            string m = can[i];
            OZG_GButton b;
            if (m.IndexOf(OZG_ThousandConst.M_SET) == 0)
            {
                array<int> values = new array<int>();
                ThValues(m, values);
                b = OZG_GButton.Make(m, OZG_Words.W2("STR_OZG_THOUSAND_BTN_SET", OZG_DiceText.Line(values), ThPoints(values).ToString()), 0, i);
            }
            else if (m == OZG_ThousandConst.M_WRITE)
            {
                b = OZG_GButton.Make(m, OZG_Words.W1("STR_OZG_THOUSAND_BTN_WRITE", m_TurnPoints.ToString()), -1, i);
            }
            else
            {
                int n = m_Left;
                if (n <= 0)
                    n = 5;
                b = OZG_GButton.Make(m, OZG_Words.W1("STR_OZG_THOUSAND_BTN_THROW", n.ToString()), -1, i);
                b.Hot = 1;
            }
            v.Buttons.Insert(b);
        }
        if (seat == DgCurrent())
            v.Status = OZG_Words.W0("STR_OZG_DICE_YOUR_TURN");
        return DgPrivateText(v);
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move.IndexOf(OZG_ThousandConst.M_SET) == 0)
        {
            into.Insert("STR_OZG_THOUSAND_LOG_SET");
            return;
        }
        if (move == OZG_ThousandConst.M_WRITE)
        {
            into.Insert("STR_OZG_THOUSAND_LOG_WRITE");
            return;
        }
        super.MoveWords(move, into);
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (m_Over || seat != DgCurrent())
            return "";
        if (m_Phase == 1)
        {
            // The set worth most.
            array<string> sets = new array<string>();
            ThSets(sets);
            string best = "";
            int bestPoints = -1;
            for (int k = 0; k < sets.Count(); k++)
            {
                array<int> values = new array<int>();
                ThValues(sets[k], values);
                int p = ThPoints(values);
                if (p > bestPoints)
                {
                    bestPoints = p;
                    best = sets[k];
                }
            }
            return best;
        }
        if (m_Phase == 2 && MayWrite())
        {
            // Writes the sooner the fewer dice are left to throw; throws on
            // rather than write into a pit while three dice or more are left.
            int left = m_Left;
            bool enough = m_TurnPoints >= 300;
            if (left <= 3 && m_TurnPoints >= 100)
                enough = true;
            if (left <= 2 && m_TurnPoints >= 50)
                enough = true;
            int now = m_Total[m_Turn] + m_TurnPoints;
            bool pit = (now >= 200 && now <= 300) || (now >= 600 && now <= 700);
            if (m_Pits && pit && left >= 3)
                enough = false;
            if (OnBarrel(m_Turn))
                enough = now >= OZG_ThousandConst.GOAL;
            if (enough)
                return OZG_ThousandConst.M_WRITE;
        }
        return OZG_DiceGameConst.M_THROW;
    }

    override string Brief()
    {
        string s = super.Brief() + " totals=" + Ints(m_Total, ",");
        s = s + " bolts=" + Ints(m_Bolts, ",") + " barrel=" + Ints(m_BarrelTries, ",");
        return s;
    }

    override string Everything()
    {
        string s = super.Everything() + " totals=" + Ints(m_Total, ",");
        s = s + " turn=" + m_TurnPoints.ToString() + " phase=" + m_Phase.ToString();
        s = s + " throw=" + Ints(m_Throw, " ");
        return s;
    }
}
