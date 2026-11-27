// Cheating at a board and with dice (boards and dice design §9, §10), on the
// session: who has the window open, sleights on a board, a die swapped, a
// loaded die noticed after a throw. The kit's window (loading a die, marking
// a card in it) waits for its mock-up; the server's half of loading is
// OZG_DiceCheat.
modded class OZG_Session
{
    // Players with this place's window open: told of a sleight on the board
    // at once. Kept from the window's own words: it asks when it opens, and
    // says when it closes.
    protected ref array<string> m_Looking;
    // When each player last did a sleight on the board (the mission's ms).
    protected ref map<string, int> m_BoardAt;
    // Swaps of a die each player did in this game, and those who swapped
    // since their last throw.
    protected ref map<string, int> m_Swaps;
    protected ref array<string> m_SwapWaits;
    // The dice somebody saw slipped into a can (a die's key), and who saw each: his window
    // marks that die with an eye while it lies in the can it was slipped into.
    protected ref map<string, ref array<string>> m_DiceEyes;
    // A die slipped into a set and not thrown since, by its key: the look of the die it
    // took the place of, and who slipped it. A can's mouth shows the dice in it, so until
    // the die is thrown only he and those who saw it go in see it there.
    protected ref map<string, string> m_SlipLooks;
    protected ref map<string, string> m_SlipBy;
    // The dice of a throw nobody but its owner saw (perudo, under the cans), kept until
    // the dice are opened: the die itself, the face it showed, who threw it.
    protected ref array<EntityAI> m_HidDice;
    protected ref array<int> m_HidFaces;
    protected ref array<string> m_HidBy;

    protected void CheatInit()
    {
        if (!m_Looking)
            m_Looking = new array<string>();
        if (!m_BoardAt)
            m_BoardAt = new map<string, int>();
        if (!m_Swaps)
            m_Swaps = new map<string, int>();
        if (!m_SwapWaits)
            m_SwapWaits = new array<string>();
        if (!m_DiceEyes)
            m_DiceEyes = new map<string, ref array<string>>();
        if (!m_SlipLooks)
            m_SlipLooks = new map<string, string>();
        if (!m_SlipBy)
            m_SlipBy = new map<string, string>();
        if (!m_HidDice)
            m_HidDice = new array<EntityAI>();
        if (!m_HidFaces)
            m_HidFaces = new array<int>();
        if (!m_HidBy)
            m_HidBy = new array<string>();
    }

    override string Ask(PlayerBase player, string op, OZG_WAsk ask)
    {
        CheatInit();
        string uid = OZG_Who.Uid(player);
        int lookers = m_Looking.Count();
        if (op == OZG_Const.OP_OPEN || op == OZG_Const.OP_WATCH)
        {
            if (uid != "" && m_Looking.Find(uid) < 0)
                m_Looking.Insert(uid);
        }
        else if (op == OZG_Const.OP_UNWATCH)
        {
            m_Looking.RemoveItem(uid);
        }
        // At a board the other player's plate says whether he watches: a window
        // opened or shut is a new view for everyone, nothing else changing.
        if (lookers != m_Looking.Count() && SeatOf(uid) >= 0)
        {
            OZG_Game kind = OZG_Games.Sample(m_R.GameId);
            if (kind && kind.SetKind() == OZG_Const.SET_BOARD)
            {
                m_R.Rev = m_R.Rev + 1;
                Pulse();
                Push();
            }
        }
        // A move or a swap after a restart: the dice each seat throws read
        // again first, so that a loaded die counts. A window opened too: it shows
        // whose dice each seat throws, the can they lie in and what may be swapped.
        bool acts = op == OZG_Const.OP_ACT || op == OZG_Const.OP_DSWAP;
        bool looks = op == OZG_Const.OP_OPEN || op == OZG_Const.OP_WATCH;
        if (acts || looks)
            DiceAgain();
        if (op == OZG_Const.OP_BSLEIGHT)
            return BoardSleightAsk(player, uid, ask);
        if (op == OZG_Const.OP_DSWAP)
            return DiceSwapAsk(player, uid, ask);
        return super.Ask(player, op, ask);
    }

    // The window of a board (the canvas's board): the seats whose player has the
    // window open and is near, for the other's plate ("watching" or "away").
    override protected OZG_WView ViewHead()
    {
        CheatInit();
        OZG_WView v = super.ViewHead();
        float reach = OZG_Cfg.Get().WatchRadius;
        for (int i = 0; i < m_Looking.Count(); i++)
        {
            int seat = SeatOf(m_Looking[i]);
            if (seat < 0)
                continue;
            PlayerBase p = OZG_Sessions.PlayerOf(m_Looking[i]);
            if (!p || !m_Place || vector.Distance(p.GetPosition(), m_Place.GetPosition()) > reach)
                continue;
            v.Looking.Insert(seat);
        }
        return v;
    }

    // Whether this player may do a sleight on the board, and in how many seconds the
    // next one may be done (§9.1): the panel "Sleight of hand" and its timer.
    override protected void SleightHead(OZG_WView v, string uid, int seat, bool dealt)
    {
        CheatInit();
        super.SleightHead(v, uid, seat, dealt);
        DiceLooksHead(v, uid);
        DiceMarksHead(v, uid);
        DiceSwapHead(v, uid, seat, dealt);
        v.BoardSleights = 0;
        v.BoardWait = 0;
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || seat < 0 || uid == "" || !dealt || !m_Game)
            return;
        if (m_R.St != OZG_Const.ST_PLAYING || m_Game.SetKind() != OZG_Const.SET_BOARD)
            return;
        v.BoardSleights = 1;
        int last;
        if (!m_BoardAt.Find(uid, last))
            return;
        int left = cfg.BoardSleightMinutes * 60000 - (GetGame().GetTime() - last);
        if (left > 0)
            v.BoardWait = (left + 999) / 1000;
    }

    // A new game: every player's swaps counted anew.
    override protected string DealPieces(OZG_Game game, array<int> seats, array<int> amounts, map<string, string> options, string hint)
    {
        CheatInit();
        ThrownLooksForget();
        // before the game is begun: its first throw is made, and taken, inside the deal
        HiddenForget();
        string why = super.DealPieces(game, seats, amounts, options, hint);
        if (why == "")
        {
            m_Swaps.Clear();
            m_SwapWaits.Clear();
        }
        return why;
    }

    override void Changed()
    {
        CheatInit();
        NoticeThrows();
        super.Changed();
    }

    // When the seats' dice were last read again (the server's time, ms), and how long
    // before they are read once more while some are still not known.
    protected int m_DiceAgainAt;
    protected static const int DICE_AGAIN_MS = 3000;

    // The dice each seat throws, read again when they are not all known: a game that
    // came back from the world's storage after a restart.
    protected void DiceAgain()
    {
        if (!m_Game || m_R.St != OZG_Const.ST_PLAYING || m_Game.DiceNeeded() <= 0)
            return;
        // all known: nothing to read. Some not (a game on own dice whose player is not
        // back yet, his can with him): read again when asked, a few seconds apart
        if (m_SeatDice && m_SeatDice.Count() >= m_R.Dealt.Count())
            return;
        int now = GetGame().GetTime();
        if (m_DiceAgainAt > 0 && now - m_DiceAgainAt < DICE_AGAIN_MS)
            return;
        m_DiceAgainAt = now;
        map<string, string> options = new map<string, string>();
        for (int i = 0; i < m_R.OptIds.Count() && i < m_R.OptVals.Count(); i++)
            options.Set(m_R.OptIds[i], m_R.OptVals[i]);
        DiceFor(m_Game, m_R.Dealt, options);
    }

    // ---- a sleight on the board (§9.1) --------------------------------------

    // ask.Move: "<kind>:<a>:<b>" (OZG_DuelConst.S_*; squares, points, or the
    // letter of a piece taken off for "back").
    protected string BoardSleightAsk(PlayerBase player, string uid, OZG_WAsk ask)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        int mine = SeatOf(uid);
        if (mine < 0)
            return OZG_Const.SAY_NOT_SEATED;
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game)
            return OZG_Const.NO_OVER;
        if (m_Game.SetKind() != OZG_Const.SET_BOARD)
            return OZG_Const.SAY_SLEIGHT_NOT_GAME;
        if (m_R.Dealt.Find(mine) < 0)
            return OZG_Const.NO_SEAT;
        if (!Present(player))
            return OZG_Const.SAY_TOO_FAR;
        int now = GetGame().GetTime();
        int last;
        if (m_BoardAt.Find(uid, last) && now - last < cfg.BoardSleightMinutes * 60000)
            return OZG_Const.SAY_BSLEIGHT_WAIT;
        array<string> parts = new array<string>();
        OZG_Text.SplitLong(ask.Move, ":", parts);
        if (parts.Count() != 3)
            return OZG_DuelConst.NO_SLEIGHT;
        string kind = parts[0];
        string a = parts[1];
        string b = parts[2];
        string refusal;
        if (!m_Game.BoardSleight(mine, kind, a, b, refusal))
            return refusal;
        m_BoardAt.Set(uid, now);

        // Seen at once by whoever has the window open and is near; nobody
        // else is told, and the move list shows nothing.
        float reach = cfg.WatchRadius;
        string seen = "";
        OZG_WBoardSaw w = new OZG_WBoardSaw();
        w.PlaceId = m_R.PlaceId;
        w.SeatNo = mine;
        w.Kind = kind;
        w.A = a;
        w.B = b;
        string body;
        string err;
        JsonFileLoader<OZG_WBoardSaw>.MakeData(w, body, err, false);
        for (int i = 0; i < m_Looking.Count(); i++)
        {
            string who = m_Looking[i];
            if (who == uid)
                continue;
            PlayerBase p = OZG_Sessions.PlayerOf(who);
            if (!p || vector.Distance(p.GetPosition(), m_Place.GetPosition()) > reach)
                continue;
            SendTo(p, OZG_Const.OP_BSAW, body);
            if (seen != "")
                seen = seen + ",";
            seen = seen + who;
        }
        if (seen == "")
            seen = "nobody";
        SleightLine(uid, mine, "board-" + kind, a + " " + b, "seen by " + seen);
        Changed();
        return "";
    }

    // What a player knows of a die, as one number (OZG_WView.SeatDiceMarks): the face a
    // load he knows is on times ten plus how heavy, 100 more for a die he saw slipped in.
    protected int DieMark(EntityAI thing, string uid)
    {
        OZG_Die_Base die = OZG_Die_Base.Cast(thing);
        if (!die || uid == "")
            return 0;
        int mark = 0;
        if (die.OZG_Loaded() && die.OZG_Knows(uid))
        {
            OZG_Settings cfg = OZG_Cfg.Get();
            mark = die.OZG_LoadFace() * 10 + cfg.LoadStep(die.OZG_LoadWeight());
        }
        array<string> saw;
        if (m_DiceEyes && m_DiceEyes.Find(OZG_Keys.Of(die), saw) && saw && saw.Find(uid) >= 0)
            mark = mark + 100;
        return mark;
    }

    // "<seat>=<mark>,<mark>.." of the first `most` of `dice`.
    protected string DiceMarksLine(int seat, array<EntityAI> dice, int most, string uid)
    {
        string line = seat.ToString() + "=";
        for (int k = 0; k < dice.Count() && k < most; k++)
        {
            if (k > 0)
                line = line + ",";
            int mark = DieMark(dice[k], uid);
            line = line + mark.ToString();
        }
        return line;
    }

    // Does this player know of a die slipped in and not thrown yet: he slipped it, or
    // saw it go in.
    protected bool SlipKnown(string key, string uid)
    {
        if (uid == "")
            return false;
        string by;
        if (m_SlipBy.Find(key, by) && by == uid)
            return true;
        array<string> saw;
        if (m_DiceEyes.Find(key, saw) && saw && saw.Find(uid) >= 0)
            return true;
        return false;
    }

    // The dice in the cans as this player may see them (the view's SeatDiceLooks made
    // again): a die slipped in and not thrown since shows as the die it took the place of.
    protected void DiceLooksHead(OZG_WView v, string uid)
    {
        if (!m_SlipLooks || m_SlipLooks.Count() == 0 || !m_SeatDice || !m_Game)
            return;
        v.SeatDiceLooks.Clear();
        for (int s = 0; s < m_R.Dealt.Count(); s++)
        {
            int seat = m_R.Dealt[s];
            array<EntityAI> dice;
            if (!m_SeatDice.Find(seat, dice) || !dice)
                continue;
            string line = seat.ToString() + "=";
            int most = m_Game.DiceThrown(seat);
            for (int k = 0; k < dice.Count() && k < most; k++)
            {
                if (k > 0)
                    line = line + ",";
                string look = "white";
                EntityAI die = dice[k];
                if (die)
                {
                    look = OZG_BoardRead.DieLookOf(die.GetType());
                    string key = OZG_Keys.Of(die);
                    string was;
                    if (m_SlipLooks.Find(key, was) && !SlipKnown(key, uid))
                        look = was;
                }
                line = line + look;
            }
            v.SeatDiceLooks.Insert(line);
        }
    }

    // What this player knows of the dice every seat throws, and of the dice as each seat
    // threw them last: beside the view's SeatDiceLooks and ThrownDiceLooks.
    protected void DiceMarksHead(OZG_WView v, string uid)
    {
        if (uid == "" || !m_Game || !m_SeatDice || m_Game.DiceNeeded() <= 0)
            return;
        for (int s = 0; s < m_R.Dealt.Count(); s++)
        {
            int seat = m_R.Dealt[s];
            array<EntityAI> dice;
            if (m_SeatDice.Find(seat, dice) && dice)
            {
                int thrownNow = m_Game.DiceThrown(seat);
                string nowLine = DiceMarksLine(seat, dice, thrownNow, uid);
                v.SeatDiceMarks.Insert(nowLine);
            }
            array<EntityAI> thrown;
            if (m_ThrownDice && m_ThrownDice.Find(seat, thrown) && thrown)
            {
                string thrownLine = DiceMarksLine(seat, thrown, thrown.Count(), uid);
                v.ThrownDiceMarks.Insert(thrownLine);
            }
        }
    }

    // The dice games' swap panel: the swaps this player has left and whether he may swap
    // now (DiceSwapAsk's own rules), the dice he carries and does not throw with the load
    // he knows on each, and the load he knows on each die of his own throw.
    protected void DiceSwapHead(OZG_WView v, string uid, int seat, bool dealt)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || seat < 0 || uid == "" || !dealt || !m_Game)
            return;
        if (m_R.St != OZG_Const.ST_PLAYING || m_Game.DiceNeeded() <= 0)
            return;
        int done = 0;
        m_Swaps.Find(uid, done);
        int left = cfg.DiceSwaps - done;
        if (left < 0)
            left = 0;
        v.DiceSwapsMost = cfg.DiceSwaps;
        v.DiceSwapsLeft = left;
        bool canThrow = m_Game.ThrowsNext(seat);
        bool waits = m_SwapWaits.Find(uid) >= 0;
        if (left > 0 && canThrow && !waits)
            v.DiceSwapNow = 1;
        // the dice in play: none of them is a pocket's
        array<EntityAI> inPlay = new array<EntityAI>();
        if (m_SeatDice)
        {
            for (int s = 0; s < m_R.Dealt.Count(); s++)
            {
                int dealtSeat = m_R.Dealt[s];
                array<EntityAI> seatDice;
                if (!m_SeatDice.Find(dealtSeat, seatDice) || !seatDice)
                    continue;
                for (int d = 0; d < seatDice.Count(); d++)
                    inPlay.Insert(seatDice[d]);
            }
        }
        PlayerBase player = OZG_Sessions.PlayerOf(uid);
        if (player)
        {
            array<EntityAI> items = OZG_KitService.Carried(player);
            for (int i = 0; i < items.Count(); i++)
            {
                OZG_Die_Base die = OZG_Die_Base.Cast(items[i]);
                if (!die || inPlay.Find(die) >= 0)
                    continue;
                int face = 0;
                int step = 0;
                if (die.OZG_Loaded() && die.OZG_Knows(uid))
                {
                    face = die.OZG_LoadFace();
                    step = cfg.LoadStep(die.OZG_LoadWeight());
                }
                v.PocketDice.Insert(OZG_Keys.Of(die));
                v.PocketLooks.Insert(OZG_BoardRead.DieLookOf(die.GetType()));
                v.PocketLoads.Insert(face);
                v.PocketSteps.Insert(step);
            }
        }
        array<EntityAI> mineDice;
        if (!m_SeatDice || !m_SeatDice.Find(seat, mineDice) || !mineDice)
            return;
        // the dice he throws now, by their places among his: the first so many (perudo
        // loses them), or the ones left on the table (thousand)
        array<int> thrownAt = new array<int>();
        m_Game.DiceThrownAt(seat, thrownAt);
        for (int k = 0; k < thrownAt.Count(); k++)
        {
            int place = thrownAt[k];
            if (place < 0 || place >= mineDice.Count())
                continue;
            OZG_Die_Base own = OZG_Die_Base.Cast(mineDice[place]);
            int ownFace = 0;
            int ownStep = 0;
            if (own && own.OZG_Loaded() && own.OZG_Knows(uid))
            {
                ownFace = own.OZG_LoadFace();
                ownStep = cfg.LoadStep(own.OZG_LoadWeight());
            }
            v.MyDicePlaces.Insert(place);
            v.MyDiceLoads.Insert(ownFace);
            v.MyDiceSteps.Insert(ownStep);
        }
    }

    // ---- a die swapped (§9.2) --------------------------------------------------

    // ask.CardKey: the die he carries; ask.SeatNo: which of his dice goes out, by its
    // place among them (from 0) -- one of those he is about to throw.
    protected string DiceSwapAsk(PlayerBase player, string uid, OZG_WAsk ask)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        int mine = SeatOf(uid);
        if (mine < 0)
            return OZG_Const.SAY_NOT_SEATED;
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game || m_Game.DiceNeeded() <= 0)
            return OZG_Const.SAY_SLEIGHT_NOT_GAME;
        if (m_R.Dealt.Find(mine) < 0)
            return OZG_Const.NO_SEAT;
        if (!Present(player))
            return OZG_Const.SAY_TOO_FAR;
        if (!m_Game.ThrowsNext(mine))
            return OZG_Const.SAY_DSWAP_NOT_NOW;
        if (m_SwapWaits.Find(uid) >= 0)
            return OZG_Const.SAY_DSWAP_ONE;
        int done = 0;
        m_Swaps.Find(uid, done);
        if (done >= cfg.DiceSwaps)
            return OZG_Const.SAY_SLEIGHT_NO_MORE;
        array<EntityAI> mineDice;
        if (!m_SeatDice || !m_SeatDice.Find(mine, mineDice) || ask.SeatNo < 0 || ask.SeatNo >= mineDice.Count())
            return OZG_Const.SAY_DSWAP_NO_DIE;
        // only a die he is about to throw goes out (thousand: not one put aside)
        int goes = ask.SeatNo;
        array<int> thrownAt = new array<int>();
        m_Game.DiceThrownAt(mine, thrownAt);
        if (thrownAt.Find(goes) < 0)
            return OZG_Const.SAY_DSWAP_NO_DIE;
        EntityAI goingOut = mineDice[ask.SeatNo];
        OZG_Die_Base comingIn = OZG_Die_Base.Cast(OZG_Keys.Find(ask.CardKey));
        if (!goingOut || !comingIn || comingIn == goingOut)
            return OZG_Const.SAY_DSWAP_NO_DIE;
        if (comingIn.GetHierarchyRootPlayer() != player)
            return OZG_Const.SAY_DSWAP_NO_DIE;
        if (!OZG_DiceCheat.Exchange(goingOut, comingIn, player))
            return OZG_Const.SAY_DSWAP_NO_ROOM;

        // The die in its place for every seat that throws it (the host's
        // dice are everybody's), the loads handed to the game again.
        for (int s = 0; s < m_R.Dealt.Count(); s++)
        {
            int seat = m_R.Dealt[s];
            array<EntityAI> dice;
            if (!m_SeatDice.Find(seat, dice))
                continue;
            int at = dice.Find(goingOut);
            if (at < 0)
                continue;
            dice[at] = comingIn;
            array<int> faces = new array<int>();
            array<int> weights = new array<int>();
            OZG_DiceRead.Loads(dice, faces, weights);
            m_Game.TakeDice(seat, faces, weights);
        }
        m_Swaps.Set(uid, done + 1);
        m_SwapWaits.Insert(uid);

        // It may show (ChanceDiceSwap); then each other player present
        // rolls his own SpotPercent.
        OZG_Random r = new OZG_Random();
        int shown = r.Below(100);
        string said = "at " + cfg.ChanceDiceSwap.ToString() + "% rolled " + shown.ToString();
        array<string> saw = new array<string>();
        if (shown < cfg.ChanceDiceSwap)
        {
            for (int i = 0; i < m_R.Seats.Count(); i++)
            {
                OZG_SeatRec rec = m_R.Seats[i];
                if (rec.St != OZG_Const.SEAT_TAKEN || rec.Uid == "" || rec.Uid == uid)
                    continue;
                int spot = r.Below(100);
                if (spot < cfg.SpotPercent)
                    saw.Insert(rec.Uid);
            }
        }
        // The die that went out is nobody's planted die any more; the one that came in is
        // marked for each who saw it. The window plays the swap for the one who did it and
        // for each who noticed (a die flies into the can); a player whose window is shut
        // is told in words.
        string outKey = OZG_Keys.Of(goingOut);
        string inKey = OZG_Keys.Of(comingIn);
        m_DiceEyes.Remove(outKey);
        m_DiceEyes.Remove(inKey);
        // until it is thrown the die that came in looks, to who did not see, like the one
        // that went out (or like the one that one stood for, slipped in and never thrown)
        string stoodFor = OZG_BoardRead.DieLookOf(goingOut.GetType());
        string stoodBefore;
        if (m_SlipLooks.Find(outKey, stoodBefore))
            stoodFor = stoodBefore;
        m_SlipLooks.Remove(outKey);
        m_SlipBy.Remove(outKey);
        m_SlipLooks.Set(inKey, stoodFor);
        m_SlipBy.Set(inKey, uid);
        if (saw.Count() > 0)
        {
            array<string> eyes = new array<string>();
            eyes.Copy(saw);
            m_DiceEyes.Set(inKey, eyes);
        }
        OZG_WDiceSaw told = new OZG_WDiceSaw();
        told.PlaceId = m_R.PlaceId;
        told.SeatNo = mine;
        told.InLook = OZG_BoardRead.DieLookOf(comingIn.GetType());
        told.OutLook = OZG_BoardRead.DieLookOf(goingOut.GetType());
        told.Place = goes;
        told.Noticed = 0;
        string toldBody;
        string toldErr;
        JsonFileLoader<OZG_WDiceSaw>.MakeData(told, toldBody, toldErr, false);
        SendTo(player, OZG_Const.OP_DSAW, toldBody);
        told.Noticed = 1;
        JsonFileLoader<OZG_WDiceSaw>.MakeData(told, toldBody, toldErr, false);
        for (int k = 0; k < saw.Count(); k++)
        {
            PlayerBase seer = OZG_Sessions.PlayerOf(saw[k]);
            if (!seer || !seer.GetIdentity())
                continue;
            if (m_Looking.Find(saw[k]) >= 0)
                SendTo(seer, OZG_Const.OP_DSAW, toldBody);
            else
                OZ_Rpc.Notice(seer.GetIdentity(), OZG_Const.NOTICE_DSWAP, true, OZG_Words.W1(OZG_Const.SAY_DSWAP_SEEN, OZG_Words.SeatArg(mine)));
        }
        string seenBy = "nobody";
        if (saw.Count() > 0)
            seenBy = OZG_Text.JoinIds(saw);
        SleightLine(uid, mine, "dice-swap", OZG_Keys.Of(goingOut) + " for " + OZG_Keys.Of(comingIn), said + "; seen by " + seenBy);
        Changed();
        return "";
    }

    // ---- a loaded die noticed (§10) ---------------------------------------------

    // After every change: each die thrown that showed its loaded face is
    // noticed by each other player present, at NoticeLoaded percent times its
    // weight's step; a throw also ends the seat's wait for its next swap.
    protected void NoticeThrows()
    {
        if (!m_Game)
            return;
        // dice thrown under the cans and opened now (perudo's showdown): the table sees
        // what they showed
        if (m_Game.TakeShown())
            NoticeHidden();
        array<int> seats = new array<int>();
        array<int> idx = new array<int>();
        array<int> faces = new array<int>();
        m_Game.TakeThrows(seats, idx, faces);
        if (seats.Count() == 0)
            return;
        ThrownLooksNote(seats, idx);
        for (int w = 0; w < seats.Count(); w++)
        {
            if (seats[w] >= 0 && seats[w] < m_R.Seats.Count())
                m_SwapWaits.RemoveItem(m_R.Seats[seats[w]].Uid);
        }
        // a die thrown is seen by everybody as it is: no more a die slipped in unseen
        if (m_SlipLooks && m_SlipLooks.Count() > 0 && m_SeatDice)
        {
            for (int q = 0; q < seats.Count(); q++)
            {
                array<EntityAI> flown;
                if (!m_SeatDice.Find(seats[q], flown) || !flown || idx[q] < 0 || idx[q] >= flown.Count())
                    continue;
                EntityAI flew = flown[idx[q]];
                if (!flew)
                    continue;
                string flewKey = OZG_Keys.Of(flew);
                m_SlipLooks.Remove(flewKey);
                m_SlipBy.Remove(flewKey);
            }
        }
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || cfg.NoticeLoaded <= 0 || !m_SeatDice)
            return;
        // a throw nobody saw (perudo, under the cans) is kept until its dice are opened
        bool hides = m_Game.ThrowsHidden();
        if (hides)
            HiddenForget();
        OZG_Random r = new OZG_Random();
        for (int k = 0; k < seats.Count(); k++)
        {
            array<EntityAI> dice;
            if (!m_SeatDice.Find(seats[k], dice) || idx[k] < 0 || idx[k] >= dice.Count())
                continue;
            OZG_Die_Base die = OZG_Die_Base.Cast(dice[idx[k]]);
            if (!die)
                continue;
            int face = faces[k];
            string thrower = "";
            if (seats[k] < m_R.Seats.Count())
                thrower = m_R.Seats[seats[k]].Uid;
            if (hides)
            {
                m_HidDice.Insert(die);
                m_HidFaces.Insert(face);
                m_HidBy.Insert(thrower);
                continue;
            }
            NoticeLoadedFace(die, face, thrower, r);
        }
        if (hides)
            OZG_Log.Dbg("a throw under the cans kept: " + m_HidDice.Count().ToString() + " dice, place " + m_R.PlaceId);
    }

    protected void HiddenForget()
    {
        if (!m_HidDice)
            return;
        m_HidDice.Clear();
        m_HidFaces.Clear();
        m_HidBy.Clear();
    }

    // The dice of the throw kept are opened: each that lies on its loaded face is noticed
    // as any die seen thrown is.
    protected void NoticeHidden()
    {
        CheatInit();
        OZG_Log.Dbg("the dice under the cans opened: " + m_HidDice.Count().ToString() + " kept, place " + m_R.PlaceId);
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating == 1 && cfg.NoticeLoaded > 0)
        {
            OZG_Random r = new OZG_Random();
            for (int k = 0; k < m_HidDice.Count(); k++)
            {
                OZG_Die_Base die = OZG_Die_Base.Cast(m_HidDice[k]);
                if (!die)
                    continue;
                int face = m_HidFaces[k];
                string thrower = m_HidBy[k];
                NoticeLoadedFace(die, face, thrower, r);
            }
        }
        HiddenForget();
    }

    // A die that showed `face` in a throw of `thrower`'s, seen by the table: when that is
    // its loaded face, each other seated player notices it at NoticeLoaded percent times
    // its weight's step.
    protected void NoticeLoadedFace(OZG_Die_Base die, int face, string thrower, OZG_Random r)
    {
        if (!die || !die.OZG_Loaded() || die.OZG_LoadFace() != face)
            return;
        OZG_Settings cfg = OZG_Cfg.Get();
        int chance = cfg.NoticeLoaded * cfg.LoadStep(die.OZG_LoadWeight());
        string faceText = die.OZG_LoadFace().ToString();
        string weightText = die.OZG_LoadWeight().ToString();
        string what = OZG_Keys.Of(die) + " (" + faceText + " at " + weightText + "%), place " + m_R.PlaceId;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec rec = m_R.Seats[i];
            if (rec.St != OZG_Const.SEAT_TAKEN || rec.Uid == "" || rec.Uid == thrower)
                continue;
            if (die.OZG_Knows(rec.Uid))
                continue;
            int roll = r.Below(100);
            if (roll >= chance)
                continue;
            die.OZG_AddKnower(rec.Uid);
            PlayerBase p = OZG_Sessions.PlayerOf(rec.Uid);
            if (p && p.GetIdentity())
                OZ_Rpc.Notice(p.GetIdentity(), OZG_Const.NOTICE_LOADED, true, OZG_Words.W1(OZG_Const.SAY_LOAD_NOTICED, faceText));
            string line = "notice-load: " + rec.Uid + " noticed " + what;
            line = line + ", rolled " + roll.ToString() + " at " + chance.ToString() + "%";
            OZG_Log.Info(line);
        }
    }
}

// The server's half of loading a die and of the swap (§9.2, §10).
class OZG_DiceCheat
{
    // A die loaded by `player` with a kit he carries: one use of the kit. A
    // die already loaded and unknown to him opens to him instead, and nothing
    // is spent. "" or the refusal; `said` the words he is told.
    static string Load(PlayerBase player, OZG_Die_Base die, int face, int step, out string said)
    {
        said = "";
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        string uid = OZG_Who.Uid(player);
        if (!player || !die || uid == "" || die.GetHierarchyRootPlayer() != player)
            return OZG_Const.SAY_DSWAP_NO_DIE;
        if (die.OZG_Loaded() && !die.OZG_Knows(uid))
        {
            die.OZG_AddKnower(uid);
            said = OZG_Words.W2(OZG_Const.SAY_LOAD_OPENED, die.OZG_LoadFace().ToString(), OZG_Words.KeyArg(OZG_Const.LOAD_STEP + cfg.LoadStep(die.OZG_LoadWeight()).ToString()));
            OZG_Log.Info("load-open: " + uid + " opened " + OZG_Keys.Of(die) + " (" + die.OZG_LoadFace().ToString() + " at " + die.OZG_LoadWeight().ToString() + "%)");
            return "";
        }
        if (face < 1 || face > 6 || step < 1 || step > 3)
            return OZG_Const.SAY_LOAD_BAD;
        ItemBase kit = Kit(player);
        if (!kit)
            return OZG_Const.SAY_LOAD_NO_KIT;
        int weight = cfg.LoadWeights[step - 1];
        die.OZG_Load(face, weight, uid);
        OZG_Recipes.Spend(kit);
        said = OZG_Words.W2(OZG_Const.SAY_LOAD_DONE, face.ToString(), OZG_Words.KeyArg(OZG_Const.LOAD_STEP + step.ToString()));
        OZG_Log.Info("load: " + uid + " loaded " + OZG_Keys.Of(die) + " on " + face.ToString() + " at " + weight.ToString() + "%");
        return "";
    }

    // A card sharp's kit he carries with a use left.
    static ItemBase Kit(PlayerBase player)
    {
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        EntityAI held = player.GetHumanInventory().GetEntityInHands();
        if (held && items.Find(held) < 0)
            items.Insert(held);
        for (int i = 0; i < items.Count(); i++)
        {
            ItemBase item = ItemBase.Cast(items[i]);
            if (item && item.IsKindOf("OZG_MarkTool") && item.GetQuantity() > 0)
                return item;
        }
        return null;
    }

    // The die going out to the player's pockets, the one coming in to the
    // slot it left; false, with nothing moved, when either way is shut. A
    // location names its item as well as the place (SetAttachment(parent,
    // item, slot)): the slot the outgoing die left is described anew for the
    // incoming one -- the old location, reused, names the outgoing die, and
    // moving another die by it left one die in two places and lost the other
    // (measured 2026-11-26).
    static bool Exchange(EntityAI goingOut, EntityAI comingIn, PlayerBase player)
    {
        InventoryLocation was = new InventoryLocation;
        if (!goingOut.GetInventory().GetCurrentInventoryLocation(was))
            return false;
        if (was.GetType() != InventoryLocationType.ATTACHMENT)
            return false;
        EntityAI holder = was.GetParent();
        int slot = was.GetSlot();
        if (!holder)
            return false;
        OZG_Place place = OZG_Place.Cast(goingOut.GetHierarchyRoot());
        if (place)
            place.OZG_Open();
        // No free place in his pockets: refused (the caller says "no room"), a sleight with
        // nowhere to palm the die. An exchange in place was tried on the stand 2026-11-26
        // and left: over the ground (the outgoing die there, the incoming one into the slot,
        // the outgoing one into the place it left) the last step failed whenever the
        // incoming die had sat in a slot of a can he carries -- where a die given to full
        // pockets goes: LocationCanAddEntity said no to that slot for the die on the ground,
        // at once and a second later, and the die stayed at his feet; the engine's
        // ServerLocationSwap answered false for the pair and moved nothing; the plain
        // LocationSwap moved both on the server and told no client.
        InventoryLocation pocket = new InventoryLocation;
        if (!player.GetInventory().FindFreeLocationFor(goingOut, FindInventoryLocationType.ANY_CARGO, pocket))
            return false;
        // The slot is filled by the session's own hand (OZG_Deck_Base.OZG_HandIn). A can in
        // a pocket refuses every attachment by itself -- an item lying in cargo does
        // (ItemBase.CanReceiveAttachment) -- so on a player's own dice the die that came
        // in was refused, the die taken out could not go back either, the can was left a
        // die short and the swap answered "no room" (the stand, 2026-11-27).
        OZG_Deck_Base box = OZG_Deck_Base.Cast(holder);
        if (box)
            box.OZG_HandIn(true);
        bool swapped = false;
        if (SyncMove(goingOut, pocket))
        {
            InventoryLocation into = new InventoryLocation;
            into.SetAttachment(holder, comingIn, slot);
            if (SyncMove(comingIn, into))
            {
                swapped = true;
            }
            else
            {
                InventoryLocation back = new InventoryLocation;
                back.SetAttachment(holder, goingOut, slot);
                if (!SyncMove(goingOut, back))
                    OZG_Log.Error("dice swap: the die taken out of " + holder.GetType() + " could not be put back");
            }
        }
        if (box)
            box.OZG_HandIn(false);
        return swapped;
    }

    // The synchronous server move the card sleights make (OZG_DeckRead).
    static bool SyncMove(EntityAI item, InventoryLocation dst)
    {
        InventoryLocation src = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(src))
            return false;
        if (!GameInventory.LocationCanAddEntity(dst))
            return false;
        if (!GameInventory.LocationSyncMoveEntity(src, dst))
            return false;
        InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst);
        return true;
    }
}
