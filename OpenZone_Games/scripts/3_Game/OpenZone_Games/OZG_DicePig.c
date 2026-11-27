// Pig and high roll (boards and dice design §6.3, §6.4).
//
// Pig: one die, thrown until the player holds or throws a 1, which loses the
// turn's points; the first to 100 wins.
//
// High roll: each throws two dice (one to three, the host's option) for a pot
// everybody put a stake in (the host's option too); the highest total takes it,
// a tie is thrown again by those tied.
class OZG_PigConst
{
    static const string GAME_ID = "pig";
    static const string LABEL = "STR_OZG_GAME_PIG";
    static const int GOAL = 100;
    static const string M_HOLD = "hold";
}

class OZG_Pig extends OZG_DiceGame
{
    protected ref array<int> m_Score;
    protected int m_TurnPoints;
    protected int m_LastDie;
    // the throws so far: the window turns the die a little differently each throw
    protected int m_Throws;

    void OZG_Pig()
    {
        m_Score = new array<int>();
    }

    override string GameId()
    {
        return OZG_PigConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_PigConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Pig();
    }

    override int DiceNeeded()
    {
        return 1;
    }

    override protected bool DgBegin(out string refusal)
    {
        refusal = "";
        m_Score.Clear();
        for (int i = 0; i < m_Seats.Count(); i++)
            m_Score.Insert(0);
        m_TurnPoints = 0;
        m_LastDie = 0;
        m_Throws = 0;
        return true;
    }

    override protected void DgPassTurn()
    {
        m_TurnPoints = 0;
        m_Turn = DgNextIndex(m_Turn);
    }

    override protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (seat != DgCurrent())
            return false;
        if (move == OZG_DiceGameConst.M_THROW)
        {
            int v = DgRoll(seat, 0);
            m_LastDie = v;
            m_Throws++;
            if (v == 1)
            {
                m_Hist.Insert(seat.ToString() + " 1");
                if (m_TurnPoints > 0)
                    SayMove(OZG_Words.W2("STR_OZG_PIG_SAID_BURNT", OZG_Words.SeatArg(seat), m_TurnPoints.ToString()));
                else
                    SayMove(OZG_Words.W1("STR_OZG_PIG_SAID_ONE", OZG_Words.SeatArg(seat)));
                DgPassTurn();
                return true;
            }
            m_TurnPoints = m_TurnPoints + v;
            SayMove(OZG_Words.W2("STR_OZG_PIG_SAID_THROW", OZG_Words.SeatArg(seat), v.ToString()));
            return true;
        }
        if (move == OZG_PigConst.M_HOLD && m_TurnPoints > 0)
        {
            int total = m_Score[m_Turn] + m_TurnPoints;
            SayMove(OZG_Words.W3("STR_OZG_PIG_SAID_HOLD", OZG_Words.SeatArg(seat), m_TurnPoints.ToString(), total.ToString()));
            m_Score[m_Turn] = m_Score[m_Turn] + m_TurnPoints;
            if (m_Score[m_Turn] >= OZG_PigConst.GOAL)
            {
                DgWonBy(m_Turn, "STR_OZG_PIG_END");
                return true;
            }
            DgPassTurn();
            return true;
        }
        return false;
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || seat != DgCurrent())
            return;
        into.Insert(OZG_DiceGameConst.M_THROW);
        if (m_TurnPoints > 0)
            into.Insert(OZG_PigConst.M_HOLD);
    }

    override string PublicView()
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = GameId();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            string line = OZG_Words.W1("STR_OZG_DICE_SCORE", m_Score[i].ToString());
            OZG_GSeat g = DgPlate(m_Seats[i], "", line);
            if (m_Gone.Find(i) >= 0)
                g.Out = 1;
            v.SeatList.Insert(g);
        }
        OZG_GZone zone = new OZG_GZone();
        zone.Id = "dice";
        zone.Label = OZG_Words.W0("STR_OZG_PIG_DIE");
        string die = "-";
        if (m_LastDie > 0)
            die = m_LastDie.ToString();
        zone.Line = OZG_Words.W2("STR_OZG_PIG_LINE", die, m_TurnPoints.ToString());
        v.Zones.Insert(zone);
        // the window's die and its pill: the last die, the turn's points, the goal, the throws
        OZG_GZone state = new OZG_GZone();
        state.Id = "pig";
        state.Line = m_LastDie.ToString() + "|" + m_TurnPoints.ToString() + "|" + OZG_PigConst.GOAL.ToString() + "|" + m_Throws.ToString();
        v.Zones.Insert(state);
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
        return DgPublicOut(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int i = 0; i < can.Count(); i++)
        {
            string key = "STR_OZG_PIG_BTN_" + can[i];
            key.ToUpper();
            OZG_GButton b = OZG_GButton.Make(can[i], OZG_Words.W0(key), -1, i);
            if (can[i] == OZG_DiceGameConst.M_THROW)
                b.Hot = 1;
            v.Buttons.Insert(b);
        }
        if (seat == DgCurrent())
            v.Status = OZG_Words.W0("STR_OZG_DICE_YOUR_TURN");
        return DgPrivateText(v);
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move == OZG_PigConst.M_HOLD)
        {
            into.Insert("STR_OZG_PIG_LOG_HOLD");
            return;
        }
        super.MoveWords(move, into);
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (seat != DgCurrent())
            return "";
        // Holds at twenty points a turn, or when it wins.
        int i = m_Seats.Find(seat);
        if (m_TurnPoints >= 20 || (m_TurnPoints > 0 && m_Score[i] + m_TurnPoints >= OZG_PigConst.GOAL))
            return OZG_PigConst.M_HOLD;
        return OZG_DiceGameConst.M_THROW;
    }

    override string Everything()
    {
        return super.Everything() + " scores=" + Ints(m_Score, ",") + " turn=" + m_TurnPoints.ToString();
    }
}

class OZG_HighRollConst
{
    static const string GAME_ID = "highroll";
    static const string LABEL = "STR_OZG_GAME_HIGHROLL";
    static const string OPT_DICE = "count";
    static const string OPT_STAKE = "stake";
}

class OZG_HighRoll extends OZG_DiceGame
{
    protected int m_Count = 2;
    protected int m_Stake = 10;
    protected int m_Pot;
    protected ref array<int> m_Chips;
    // What each seat put in the pot: its own until the pot is won.
    protected ref array<int> m_Put;
    // The seats still throwing for the pot (by index), and each one's total
    // this throw (-1: not thrown yet).
    protected ref array<int> m_InIt;
    protected ref array<int> m_Total;
    protected ref array<string> m_Shown;
    // the seat (by index) that took the pot, -1 before
    protected int m_Won = -1;

    void OZG_HighRoll()
    {
        m_Chips = new array<int>();
        m_Put = new array<int>();
        m_InIt = new array<int>();
        m_Total = new array<int>();
        m_Shown = new array<string>();
    }

    override string GameId()
    {
        return OZG_HighRollConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_HighRollConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_HighRoll();
    }

    override int MaxPlayers()
    {
        return 8;
    }

    override int DiceNeeded()
    {
        return m_Count;
    }

    override int DiceNeededWith(map<string, string> options)
    {
        int count = OptionOf(options, OZG_HighRollConst.OPT_DICE, "2").ToInt();
        if (count < 1 || count > 3)
            count = 2;
        return count;
    }

    override bool UsesStakes()
    {
        return true;
    }

    override protected void DgOptions(array<ref OZG_Option> into)
    {
        OZG_Option dice = new OZG_Option();
        dice.OptionId = OZG_HighRollConst.OPT_DICE;
        dice.LabelKey = "STR_OZG_HIGHROLL_DICE";
        for (int n = 1; n <= 3; n++)
        {
            dice.ValueIds.Insert(n.ToString());
            dice.ValueKeys.Insert("STR_OZG_HIGHROLL_DICE_" + n.ToString());
        }
        dice.DefaultId = "2";
        into.Insert(dice);
        into.Insert(OZG_DiceOptions.Stake());
    }

    override protected void DgRead(map<string, string> options)
    {
        m_Count = OptionOf(options, OZG_HighRollConst.OPT_DICE, "2").ToInt();
        if (m_Count < 1 || m_Count > 3)
            m_Count = 2;
        m_Stake = OptionOf(options, OZG_HighRollConst.OPT_STAKE, "10").ToInt();
        if (m_Stake < 1)
            m_Stake = 10;
    }

    override protected bool DgBegin(out string refusal)
    {
        refusal = "";
        m_Chips.Clear();
        m_Put.Clear();
        m_InIt.Clear();
        m_Total.Clear();
        m_Shown.Clear();
        m_Pot = 0;
        m_Won = -1;
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            int had = 0;
            int at = m_StartSeats.Find(m_Seats[i]);
            if (at >= 0 && at < m_StartChips.Count())
                had = m_StartChips[at];
            int put = m_Stake;
            if (put > had)
                put = had;
            m_Chips.Insert(had - put);
            m_Put.Insert(put);
            m_Pot = m_Pot + put;
            m_InIt.Insert(i);
            m_Total.Insert(-1);
            m_Shown.Insert("");
        }
        return true;
    }

    override int StackOf(int seat)
    {
        int i = m_Seats.Find(seat);
        if (i < 0 || i >= m_Chips.Count())
            return -1;
        return m_Chips[i] + m_Put[i];
    }

    override protected void DgGone(int index)
    {
        int at = m_InIt.Find(index);
        if (at >= 0)
            m_InIt.RemoveOrdered(at);
        if (m_InIt.Count() == 1)
            HrSettle(m_InIt[0]);
    }

    // The next seat of those throwing that has not thrown, from the turn on.
    protected int HrNext()
    {
        int n = m_Seats.Count();
        for (int k = 0; k < n; k++)
        {
            int j = (m_Turn + k) % n;
            if (m_InIt.Find(j) >= 0 && m_Total[j] < 0)
                return j;
        }
        return -1;
    }

    override protected void DgPassTurn()
    {
        // A seat skipped throws nothing: it is out of this pot's throwing.
        int at = m_InIt.Find(m_Turn);
        if (at >= 0 && m_InIt.Count() > 1)
            m_InIt.RemoveOrdered(at);
        HrAfterThrow();
    }

    override protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (seat != DgCurrent() || move != OZG_DiceGameConst.M_THROW)
            return false;
        array<int> dice = new array<int>();
        DgRollInto(seat, m_Count, dice);
        int sum = 0;
        for (int i = 0; i < dice.Count(); i++)
            sum = sum + dice[i];
        m_Total[m_Turn] = sum;
        m_Shown[m_Turn] = OZG_DiceText.Line(dice);
        string key = "STR_OZG_HIGHROLL_SAID_THROW_" + dice.Count().ToString();
        string said = OZG_Words.W1(key, OZG_Words.SeatArg(seat));
        for (int k = 0; k < dice.Count(); k++)
            said = said + OZG_Words.SEP + dice[k].ToString();
        SayMove(said + OZG_Words.SEP + sum.ToString());
        HrAfterThrow();
        return true;
    }

    // Everybody throwing has thrown: the highest takes the pot, or those tied
    // throw again.
    protected void HrAfterThrow()
    {
        int next = HrNext();
        if (next >= 0)
        {
            m_Turn = next;
            return;
        }
        int best = -1;
        int i;
        for (i = 0; i < m_InIt.Count(); i++)
        {
            if (m_Total[m_InIt[i]] > best)
                best = m_Total[m_InIt[i]];
        }
        array<int> tied = new array<int>();
        for (i = 0; i < m_InIt.Count(); i++)
        {
            if (m_Total[m_InIt[i]] == best)
                tied.Insert(m_InIt[i]);
        }
        // one is the highest: the pot is his; those who threw with him stay in the throwing
        // (the window dims only who fell out of a tie's throw again)
        if (tied.Count() == 1)
        {
            HrSettle(tied[0]);
            return;
        }
        m_InIt.Copy(tied);
        m_Hist.Insert("tie " + best.ToString());
        Say(OZG_Words.W1("STR_OZG_HIGHROLL_SAID_TIE", best.ToString()));
        for (i = 0; i < m_InIt.Count(); i++)
        {
            m_Total[m_InIt[i]] = -1;
            m_Shown[m_InIt[i]] = "";
        }
        m_Turn = m_InIt[0];
    }

    protected void HrSettle(int winner)
    {
        if (winner < 0 || winner >= m_Chips.Count())
            return;
        m_Won = winner;
        m_Chips[winner] = m_Chips[winner] + m_Pot;
        string pot = m_Pot.ToString();
        // the words made first (dice poker's view says why)
        int winnerSeat = m_Seats[winner];
        int winnerTotal = m_Total[winner];
        string winnerArg = OZG_Words.SeatArg(winnerSeat);
        string totalText = winnerTotal.ToString();
        if (winnerTotal >= 0)
            Say(OZG_Words.W3("STR_OZG_HIGHROLL_SAID_WON", winnerArg, totalText, pot));
        else
            Say(OZG_Words.W2("STR_OZG_HIGHROLL_SAID_TAKES", winnerArg, pot));
        m_Pot = 0;
        for (int i = 0; i < m_Put.Count(); i++)
            m_Put[i] = 0;
        DgFinish(OZG_Words.W2("STR_OZG_HIGHROLL_END", winnerArg, pot));
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (!m_Over && seat == DgCurrent())
            into.Insert(OZG_DiceGameConst.M_THROW);
    }

    override string PublicView()
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = GameId();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            string line = OZG_Words.W1("STR_OZG_DICE_CHIPS", m_Chips[i].ToString());
            if (m_Shown[i] != "")
                line = OZG_Words.W2("STR_OZG_HIGHROLL_THREW", m_Shown[i], m_Chips[i].ToString());
            // the window's cards: the dice of this throw, the chips, the stake
            string role = "";
            if (m_Shown[i] != "")
                role = OZG_Words.W1("STR_OZG_DICE_SHOWN", m_Shown[i]);
            OZG_GSeat g = DgPlate(m_Seats[i], role, line);
            if (m_InIt.Find(i) < 0)
                g.Out = 1;
            if (i == m_Won)
                g.Mark = 1;
            v.SeatList.Insert(g);
        }
        OZG_GZone state = new OZG_GZone();
        state.Id = "highroll";
        string chips = "";
        string puts = "";
        for (int c = 0; c < m_Seats.Count(); c++)
        {
            if (c > 0)
            {
                chips = chips + ",";
                puts = puts + ",";
            }
            chips = chips + m_Chips[c].ToString();
            puts = puts + m_Put[c].ToString();
        }
        state.Line = m_Count.ToString() + "|" + m_Stake.ToString() + "|" + chips + "|" + puts;
        v.Zones.Insert(state);
        OZG_GZone zone = new OZG_GZone();
        zone.Id = "pot";
        zone.Label = OZG_Words.W0("STR_OZG_DICE_POT");
        zone.Line = OZG_Words.W1("STR_OZG_DICE_POT_LINE", m_Pot.ToString());
        v.Zones.Insert(zone);
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
        return DgPublicOut(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        if (!m_Over && seat == DgCurrent())
        {
            OZG_GButton b = OZG_GButton.Make(OZG_DiceGameConst.M_THROW, OZG_Words.W0("STR_OZG_DICE_BTN_THROW"), -1, 0);
            b.Hot = 1;
            v.Buttons.Insert(b);
            v.Status = OZG_Words.W0("STR_OZG_DICE_YOUR_TURN");
        }
        return DgPrivateText(v);
    }

    override string Everything()
    {
        return super.Everything() + " pot=" + m_Pot.ToString() + " chips=" + Ints(m_Chips, ",") + " totals=" + Ints(m_Total, ",");
    }
}
