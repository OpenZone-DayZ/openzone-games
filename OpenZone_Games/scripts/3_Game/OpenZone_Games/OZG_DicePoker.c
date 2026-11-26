// Dice poker by The Witcher's rules (boards and dice design §6.2), for two to
// six, for a pot of chips; the first to win two rounds takes it. A round: each
// puts in the stake; each throws his five dice; in turn each may raise, and the
// others call or fold; each picks any of his dice and throws them once more;
// the best hand wins the round. Hands from the top: five of a kind, four of a
// kind, full house, the six-high straight (2-6), the five-high straight (1-5),
// three of a kind, two pairs, a pair, nothing. The same hand is decided by its
// dice (the higher pair, then the higher second pair, the higher three in a
// full house); then a draw, and the round is played again.
//
// Moves: "throw"; "check", "raise:<n>", "call", "fold"; "again:<positions>"
// (1-based, "again:0" for none).
class OZG_DPokerConst
{
    static const string GAME_ID = "dicepoker";
    static const string LABEL = "STR_OZG_GAME_DICEPOKER";
    static const string M_CHECK = "check";
    static const string M_RAISE = "raise:";
    static const string M_CALL = "call";
    static const string M_FOLD = "fold";
    static const string M_AGAIN = "again:";
    static const int WINS = 2;
}

class OZG_DicePoker extends OZG_DiceGame
{
    protected int m_Stake = 10;
    protected int m_Pot;
    protected ref array<int> m_Chips;
    // What each seat put in the pot: its own until the pot is won.
    protected ref array<int> m_Put;
    protected ref array<int> m_Wins;
    protected ref array<ref array<int>> m_Hand;
    protected ref array<int> m_In;
    // 0 throwing, 1 betting, 2 throwing again; the seats still to act, in order.
    protected int m_Phase;
    protected ref array<int> m_Pending;
    protected int m_Raise;
    protected int m_Raiser = -1;
    protected int m_Round;
    protected int m_RoundFirst;
    // Words: how the last round ended.
    protected string m_LastRound;

    void OZG_DicePoker()
    {
        m_Chips = new array<int>();
        m_Put = new array<int>();
        m_Wins = new array<int>();
        m_Hand = new array<ref array<int>>();
        m_In = new array<int>();
        m_Pending = new array<int>();
    }

    override string GameId()
    {
        return OZG_DPokerConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_DPokerConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_DicePoker();
    }

    override bool UsesStakes()
    {
        return true;
    }

    override protected void DgOptions(array<ref OZG_Option> into)
    {
        into.Insert(OZG_DiceOptions.Stake());
    }

    override protected void DgRead(map<string, string> options)
    {
        m_Stake = OptionOf(options, OZG_DiceConst.OPT_STAKE, "10").ToInt();
        if (m_Stake < 1)
            m_Stake = 10;
    }

    override protected bool DgBegin(out string refusal)
    {
        refusal = "";
        m_Chips.Clear();
        m_Put.Clear();
        m_Wins.Clear();
        m_Hand.Clear();
        m_In.Clear();
        m_Pot = 0;
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            int had = 0;
            int at = m_StartSeats.Find(m_Seats[i]);
            if (at >= 0 && at < m_StartChips.Count())
                had = m_StartChips[at];
            m_Chips.Insert(had);
            m_Put.Insert(0);
            m_Wins.Insert(0);
            m_Hand.Insert(new array<int>());
            m_In.Insert(0);
        }
        m_Round = 0;
        m_LastRound = "";
        m_RoundFirst = m_Turn;
        DpRound(true);
        return true;
    }

    override int StackOf(int seat)
    {
        int i = m_Seats.Find(seat);
        if (i < 0 || i >= m_Chips.Count())
            return -1;
        return m_Chips[i] + m_Put[i];
    }

    // Chips from a seat into the pot.
    protected void DpPay(int i, int n)
    {
        m_Chips[i] = m_Chips[i] - n;
        m_Put[i] = m_Put[i] + n;
        m_Pot = m_Pot + n;
    }

    // A round begins: the stakes put in (none when a drawn round is played
    // again), everybody throws in turn.
    protected void DpRound(bool stakes)
    {
        m_Round++;
        if (m_Round > 1)
            Say(OZG_Words.W1("STR_OZG_DPOKER_SAID_NEW", m_Round.ToString()));
        m_Raise = 0;
        m_Raiser = -1;
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            m_Hand[i].Clear();
            if (m_Gone.Find(i) >= 0)
            {
                m_In[i] = 0;
                continue;
            }
            m_In[i] = 1;
            if (!stakes)
                continue;
            int put = m_Stake;
            if (put > m_Chips[i])
                put = m_Chips[i];
            DpPay(i, put);
        }
        DpPhase(0);
    }

    // A phase begins: every seat still in, from the round's first on.
    protected void DpPhase(int phase)
    {
        m_Phase = phase;
        m_Pending.Clear();
        int n = m_Seats.Count();
        for (int k = 0; k < n; k++)
        {
            int j = (m_RoundFirst + k) % n;
            if (m_In[j] == 1)
                m_Pending.Insert(j);
        }
        DpNext();
    }

    protected int InCount()
    {
        int n = 0;
        for (int i = 0; i < m_In.Count(); i++)
        {
            if (m_In[i] == 1)
                n++;
        }
        return n;
    }

    // The next seat to act, or the phase's end.
    protected void DpNext()
    {
        if (m_Over)
            return;
        if (InCount() == 1 && m_Phase != 0)
        {
            for (int i = 0; i < m_In.Count(); i++)
            {
                if (m_In[i] == 1)
                {
                    Say(OZG_Words.W1("STR_OZG_DPOKER_SAID_FOLDS", OZG_Words.SeatArg(m_Seats[i])));
                    DpRoundWon(i, false);
                    return;
                }
            }
        }
        if (m_Pending.Count() > 0)
        {
            m_Turn = m_Pending[0];
            return;
        }
        if (m_Phase == 0)
        {
            DpPhase(1);
            return;
        }
        if (m_Phase == 1)
        {
            DpPhase(2);
            return;
        }
        DpShowdown();
    }

    protected void DpDone()
    {
        if (m_Pending.Count() > 0)
            m_Pending.RemoveOrdered(0);
        DpNext();
    }

    override protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (seat != DgCurrent() || m_Pending.Count() == 0 || m_Pending[0] != m_Turn)
            return false;
        int i = m_Turn;
        if (m_Phase == 0 && move == OZG_DiceGameConst.M_THROW)
        {
            DgRollInto(seat, 5, m_Hand[i]);
            SayMove(OZG_Words.W2("STR_OZG_DPOKER_SAID_THROW", OZG_Words.SeatArg(seat), DpHandArg(m_Hand[i])));
            DpDone();
            return true;
        }
        if (m_Phase == 1)
        {
            if (move == OZG_DPokerConst.M_CHECK && m_Raise == 0)
            {
                DpDone();
                return true;
            }
            if (move.IndexOf(OZG_DPokerConst.M_RAISE) == 0 && m_Raise == 0)
            {
                int n = move.Substring(6, move.Length() - 6).ToInt();
                if (n < 1 || n > m_Chips[i])
                    return false;
                m_Raise = n;
                m_Raiser = i;
                DpPay(i, n);
                // Everybody else still in answers it.
                m_Pending.Clear();
                int count = m_Seats.Count();
                for (int k = 1; k < count; k++)
                {
                    int j = (i + k) % count;
                    if (m_In[j] == 1)
                        m_Pending.Insert(j);
                }
                DpNext();
                return true;
            }
            if (move == OZG_DPokerConst.M_CALL && m_Raise > 0)
            {
                int pay = m_Raise;
                if (pay > m_Chips[i])
                    pay = m_Chips[i];
                DpPay(i, pay);
                DpDone();
                return true;
            }
            if (move == OZG_DPokerConst.M_FOLD && m_Raise > 0)
            {
                m_In[i] = 0;
                DpDone();
                return true;
            }
            return false;
        }
        if (m_Phase == 2 && move.IndexOf(OZG_DPokerConst.M_AGAIN) == 0)
        {
            array<int> positions = new array<int>();
            IntsOf(move.Substring(6, move.Length() - 6), ",", positions);
            array<int> hand = m_Hand[i];
            int thrown = 0;
            for (int p = 0; p < positions.Count(); p++)
            {
                int at = positions[p] - 1;
                if (at < 0 || at >= hand.Count())
                    continue;
                hand[at] = DgRoll(seat, at);
                thrown++;
            }
            string who = OZG_Words.SeatArg(seat);
            string handArg = DpHandArg(hand);
            string thrownText = thrown.ToString();
            if (thrown > 0)
                SayMove(OZG_Words.W3("STR_OZG_DPOKER_SAID_AGAIN", who, thrownText, handArg));
            else
                SayMove(OZG_Words.W2("STR_OZG_DPOKER_SAID_KEEP", who, handArg));
            DpDone();
            return true;
        }
        return false;
    }

    // The rank of a hand as one number: the hand's kind first, then the dice
    // that decide it.
    static int DpRank(array<int> hand)
    {
        int c[7];
        int f;
        for (f = 0; f < 7; f++)
            c[f] = 0;
        for (int i = 0; i < hand.Count(); i++)
            c[hand[i]] = c[hand[i]] + 1;
        int five = 0;
        int four = 0;
        int three = 0;
        int pairHigh = 0;
        int pairLow = 0;
        for (f = 6; f >= 1; f--)
        {
            if (c[f] == 5)
                five = f;
            else if (c[f] == 4)
                four = f;
            else if (c[f] == 3)
                three = f;
            else if (c[f] == 2 && pairHigh == 0)
                pairHigh = f;
            else if (c[f] == 2)
                pairLow = f;
        }
        if (five > 0)
            return 80000 + five * 100;
        if (four > 0)
            return 70000 + four * 100;
        if (three > 0 && pairHigh > 0)
            return 60000 + three * 100 + pairHigh;
        bool low = true;
        bool high = true;
        for (f = 1; f <= 5; f++)
        {
            if (c[f] != 1)
                low = false;
            if (c[f + 1] != 1)
                high = false;
        }
        if (high)
            return 50000;
        if (low)
            return 40000;
        if (three > 0)
            return 30000 + three * 100;
        if (pairHigh > 0 && pairLow > 0)
            return 20000 + pairHigh * 100 + pairLow;
        if (pairHigh > 0)
            return 10000 + pairHigh * 100;
        return 0;
    }

    protected void DpShowdown()
    {
        int best = -1;
        int i;
        for (i = 0; i < m_Seats.Count(); i++)
        {
            if (m_In[i] == 1 && DpRank(m_Hand[i]) > best)
                best = DpRank(m_Hand[i]);
        }
        array<int> top = new array<int>();
        for (i = 0; i < m_Seats.Count(); i++)
        {
            if (m_In[i] == 1 && DpRank(m_Hand[i]) == best)
                top.Insert(i);
        }
        if (top.Count() == 1)
        {
            DpRoundWon(top[0], true);
            return;
        }
        // A draw: the round again, the pot as it is.
        m_LastRound = OZG_Words.W0("STR_OZG_DPOKER_DRAW");
        m_Hist.Insert("draw");
        Say(OZG_Words.W0("STR_OZG_DPOKER_SAID_DRAW"));
        DpRound(false);
    }

    // A hand's name as an argument ("k:" its key).
    protected string DpHandArg(array<int> hand)
    {
        int kind = DpRank(hand) / 10000;
        return OZG_Words.KeyArg("STR_OZG_DPOKER_HAND_" + kind.ToString());
    }

    protected void DpRoundWon(int i, bool shown)
    {
        m_Wins[i] = m_Wins[i] + 1;
        int kind = DpRank(m_Hand[i]) / 10000;
        m_LastRound = OZG_Words.W2("STR_OZG_DPOKER_ROUND", OZG_Words.SeatArg(m_Seats[i]), OZG_Words.KeyArg("STR_OZG_DPOKER_HAND_" + kind.ToString()));
        if (shown)
            Say(OZG_Words.W2("STR_OZG_DPOKER_SAID_ROUND", OZG_Words.SeatArg(m_Seats[i]), OZG_Words.KeyArg("STR_OZG_DPOKER_HAND_" + kind.ToString())));
        if (m_Wins[i] >= OZG_DPokerConst.WINS)
        {
            m_Chips[i] = m_Chips[i] + m_Pot;
            string pot = m_Pot.ToString();
            m_Pot = 0;
            for (int k = 0; k < m_Put.Count(); k++)
                m_Put[k] = 0;
            DgFinish(OZG_Words.W2("STR_OZG_DPOKER_END", OZG_Words.SeatArg(m_Seats[i]), pot));
            return;
        }
        m_RoundFirst = DgNextIndex(m_RoundFirst);
        DpRound(true);
    }

    // Won with the others gone: the pot goes along.
    override protected void DgWonBy(int i, string reasonKey)
    {
        m_Chips[i] = m_Chips[i] + m_Pot;
        m_Pot = 0;
        for (int k = 0; k < m_Put.Count(); k++)
            m_Put[k] = 0;
        super.DgWonBy(i, reasonKey);
    }

    override protected void DgGone(int index)
    {
        if (index < m_In.Count())
            m_In[index] = 0;
        int at = m_Pending.Find(index);
        if (at >= 0)
            m_Pending.RemoveOrdered(at);
        if (DgLeft() > 1)
            DpNext();
    }

    override protected void DgPassTurn()
    {
        // An absent player folds, keeps his dice or checks, as the phase asks.
        int i = m_Turn;
        if (m_Phase == 0 || (m_Phase == 1 && m_Raise > 0))
            m_In[i] = 0;
        DpDone();
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || seat != DgCurrent())
            return;
        int i = m_Turn;
        if (m_Phase == 0)
        {
            into.Insert(OZG_DiceGameConst.M_THROW);
            return;
        }
        if (m_Phase == 1)
        {
            if (m_Raise == 0)
            {
                into.Insert(OZG_DPokerConst.M_CHECK);
                // A raise of any amount: the window's box; listed by its ends.
                if (m_Chips[i] > 0)
                    into.Insert(OZG_DPokerConst.M_RAISE + "1");
                if (m_Chips[i] > 1)
                    into.Insert(OZG_DPokerConst.M_RAISE + m_Chips[i].ToString());
                return;
            }
            into.Insert(OZG_DPokerConst.M_CALL);
            into.Insert(OZG_DPokerConst.M_FOLD);
            return;
        }
        DpAgainChoices(i, into);
    }

    // What may be thrown again: nothing, each die alone, the dice of no pair
    // or more, everything.
    protected void DpAgainChoices(int i, array<string> into)
    {
        array<int> hand = m_Hand[i];
        into.Insert(OZG_DPokerConst.M_AGAIN + "0");
        int c[7];
        int f;
        for (f = 0; f < 7; f++)
            c[f] = 0;
        int k;
        for (k = 0; k < hand.Count(); k++)
            c[hand[k]] = c[hand[k]] + 1;
        string loose = "";
        for (k = 0; k < hand.Count(); k++)
        {
            into.Insert(OZG_DPokerConst.M_AGAIN + (k + 1).ToString());
            if (c[hand[k]] < 2)
            {
                if (loose != "")
                    loose = loose + ",";
                loose = loose + (k + 1).ToString();
            }
        }
        string loosed = OZG_DPokerConst.M_AGAIN + loose;
        if (loose != "" && into.Find(loosed) < 0)
            into.Insert(loosed);
        into.Insert(OZG_DPokerConst.M_AGAIN + "1,2,3,4,5");
    }

    override string PublicView()
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = GameId();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            // the chips, the rounds won, what it put in the pot (the window's plates)
            string line = OZG_Words.W3("STR_OZG_DPOKER_SEAT", m_Chips[i].ToString(), m_Wins[i].ToString(), m_Put[i].ToString());
            OZG_GSeat g = DgPlate(m_Seats[i], OZG_Words.W1("STR_OZG_DICE_SHOWN", OZG_DiceText.Line(m_Hand[i])), line);
            if (m_In[i] == 0)
                g.Out = 1;
            v.SeatList.Insert(g);
        }
        OZG_GZone pot = new OZG_GZone();
        pot.Id = "pot";
        pot.Label = OZG_Words.W0("STR_OZG_DICE_POT");
        pot.Line = OZG_Words.W1("STR_OZG_DICE_POT_LINE", m_Pot.ToString());
        v.Zones.Insert(pot);
        // the window's round panel: the phase, the round, a raise standing and its seat
        OZG_GZone state = new OZG_GZone();
        state.Id = "dpoker";
        int raiser = -1;
        if (m_Raiser >= 0 && m_Raiser < m_Seats.Count())
            raiser = m_Seats[m_Raiser];
        state.Line = m_Phase.ToString() + "|" + m_Round.ToString() + "|" + m_Raise.ToString() + "|" + raiser.ToString() + "|" + m_Stake.ToString();
        v.Zones.Insert(state);
        if (m_LastRound != "")
        {
            OZG_GZone last = new OZG_GZone();
            last.Id = "last";
            last.Label = OZG_Words.W0("STR_OZG_DPOKER_LAST");
            last.Line = m_LastRound;
            v.Zones.Insert(last);
        }
        if (m_Over)
        {
            v.Ended = 1;
            v.Result = m_End;
            v.Status = m_End;
        }
        else
        {
            v.Awaited.Insert(DgCurrent());
            string key = "STR_OZG_DPOKER_PHASE_" + m_Phase.ToString();
            v.Status = OZG_Words.W2(key, OZG_Words.SeatArg(DgCurrent()), m_Round.ToString());
        }
        return DgPublicText(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        array<string> can = new array<string>();
        Moves(seat, can);
        int col = 0;
        int i = m_Seats.Find(seat);
        for (int k = 0; k < can.Count(); k++)
        {
            string m = can[k];
            if (m.IndexOf(OZG_DPokerConst.M_RAISE) == 0)
            {
                v.AmountMove = OZG_DPokerConst.M_RAISE;
                v.AmountLabel = OZG_Words.W0("STR_OZG_DPOKER_BTN_RAISE");
                v.AmountMin = 1;
                v.AmountMax = m_Chips[i];
                v.AmountStart = m_Stake;
                if (v.AmountStart > v.AmountMax)
                    v.AmountStart = v.AmountMax;
                continue;
            }
            string label;
            if (m.IndexOf(OZG_DPokerConst.M_AGAIN) == 0)
                label = OZG_Words.W1("STR_OZG_DPOKER_BTN_AGAIN", DpAgainText(i, m));
            else if (m == OZG_DPokerConst.M_CALL)
                label = OZG_Words.W1("STR_OZG_DPOKER_BTN_CALL", m_Raise.ToString());
            else
            {
                string key = "STR_OZG_DPOKER_BTN_" + m;
                key.ToUpper();
                label = OZG_Words.W0(key);
            }
            OZG_GButton b = OZG_GButton.Make(m, label, -1, col);
            if (m == OZG_DiceGameConst.M_THROW || m == OZG_DPokerConst.M_CALL)
                b.Hot = 1;
            v.Buttons.Insert(b);
            col++;
        }
        if (i >= 0 && m_Hand[i].Count() > 0)
            v.Status = OZG_Words.W2("STR_OZG_DPOKER_YOURS", OZG_DiceText.Line(m_Hand[i]), OZG_Words.KeyArg("STR_OZG_DPOKER_HAND_" + (DpRank(m_Hand[i]) / 10000).ToString()));
        return DgPrivateText(v);
    }

    // The dice a choice throws again, as their values ("-" for none).
    protected string DpAgainText(int i, string move)
    {
        array<int> positions = new array<int>();
        IntsOf(move.Substring(6, move.Length() - 6), ",", positions);
        array<int> hand = m_Hand[i];
        array<int> values = new array<int>();
        for (int p = 0; p < positions.Count(); p++)
        {
            int at = positions[p] - 1;
            if (at >= 0 && at < hand.Count())
                values.Insert(hand[at]);
        }
        if (values.Count() == 0)
            return "-";
        return OZG_DiceText.Line(values);
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move.IndexOf(OZG_DPokerConst.M_RAISE) == 0)
        {
            into.Insert("STR_OZG_DPOKER_LOG_RAISE");
            into.Insert(move.Substring(6, move.Length() - 6));
            return;
        }
        if (move.IndexOf(OZG_DPokerConst.M_AGAIN) == 0)
        {
            into.Insert("STR_OZG_DPOKER_LOG_AGAIN");
            return;
        }
        if (move == OZG_DPokerConst.M_CHECK || move == OZG_DPokerConst.M_CALL || move == OZG_DPokerConst.M_FOLD)
        {
            string key = "STR_OZG_DPOKER_LOG_" + move;
            key.ToUpper();
            into.Insert(key);
            return;
        }
        super.MoveWords(move, into);
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (m_Over || seat != DgCurrent())
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        int i = m_Turn;
        if (m_Phase == 0)
            return OZG_DiceGameConst.M_THROW;
        if (m_Phase == 1)
        {
            int rank = DpRank(m_Hand[i]);
            if (m_Raise == 0)
            {
                if (rank >= 30000 && m_Chips[i] > 0)
                    return OZG_DPokerConst.M_RAISE + "1";
                return OZG_DPokerConst.M_CHECK;
            }
            if (rank >= 10000 || dice.Below(2) == 0)
                return OZG_DPokerConst.M_CALL;
            return OZG_DPokerConst.M_FOLD;
        }
        // The dice of no pair or more thrown again.
        array<string> choices = new array<string>();
        DpAgainChoices(i, choices);
        if (choices.Count() >= 2)
            return choices[choices.Count() - 2];
        return OZG_DPokerConst.M_AGAIN + "0";
    }

    override string Everything()
    {
        string s = super.Everything() + " pot=" + m_Pot.ToString() + " chips=" + Ints(m_Chips, ",");
        s = s + " wins=" + Ints(m_Wins, ",") + " phase=" + m_Phase.ToString();
        for (int i = 0; i < m_Hand.Count(); i++)
            s = s + " h" + i.ToString() + "=" + Ints(m_Hand[i], "");
        return s;
    }
}
