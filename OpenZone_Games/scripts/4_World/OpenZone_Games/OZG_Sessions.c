// Every session of this server, and the doors into them: laying out, the
// actions on a place, the clock, the restart.
class OZG_Sessions
{
    protected static ref map<string, ref OZG_Session> s_ById = new map<string, ref OZG_Session>();
    protected static ref map<string, PlayerBase> s_Players = new map<string, PlayerBase>();
    protected static int s_Now;
    protected static bool s_Closing;
    protected static bool s_Swept;
    protected static bool s_Started;
    protected static ref array<OZG_Place> s_Waiting = new array<OZG_Place>();

    // Seconds since the mission started, by the mod's own clock.
    static int Now()
    {
        return s_Now;
    }

    static int Count()
    {
        return s_ById.Count();
    }

    static OZG_Session ById(string id)
    {
        return s_ById.Get(id);
    }

    static OZG_Session Of(OZG_Place place)
    {
        if (!place)
            return null;
        return s_ById.Get(place.OZG_Id());
    }

    static OZG_Session At(int index)
    {
        if (index < 0 || index >= s_ById.Count())
            return null;
        return s_ById.GetElement(index);
    }

    // A player by id. The table is made anew every second; a miss looks once
    // more, for somebody who connected since.
    static PlayerBase PlayerOf(string uid)
    {
        if (uid == "")
            return null;
        PlayerBase p;
        if (s_Players.Find(uid, p) && p)
            return p;
        OZG_Who.Everyone(s_Players);
        if (s_Players.Find(uid, p))
            return p;
        return null;
    }

    // For the admin window's card (OZG_Facts): the places where a deal is being
    // played, and the seats held by somebody, kept seats with them.
    static void Counts(out int playing, out int seated)
    {
        playing = 0;
        seated = 0;
        for (int i = 0; i < s_ById.Count(); i++)
        {
            OZG_Session s = s_ById.GetElement(i);
            if (!s || !s.m_R)
                continue;
            if (s.m_R.St == OZG_Const.ST_PLAYING)
                playing++;
            for (int k = 0; k < s.m_R.Seats.Count(); k++)
            {
                OZG_SeatRec seat = s.m_R.Seats[k];
                if (seat && seat.Uid != "")
                    seated++;
            }
        }
    }

    // The session where this player holds a seat: one seat a player, in the
    // whole world.
    static OZG_Session SeatedIn(string uid)
    {
        for (int i = 0; i < s_ById.Count(); i++)
        {
            OZG_Session s = s_ById.GetElement(i);
            if (s && s.SeatOf(uid) >= 0)
                return s;
        }
        return null;
    }

    // ------------------------------------------------------------ laying out

    protected static void Tell(PlayerBase p, string op, string why)
    {
        if (!p)
            return;
        PlayerIdentity id = p.GetIdentity();
        if (id)
            OZ_Rpc.Notice(id, op, false, why);
    }

    // A new deck of a class: into free hands, else onto the spot.
    protected static void GiveDeck(PlayerBase to, string deckClass, vector at)
    {
        if (deckClass == "")
            deckClass = OZG_Decks.FirstClass();
        if (deckClass == "")
            return;
        if (to && to.IsAlive() && !to.GetItemInHands())
        {
            if (to.GetHumanInventory().CreateInHands(deckClass))
                return;
        }
        Object deck = GetGame().CreateObjectEx(deckClass, at, ECE_PLACE_ON_SURFACE);
        if (deck)
            deck.SetPosition(at);
    }

    // The deploy completed: the server decides again and makes the place.
    static void LayOut(PlayerBase player, OZG_Deck_Base deck, vector position, vector orientation)
    {
        if (!player || !deck)
            return;
        string deckClass = deck.GetType();
        OZG_Settings cfg = OZG_Cfg.Get();
        int mode = OZG_Aim.ServerMode(player);

        // The ring the server's own hologram ran a moment ago, when it is the
        // ring of this very spot.
        OZG_RingResult ring;
        Object ghost;
        Hologram holo = player.GetHologramServer();
        if (holo)
        {
            ghost = holo.GetProjectionEntity();
            if (vector.Distance(holo.GetProjectionPosition(), position) < 0.01)
                ring = holo.OZG_LastRing();
        }

        string why = "";
        if (OZG_Place.NearOther(position, cfg.PlaceSpacing, null))
            why = OZG_Const.LAY_NO_SPACING;
        if (why == "")
        {
            if (!ring)
                ring = OZG_Ring.Compute(position, mode, player.GetPosition(), ghost);
            if (ring.Count() < 2)
                why = OZG_Const.LAY_NO_ROOM;
        }
        if (why != "")
        {
            // The deck is still in his hands: the action that placed it does
            // not delete a deck (OZG_ActionLayOut.OnEndServer).
            Tell(player, OZG_Const.NOTICE_LAY, why);
            return;
        }

        // The place keeps what was decided, never "guess again": a ring worked
        // out anew later has no placer to read the guess from.
        int laid = ring.LayMode;
        OZG_Session s = Make(deckClass, laid, position, orientation, ring);
        if (!s)
        {
            Tell(player, OZG_Const.NOTICE_LAY, OZG_Const.SAY_CANNOT);
            return;
        }

        // The place asks for the deck, and what follows does not wait for it.
        s.m_Place.OZG_TakeDeck(player, deck);
        s.AwaitDeck(player);

        // The placer is the host, and sits at the seat nearest to him when one
        // is within a metre.
        s.SetHost(player);
        float dist;
        int nearest = ring.Nearest(player.GetPosition(), dist);
        string no;
        bool seated = false;
        if (nearest >= 0 && dist <= 1.0)
            seated = s.Sit(player, nearest, no);
        if (seated)
            s.ShowTo(player);
        else
            s.Changed();

        string line = "place " + s.Id() + " laid out by " + OZG_Who.Uid(player);
        line = line + " at " + position.ToString(false) + " seats=" + ring.Count().ToString();
        line = line + " kind=" + ring.Kind.ToString() + " asked=" + mode.ToString();
        line = line + " laid=" + laid.ToString();
        OZG_Log.Info(line);
    }

    // A place with its session. Also the stand's door: a place made by script.
    static OZG_Session Make(string deckClass, int mode, vector position, vector orientation, OZG_RingResult ring)
    {
        OZ_Json.EnsureTree();
        MakeDirectory(OZG_Const.DIR_GAMES);

        // A board lies as a place of its own class (boards and dice design §4).
        string placeClass = OZG_Decks.PlaceOf(deckClass);
        OZG_Place place = OZG_Place.Cast(GetGame().CreateObjectEx(placeClass, position, ECE_PLACE_ON_SURFACE));
        if (!place)
        {
            OZG_Log.Error("no place could be made at " + position.ToString(false));
            return null;
        }
        place.SetPosition(position);
        place.SetOrientation(orientation);
        place.OZG_Setup(deckClass, mode);
        place.SetLifetime(OZG_Const.PLACE_LIFETIME);

        OZG_Session s = new OZG_Session(place);
        s.Fresh(ring, deckClass, mode);
        s_ById.Set(s.Id(), s);
        return s;
    }

    // A place its deck never reached is no game: it goes, and no deck is
    // given, because the deck never left its owner.
    static void Unlay(OZG_Session s, PlayerBase placer)
    {
        if (!s)
            return;
        string id = s.Id();
        OZG_Place place = s.m_Place;
        s.CloseAll(OZG_Const.SAY_CANNOT);
        s.DropFans();
        DeleteFile(OZG_Session.PathOf(id));
        // Out of the table first: deleting the place looks for its session.
        s.m_Place = null;
        s_ById.Remove(id);
        if (place)
            GetGame().ObjectDelete(place);
        Tell(placer, OZG_Const.NOTICE_LAY, OZG_Const.SAY_CANNOT);
        OZG_Log.Warn("place " + id + " was unlaid: its deck did not reach it");
    }

    // The deck goes back. It leaves the place for the ground where the place
    // lies, at once, so that the place can be deleted; then free hands are
    // asked to take it up (man.c:197; the hands answer a tick later, and a
    // deck they do not take lies where the place lay). A place that holds no
    // deck gives a new one of its class.
    protected static void ReturnDeck(OZG_Place place, PlayerBase to, string deckClass, vector at)
    {
        OZG_Deck_Base deck;
        if (place)
        {
            int left = place.OZG_Tidy();
            if (left > 0)
                OZG_Log.Info("place " + place.OZG_Id() + ": " + left.ToString() + " card(s) found no slot and stay where it lay");
            deck = place.OZG_Release();
        }
        if (!deck)
        {
            GiveDeck(to, deckClass, at);
            return;
        }
        // Gathered up (design §17): the place goes in this frame, so the deck
        // carries the sound. A deck made anew for a place that held none
        // (GiveDeck) comes into the clients' world with it and is not heard.
        deck.OZG_HearOne(OZG_SoundIds.PICKUP);
        if (to && to.IsAlive() && !to.GetItemInHands())
            to.ServerTakeEntityToHands(deck);
    }

    // ------------------------------------------------------------ the actions on a place

    static void SitAction(OZG_Place place, PlayerBase player)
    {
        OZG_Session s = Of(place);
        if (s)
            s.SitAction(player);
        else
            Tell(player, OZG_Const.NOTICE_SIT, OZG_Const.SAY_NO_PLACE);
    }

    static void WatchAction(OZG_Place place, PlayerBase player)
    {
        OZG_Session s = Of(place);
        if (s)
            s.WatchAction(player);
        else
            Tell(player, OZG_Const.NOTICE_SIT, OZG_Const.SAY_NO_PLACE);
    }

    // "Gather the deck": anyone, when no seat is taken, kept or left with
    // cards.
    static void GatherAction(OZG_Place place, PlayerBase player)
    {
        OZG_Session s = Of(place);
        if (!s)
        {
            // A place with no session is a mat and a deck: it folds.
            if (place)
            {
                string deck = place.OZG_DeckClass();
                vector at = place.GetPosition();
                // The deck is out before the place goes: a thing deleted
                // takes what it holds with it.
                ReturnDeck(place, player, deck, at);
                GetGame().ObjectDelete(place);
            }
            return;
        }
        if (s.Busy() > 0)
        {
            Tell(player, OZG_Const.NOTICE_SIT, OZG_Const.SAY_BUSY);
            return;
        }
        Fold(s, player, OZG_Const.SAY_FOLDED);
    }

    // The game folds into its deck: into the folder's free hands, else where
    // the place lay. Every window on it closes with a word why.
    static void Fold(OZG_Session s, PlayerBase by, string why)
    {
        if (!s)
            return;
        string id = s.Id();
        OZG_Place place = s.m_Place;
        string deck = s.m_R.DeckClass;
        vector at;
        if (place)
            at = place.GetPosition();
        else if (by)
            at = by.GetPosition();

        // A game for stakes leaves its paper (stakes-and-games design §3).
        s.MakePaper(by, at);
        s.CloseAll(why);
        s.DropFans();
        DeleteFile(OZG_Session.PathOf(id));
        // Out of the table first: deleting the place looks for its session.
        s.m_Place = null;
        s_ById.Remove(id);
        // The deck is out before the place goes: a thing deleted takes what
        // it holds with it.
        ReturnDeck(place, by, deck, at);
        if (place)
            GetGame().ObjectDelete(place);
        OZG_Log.Info("place " + id + " folded: " + why);
    }

    // The place left the world some other way: an admin's hand, or the server
    // going down. The file stays; a file with no place is swept after the next
    // start.
    static void PlaceGone(OZG_Place place)
    {
        if (!place)
            return;
        string id = place.OZG_Id();
        OZG_Session s = s_ById.Get(id);
        if (!s || s.m_Place != place)
            return;
        if (!s_Closing)
        {
            s.DropFans();
            s.CloseAll(OZG_Const.SAY_GONE);
            OZG_Log.Info("place " + id + " left the world");
        }
        s.m_Place = null;
        s_ById.Remove(id);
    }

    // ------------------------------------------------------------ the restart

    // A place came back from the world's storage. That happens BEFORE the
    // mission starts and the games are registered, and a session cannot be
    // read without its game: the place waits for Start.
    static void Restore(OZG_Place place)
    {
        if (!place)
            return;
        if (!s_Started)
        {
            if (s_Waiting.Find(place) < 0)
                s_Waiting.Insert(place);
            return;
        }
        RestoreNow(place);
    }

    protected static void RestoreNow(OZG_Place place)
    {
        if (!place)
            return;
        string id = place.OZG_Id();
        if (id == "" || s_ById.Contains(id))
            return;

        OZ_Json.EnsureTree();
        MakeDirectory(OZG_Const.DIR_GAMES);

        OZG_Session s = new OZG_Session(place);
        bool hadFile;
        if (s.Load(hadFile))
        {
            OZG_Log.Info("place " + id + " restored: " + s.Describe());
        }
        else
        {
            s.Blank(hadFile);
            OZG_Log.Info("place " + id + " restored as an empty lobby, seats=" + s.m_R.Seats.Count().ToString());
        }
        s_ById.Set(id, s);
        s.Pulse();
    }

    // A session file with no place is deleted five minutes after the start.
    protected static void Sweep()
    {
        s_Swept = true;
        string name;
        FileAttr attr;
        array<string> orphans = new array<string>();
        FindFileHandle h = FindFile(OZG_Const.DIR_GAMES + "\\*.json", name, attr, FindFileFlags.ALL);
        if (!h)
            return;
        bool more = true;
        while (more)
        {
            string id = name;
            int dot = id.IndexOf(".json");
            if (dot > 0)
                id = id.Substring(0, dot);
            if (id != "" && !s_ById.Contains(id))
                orphans.Insert(name);
            more = FindNextFile(h, name, attr);
        }
        CloseFindFile(h);
        for (int i = 0; i < orphans.Count(); i++)
        {
            DeleteFile(OZG_Const.DIR_GAMES + "\\" + orphans[i]);
            OZG_Log.Info("session file " + orphans[i] + " had no place and was deleted");
        }
    }

    // ------------------------------------------------------------ the clock

    // Once a second, on the server.
    static void Tick()
    {
        s_Now++;
        OZG_Who.Everyone(s_Players);

        // A session may fold inside its own tick: walk a copy of the keys.
        array<string> ids = new array<string>();
        int i;
        for (i = 0; i < s_ById.Count(); i++)
            ids.Insert(s_ById.GetKey(i));
        for (i = 0; i < ids.Count(); i++)
        {
            OZG_Session s = s_ById.Get(ids[i]);
            if (s)
                s.Tick();
        }

        if (!s_Swept && s_Now >= 300)
            Sweep();
    }

    static void Start()
    {
        // The engine's chance starts every server from the same place:
        // measured 2026-11-24, the first deal of two boots gave the same trump
        // and the same hands. Seeded from the clock, once a mission.
        Math.Randomize(-1);
        s_Closing = false;
        s_Swept = false;
        s_Now = 0;
        s_Started = true;
        for (int i = 0; i < s_Waiting.Count(); i++)
            RestoreNow(s_Waiting[i]);
        s_Waiting.Clear();
    }

    // The mission ends: nothing a place does on its way out is a player's
    // doing.
    static void Shutdown()
    {
        s_Closing = true;
        s_Started = false;
        s_Waiting.Clear();
        s_ById.Clear();
        s_Players.Clear();
    }

    static string Describe()
    {
        string line = "sessions=" + s_ById.Count().ToString();
        line = line + " places=" + OZG_Place.All().Count().ToString();
        line = line + " now=" + s_Now.ToString();
        return line;
    }
}

// The core's admin window saves the settings through this: the text is read
// and checked, the file written, the live settings read anew. A text that does
// not read never reaches the disk.
class OZG_SettingsApplier : OZ_AdminCfgApplier
{
    override bool Apply(string json)
    {
        OZG_Settings c = new OZG_Settings();
        string err;
        if (!JsonFileLoader<OZG_Settings>.LoadData(json, c, err))
        {
            OZG_Log.Warn("admin: the settings were not taken: " + err);
            return false;
        }
        int warnings;
        c.Validate(warnings);
        OZ_ConfigLoader<OZG_Settings>.Save(OZG_Cfg.PATH, OZG_Cfg.TAG, c, true);
        OZG_Cfg.ServerLoad();
        // What a client holds of the settings (the poses, the radii, whether
        // card sharping is on) rides in the core's sync packet, which a client
        // gets when it connects and does not ask for again: everyone online
        // gets it anew, or they would play by the old numbers until they
        // reconnect.
        OZ_SyncSender.SendAll("games settings edited");
        return true;
    }
}

// The service "games" of the core's registry: what a client's window asks.
class OZG_Service : OZ_ServiceHandler
{
    override string Handle(string op, string json, PlayerIdentity sender, out bool ok, out string error)
    {
        ok = false;
        error = OZG_Const.SAY_CANNOT;
        if (!sender)
            return "";
        PlayerBase player = PlayerBase.Cast(sender.GetPlayer());
        if (!player)
            return "";

        // Where this client holds a seat: asked once, when it connects.
        if (op == OZG_Const.OP_MINE)
        {
            OZG_WMine mine = new OZG_WMine();
            OZG_Session seated = OZG_Sessions.SeatedIn(sender.GetPlainId());
            if (seated)
            {
                mine.PlaceId = seated.Id();
                mine.SeatNo = seated.SeatOf(sender.GetPlainId());
            }
            string body;
            string err;
            JsonFileLoader<OZG_WMine>.MakeData(mine, body, err, false);
            ok = true;
            error = "";
            return body;
        }

        OZG_WAsk ask = new OZG_WAsk();
        if (json != "")
        {
            string bad;
            if (!JsonFileLoader<OZG_WAsk>.LoadData(json, ask, bad) || !ask)
            {
                error = OZG_Const.SAY_CANNOT;
                return "";
            }
        }

        OZG_Session s = OZG_Sessions.ById(ask.PlaceId);
        if (!s || !s.m_Place)
        {
            error = OZG_Const.SAY_NO_PLACE;
            return "";
        }

        if (op == OZG_Const.OP_FOLD)
        {
            if (!s.IsHost(sender.GetPlainId()))
            {
                error = OZG_Const.SAY_NOT_HOST;
                return "";
            }
            OZG_Sessions.Fold(s, player, OZG_Const.SAY_FOLDED);
            error = OZ_Const.NO_REPLY;
            return "";
        }

        string why = s.Ask(player, op, ask);
        if (why == "")
        {
            // Taken: the answer is the view every client of the place gets.
            error = OZ_Const.NO_REPLY;
            return "";
        }
        error = why;
        return "";
    }
}
