// Durak, podkidnoy and perevodnoy (design §8). The rules and nothing else: no
// world, no players, no network. A seat comes in as the session's number;
// inside, it is its index in the list of the seats dealt in.
//
// A bout: the opener lays a card; every seat but the defender may lay cards of
// ranks already on the table; the defender beats each, takes, or (perevodnoy)
// moves the attack on. The bout closes by itself once nothing more can come:
// Settle() is asked after every move.
class OZG_Durak : OZG_Game
{
    protected ref OZG_DurakState m_S;

    // ---- what the lobby asks -------------------------------------------------

    override string GameId()
    {
        return OZG_DurakConst.GAME_ID;
    }

    override string LabelKey()
    {
        return "STR_OZG_GAME_DURAK";
    }

    override int MinPlayers()
    {
        return 2;
    }

    override int MaxPlayers()
    {
        return 6;
    }

    override void CardsUsed(array<string> into)
    {
        OZG_Cards.FromRank(OZG_DurakConst.LOWEST, into);
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        OZG_Option variant = new OZG_Option();
        variant.OptionId = OZG_DurakConst.OPT_VARIANT;
        variant.LabelKey = "STR_OZG_DURAK_VARIANT";
        variant.ValueIds.Insert(OZG_DurakConst.VARIANT_THROW);
        variant.ValueKeys.Insert("STR_OZG_DURAK_THROW");
        variant.ValueIds.Insert(OZG_DurakConst.VARIANT_TRANSFER);
        variant.ValueKeys.Insert("STR_OZG_DURAK_TRANSFER");
        variant.DefaultId = OZG_DurakConst.VARIANT_THROW;
        into.Insert(variant);
    }

    override OZG_Game Fresh()
    {
        return new OZG_Durak();
    }

    // ---- the deal (rules 1 and 2) --------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        int n = seats.Count();
        if (n < MinPlayers() || n > MaxPlayers())
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }

        OZG_DurakState s = new OZG_DurakState();
        s.Version = OZG_DurakConst.VERSION;
        string variant = OZG_Game.OptionOf(options, OZG_DurakConst.OPT_VARIANT, OZG_DurakConst.VARIANT_THROW);
        if (variant == OZG_DurakConst.VARIANT_TRANSFER)
            s.Transfer = 1;

        // The cards of this deal: what the session read out of the deck, or
        // the game's whole set when nobody handed any. The state keeps its
        // own record of them, in the order they were handed over.
        array<string> deck = new array<string>();
        int f;
        if (m_Given)
        {
            for (f = 0; f < m_Given.Count(); f++)
            {
                s.Dealt.Insert(m_Given[f].Copy());
                deck.Insert(m_Given[f].Name);
            }
        }
        else
        {
            CardsUsed(deck);
            for (f = 0; f < deck.Count(); f++)
                s.Dealt.Insert(OZG_DealCard.Make(deck[f], 0));
        }
        // Six to each seat at the least; the trump is then the last card dealt.
        if (deck.Count() < n * OZG_DurakConst.HAND)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
        // A deal with no chance handed in takes the real one. Into a local: a
        // parameter is not a strong reference and cannot take `new`.
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        dice.Shuffle(deck);

        // Six to each seat, a seat at a time; what is left is the stock.
        int at = 0;
        for (int i = 0; i < n; i++)
        {
            OZG_DurakSeat seat = new OZG_DurakSeat();
            seat.SeatNo = seats[i];
            for (int k = 0; k < OZG_DurakConst.HAND; k++)
            {
                seat.Held.Insert(deck[at]);
                at++;
            }
            s.SeatList.Insert(seat);
        }
        while (at < deck.Count())
        {
            s.StockCards.Insert(deck[at]);
            at++;
        }

        // The trump is the last card of the deck: the bottom of the stock, face
        // up. Six seats take the whole deck, and then it is the last card
        // dealt: shown, and kept in that hand.
        s.TrumpCard = deck[deck.Count() - 1];
        s.FirstBout = 1;

        m_S = s;
        OpenBout(FirstOpener(hint));
        // Heard: the shuffle, the deal's quick run, the trump turned up.
        HearSound(OZG_SoundIds.SHUFFLE);
        HearSound(OZG_SoundIds.DEAL);
        HearSound(OZG_SoundIds.TRUMP);
        return true;
    }

    // Who lays the first card of the game (rule 2). After a game with a durak
    // the seat before him, so that he defends; otherwise the holder of the
    // lowest trump; with no trump dealt, the first seat after the host.
    protected int FirstOpener(string hint)
    {
        int n = m_S.SeatList.Count();
        int durak = OZG_Game.HintInt(hint, OZG_DurakConst.HINT_DURAK, -1);
        int durakAt = IndexOfSeat(durak);
        if (durakAt >= 0)
            return (durakAt + n - 1) % n;

        string trump = OZG_Cards.SuitOf(m_S.TrumpCard);
        int lowestAt = -1;
        int lowestRank = 99;
        for (int i = 0; i < n; i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (int k = 0; k < held.Count(); k++)
            {
                if (OZG_Cards.SuitOf(held[k]) != trump)
                    continue;
                int rank = OZG_Cards.RankOf(held[k]);
                if (rank < lowestRank)
                {
                    lowestRank = rank;
                    lowestAt = i;
                }
            }
        }
        if (lowestAt >= 0)
            return lowestAt;

        int host = OZG_Game.HintInt(hint, OZG_Const.HINT_HOST, -1);
        for (int j = 0; j < n; j++)
        {
            if (m_S.SeatList[j].SeatNo > host)
                return j;
        }
        return 0;
    }

    // ---- a move --------------------------------------------------------------

    // On the wire (design §8): `a:<card>` attack or throw in, `d:<card>:<n>`
    // beat the n-th attack card (counted from 1), `t:<card>` move the attack
    // on, `take`, `pass`.
    override bool Act(int seat, string move, out string refusal)
    {
        refusal = "";
        if (!m_S || m_S.Ended == 1)
        {
            refusal = OZG_Const.NO_OVER;
            return false;
        }
        int i = IndexOfSeat(seat);
        if (i < 0 || m_S.SeatList[i].Gone != OZG_DurakConst.IN_PLAY)
        {
            refusal = OZG_Const.NO_SEAT;
            return false;
        }

        array<string> parts = new array<string>();
        move.Split(":", parts);
        string verb = "";
        if (parts.Count() > 0)
            verb = parts[0];

        bool done = false;
        if (verb == "a" && parts.Count() == 2)
            done = MoveAttack(i, parts[1], refusal);
        else if (verb == "d" && parts.Count() == 3)
            done = MoveDefend(i, parts[1], parts[2].ToInt(), refusal);
        else if (verb == "t" && parts.Count() == 2)
            done = MoveTransfer(i, parts[1], refusal);
        else if (verb == "take" && parts.Count() == 1)
            done = MoveTake(i, refusal);
        else if (verb == "pass" && parts.Count() == 1)
            done = MovePass(i, refusal);
        else
            refusal = OZG_Const.NO_MOVE;

        if (!done)
            return false;
        Settle();
        return true;
    }

    // a:<card> -- the opener's first card, or one more card beside the attack
    // (rules 3 and 4).
    protected bool MoveAttack(int i, string card, out string refusal)
    {
        if (i == m_S.Defender)
        {
            refusal = OZG_DurakConst.NO_DEFENDER;
            return false;
        }
        array<string> held = m_S.SeatList[i].Held;
        int at = held.Find(card);
        if (at < 0)
        {
            refusal = OZG_Const.NO_CARD;
            return false;
        }
        if (m_S.AttackCards.Count() == 0)
        {
            if (i != m_S.Opener)
            {
                refusal = OZG_DurakConst.NO_TURN;
                return false;
            }
        }
        else
        {
            if (m_S.AttackCards.Count() >= m_S.AttackLimit)
            {
                refusal = OZG_DurakConst.NO_LIMIT;
                return false;
            }
            if (!RankOnTable(OZG_Cards.RankOf(card)))
            {
                refusal = OZG_DurakConst.NO_RANK;
                return false;
            }
        }

        held.RemoveOrdered(at);
        m_S.AttackCards.Insert(card);
        m_S.DefenceCards.Insert("");
        ClearPassed();
        HearSound(OZG_SoundIds.LAY);
        return true;
    }

    // d:<card>:<n> -- beat the n-th attack card (rule 5).
    protected bool MoveDefend(int i, string card, int nth, out string refusal)
    {
        if (i != m_S.Defender)
        {
            refusal = OZG_DurakConst.NO_NOT_DEFENDER;
            return false;
        }
        if (m_S.Taking == 1)
        {
            refusal = OZG_DurakConst.NO_TAKING;
            return false;
        }
        array<string> held = m_S.SeatList[i].Held;
        int at = held.Find(card);
        if (at < 0)
        {
            refusal = OZG_Const.NO_CARD;
            return false;
        }
        int target = nth - 1;
        if (target < 0 || target >= m_S.AttackCards.Count())
        {
            refusal = OZG_DurakConst.NO_TARGET;
            return false;
        }
        if (m_S.DefenceCards[target] != "")
        {
            refusal = OZG_DurakConst.NO_TARGET;
            return false;
        }
        if (!Beats(card, m_S.AttackCards[target]))
        {
            refusal = OZG_DurakConst.NO_BEAT;
            return false;
        }

        held.RemoveOrdered(at);
        m_S.DefenceCards[target] = card;
        ClearPassed();
        HearSound(OZG_SoundIds.BEAT);
        return true;
    }

    // t:<card> -- perevodnoy: a card of the attack's rank laid beside it, and
    // the attack moves on to the next seat in play (rule 7).
    protected bool MoveTransfer(int i, string card, out string refusal)
    {
        if (i != m_S.Defender)
        {
            refusal = OZG_DurakConst.NO_NOT_DEFENDER;
            return false;
        }
        if (!TransferOpen(refusal))
            return false;
        array<string> held = m_S.SeatList[i].Held;
        int at = held.Find(card);
        if (at < 0)
        {
            refusal = OZG_Const.NO_CARD;
            return false;
        }
        if (OZG_Cards.RankOf(card) != OZG_Cards.RankOf(m_S.AttackCards[0]))
        {
            refusal = OZG_DurakConst.NO_TRANSFER_RANK;
            return false;
        }

        held.RemoveOrdered(at);
        m_S.AttackCards.Insert(card);
        m_S.DefenceCards.Insert("");
        // The one who moved it on attacks now; the limit is counted anew.
        m_S.Defender = NextLive(i);
        m_S.AttackLimit = LimitFor(m_S.Defender);
        ClearPassed();
        // Moved on: a card laid on the table like any other.
        HearSound(OZG_SoundIds.LAY);
        return true;
    }

    // May the attack be moved on now, whatever the card? Says why not.
    protected bool TransferOpen(out string refusal)
    {
        refusal = "";
        if (m_S.Transfer != 1)
        {
            refusal = OZG_DurakConst.NO_TRANSFER_OFF;
            return false;
        }
        if (m_S.AttackCards.Count() == 0)
        {
            refusal = OZG_DurakConst.NO_EMPTY;
            return false;
        }
        if (m_S.Taking == 1 || BeatenCount() > 0)
        {
            refusal = OZG_DurakConst.NO_TRANSFER_LATE;
            return false;
        }
        // The next seat has to hold a card for every card that will lie there.
        int next = NextLive(m_S.Defender);
        if (next < 0)
        {
            refusal = OZG_DurakConst.NO_TRANSFER_FEW;
            return false;
        }
        if (m_S.SeatList[next].Held.Count() < m_S.AttackCards.Count() + 1)
        {
            refusal = OZG_DurakConst.NO_TRANSFER_FEW;
            return false;
        }
        return true;
    }

    // take -- the defender takes everything once nobody throws in any more.
    protected bool MoveTake(int i, out string refusal)
    {
        if (i != m_S.Defender)
        {
            refusal = OZG_DurakConst.NO_NOT_DEFENDER;
            return false;
        }
        if (m_S.AttackCards.Count() == 0)
        {
            refusal = OZG_DurakConst.NO_EMPTY;
            return false;
        }
        if (m_S.Taking == 1)
        {
            refusal = OZG_DurakConst.NO_TAKING;
            return false;
        }
        if (!HasUnbeaten())
        {
            refusal = OZG_DurakConst.NO_TAKE;
            return false;
        }

        m_S.Taking = 1;
        // The attackers are asked once more: they may add to what he takes.
        ClearPassed();
        return true;
    }

    // pass -- "nothing more from me", until a new card is laid.
    protected bool MovePass(int i, out string refusal)
    {
        if (i == m_S.Defender)
        {
            refusal = OZG_DurakConst.NO_DEFENDER;
            return false;
        }
        if (m_S.AttackCards.Count() == 0)
        {
            refusal = OZG_DurakConst.NO_EMPTY;
            return false;
        }
        if (m_S.SeatList[i].Passed == 1 || !CanThrow(i))
        {
            refusal = OZG_DurakConst.NO_PASS;
            return false;
        }

        m_S.SeatList[i].Passed = 1;
        return true;
    }

    // ---- the bout ------------------------------------------------------------

    // Starts a bout with this seat as the opener.
    protected void OpenBout(int opener)
    {
        m_S.Opener = opener;
        m_S.Defender = NextLive(opener);
        m_S.AttackCards.Clear();
        m_S.DefenceCards.Clear();
        m_S.Taking = 0;
        ClearPassed();
        m_S.AttackLimit = LimitFor(m_S.Defender);
    }

    // No more attack cards than the defender holds now, and no more than six;
    // in the first bout of a game, five (rule 4).
    protected int LimitFor(int defender)
    {
        int most = OZG_DurakConst.LIMIT;
        if (m_S.FirstBout == 1)
            most = OZG_DurakConst.LIMIT_FIRST;
        int held = m_S.SeatList[defender].Held.Count();
        if (held < most)
            return held;
        return most;
    }

    // After every move: closes the bout once everything is beaten, or the
    // defender takes, and every seat that could still throw in has passed
    // (rule 6). A seat with nothing to throw in is passed by itself.
    protected void Settle()
    {
        if (m_S.AttackCards.Count() == 0)
            return;
        if (m_S.Taking == 0 && HasUnbeaten())
            return;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (m_S.SeatList[i].Passed == 0 && CanThrow(i))
                return;
        }
        CloseBout();
    }

    protected void CloseBout()
    {
        int defender = m_S.Defender;
        bool took = m_S.Taking == 1;
        int i;
        if (took)
        {
            array<string> held = m_S.SeatList[defender].Held;
            for (i = 0; i < m_S.AttackCards.Count(); i++)
            {
                held.Insert(m_S.AttackCards[i]);
                if (m_S.DefenceCards[i] != "")
                    held.Insert(m_S.DefenceCards[i]);
            }
        }
        else
        {
            for (i = 0; i < m_S.AttackCards.Count(); i++)
            {
                m_S.DiscardCards.Insert(m_S.AttackCards[i]);
                m_S.DiscardCards.Insert(m_S.DefenceCards[i]);
            }
        }
        if (took)
            HearSound(OZG_SoundIds.TAKE);
        else
            HearSound(OZG_SoundIds.SWEEP);
        m_S.AttackCards.Clear();
        m_S.DefenceCards.Clear();
        m_S.Taking = 0;
        m_S.FirstBout = 0;
        ClearPassed();

        DrawAll(m_S.Opener, defender);
        MarkOut();
        if (Finish(false))
            return;

        // A defender who beat everything attacks next; one who took is skipped
        // (rule 9).
        int opener = defender;
        if (took || !Live(defender))
            opener = NextLive(defender);
        OpenBout(opener);
    }

    // Everyone draws up to six while the stock lasts: from this seat around
    // the mat, the defender last (rule 8). With no defender (-1), simply
    // around the mat.
    protected void DrawAll(int from, int defender)
    {
        int before = m_S.StockCards.Count();
        int n = m_S.SeatList.Count();
        for (int k = 0; k < n; k++)
        {
            int i = (from + k) % n;
            if (i == defender)
                continue;
            DrawUp(i);
        }
        if (defender >= 0)
            DrawUp(defender);
        if (m_S.StockCards.Count() < before)
            HearSound(OZG_SoundIds.DRAW);
    }

    protected void DrawUp(int i)
    {
        OZG_DurakSeat seat = m_S.SeatList[i];
        if (seat.Gone != OZG_DurakConst.IN_PLAY)
            return;
        while (seat.Held.Count() < OZG_DurakConst.HAND && m_S.StockCards.Count() > 0)
        {
            seat.Held.Insert(m_S.StockCards[0]);
            m_S.StockCards.RemoveOrdered(0);
        }
    }

    // A seat with no cards once the stock is empty is out (rule 10).
    protected void MarkOut()
    {
        if (m_S.StockCards.Count() > 0)
            return;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat seat = m_S.SeatList[i];
            if (seat.Gone == OZG_DurakConst.IN_PLAY && seat.Held.Count() == 0)
                seat.Gone = OZG_DurakConst.GONE_OUT;
        }
    }

    // Ends the game when fewer than two seats are in play: the last one with
    // cards is the durak, nobody left is a tie. A game that ends because a seat
    // was dropped has no durak (rule 11).
    protected bool Finish(bool afterDrop)
    {
        int inPlay = 0;
        int last = -1;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (m_S.SeatList[i].Gone == OZG_DurakConst.IN_PLAY)
            {
                inPlay++;
                last = i;
            }
        }
        if (inPlay >= 2)
            return false;

        m_S.Ended = 1;
        m_S.DurakSeat = -1;
        m_S.Tie = 0;
        if (!afterDrop)
        {
            if (inPlay == 1)
                m_S.DurakSeat = m_S.SeatList[last].SeatNo;
            else
                m_S.Tie = 1;
        }
        return true;
    }

    // The seat is out in mid-game: its hand and whatever lies on the table go
    // to the discards and nobody takes; everyone draws; the first seat in play
    // after it attacks (rule 11).
    override void DropSeat(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return;
        int at = IndexOfSeat(seat);
        if (at < 0)
            return;
        OZG_DurakSeat dropped = m_S.SeatList[at];
        if (dropped.Gone != OZG_DurakConst.IN_PLAY)
            return;

        int i;
        int before = m_S.DiscardCards.Count();
        for (i = 0; i < dropped.Held.Count(); i++)
            m_S.DiscardCards.Insert(dropped.Held[i]);
        dropped.Held.Clear();
        dropped.Gone = OZG_DurakConst.GONE_DROPPED;

        for (i = 0; i < m_S.AttackCards.Count(); i++)
        {
            m_S.DiscardCards.Insert(m_S.AttackCards[i]);
            if (m_S.DefenceCards[i] != "")
                m_S.DiscardCards.Insert(m_S.DefenceCards[i]);
        }
        m_S.AttackCards.Clear();
        m_S.DefenceCards.Clear();
        m_S.Taking = 0;
        ClearPassed();
        // Its hand and the table swept to the discards.
        if (m_S.DiscardCards.Count() > before)
            HearSound(OZG_SoundIds.SWEEP);

        DrawAll((at + 1) % m_S.SeatList.Count(), -1);
        MarkOut();
        if (Finish(true))
            return;
        OpenBout(NextLive(at));
    }

    // ---- who is waited for ---------------------------------------------------

    override int HeldCount(int seat)
    {
        if (!m_S)
            return 0;
        int at = IndexOfSeat(seat);
        if (at < 0)
            return 0;
        return m_S.SeatList[at].Held.Count();
    }

    // The game stands still for: the opener while the table is empty; the
    // defender while a card lies unbeaten and he has not said "take"; and, once
    // the bout could close, every seat that may still throw in and has not
    // passed.
    override bool Waits(int seat)
    {
        if (!m_S || m_S.Ended == 1)
            return false;
        int i = IndexOfSeat(seat);
        if (i < 0 || m_S.SeatList[i].Gone != OZG_DurakConst.IN_PLAY)
            return false;
        if (m_S.AttackCards.Count() == 0)
            return i == m_S.Opener;
        if (i == m_S.Defender)
        {
            if (m_S.Taking == 1)
                return false;
            return HasUnbeaten();
        }
        if (m_S.Taking == 0 && HasUnbeaten())
            return false;
        if (m_S.SeatList[i].Passed == 1)
            return false;
        return CanThrow(i);
    }

    // Throwing in is a move a seat need not make; the opener's first card and
    // the defender's answer are not.
    override bool Skippable(int seat)
    {
        if (!Waits(seat))
            return false;
        if (m_S.AttackCards.Count() == 0)
            return false;
        return IndexOfSeat(seat) != m_S.Defender;
    }

    override void SkipSeat(int seat)
    {
        if (!Skippable(seat))
            return;
        string refusal;
        Act(seat, "pass", refusal);
    }

    // What this seat may do now. The window lights these; a check plays them.
    override void Moves(int seat, array<string> into)
    {
        if (!m_S || m_S.Ended == 1)
            return;
        int i = IndexOfSeat(seat);
        if (i < 0 || m_S.SeatList[i].Gone != OZG_DurakConst.IN_PLAY)
            return;
        array<string> held = m_S.SeatList[i].Held;
        int k;

        if (m_S.AttackCards.Count() == 0)
        {
            if (i != m_S.Opener)
                return;
            for (k = 0; k < held.Count(); k++)
                into.Insert("a:" + held[k]);
            return;
        }

        if (i == m_S.Defender)
        {
            if (m_S.Taking == 1)
                return;
            for (int a = 0; a < m_S.AttackCards.Count(); a++)
            {
                if (m_S.DefenceCards[a] != "")
                    continue;
                int nth = a + 1;
                for (k = 0; k < held.Count(); k++)
                {
                    if (Beats(held[k], m_S.AttackCards[a]))
                        into.Insert("d:" + held[k] + ":" + nth.ToString());
                }
            }
            string why;
            if (TransferOpen(why))
            {
                int rank = OZG_Cards.RankOf(m_S.AttackCards[0]);
                for (k = 0; k < held.Count(); k++)
                {
                    if (OZG_Cards.RankOf(held[k]) == rank)
                        into.Insert("t:" + held[k]);
                }
            }
            if (HasUnbeaten())
                into.Insert("take");
            return;
        }

        if (m_S.AttackCards.Count() < m_S.AttackLimit)
        {
            for (k = 0; k < held.Count(); k++)
            {
                if (RankOnTable(OZG_Cards.RankOf(held[k])))
                    into.Insert("a:" + held[k]);
            }
        }
        if (m_S.SeatList[i].Passed == 0 && CanThrow(i))
            into.Insert("pass");
    }

    // ---- the table, read -----------------------------------------------------

    protected int IndexOfSeat(int seat)
    {
        if (!m_S)
            return -1;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            if (m_S.SeatList[i].SeatNo == seat)
                return i;
        }
        return -1;
    }

    // The look of a card's back, by what the deal began with; 0 for a card the
    // deal does not know.
    protected int LookOf(string card)
    {
        if (!m_S || card == "")
            return 0;
        for (int i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (m_S.Dealt[i] && m_S.Dealt[i].Name == card)
                return m_S.Dealt[i].Look;
        }
        return 0;
    }

    protected int SeatNoAt(int i)
    {
        if (i < 0 || i >= m_S.SeatList.Count())
            return -1;
        return m_S.SeatList[i].SeatNo;
    }

    // In play, with something to play or to draw: a seat that has shed its
    // last card after the stock ran out is not, even before the bout closes.
    protected bool Live(int i)
    {
        OZG_DurakSeat seat = m_S.SeatList[i];
        if (seat.Gone != OZG_DurakConst.IN_PLAY)
            return false;
        if (seat.Held.Count() > 0)
            return true;
        return m_S.StockCards.Count() > 0;
    }

    // The next such seat after this one, around the mat; -1 when there is no
    // other.
    protected int NextLive(int from)
    {
        int n = m_S.SeatList.Count();
        for (int k = 1; k < n; k++)
        {
            int i = (from + k) % n;
            if (Live(i))
                return i;
        }
        return -1;
    }

    protected void ClearPassed()
    {
        for (int i = 0; i < m_S.SeatList.Count(); i++)
            m_S.SeatList[i].Passed = 0;
    }

    protected bool HasUnbeaten()
    {
        for (int i = 0; i < m_S.DefenceCards.Count(); i++)
        {
            if (m_S.DefenceCards[i] == "")
                return true;
        }
        return false;
    }

    protected int BeatenCount()
    {
        int beaten = 0;
        for (int i = 0; i < m_S.DefenceCards.Count(); i++)
        {
            if (m_S.DefenceCards[i] != "")
                beaten++;
        }
        return beaten;
    }

    protected bool RankOnTable(int rank)
    {
        int i;
        for (i = 0; i < m_S.AttackCards.Count(); i++)
        {
            if (OZG_Cards.RankOf(m_S.AttackCards[i]) == rank)
                return true;
        }
        for (i = 0; i < m_S.DefenceCards.Count(); i++)
        {
            if (m_S.DefenceCards[i] == "")
                continue;
            if (OZG_Cards.RankOf(m_S.DefenceCards[i]) == rank)
                return true;
        }
        return false;
    }

    // May this seat lay one more card beside the attack now?
    protected bool CanThrow(int i)
    {
        if (i == m_S.Defender)
            return false;
        if (m_S.SeatList[i].Gone != OZG_DurakConst.IN_PLAY)
            return false;
        if (m_S.AttackCards.Count() == 0)
            return false;
        if (m_S.AttackCards.Count() >= m_S.AttackLimit)
            return false;
        array<string> held = m_S.SeatList[i].Held;
        for (int k = 0; k < held.Count(); k++)
        {
            if (RankOnTable(OZG_Cards.RankOf(held[k])))
                return true;
        }
        return false;
    }

    // A higher card of the same suit, or a trump over what is not one (rule 5).
    protected bool Beats(string card, string target)
    {
        string suit = OZG_Cards.SuitOf(card);
        if (suit == OZG_Cards.SuitOf(target))
            return OZG_Cards.RankOf(card) > OZG_Cards.RankOf(target);
        return suit == OZG_Cards.SuitOf(m_S.TrumpCard);
    }

    protected string VariantId()
    {
        if (m_S && m_S.Transfer == 1)
            return OZG_DurakConst.VARIANT_TRANSFER;
        return OZG_DurakConst.VARIANT_THROW;
    }

    // ---- the end, the views, the save ----------------------------------------

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
        return OZG_DurakConst.HINT_DURAK + "=" + m_S.DurakSeat.ToString();
    }

    override string PublicView()
    {
        if (!m_S)
            return "{}";
        OZG_DurakPublic v = new OZG_DurakPublic();
        v.GameId = OZG_DurakConst.GAME_ID;
        v.VariantId = VariantId();
        v.TrumpCard = m_S.TrumpCard;
        v.StockCount = m_S.StockCards.Count();
        v.DiscardCount = m_S.DiscardCards.Count();
        v.AttackCards.Copy(m_S.AttackCards);
        v.DefenceCards.Copy(m_S.DefenceCards);
        v.OpenerSeat = SeatNoAt(m_S.Opener);
        v.DefenderSeat = SeatNoAt(m_S.Defender);
        v.AttackLimit = m_S.AttackLimit;
        v.Taking = m_S.Taking;
        v.Ended = m_S.Ended;
        v.DurakSeat = m_S.DurakSeat;
        v.Tie = m_S.Tie;
        v.TrumpLook = LookOf(m_S.TrumpCard);
        if (m_S.StockCards.Count() > 0)
            v.StockTop = LookOf(m_S.StockCards[0]);
        for (int t = 0; t < m_S.AttackCards.Count(); t++)
        {
            v.AttackLooks.Insert(LookOf(m_S.AttackCards[t]));
            if (m_S.DefenceCards[t] != "")
                v.DefenceLooks.Insert(LookOf(m_S.DefenceCards[t]));
            else
                v.DefenceLooks.Insert(-1);
        }
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat seat = m_S.SeatList[i];
            OZG_DurakSeatPublic shown = new OZG_DurakSeatPublic();
            shown.SeatNo = seat.SeatNo;
            shown.CardCount = seat.Held.Count();
            for (int b = 0; b < seat.Held.Count(); b++)
                shown.Backs.Insert(LookOf(seat.Held[b]));
            shown.Backs.Sort();
            shown.Gone = seat.Gone;
            shown.Passed = seat.Passed;
            v.SeatList.Insert(shown);
            if (Waits(seat.SeatNo))
                v.Awaited.Insert(seat.SeatNo);
        }
        string text;
        string err;
        JsonFileLoader<OZG_DurakPublic>.MakeData(v, text, err, false);
        return text;
    }

    override string SeatView(int seat)
    {
        OZG_DurakPrivate v = new OZG_DurakPrivate();
        int i = IndexOfSeat(seat);
        if (i >= 0)
        {
            v.Held.Copy(m_S.SeatList[i].Held);
            for (int h = 0; h < v.Held.Count(); h++)
                v.HeldLooks.Insert(LookOf(v.Held[h]));
            Moves(seat, v.CanPlay);
        }
        string text;
        string err;
        JsonFileLoader<OZG_DurakPrivate>.MakeData(v, text, err, false);
        return text;
    }

    override void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
        if (!m_S || uid == "")
            return;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat other = m_S.SeatList[i];
            if (other.SeatNo == seat)
                continue;
            for (int h = 0; h < other.Held.Count(); h++)
                ReadOne(other.SeatNo, other.Held[h], uid, into);
        }
        // The top of the stock; the last card of the stock is the trump, face
        // up already.
        if (m_S.StockCards.Count() > 1)
            ReadOne(-1, m_S.StockCards[0], uid, into);
    }

    protected void ReadOne(int where, string card, string uid, array<ref OZG_WRead> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.ReadBy(uid))
            return;
        into.Insert(OZG_WRead.Make(where, rec.Look, OZG_Cards.FaceOf(card), rec.Sign));
    }

    protected OZG_DealCard DealtCard(string card)
    {
        if (!m_S || card == "")
            return null;
        for (int i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (m_S.Dealt[i] && m_S.Dealt[i].Name == card)
                return m_S.Dealt[i];
        }
        return null;
    }

    override bool AddNoticer(string card, string uid)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || rec.Sign <= 0 || uid == "")
            return false;
        if (rec.Noticers.Find(uid) < 0)
            rec.Noticers.Insert(uid);
        return true;
    }

    override void NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)
    {
        into.Clear();
        if (!m_S || uid == "")
            return;
        int i;
        int h;
        // Backs: the other seats' hands and the top of the stock, as the reads.
        for (i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat other = m_S.SeatList[i];
            if (other.SeatNo == seat)
                continue;
            for (h = 0; h < other.Held.Count(); h++)
                NoticeBack(other.SeatNo, other.Held[h], uid, into);
        }
        if (m_S.StockCards.Count() > 1)
            NoticeBack(-1, m_S.StockCards[0], uid, into);
        // Faces: the seat's own hand, the table, the trump while it lies
        // under the stock (the window draws it there only then).
        int mine = IndexOfSeat(seat);
        if (mine >= 0)
        {
            array<string> held = m_S.SeatList[mine].Held;
            for (h = 0; h < held.Count(); h++)
                NoticeFace(held[h], uid, into);
        }
        for (i = 0; i < m_S.AttackCards.Count(); i++)
        {
            NoticeFace(m_S.AttackCards[i], uid, into);
            NoticeFace(m_S.DefenceCards[i], uid, into);
        }
        if (m_S.StockCards.Count() > 0)
            NoticeFace(m_S.TrumpCard, uid, into);
    }

    protected void NoticeBack(int where, string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.NoticedBy(uid) || rec.ReadBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeBack(where, rec.Look, rec.Sign));
    }

    protected void NoticeFace(string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.NoticedBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeFace(card, rec.Look, rec.Sign));
    }

    override string SaveState()
    {
        if (!m_S)
            return "";
        string text;
        string err;
        JsonFileLoader<OZG_DurakState>.MakeData(m_S, text, err, false);
        return text;
    }

    override bool LoadState(string data)
    {
        OZG_DurakState s = new OZG_DurakState();
        string err;
        if (!JsonFileLoader<OZG_DurakState>.LoadData(data, s, err))
            return false;
        if (!s)
            return false;
        // A game saved before cards were things: it was dealt from the whole
        // set, every back the deck's own.
        if (s.Version == 1)
        {
            s.Dealt = new array<ref OZG_DealCard>();
            array<string> whole = new array<string>();
            CardsUsed(whole);
            for (int old = 0; old < whole.Count(); old++)
                s.Dealt.Insert(OZG_DealCard.Make(whole[old], 0));
            s.Version = OZG_DurakConst.VERSION;
        }
        // A game saved before marks: its records carry the name and the look
        // and nothing more, and a member the file did not carry is raw memory.
        if (s.Version == 2 && s.Dealt)
        {
            array<ref OZG_DealCard> plain = new array<ref OZG_DealCard>();
            for (int was = 0; was < s.Dealt.Count(); was++)
            {
                if (s.Dealt[was])
                    plain.Insert(OZG_DealCard.Make(s.Dealt[was].Name, s.Dealt[was].Look));
            }
            s.Dealt = plain;
            s.Version = OZG_DurakConst.VERSION;
        }
        if (s.Version != OZG_DurakConst.VERSION)
            return false;
        if (!s.Dealt)
            return false;
        // Every record made anew by script (the core's rule for what the JSON
        // loader filled).
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        for (int rec = 0; rec < s.Dealt.Count(); rec++)
        {
            if (s.Dealt[rec])
                made.Insert(s.Dealt[rec].Copy());
        }
        s.Dealt = made;
        if (!s.SeatList || !s.StockCards || !s.DiscardCards)
            return false;
        if (!s.AttackCards || !s.DefenceCards)
            return false;
        for (int i = 0; i < s.SeatList.Count(); i++)
        {
            if (!s.SeatList[i] || !s.SeatList[i].Held)
                return false;
        }

        OZG_DurakState before = m_S;
        m_S = s;
        string why;
        if (!Sound(why))
        {
            m_S = before;
            return false;
        }
        return true;
    }

    // Every card the deal began with exactly once, between the hands, the
    // stock, the table and the discards; and the few numbers in their ranges.
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
        if (m_S.DefenceCards.Count() != m_S.AttackCards.Count())
        {
            why = "the table does not pair up";
            return false;
        }
        if (m_S.Opener < 0 || m_S.Opener >= n || m_S.Defender < 0 || m_S.Defender >= n)
        {
            why = "the opener or the defender is no seat";
            return false;
        }
        if (!OZG_Cards.IsCard(m_S.TrumpCard))
        {
            why = "the trump is no card";
            return false;
        }

        array<string> have = new array<string>();
        int i;
        int k;
        for (i = 0; i < n; i++)
        {
            array<string> held = m_S.SeatList[i].Held;
            for (k = 0; k < held.Count(); k++)
                have.Insert(held[k]);
        }
        for (i = 0; i < m_S.StockCards.Count(); i++)
            have.Insert(m_S.StockCards[i]);
        for (i = 0; i < m_S.DiscardCards.Count(); i++)
            have.Insert(m_S.DiscardCards[i]);
        for (i = 0; i < m_S.AttackCards.Count(); i++)
        {
            have.Insert(m_S.AttackCards[i]);
            if (m_S.DefenceCards[i] != "")
                have.Insert(m_S.DefenceCards[i]);
        }

        array<string> want = new array<string>();
        for (i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (!m_S.Dealt[i])
            {
                why = "a card of the deal has no record";
                return false;
            }
            want.Insert(m_S.Dealt[i].Name);
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
                why = "the card " + have[i] + " is there twice, or is not of this game";
                return false;
            }
            want.RemoveOrdered(at);
        }
        return true;
    }

    // One line: `durak <variant> trump= stock= dis= table= op= def= lim= take=
    // first= hands= waits=`. A hand is `<seat>:<cards>` with `x` for a seat
    // that is out, `d` for a dropped one and `p` for one that passed.
    override string Brief()
    {
        if (!m_S)
            return "durak: no deal";
        string text = "durak " + VariantId();
        if (m_S.Ended == 1)
            text = text + " over " + EndWord();
        text = text + " trump=" + m_S.TrumpCard;
        text = text + " stock=" + m_S.StockCards.Count().ToString();
        text = text + " dis=" + m_S.DiscardCards.Count().ToString();
        if (m_S.Ended == 0)
        {
            text = text + " table=" + TableLine();
            text = text + " op=" + SeatNoAt(m_S.Opener).ToString();
            text = text + " def=" + SeatNoAt(m_S.Defender).ToString();
            text = text + " lim=" + m_S.AttackLimit.ToString();
            text = text + " take=" + m_S.Taking.ToString();
            text = text + " first=" + m_S.FirstBout.ToString();
        }
        text = text + " hands=" + HandsLine();
        if (m_S.Ended == 0)
            text = text + " waits=" + WaitsLine();
        return text;
    }

    protected string EndWord()
    {
        if (m_S.DurakSeat >= 0)
            return "durak=" + m_S.DurakSeat.ToString();
        if (m_S.Tie == 1)
            return "tie";
        return "nodurak";
    }

    protected string TableLine()
    {
        if (m_S.AttackCards.Count() == 0)
            return "-";
        string text = "";
        for (int i = 0; i < m_S.AttackCards.Count(); i++)
        {
            if (i > 0)
                text = text + ",";
            string beat = m_S.DefenceCards[i];
            if (beat == "")
                beat = "-";
            text = text + m_S.AttackCards[i] + "/" + beat;
        }
        return text;
    }

    protected string HandsLine()
    {
        string text = "";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat seat = m_S.SeatList[i];
            if (i > 0)
                text = text + ",";
            text = text + seat.SeatNo.ToString() + ":" + seat.Held.Count().ToString();
            if (seat.Gone == OZG_DurakConst.GONE_OUT)
                text = text + "x";
            if (seat.Gone == OZG_DurakConst.GONE_DROPPED)
                text = text + "d";
            if (seat.Passed == 1)
                text = text + "p";
        }
        return text;
    }

    protected string WaitsLine()
    {
        string text = "";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            int seat = m_S.SeatList[i].SeatNo;
            if (!Waits(seat))
                continue;
            if (text != "")
                text = text + ",";
            text = text + seat.ToString();
        }
        if (text == "")
            return "-";
        return text;
    }

    // Every hand, the table and the stock, in full.
    override string Everything()
    {
        if (!m_S)
            return "durak: no deal";
        string text = "";
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat seat = m_S.SeatList[i];
            text = text + seat.SeatNo.ToString() + "[" + OZG_Cards.Line(seat.Held) + "] ";
        }
        text = text + "table[" + TableLine() + "] ";
        text = text + "stock[" + OZG_Cards.Line(m_S.StockCards) + "]";
        return text;
    }
}
