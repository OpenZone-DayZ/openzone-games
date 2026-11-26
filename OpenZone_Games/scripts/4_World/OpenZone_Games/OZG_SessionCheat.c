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
        // again first, so that a loaded die counts.
        if (op == OZG_Const.OP_ACT || op == OZG_Const.OP_DSWAP)
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

    // The dice each seat throws, read again when none are known: a game that
    // came back from the world's storage after a restart.
    protected void DiceAgain()
    {
        if (!m_Game || m_R.St != OZG_Const.ST_PLAYING || m_Game.DiceNeeded() <= 0)
            return;
        if (m_SeatDice && m_SeatDice.Count() > 0)
            return;
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
        // the dice he throws now: the first so many of his (perudo loses them)
        int thrown = m_Game.DiceThrown(seat);
        for (int k = 0; k < mineDice.Count() && k < thrown; k++)
        {
            OZG_Die_Base own = OZG_Die_Base.Cast(mineDice[k]);
            int ownFace = 0;
            int ownStep = 0;
            if (own && own.OZG_Loaded() && own.OZG_Knows(uid))
            {
                ownFace = own.OZG_LoadFace();
                ownStep = cfg.LoadStep(own.OZG_LoadWeight());
            }
            v.MyDiceLoads.Insert(ownFace);
            v.MyDiceSteps.Insert(ownStep);
        }
    }

    // ---- a die swapped (§9.2) --------------------------------------------------

    // ask.CardKey: the die he carries; ask.SeatNo: which of the dice he is
    // about to throw goes out (from 0).
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
        // only a die he is about to throw goes out
        if (ask.SeatNo >= m_Game.DiceThrown(mine))
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
        for (int k = 0; k < saw.Count(); k++)
        {
            PlayerBase seer = OZG_Sessions.PlayerOf(saw[k]);
            if (seer && seer.GetIdentity())
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
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || cfg.NoticeLoaded <= 0 || !m_SeatDice)
            return;
        OZG_Random r = new OZG_Random();
        for (int k = 0; k < seats.Count(); k++)
        {
            array<EntityAI> dice;
            if (!m_SeatDice.Find(seats[k], dice) || idx[k] < 0 || idx[k] >= dice.Count())
                continue;
            OZG_Die_Base die = OZG_Die_Base.Cast(dice[idx[k]]);
            if (!die || !die.OZG_Loaded() || die.OZG_LoadFace() != faces[k])
                continue;
            int chance = cfg.NoticeLoaded * cfg.LoadStep(die.OZG_LoadWeight());
            string thrower = "";
            if (seats[k] < m_R.Seats.Count())
                thrower = m_R.Seats[seats[k]].Uid;
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
                    OZ_Rpc.Notice(p.GetIdentity(), OZG_Const.NOTICE_LOADED, true, OZG_Words.W1(OZG_Const.SAY_LOAD_NOTICED, die.OZG_LoadFace().ToString()));
                OZG_Log.Info("notice-load: " + rec.Uid + " noticed " + OZG_Keys.Of(die) + " (" + die.OZG_LoadFace().ToString() + " at " + die.OZG_LoadWeight().ToString() + "%), place " + m_R.PlaceId + ", rolled " + roll.ToString() + " at " + chance.ToString() + "%");
            }
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
        if (!SyncMove(goingOut, pocket))
            return false;
        InventoryLocation into = new InventoryLocation;
        into.SetAttachment(holder, comingIn, slot);
        if (SyncMove(comingIn, into))
            return true;
        InventoryLocation back = new InventoryLocation;
        back.SetAttachment(holder, goingOut, slot);
        SyncMove(goingOut, back);
        return false;
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
