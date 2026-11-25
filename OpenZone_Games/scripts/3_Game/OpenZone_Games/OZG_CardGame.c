// What the games after durak share (stakes-and-games design §2): the cards of
// a deal handed over, the reads and notices of the hands that lie face down,
// the check that every card of the deal is somewhere exactly once, the way a
// hand is sorted and the helpers of the table view (§4).
//
// A game below tells its base where its cards are through the protected
// methods of the first block; its state stays its own class, which JSON saves.
class OZG_CardGame : OZG_Game
{
    // ---- what a game tells its base ------------------------------------------

    // The records of the deal: the state's own array.
    protected array<ref OZG_DealCard> DealtList()
    {
        return null;
    }

    // The seats dealt in, in their order around the mat.
    protected int SeatsIn()
    {
        return 0;
    }

    // The session's number of the seat at index i.
    protected int SeatNoAt(int i)
    {
        return -1;
    }

    // The cards seat index i holds that lie face down for the others.
    protected void HiddenAt(int i, array<string> into)
    {
        into.Clear();
    }

    // The cards of seat index i that lie face up for everybody.
    protected void ShownAt(int i, array<string> into)
    {
        into.Clear();
    }

    // Every other card that lies face up for everybody now: the table.
    protected void TableUp(array<string> into)
    {
        into.Clear();
    }

    // The card on top of a pile that lies back up, "" when none.
    protected string StockTop()
    {
        return "";
    }

    // Every card of the deal, wherever it lies now.
    protected void AllCards(array<string> into)
    {
        into.Clear();
    }

    // ---- the cards of a deal -------------------------------------------------

    // The cards this deal is dealt from: what the session read out of the
    // deck, or the game's whole set when nobody handed any. The records go
    // into `dealt` in the order they were handed over, the names into `deck`.
    protected void DeckOf(array<string> deck, array<ref OZG_DealCard> dealt)
    {
        deck.Clear();
        dealt.Clear();
        int f;
        if (m_Given)
        {
            for (f = 0; f < m_Given.Count(); f++)
            {
                dealt.Insert(m_Given[f].Copy());
                deck.Insert(m_Given[f].Name);
            }
            return;
        }
        CardsUsed(deck);
        for (f = 0; f < deck.Count(); f++)
            dealt.Insert(OZG_DealCard.Make(deck[f], 0));
    }

    protected OZG_DealCard RecOf(string card)
    {
        array<ref OZG_DealCard> dealt = DealtList();
        if (!dealt || card == "")
            return null;
        for (int i = 0; i < dealt.Count(); i++)
        {
            if (dealt[i] && dealt[i].Name == card)
                return dealt[i];
        }
        return null;
    }

    protected int LookOf(string card)
    {
        OZG_DealCard rec = RecOf(card);
        if (!rec)
            return 0;
        return rec.Look;
    }

    protected int IndexOfSeatNo(int seat)
    {
        int n = SeatsIn();
        for (int i = 0; i < n; i++)
        {
            if (SeatNoAt(i) == seat)
                return i;
        }
        return -1;
    }

    // A record of the cards a game reloaded: every record made anew by script
    // (the core's rule for what the JSON loader filled).
    static void Remade(array<ref OZG_DealCard> loaded, array<ref OZG_DealCard> into)
    {
        into.Clear();
        if (!loaded)
            return;
        for (int i = 0; i < loaded.Count(); i++)
        {
            if (loaded[i])
                into.Insert(loaded[i].Copy());
        }
    }

    // ---- a hand, sorted ------------------------------------------------------

    // By suit in the order given (`order` a string of suit letters), and in a
    // suit from the lowest rank up.
    static void SortBySuit(array<string> cards, string order)
    {
        int n = cards.Count();
        for (int i = 1; i < n; i++)
        {
            string c = cards[i];
            int key = SortKey(c, order);
            int j = i - 1;
            while (j >= 0 && SortKey(cards[j], order) > key)
            {
                cards[j + 1] = cards[j];
                j--;
            }
            cards[j + 1] = c;
        }
    }

    protected static int SortKey(string card, string order)
    {
        int suit = order.IndexOf(OZG_Cards.SuitOf(card));
        if (suit < 0)
            suit = 9;
        int rank = OZG_Cards.RankOf(card);
        int copy = OZG_Cards.CopyOf(card);
        int key = suit * 100;
        key = key + rank * 4;
        return key + copy;
    }

    // Takes `n` cards off the top of `stock` into `into`; fewer when the
    // stock runs out. Returns how many it took.
    static int Draw(array<string> stock, array<string> into, int n)
    {
        int took = 0;
        while (took < n && stock.Count() > 0)
        {
            into.Insert(stock[0]);
            stock.RemoveOrdered(0);
            took++;
        }
        return took;
    }

    // ---- reads and notices (card sharp's design §4.4, §4.5) ------------------

    override void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
        if (uid == "")
            return;
        int n = SeatsIn();
        array<string> hidden = new array<string>();
        for (int i = 0; i < n; i++)
        {
            int other = SeatNoAt(i);
            if (other == seat)
                continue;
            HiddenAt(i, hidden);
            for (int h = 0; h < hidden.Count(); h++)
                ReadOne(other, hidden[h], uid, into);
        }
        string top = StockTop();
        if (top != "")
            ReadOne(-1, top, uid, into);
    }

    protected void ReadOne(int where, string card, string uid, array<ref OZG_WRead> into)
    {
        OZG_DealCard rec = RecOf(card);
        if (!rec || !rec.ReadBy(uid))
            return;
        into.Insert(OZG_WRead.Make(where, rec.Look, OZG_Cards.FaceOf(card), rec.Sign));
    }

    override bool AddNoticer(string card, string uid)
    {
        OZG_DealCard rec = RecOf(card);
        if (!rec || rec.Sign <= 0 || uid == "")
            return false;
        if (rec.Noticers.Find(uid) < 0)
            rec.Noticers.Insert(uid);
        return true;
    }

    override void NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)
    {
        into.Clear();
        if (uid == "")
            return;
        int n = SeatsIn();
        int i;
        int h;
        array<string> cards = new array<string>();
        // Backs: the other seats' hidden hands and the top of a pile.
        for (i = 0; i < n; i++)
        {
            int other = SeatNoAt(i);
            if (other == seat)
                continue;
            HiddenAt(i, cards);
            for (h = 0; h < cards.Count(); h++)
                NoticeBack(other, cards[h], uid, into);
        }
        string top = StockTop();
        if (top != "")
            NoticeBack(-1, top, uid, into);
        // Faces: the seat's own hand, every hand lying open, the table.
        int mine = IndexOfSeatNo(seat);
        if (mine >= 0)
        {
            HiddenAt(mine, cards);
            for (h = 0; h < cards.Count(); h++)
                NoticeFace(cards[h], uid, into);
        }
        for (i = 0; i < n; i++)
        {
            ShownAt(i, cards);
            for (h = 0; h < cards.Count(); h++)
                NoticeFace(cards[h], uid, into);
        }
        TableUp(cards);
        for (h = 0; h < cards.Count(); h++)
            NoticeFace(cards[h], uid, into);
    }

    protected void NoticeBack(int where, string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = RecOf(card);
        if (!rec || !rec.NoticedBy(uid) || rec.ReadBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeBack(where, rec.Look, rec.Sign));
    }

    protected void NoticeFace(string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = RecOf(card);
        if (!rec || !rec.NoticedBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeFace(card, rec.Look, rec.Sign));
    }

    // ---- the check of a deal -------------------------------------------------

    // Every card the deal began with is somewhere exactly once.
    protected bool CardsSound(out string why)
    {
        why = "";
        array<ref OZG_DealCard> dealt = DealtList();
        if (!dealt)
        {
            why = "no deal";
            return false;
        }
        array<string> have = new array<string>();
        AllCards(have);
        array<string> want = new array<string>();
        int i;
        for (i = 0; i < dealt.Count(); i++)
        {
            if (!dealt[i])
            {
                why = "a card of the deal has no record";
                return false;
            }
            want.Insert(dealt[i].Name);
        }
        if (have.Count() != want.Count())
        {
            why = "cards: " + have.Count().ToString() + " of " + want.Count().ToString();
            return false;
        }
        for (i = 0; i < have.Count(); i++)
        {
            int at = want.Find(have[i]);
            if (at < 0)
            {
                why = "the card " + have[i] + " is there twice, or is not of this deal";
                return false;
            }
            want.RemoveOrdered(at);
        }
        return true;
    }

    // ---- the table view (design §4) ------------------------------------------

    // A seat as the view shows it: its hidden cards as backs, its open ones as
    // faces. The game adds the words.
    protected OZG_GSeat SeatShown(int i)
    {
        OZG_GSeat g = new OZG_GSeat();
        g.SeatNo = SeatNoAt(i);
        array<string> cards = new array<string>();
        HiddenAt(i, cards);
        int k;
        for (k = 0; k < cards.Count(); k++)
            g.Backs.Insert(LookOf(cards[k]));
        g.Backs.Sort();
        ShownAt(i, cards);
        for (k = 0; k < cards.Count(); k++)
        {
            g.Faces.Insert(cards[k]);
            g.FaceLooks.Insert(LookOf(cards[k]));
        }
        return g;
    }

    // A pile of face-up cards, each with its look and who laid it.
    protected OZG_GZone ZoneOf(string id, string label, array<string> faces, array<int> by)
    {
        OZG_GZone z = new OZG_GZone();
        z.Id = id;
        z.Label = label;
        if (faces)
        {
            for (int k = 0; k < faces.Count(); k++)
            {
                z.Faces.Insert(faces[k]);
                z.Looks.Insert(LookOf(faces[k]));
                int who = -1;
                if (by && k < by.Count())
                    who = by[k];
                z.By.Insert(who);
            }
        }
        return z;
    }

    // A seat's own half: its hand with the look of every card.
    protected void OwnHand(int i, OZG_GPrivate v)
    {
        array<string> cards = new array<string>();
        HiddenAt(i, cards);
        array<string> open = new array<string>();
        ShownAt(i, open);
        for (int k = 0; k < open.Count(); k++)
        {
            if (cards.Find(open[k]) < 0)
                cards.Insert(open[k]);
        }
        for (int h = 0; h < cards.Count(); h++)
        {
            v.Held.Insert(cards[h]);
            v.HeldLooks.Insert(LookOf(cards[h]));
        }
    }

    static string PublicText(OZG_GPublic v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_GPublic>.MakeData(v, text, err, false);
        return text;
    }

    static string PrivateText(OZG_GPrivate v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_GPrivate>.MakeData(v, text, err, false);
        return text;
    }

    // Whose move it is, as everybody reads it.
    protected string WaitWords(array<int> awaited)
    {
        if (!awaited || awaited.Count() == 0)
            return "";
        return OZG_Words.W1("STR_OZG_G_ST_WAIT", OZG_Words.SeatArg(awaited[0]));
    }

    // A whole number out of a hint, as a list of numbers parted by commas.
    static void HintList(string hint, string key, array<int> into)
    {
        into.Clear();
        array<string> pairs = new array<string>();
        hint.Split(";", pairs);
        for (int i = 0; i < pairs.Count(); i++)
        {
            string pair = pairs[i];
            int eq = pair.IndexOf("=");
            if (eq <= 0)
                continue;
            if (pair.Substring(0, eq) != key)
                continue;
            string rest = pair.Substring(eq + 1, pair.Length() - eq - 1);
            array<string> nums = new array<string>();
            rest.Split(",", nums);
            for (int k = 0; k < nums.Count(); k++)
                into.Insert(nums[k].ToInt());
            return;
        }
    }

    static string ListText(array<int> nums)
    {
        string text = "";
        for (int i = 0; i < nums.Count(); i++)
        {
            if (i > 0)
                text = text + ",";
            text = text + nums[i].ToString();
        }
        return text;
    }
}
