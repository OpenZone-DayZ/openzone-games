// The flights of a durak table (design §11, §17): what moved between the view
// the window showed and the one it shows now flies from where it was to where
// it is. The window has drawn the new view already; a card landing where a
// card is drawn hides that card until it lands (OZG_GameMenu.Fly).
//
// What moved is read from three things: the lines the log gained (who laid
// which card, a seat dropped), the bout (did it go on or close), and the
// counts (the stock, the discards, every hand). A card nobody at this table
// can know (another seat's draw) flies as a back; a view that came after
// several changes is flown as far as those three things tell.
class OZG_FlightPlan
{
    static const float ONE = 0.35;        // a card laid, beaten or moved on
    static const float GROUP = 0.45;      // the table swept off or taken
    static const float GROUP_GAP = 0.04;  // between two cards of the group
    static const float DRAW = 0.35;       // a card drawn
    static const float DRAW_GAP = 0.09;   // between two cards drawn
    static const float DEAL = 0.30;       // a card dealt
    static const float LAID_GAP = 0.08;   // between two cards laid in one view
    // More lines than this gained at once: too much happened to tell who laid
    // what, and only the counts are flown.
    static const int MOST_LINES = 6;

    protected OZG_GameMenu m_Menu;
    protected OZG_WView m_WasV;
    protected OZG_DurakPublic m_Was;
    protected OZG_DurakPrivate m_WasOwn;
    protected OZG_WView m_NowV;
    protected OZG_DurakPublic m_Now;
    protected OZG_DurakPrivate m_NowOwn;
    protected string m_Trump;
    protected int m_Flown;
    protected string m_Said;

    // The cards laid in this change, as the log names them: the seat, the
    // card, and the attack card it beat (from 0; -1 for an attack card).
    protected ref array<int> m_LaidSeat;
    protected ref array<string> m_LaidCard;
    protected ref array<int> m_LaidOn;
    protected int m_Dropped;
    // My new hand as the window shows it, the places a card already flies
    // to, the cards I took, the cards I drew.
    protected ref array<string> m_Hand;
    protected ref array<int> m_HandUsed;
    protected ref array<string> m_Taken;
    protected ref array<string> m_MineDrawn;

    void OZG_FlightPlan()
    {
        m_LaidSeat = new array<int>();
        m_LaidCard = new array<string>();
        m_LaidOn = new array<int>();
        m_Hand = new array<string>();
        m_HandUsed = new array<int>();
        m_Taken = new array<string>();
        m_Dropped = -1;
    }

    static void Make(OZG_GameMenu menu, OZG_WView wasV, OZG_DurakPublic was, OZG_DurakPrivate wasOwn, OZG_WView nowV, OZG_DurakPublic now, OZG_DurakPrivate nowOwn)
    {
        if (!menu || !wasV || !nowV || !now)
            return;
        OZG_FlightPlan p = new OZG_FlightPlan();
        p.m_Menu = menu;
        p.m_WasV = wasV;
        p.m_Was = was;
        p.m_WasOwn = wasOwn;
        p.m_NowV = nowV;
        p.m_Now = now;
        p.m_NowOwn = nowOwn;
        p.RunPlan();
    }

    protected void RunPlan()
    {
        if (m_NowV.PlaceId != m_WasV.PlaceId)
            return;
        if (m_NowV.GameId != OZG_DurakConst.GAME_ID)
            return;
        if (m_NowV.St == OZG_Const.ST_LOBBY)
            return;
        m_Trump = OZG_Cards.SuitOf(m_Now.TrumpCard);
        if (m_NowOwn)
            m_Menu.SortHand(m_NowOwn.Held, m_Trump, m_Hand);
        bool dealt = m_NowV.St == OZG_Const.ST_PLAYING && m_WasV.St != OZG_Const.ST_PLAYING;
        if (dealt)
            FlyDeal();
        else if (m_Was && m_WasV.St == OZG_Const.ST_PLAYING)
            FlyMoves();
        if (m_Flown > 0)
            OZG_Log.Dbg("flights rev " + m_NowV.Rev.ToString() + ": " + m_Said);
    }

    // ------------------------------------------------------------ small things

    protected void FlyOne(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        if (!from || !to)
            return;
        if (m_Menu.Fly(picture, pictureLand, from, to, delay, seconds, reveal))
            m_Flown++;
    }

    protected void SayCount(string what, int n)
    {
        if (n <= 0)
            return;
        if (m_Said != "")
            m_Said = m_Said + ", ";
        m_Said = m_Said + what + " " + n.ToString();
    }

    protected static int LookIn(array<int> looks, int i)
    {
        if (!looks || i < 0 || i >= looks.Count())
            return -1;
        return looks[i];
    }

    protected static OZG_DurakSeatPublic SeatIn(OZG_DurakPublic pub, int no)
    {
        if (!pub || !pub.SeatList)
            return null;
        for (int i = 0; i < pub.SeatList.Count(); i++)
        {
            if (pub.SeatList[i].SeatNo == no)
                return pub.SeatList[i];
        }
        return null;
    }

    protected static int CountIn(OZG_DurakPublic pub, int no)
    {
        OZG_DurakSeatPublic g = SeatIn(pub, no);
        if (!g)
            return 0;
        return g.CardCount;
    }

    // The look of a card of a hand of mine (HeldLooks lies beside Held).
    protected static int MyLook(OZG_DurakPrivate own, string card)
    {
        if (!own || !own.Held)
            return -1;
        return LookIn(own.HeldLooks, own.Held.Find(card));
    }

    // The place of `card` in my new hand that no flight lands on yet; -1 when
    // it is not there.
    protected int HandPlaceOf(string card)
    {
        for (int i = 0; i < m_Hand.Count(); i++)
        {
            if (m_Hand[i] != card || m_HandUsed.Find(i) >= 0)
                continue;
            m_HandUsed.Insert(i);
            return i;
        }
        return -1;
    }

    // ------------------------------------------------------------ the deal

    // A new deal: the shuffle is heard first (OZG_SoundIds.SHUFFLE), then the
    // cards fly from the stock a seat at a time, as durak deals them, through
    // the run the deal sounds; the trump turns up as the run ends.
    protected void FlyDeal()
    {
        float start = OZG_Sounds.LengthOf(OZG_SoundIds.SHUFFLE) / 1000.0;
        float run = OZG_Sounds.LengthOf(OZG_SoundIds.DEAL) / 1000.0;
        int total = 0;
        int s;
        for (s = 0; s < m_Now.SeatList.Count(); s++)
            total = total + m_Now.SeatList[s].CardCount;
        if (total == 0)
            return;
        float gap = 0.05;
        if (total > 1)
            gap = (run - DEAL) / (total - 1);
        if (gap < 0.02)
            gap = 0.02;
        OZG_CardSpot from = m_Menu.SpotOfStock(m_Now.StockCount);
        int k = 0;
        for (s = 0; s < m_Now.SeatList.Count(); s++)
        {
            OZG_DurakSeatPublic g = m_Now.SeatList[s];
            if (g.SeatNo == m_NowV.Mine)
            {
                for (int h = 0; h < m_Hand.Count(); h++)
                {
                    int myLook = MyLook(m_NowOwn, m_Hand[h]);
                    string myBack = m_Menu.PictureOfBack(myLook);
                    string myFace = m_Menu.PictureOfFace(m_Hand[h], myLook);
                    OZG_CardSpot mine = m_Menu.SpotOfHand(h, m_Hand.Count());
                    FlyOne(myBack, myFace, from, mine, start + k * gap, DEAL, OZG_GameMenu.SlotOfHand(h));
                    k++;
                }
                continue;
            }
            for (int i = 0; i < g.CardCount; i++)
            {
                OZG_CardSpot theirs = m_Menu.SpotOfFan(m_NowV, g.SeatNo, i, g.CardCount);
                string theirBack = m_Menu.PictureOfBack(LookIn(g.Backs, i));
                FlyOne(theirBack, "", from, theirs, start + k * gap, DEAL, OZG_GameMenu.SlotOfFan(g.SeatNo, i));
                k++;
            }
        }
        SayCount("dealt", k);
        // The trump, turned up as the run ends; six seats take the whole deck
        // and then it is the last card dealt, in a hand.
        if (m_Now.TrumpCard == "" || m_Now.StockCount == 0)
            return;
        string trumpBack = m_Menu.PictureOfBack(m_Now.TrumpLook);
        string trumpFace = m_Menu.PictureOfFace(m_Now.TrumpCard, m_Now.TrumpLook);
        FlyOne(trumpBack, trumpFace, from, m_Menu.SpotOfTrump(), start + run, DEAL, OZG_GameMenu.SLOT_TRUMP);
        SayCount("trump", 1);
    }

    // ------------------------------------------------------------ a change in play

    // The lines the log gained since the view shown before. The log keeps its
    // last OZG_Const.LOG_LINES lines: the fewest lines added that make the two
    // agree are the new ones.
    protected void ReadLog()
    {
        array<string> was = m_WasV.Log;
        array<string> now = m_NowV.Log;
        int nw = was.Count();
        int nn = now.Count();
        int added = -1;
        for (int k = 0; k <= nn && added < 0; k++)
        {
            int keep = nn - k;
            if (keep > nw)
                continue;
            bool same = true;
            for (int i = 0; i < keep && same; i++)
            {
                if (now[i] != was[nw - keep + i])
                    same = false;
            }
            if (same)
                added = k;
        }
        if (added < 0 || added > MOST_LINES)
            return;
        for (int j = nn - added; j < nn; j++)
            ReadLine(now[j]);
    }

    // "<seat> a:<card>", "<seat> t:<card>", "<seat> d:<card>:<n>" are cards
    // laid; "drop <seat>" a seat whose cards went to the discards.
    protected void ReadLine(string line)
    {
        int space = line.IndexOf(" ");
        if (space <= 0)
            return;
        string head = line.Substring(0, space);
        string rest = line.Substring(space + 1, line.Length() - space - 1);
        if (head == "drop")
        {
            m_Dropped = rest.ToInt();
            return;
        }
        if (rest.Length() < 3)
            return;
        string kind = rest.Substring(0, 2);
        if (kind != "a:" && kind != "t:" && kind != "d:")
            return;
        string card = OZG_Cards.OfMove(rest);
        if (card == "")
            return;
        int target = -1;
        if (kind == "d:")
        {
            array<string> parts = new array<string>();
            rest.Split(":", parts);
            if (parts.Count() == 3)
                target = parts[2].ToInt() - 1;
        }
        m_LaidSeat.Insert(head.ToInt());
        m_LaidCard.Insert(card);
        m_LaidOn.Insert(target);
    }

    protected void FlyMoves()
    {
        ReadLog();
        int wasN = m_Was.AttackCards.Count();
        int nowN = m_Now.AttackCards.Count();
        bool goesOn = nowN >= wasN;
        for (int a = 0; a < wasN && goesOn; a++)
        {
            if (m_Now.AttackCards[a] != m_Was.AttackCards[a])
                goesOn = false;
        }
        bool closed = wasN > 0 && !goesOn;

        // Cards laid: from their seats onto the table where they lie now. One
        // the table never showed (the bout closed in the same change) leaves
        // with the table.
        array<int> unseenSeat = new array<int>();
        array<string> unseenCard = new array<string>();
        float next = 0;
        int laid = 0;
        for (int c = 0; c < m_LaidCard.Count(); c++)
        {
            bool beats = m_LaidOn[c] >= 0;
            int where = -1;
            if (!closed)
                where = OnTable(m_LaidCard[c], m_LaidOn[c]);
            if (where < 0)
            {
                unseenSeat.Insert(m_LaidSeat[c]);
                unseenCard.Insert(m_LaidCard[c]);
                continue;
            }
            int look = TableLook(where, beats);
            string face = m_Menu.PictureOfFace(m_LaidCard[c], look);
            string first = FirstPicture(m_LaidSeat[c], m_LaidCard[c], look);
            OZG_CardSpot from = FromSeat(m_LaidSeat[c], m_LaidCard[c]);
            OZG_CardSpot to = m_Menu.SpotOfBout(nowN, where, beats);
            float at = laid * LAID_GAP;
            FlyOne(first, face, from, to, at, ONE, OZG_GameMenu.SlotOfBout(where, beats));
            next = at + ONE;
            laid++;
        }
        SayCount("laid", laid);

        int taker = -1;
        int taken = 0;
        if (closed || m_Dropped >= 0)
            next = FlyLeaving(closed, unseenSeat, unseenCard, next, taker, taken);
        FlyDraws(next, taker, taken);
    }

    // Where a card laid now lies on the new table: over the attack card it
    // beat (`target`, from 0), or the last attack card of that name; -1 when
    // it is not there.
    protected int OnTable(string card, int target)
    {
        if (target >= 0)
        {
            if (target < m_Now.DefenceCards.Count() && m_Now.DefenceCards[target] == card)
                return target;
            return -1;
        }
        for (int i = m_Now.AttackCards.Count() - 1; i >= 0; i--)
        {
            if (m_Now.AttackCards[i] == card)
                return i;
        }
        return -1;
    }

    protected int TableLook(int where, bool beats)
    {
        if (beats)
            return LookIn(m_Now.DefenceLooks, where);
        return LookIn(m_Now.AttackLooks, where);
    }

    // My card shows its face all the way; another seat's is a back until
    // halfway.
    protected string FirstPicture(int seat, string card, int look)
    {
        if (seat == m_NowV.Mine)
            return m_Menu.PictureOfFace(card, look);
        return m_Menu.PictureOfBack(look);
    }

    // Where a card a seat laid comes from: mine from its place in my hand as
    // it was, another seat's from the middle of its fan as it was.
    protected OZG_CardSpot FromSeat(int seat, string card)
    {
        if (seat == m_WasV.Mine && m_WasOwn)
        {
            array<string> before = new array<string>();
            m_Menu.SortHand(m_WasOwn.Held, m_Trump, before);
            int at = before.Find(card);
            if (at >= 0)
                return m_Menu.SpotOfHand(at, before.Count());
        }
        OZG_CardSpot fan = m_Menu.SpotOfFan(m_WasV, seat, -1, 0);
        if (fan)
            return fan;
        return m_Menu.SpotOfFan(m_NowV, seat, -1, 0);
    }

    // The table, and a dropped seat's hand, leave: to the discards, or into
    // the hand of the defender who took. Returns when the last card lands;
    // `taker` and `taken` say who took how many.
    protected float FlyLeaving(bool closed, array<int> unseenSeat, array<string> unseenCard, float start, out int taker, out int taken)
    {
        taker = -1;
        taken = 0;
        array<ref OZG_CardSpot> froms = new array<ref OZG_CardSpot>();
        array<string> cards = new array<string>();
        array<int> looks = new array<int>();
        int i;
        if (closed)
        {
            int wasN = m_Was.AttackCards.Count();
            for (i = 0; i < wasN; i++)
            {
                froms.Insert(m_Menu.SpotOfBout(wasN, i, false));
                cards.Insert(m_Was.AttackCards[i]);
                looks.Insert(LookIn(m_Was.AttackLooks, i));
                string beatBy = "";
                if (i < m_Was.DefenceCards.Count())
                    beatBy = m_Was.DefenceCards[i];
                if (beatBy == "")
                    continue;
                froms.Insert(m_Menu.SpotOfBout(wasN, i, true));
                cards.Insert(beatBy);
                looks.Insert(LookIn(m_Was.DefenceLooks, i));
            }
            for (i = 0; i < unseenCard.Count(); i++)
            {
                froms.Insert(FromSeat(unseenSeat[i], unseenCard[i]));
                cards.Insert(unseenCard[i]);
                looks.Insert(-1);
            }
        }
        if (m_Dropped >= 0)
        {
            OZG_DurakSeatPublic g = SeatIn(m_Was, m_Dropped);
            int held = 0;
            if (g)
                held = g.CardCount;
            for (i = 0; i < held; i++)
            {
                froms.Insert(m_Menu.SpotOfFan(m_WasV, m_Dropped, i, held));
                cards.Insert("");
                looks.Insert(LookIn(g.Backs, i));
            }
        }
        int n = froms.Count();
        if (n == 0)
            return start;
        bool swept = m_Now.DiscardCount > m_Was.DiscardCount;
        if (!swept)
        {
            taker = m_Was.DefenderSeat;
            taken = n;
            return FlyTaken(froms, cards, looks, start);
        }
        int heapWas = m_Menu.HeapOf(m_Was.DiscardCount);
        int heapNow = m_Menu.HeapOf(m_Now.DiscardCount);
        for (i = 0; i < n; i++)
        {
            int k = heapWas + i / 2;
            if (k >= heapNow)
                k = heapNow - 1;
            if (k < 0)
                k = 0;
            string down = m_Menu.PictureOfBack(looks[i]);
            string up = down;
            if (cards[i] != "")
                up = m_Menu.PictureOfFace(cards[i], looks[i]);
            FlyOne(up, down, froms[i], m_Menu.SpotOfDiscards(k), start + i * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfHeap(k));
        }
        SayCount("swept", n);
        return start + (n - 1) * GROUP_GAP + GROUP;
    }

    // The table taken: into my hand, at the places the cards hold there now;
    // or into the taker's fan, as its newest backs.
    protected float FlyTaken(array<ref OZG_CardSpot> froms, array<string> cards, array<int> looks, float start)
    {
        int n = froms.Count();
        int taker = m_Was.DefenderSeat;
        if (taker == m_NowV.Mine)
        {
            for (int i = 0; i < n; i++)
            {
                if (cards[i] == "")
                    continue;
                m_Taken.Insert(cards[i]);
                int place = HandPlaceOf(cards[i]);
                if (place < 0)
                    continue;
                int myLook = MyLook(m_NowOwn, cards[i]);
                string myFace = m_Menu.PictureOfFace(cards[i], myLook);
                OZG_CardSpot mine = m_Menu.SpotOfHand(place, m_Hand.Count());
                FlyOne(myFace, "", froms[i], mine, start + i * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfHand(place));
            }
        }
        else
        {
            int count = CountIn(m_Now, taker);
            int drew = DrewNow(taker, taker, n);
            int firstIdx = count - drew - n;
            if (firstIdx < 0)
                firstIdx = 0;
            OZG_DurakSeatPublic g = SeatIn(m_Now, taker);
            for (int j = 0; j < n; j++)
            {
                int idx = firstIdx + j;
                if (idx >= count)
                    idx = count - 1;
                int backLook = -1;
                if (g)
                    backLook = LookIn(g.Backs, idx);
                string down = m_Menu.PictureOfBack(backLook);
                string up = down;
                if (cards[j] != "")
                    up = m_Menu.PictureOfFace(cards[j], looks[j]);
                OZG_CardSpot fan = m_Menu.SpotOfFan(m_NowV, taker, idx, count);
                FlyOne(up, down, froms[j], fan, start + j * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfFan(taker, idx));
            }
        }
        SayCount("taken", n);
        return start + (n - 1) * GROUP_GAP + GROUP;
    }

    // How many cards the seat `no` drew in this change: what its hand gained,
    // with the cards it laid given back and the cards it took kept out.
    protected int DrewNow(int no, int taker, int taken)
    {
        int gained = CountIn(m_Now, no) - CountIn(m_Was, no);
        for (int i = 0; i < m_LaidSeat.Count(); i++)
        {
            if (m_LaidSeat[i] == no)
                gained++;
        }
        if (no == taker)
            gained = gained - taken;
        if (gained < 0)
            gained = 0;
        return gained;
    }

    // The seats in the order they draw: from the first attacker of the bout
    // round the table, the defender last (durak's rule 8).
    protected void DrawOrder(array<int> order)
    {
        order.Clear();
        int n = m_Was.SeatList.Count();
        int from = 0;
        for (int i = 0; i < n; i++)
        {
            if (m_Was.SeatList[i].SeatNo == m_Was.OpenerSeat)
                from = i;
        }
        for (int k = 0; k < n; k++)
        {
            int no = m_Was.SeatList[(from + k) % n].SeatNo;
            if (no != m_Was.DefenderSeat)
                order.Insert(no);
        }
        if (m_Was.DefenderSeat >= 0)
            order.Insert(m_Was.DefenderSeat);
    }

    // Cards drawn after a bout or a drop, each from the top of the stock as it
    // was; the last card of the stock is the trump. A dropped seat draws none.
    protected void FlyDraws(float start, int taker, int taken)
    {
        int drawn = m_Was.StockCount - m_Now.StockCount;
        if (drawn <= 0)
            return;
        array<int> order = new array<int>();
        DrawOrder(order);
        int k = 0;
        for (int i = 0; i < order.Count(); i++)
        {
            int no = order[i];
            if (no == m_Dropped)
                continue;
            int got = DrewNow(no, taker, taken);
            for (int j = 0; j < got && k < drawn; j++)
            {
                bool last = k == drawn - 1 && m_Now.StockCount == 0;
                FlyDrawn(no, got, j, k, last, start);
                k++;
            }
        }
        SayCount("drawn", k);
    }

    // The j-th of the `got` cards the seat `no` drew, the k-th drawn of all.
    protected void FlyDrawn(int no, int got, int j, int k, bool last, float start)
    {
        OZG_CardSpot from = m_Menu.SpotOfStock(m_Was.StockCount - k);
        if (last)
            from = m_Menu.SpotOfTrump();
        float at = start + k * DRAW_GAP;
        if (no == m_NowV.Mine)
        {
            string card = MineDrawn(j);
            int place = -1;
            if (card != "")
                place = HandPlaceOf(card);
            if (place < 0)
                return;
            int myLook = MyLook(m_NowOwn, card);
            string myFace = m_Menu.PictureOfFace(card, myLook);
            string myFirst = m_Menu.PictureOfBack(myLook);
            if (last)
                myFirst = myFace;
            OZG_CardSpot mine = m_Menu.SpotOfHand(place, m_Hand.Count());
            FlyOne(myFirst, myFace, from, mine, at, DRAW, OZG_GameMenu.SlotOfHand(place));
            return;
        }
        int count = CountIn(m_Now, no);
        int idx = count - got + j;
        if (idx < 0)
            idx = 0;
        OZG_DurakSeatPublic g = SeatIn(m_Now, no);
        int backLook = -1;
        if (g)
            backLook = LookIn(g.Backs, idx);
        string down = m_Menu.PictureOfBack(backLook);
        string up = down;
        if (last)
            up = m_Menu.PictureOfFace(m_Now.TrumpCard, m_Now.TrumpLook);
        OZG_CardSpot fan = m_Menu.SpotOfFan(m_NowV, no, idx, count);
        FlyOne(up, down, from, fan, at, DRAW, OZG_GameMenu.SlotOfFan(no, idx));
    }

    // The j-th card I drew in this change: my new hand, less what I held
    // before and what I took, in the order my hand is shown.
    protected string MineDrawn(int j)
    {
        if (!m_MineDrawn)
        {
            m_MineDrawn = new array<string>();
            array<string> known = new array<string>();
            if (m_WasOwn && m_WasOwn.Held)
                known.Copy(m_WasOwn.Held);
            for (int t = 0; t < m_Taken.Count(); t++)
                known.Insert(m_Taken[t]);
            for (int i = 0; i < m_Hand.Count(); i++)
            {
                int at = known.Find(m_Hand[i]);
                if (at >= 0)
                {
                    known.Remove(at);
                    continue;
                }
                m_MineDrawn.Insert(m_Hand[i]);
            }
        }
        if (j < 0 || j >= m_MineDrawn.Count())
            return "";
        return m_MineDrawn[j];
    }
}
