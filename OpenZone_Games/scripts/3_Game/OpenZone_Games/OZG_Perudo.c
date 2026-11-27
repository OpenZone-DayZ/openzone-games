// Perudo (boards and dice design §6.5): each player's own five dice under his
// can, seen by him alone. In turn each bids on the dice of the whole table
// ("four fives"), ones being any face; a bid must raise the last (more dice, or
// as many of a higher face); bidding on ones halves the count, leaving them
// doubles it and one more. Instead of raising, a player says "I do not
// believe": all show; whoever was wrong loses a die (the bidder if the bid was
// short, the doubter if not) and begins the next round. A player down to one
// die for the first time begins a round in which ones are not wild and the
// face does not change (palifico). The last with dice wins.
//
// Moves: "bid:<count>:<face>", "dudo".
class OZG_PerudoConst
{
    static const string GAME_ID = "perudo";
    static const string LABEL = "STR_OZG_GAME_PERUDO";
    static const string M_BID = "bid:";
    static const string M_DUDO = "dudo";
    static const int DICE = 5;
    // The bids offered for each face: the least that raises and this many more.
    static const int BID_SPAN = 3;
}

class OZG_Perudo extends OZG_DiceGame
{
    // A seat's dice stay under its cup: no plate shows them.
    override protected bool DgShowsLast()
    {
        return false;
    }

    protected ref array<int> m_Count;
    protected ref array<ref array<int>> m_Dice;
    protected ref array<int> m_PalDone;
    protected int m_BidCount;
    protected int m_BidFace;
    protected int m_Bidder = -1;
    protected bool m_Palifico;
    // Words: the last showdown.
    protected string m_Shown;
    // The last showdown for the window: the bid, whether ones were wild, who lost a die,
    // every seat's dice as they lay.
    protected int m_LastBidCount;
    protected int m_LastBidFace;
    protected bool m_LastPalifico;
    protected int m_LastLoser = -1;
    protected int m_LastBidder = -1;
    protected ref array<string> m_LastShown;

    void OZG_Perudo()
    {
        m_Count = new array<int>();
        m_Dice = new array<ref array<int>>();
        m_PalDone = new array<int>();
        m_LastShown = new array<string>();
    }

    override string GameId()
    {
        return OZG_PerudoConst.GAME_ID;
    }

    override string LabelKey()
    {
        return OZG_PerudoConst.LABEL;
    }

    override OZG_Game Fresh()
    {
        return new OZG_Perudo();
    }

    override protected bool OwnOnly()
    {
        return true;
    }

    // The dice lie under the cans until somebody says "I do not believe".
    protected bool m_Opened;

    override bool ThrowsHidden()
    {
        return true;
    }

    override bool TakeShown()
    {
        bool was = m_Opened;
        m_Opened = false;
        return was;
    }

    // A seat throws the dice it has left: the first of its can.
    override int DiceThrown(int seat)
    {
        int i = m_Seats.Find(seat);
        if (i < 0 || i >= m_Count.Count() || m_Gone.Find(i) >= 0)
            return 0;
        return m_Count[i];
    }

    // A die may be swapped into his can at the start of a round, in his turn, before the
    // first bid (the canvas's perudo); it lies from the next round's throw.
    override bool ThrowsNext(int seat)
    {
        if (m_Over || seat != DgCurrent())
            return false;
        return m_BidCount == 0;
    }

    override protected bool DgBegin(out string refusal)
    {
        refusal = "";
        m_Count.Clear();
        m_Dice.Clear();
        m_PalDone.Clear();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            m_Count.Insert(OZG_PerudoConst.DICE);
            m_Dice.Insert(new array<int>());
            m_PalDone.Insert(0);
        }
        m_Shown = "";
        m_Palifico = false;
        m_LastShown.Clear();
        m_LastLoser = -1;
        m_LastBidder = -1;
        PeRound(m_Turn);
        return true;
    }

    // A round: everybody with dice throws them; `starter` bids first.
    protected void PeRound(int starter)
    {
        m_BidCount = 0;
        m_BidFace = 0;
        m_Bidder = -1;
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            if (m_Gone.Find(i) >= 0)
            {
                m_Dice[i].Clear();
                continue;
            }
            DgRollInto(m_Seats[i], m_Count[i], m_Dice[i]);
        }
        m_Turn = starter;
        if (m_Gone.Find(m_Turn) >= 0)
            m_Turn = DgNextIndex(m_Turn);
        if (m_Palifico)
            Say(OZG_Words.W0("STR_OZG_PERUDO_SAID_PALIFICO"));
        else
            Say(OZG_Words.W0("STR_OZG_PERUDO_SAID_ROUND"));
    }

    protected int InPlay()
    {
        int n = 0;
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            if (m_Gone.Find(i) < 0)
                n = n + m_Count[i];
        }
        return n;
    }

    // Does a bid of `c` dice of face `f` raise the last?
    protected bool PeRaises(int c, int f)
    {
        if (c < 1 || f < 1 || f > 6 || c > InPlay())
            return false;
        if (m_BidCount == 0)
        {
            if (m_Palifico)
                return true;
            return f >= 2;
        }
        if (m_Palifico)
            return f == m_BidFace && c > m_BidCount;
        if (m_BidFace != 1)
        {
            if (f == 1)
                return c * 2 >= m_BidCount;
            if (c > m_BidCount)
                return true;
            return c == m_BidCount && f > m_BidFace;
        }
        if (f == 1)
            return c > m_BidCount;
        return c >= 2 * m_BidCount + 1;
    }

    protected void PeBids(array<string> into)
    {
        into.Clear();
        int most = InPlay();
        for (int f = 1; f <= 6; f++)
        {
            int least = -1;
            for (int c = 1; c <= most; c++)
            {
                if (PeRaises(c, f))
                {
                    least = c;
                    break;
                }
            }
            if (least < 0)
                continue;
            for (int k = least; k <= most && k < least + OZG_PerudoConst.BID_SPAN; k++)
            {
                if (PeRaises(k, f))
                    into.Insert(OZG_PerudoConst.M_BID + k.ToString() + ":" + f.ToString());
            }
        }
    }

    override protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (seat != DgCurrent())
            return false;
        if (move.IndexOf(OZG_PerudoConst.M_BID) == 0)
        {
            array<string> parts = new array<string>();
            move.Split(":", parts);
            if (parts.Count() != 3)
                return false;
            int c = parts[1].ToInt();
            int f = parts[2].ToInt();
            if (!PeRaises(c, f))
                return false;
            m_BidCount = c;
            m_BidFace = f;
            m_Bidder = m_Turn;
            m_Turn = DgNextIndex(m_Turn);
            SayMove(OZG_Words.W2("STR_OZG_PERUDO_SAID_BID", OZG_Words.SeatArg(seat), "f:" + c.ToString() + ":" + f.ToString()));
            return true;
        }
        if (move == OZG_PerudoConst.M_DUDO && m_Bidder >= 0)
        {
            PeShow(m_Turn);
            return true;
        }
        return false;
    }

    // The dice everybody showed, for the window: they lie open until the next bid.
    protected void PeKeepShown(int loserSeat)
    {
        m_LastBidCount = m_BidCount;
        m_LastBidFace = m_BidFace;
        m_LastPalifico = m_Palifico;
        m_LastLoser = loserSeat;
        m_LastBidder = -1;
        if (m_Bidder >= 0 && m_Bidder < m_Seats.Count())
            m_LastBidder = m_Seats[m_Bidder];
        m_LastShown.Clear();
        for (int s = 0; s < m_Seats.Count(); s++)
        {
            array<int> laid = m_Dice[s];
            string line = OZG_DiceText.Line(laid);
            m_LastShown.Insert(line);
        }
    }

    // "I do not believe" from the seat of index `doubter`.
    protected void PeShow(int doubter)
    {
        m_Opened = true;
        int actual = 0;
        string all = "";
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            if (m_Gone.Find(i) >= 0)
                continue;
            array<int> dice = m_Dice[i];
            for (int k = 0; k < dice.Count(); k++)
            {
                int v = dice[k];
                bool wild = v == 1 && !m_Palifico && m_BidFace != 1;
                if (v == m_BidFace || wild)
                    actual++;
            }
            if (all != "")
                all = all + " / ";
            all = all + OZG_DiceText.Line(dice);
        }
        int loser = doubter;
        if (actual < m_BidCount)
            loser = m_Bidder;
        // the log: who doubted, how many there were, whether the bid held, who loses a die
        string held = "k:STR_OZG_PERUDO_HELD";
        if (actual < m_BidCount)
            held = "k:STR_OZG_PERUDO_FELL";
        string showKey = "STR_OZG_PERUDO_SAID_SHOW";
        if (!m_Palifico && m_BidFace != 1)
            showKey = "STR_OZG_PERUDO_SAID_SHOW_ONES";
        // every word made into a local before a call takes them: written inline as
        // W4(key, all, actual.ToString(), m_BidFace.ToString(), OZG_Words.SeatArg(..)) the
        // count came out as the seat's number SeatArg made inside (the stand's bench,
        // 2026-11-26; dayz-modding enforce-script.md, two calls of one function)
        int doubterSeat = m_Seats[doubter];
        int loserSeat = m_Seats[loser];
        string actualText = actual.ToString();
        string faceText = m_BidFace.ToString();
        string doubterArg = OZG_Words.SeatArg(doubterSeat);
        string loserArg = OZG_Words.SeatArg(loserSeat);
        string counted = "f:" + actualText + ":" + faceText;
        SayMove(OZG_Words.W4(showKey, doubterArg, counted, held, loserArg));
        m_Shown = OZG_Words.W4("STR_OZG_PERUDO_SHOWN", all, actualText, faceText, loserArg);
        m_Hist.Insert("show " + actualText + " of " + m_BidCount.ToString() + "x" + faceText);
        PeKeepShown(loserSeat);
        m_Count[loser] = m_Count[loser] - 1;
        if (m_Count[loser] <= 0)
        {
            Say(OZG_Words.W1("STR_OZG_PERUDO_SAID_OUT", OZG_Words.SeatArg(m_Seats[loser])));
            m_Gone.Insert(loser);
            if (DgLeft() <= 1)
            {
                DgWonBy(DgNextIndex(loser), "STR_OZG_PERUDO_END");
                return;
            }
            m_Palifico = false;
            PeRound(DgNextIndex(loser));
            return;
        }
        // Down to one die for the first time: a palifico round.
        bool pal = m_Count[loser] == 1 && m_PalDone[loser] == 0;
        m_Palifico = pal;
        if (pal)
            m_PalDone[loser] = 1;
        PeRound(loser);
    }

    override protected void DgPassTurn()
    {
        // An absent player says nothing: he believes, and the round goes on.
        m_Turn = DgNextIndex(m_Turn);
    }

    override void Moves(int seat, array<string> into)
    {
        into.Clear();
        if (m_Over || seat != DgCurrent())
            return;
        PeBids(into);
        if (m_Bidder >= 0)
            into.Insert(OZG_PerudoConst.M_DUDO);
    }

    override string PublicView()
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = GameId();
        for (int i = 0; i < m_Seats.Count(); i++)
        {
            // the words made first (dice poker's view says why)
            int plateSeat = m_Seats[i];
            string plateCount = m_Count[i].ToString();
            string plateLine = OZG_Words.W1("STR_OZG_PERUDO_DICE", plateCount);
            OZG_GSeat g = DgPlate(plateSeat, "", plateLine);
            if (m_Gone.Find(i) >= 0)
                g.Out = 1;
            v.SeatList.Insert(g);
        }
        OZG_GZone bid = new OZG_GZone();
        bid.Id = "bid";
        bid.Label = OZG_Words.W0("STR_OZG_PERUDO_BID_NOW");
        if (m_Bidder >= 0)
        {
            // the words made first (PeShow says why)
            string bidCount = m_BidCount.ToString();
            string bidFace = m_BidFace.ToString();
            string bidBy = OZG_Words.SeatArg(m_Seats[m_Bidder]);
            bid.Line = OZG_Words.W3("STR_OZG_PERUDO_BID_LINE", bidCount, bidFace, bidBy);
        }
        else
            bid.Line = OZG_Words.W0("STR_OZG_PERUDO_NO_BID");
        v.Zones.Insert(bid);
        if (m_Shown != "")
        {
            OZG_GZone shown = new OZG_GZone();
            shown.Id = "shown";
            shown.Label = OZG_Words.W0("STR_OZG_PERUDO_LAST");
            shown.Line = m_Shown;
            v.Zones.Insert(shown);
        }
        // the window: the bid (count, face, bidder's seat), palifico, the dice on the table;
        // the last showdown while no new bid stands (its bid, wild ones, the loser, whose
        // bid it was, the dice of every seat)
        OZG_GZone state = new OZG_GZone();
        state.Id = "perudo";
        int bidder = -1;
        if (m_Bidder >= 0)
            bidder = m_Seats[m_Bidder];
        int pal = 0;
        if (m_Palifico)
            pal = 1;
        state.Line = m_BidCount.ToString() + "|" + m_BidFace.ToString() + "|" + bidder.ToString() + "|" + pal.ToString() + "|" + InPlay().ToString();
        v.Zones.Insert(state);
        // (the last showdown of a game stays: the window shows it under its last words)
        bool openNow = m_Bidder < 0 || m_Over;
        if (openNow && m_LastShown.Count() > 0)
        {
            OZG_GZone open = new OZG_GZone();
            open.Id = "perudo_open";
            int lastPal = 0;
            if (m_LastPalifico)
                lastPal = 1;
            string line = m_LastBidCount.ToString() + "|" + m_LastBidFace.ToString() + "|" + lastPal.ToString() + "|" + m_LastLoser.ToString();
            line = line + "|" + m_LastBidder.ToString();
            for (int k = 0; k < m_LastShown.Count(); k++)
                line = line + "|" + m_Seats[k].ToString() + "=" + m_LastShown[k];
            open.Line = line;
            v.Zones.Insert(open);
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
            string key = "STR_OZG_DICE_TURN";
            if (m_Palifico)
                key = "STR_OZG_PERUDO_TURN_PALIFICO";
            v.Status = OZG_Words.W1(key, OZG_Words.SeatArg(DgCurrent()));
        }
        return DgPublicOut(v);
    }

    override string SeatView(int seat)
    {
        OZG_GPrivate v = new OZG_GPrivate();
        int i = m_Seats.Find(seat);
        if (i >= 0 && m_Gone.Find(i) < 0)
            v.Status = OZG_Words.W1("STR_OZG_PERUDO_YOURS", OZG_DiceText.Line(m_Dice[i]));
        array<string> can = new array<string>();
        Moves(seat, can);
        for (int k = 0; k < can.Count(); k++)
        {
            string m = can[k];
            if (m == OZG_PerudoConst.M_DUDO)
            {
                OZG_GButton dudo = OZG_GButton.Make(m, OZG_Words.W0("STR_OZG_PERUDO_BTN_DUDO"), -1, 0);
                dudo.Hot = 1;
                v.Buttons.Insert(dudo);
                continue;
            }
            array<string> parts = new array<string>();
            m.Split(":", parts);
            int face = parts[2].ToInt();
            OZG_GButton b = OZG_GButton.Make(m, OZG_Words.W2("STR_OZG_PERUDO_BTN_BID", parts[1], parts[2]), face - 1, v.Buttons.Count());
            v.Buttons.Insert(b);
        }
        // every bid that raises the last: the window's count and face are checked on them
        if (!m_Over && seat == DgCurrent())
        {
            int most = InPlay();
            for (int f = 1; f <= 6; f++)
            {
                for (int c = 1; c <= most; c++)
                {
                    if (PeRaises(c, f))
                        v.CanPlay.Insert(OZG_PerudoConst.M_BID + c.ToString() + ":" + f.ToString());
                }
            }
        }
        return DgPrivateText(v);
    }

    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move == OZG_PerudoConst.M_DUDO)
        {
            into.Insert("STR_OZG_PERUDO_LOG_DUDO");
            return;
        }
        if (move.IndexOf(OZG_PerudoConst.M_BID) == 0)
        {
            array<string> parts = new array<string>();
            move.Split(":", parts);
            if (parts.Count() == 3)
            {
                into.Insert("STR_OZG_PERUDO_LOG_BID");
                into.Insert(parts[1]);
                into.Insert(parts[2]);
                return;
            }
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
        // Doubts a bid above what a third of the unseen dice and its own make.
        if (m_Bidder >= 0)
        {
            int mine = 0;
            array<int> own = m_Dice[i];
            for (int k = 0; k < own.Count(); k++)
            {
                int v = own[k];
                if (v == m_BidFace || (v == 1 && !m_Palifico && m_BidFace != 1))
                    mine++;
            }
            int unseen = InPlay() - m_Count[i];
            if (m_BidCount > mine + unseen / 3 + 1)
                return OZG_PerudoConst.M_DUDO;
        }
        array<string> bids = new array<string>();
        PeBids(bids);
        if (bids.Count() == 0)
            return OZG_PerudoConst.M_DUDO;
        return bids[dice.Below(bids.Count())];
    }

    override string Everything()
    {
        string s = super.Everything() + " counts=" + Ints(m_Count, ",");
        s = s + " bid=" + m_BidCount.ToString() + "x" + m_BidFace.ToString();
        for (int i = 0; i < m_Dice.Count(); i++)
            s = s + " d" + i.ToString() + "=" + Ints(m_Dice[i], "");
        return s;
    }
}
