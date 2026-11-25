// Texas hold'em, no limit (stakes-and-games design §6). The rules and nothing
// else. A deal is one hand; the button passes from hand to hand through the
// hint. Chips put in this hand count as the seat's until the pots are
// shared (StackOf), so a hand broken off gives every seat its chips back.
class OZG_PokerConst
{
    static const string GAME_ID = "poker";
    static const int VERSION = 1;
    static const int MAX_SEATS = 8;

    static const string OPT_BLINDS = "blinds";
    static const string HINT_BUTTON = "button";

    // The streets.
    static const int PRE   = 0;
    static const int FLOP  = 1;
    static const int TURN  = 2;
    static const int RIVER = 3;
    static const int DONE  = 4;

    static const string NO_TURN   = "STR_OZG_G_NO_TURN";
    static const string NO_AMOUNT = "STR_OZG_G_NO_AMOUNT";
    static const string NO_CHECK  = "STR_OZG_POKER_NO_CHECK";
    static const string NO_RAISE  = "STR_OZG_POKER_NO_RAISE";
    static const string NO_SMALL  = "STR_OZG_POKER_NO_SMALL";
}

class OZG_PokerSeat
{
    int SeatNo;
    ref array<string> Held;
    // Chips behind; put in on this street; put in this hand in all.
    int Chips;
    int BetNow;
    int Put;
    int Folded;
    int AllIn;
    // Has acted since the betting was last reopened; may raise when its turn
    // comes (a short all-in does not reopen it).
    int Acted;
    int CanRaise;
    int Shown;
    // Chips this hand's pots gave it.
    int Won;
    // Words: what it did last.
    string Last;

    void OZG_PokerSeat()
    {
        Held = new array<string>();
    }
}

class OZG_PokerState
{
    int Version;
    int SmallBlind;
    int BigBlind;
    ref array<ref OZG_PokerSeat> SeatList;
    ref array<string> StockCards;
    ref array<string> BoardCards;
    // Folded hands, face down.
    ref array<string> Muck;
    int Button;
    int StreetNo;
    int Turn;
    int CurrentBet;
    int MinRaise;
    int Ended;
    // Words, a line each: who took what.
    ref array<string> Results;
    ref array<ref OZG_DealCard> Dealt;

    void OZG_PokerState()
    {
        SeatList = new array<ref OZG_PokerSeat>();
        StockCards = new array<string>();
        BoardCards = new array<string>();
        Muck = new array<string>();
        Results = new array<string>();
        Dealt = new array<ref OZG_DealCard>();
        Turn = -1;
    }
}

class OZG_Poker : OZG_CardGame
{
    protected ref OZG_PokerState m_S;
    protected ref map<int, int> m_Stakes;

    override string GameId()
    {
        return OZG_PokerConst.GAME_ID;
    }

    override string LabelKey()
    {
        return "STR_OZG_GAME_POKER";
    }

    override int MinPlayers()
    {
        return 2;
    }

    override int MaxPlayers()
    {
        return OZG_PokerConst.MAX_SEATS;
    }

    override void CardsUsed(array<string> into)
    {
        OZG_Cards.Whole(into);
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        OZG_Option blinds = new OZG_Option();
        blinds.OptionId = OZG_PokerConst.OPT_BLINDS;
        blinds.LabelKey = "STR_OZG_POKER_BLINDS";
        array<string> values = {"1-2", "2-5", "5-10", "10-20", "25-50", "50-100", "100-200"};
        for (int i = 0; i < values.Count(); i++)
        {
            blinds.ValueIds.Insert(values[i]);
            string key = "STR_OZG_POKER_BLINDS_" + values[i];
            key.Replace("-", "_");
            blinds.ValueKeys.Insert(key);
        }
        blinds.DefaultId = "5-10";
        into.Insert(blinds);
    }

    override OZG_Game Fresh()
    {
        return new OZG_Poker();
    }

    override bool UsesStakes()
    {
        return true;
    }

    override void TakeStacks(array<int> seats, array<int> amounts)
    {
        m_Stakes = new map<int, int>();
        for (int i = 0; i < seats.Count() && i < amounts.Count(); i++)
            m_Stakes.Set(seats[i], amounts[i]);
    }

    protected int StakeFor(int seat)
    {
        if (!m_Stakes)
            return 0;
        int n = 0;
        m_Stakes.Find(seat, n);
        if (n < 0)
            n = 0;
        return n;
    }

    // ---- what the base asks ---------------------------------------------------

    override protected array<ref OZG_DealCard> DealtList()
    {
        if (!m_S)
            return null;
        return m_S.Dealt;
    }

    override protected int SeatsIn()
    {
        if (!m_S)
            return 0;
        return m_S.SeatList.Count();
    }

    override protected int SeatNoAt(int i)
    {
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return -1;
        return m_S.SeatList[i].SeatNo;
    }

    override protected void HiddenAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        OZG_PokerSeat s = m_S.SeatList[i];
        if (s.Shown == 0)
            into.Copy(s.Held);
    }

    override protected void ShownAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        OZG_PokerSeat s = m_S.SeatList[i];
        if (s.Shown == 1)
            into.Copy(s.Held);
    }

    override protected void TableUp(array<string> into)
    {
        into.Clear();
        if (m_S)
            into.Copy(m_S.BoardCards);
    }

    override protected string StockTop()
    {
        if (!m_S || m_S.StockCards.Count() == 0)
            return "";
        return m_S.StockCards[0];
    }

    override protected void AllCards(array<string> into)
    {
        into.Clear();
        if (!m_S)
            return;
        int k;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (k = 0; k < held.Count(); k++)
                into.Insert(held[k]);
        }
        for (k = 0; k < m_S.StockCards.Count(); k++)
            into.Insert(m_S.StockCards[k]);
        for (k = 0; k < m_S.BoardCards.Count(); k++)
            into.Insert(m_S.BoardCards[k]);
        for (k = 0; k < m_S.Muck.Count(); k++)
            into.Insert(m_S.Muck[k]);
    }

    // ---- the hand -------------------------------------------------------------

    protected static void BlindsOf(string value, out int small, out int big)
    {
        small = 5;
        big = 10;
        int dash = value.IndexOf("-");
        if (dash <= 0)
            return;
        string a = value.Substring(0, dash);
        string b = value.Substring(dash + 1, value.Length() - dash - 1);
        int sa = a.ToInt();
        int sb = b.ToInt();
        if (sa < 1 || sb < sa)
            return;
        small = sa;
        big = sb;
    }

    protected bool Active(int i)
    {
        OZG_PokerSeat s = m_S.SeatList[i];
        return s.Folded == 0;
    }

    // The next seat after `from` still in the hand; -1 for none.
    protected int NextIn(int from)
    {
        int n = m_S.SeatList.Count();
        for (int k = 1; k <= n; k++)
        {
            int j = (from + k) % n;
            if (Active(j))
                return j;
        }
        return -1;
    }

    protected int InCount()
    {
        int c = 0;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (Active(i))
                c++;
        }
        return c;
    }

    // Seats that may still bet: in the hand and not all-in.
    protected int ActingCount()
    {
        int c = 0;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            if (s.Folded == 0 && s.AllIn == 0)
                c++;
        }
        return c;
    }

    protected bool NeedsToAct(int i)
    {
        OZG_PokerSeat s = m_S.SeatList[i];
        if (s.Folded == 1 || s.AllIn == 1)
            return false;
        if (s.Acted == 0)
            return true;
        return s.BetNow < m_S.CurrentBet;
    }

    protected void PutIn(int i, int amount)
    {
        OZG_PokerSeat s = m_S.SeatList[i];
        int pay = amount;
        if (pay > s.Chips)
            pay = s.Chips;
        if (pay < 0)
            pay = 0;
        s.Chips = s.Chips - pay;
        s.BetNow = s.BetNow + pay;
        s.Put = s.Put + pay;
        if (s.Chips == 0)
            s.AllIn = 1;
    }

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        int n = seats.Count();
        if (n < MinPlayers() || n > MaxPlayers())
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        OZG_PokerState s = new OZG_PokerState();
        s.Version = OZG_PokerConst.VERSION;
        int small;
        int big;
        BlindsOf(OZG_Game.OptionOf(options, OZG_PokerConst.OPT_BLINDS, "5-10"), small, big);
        s.SmallBlind = small;
        s.BigBlind = big;

        int i;
        int with = 0;
        for (i = 0; i < n; i++)
        {
            OZG_PokerSeat seat = new OZG_PokerSeat();
            seat.SeatNo = seats[i];
            seat.Chips = StakeFor(seats[i]);
            // A seat with no chips sits this hand out.
            if (seat.Chips <= 0)
                seat.Folded = 1;
            else
                with++;
            seat.CanRaise = 1;
            s.SeatList.Insert(seat);
        }
        if (with < 2)
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }

        array<string> deck = new array<string>();
        DeckOf(deck, s.Dealt);
        if (deck.Count() < with * 2 + 5)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        dice.Shuffle(deck);
        s.StockCards.Copy(deck);
        m_S = s;

        // The button: the seat after the one that had it, else the host's.
        int was = OZG_Game.HintInt(hint, OZG_PokerConst.HINT_BUTTON, -1);
        int b = -1;
        if (was >= 0)
        {
            int at = seats.Find(was);
            if (at >= 0)
            {
                b = NextIn(at);
            }
            else
            {
                for (i = 0; i < n; i++)
                {
                    if (seats[i] > was && Active(i))
                    {
                        b = i;
                        break;
                    }
                }
            }
        }
        if (b < 0)
        {
            b = seats.Find(OZG_Game.HintInt(hint, OZG_Const.HINT_HOST, -1));
            if (b < 0 || !Active(b))
                b = NextIn(n - 1);
        }
        if (b < 0)
            b = 0;
        s.Button = b;

        for (i = 0; i < n; i++)
        {
            if (Active(i))
                Draw(s.StockCards, s.SeatList[i].Held, 2);
            SortBySuit(s.SeatList[i].Held, "shdc");
        }

        // The blinds; heads up the button posts the small one.
        int sb = NextIn(b);
        if (with == 2)
            sb = b;
        int bb = NextIn(sb);
        PutIn(sb, small);
        s.SeatList[sb].Last = OZG_Words.W1("STR_OZG_POKER_LAST_SB", OZG_Words.NumArg(s.SeatList[sb].BetNow));
        PutIn(bb, big);
        s.SeatList[bb].Last = OZG_Words.W1("STR_OZG_POKER_LAST_BB", OZG_Words.NumArg(s.SeatList[bb].BetNow));
        s.CurrentBet = s.SeatList[sb].BetNow;
        if (s.SeatList[bb].BetNow > s.CurrentBet)
            s.CurrentBet = s.SeatList[bb].BetNow;
        s.MinRaise = big;
        s.StreetNo = OZG_PokerConst.PRE;

        int first = NextIn(bb);
        if (with == 2)
            first = sb;
        s.Turn = -1;
        StartAt(first);
        HearSound(OZG_SoundIds.SHUFFLE);
        HearSound(OZG_SoundIds.DEAL);
        return true;
    }

    // The turn goes to the first seat from `from` on that must act; none
    // left closes the street.
    protected void StartAt(int from)
    {
        int n = m_S.SeatList.Count();
        for (int k = 0; k < n; k++)
        {
            int j = (from + k) % n;
            if (NeedsToAct(j))
            {
                m_S.Turn = j;
                return;
            }
        }
        EndStreet();
    }

    protected void Advance(int from)
    {
        if (InCount() <= 1)
        {
            WinUncontested();
            return;
        }
        StartAt(from + 1);
    }

    protected void EndStreet()
    {
        int i;
        for (i = 0; i < m_S.SeatList.Count(); i++)
            m_S.SeatList[i].BetNow = 0;
        m_S.CurrentBet = 0;
        m_S.MinRaise = m_S.BigBlind;
        m_S.Turn = -1;
        if (m_S.StreetNo >= OZG_PokerConst.RIVER || ActingCount() <= 1)
        {
            RunOut();
            Showdown();
            return;
        }
        m_S.StreetNo = m_S.StreetNo + 1;
        if (m_S.StreetNo == OZG_PokerConst.FLOP)
            Draw(m_S.StockCards, m_S.BoardCards, 3);
        else
            Draw(m_S.StockCards, m_S.BoardCards, 1);
        HearSound(OZG_SoundIds.LAY);
        for (i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            s.Acted = 0;
            s.CanRaise = 1;
            if (s.Folded == 0 && s.AllIn == 0)
                s.Last = "";
        }
        StartAt(NextIn(m_S.Button));
    }

    protected void RunOut()
    {
        int want = 5 - m_S.BoardCards.Count();
        if (want > 0)
        {
            Draw(m_S.StockCards, m_S.BoardCards, want);
            HearSound(OZG_SoundIds.LAY);
        }
        m_S.StreetNo = OZG_PokerConst.RIVER;
    }

    protected int PotTotal()
    {
        int sum = 0;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
            sum = sum + m_S.SeatList[i].Put;
        return sum;
    }

    protected void Finish()
    {
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            m_S.SeatList[i].Put = 0;
            m_S.SeatList[i].BetNow = 0;
        }
        m_S.StreetNo = OZG_PokerConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
        HearSound(OZG_SoundIds.SWEEP);
    }

    // Everybody else folded: the last seat takes the pot unseen.
    protected void WinUncontested()
    {
        int w = NextIn(m_S.Button);
        if (w < 0)
        {
            Finish();
            return;
        }
        int pot = PotTotal();
        OZG_PokerSeat s = m_S.SeatList[w];
        s.Chips = s.Chips + pot;
        s.Won = pot;
        m_S.Results.Insert(OZG_Words.W2("STR_OZG_POKER_TOOK_UNSEEN", OZG_Words.SeatArg(s.SeatNo), OZG_Words.NumArg(pot)));
        Finish();
    }

    protected void Showdown()
    {
        int n = m_S.SeatList.Count();
        int i;
        array<int> left = new array<int>();
        array<int> score = new array<int>();
        for (i = 0; i < n; i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            left.Insert(s.Put);
            int sc = -1;
            if (s.Folded == 0)
            {
                s.Shown = 1;
                array<string> seven = new array<string>();
                seven.Copy(s.Held);
                for (int b = 0; b < m_S.BoardCards.Count(); b++)
                    seven.Insert(m_S.BoardCards[b]);
                sc = BestScore(seven);
            }
            score.Insert(sc);
        }

        // Pot after pot: the smallest stake still in, from every seat.
        int guard = 0;
        while (guard < 32)
        {
            guard++;
            int level = -1;
            for (i = 0; i < n; i++)
            {
                if (m_S.SeatList[i].Folded == 0 && left[i] > 0)
                {
                    if (level < 0 || left[i] < level)
                        level = left[i];
                }
            }
            if (level < 0)
                break;
            int pot = 0;
            array<int> eligible = new array<int>();
            for (i = 0; i < n; i++)
            {
                int take = left[i];
                if (take > level)
                    take = level;
                if (take <= 0)
                    continue;
                pot = pot + take;
                left[i] = left[i] - take;
                if (m_S.SeatList[i].Folded == 0 && take == level)
                    eligible.Insert(i);
            }
            Share(pot, eligible, score);
        }
        // Chips of folded seats beyond every stake still in (none by the
        // rules): to the best hand.
        int spill = 0;
        for (i = 0; i < n; i++)
        {
            spill = spill + left[i];
            left[i] = 0;
        }
        if (spill > 0)
        {
            array<int> all = new array<int>();
            for (i = 0; i < n; i++)
            {
                if (m_S.SeatList[i].Folded == 0)
                    all.Insert(i);
            }
            Share(spill, all, score);
        }
        // A line a winner.
        for (i = 0; i < n; i++)
        {
            OZG_PokerSeat w = m_S.SeatList[i];
            if (w.Won <= 0)
                continue;
            string hand = OZG_Words.KeyArg(HandKey(score[i]));
            m_S.Results.Insert(OZG_Words.W3("STR_OZG_POKER_TOOK", OZG_Words.SeatArg(w.SeatNo), OZG_Words.NumArg(w.Won), hand));
        }
        Finish();
    }

    // One pot to the best of `eligible`, an odd chip to the first winner
    // after the button.
    protected void Share(int pot, array<int> eligible, array<int> score)
    {
        if (pot <= 0 || eligible.Count() == 0)
            return;
        int best = -1;
        int i;
        for (i = 0; i < eligible.Count(); i++)
        {
            if (score[eligible[i]] > best)
                best = score[eligible[i]];
        }
        array<int> winners = new array<int>();
        int n = m_S.SeatList.Count();
        for (int k = 1; k <= n; k++)
        {
            int j = (m_S.Button + k) % n;
            if (eligible.Find(j) >= 0 && score[j] == best)
                winners.Insert(j);
        }
        if (winners.Count() == 0)
            return;
        int part = pot / winners.Count();
        int odd = pot - part * winners.Count();
        for (i = 0; i < winners.Count(); i++)
        {
            int give = part;
            if (i < odd)
                give = give + 1;
            OZG_PokerSeat s = m_S.SeatList[winners[i]];
            s.Chips = s.Chips + give;
            s.Won = s.Won + give;
        }
    }

    // ---- hands ----------------------------------------------------------------

    // The best five of the cards given, as one number: the higher, the better.
    static int BestScore(array<string> cards)
    {
        int n = cards.Count();
        if (n < 5)
            return ScoreOf(cards);
        int best = -1;
        array<string> five = new array<string>();
        for (int a = 0; a < n; a++)
        {
            for (int b = a + 1; b < n; b++)
            {
                for (int c = b + 1; c < n; c++)
                {
                    for (int d = c + 1; d < n; d++)
                    {
                        for (int e = d + 1; e < n; e++)
                        {
                            five.Clear();
                            five.Insert(cards[a]);
                            five.Insert(cards[b]);
                            five.Insert(cards[c]);
                            five.Insert(cards[d]);
                            five.Insert(cards[e]);
                            int sc = ScoreOf(five);
                            if (sc > best)
                                best = sc;
                        }
                    }
                }
            }
        }
        return best;
    }

    // Five cards (or fewer): the kind of hand times 15^5 and five tie-breakers
    // of 2..14 in base 15.
    static int ScoreOf(array<string> cards)
    {
        int n = cards.Count();
        array<int> ranks = new array<int>();
        int i;
        int j;
        bool flush = n == 5;
        string suit = "";
        for (i = 0; i < n; i++)
        {
            ranks.Insert(OZG_Cards.RankOf(cards[i]) + 2);
            string su = OZG_Cards.SuitOf(cards[i]);
            if (i == 0)
                suit = su;
            else if (su != suit)
                flush = false;
        }
        // Highest first.
        for (i = 1; i < n; i++)
        {
            int r = ranks[i];
            j = i - 1;
            while (j >= 0 && ranks[j] < r)
            {
                ranks[j + 1] = ranks[j];
                j--;
            }
            ranks[j + 1] = r;
        }
        // Groups: each rank with its count, by count then rank.
        array<int> gRank = new array<int>();
        array<int> gCount = new array<int>();
        for (i = 0; i < n; i++)
        {
            int at = gRank.Find(ranks[i]);
            if (at < 0)
            {
                gRank.Insert(ranks[i]);
                gCount.Insert(1);
            }
            else
            {
                gCount[at] = gCount[at] + 1;
            }
        }
        for (i = 1; i < gRank.Count(); i++)
        {
            int rr = gRank[i];
            int cc = gCount[i];
            j = i - 1;
            while (j >= 0)
            {
                bool later = gCount[j] < cc;
                if (gCount[j] == cc && gRank[j] < rr)
                    later = true;
                if (!later)
                    break;
                gRank[j + 1] = gRank[j];
                gCount[j + 1] = gCount[j];
                j--;
            }
            gRank[j + 1] = rr;
            gCount[j + 1] = cc;
        }
        int straightHigh = 0;
        if (n == 5 && gRank.Count() == 5)
        {
            if (ranks[0] - ranks[4] == 4)
                straightHigh = ranks[0];
            if (ranks[0] == 14 && ranks[1] == 5 && ranks[4] == 2)
                straightHigh = 5;
        }
        int kind = 0;
        array<int> ties = new array<int>();
        if (straightHigh > 0 && flush)
        {
            kind = 8;
            ties.Insert(straightHigh);
        }
        else if (gCount[0] == 4)
        {
            kind = 7;
            ties.Copy(gRank);
        }
        else if (gCount[0] == 3 && gCount.Count() > 1 && gCount[1] >= 2)
        {
            kind = 6;
            ties.Copy(gRank);
        }
        else if (flush)
        {
            kind = 5;
            ties.Copy(ranks);
        }
        else if (straightHigh > 0)
        {
            kind = 4;
            ties.Insert(straightHigh);
        }
        else if (gCount[0] == 3)
        {
            kind = 3;
            ties.Copy(gRank);
        }
        else if (gCount[0] == 2 && gCount.Count() > 1 && gCount[1] == 2)
        {
            kind = 2;
            ties.Copy(gRank);
        }
        else if (gCount[0] == 2)
        {
            kind = 1;
            ties.Copy(gRank);
        }
        else
        {
            kind = 0;
            ties.Copy(ranks);
        }
        int value = kind;
        for (i = 0; i < 5; i++)
        {
            int t = 0;
            if (i < ties.Count())
                t = ties[i];
            value = value * 15 + t;
        }
        return value;
    }

    static int KindOf(int score)
    {
        if (score < 0)
            return -1;
        return score / 759375;
    }

    static string HandKey(int score)
    {
        int kind = KindOf(score);
        if (kind < 0)
            kind = 0;
        return "STR_OZG_POKER_HAND_" + kind.ToString();
    }

    // ---- moves ---------------------------------------------------------------

    // The least a raise may raise to now, and the most this seat can.
    protected int RaiseLeast()
    {
        if (m_S.CurrentBet == 0)
            return m_S.BigBlind;
        return m_S.CurrentBet + m_S.MinRaise;
    }

    protected int RaiseMost(int i)
    {
        OZG_PokerSeat s = m_S.SeatList[i];
        return s.BetNow + s.Chips;
    }

    // On the wire: `fold`, `check`, `call`, `raise:<to>`, `allin`.
    override bool Act(int seat, string move, out string refusal)
    {
        refusal = "";
        if (!m_S || m_S.Ended == 1)
        {
            refusal = OZG_Const.NO_OVER;
            return false;
        }
        int i = IndexOfSeatNo(seat);
        if (i < 0)
        {
            refusal = OZG_Const.NO_SEAT;
            return false;
        }
        if (i != m_S.Turn)
        {
            refusal = OZG_PokerConst.NO_TURN;
            return false;
        }
        OZG_PokerSeat s = m_S.SeatList[i];
        int toCall = m_S.CurrentBet - s.BetNow;
        string verb = move;
        int amount = 0;
        int colon = move.IndexOf(":");
        if (colon > 0)
        {
            verb = move.Substring(0, colon);
            string number = move.Substring(colon + 1, move.Length() - colon - 1);
            amount = number.ToInt();
        }

        if (verb == "fold")
        {
            s.Folded = 1;
            for (int h = 0; h < s.Held.Count(); h++)
                m_S.Muck.Insert(s.Held[h]);
            s.Held.Clear();
            s.Acted = 1;
            s.Last = OZG_Words.W0("STR_OZG_POKER_LAST_FOLD");
            HearSound(OZG_SoundIds.SWEEP);
            Advance(i);
            return true;
        }
        if (verb == "check")
        {
            if (toCall > 0)
            {
                refusal = OZG_PokerConst.NO_CHECK;
                return false;
            }
            s.Acted = 1;
            s.CanRaise = 0;
            s.Last = OZG_Words.W0("STR_OZG_POKER_LAST_CHECK");
            Advance(i);
            return true;
        }
        if (verb == "call")
        {
            if (toCall <= 0)
            {
                refusal = OZG_Const.NO_MOVE;
                return false;
            }
            PutIn(i, toCall);
            s.Acted = 1;
            s.CanRaise = 0;
            s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_CALL", OZG_Words.NumArg(s.BetNow));
            if (s.AllIn == 1)
                s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_ALLIN", OZG_Words.NumArg(s.BetNow));
            HearSound(OZG_SoundIds.LAY);
            Advance(i);
            return true;
        }
        if (verb == "allin")
        {
            int all = RaiseMost(i);
            if (all <= m_S.CurrentBet)
            {
                PutIn(i, s.Chips);
                s.Acted = 1;
                s.CanRaise = 0;
                s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_ALLIN", OZG_Words.NumArg(s.BetNow));
                HearSound(OZG_SoundIds.LAY);
                Advance(i);
                return true;
            }
            if (s.CanRaise == 0)
            {
                refusal = OZG_PokerConst.NO_RAISE;
                return false;
            }
            RaiseTo(i, all);
            s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_ALLIN", OZG_Words.NumArg(s.BetNow));
            Advance(i);
            return true;
        }
        // `bet:` is what the game offers when nothing is bet in the round yet;
        // it is played as `raise:`, and the log words it as a bet.
        if (verb == "raise" || verb == "bet")
        {
            if (s.CanRaise == 0)
            {
                refusal = OZG_PokerConst.NO_RAISE;
                return false;
            }
            int most = RaiseMost(i);
            if (amount <= m_S.CurrentBet || amount > most)
            {
                refusal = OZG_PokerConst.NO_AMOUNT;
                return false;
            }
            if (amount < RaiseLeast() && amount < most)
            {
                refusal = OZG_PokerConst.NO_SMALL;
                return false;
            }
            bool bet = m_S.CurrentBet == 0;
            RaiseTo(i, amount);
            if (s.AllIn == 1)
                s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_ALLIN", OZG_Words.NumArg(s.BetNow));
            else if (bet)
                s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_BET", OZG_Words.NumArg(s.BetNow));
            else
                s.Last = OZG_Words.W1("STR_OZG_POKER_LAST_RAISE", OZG_Words.NumArg(s.BetNow));
            Advance(i);
            return true;
        }
        refusal = OZG_Const.NO_MOVE;
        return false;
    }

    // Seat i puts in up to `to` this street: a full raise reopens the betting
    // for everybody; a short all-in only asks them to answer it.
    protected void RaiseTo(int i, int to)
    {
        OZG_PokerSeat s = m_S.SeatList[i];
        int by = to - m_S.CurrentBet;
        bool full = by >= m_S.MinRaise;
        if (m_S.CurrentBet == 0)
            full = to >= m_S.BigBlind;
        PutIn(i, to - s.BetNow);
        if (full)
            m_S.MinRaise = by;
        m_S.CurrentBet = s.BetNow;
        for (int k = 0; k < m_S.SeatList.Count(); k++)
        {
            if (k == i)
                continue;
            OZG_PokerSeat o = m_S.SeatList[k];
            if (o.Folded == 1 || o.AllIn == 1)
                continue;
            o.Acted = 0;
            if (full)
                o.CanRaise = 1;
        }
        s.Acted = 1;
        s.CanRaise = 0;
        HearSound(OZG_SoundIds.LAY);
    }

    override int HeldCount(int seat)
    {
        if (!m_S)
            return 0;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return 0;
        return m_S.SeatList[i].Held.Count();
    }

    override bool Waits(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return false;
        int i = IndexOfSeatNo(seat);
        return i >= 0 && i == m_S.Turn;
    }

    override bool Skippable(int seat)
    {
        return Waits(seat);
    }

    // An absent seat checks when it can, else folds.
    override void SkipSeat(int seat)
    {
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        string why;
        if (m_S.CurrentBet - m_S.SeatList[i].BetNow <= 0)
            Act(seat, "check", why);
        else
            Act(seat, "fold", why);
    }

    // Removed from the hand: it folds, and what it put in stays in the pot.
    override void DropSeat(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return;
        OZG_PokerSeat s = m_S.SeatList[i];
        if (s.Folded == 1)
            return;
        if (m_S.Turn == i)
        {
            string why;
            Act(seat, "fold", why);
            return;
        }
        s.Folded = 1;
        for (int h = 0; h < s.Held.Count(); h++)
            m_S.Muck.Insert(s.Held[h]);
        s.Held.Clear();
        s.Last = OZG_Words.W0("STR_OZG_POKER_LAST_FOLD");
        if (InCount() <= 1)
            WinUncontested();
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        OZG_PokerSeat s = m_S.SeatList[i];
        int toCall = m_S.CurrentBet - s.BetNow;
        into.Insert("fold");
        if (toCall <= 0)
            into.Insert("check");
        else
            into.Insert("call");
        int most = RaiseMost(i);
        if (s.CanRaise == 1 && most > m_S.CurrentBet)
        {
            int least = RaiseLeast();
            if (least < most)
                into.Insert(RaiseVerb() + least.ToString());
            into.Insert("allin");
        }
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (!Waits(seat))
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        int i = IndexOfSeatNo(seat);
        OZG_PokerSeat s = m_S.SeatList[i];
        int toCall = m_S.CurrentBet - s.BetNow;
        int roll = dice.Below(100);
        int least = RaiseLeast();
        bool canRaise = s.CanRaise == 1 && RaiseMost(i) > least;
        if (toCall <= 0)
        {
            if (roll < 20 && canRaise)
                return RaiseVerb() + least.ToString();
            return "check";
        }
        if (toCall * 5 <= s.Chips + s.BetNow)
        {
            if (roll < 10 && canRaise)
                return RaiseVerb() + least.ToString();
            if (roll < 85)
                return "call";
            return "fold";
        }
        if (roll < 40)
            return "call";
        return "fold";
    }

    // The verb of a bet or a raise with its colon: a bet while nothing is bet in
    // the round.
    protected string RaiseVerb()
    {
        if (m_S.CurrentBet == 0)
            return "bet:";
        return "raise:";
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        int colon = move.IndexOf(":");
        if (colon > 0)
        {
            string head = move.Substring(0, colon);
            if (head == "bet")
                into.Insert("STR_OZG_POKER_LOG_BET");
            else
                into.Insert("STR_OZG_POKER_LOG_RAISE");
            into.Insert(move.Substring(colon + 1, move.Length() - colon - 1));
            return;
        }
        string key = "STR_OZG_POKER_LOG_" + move;
        key.ToUpper();
        into.Insert(key);
    }

    override bool IsOver()
    {
        if (!m_S)
            return true;
        return m_S.Ended == 1;
    }

    override string Outcome()
    {
        if (!m_S || m_S.Ended == 0)
            return "";
        return OZG_PokerConst.HINT_BUTTON + "=" + SeatNoAt(m_S.Button).ToString();
    }

    override int StackOf(int seat)
    {
        if (!m_S)
            return -1;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return -1;
        OZG_PokerSeat s = m_S.SeatList[i];
        return s.Chips + s.Put;
    }

    // ---- the view -------------------------------------------------------------

    override string PublicView()
    {
        if (!m_S)
            return "{}";
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = OZG_PokerConst.GAME_ID;
        v.Trump = "";
        int i;
        for (i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            OZG_GSeat g = SeatShown(i);
            g.Role = s.Last;
            if (s.Won > 0)
                g.Role = OZG_Words.W1("STR_OZG_POKER_ROLE_WON", OZG_Words.NumArg(s.Won));
            if (s.BetNow > 0)
                g.Line = OZG_Words.W2("STR_OZG_G_LINE_CHIPS_BET", OZG_Words.NumArg(s.Chips), OZG_Words.NumArg(s.BetNow));
            else
                g.Line = OZG_Words.W1("STR_OZG_G_LINE_CHIPS", OZG_Words.NumArg(s.Chips));
            if (i == m_S.Button)
                g.Mark = 1;
            if (s.Folded == 1)
                g.Out = 1;
            v.SeatList.Insert(g);
            if (Waits(s.SeatNo))
                v.Awaited.Insert(s.SeatNo);
        }
        // Only the cards dealt to the board lie on it: before the flop, none.
        OZG_GZone board = ZoneOf("board", OZG_Words.W0("STR_OZG_POKER_BOARD"), m_S.BoardCards, null);
        v.Zones.Insert(board);
        OZG_GZone pot = ZoneOf("pot", OZG_Words.W0("STR_OZG_POKER_POT"), null, null);
        pot.Line = OZG_Words.W1("STR_OZG_G_N", OZG_Words.NumArg(PotTotal()));
        v.Zones.Insert(pot);
        v.Score.Copy(m_S.Results);
        if (m_S.Ended == 1)
        {
            v.Ended = 1;
            if (m_S.Results.Count() > 0)
                v.Result = m_S.Results[0];
            v.Status = v.Result;
        }
        else if (m_S.Turn >= 0)
        {
            v.Status = OZG_Words.W1("STR_OZG_POKER_ST_TURN", OZG_Words.SeatArg(SeatNoAt(m_S.Turn)));
        }
        return PublicText(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        if (!m_S)
            return PrivateText(v);
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return PrivateText(v);
        OwnHand(i, v);
        OZG_PokerSeat s = m_S.SeatList[i];
        if (s.Held.Count() == 2 && m_S.BoardCards.Count() >= 3)
        {
            array<string> seven = new array<string>();
            seven.Copy(s.Held);
            for (int b = 0; b < m_S.BoardCards.Count(); b++)
                seven.Insert(m_S.BoardCards[b]);
            v.Status = OZG_Words.W1("STR_OZG_POKER_YOU", OZG_Words.KeyArg(HandKey(BestScore(seven))));
        }
        if (!Waits(seat))
            return PrivateText(v);
        int toCall = m_S.CurrentBet - s.BetNow;
        OZG_GButton fold = OZG_GButton.Make("fold", OZG_Words.W0("STR_OZG_POKER_BTN_FOLD"), -1, 0);
        v.Buttons.Insert(fold);
        if (toCall <= 0)
        {
            OZG_GButton check = OZG_GButton.Make("check", OZG_Words.W0("STR_OZG_POKER_BTN_CHECK"), -1, 1);
            check.Hot = 1;
            v.Buttons.Insert(check);
        }
        else
        {
            int pay = toCall;
            if (pay > s.Chips)
                pay = s.Chips;
            OZG_GButton call = OZG_GButton.Make("call", OZG_Words.W1("STR_OZG_POKER_BTN_CALL", OZG_Words.NumArg(pay)), -1, 1);
            call.Hot = 1;
            v.Buttons.Insert(call);
        }
        int most = RaiseMost(i);
        if (s.CanRaise == 1 && most > m_S.CurrentBet)
        {
            v.Buttons.Insert(OZG_GButton.Make("allin", OZG_Words.W1("STR_OZG_POKER_BTN_ALLIN", OZG_Words.NumArg(most)), -1, 2));
            int least = RaiseLeast();
            if (least < most)
            {
                v.AmountMove = RaiseVerb();
                if (m_S.CurrentBet == 0)
                    v.AmountLabel = OZG_Words.W0("STR_OZG_POKER_BTN_BET");
                else
                    v.AmountLabel = OZG_Words.W0("STR_OZG_POKER_BTN_RAISE");
                v.AmountMin = least;
                v.AmountMax = most;
                v.AmountStart = least;
            }
        }
        return PrivateText(v);
    }

    // ---- saving and checking --------------------------------------------------

    override string SaveState()
    {
        if (!m_S)
            return "";
        string text;
        string err;
        JsonFileLoader<OZG_PokerState>.MakeData(m_S, text, err, false);
        return text;
    }

    override bool LoadState(string data)
    {
        OZG_PokerState s = new OZG_PokerState();
        string err;
        if (!JsonFileLoader<OZG_PokerState>.LoadData(data, s, err))
            return false;
        if (!s || s.Version != OZG_PokerConst.VERSION)
            return false;
        if (!s.SeatList || !s.StockCards || !s.BoardCards || !s.Muck || !s.Dealt)
            return false;
        if (!s.Results)
            s.Results = new array<string>();
        for (int i = 0; i < s.SeatList.Count(); i++)
        {
            if (!s.SeatList[i] || !s.SeatList[i].Held)
                return false;
        }
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        OZG_CardGame.Remade(s.Dealt, made);
        s.Dealt = made;
        OZG_PokerState before = m_S;
        m_S = s;
        string why;
        if (!Sound(why))
        {
            m_S = before;
            return false;
        }
        return true;
    }

    override bool Sound(out string why)
    {
        why = "";
        if (!m_S)
        {
            why = "no deal";
            return false;
        }
        int n = m_S.SeatList.Count();
        if (n < MinPlayers() || n > MaxPlayers())
        {
            why = "seats: " + n.ToString();
            return false;
        }
        if (m_S.Button < 0 || m_S.Button >= n)
        {
            why = "the button is no seat";
            return false;
        }
        if (m_S.BoardCards.Count() > 5)
        {
            why = "a board of " + m_S.BoardCards.Count().ToString();
            return false;
        }
        for (int i = 0; i < n; i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            if (s.Chips < 0 || s.BetNow < 0 || s.Put < s.BetNow)
            {
                why = "chips out of order at seat " + s.SeatNo.ToString();
                return false;
            }
            if (s.Folded == 0 && m_S.Ended == 0 && s.Held.Count() != 2)
            {
                why = "seat " + s.SeatNo.ToString() + " holds " + s.Held.Count().ToString();
                return false;
            }
        }
        if (m_S.Ended == 0 && (m_S.Turn < 0 || m_S.Turn >= n))
        {
            why = "nobody's turn";
            return false;
        }
        return CardsSound(why);
    }

    override string Brief()
    {
        if (!m_S)
            return "poker: no deal";
        string text = "poker " + m_S.SmallBlind.ToString() + "/" + m_S.BigBlind.ToString();
        text = text + " street=" + m_S.StreetNo.ToString();
        text = text + " button=" + SeatNoAt(m_S.Button).ToString();
        text = text + " board=" + OZG_Cards.Line(m_S.BoardCards);
        text = text + " bet=" + m_S.CurrentBet.ToString() + "+" + m_S.MinRaise.ToString();
        text = text + " pot=" + PotTotal().ToString();
        if (m_S.Ended == 1)
            text = text + " over";
        else
            text = text + " turn=" + SeatNoAt(m_S.Turn).ToString();
        text = text + " seats=";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            if (i > 0)
                text = text + ",";
            text = text + s.SeatNo.ToString() + ":" + s.Chips.ToString();
            text = text + "/" + s.BetNow.ToString() + "/" + s.Put.ToString();
            if (s.Folded == 1)
                text = text + "f";
            if (s.AllIn == 1)
                text = text + "a";
        }
        return text;
    }

    override string Everything()
    {
        if (!m_S)
            return "poker: no deal";
        string text = "";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_PokerSeat s = m_S.SeatList[i];
            text = text + s.SeatNo.ToString() + "[" + OZG_Cards.Line(s.Held) + "] ";
        }
        text = text + "board[" + OZG_Cards.Line(m_S.BoardCards) + "] ";
        text = text + "stock[" + OZG_Cards.Line(m_S.StockCards) + "]";
        return text;
    }
}
