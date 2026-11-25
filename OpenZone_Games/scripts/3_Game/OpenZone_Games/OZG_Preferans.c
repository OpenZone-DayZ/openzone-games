// Preferans, the Sochi convention, three players (stakes-and-games design §7).
// The rules and the score sheet: the pools, the mountains and the whists pass
// from deal to deal through the hint until every pool holds the target.
class OZG_PrefConst
{
    static const string GAME_ID = "preferans";
    static const int VERSION = 1;
    static const string LOWEST = "7";

    static const string OPT_POOL = "pool";

    static const int AUCTION  = 0;
    static const int TALON    = 1;   // the declarer discards two
    static const int CONTRACT = 2;   // the declarer names his game
    static const int WHIST    = 3;   // the defenders whist or pass
    static const int CHOICE   = 4;   // a lone whister: open or closed
    static const int PLAY     = 5;
    static const int DONE     = 6;

    // Strains from the lowest: spades, clubs, diamonds, hearts, no trumps.
    static const string STRAINS = "SCDHN";
    static const string STRAIN_SUITS = "scdh";
    // The rank of misère between 8 no trumps (28) and 9 spades (30).
    static const int MISERE_RANK = 29;

    static const string HINT_DEALER = "dealer";
    static const string HINT_TARGET = "tg";
    static const string HINT_RUN    = "rr";
    static const string HINT_POOL   = "p";
    static const string HINT_HILL   = "m";
    static const string HINT_WHISTS = "w";

    static const string NO_TURN = "STR_OZG_G_NO_TURN";
    static const string NO_BID  = "STR_OZG_PREF_NO_BID";
    static const string NO_SUIT = "STR_OZG_G_NO_SUIT";
}

class OZG_PrefSeat
{
    int SeatNo;
    ref array<string> Held;
    int Taken;
    // Words: its last word.
    string Said;
    int Passed;
    int Spoke;
    // 0 not asked, 1 whist, 2 pass.
    int Whist;

    void OZG_PrefSeat()
    {
        Held = new array<string>();
    }
}

class OZG_PrefState
{
    int Version;
    ref array<ref OZG_PrefSeat> SeatList;
    // Face down: the talon until the auction ends; the declarer's discards.
    ref array<string> Talon;
    ref array<string> Discards;
    // The talon's two as everybody saw them when it was turned up (shown, not
    // counted: they are in the declarer's hand).
    ref array<string> TalonSeen;
    int Dealer;
    int Phase;
    int Turn;
    int BidTop;
    string BidCode;
    int Bidder;
    int Declarer;
    int Level;
    string Strain;
    int Misere;
    int Raspasy;
    int Open;
    int Whisters;
    int Drops;
    ref array<string> TrickCards;
    ref array<int> TrickBy;
    // In raspasy, the talon's card that leads the trick being played.
    string TalonLead;
    ref array<string> Played;
    int TrickNo;
    int PoolTarget;
    int RaspasyRun;
    ref array<int> Pool;
    ref array<int> Hill;
    // Whists[i * 3 + j]: the whists seat index i wrote on j.
    ref array<int> Whists;
    int Closed;
    int Ended;
    ref array<string> Results;
    ref array<ref OZG_DealCard> Dealt;

    void OZG_PrefState()
    {
        SeatList = new array<ref OZG_PrefSeat>();
        Talon = new array<string>();
        Discards = new array<string>();
        TalonSeen = new array<string>();
        TrickCards = new array<string>();
        TrickBy = new array<int>();
        Played = new array<string>();
        Pool = new array<int>();
        Hill = new array<int>();
        Whists = new array<int>();
        Results = new array<string>();
        Dealt = new array<ref OZG_DealCard>();
        Turn = -1;
        BidTop = -1;
        Bidder = -1;
        Declarer = -1;
    }
}

class OZG_Preferans : OZG_CardGame
{
    protected ref OZG_PrefState m_S;

    override string GameId()
    {
        return OZG_PrefConst.GAME_ID;
    }

    override string LabelKey()
    {
        return "STR_OZG_GAME_PREFERANS";
    }

    override int MinPlayers()
    {
        return 3;
    }

    override int MaxPlayers()
    {
        return 3;
    }

    override void CardsUsed(array<string> into)
    {
        OZG_Cards.FromRank(OZG_PrefConst.LOWEST, into);
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        OZG_Option pool = new OZG_Option();
        pool.OptionId = OZG_PrefConst.OPT_POOL;
        pool.LabelKey = "STR_OZG_PREF_POOL";
        array<string> values = {"10", "20", "30", "50"};
        for (int i = 0; i < values.Count(); i++)
        {
            pool.ValueIds.Insert(values[i]);
            pool.ValueKeys.Insert("STR_OZG_PREF_POOL_" + values[i]);
        }
        pool.DefaultId = "20";
        into.Insert(pool);
    }

    override OZG_Game Fresh()
    {
        return new OZG_Preferans();
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

    // A defender's hand lying open for everybody.
    protected bool HandOpen(int i)
    {
        if (m_S.Phase != OZG_PrefConst.PLAY || m_S.Open == 0)
            return false;
        return i != m_S.Declarer;
    }

    override protected void HiddenAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (!HandOpen(i))
            into.Copy(m_S.SeatList[i].Held);
    }

    override protected void ShownAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (HandOpen(i))
            into.Copy(m_S.SeatList[i].Held);
    }

    override protected void TableUp(array<string> into)
    {
        into.Clear();
        if (!m_S)
            return;
        into.Copy(m_S.TrickCards);
        if (m_S.TalonLead != "")
            into.Insert(m_S.TalonLead);
        if (m_S.Phase == OZG_PrefConst.TALON)
        {
            for (int k = 0; k < m_S.TalonSeen.Count(); k++)
                into.Insert(m_S.TalonSeen[k]);
        }
    }

    override protected string StockTop()
    {
        if (!m_S || m_S.Talon.Count() == 0)
            return "";
        return m_S.Talon[0];
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
        for (k = 0; k < m_S.Talon.Count(); k++)
            into.Insert(m_S.Talon[k]);
        for (k = 0; k < m_S.Discards.Count(); k++)
            into.Insert(m_S.Discards[k]);
        for (k = 0; k < m_S.TrickCards.Count(); k++)
            into.Insert(m_S.TrickCards[k]);
        if (m_S.TalonLead != "")
            into.Insert(m_S.TalonLead);
        for (k = 0; k < m_S.Played.Count(); k++)
            into.Insert(m_S.Played[k]);
    }

    // ---- bids -----------------------------------------------------------------

    // "6S".."10N" rank 0..48 by twos; "M" (misère) 29; -1 for what is none.
    static int BidRank(string code)
    {
        if (code == "M")
            return OZG_PrefConst.MISERE_RANK;
        int n = code.Length();
        if (n < 2 || n > 3)
            return -1;
        int level = code.Substring(0, n - 1).ToInt();
        string strains = OZG_PrefConst.STRAINS;
        int strain = strains.IndexOf(code.Substring(n - 1, 1));
        if (level < 6 || level > 10 || strain < 0)
            return -1;
        return ((level - 6) * 5 + strain) * 2;
    }

    static string BidOfIndex(int index)
    {
        int level = index / 5 + 6;
        string strains = OZG_PrefConst.STRAINS;
        return level.ToString() + strains.Substring(index % 5, 1);
    }

    static int Price(int level)
    {
        if (level == 6)
            return 2;
        if (level == 7)
            return 4;
        if (level == 8)
            return 6;
        if (level == 9)
            return 8;
        return 10;
    }

    static int Owed(int level)
    {
        if (level == 6)
            return 4;
        if (level == 7)
            return 2;
        if (level == 8 || level == 9)
            return 1;
        return 0;
    }

    protected string TrumpSuit()
    {
        if (m_S.Misere == 1 || m_S.Raspasy == 1)
            return "";
        if (m_S.Strain == "" || m_S.Strain == "N")
            return "";
        string strains = OZG_PrefConst.STRAINS;
        string suits = OZG_PrefConst.STRAIN_SUITS;
        int at = strains.IndexOf(m_S.Strain);
        if (at < 0 || at >= suits.Length())
            return "";
        return suits.Substring(at, 1);
    }

    protected int FirstHand()
    {
        return (m_S.Dealer + 1) % 3;
    }

    // ---- the deal -------------------------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        if (seats.Count() != 3)
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        OZG_PrefState s = new OZG_PrefState();
        s.Version = OZG_PrefConst.VERSION;
        array<string> deck = new array<string>();
        DeckOf(deck, s.Dealt);
        if (deck.Count() < 32)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        dice.Shuffle(deck);
        int i;
        for (i = 0; i < 3; i++)
        {
            OZG_PrefSeat seat = new OZG_PrefSeat();
            seat.SeatNo = seats[i];
            s.SeatList.Insert(seat);
        }
        for (int k = 0; k < 30; k++)
            s.SeatList[k % 3].Held.Insert(deck[k]);
        s.Talon.Insert(deck[30]);
        s.Talon.Insert(deck[31]);
        array<ref OZG_DealCard> dealt = new array<ref OZG_DealCard>();
        for (int d = 0; d < s.Dealt.Count(); d++)
        {
            if (deck.Find(s.Dealt[d].Name) < 32)
                dealt.Insert(s.Dealt[d]);
        }
        s.Dealt = dealt;
        for (i = 0; i < 3; i++)
            SortBySuit(s.SeatList[i].Held, "scdh");

        // The pool so far, or a new one.
        int target = OZG_Game.OptionOf(options, OZG_PrefConst.OPT_POOL, "20").ToInt();
        if (target < 1)
            target = 20;
        int was = OZG_Game.HintInt(hint, OZG_PrefConst.HINT_DEALER, -1);
        int at = seats.Find(was);
        array<int> pool = new array<int>();
        array<int> hill = new array<int>();
        array<int> whists = new array<int>();
        HintList(hint, OZG_PrefConst.HINT_POOL, pool);
        HintList(hint, OZG_PrefConst.HINT_HILL, hill);
        HintList(hint, OZG_PrefConst.HINT_WHISTS, whists);
        bool carried = at >= 0 && pool.Count() == 3 && hill.Count() == 3 && whists.Count() == 9;
        if (carried)
        {
            s.Pool.Copy(pool);
            s.Hill.Copy(hill);
            s.Whists.Copy(whists);
            s.PoolTarget = OZG_Game.HintInt(hint, OZG_PrefConst.HINT_TARGET, target);
            s.RaspasyRun = OZG_Game.HintInt(hint, OZG_PrefConst.HINT_RUN, 0);
        }
        else
        {
            for (i = 0; i < 3; i++)
            {
                s.Pool.Insert(0);
                s.Hill.Insert(0);
            }
            for (i = 0; i < 9; i++)
                s.Whists.Insert(0);
            s.PoolTarget = target;
            s.RaspasyRun = 0;
        }
        if (at >= 0)
        {
            s.Dealer = (at + 1) % 3;
        }
        else
        {
            s.Dealer = seats.Find(OZG_Game.HintInt(hint, OZG_Const.HINT_HOST, -1));
            if (s.Dealer < 0)
                s.Dealer = 0;
        }
        s.Phase = OZG_PrefConst.AUCTION;
        m_S = s;
        m_S.Turn = FirstHand();
        HearSound(OZG_SoundIds.SHUFFLE);
        HearSound(OZG_SoundIds.DEAL);
        return true;
    }

    // ---- the auction ----------------------------------------------------------

    protected int NotPassed()
    {
        int c = 0;
        for (int i = 0; i < 3; i++)
        {
            if (m_S.SeatList[i].Passed == 0)
                c++;
        }
        return c;
    }

    protected void AfterCall(int i)
    {
        int left = NotPassed();
        if (left == 0 && m_S.BidTop < 0)
        {
            StartRaspasy();
            return;
        }
        if (m_S.BidTop >= 0 && left <= 1)
        {
            if (m_S.SeatList[m_S.Bidder].Passed == 0)
            {
                TakeTalon();
                return;
            }
        }
        for (int k = 1; k <= 3; k++)
        {
            int j = (i + k) % 3;
            if (m_S.SeatList[j].Passed == 0)
            {
                m_S.Turn = j;
                return;
            }
        }
        StartRaspasy();
    }

    protected void TakeTalon()
    {
        m_S.Declarer = m_S.Bidder;
        m_S.Misere = 0;
        if (m_S.BidCode == "M")
            m_S.Misere = 1;
        m_S.TalonSeen.Copy(m_S.Talon);
        array<string> held = m_S.SeatList[m_S.Declarer].Held;
        for (int k = 0; k < m_S.Talon.Count(); k++)
            held.Insert(m_S.Talon[k]);
        m_S.Talon.Clear();
        SortBySuit(held, "scdh");
        m_S.Phase = OZG_PrefConst.TALON;
        m_S.Turn = m_S.Declarer;
        m_S.Drops = 0;
        HearSound(OZG_SoundIds.TRUMP);
    }

    protected void StartRaspasy()
    {
        m_S.Raspasy = 1;
        m_S.Declarer = -1;
        m_S.Phase = OZG_PrefConst.PLAY;
        StartTrick(FirstHand());
    }

    // ---- the play -------------------------------------------------------------

    // Who plays the cards of hand index `hand`: a lone open whister plays the
    // passer's too.
    protected int Controller(int hand)
    {
        if (m_S.Phase != OZG_PrefConst.PLAY)
            return hand;
        if (m_S.Open == 1 && m_S.Whisters == 1 && m_S.Misere == 0 && hand != m_S.Declarer)
        {
            if (m_S.SeatList[hand].Whist == 2)
            {
                for (int j = 0; j < 3; j++)
                {
                    if (j != m_S.Declarer && m_S.SeatList[j].Whist == 1)
                        return j;
                }
            }
        }
        return hand;
    }

    protected void StartPlay()
    {
        m_S.Phase = OZG_PrefConst.PLAY;
        StartTrick(FirstHand());
    }

    protected void StartTrick(int leader)
    {
        m_S.TrickCards.Clear();
        m_S.TrickBy.Clear();
        m_S.TalonLead = "";
        // Raspasy: the talon's cards lead the first two tricks, the first
        // hand plays first to each.
        if (m_S.Raspasy == 1 && m_S.TrickNo < 2 && m_S.Talon.Count() > 0)
        {
            m_S.TalonLead = m_S.Talon[0];
            m_S.Talon.RemoveOrdered(0);
            m_S.Turn = FirstHand();
            HearSound(OZG_SoundIds.TRUMP);
            return;
        }
        m_S.Turn = leader;
    }

    protected string LeadSuit()
    {
        if (m_S.TalonLead != "")
            return OZG_Cards.SuitOf(m_S.TalonLead);
        if (m_S.TrickCards.Count() == 0)
            return "";
        return OZG_Cards.SuitOf(m_S.TrickCards[0]);
    }

    protected void PlayCard(int hand, string card)
    {
        m_S.SeatList[hand].Held.RemoveItem(card);
        m_S.TrickCards.Insert(card);
        m_S.TrickBy.Insert(hand);
        HearSound(OZG_SoundIds.LAY);
        if (m_S.TrickCards.Count() < 3)
        {
            m_S.Turn = (hand + 1) % 3;
            return;
        }
        int winner = -1;
        if (m_S.TalonLead != "")
        {
            string lead = OZG_Cards.SuitOf(m_S.TalonLead);
            int best = -1;
            for (int k = 0; k < 3; k++)
            {
                if (OZG_Cards.SuitOf(m_S.TrickCards[k]) != lead)
                    continue;
                if (best < 0 || OZG_Cards.RankOf(m_S.TrickCards[k]) > OZG_Cards.RankOf(m_S.TrickCards[best]))
                    best = k;
            }
            if (best >= 0)
                winner = m_S.TrickBy[best];
            m_S.Played.Insert(m_S.TalonLead);
            m_S.TalonLead = "";
        }
        else
        {
            int w = TrickWinner(m_S.TrickCards, TrumpSuit());
            winner = m_S.TrickBy[w];
        }
        if (winner >= 0)
            m_S.SeatList[winner].Taken = m_S.SeatList[winner].Taken + 1;
        for (int t = 0; t < m_S.TrickCards.Count(); t++)
            m_S.Played.Insert(m_S.TrickCards[t]);
        m_S.TrickCards.Clear();
        m_S.TrickBy.Clear();
        m_S.TrickNo = m_S.TrickNo + 1;
        HearSound(OZG_SoundIds.SWEEP);
        if (m_S.TrickNo >= 10)
        {
            ScoreDeal();
            return;
        }
        if (winner < 0)
            winner = FirstHand();
        StartTrick(winner);
    }

    // ---- the score ------------------------------------------------------------

    protected void AddWhists(int from, int on, int n)
    {
        int at = from * 3 + on;
        m_S.Whists[at] = m_S.Whists[at] + n;
    }

    protected void ScoreDeal()
    {
        int i;
        int d = m_S.Declarer;
        if (m_S.Raspasy == 1)
        {
            int m = m_S.RaspasyRun + 1;
            if (m > 3)
                m = 3;
            int least = 99;
            for (i = 0; i < 3; i++)
            {
                if (m_S.SeatList[i].Taken < least)
                    least = m_S.SeatList[i].Taken;
            }
            for (i = 0; i < 3; i++)
            {
                int t = m_S.SeatList[i].Taken;
                m_S.Hill[i] = m_S.Hill[i] + m * (t - least);
                if (t == 0)
                    m_S.Pool[i] = m_S.Pool[i] + m;
                m_S.Results.Insert(OZG_Words.W2("STR_OZG_PREF_RES_RASPASY", OZG_Words.SeatArg(SeatNoAt(i)), OZG_Words.NumArg(t)));
            }
        }
        else if (m_S.Misere == 1)
        {
            int taken = m_S.SeatList[d].Taken;
            if (taken == 0)
            {
                m_S.Pool[d] = m_S.Pool[d] + 10;
                m_S.Results.Insert(OZG_Words.W1("STR_OZG_PREF_RES_MISERE_MADE", OZG_Words.SeatArg(SeatNoAt(d))));
            }
            else
            {
                m_S.Hill[d] = m_S.Hill[d] + 10 * taken;
                m_S.Results.Insert(OZG_Words.W2("STR_OZG_PREF_RES_MISERE_DOWN", OZG_Words.SeatArg(SeatNoAt(d)), OZG_Words.NumArg(taken)));
            }
        }
        else
        {
            ScoreContract();
        }
        int run = 0;
        if (m_S.Raspasy == 1)
            run = m_S.RaspasyRun + 1;
        m_S.RaspasyRun = run;
        bool closed = true;
        for (i = 0; i < 3; i++)
        {
            if (m_S.Pool[i] < m_S.PoolTarget)
                closed = false;
        }
        if (closed)
            m_S.Closed = 1;
        m_S.Phase = OZG_PrefConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
    }

    protected void ScoreContract()
    {
        int d = m_S.Declarer;
        int level = m_S.Level;
        int price = Price(level);
        int owed = Owed(level);
        int i;
        array<int> defenders = new array<int>();
        for (i = 0; i < 3; i++)
        {
            if (i != d)
                defenders.Insert(i);
        }
        // Both defenders passed: the game is the declarer's unplayed.
        if (m_S.Whisters == 0)
        {
            m_S.Pool[d] = m_S.Pool[d] + price;
            m_S.Results.Insert(OZG_Words.W2("STR_OZG_PREF_RES_UNPLAYED_BY", OZG_Words.SeatArg(SeatNoAt(d)), ContractArg()));
            return;
        }
        int decl = m_S.SeatList[d].Taken;
        int under = level - decl;
        if (under < 0)
            under = 0;
        if (under == 0)
        {
            m_S.Pool[d] = m_S.Pool[d] + price;
            m_S.Results.Insert(OZG_Words.W3("STR_OZG_PREF_RES_MADE", OZG_Words.SeatArg(SeatNoAt(d)), ContractArg(), OZG_Words.NumArg(decl)));
        }
        else
        {
            m_S.Hill[d] = m_S.Hill[d] + price * under;
            m_S.Results.Insert(OZG_Words.W3("STR_OZG_PREF_RES_DOWN", OZG_Words.SeatArg(SeatNoAt(d)), ContractArg(), OZG_Words.NumArg(under)));
        }
        if (m_S.Whisters == 0)
            return;
        if (m_S.Whisters == 1)
        {
            int w = defenders[0];
            if (m_S.SeatList[w].Whist != 1)
                w = defenders[1];
            int credited = m_S.SeatList[defenders[0]].Taken + m_S.SeatList[defenders[1]].Taken;
            AddWhists(w, d, price * (credited + under));
            int short1 = owed - credited;
            if (short1 > 0)
            {
                m_S.Hill[w] = m_S.Hill[w] + price * short1;
                m_S.Results.Insert(OZG_Words.W2("STR_OZG_PREF_RES_WHIST_SHORT", OZG_Words.SeatArg(SeatNoAt(w)), OZG_Words.NumArg(short1)));
            }
            return;
        }
        int both = m_S.SeatList[defenders[0]].Taken + m_S.SeatList[defenders[1]].Taken;
        for (i = 0; i < 2; i++)
        {
            int wi = defenders[i];
            int own = m_S.SeatList[wi].Taken;
            AddWhists(wi, d, price * (own + under));
            if (both < owed)
            {
                int halves = owed - 2 * own;
                if (halves > 0)
                {
                    m_S.Hill[wi] = m_S.Hill[wi] + price * halves / 2;
                    m_S.Results.Insert(OZG_Words.W2("STR_OZG_PREF_RES_WHIST_SHORT", OZG_Words.SeatArg(SeatNoAt(wi)), OZG_Words.NumArg(halves)));
                }
            }
        }
    }

    // A player's balance in whists, in thirtieths, as if the pool closed now:
    // the mountain less what the pool holds beyond the target (or plus what it
    // lacks), turned into whists at ten a point against the other two.
    protected int BalanceThirtieths(int i)
    {
        array<int> g = new array<int>();
        int sum = 0;
        int k;
        for (k = 0; k < 3; k++)
        {
            int gk = m_S.Hill[k] - (m_S.Pool[k] - m_S.PoolTarget);
            g.Insert(gk);
            sum = sum + gk;
        }
        int whists = 0;
        for (k = 0; k < 3; k++)
        {
            if (k == i)
                continue;
            whists = whists + m_S.Whists[i * 3 + k] - m_S.Whists[k * 3 + i];
        }
        return 30 * whists + 100 * (sum - 3 * g[i]);
    }

    // Thirtieths as whists with one decimal: "12.3", "-4.0".
    static string WhistText(int thirtieths)
    {
        bool neg = thirtieths < 0;
        int x = thirtieths;
        if (neg)
            x = -x;
        int tenths = (x + 1) / 3;
        int whole = tenths / 10;
        int part = tenths % 10;
        string text = whole.ToString() + "." + part.ToString();
        if (neg && tenths > 0)
            text = "-" + text;
        return text;
    }

    // ---- moves ---------------------------------------------------------------

    // On the wire: `bid:<6..10><S|C|D|H|N>`, `misere`, `pass` (the auction);
    // `drop:<card>` twice (the declarer's discard); `contract:<code>`;
    // `whist`, `nowhist`; `open`, `closed`; `play:<card>`.
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
        string verb = move;
        string arg = "";
        int colon = move.IndexOf(":");
        if (colon > 0)
        {
            verb = move.Substring(0, colon);
            arg = move.Substring(colon + 1, move.Length() - colon - 1);
        }
        int phase = m_S.Phase;
        if (phase == OZG_PrefConst.PLAY)
        {
            int hand = m_S.Turn;
            if (verb != "play" || Controller(hand) != i)
            {
                refusal = OZG_PrefConst.NO_TURN;
                return false;
            }
            array<string> held = m_S.SeatList[hand].Held;
            if (!PlayAllowed(held, arg, LeadSuit(), TrumpSuit(), true))
            {
                refusal = OZG_PrefConst.NO_SUIT;
                return false;
            }
            PlayCard(hand, arg);
            return true;
        }
        if (i != m_S.Turn)
        {
            refusal = OZG_PrefConst.NO_TURN;
            return false;
        }
        OZG_PrefSeat s = m_S.SeatList[i];

        if (phase == OZG_PrefConst.AUCTION)
        {
            if (verb == "pass")
            {
                s.Passed = 1;
                s.Spoke = 1;
                s.Said = OZG_Words.W1("STR_OZG_G_SAY", "b:P");
                AfterCall(i);
                return true;
            }
            string code = arg;
            if (verb == "misere")
                code = "M";
            int rank = BidRank(code);
            bool bidding = verb == "bid" || verb == "misere";
            if (!bidding || rank < 0 || rank <= m_S.BidTop)
            {
                refusal = OZG_PrefConst.NO_BID;
                return false;
            }
            if (code == "M" && s.Spoke == 1)
            {
                refusal = OZG_PrefConst.NO_BID;
                return false;
            }
            s.Spoke = 1;
            s.Said = OZG_Words.W1("STR_OZG_G_SAY", "b:" + code);
            m_S.BidTop = rank;
            m_S.BidCode = code;
            m_S.Bidder = i;
            AfterCall(i);
            return true;
        }

        if (phase == OZG_PrefConst.TALON)
        {
            if (verb != "drop" || s.Held.Find(arg) < 0)
            {
                refusal = OZG_Const.NO_CARD;
                return false;
            }
            s.Held.RemoveItem(arg);
            m_S.Discards.Insert(arg);
            m_S.Drops = m_S.Drops + 1;
            if (m_S.Drops < 2)
                return true;
            m_S.TalonSeen.Clear();
            if (m_S.Misere == 1)
            {
                m_S.Open = 1;
                StartPlay();
                return true;
            }
            m_S.Phase = OZG_PrefConst.CONTRACT;
            return true;
        }

        if (phase == OZG_PrefConst.CONTRACT)
        {
            int named = BidRank(arg);
            if (verb != "contract" || arg == "M" || named < 0 || named < m_S.BidTop)
            {
                refusal = OZG_PrefConst.NO_BID;
                return false;
            }
            int n = arg.Length();
            m_S.Level = arg.Substring(0, n - 1).ToInt();
            m_S.Strain = arg.Substring(n - 1, 1);
            m_S.Phase = OZG_PrefConst.WHIST;
            m_S.Turn = (i + 1) % 3;
            return true;
        }

        if (phase == OZG_PrefConst.WHIST)
        {
            if (verb == "whist")
            {
                s.Whist = 1;
                m_S.Whisters = m_S.Whisters + 1;
                s.Said = OZG_Words.W0("STR_OZG_PREF_SAID_WHIST");
            }
            else if (verb == "nowhist")
            {
                s.Whist = 2;
                s.Said = OZG_Words.W0("STR_OZG_PREF_SAID_PASS");
            }
            else
            {
                refusal = OZG_Const.NO_MOVE;
                return false;
            }
            int next = (i + 1) % 3;
            if (next != m_S.Declarer)
            {
                m_S.Turn = next;
                return true;
            }
            if (m_S.Whisters == 0)
            {
                ScoreDeal();
                return true;
            }
            if (m_S.Whisters == 1)
            {
                m_S.Phase = OZG_PrefConst.CHOICE;
                for (int j = 0; j < 3; j++)
                {
                    if (m_S.SeatList[j].Whist == 1)
                        m_S.Turn = j;
                }
                return true;
            }
            StartPlay();
            return true;
        }

        if (phase == OZG_PrefConst.CHOICE)
        {
            if (verb == "open")
                m_S.Open = 1;
            else if (verb == "closed")
                m_S.Open = 0;
            else
            {
                refusal = OZG_Const.NO_MOVE;
                return false;
            }
            StartPlay();
            return true;
        }
        refusal = OZG_Const.NO_MOVE;
        return false;
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
        if (!m_S || m_S.Ended == 1 || m_S.Turn < 0)
            return false;
        int i = IndexOfSeatNo(seat);
        if (i < 0)
            return false;
        if (m_S.Phase == OZG_PrefConst.PLAY)
            return Controller(m_S.Turn) == i;
        return i == m_S.Turn;
    }

    // An absent player passes in the auction and does not whist; the rest is
    // waited for.
    override bool Skippable(int seat)
    {
        if (!Waits(seat))
            return false;
        return m_S.Phase == OZG_PrefConst.AUCTION || m_S.Phase == OZG_PrefConst.WHIST;
    }

    override void SkipSeat(int seat)
    {
        if (!Skippable(seat))
            return;
        string why;
        if (m_S.Phase == OZG_PrefConst.AUCTION)
            Act(seat, "pass", why);
        else
            Act(seat, "nowhist", why);
    }

    // A player gone: the deal is thrown in, unscored; the sheet stays.
    override void DropSeat(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return;
        if (IndexOfSeatNo(seat) < 0)
            return;
        for (int i = 0; i < 3; i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (int k = 0; k < held.Count(); k++)
                m_S.Played.Insert(held[k]);
            held.Clear();
        }
        int t;
        for (t = 0; t < m_S.TrickCards.Count(); t++)
            m_S.Played.Insert(m_S.TrickCards[t]);
        for (t = 0; t < m_S.Talon.Count(); t++)
            m_S.Played.Insert(m_S.Talon[t]);
        if (m_S.TalonLead != "")
            m_S.Played.Insert(m_S.TalonLead);
        m_S.TalonLead = "";
        m_S.Talon.Clear();
        m_S.TrickCards.Clear();
        m_S.TrickBy.Clear();
        m_S.TalonSeen.Clear();
        m_S.Phase = OZG_PrefConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
        m_S.Results.Insert(OZG_Words.W0("STR_OZG_PREF_RES_THROWN"));
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        int phase = m_S.Phase;
        int k;
        if (phase == OZG_PrefConst.AUCTION)
        {
            into.Insert("pass");
            if (m_S.SeatList[i].Spoke == 0 && OZG_PrefConst.MISERE_RANK > m_S.BidTop)
                into.Insert("misere");
            for (k = 0; k < 25; k++)
            {
                string code = BidOfIndex(k);
                if (BidRank(code) > m_S.BidTop)
                    into.Insert("bid:" + code);
            }
            return;
        }
        if (phase == OZG_PrefConst.TALON)
        {
            array<string> mine = m_S.SeatList[i].Held;
            for (k = 0; k < mine.Count(); k++)
                into.Insert("drop:" + mine[k]);
            return;
        }
        if (phase == OZG_PrefConst.CONTRACT)
        {
            for (k = 0; k < 25; k++)
            {
                string named = BidOfIndex(k);
                if (BidRank(named) >= m_S.BidTop)
                    into.Insert("contract:" + named);
            }
            return;
        }
        if (phase == OZG_PrefConst.WHIST)
        {
            into.Insert("whist");
            into.Insert("nowhist");
            return;
        }
        if (phase == OZG_PrefConst.CHOICE)
        {
            into.Insert("closed");
            into.Insert("open");
            return;
        }
        int hand = m_S.Turn;
        array<string> held = m_S.SeatList[hand].Held;
        string lead = LeadSuit();
        string trump = TrumpSuit();
        for (k = 0; k < held.Count(); k++)
        {
            if (PlayAllowed(held, held[k], lead, trump, true))
                into.Insert("play:" + held[k]);
        }
    }

    static int Aces(array<string> held)
    {
        int n = 0;
        for (int i = 0; i < held.Count(); i++)
        {
            if (OZG_Cards.RankOf(held[i]) == 12)
                n++;
        }
        return n;
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (!Waits(seat))
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        int i = IndexOfSeatNo(seat);
        array<string> held = m_S.SeatList[i].Held;
        array<string> can = new array<string>();
        Moves(seat, can);
        if (can.Count() == 0)
            return "";
        int phase = m_S.Phase;
        if (phase == OZG_PrefConst.AUCTION)
        {
            // Aces, and the length of the longest suit beyond four.
            int longest = 0;
            string suits = OZG_PrefConst.STRAIN_SUITS;
            for (int su = 0; su < 4; su++)
            {
                int len = 0;
                for (int c = 0; c < held.Count(); c++)
                {
                    if (OZG_Cards.SuitOf(held[c]) == suits.Substring(su, 1))
                        len++;
                }
                if (len > longest)
                    longest = len;
            }
            int strength = Aces(held);
            if (longest > 4)
                strength = strength + longest - 4;
            if (strength >= 3 && m_S.BidTop < 10)
            {
                for (int b = 0; b < can.Count(); b++)
                {
                    if (can[b].IndexOf("bid:") == 0)
                        return can[b];
                }
            }
            return "pass";
        }
        if (phase == OZG_PrefConst.TALON)
            return can[dice.Below(can.Count())];
        if (phase == OZG_PrefConst.CONTRACT)
            return can[0];
        if (phase == OZG_PrefConst.WHIST)
        {
            if (Aces(held) >= 1)
                return "whist";
            return "nowhist";
        }
        if (phase == OZG_PrefConst.CHOICE)
            return "closed";
        return can[dice.Below(can.Count())];
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        int colon = move.IndexOf(":");
        string verb = move;
        string arg = "";
        if (colon > 0)
        {
            verb = move.Substring(0, colon);
            arg = move.Substring(colon + 1, move.Length() - colon - 1);
        }
        if (verb == "play")
        {
            into.Insert("STR_OZG_G_LOG_PLAY");
            into.Insert(OZG_Words.CardArg(arg));
            return;
        }
        if (verb == "bid")
        {
            into.Insert("STR_OZG_PREF_LOG_CALL");
            into.Insert("b:" + arg);
            return;
        }
        if (verb == "pass")
        {
            into.Insert("STR_OZG_PREF_LOG_CALL");
            into.Insert("b:P");
            return;
        }
        if (verb == "misere")
        {
            into.Insert("STR_OZG_PREF_LOG_CALL");
            into.Insert("b:M");
            return;
        }
        if (verb == "contract")
        {
            into.Insert("STR_OZG_PREF_LOG_CONTRACT");
            into.Insert("b:" + arg);
            return;
        }
        // drop (never the card), whist, nowhist, open, closed
        string key = "STR_OZG_PREF_LOG_" + verb;
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
        string text = OZG_PrefConst.HINT_DEALER + "=" + SeatNoAt(m_S.Dealer).ToString();
        // A closed pool starts a new one at the next deal.
        if (m_S.Closed == 1)
            return text;
        text = text + ";" + OZG_PrefConst.HINT_TARGET + "=" + m_S.PoolTarget.ToString();
        text = text + ";" + OZG_PrefConst.HINT_RUN + "=" + m_S.RaspasyRun.ToString();
        text = text + ";" + OZG_PrefConst.HINT_POOL + "=" + ListText(m_S.Pool);
        text = text + ";" + OZG_PrefConst.HINT_HILL + "=" + ListText(m_S.Hill);
        text = text + ";" + OZG_PrefConst.HINT_WHISTS + "=" + ListText(m_S.Whists);
        return text;
    }

    // ---- the view -------------------------------------------------------------

    protected string ContractArg()
    {
        if (m_S.Misere == 1)
            return "b:M";
        return "b:" + m_S.Level.ToString() + m_S.Strain;
    }

    protected string RoleWords(int i)
    {
        OZG_PrefSeat s = m_S.SeatList[i];
        int phase = m_S.Phase;
        if (phase == OZG_PrefConst.AUCTION)
            return s.Said;
        if (m_S.Raspasy == 1)
            return "";
        if (i == m_S.Declarer)
        {
            if (phase == OZG_PrefConst.TALON)
                return OZG_Words.W0("STR_OZG_PREF_ROLE_TALON");
            if (phase == OZG_PrefConst.CONTRACT)
                return OZG_Words.W0("STR_OZG_PREF_ROLE_DECL");
            return OZG_Words.W1("STR_OZG_PREF_ROLE_PLAYS", ContractArg());
        }
        if (s.Whist == 1)
            return OZG_Words.W0("STR_OZG_PREF_SAID_WHIST");
        if (s.Whist == 2)
            return OZG_Words.W0("STR_OZG_PREF_SAID_PASS");
        return "";
    }

    override string PublicView()
    {
        if (!m_S)
            return "{}";
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = OZG_PrefConst.GAME_ID;
        int i;
        for (i = 0; i < 3; i++)
        {
            OZG_PrefSeat s = m_S.SeatList[i];
            OZG_GSeat g = SeatShown(i);
            g.Role = RoleWords(i);
            g.Line = OZG_Words.W1("STR_OZG_PREF_LINE_TRICKS", OZG_Words.NumArg(s.Taken));
            if (i == m_S.Dealer)
                g.Mark = 1;
            v.SeatList.Insert(g);
            if (Waits(s.SeatNo))
                v.Awaited.Insert(s.SeatNo);
        }
        // The talon: face down until the auction ends, face up while the
        // declarer discards; in raspasy the card leading the trick.
        OZG_GZone talon = ZoneOf("talon", OZG_Words.W0("STR_OZG_PREF_TALON"), null, null);
        if (m_S.Phase == OZG_PrefConst.TALON)
        {
            for (i = 0; i < m_S.TalonSeen.Count(); i++)
            {
                talon.Faces.Insert(m_S.TalonSeen[i]);
                talon.Looks.Insert(LookOf(m_S.TalonSeen[i]));
                talon.By.Insert(-1);
            }
        }
        if (m_S.TalonLead != "")
        {
            talon.Faces.Insert(m_S.TalonLead);
            talon.Looks.Insert(LookOf(m_S.TalonLead));
            talon.By.Insert(-1);
        }
        talon.Backs = m_S.Talon.Count();
        if (talon.Backs > 0)
            talon.BackLook = LookOf(m_S.Talon[0]);
        v.Zones.Insert(talon);
        array<int> by = new array<int>();
        for (i = 0; i < m_S.TrickBy.Count(); i++)
            by.Insert(SeatNoAt(m_S.TrickBy[i]));
        v.Zones.Insert(ZoneOf("trick", OZG_Words.W0("STR_OZG_G_TRICK"), m_S.TrickCards, by));

        v.Trump = "";
        if (m_S.Phase == OZG_PrefConst.PLAY)
        {
            v.Trump = "n";
            string trump = TrumpSuit();
            if (trump != "")
                v.Trump = trump;
        }

        // The sheet.
        v.Score.Insert(OZG_Words.W1("STR_OZG_PREF_TARGET", OZG_Words.NumArg(m_S.PoolTarget)));
        for (i = 0; i < 3; i++)
        {
            string balance = WhistText(BalanceThirtieths(i));
            v.Score.Insert(OZG_Words.W4("STR_OZG_PREF_SHEET", OZG_Words.SeatArg(SeatNoAt(i)), OZG_Words.NumArg(m_S.Pool[i]), OZG_Words.NumArg(m_S.Hill[i]), balance));
        }
        if (m_S.RaspasyRun > 0 && m_S.Ended == 0)
            v.Score.Insert(OZG_Words.W1("STR_OZG_PREF_RUN", OZG_Words.NumArg(m_S.RaspasyRun)));
        for (i = 0; i < m_S.Results.Count(); i++)
            v.Score.Insert(m_S.Results[i]);
        if (m_S.Closed == 1)
            v.Score.Insert(OZG_Words.W0("STR_OZG_PREF_CLOSED"));

        int turn = SeatNoAt(m_S.Turn);
        int phase = m_S.Phase;
        if (m_S.Ended == 1)
        {
            v.Ended = 1;
            if (m_S.Results.Count() > 0)
                v.Result = m_S.Results[0];
            else
                v.Result = OZG_Words.W0("STR_OZG_PREF_RES_UNPLAYED");
            if (m_S.Whisters == 0 && m_S.Raspasy == 0 && m_S.Misere == 0)
                v.Result = OZG_Words.W2("STR_OZG_PREF_RES_UNPLAYED_BY", OZG_Words.SeatArg(SeatNoAt(m_S.Declarer)), ContractArg());
            v.Status = v.Result;
        }
        else if (phase == OZG_PrefConst.AUCTION)
            v.Status = OZG_Words.W1("STR_OZG_PREF_ST_AUCTION", OZG_Words.SeatArg(turn));
        else if (phase == OZG_PrefConst.TALON)
            v.Status = OZG_Words.W1("STR_OZG_PREF_ST_TALON", OZG_Words.SeatArg(turn));
        else if (phase == OZG_PrefConst.CONTRACT)
            v.Status = OZG_Words.W1("STR_OZG_PREF_ST_CONTRACT", OZG_Words.SeatArg(turn));
        else if (phase == OZG_PrefConst.WHIST)
            v.Status = OZG_Words.W2("STR_OZG_PREF_ST_WHIST", OZG_Words.SeatArg(turn), ContractArg());
        else if (phase == OZG_PrefConst.CHOICE)
            v.Status = OZG_Words.W1("STR_OZG_PREF_ST_CHOICE", OZG_Words.SeatArg(turn));
        else if (m_S.Raspasy == 1)
            v.Status = OZG_Words.W1("STR_OZG_PREF_ST_RASPASY", OZG_Words.SeatArg(SeatNoAt(Controller(m_S.Turn))));
        else
            v.Status = OZG_Words.W1("STR_OZG_G_ST_PLAY", OZG_Words.SeatArg(SeatNoAt(Controller(m_S.Turn))));
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
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int k = 0; k < can.Count(); k++)
        {
            string move = can[k];
            if (move.IndexOf("play:") == 0 || move.IndexOf("drop:") == 0)
            {
                v.CanPlay.Insert(move);
                continue;
            }
            string code = "";
            if (move.IndexOf("bid:") == 0)
                code = move.Substring(4, move.Length() - 4);
            if (move.IndexOf("contract:") == 0)
                code = move.Substring(9, move.Length() - 9);
            if (code != "")
            {
                int rank = BidRank(code) / 2;
                v.Buttons.Insert(OZG_GButton.Make(move, OZG_Words.W1("STR_OZG_G_SAY", "b:" + code), rank / 5, rank % 5));
                continue;
            }
            string label = "";
            if (move == "pass")
                label = OZG_Words.W1("STR_OZG_G_SAY", "b:P");
            else if (move == "misere")
                label = OZG_Words.W1("STR_OZG_G_SAY", "b:M");
            else
            {
                string key = "STR_OZG_PREF_BTN_" + move;
                key.ToUpper();
                label = OZG_Words.W0(key);
            }
            OZG_GButton b = OZG_GButton.Make(move, label, -1, k);
            if (move == "pass" || move == "whist" || move == "closed")
                b.Hot = 1;
            v.Buttons.Insert(b);
        }
        if (m_S.Phase == OZG_PrefConst.TALON && i == m_S.Declarer)
            v.Status = OZG_Words.W1("STR_OZG_PREF_YOU_DROP", OZG_Words.NumArg(2 - m_S.Drops));
        return PrivateText(v);
    }

    // ---- a check of the sheet on known deals (the stand's `pscores`) ---------

    // A deal scored from tricks given by hand: `kind` "game", "misere" or
    // "raspasy"; for a game the contract, who whisted (1 whist, 2 pass) and
    // the tricks of seat indices 0..2 (0 is the declarer of a game).
    protected static OZG_Preferans Scored(string kind, int level, int w1, int w2, int t0, int t1, int t2, int run)
    {
        OZG_Preferans g = new OZG_Preferans();
        OZG_PrefState s = new OZG_PrefState();
        for (int i = 0; i < 3; i++)
        {
            OZG_PrefSeat seat = new OZG_PrefSeat();
            seat.SeatNo = i;
            s.SeatList.Insert(seat);
            s.Pool.Insert(0);
            s.Hill.Insert(0);
        }
        for (int k = 0; k < 9; k++)
            s.Whists.Insert(0);
        s.PoolTarget = 10;
        s.RaspasyRun = run;
        s.SeatList[0].Taken = t0;
        s.SeatList[1].Taken = t1;
        s.SeatList[2].Taken = t2;
        s.Declarer = 0;
        s.Level = level;
        s.Strain = "S";
        if (kind == "misere")
            s.Misere = 1;
        if (kind == "raspasy")
        {
            s.Raspasy = 1;
            s.Declarer = -1;
        }
        s.SeatList[1].Whist = w1;
        s.SeatList[2].Whist = w2;
        int whisters = 0;
        if (w1 == 1)
            whisters++;
        if (w2 == 1)
            whisters++;
        s.Whisters = whisters;
        g.m_S = s;
        g.ScoreDeal();
        return g;
    }

    protected string SheetLine()
    {
        string text = "p=" + ListText(m_S.Pool) + " m=" + ListText(m_S.Hill);
        text = text + " w=" + ListText(m_S.Whists);
        return text;
    }

    static string SelfCheck()
    {
        // kind, level, whist of 1 and 2, tricks of 0 1 2, raspasy run, the
        // sheet that must come out.
        array<string> cases = {
            "game 6 1 2 6 3 1 0 = p=2,0,0 m=0,0,0 w=0,0,0,8,0,0,0,0,0",
            "game 6 1 2 7 2 1 0 = p=2,0,0 m=0,2,0 w=0,0,0,6,0,0,0,0,0",
            "game 7 1 1 6 2 2 0 = p=0,0,0 m=4,0,0 w=0,0,0,12,0,0,12,0,0",
            "game 8 1 1 8 1 1 0 = p=6,0,0 m=0,0,0 w=0,0,0,6,0,0,6,0,0",
            "game 8 1 1 10 0 0 0 = p=6,0,0 m=0,3,3 w=0,0,0,0,0,0,0,0,0",
            "game 6 2 2 6 3 1 0 = p=2,0,0 m=0,0,0 w=0,0,0,0,0,0,0,0,0",
            "misere 0 0 0 0 5 5 0 = p=10,0,0 m=0,0,0 w=0,0,0,0,0,0,0,0,0",
            "misere 0 0 0 2 4 4 0 = p=0,0,0 m=20,0,0 w=0,0,0,0,0,0,0,0,0",
            "raspasy 0 0 0 3 5 2 0 = p=0,0,0 m=1,3,0 w=0,0,0,0,0,0,0,0,0",
            "raspasy 0 0 0 0 4 6 1 = p=2,0,0 m=0,8,12 w=0,0,0,0,0,0,0,0,0",
            "raspasy 0 0 0 4 4 2 5 = p=0,0,0 m=6,6,0 w=0,0,0,0,0,0,0,0,0"
        };
        for (int c = 0; c < cases.Count(); c++)
        {
            array<string> p = new array<string>();
            cases[c].Split(" ", p);
            OZG_Preferans g = Scored(p[0], p[1].ToInt(), p[2].ToInt(), p[3].ToInt(), p[4].ToInt(), p[5].ToInt(), p[6].ToInt(), p[7].ToInt());
            string want = p[9] + " " + p[10] + " " + p[11];
            string got = g.SheetLine();
            if (got != want)
                return "WRONG on '" + cases[c] + "': " + got;
        }
        // The pool closes once every pool holds the target, and the balance
        // is the mountains turned into whists.
        OZG_Preferans last = Scored("game", 6, 2, 2, 6, 2, 2, 0);
        last.m_S.Pool.Clear();
        last.m_S.Pool.Insert(10);
        last.m_S.Pool.Insert(10);
        last.m_S.Pool.Insert(8);
        last.m_S.Hill.Clear();
        last.m_S.Hill.Insert(0);
        last.m_S.Hill.Insert(10);
        last.m_S.Hill.Insert(20);
        string bal = WhistText(last.BalanceThirtieths(0)) + " " + WhistText(last.BalanceThirtieths(1));
        bal = bal + " " + WhistText(last.BalanceThirtieths(2));
        if (bal != "106.7 6.7 -113.3")
            return "WRONG balance: " + bal;
        return "";
    }

    // ---- saving and checking --------------------------------------------------

    override string SaveState()
    {
        if (!m_S)
            return "";
        string text;
        string err;
        JsonFileLoader<OZG_PrefState>.MakeData(m_S, text, err, false);
        return text;
    }

    override bool LoadState(string data)
    {
        OZG_PrefState s = new OZG_PrefState();
        string err;
        if (!JsonFileLoader<OZG_PrefState>.LoadData(data, s, err))
            return false;
        if (!s || s.Version != OZG_PrefConst.VERSION)
            return false;
        if (!s.SeatList || !s.Talon || !s.Discards || !s.TalonSeen || !s.TrickCards || !s.TrickBy || !s.Played)
            return false;
        if (!s.Pool || !s.Hill || !s.Whists || !s.Dealt)
            return false;
        if (!s.Results)
            s.Results = new array<string>();
        if (s.SeatList.Count() != 3 || s.Pool.Count() != 3 || s.Hill.Count() != 3 || s.Whists.Count() != 9)
            return false;
        for (int i = 0; i < 3; i++)
        {
            if (!s.SeatList[i] || !s.SeatList[i].Held)
                return false;
        }
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        OZG_CardGame.Remade(s.Dealt, made);
        s.Dealt = made;
        OZG_PrefState before = m_S;
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
        if (m_S.SeatList.Count() != 3)
        {
            why = "seats: " + m_S.SeatList.Count().ToString();
            return false;
        }
        if (m_S.TrickCards.Count() != m_S.TrickBy.Count() || m_S.TrickCards.Count() > 2)
        {
            why = "the trick does not add up";
            return false;
        }
        // The balances of a closed pool always come to nothing between them.
        int net = 0;
        for (int b = 0; b < 3; b++)
        {
            if (m_S.Pool[b] < 0 || m_S.Hill[b] < 0)
            {
                why = "a pool or a mountain below nothing";
                return false;
            }
            net = net + BalanceThirtieths(b);
        }
        if (net != 0)
        {
            why = "the balances come to " + net.ToString();
            return false;
        }
        if (m_S.Phase == OZG_PrefConst.PLAY)
        {
            int left = 10 - m_S.TrickNo;
            for (int i = 0; i < 3; i++)
            {
                int held = m_S.SeatList[i].Held.Count();
                int laid = 0;
                if (m_S.TrickBy.Find(i) >= 0)
                    laid = 1;
                if (held + laid != left)
                {
                    why = "hand " + i.ToString() + " holds " + held.ToString();
                    why = why + " with " + left.ToString() + " tricks left";
                    return false;
                }
            }
        }
        return CardsSound(why);
    }

    override string Brief()
    {
        if (!m_S)
            return "preferans: no deal";
        string text = "preferans dealer=" + SeatNoAt(m_S.Dealer).ToString();
        text = text + " phase=" + m_S.Phase.ToString();
        text = text + " bid=" + m_S.BidCode + "@" + SeatNoAt(m_S.Bidder).ToString();
        if (m_S.Raspasy == 1)
            text = text + " raspasy";
        else if (m_S.Declarer >= 0)
        {
            text = text + " decl=" + SeatNoAt(m_S.Declarer).ToString() + " game=" + ContractArg();
            text = text + " whist=" + m_S.Whisters.ToString() + " open=" + m_S.Open.ToString();
        }
        text = text + " tricks=" + m_S.TrickNo.ToString() + ":";
        for (int i = 0; i < 3; i++)
            text = text + m_S.SeatList[i].Taken.ToString();
        text = text + " pool=" + ListText(m_S.Pool) + " hill=" + ListText(m_S.Hill);
        if (m_S.Ended == 1)
            text = text + " over closed=" + m_S.Closed.ToString();
        else
            text = text + " turn=" + SeatNoAt(m_S.Turn).ToString();
        return text;
    }

    override string Everything()
    {
        if (!m_S)
            return "preferans: no deal";
        string text = "";
        for (int i = 0; i < 3; i++)
        {
            OZG_PrefSeat s = m_S.SeatList[i];
            text = text + s.SeatNo.ToString() + "[" + OZG_Cards.Line(s.Held) + "] ";
        }
        text = text + "talon[" + OZG_Cards.Line(m_S.Talon) + "] ";
        text = text + "discards[" + OZG_Cards.Line(m_S.Discards) + "]";
        return text;
    }
}
