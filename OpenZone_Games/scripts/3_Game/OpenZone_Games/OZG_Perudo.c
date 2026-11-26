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
    protected ref array<int> m_Count;
    protected ref array<ref array<int>> m_Dice;
    protected ref array<int> m_PalDone;
    protected int m_BidCount;
    protected int m_BidFace;
    protected int m_Bidder = -1;
    protected bool m_Palifico;
    // Words: the last showdown.
    protected string m_Shown;

    void OZG_Perudo()
    {
        m_Count = new array<int>();
        m_Dice = new array<ref array<int>>();
        m_PalDone = new array<int>();
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
            return true;
        }
        if (move == OZG_PerudoConst.M_DUDO && m_Bidder >= 0)
        {
            PeShow(m_Turn);
            return true;
        }
        return false;
    }

    // "I do not believe" from the seat of index `doubter`.
    protected void PeShow(int doubter)
    {
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
        m_Shown = OZG_Words.W4("STR_OZG_PERUDO_SHOWN", all, actual.ToString(), m_BidFace.ToString(), OZG_Words.SeatArg(m_Seats[loser]));
        m_Hist.Insert("show " + actual.ToString() + " of " + m_BidCount.ToString() + "x" + m_BidFace.ToString());
        m_Count[loser] = m_Count[loser] - 1;
        if (m_Count[loser] <= 0)
        {
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
            OZG_GSeat g = DgPlate(m_Seats[i], "", OZG_Words.W1("STR_OZG_PERUDO_DICE", m_Count[i].ToString()));
            if (m_Gone.Find(i) >= 0)
                g.Out = 1;
            v.SeatList.Insert(g);
        }
        OZG_GZone bid = new OZG_GZone();
        bid.Id = "bid";
        bid.Label = OZG_Words.W0("STR_OZG_PERUDO_BID_NOW");
        if (m_Bidder >= 0)
            bid.Line = OZG_Words.W3("STR_OZG_PERUDO_BID_LINE", m_BidCount.ToString(), m_BidFace.ToString(), OZG_Words.SeatArg(m_Seats[m_Bidder]));
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
        return DgPublicText(v);
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
