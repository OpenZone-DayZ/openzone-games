// Ochko, twenty-one with a bank (stakes-and-games design §5). The rules and
// nothing else. A deal is one round against the bank; the bank and its banker
// pass from deal to deal through the hint.
class OZG_OchkoConst
{
    static const string GAME_ID = "ochko";
    static const int VERSION = 1;
    static const int MAX_SEATS = 8;
    // The lowest rank of the 36 cards.
    static const string LOWEST = "6";

    // The steps of a round.
    static const int STEP_OPEN   = 0;   // the banker puts up a bank, keeps it or closes it
    static const int STEP_BETS   = 1;   // each player in turn bets and takes cards
    static const int STEP_DEALER = 2;   // the banker takes his own cards
    static const int STEP_DONE   = 3;

    // A seat in a round.
    static const int P_WAIT  = 0;   // its turn has not come
    static const int P_PLAY  = 1;   // bet, taking cards
    static const int P_STOOD = 2;
    static const int P_BUST  = 3;
    static const int P_SKIP  = 4;
    static const int P_WON   = 5;
    static const int P_LOST  = 6;
    static const int P_OUT   = 7;   // dropped

    static const string HINT_BANKER = "banker";
    static const string HINT_BANK   = "bank";
    static const string HINT_ROUNDS = "rounds";

    static const string NO_TURN   = "STR_OZG_G_NO_TURN";
    static const string NO_AMOUNT = "STR_OZG_G_NO_AMOUNT";
    static const string NO_STOCK  = "STR_OZG_OCHKO_NO_STOCK";
    static const string NO_CLOSE  = "STR_OZG_OCHKO_NO_CLOSE";
}

class OZG_OchkoSeat
{
    int SeatNo;
    ref array<string> Held;
    // Chips behind: not in the bank, not bet.
    int Chips;
    int BetNow;
    int Phase;
    // 1: its cards lie face up.
    int Shown;
    // What this round brought it: won (+) or lost (-).
    int Won;

    void OZG_OchkoSeat()
    {
        Held = new array<string>();
    }
}

class OZG_OchkoState
{
    int Version;
    ref array<ref OZG_OchkoSeat> SeatList;
    ref array<string> StockCards;
    // The banker's index, the chips in the bank, and how many rounds he has
    // banked with this bank before this one.
    int Banker;
    int BankChips;
    int BankRounds;
    int Turn;
    int StepNo;
    int Ended;
    // 1: the bank went to nothing this round.
    int Broken;
    ref array<ref OZG_DealCard> Dealt;

    void OZG_OchkoState()
    {
        SeatList = new array<ref OZG_OchkoSeat>();
        StockCards = new array<string>();
        Dealt = new array<ref OZG_DealCard>();
        Turn = -1;
    }
}

class OZG_Ochko : OZG_CardGame
{
    protected ref OZG_OchkoState m_S;
    protected ref map<int, int> m_Stakes;

    override string GameId()
    {
        return OZG_OchkoConst.GAME_ID;
    }

    override string LabelKey()
    {
        return "STR_OZG_GAME_OCHKO";
    }

    override int MinPlayers()
    {
        return 2;
    }

    override int MaxPlayers()
    {
        return OZG_OchkoConst.MAX_SEATS;
    }

    override void CardsUsed(array<string> into)
    {
        OZG_Cards.FromRank(OZG_OchkoConst.LOWEST, into);
    }

    override OZG_Game Fresh()
    {
        return new OZG_Ochko();
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

    protected bool Open(int i)
    {
        OZG_OchkoSeat s = m_S.SeatList[i];
        if (s.Shown == 1)
            return true;
        return i == m_S.Banker;
    }

    override protected void HiddenAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (Open(i))
            return;
        into.Copy(m_S.SeatList[i].Held);
    }

    override protected void ShownAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (!Open(i))
            return;
        into.Copy(m_S.SeatList[i].Held);
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
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (int k = 0; k < held.Count(); k++)
                into.Insert(held[k]);
        }
        for (int t = 0; t < m_S.StockCards.Count(); t++)
            into.Insert(m_S.StockCards[t]);
    }

    // ---- points ---------------------------------------------------------------

    static int PointsOf(string card)
    {
        int r = OZG_Cards.RankOf(card);
        if (r < 0)
            return 0;
        if (r <= 8)
            return r + 2;
        if (r == 9)
            return 2;
        if (r == 10)
            return 3;
        if (r == 11)
            return 4;
        return 11;
    }

    static int PointsOfHand(array<string> held)
    {
        int total = 0;
        for (int i = 0; i < held.Count(); i++)
            total = total + PointsOf(held[i]);
        return total;
    }

    // Two aces and nothing else: the golden ochko.
    static bool Golden(array<string> held)
    {
        if (held.Count() != 2)
            return false;
        return OZG_Cards.RankOf(held[0]) == 12 && OZG_Cards.RankOf(held[1]) == 12;
    }

    // ---- the deal -------------------------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        int n = seats.Count();
        if (n < MinPlayers() || n > MaxPlayers())
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        OZG_OchkoState s = new OZG_OchkoState();
        s.Version = OZG_OchkoConst.VERSION;
        array<string> deck = new array<string>();
        DeckOf(deck, s.Dealt);
        if (deck.Count() < n * 2)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        dice.Shuffle(deck);
        s.StockCards.Copy(deck);

        int i;
        for (i = 0; i < n; i++)
        {
            OZG_OchkoSeat seat = new OZG_OchkoSeat();
            seat.SeatNo = seats[i];
            seat.Chips = StakeFor(seats[i]);
            s.SeatList.Insert(seat);
        }

        // The banker and his bank, from the round before; the bank was part of
        // his chips as the session counts them (StackOf).
        int named = OZG_Game.HintInt(hint, OZG_OchkoConst.HINT_BANKER, -1);
        int bank = OZG_Game.HintInt(hint, OZG_OchkoConst.HINT_BANK, 0);
        int rounds = OZG_Game.HintInt(hint, OZG_OchkoConst.HINT_ROUNDS, 0);
        int b = seats.Find(named);
        if (b < 0)
        {
            bank = 0;
            rounds = 0;
            b = seats.Find(OZG_Game.HintInt(hint, OZG_Const.HINT_HOST, -1));
            if (b < 0)
                b = 0;
        }
        OZG_OchkoSeat banker = s.SeatList[b];
        if (bank > banker.Chips)
            bank = banker.Chips;
        if (bank < 0)
            bank = 0;
        banker.Chips = banker.Chips - bank;
        s.Banker = b;
        s.BankChips = bank;
        s.BankRounds = rounds;
        s.StepNo = OZG_OchkoConst.STEP_OPEN;
        s.Turn = b;
        m_S = s;
        PassIfStuck();
        HearSound(OZG_SoundIds.SHUFFLE);
        return true;
    }

    // The next seat after `from` with chips; -1 when nobody has any.
    protected int NextWithChips(int from)
    {
        int n = m_S.SeatList.Count();
        for (int k = 1; k <= n; k++)
        {
            int j = (from + k) % n;
            OZG_OchkoSeat s = m_S.SeatList[j];
            if (s.Phase == OZG_OchkoConst.P_OUT)
                continue;
            if (s.Chips > 0)
                return j;
        }
        return -1;
    }

    // The bank passes to the next seat with chips.
    protected void NextBanker()
    {
        int next = NextWithChips(m_S.Banker);
        if (next < 0)
        {
            m_S.Ended = 1;
            m_S.StepNo = OZG_OchkoConst.STEP_DONE;
            m_S.Turn = -1;
            return;
        }
        m_S.Banker = next;
        m_S.BankChips = 0;
        m_S.BankRounds = 0;
        m_S.Turn = next;
        m_S.StepNo = OZG_OchkoConst.STEP_OPEN;
    }

    // A banker with no bank and no chips cannot bank.
    protected void PassIfStuck()
    {
        if (m_S.Ended == 1 || m_S.StepNo != OZG_OchkoConst.STEP_OPEN)
            return;
        OZG_OchkoSeat b = m_S.SeatList[m_S.Banker];
        bool stuck = m_S.BankChips <= 0 && b.Chips <= 0;
        if (b.Phase == OZG_OchkoConst.P_OUT)
            stuck = true;
        if (stuck)
            NextBanker();
    }

    // What the players still in this round have bet, not counting `but`.
    protected int Outstanding(int but)
    {
        int sum = 0;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (i == but)
                continue;
            OZG_OchkoSeat s = m_S.SeatList[i];
            if (s.Phase == OZG_OchkoConst.P_PLAY || s.Phase == OZG_OchkoConst.P_STOOD)
                sum = sum + s.BetNow;
        }
        return sum;
    }

    // The most seat i may bet now.
    protected int BetMost(int i)
    {
        int left = m_S.BankChips - Outstanding(i);
        int chips = m_S.SeatList[i].Chips;
        if (chips < left)
            left = chips;
        if (left < 0)
            left = 0;
        return left;
    }

    // The next player after `from` whose turn has not come and who can bet;
    // one who cannot sits the round out. -1: nobody is left.
    protected int NextBettor(int from)
    {
        int n = m_S.SeatList.Count();
        for (int k = 1; k < n; k++)
        {
            int j = (from + k) % n;
            if (j == m_S.Banker)
                continue;
            OZG_OchkoSeat s = m_S.SeatList[j];
            if (s.Phase != OZG_OchkoConst.P_WAIT)
                continue;
            if (BetMost(j) < 1 || m_S.StockCards.Count() == 0)
            {
                s.Phase = OZG_OchkoConst.P_SKIP;
                continue;
            }
            return j;
        }
        return -1;
    }

    protected void StartRound()
    {
        m_S.StepNo = OZG_OchkoConst.STEP_BETS;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            if (s.Phase != OZG_OchkoConst.P_OUT)
                s.Phase = OZG_OchkoConst.P_WAIT;
            s.BetNow = 0;
            s.Won = 0;
            s.Shown = 0;
        }
        AfterPlayer(m_S.Banker);
    }

    // The next player, or the banker's own cards, or the end.
    protected void AfterPlayer(int from)
    {
        int next = NextBettor(from);
        if (next >= 0)
        {
            m_S.Turn = next;
            return;
        }
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (m_S.SeatList[i].Phase == OZG_OchkoConst.P_STOOD)
            {
                m_S.StepNo = OZG_OchkoConst.STEP_DEALER;
                m_S.Turn = m_S.Banker;
                return;
            }
        }
        EndRound();
    }

    protected void EndRound()
    {
        for (int i = 0; i < m_S.SeatList.Count(); i++)
            m_S.SeatList[i].Shown = 1;
        m_S.StepNo = OZG_OchkoConst.STEP_DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
        if (m_S.BankChips <= 0)
        {
            m_S.BankChips = 0;
            m_S.Broken = 1;
        }
        HearSound(OZG_SoundIds.SWEEP);
    }

    // A player won his bet back with as much again out of the bank.
    protected void PayOut(int i)
    {
        OZG_OchkoSeat s = m_S.SeatList[i];
        s.Chips = s.Chips + s.BetNow * 2;
        m_S.BankChips = m_S.BankChips - s.BetNow;
        s.Won = s.BetNow;
        s.BetNow = 0;
        s.Phase = OZG_OchkoConst.P_WON;
        s.Shown = 1;
    }

    // A player's bet went into the bank.
    protected void TakeIn(int i, int phase)
    {
        OZG_OchkoSeat s = m_S.SeatList[i];
        m_S.BankChips = m_S.BankChips + s.BetNow;
        s.Won = -s.BetNow;
        s.BetNow = 0;
        s.Phase = phase;
        s.Shown = 1;
    }

    protected void DealerBusts()
    {
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (m_S.SeatList[i].Phase == OZG_OchkoConst.P_STOOD)
                PayOut(i);
        }
        EndRound();
    }

    protected void DealerStands()
    {
        array<string> mine = m_S.SeatList[m_S.Banker].Held;
        int bank = PointsOfHand(mine);
        bool bankGold = Golden(mine);
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            if (s.Phase != OZG_OchkoConst.P_STOOD)
                continue;
            bool gold = Golden(s.Held);
            bool wins = false;
            if (gold && !bankGold)
                wins = true;
            if (!gold && !bankGold && PointsOfHand(s.Held) > bank)
                wins = true;
            if (wins)
                PayOut(i);
            else
                TakeIn(i, OZG_OchkoConst.P_LOST);
        }
        EndRound();
    }

    // ---- moves ---------------------------------------------------------------

    // On the wire: `bank:<n>`, `go`, `close`, `pass` (the banker before a
    // round); `bet:<n>`, `skip`, `hit`, `stand` (a player); `hit`, `stand`
    // (the banker at the end).
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
            refusal = OZG_OchkoConst.NO_TURN;
            return false;
        }
        string verb = move;
        int amount = 0;
        int colon = move.IndexOf(":");
        if (colon > 0)
        {
            verb = move.Substring(0, colon);
            string number = move.Substring(colon + 1, move.Length() - colon - 1);
            amount = number.ToInt();
        }
        OZG_OchkoSeat s = m_S.SeatList[i];
        int step = m_S.StepNo;

        if (step == OZG_OchkoConst.STEP_OPEN)
        {
            if (verb == "bank" && m_S.BankChips == 0)
            {
                if (amount < 1 || amount > s.Chips)
                {
                    refusal = OZG_OchkoConst.NO_AMOUNT;
                    return false;
                }
                s.Chips = s.Chips - amount;
                m_S.BankChips = amount;
                m_S.BankRounds = 0;
                StartRound();
                return true;
            }
            if (verb == "go" && m_S.BankChips > 0)
            {
                StartRound();
                return true;
            }
            if (verb == "close" && m_S.BankChips > 0)
            {
                if (m_S.BankRounds < 1)
                {
                    refusal = OZG_OchkoConst.NO_CLOSE;
                    return false;
                }
                s.Chips = s.Chips + m_S.BankChips;
                m_S.BankChips = 0;
                NextBanker();
                PassIfStuck();
                return true;
            }
            if (verb == "pass" && m_S.BankChips == 0)
            {
                NextBanker();
                PassIfStuck();
                return true;
            }
            refusal = OZG_Const.NO_MOVE;
            return false;
        }

        if (step == OZG_OchkoConst.STEP_BETS)
        {
            if (s.Phase == OZG_OchkoConst.P_WAIT)
            {
                if (verb == "skip")
                {
                    s.Phase = OZG_OchkoConst.P_SKIP;
                    AfterPlayer(i);
                    return true;
                }
                if (verb == "bet")
                {
                    if (amount < 1 || amount > BetMost(i))
                    {
                        refusal = OZG_OchkoConst.NO_AMOUNT;
                        return false;
                    }
                    if (m_S.StockCards.Count() == 0)
                    {
                        refusal = OZG_OchkoConst.NO_STOCK;
                        return false;
                    }
                    s.Chips = s.Chips - amount;
                    s.BetNow = amount;
                    s.Phase = OZG_OchkoConst.P_PLAY;
                    Draw(m_S.StockCards, s.Held, 1);
                    HearSound(OZG_SoundIds.DRAW);
                    return true;
                }
                refusal = OZG_Const.NO_MOVE;
                return false;
            }
            if (s.Phase == OZG_OchkoConst.P_PLAY)
            {
                if (verb == "stand")
                {
                    s.Phase = OZG_OchkoConst.P_STOOD;
                    AfterPlayer(i);
                    return true;
                }
                if (verb == "hit")
                {
                    if (m_S.StockCards.Count() == 0)
                    {
                        refusal = OZG_OchkoConst.NO_STOCK;
                        return false;
                    }
                    Draw(m_S.StockCards, s.Held, 1);
                    HearSound(OZG_SoundIds.DRAW);
                    AfterHit(i);
                    return true;
                }
            }
            refusal = OZG_Const.NO_MOVE;
            return false;
        }

        if (step == OZG_OchkoConst.STEP_DEALER)
        {
            if (verb == "hit")
            {
                if (m_S.StockCards.Count() == 0)
                {
                    refusal = OZG_OchkoConst.NO_STOCK;
                    return false;
                }
                Draw(m_S.StockCards, s.Held, 1);
                HearSound(OZG_SoundIds.DRAW);
                AfterDealerHit();
                return true;
            }
            if (verb == "stand" && s.Held.Count() > 0)
            {
                DealerStands();
                return true;
            }
        }
        refusal = OZG_Const.NO_MOVE;
        return false;
    }

    protected void AfterHit(int i)
    {
        OZG_OchkoSeat s = m_S.SeatList[i];
        if (Golden(s.Held))
        {
            s.Phase = OZG_OchkoConst.P_STOOD;
            AfterPlayer(i);
            return;
        }
        int points = PointsOfHand(s.Held);
        if (points > 21)
        {
            TakeIn(i, OZG_OchkoConst.P_BUST);
            AfterPlayer(i);
            return;
        }
        if (points == 21)
        {
            s.Phase = OZG_OchkoConst.P_STOOD;
            AfterPlayer(i);
        }
    }

    protected void AfterDealerHit()
    {
        array<string> mine = m_S.SeatList[m_S.Banker].Held;
        if (Golden(mine))
        {
            DealerStands();
            return;
        }
        int points = PointsOfHand(mine);
        if (points > 21)
        {
            DealerBusts();
            return;
        }
        if (points == 21 || m_S.StockCards.Count() == 0)
            DealerStands();
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

    // An absent seat: the banker keeps his bank or passes it on and stops
    // with what he has; a player sits out or stops.
    override void SkipSeat(int seat)
    {
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        OZG_OchkoSeat s = m_S.SeatList[i];
        string why;
        if (m_S.StepNo == OZG_OchkoConst.STEP_OPEN)
        {
            if (m_S.BankChips > 0)
                Act(seat, "go", why);
            else
                Act(seat, "pass", why);
            return;
        }
        if (m_S.StepNo == OZG_OchkoConst.STEP_BETS)
        {
            if (s.Phase == OZG_OchkoConst.P_WAIT)
                Act(seat, "skip", why);
            else
                Act(seat, "stand", why);
            return;
        }
        if (m_S.StepNo == OZG_OchkoConst.STEP_DEALER)
        {
            if (s.Held.Count() == 0 && m_S.StockCards.Count() > 0)
                Act(seat, "hit", why);
            if (m_S.Ended == 0)
                DealerStands();
        }
    }

    override void DropSeat(int seat)
    {
        if (!m_S)
            return;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return;
        OZG_OchkoSeat s = m_S.SeatList[i];
        int k;
        if (m_S.Ended == 1)
        {
            s.Phase = OZG_OchkoConst.P_OUT;
            return;
        }
        if (i == m_S.Banker)
        {
            // The round is void: every bet goes back, the bank to its banker.
            for (k = 0; k < m_S.SeatList.Count(); k++)
            {
                OZG_OchkoSeat p = m_S.SeatList[k];
                p.Chips = p.Chips + p.BetNow;
                p.BetNow = 0;
            }
            s.Chips = s.Chips + m_S.BankChips;
            m_S.BankChips = 0;
            s.Phase = OZG_OchkoConst.P_OUT;
            if (m_S.StepNo == OZG_OchkoConst.STEP_OPEN)
            {
                NextBanker();
                PassIfStuck();
                return;
            }
            EndRound();
            return;
        }
        if (s.Phase == OZG_OchkoConst.P_PLAY || s.Phase == OZG_OchkoConst.P_STOOD)
            TakeIn(i, OZG_OchkoConst.P_OUT);
        else
            s.Phase = OZG_OchkoConst.P_OUT;
        if (m_S.Turn == i)
        {
            if (m_S.StepNo == OZG_OchkoConst.STEP_BETS)
                AfterPlayer(i);
        }
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        OZG_OchkoSeat s = m_S.SeatList[i];
        bool stock = m_S.StockCards.Count() > 0;
        if (m_S.StepNo == OZG_OchkoConst.STEP_OPEN)
        {
            if (m_S.BankChips == 0)
            {
                if (s.Chips > 0)
                {
                    into.Insert("bank:1");
                    if (s.Chips > 1)
                        into.Insert("bank:" + s.Chips.ToString());
                }
                into.Insert("pass");
                return;
            }
            into.Insert("go");
            if (m_S.BankRounds >= 1)
                into.Insert("close");
            return;
        }
        if (m_S.StepNo == OZG_OchkoConst.STEP_BETS)
        {
            if (s.Phase == OZG_OchkoConst.P_WAIT)
            {
                int most = BetMost(i);
                if (most >= 1 && stock)
                {
                    into.Insert("bet:1");
                    if (most > 1)
                        into.Insert("bet:" + most.ToString());
                }
                into.Insert("skip");
                return;
            }
            if (stock)
                into.Insert("hit");
            into.Insert("stand");
            return;
        }
        if (m_S.StepNo == OZG_OchkoConst.STEP_DEALER)
        {
            if (stock)
                into.Insert("hit");
            if (s.Held.Count() > 0)
                into.Insert("stand");
        }
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (!Waits(seat))
            return "";
        int i = IndexOfSeatNo(seat);
        OZG_OchkoSeat s = m_S.SeatList[i];
        bool stock = m_S.StockCards.Count() > 0;
        int points = PointsOfHand(s.Held);
        if (m_S.StepNo == OZG_OchkoConst.STEP_OPEN)
        {
            if (m_S.BankChips == 0)
            {
                if (s.Chips <= 0)
                    return "pass";
                int put = s.Chips / 4;
                if (put < 1)
                    put = 1;
                return "bank:" + put.ToString();
            }
            return "go";
        }
        if (m_S.StepNo == OZG_OchkoConst.STEP_BETS)
        {
            if (s.Phase == OZG_OchkoConst.P_WAIT)
            {
                int most = BetMost(i);
                if (most < 1 || !stock)
                    return "skip";
                int bet = s.Chips / 10;
                if (bet < 1)
                    bet = 1;
                if (bet > most)
                    bet = most;
                return "bet:" + bet.ToString();
            }
            if (points < 17 && stock)
                return "hit";
            return "stand";
        }
        if (s.Held.Count() == 0 && stock)
            return "hit";
        if (points < 17 && stock)
            return "hit";
        return "stand";
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        string verb = move;
        string arg = "";
        int colon = move.IndexOf(":");
        if (colon > 0)
        {
            verb = move.Substring(0, colon);
            arg = move.Substring(colon + 1, move.Length() - colon - 1);
        }
        if (verb == "bank")
        {
            into.Insert("STR_OZG_OCHKO_LOG_BANK");
            into.Insert(arg);
            return;
        }
        if (verb == "bet")
        {
            into.Insert("STR_OZG_OCHKO_LOG_BET");
            into.Insert(arg);
            return;
        }
        into.Insert("STR_OZG_OCHKO_LOG_" + verb);
        string key = into[0];
        key.ToUpper();
        into[0] = key;
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
        int banker = m_S.Banker;
        int bank = m_S.BankChips;
        int rounds = m_S.BankRounds + 1;
        if (m_S.Broken == 1 || bank <= 0)
        {
            int next = NextWithChips(banker);
            if (next < 0)
                next = banker;
            banker = next;
            bank = 0;
            rounds = 0;
        }
        string text = OZG_OchkoConst.HINT_BANKER + "=" + m_S.SeatList[banker].SeatNo.ToString();
        text = text + ";" + OZG_OchkoConst.HINT_BANK + "=" + bank.ToString();
        text = text + ";" + OZG_OchkoConst.HINT_ROUNDS + "=" + rounds.ToString();
        return text;
    }

    override int StackOf(int seat)
    {
        if (!m_S)
            return -1;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return -1;
        OZG_OchkoSeat s = m_S.SeatList[i];
        int chips = s.Chips + s.BetNow;
        if (i == m_S.Banker)
            chips = chips + m_S.BankChips;
        return chips;
    }

    // ---- the view -------------------------------------------------------------

    protected string RoleWords(int i)
    {
        OZG_OchkoSeat s = m_S.SeatList[i];
        int pts = PointsOfHand(s.Held);
        if (i == m_S.Banker)
        {
            if (s.Held.Count() > 0)
                return OZG_Words.W1("STR_OZG_OCHKO_ROLE_BANKER_PTS", OZG_Words.NumArg(pts));
            return OZG_Words.W0("STR_OZG_OCHKO_ROLE_BANKER");
        }
        int p = s.Phase;
        if (p == OZG_OchkoConst.P_PLAY)
            return OZG_Words.W1("STR_OZG_OCHKO_ROLE_BET", OZG_Words.NumArg(s.BetNow));
        if (p == OZG_OchkoConst.P_STOOD)
            return OZG_Words.W1("STR_OZG_OCHKO_ROLE_STOOD", OZG_Words.NumArg(s.BetNow));
        if (p == OZG_OchkoConst.P_BUST)
            return OZG_Words.W2("STR_OZG_OCHKO_ROLE_BUST", OZG_Words.NumArg(pts), OZG_Words.NumArg(-s.Won));
        if (p == OZG_OchkoConst.P_SKIP)
            return OZG_Words.W0("STR_OZG_OCHKO_ROLE_SKIP");
        if (p == OZG_OchkoConst.P_WON)
            return OZG_Words.W2("STR_OZG_OCHKO_ROLE_WON", OZG_Words.NumArg(pts), OZG_Words.NumArg(s.Won));
        if (p == OZG_OchkoConst.P_LOST)
            return OZG_Words.W2("STR_OZG_OCHKO_ROLE_LOST", OZG_Words.NumArg(pts), OZG_Words.NumArg(-s.Won));
        if (p == OZG_OchkoConst.P_OUT)
            return OZG_Words.W0("STR_OZG_G_ROLE_OUT");
        return "";
    }

    override string PublicView()
    {
        if (!m_S)
            return "{}";
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = OZG_OchkoConst.GAME_ID;
        v.Trump = "";
        int i;
        for (i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            OZG_GSeat g = SeatShown(i);
            g.Role = RoleWords(i);
            g.Line = OZG_Words.W1("STR_OZG_G_LINE_CHIPS", OZG_Words.NumArg(s.Chips));
            if (i == m_S.Banker)
                g.Mark = 1;
            bool gone = s.Phase == OZG_OchkoConst.P_SKIP || s.Phase == OZG_OchkoConst.P_OUT;
            if (gone)
                g.Out = 1;
            v.SeatList.Insert(g);
            if (Waits(s.SeatNo))
                v.Awaited.Insert(s.SeatNo);
        }
        OZG_GZone bank = ZoneOf("bank", OZG_Words.W0("STR_OZG_OCHKO_BANK"), null, null);
        bank.Line = OZG_Words.W1("STR_OZG_G_N", OZG_Words.NumArg(m_S.BankChips));
        v.Zones.Insert(bank);
        OZG_GZone stock = ZoneOf("stock", OZG_Words.W0("STR_OZG_G_STOCK"), null, null);
        stock.Backs = m_S.StockCards.Count();
        if (stock.Backs > 0)
            stock.BackLook = LookOf(m_S.StockCards[0]);
        v.Zones.Insert(stock);

        int bankerNo = m_S.SeatList[m_S.Banker].SeatNo;
        if (m_S.Ended == 1)
        {
            v.Ended = 1;
            if (m_S.Broken == 1)
                v.Result = OZG_Words.W1("STR_OZG_OCHKO_RES_BROKEN", OZG_Words.SeatArg(bankerNo));
            else
                v.Result = OZG_Words.W2("STR_OZG_OCHKO_RES_BANK", OZG_Words.SeatArg(bankerNo), OZG_Words.NumArg(m_S.BankChips));
            v.Status = v.Result;
        }
        else if (m_S.StepNo == OZG_OchkoConst.STEP_OPEN)
        {
            v.Status = OZG_Words.W1("STR_OZG_OCHKO_ST_OPEN", OZG_Words.SeatArg(bankerNo));
        }
        else if (m_S.StepNo == OZG_OchkoConst.STEP_BETS)
        {
            v.Status = OZG_Words.W1("STR_OZG_OCHKO_ST_BET", OZG_Words.SeatArg(SeatNoAt(m_S.Turn)));
        }
        else
        {
            v.Status = OZG_Words.W1("STR_OZG_OCHKO_ST_DEALER", OZG_Words.SeatArg(bankerNo));
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
        OZG_OchkoSeat s = m_S.SeatList[i];
        if (s.Held.Count() > 0)
            v.Status = OZG_Words.W1("STR_OZG_OCHKO_YOU", OZG_Words.NumArg(PointsOfHand(s.Held)));
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int k = 0; k < can.Count(); k++)
        {
            string move = can[k];
            if (move.IndexOf("bank:") == 0)
            {
                v.AmountMove = "bank:";
                v.AmountLabel = OZG_Words.W0("STR_OZG_OCHKO_PUT_BANK");
                v.AmountMin = 1;
                v.AmountMax = s.Chips;
                v.AmountStart = s.Chips / 4;
                if (v.AmountStart < 1)
                    v.AmountStart = 1;
                continue;
            }
            if (move.IndexOf("bet:") == 0)
            {
                v.AmountMove = "bet:";
                v.AmountLabel = OZG_Words.W0("STR_OZG_OCHKO_BET");
                v.AmountMin = 1;
                v.AmountMax = BetMost(i);
                v.AmountStart = 1;
                continue;
            }
            string label = "STR_OZG_OCHKO_BTN_" + move;
            label.ToUpper();
            OZG_GButton b = OZG_GButton.Make(move, OZG_Words.W0(label), -1, k);
            if (move == "go")
                b.Label = OZG_Words.W1("STR_OZG_OCHKO_BTN_GO", OZG_Words.NumArg(m_S.BankChips));
            if (move == "hit" || move == "go")
                b.Hot = 1;
            v.Buttons.Insert(b);
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
        JsonFileLoader<OZG_OchkoState>.MakeData(m_S, text, err, false);
        return text;
    }

    override bool LoadState(string data)
    {
        OZG_OchkoState s = new OZG_OchkoState();
        string err;
        if (!JsonFileLoader<OZG_OchkoState>.LoadData(data, s, err))
            return false;
        if (!s || s.Version != OZG_OchkoConst.VERSION)
            return false;
        if (!s.SeatList || !s.StockCards || !s.Dealt)
            return false;
        for (int i = 0; i < s.SeatList.Count(); i++)
        {
            if (!s.SeatList[i] || !s.SeatList[i].Held)
                return false;
        }
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        OZG_CardGame.Remade(s.Dealt, made);
        s.Dealt = made;
        OZG_OchkoState before = m_S;
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
        if (m_S.Banker < 0 || m_S.Banker >= n)
        {
            why = "the banker is no seat";
            return false;
        }
        if (m_S.BankChips < 0)
        {
            why = "the bank is below nothing";
            return false;
        }
        for (int i = 0; i < n; i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            if (s.Chips < 0 || s.BetNow < 0)
            {
                why = "chips below nothing at seat " + s.SeatNo.ToString();
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
            return "ochko: no deal";
        string text = "ochko step=" + m_S.StepNo.ToString();
        text = text + " banker=" + SeatNoAt(m_S.Banker).ToString();
        text = text + " bank=" + m_S.BankChips.ToString();
        text = text + " rounds=" + m_S.BankRounds.ToString();
        if (m_S.Ended == 1)
            text = text + " over broken=" + m_S.Broken.ToString();
        else
            text = text + " turn=" + SeatNoAt(m_S.Turn).ToString();
        text = text + " stock=" + m_S.StockCards.Count().ToString();
        text = text + " seats=";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            if (i > 0)
                text = text + ",";
            text = text + s.SeatNo.ToString() + ":" + s.Chips.ToString();
            text = text + "/" + s.BetNow.ToString() + "/p" + s.Phase.ToString();
            text = text + "/" + PointsOfHand(s.Held).ToString();
        }
        return text;
    }

    override string Everything()
    {
        if (!m_S)
            return "ochko: no deal";
        string text = "";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_OchkoSeat s = m_S.SeatList[i];
            text = text + s.SeatNo.ToString() + "[" + OZG_Cards.Line(s.Held) + "] ";
        }
        text = text + "stock[" + OZG_Cards.Line(m_S.StockCards) + "]";
        return text;
    }
}
