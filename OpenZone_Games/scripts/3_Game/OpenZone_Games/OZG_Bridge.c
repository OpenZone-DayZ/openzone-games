// Bridge (stakes-and-games design §7): four players, two sides across the mat,
// the auction, the dummy played by the declarer, each deal scored as in
// duplicate with Chicago vulnerability. The dealer, the deal of the set and the
// two totals pass from deal to deal through the hint.
class OZG_BridgeConst
{
    static const string GAME_ID = "bridge";
    static const int VERSION = 1;

    static const int AUCTION = 0;
    static const int PLAY    = 1;
    static const int DONE    = 2;

    // Strains, the lowest first; their suit letters beside them.
    static const string STRAINS = "CDHSN";
    static const string STRAIN_SUITS = "cdhs";

    static const string HINT_DEALER = "dealer";
    static const string HINT_DEAL   = "k";
    static const string HINT_NS     = "ns";
    static const string HINT_EW     = "ew";
    static const string HINT_REDEAL = "redeal";

    static const string NO_TURN = "STR_OZG_G_NO_TURN";
    static const string NO_BID  = "STR_OZG_BRIDGE_NO_BID";
    static const string NO_SUIT = "STR_OZG_G_NO_SUIT";
}

class OZG_BridgeSeat
{
    int SeatNo;
    ref array<string> Held;
    // Words: its last call.
    string Said;

    void OZG_BridgeSeat()
    {
        Held = new array<string>();
    }
}

class OZG_BridgeState
{
    int Version;
    // Index 0 and 2 are one side (N-S), 1 and 3 the other (E-W).
    ref array<ref OZG_BridgeSeat> SeatList;
    int Dealer;
    // 1..4: the deal of the set of four (Chicago).
    int DealNo;
    int Phase;
    // "1C".."7N", "P", "X", "XX", from the dealer on.
    ref array<string> Calls;
    int Turn;
    int BidLevel;
    string BidStrain;
    int Doubled;
    int Declarer;
    int Dummy;
    int DummyOpen;
    ref array<string> TrickCards;
    // Beside TrickCards: the index of the hand each came from.
    ref array<int> TrickBy;
    ref array<string> Played;
    int TricksNS;
    int TricksEW;
    int TotalNS;
    int TotalEW;
    // What this deal gave, and to which side (0 N-S, 1 E-W, -1 none).
    int DealScore;
    int DealSide;
    int PassedOut;
    int Ended;
    ref array<ref OZG_DealCard> Dealt;

    void OZG_BridgeState()
    {
        SeatList = new array<ref OZG_BridgeSeat>();
        Calls = new array<string>();
        TrickCards = new array<string>();
        TrickBy = new array<int>();
        Played = new array<string>();
        Dealt = new array<ref OZG_DealCard>();
        Turn = -1;
        Declarer = -1;
        Dummy = -1;
        DealSide = -1;
    }
}

class OZG_Bridge : OZG_CardGame
{
    protected ref OZG_BridgeState m_S;

    override string GameId()
    {
        return OZG_BridgeConst.GAME_ID;
    }

    override string LabelKey()
    {
        return "STR_OZG_GAME_BRIDGE";
    }

    override int MinPlayers()
    {
        return 4;
    }

    override int MaxPlayers()
    {
        return 4;
    }

    override void CardsUsed(array<string> into)
    {
        OZG_Cards.Whole(into);
    }

    override OZG_Game Fresh()
    {
        return new OZG_Bridge();
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

    protected bool DummyShown(int i)
    {
        return m_S.Phase == OZG_BridgeConst.PLAY && m_S.DummyOpen == 1 && i == m_S.Dummy;
    }

    override protected void HiddenAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (!DummyShown(i))
            into.Copy(m_S.SeatList[i].Held);
    }

    override protected void ShownAt(int i, array<string> into)
    {
        into.Clear();
        if (!m_S || i < 0 || i >= m_S.SeatList.Count())
            return;
        if (DummyShown(i))
            into.Copy(m_S.SeatList[i].Held);
    }

    override protected void TableUp(array<string> into)
    {
        into.Clear();
        if (m_S)
            into.Copy(m_S.TrickCards);
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
        for (k = 0; k < m_S.TrickCards.Count(); k++)
            into.Insert(m_S.TrickCards[k]);
        for (k = 0; k < m_S.Played.Count(); k++)
            into.Insert(m_S.Played[k]);
    }

    // ---- small helpers --------------------------------------------------------

    static int SideOf(int i)
    {
        return i % 2;
    }

    // A bid's rank: 1C is 0, 7N is 34; -1 for what is no bid.
    static int BidRank(string bid)
    {
        if (bid.Length() != 2)
            return -1;
        int level = bid.Substring(0, 1).ToInt();
        string strains = OZG_BridgeConst.STRAINS;
        int strain = strains.IndexOf(bid.Substring(1, 1));
        if (level < 1 || level > 7 || strain < 0)
            return -1;
        return (level - 1) * 5 + strain;
    }

    static string BidOfRank(int rank)
    {
        int level = rank / 5 + 1;
        int strain = rank % 5;
        string strains = OZG_BridgeConst.STRAINS;
        return level.ToString() + strains.Substring(strain, 1);
    }

    protected string TrumpSuit()
    {
        if (m_S.BidStrain == "" || m_S.BidStrain == "N")
            return "";
        string strains = OZG_BridgeConst.STRAINS;
        string suits = OZG_BridgeConst.STRAIN_SUITS;
        int at = strains.IndexOf(m_S.BidStrain);
        if (at < 0 || at >= suits.Length())
            return "";
        return suits.Substring(at, 1);
    }

    // The index of the hand whose card is played now, and who plays it.
    protected int Controller(int hand)
    {
        if (hand == m_S.Dummy)
            return m_S.Declarer;
        return hand;
    }

    // The last call that is not a pass, its index; -1 for none.
    protected int LastNonPass()
    {
        for (int j = m_S.Calls.Count() - 1; j >= 0; j--)
        {
            if (m_S.Calls[j] != "P")
                return j;
        }
        return -1;
    }

    protected int CallerOf(int j)
    {
        return (m_S.Dealer + j) % 4;
    }

    protected int HighestRank()
    {
        for (int j = m_S.Calls.Count() - 1; j >= 0; j--)
        {
            int r = BidRank(m_S.Calls[j]);
            if (r >= 0)
                return r;
        }
        return -1;
    }

    protected bool MayDouble(int i)
    {
        int j = LastNonPass();
        if (j < 0)
            return false;
        if (BidRank(m_S.Calls[j]) < 0)
            return false;
        return SideOf(CallerOf(j)) != SideOf(i);
    }

    protected bool MayRedouble(int i)
    {
        int j = LastNonPass();
        if (j < 0)
            return false;
        if (m_S.Calls[j] != "X")
            return false;
        return SideOf(CallerOf(j)) != SideOf(i);
    }

    static bool Vulnerable(int side, int dealNo, int dealer)
    {
        if (dealNo == 4)
            return true;
        if (dealNo == 2 || dealNo == 3)
            return SideOf(dealer) == side;
        return false;
    }

    // ---- the deal -------------------------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        if (seats.Count() != 4)
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        OZG_BridgeState s = new OZG_BridgeState();
        s.Version = OZG_BridgeConst.VERSION;
        array<string> deck = new array<string>();
        DeckOf(deck, s.Dealt);
        if (deck.Count() < 52)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        dice.Shuffle(deck);
        int i;
        for (i = 0; i < 4; i++)
        {
            OZG_BridgeSeat seat = new OZG_BridgeSeat();
            seat.SeatNo = seats[i];
            s.SeatList.Insert(seat);
        }
        // Thirteen each; a deck of more cards leaves the rest unplayed out of
        // the deal (a card sharp's deck), so only 52 are dealt.
        for (int k = 0; k < 52; k++)
            s.SeatList[k % 4].Held.Insert(deck[k]);
        // Cards beyond 52 are not of this deal.
        array<ref OZG_DealCard> dealt = new array<ref OZG_DealCard>();
        for (int d = 0; d < s.Dealt.Count(); d++)
        {
            if (deck.Find(s.Dealt[d].Name) < 52)
                dealt.Insert(s.Dealt[d]);
        }
        s.Dealt = dealt;
        for (i = 0; i < 4; i++)
            SortBySuit(s.SeatList[i].Held, "shcd");

        // The dealer, the deal of the set, the totals.
        int was = OZG_Game.HintInt(hint, OZG_BridgeConst.HINT_DEALER, -1);
        int at = seats.Find(was);
        if (at >= 0)
        {
            s.TotalNS = OZG_Game.HintInt(hint, OZG_BridgeConst.HINT_NS, 0);
            s.TotalEW = OZG_Game.HintInt(hint, OZG_BridgeConst.HINT_EW, 0);
            int k0 = OZG_Game.HintInt(hint, OZG_BridgeConst.HINT_DEAL, 0);
            if (OZG_Game.HintInt(hint, OZG_BridgeConst.HINT_REDEAL, 0) == 1)
            {
                s.Dealer = at;
                s.DealNo = k0;
            }
            else
            {
                s.Dealer = (at + 1) % 4;
                s.DealNo = k0 % 4 + 1;
            }
            if (s.DealNo < 1 || s.DealNo > 4)
                s.DealNo = 1;
        }
        else
        {
            s.Dealer = seats.Find(OZG_Game.HintInt(hint, OZG_Const.HINT_HOST, -1));
            if (s.Dealer < 0)
                s.Dealer = 0;
            s.DealNo = 1;
        }
        s.Phase = OZG_BridgeConst.AUCTION;
        s.Turn = s.Dealer;
        m_S = s;
        HearSound(OZG_SoundIds.SHUFFLE);
        HearSound(OZG_SoundIds.DEAL);
        return true;
    }

    // ---- the auction ----------------------------------------------------------

    protected void Call(int i, string call)
    {
        m_S.Calls.Insert(call);
        m_S.SeatList[i].Said = OZG_Words.W1("STR_OZG_G_SAY", "b:" + call);
        m_S.Turn = (i + 1) % 4;
        int n = m_S.Calls.Count();
        if (n < 4)
            return;
        bool lastThree = m_S.Calls[n - 1] == "P" && m_S.Calls[n - 2] == "P" && m_S.Calls[n - 3] == "P";
        if (!lastThree)
            return;
        int high = HighestRank();
        if (high < 0)
        {
            if (n >= 4 && m_S.Calls[n - 4] == "P")
                PassOut();
            return;
        }
        Contract();
    }

    protected void PassOut()
    {
        m_S.PassedOut = 1;
        m_S.Phase = OZG_BridgeConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
        // Every hand back face down into the deal's discards.
        for (int i = 0; i < 4; i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (int k = 0; k < held.Count(); k++)
                m_S.Played.Insert(held[k]);
            held.Clear();
        }
    }

    protected void Contract()
    {
        int j;
        int last = -1;
        for (j = m_S.Calls.Count() - 1; j >= 0; j--)
        {
            if (BidRank(m_S.Calls[j]) >= 0)
            {
                last = j;
                break;
            }
        }
        string bid = m_S.Calls[last];
        m_S.BidLevel = bid.Substring(0, 1).ToInt();
        m_S.BidStrain = bid.Substring(1, 1);
        m_S.Doubled = 0;
        for (j = last + 1; j < m_S.Calls.Count(); j++)
        {
            if (m_S.Calls[j] == "X")
                m_S.Doubled = 1;
            if (m_S.Calls[j] == "XX")
                m_S.Doubled = 2;
        }
        int side = SideOf(CallerOf(last));
        m_S.Declarer = CallerOf(last);
        for (j = 0; j <= last; j++)
        {
            int r = BidRank(m_S.Calls[j]);
            if (r < 0)
                continue;
            if (SideOf(CallerOf(j)) != side)
                continue;
            if (m_S.Calls[j].Substring(1, 1) == m_S.BidStrain)
            {
                m_S.Declarer = CallerOf(j);
                break;
            }
        }
        m_S.Dummy = (m_S.Declarer + 2) % 4;
        m_S.Phase = OZG_BridgeConst.PLAY;
        m_S.Turn = (m_S.Declarer + 1) % 4;
        for (int i = 0; i < 4; i++)
            m_S.SeatList[i].Said = "";
    }

    // ---- the play -------------------------------------------------------------

    protected string LeadSuit()
    {
        if (m_S.TrickCards.Count() == 0)
            return "";
        return OZG_Cards.SuitOf(m_S.TrickCards[0]);
    }

    protected void PlayCard(int hand, string card)
    {
        array<string> held = m_S.SeatList[hand].Held;
        held.RemoveItem(card);
        m_S.TrickCards.Insert(card);
        m_S.TrickBy.Insert(hand);
        m_S.DummyOpen = 1;
        HearSound(OZG_SoundIds.LAY);
        if (m_S.TrickCards.Count() < 4)
        {
            m_S.Turn = (hand + 1) % 4;
            return;
        }
        int w = TrickWinner(m_S.TrickCards, TrumpSuit());
        int winner = m_S.TrickBy[w];
        if (SideOf(winner) == 0)
            m_S.TricksNS = m_S.TricksNS + 1;
        else
            m_S.TricksEW = m_S.TricksEW + 1;
        for (int k = 0; k < m_S.TrickCards.Count(); k++)
            m_S.Played.Insert(m_S.TrickCards[k]);
        m_S.TrickCards.Clear();
        m_S.TrickBy.Clear();
        HearSound(OZG_SoundIds.SWEEP);
        m_S.Turn = winner;
        if (m_S.TricksNS + m_S.TricksEW >= 13)
            ScoreDeal();
    }

    protected void ScoreDeal()
    {
        int side = SideOf(m_S.Declarer);
        int tricks = m_S.TricksNS;
        if (side == 1)
            tricks = m_S.TricksEW;
        bool vul = Vulnerable(side, m_S.DealNo, m_S.Dealer);
        int need = m_S.BidLevel + 6;
        int score = 0;
        if (tricks >= need)
        {
            score = MadeScore(m_S.BidLevel, m_S.BidStrain, m_S.Doubled, vul, tricks - need);
            m_S.DealSide = side;
        }
        else
        {
            score = DownScore(need - tricks, m_S.Doubled, vul);
            m_S.DealSide = 1 - side;
        }
        m_S.DealScore = score;
        if (m_S.DealSide == 0)
            m_S.TotalNS = m_S.TotalNS + score;
        else
            m_S.TotalEW = m_S.TotalEW + score;
        m_S.Phase = OZG_BridgeConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
    }

    static int MadeScore(int level, string strain, int doubled, bool vul, int over)
    {
        int per = 30;
        if (strain == "C" || strain == "D")
            per = 20;
        int points = per * level;
        if (strain == "N")
            points = 40 + 30 * (level - 1);
        int mult = 1;
        if (doubled == 1)
            mult = 2;
        if (doubled == 2)
            mult = 4;
        points = points * mult;
        int score = points;
        if (points >= 100)
        {
            if (vul)
                score = score + 500;
            else
                score = score + 300;
        }
        else
        {
            score = score + 50;
        }
        if (level == 6)
        {
            if (vul)
                score = score + 750;
            else
                score = score + 500;
        }
        if (level == 7)
        {
            if (vul)
                score = score + 1500;
            else
                score = score + 1000;
        }
        if (doubled == 1)
            score = score + 50;
        if (doubled == 2)
            score = score + 100;
        int each = per;
        if (doubled == 1)
        {
            each = 100;
            if (vul)
                each = 200;
        }
        if (doubled == 2)
        {
            each = 200;
            if (vul)
                each = 400;
        }
        return score + each * over;
    }

    static int DownScore(int under, int doubled, bool vul)
    {
        if (doubled == 0)
        {
            if (vul)
                return under * 100;
            return under * 50;
        }
        int pen = 0;
        for (int t = 1; t <= under; t++)
        {
            int add = 300;
            if (vul)
            {
                if (t == 1)
                    add = 200;
            }
            else
            {
                if (t == 1)
                    add = 100;
                else if (t <= 3)
                    add = 200;
            }
            pen = pen + add;
        }
        if (doubled == 2)
            pen = pen * 2;
        return pen;
    }

    // ---- moves ---------------------------------------------------------------

    // On the wire: `bid:<level><C|D|H|S|N>`, `pass`, `dbl`, `rdbl`;
    // `play:<card>` -- the declarer's own or, at the dummy's turn, the dummy's.
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

        if (m_S.Phase == OZG_BridgeConst.AUCTION)
        {
            if (i != m_S.Turn)
            {
                refusal = OZG_BridgeConst.NO_TURN;
                return false;
            }
            if (verb == "pass")
            {
                Call(i, "P");
                return true;
            }
            if (verb == "dbl" && MayDouble(i))
            {
                Call(i, "X");
                return true;
            }
            if (verb == "rdbl" && MayRedouble(i))
            {
                Call(i, "XX");
                return true;
            }
            if (verb == "bid")
            {
                int rank = BidRank(arg);
                if (rank < 0 || rank <= HighestRank())
                {
                    refusal = OZG_BridgeConst.NO_BID;
                    return false;
                }
                Call(i, arg);
                return true;
            }
            refusal = OZG_Const.NO_MOVE;
            return false;
        }

        if (m_S.Phase == OZG_BridgeConst.PLAY && verb == "play")
        {
            int hand = m_S.Turn;
            if (Controller(hand) != i)
            {
                refusal = OZG_BridgeConst.NO_TURN;
                return false;
            }
            array<string> held = m_S.SeatList[hand].Held;
            if (held.Find(arg) < 0)
            {
                refusal = OZG_Const.NO_CARD;
                return false;
            }
            if (!PlayAllowed(held, arg, LeadSuit(), TrumpSuit(), false))
            {
                refusal = OZG_BridgeConst.NO_SUIT;
                return false;
            }
            PlayCard(hand, arg);
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
        if (m_S.Phase == OZG_BridgeConst.AUCTION)
            return i == m_S.Turn;
        return Controller(m_S.Turn) == i;
    }

    // An absent player passes in the auction; a card is waited for.
    override bool Skippable(int seat)
    {
        return Waits(seat) && m_S.Phase == OZG_BridgeConst.AUCTION;
    }

    override void SkipSeat(int seat)
    {
        if (!Skippable(seat))
            return;
        string why;
        Act(seat, "pass", why);
    }

    // A player gone: the deal is thrown in, unscored, and dealt again by the
    // same dealer.
    override void DropSeat(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return;
        if (IndexOfSeatNo(seat) < 0)
            return;
        for (int i = 0; i < 4; i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (int k = 0; k < held.Count(); k++)
                m_S.Played.Insert(held[k]);
            held.Clear();
        }
        for (int t = 0; t < m_S.TrickCards.Count(); t++)
            m_S.Played.Insert(m_S.TrickCards[t]);
        m_S.TrickCards.Clear();
        m_S.TrickBy.Clear();
        m_S.PassedOut = 1;
        m_S.Phase = OZG_BridgeConst.DONE;
        m_S.Turn = -1;
        m_S.Ended = 1;
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (!Waits(seat))
            return;
        int i = IndexOfSeatNo(seat);
        if (m_S.Phase == OZG_BridgeConst.AUCTION)
        {
            into.Insert("pass");
            if (MayDouble(i))
                into.Insert("dbl");
            if (MayRedouble(i))
                into.Insert("rdbl");
            for (int r = HighestRank() + 1; r < 35; r++)
                into.Insert("bid:" + BidOfRank(r));
            return;
        }
        int hand = m_S.Turn;
        array<string> held = m_S.SeatList[hand].Held;
        string lead = LeadSuit();
        string trump = TrumpSuit();
        for (int k = 0; k < held.Count(); k++)
        {
            if (PlayAllowed(held, held[k], lead, trump, false))
                into.Insert("play:" + held[k]);
        }
    }

    // A strength by high cards: ace 4, king 3, queen 2, jack 1.
    static int HighPoints(array<string> held)
    {
        int hcp = 0;
        for (int i = 0; i < held.Count(); i++)
        {
            int r = OZG_Cards.RankOf(held[i]);
            if (r >= 9)
                hcp = hcp + r - 8;
        }
        return hcp;
    }

    override string BotMove(int seat, OZG_Random chance)
    {
        if (!Waits(seat))
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        int i = IndexOfSeatNo(seat);
        if (m_S.Phase == OZG_BridgeConst.AUCTION)
        {
            // Opens one of its longest suit with 13 points, when nobody bid.
            if (HighestRank() >= 0)
                return "pass";
            array<string> held = m_S.SeatList[i].Held;
            if (HighPoints(held) < 13)
                return "pass";
            string strains = OZG_BridgeConst.STRAINS;
            string suits = OZG_BridgeConst.STRAIN_SUITS;
            int best = 0;
            int bestCount = -1;
            for (int s = 0; s < 4; s++)
            {
                int count = 0;
                string suit = suits.Substring(s, 1);
                for (int k = 0; k < held.Count(); k++)
                {
                    if (OZG_Cards.SuitOf(held[k]) == suit)
                        count++;
                }
                if (count >= bestCount)
                {
                    bestCount = count;
                    best = s;
                }
            }
            return "bid:1" + strains.Substring(best, 1);
        }
        array<string> can = new array<string>();
        Moves(seat, can);
        if (can.Count() == 0)
            return "";
        return can[dice.Below(can.Count())];
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        int colon = move.IndexOf(":");
        string arg = "";
        if (colon > 0)
            arg = move.Substring(colon + 1, move.Length() - colon - 1);
        if (move.IndexOf("play:") == 0)
        {
            into.Insert("STR_OZG_G_LOG_PLAY");
            into.Insert(OZG_Words.CardArg(arg));
            return;
        }
        into.Insert("STR_OZG_BRIDGE_LOG_CALL");
        if (move.IndexOf("bid:") == 0)
            into.Insert("b:" + arg);
        else if (move == "dbl")
            into.Insert("b:X");
        else if (move == "rdbl")
            into.Insert("b:XX");
        else
            into.Insert("b:P");
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
        string text = OZG_BridgeConst.HINT_DEALER + "=" + SeatNoAt(m_S.Dealer).ToString();
        text = text + ";" + OZG_BridgeConst.HINT_DEAL + "=" + m_S.DealNo.ToString();
        text = text + ";" + OZG_BridgeConst.HINT_NS + "=" + m_S.TotalNS.ToString();
        text = text + ";" + OZG_BridgeConst.HINT_EW + "=" + m_S.TotalEW.ToString();
        if (m_S.PassedOut == 1)
            text = text + ";" + OZG_BridgeConst.HINT_REDEAL + "=1";
        return text;
    }

    // ---- the view -------------------------------------------------------------

    protected string ContractArg()
    {
        return "b:" + m_S.BidLevel.ToString() + m_S.BidStrain;
    }

    protected string DoubledKey()
    {
        return OZG_Words.KeyArg("STR_OZG_BRIDGE_X" + m_S.Doubled.ToString());
    }

    override string PublicView()
    {
        if (!m_S)
            return "{}";
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = OZG_BridgeConst.GAME_ID;
        int i;
        for (i = 0; i < 4; i++)
        {
            OZG_BridgeSeat s = m_S.SeatList[i];
            OZG_GSeat g = SeatShown(i);
            int sideTricks = m_S.TricksNS;
            string sideKey = "STR_OZG_BRIDGE_SIDE_NS";
            if (SideOf(i) == 1)
            {
                sideTricks = m_S.TricksEW;
                sideKey = "STR_OZG_BRIDGE_SIDE_EW";
            }
            g.Line = OZG_Words.W2("STR_OZG_BRIDGE_LINE", OZG_Words.KeyArg(sideKey), OZG_Words.NumArg(sideTricks));
            if (m_S.Phase == OZG_BridgeConst.AUCTION)
                g.Role = s.Said;
            else if (i == m_S.Declarer && m_S.PassedOut == 0)
                g.Role = OZG_Words.W0("STR_OZG_BRIDGE_ROLE_DECL");
            else if (i == m_S.Dummy && m_S.PassedOut == 0)
                g.Role = OZG_Words.W0("STR_OZG_BRIDGE_ROLE_DUMMY");
            if (i == m_S.Dealer)
                g.Mark = 1;
            v.SeatList.Insert(g);
            if (Waits(s.SeatNo))
                v.Awaited.Insert(s.SeatNo);
        }
        array<int> by = new array<int>();
        for (i = 0; i < m_S.TrickBy.Count(); i++)
            by.Insert(SeatNoAt(m_S.TrickBy[i]));
        v.Zones.Insert(ZoneOf("trick", OZG_Words.W0("STR_OZG_G_TRICK"), m_S.TrickCards, by));

        bool vulNS = Vulnerable(0, m_S.DealNo, m_S.Dealer);
        bool vulEW = Vulnerable(1, m_S.DealNo, m_S.Dealer);
        string vul = "STR_OZG_BRIDGE_VUL_NONE";
        if (vulNS && vulEW)
            vul = "STR_OZG_BRIDGE_VUL_ALL";
        else if (vulNS)
            vul = "STR_OZG_BRIDGE_VUL_NS";
        else if (vulEW)
            vul = "STR_OZG_BRIDGE_VUL_EW";
        v.Score.Insert(OZG_Words.W2("STR_OZG_BRIDGE_DEAL_LINE", OZG_Words.NumArg(m_S.DealNo), OZG_Words.KeyArg(vul)));
        v.Score.Insert(OZG_Words.W2("STR_OZG_BRIDGE_TOTALS", OZG_Words.NumArg(m_S.TotalNS), OZG_Words.NumArg(m_S.TotalEW)));

        if (m_S.Phase == OZG_BridgeConst.AUCTION)
        {
            v.Trump = "";
            int from = m_S.Calls.Count() - 8;
            if (from < 0)
                from = 0;
            for (i = from; i < m_S.Calls.Count(); i++)
                v.Score.Insert(OZG_Words.W2("STR_OZG_BRIDGE_CALL_LINE", OZG_Words.SeatArg(SeatNoAt(CallerOf(i))), "b:" + m_S.Calls[i]));
            if (m_S.Turn >= 0)
                v.Status = OZG_Words.W1("STR_OZG_BRIDGE_ST_CALL", OZG_Words.SeatArg(SeatNoAt(m_S.Turn)));
        }
        else if (m_S.PassedOut == 0)
        {
            v.Trump = "n";
            string trump = TrumpSuit();
            if (trump != "")
                v.Trump = trump;
            v.Score.Insert(OZG_Words.W3("STR_OZG_BRIDGE_CONTRACT_LINE", OZG_Words.SeatArg(SeatNoAt(m_S.Declarer)), ContractArg(), DoubledKey()));
            if (m_S.Phase == OZG_BridgeConst.PLAY && m_S.Turn >= 0)
            {
                if (m_S.Turn == m_S.Dummy)
                    v.Status = OZG_Words.W2("STR_OZG_BRIDGE_ST_DUMMY", OZG_Words.SeatArg(SeatNoAt(m_S.Declarer)), OZG_Words.SeatArg(SeatNoAt(m_S.Dummy)));
                else
                    v.Status = OZG_Words.W1("STR_OZG_G_ST_PLAY", OZG_Words.SeatArg(SeatNoAt(m_S.Turn)));
            }
        }
        if (m_S.Ended == 1)
        {
            v.Ended = 1;
            if (m_S.PassedOut == 1)
            {
                v.Result = OZG_Words.W0("STR_OZG_BRIDGE_RES_PASSED");
            }
            else
            {
                int side = SideOf(m_S.Declarer);
                int tricks = m_S.TricksNS;
                if (side == 1)
                    tricks = m_S.TricksEW;
                int need = m_S.BidLevel + 6;
                if (tricks >= need)
                    v.Result = OZG_Words.W3("STR_OZG_BRIDGE_RES_MADE", ContractArg(), OZG_Words.NumArg(tricks), OZG_Words.NumArg(m_S.DealScore));
                else
                    v.Result = OZG_Words.W3("STR_OZG_BRIDGE_RES_DOWN", ContractArg(), OZG_Words.NumArg(need - tricks), OZG_Words.NumArg(m_S.DealScore));
            }
            v.Status = v.Result;
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
        // The dummy's own hand lies open: the base gives it as it is.
        OwnHand(i, v);
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int k = 0; k < can.Count(); k++)
        {
            string move = can[k];
            if (move.IndexOf("play:") == 0)
            {
                v.CanPlay.Insert(move);
                continue;
            }
            if (move.IndexOf("bid:") == 0)
            {
                string bid = move.Substring(4, move.Length() - 4);
                int rank = BidRank(bid);
                OZG_GButton b = OZG_GButton.Make(move, OZG_Words.W1("STR_OZG_G_SAY", "b:" + bid), rank / 5, rank % 5);
                v.Buttons.Insert(b);
                continue;
            }
            string call = "P";
            if (move == "dbl")
                call = "X";
            if (move == "rdbl")
                call = "XX";
            OZG_GButton c = OZG_GButton.Make(move, OZG_Words.W1("STR_OZG_G_SAY", "b:" + call), -1, k);
            if (move == "pass")
                c.Hot = 1;
            v.Buttons.Insert(c);
        }
        if (m_S.Phase == OZG_BridgeConst.PLAY && Waits(seat) && m_S.Turn == m_S.Dummy)
            v.Status = OZG_Words.W0("STR_OZG_BRIDGE_YOU_DUMMY");
        return PrivateText(v);
    }

    // ---- saving and checking --------------------------------------------------

    override string SaveState()
    {
        if (!m_S)
            return "";
        string text;
        string err;
        JsonFileLoader<OZG_BridgeState>.MakeData(m_S, text, err, false);
        return text;
    }

    override bool LoadState(string data)
    {
        OZG_BridgeState s = new OZG_BridgeState();
        string err;
        if (!JsonFileLoader<OZG_BridgeState>.LoadData(data, s, err))
            return false;
        if (!s || s.Version != OZG_BridgeConst.VERSION)
            return false;
        if (!s.SeatList || !s.Calls || !s.TrickCards || !s.TrickBy || !s.Played || !s.Dealt)
            return false;
        if (s.SeatList.Count() != 4)
            return false;
        for (int i = 0; i < 4; i++)
        {
            if (!s.SeatList[i] || !s.SeatList[i].Held)
                return false;
        }
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        OZG_CardGame.Remade(s.Dealt, made);
        s.Dealt = made;
        OZG_BridgeState before = m_S;
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
        if (m_S.SeatList.Count() != 4)
        {
            why = "seats: " + m_S.SeatList.Count().ToString();
            return false;
        }
        if (m_S.TrickCards.Count() != m_S.TrickBy.Count() || m_S.TrickCards.Count() > 3)
        {
            why = "the trick does not add up";
            return false;
        }
        if (m_S.Phase == OZG_BridgeConst.PLAY)
        {
            int left = 13 - m_S.TricksNS - m_S.TricksEW;
            for (int i = 0; i < 4; i++)
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
            return "bridge: no deal";
        string text = "bridge k=" + m_S.DealNo.ToString() + " dealer=" + SeatNoAt(m_S.Dealer).ToString();
        text = text + " phase=" + m_S.Phase.ToString();
        string calls = "";
        for (int c = 0; c < m_S.Calls.Count(); c++)
        {
            if (c > 0)
                calls = calls + ",";
            calls = calls + m_S.Calls[c];
        }
        text = text + " calls=" + calls;
        if (m_S.Phase != OZG_BridgeConst.AUCTION && m_S.PassedOut == 0)
        {
            text = text + " contract=" + m_S.BidLevel.ToString() + m_S.BidStrain + "x" + m_S.Doubled.ToString();
            text = text + " decl=" + SeatNoAt(m_S.Declarer).ToString();
            text = text + " tricks=" + m_S.TricksNS.ToString() + ":" + m_S.TricksEW.ToString();
        }
        text = text + " totals=" + m_S.TotalNS.ToString() + ":" + m_S.TotalEW.ToString();
        if (m_S.Ended == 1)
            text = text + " over score=" + m_S.DealScore.ToString() + " to " + m_S.DealSide.ToString();
        else
            text = text + " turn=" + SeatNoAt(m_S.Turn).ToString();
        return text;
    }

    override string Everything()
    {
        if (!m_S)
            return "bridge: no deal";
        string text = "";
        for (int i = 0; i < 4; i++)
        {
            OZG_BridgeSeat s = m_S.SeatList[i];
            text = text + s.SeatNo.ToString() + "[" + OZG_Cards.Line(s.Held) + "] ";
        }
        text = text + "trick[" + OZG_Cards.Line(m_S.TrickCards) + "]";
        return text;
    }
}
