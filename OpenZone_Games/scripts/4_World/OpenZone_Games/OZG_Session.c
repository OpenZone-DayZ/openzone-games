// What the server holds for one place: the seats, the host, the game.
//
// THE SERVER HOLDS THE GAME, clients show it. Every accepted change raises the
// revision, writes the session's file whole, moves the place's pulse and sends
// a view to each present seated player and each watcher. Nobody is sent
// another seat's cards or the order of the stock.

// One seat as the session keeps it and writes it.
class OZG_SeatRec
{
    int    No;
    float  X;
    float  Y;
    float  Z;
    float  Yaw;
    int    Kind;
    int    St;
    string Uid;
    string Name;
    int    PoseNo;

    vector Where()
    {
        return Vector(X, Y, Z);
    }
}

// The session's file, $profile:OpenZone\Games\<id>.json. No string in it is
// longer than OZG_Const.PART_BYTES: the game's state is cut into parts.
class OZG_SessionRec
{
    int    Version;
    string PlaceId;
    string DeckClass;
    int    Mode;
    int    Kind;
    string HostUid;
    string HostName;
    int    St;
    string GameId;
    ref array<string> OptIds;
    ref array<string> OptVals;
    string Hint;       // what the game that ended said
    string HintUid;    // whose seat it named, so a newcomer on that seat is not the durak
    int    Rev;
    ref array<ref OZG_SeatRec> Seats;
    ref array<int> Dealt;
    ref array<string> GameParts;
    ref array<string> Log;
    // Sleights (card sharp's design §6.1): the deals dealt at this place, and
    // how many sleights each player did in the deal in play, by id -- kept
    // with the game, so a restart in mid-deal does not give them back.
    int    DealNo;
    ref array<string> SleightUids;
    ref array<int> SleightCounts;
    // Stakes (stakes-and-games design §3): for every player who sat here at a
    // game played for them, by id, his name, what he brought in all told and
    // his chips now; the requests waiting for the host (the seat asked for,
    // -1 for chips for the seat held); the id on each seat of the deal in
    // play, beside Dealt.
    ref array<string> StakeUids;
    ref array<string> StakeNames;
    ref array<int> StakeIn;
    ref array<int> StakeChips;
    ref array<string> ReqUids;
    ref array<string> ReqNames;
    ref array<int> ReqSeats;
    ref array<string> DealtUids;

    void OZG_SessionRec()
    {
        Version = OZG_Const.FILE_VERSION;
        OptIds = new array<string>();
        OptVals = new array<string>();
        Seats = new array<ref OZG_SeatRec>();
        Dealt = new array<int>();
        GameParts = new array<string>();
        Log = new array<string>();
        SleightUids = new array<string>();
        SleightCounts = new array<int>();
        StakeUids = new array<string>();
        StakeNames = new array<string>();
        StakeIn = new array<int>();
        StakeChips = new array<int>();
        ReqUids = new array<string>();
        ReqNames = new array<string>();
        ReqSeats = new array<int>();
        DealtUids = new array<string>();
    }

    // A file written before stakes, or with lists that do not pair up: every
    // list made anew by script, and the pairs cut to their shortest.
    void MendStakes()
    {
        array<string> uids = new array<string>();
        array<string> names = new array<string>();
        array<int> ins = new array<int>();
        array<int> chips = new array<int>();
        int i;
        if (StakeUids && StakeNames && StakeIn && StakeChips)
        {
            for (i = 0; i < StakeUids.Count(); i++)
            {
                if (i >= StakeNames.Count() || i >= StakeIn.Count() || i >= StakeChips.Count())
                    break;
                uids.Insert(StakeUids[i]);
                names.Insert(StakeNames[i]);
                ins.Insert(StakeIn[i]);
                chips.Insert(StakeChips[i]);
            }
        }
        StakeUids = uids;
        StakeNames = names;
        StakeIn = ins;
        StakeChips = chips;
        array<string> rUids = new array<string>();
        array<string> rNames = new array<string>();
        array<int> rSeats = new array<int>();
        if (ReqUids && ReqNames && ReqSeats)
        {
            for (i = 0; i < ReqUids.Count(); i++)
            {
                if (i >= ReqNames.Count() || i >= ReqSeats.Count())
                    break;
                rUids.Insert(ReqUids[i]);
                rNames.Insert(ReqNames[i]);
                rSeats.Insert(ReqSeats[i]);
            }
        }
        ReqUids = rUids;
        ReqNames = rNames;
        ReqSeats = rSeats;
        array<string> dealt = new array<string>();
        if (DealtUids)
            dealt.Copy(DealtUids);
        DealtUids = dealt;
    }
}

class OZG_Session
{
    ref OZG_SessionRec m_R;
    OZG_Place m_Place;
    ref OZG_Game m_Game;

    protected ref array<string> m_Watchers;
    protected ref map<string, int> m_Asked;
    protected ref map<string, int> m_Moves;
    protected int m_MoveSecond;
    protected int m_HostAway;
    protected bool m_HostWasFree;
    protected int m_Idle;
    protected string m_Note;
    // Laid out by hand: the second of the mod's clock by which the deck must
    // have come out of the placer's hands (0: nothing is waited for), and who
    // placed it.
    protected int m_DeckDue;
    protected string m_DeckUid;
    // Noticing (card sharp's design §5): for each player id, the deals he was
    // dealt in while one of the deal's cards was marked and he neither read
    // nor noticed it. Kept while the session lives and never written: a
    // restart, or a game laid out anew, starts every count again (§8).
    protected ref map<string, int> m_NoticeDeals;
    // A throw of up to two cards (card sharp's design §6.4) comes as two
    // asks: for each player, the revision his first card left the session at
    // and how many cards the throw has sent. A second card that comes after
    // anything else changed is a throw of its own.
    protected ref map<string, int> m_ThrowRev;
    protected ref map<string, int> m_ThrowCards;

    void OZG_Session(OZG_Place place)
    {
        m_Place = place;
        m_R = new OZG_SessionRec();
        m_Watchers = new array<string>();
        m_Asked = new map<string, int>();
        m_Moves = new map<string, int>();
        m_NoticeDeals = new map<string, int>();
        m_ThrowRev = new map<string, int>();
        m_ThrowCards = new map<string, int>();
    }

    string Id()
    {
        return m_R.PlaceId;
    }

    static string PathOf(string id)
    {
        return OZG_Const.DIR_GAMES + "\\" + id + ".json";
    }

    // ------------------------------------------------------------ a new session

    protected void SeatsFrom(OZG_RingResult ring)
    {
        m_R.Seats.Clear();
        m_R.Kind = ring.Kind;
        for (int i = 0; i < ring.Seats.Count(); i++)
        {
            OZG_SeatSpot spot = ring.Seats[i];
            OZG_SeatRec s = new OZG_SeatRec();
            s.No = i;
            s.X = spot.Xyz[0];
            s.Y = spot.Xyz[1];
            s.Z = spot.Xyz[2];
            s.Yaw = spot.Yaw;
            s.Kind = spot.Kind;
            s.St = OZG_Const.SEAT_FREE;
            s.Uid = "";
            s.Name = "";
            m_R.Seats.Insert(s);
        }
    }

    // The first game the settings allow, with its options at their defaults.
    protected void DefaultGame()
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        m_R.GameId = "";
        for (int i = 0; i < cfg.GameIds.Count(); i++)
        {
            if (FitsSet(cfg.GameIds[i]))
            {
                m_R.GameId = cfg.GameIds[i];
                break;
            }
        }
        DefaultOptions();
    }

    // Is the game one the place's set is played with (boards and dice design
    // §2): a game of cards on a deck, a board game on a board?
    protected bool FitsSet(string id)
    {
        OZG_Game sample = OZG_Games.Sample(id);
        if (!sample)
            return false;
        return sample.SetKind() == OZG_Decks.KindOf(m_R.DeckClass);
    }

    protected void DefaultOptions()
    {
        m_R.OptIds.Clear();
        m_R.OptVals.Clear();
        OZG_Game sample = OZG_Games.Sample(m_R.GameId);
        if (!sample)
            return;
        array<ref OZG_Option> opts = new array<ref OZG_Option>();
        sample.OptionList(opts);
        for (int i = 0; i < opts.Count(); i++)
        {
            m_R.OptIds.Insert(opts[i].OptionId);
            m_R.OptVals.Insert(opts[i].DefaultId);
        }
    }

    void Fresh(OZG_RingResult ring, string deckClass, int mode)
    {
        m_R.PlaceId = m_Place.OZG_Id();
        m_R.DeckClass = deckClass;
        m_R.Mode = mode;
        m_R.St = OZG_Const.ST_LOBBY;
        m_R.HostUid = "";
        m_R.HostName = "";
        m_R.Rev = 0;
        SeatsFrom(ring);
        DefaultGame();
    }

    // ------------------------------------------------------------ the file

    void Save()
    {
        m_R.GameParts.Clear();
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY)
            OZG_Text.Parts(m_Game.SaveState(), OZG_Const.PART_BYTES, m_R.GameParts);
        string err;
        if (!JsonFileLoader<OZG_SessionRec>.SaveFile(PathOf(m_R.PlaceId), m_R, err))
            OZG_Log.Error("session " + m_R.PlaceId + " was not written: " + err);
    }

    // After a restart: everything as it stood. False when there is no file, or
    // it does not load or does not add up; the caller then makes an empty
    // lobby.
    bool Load(out bool hadFile)
    {
        string id = m_Place.OZG_Id();
        string path = PathOf(id);
        hadFile = FileExist(path);
        if (!hadFile)
            return false;

        OZG_SessionRec rec;
        string err;
        if (!JsonFileLoader<OZG_SessionRec>.LoadFile(path, rec, err) || !rec)
        {
            OZG_Log.Warn("session " + id + " did not load: " + err);
            return false;
        }
        if (rec.PlaceId != id || !rec.Seats || rec.Seats.Count() == 0)
        {
            OZG_Log.Warn("session " + id + " is not this place's file");
            return false;
        }
        if (!rec.OptIds)
            rec.OptIds = new array<string>();
        if (!rec.OptVals)
            rec.OptVals = new array<string>();
        if (!rec.Dealt)
            rec.Dealt = new array<int>();
        if (!rec.GameParts)
            rec.GameParts = new array<string>();
        if (!rec.Log)
            rec.Log = new array<string>();
        // A file written before sleights: none done.
        array<string> uids = new array<string>();
        array<int> counts = new array<int>();
        if (rec.SleightUids && rec.SleightCounts)
        {
            uids.Copy(rec.SleightUids);
            counts.Copy(rec.SleightCounts);
        }
        rec.SleightUids = uids;
        rec.SleightCounts = counts;
        rec.MendStakes();

        OZG_Game game;
        if (rec.St != OZG_Const.ST_LOBBY)
        {
            game = OZG_Games.Create(rec.GameId);
            string state = OZG_Text.Glue(rec.GameParts);
            string why;
            bool good = game && game.LoadState(state);
            if (good)
                good = game.Sound(why);
            // Every card of the game is found again in the place, the deck
            // and its holding (card sharp's design §8): sleights moved items,
            // and a card the file names that the place no longer holds is a
            // broken game.
            if (good)
            {
                array<ref OZG_DealCard> deal = new array<ref OZG_DealCard>();
                game.DealtRecords(deal);
                array<OZG_Card_Base> things = new array<OZG_Card_Base>();
                int missing = OZG_DeckRead.MatchDeal(m_Place, deal, things);
                if (missing > 0)
                {
                    why = missing.ToString() + " of its " + deal.Count().ToString() + " cards are not in the place";
                    why = why + "; " + OZG_DeckRead.Unmatched(m_Place, deal, things);
                    good = false;
                }
            }
            if (!good)
            {
                OZG_Log.Warn("session " + id + ": the game does not add up (" + why + ")");
                return false;
            }
        }

        m_R = rec;
        m_Game = game;
        // Nobody is at the table the moment a server starts: every seat that
        // had a player is kept until he comes back.
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            s.No = i;
            if (s.St == OZG_Const.SEAT_TAKEN)
                s.St = OZG_Const.SEAT_KEPT;
        }
        return true;
    }

    // An empty lobby on the place, with the ring worked out anew.
    void Blank(bool broken)
    {
        vector at = m_Place.GetPosition();
        vector front = at + m_Place.GetDirection();
        OZG_RingResult ring = OZG_Ring.Compute(at, m_Place.OZG_Mode(), front, m_Place);
        Fresh(ring, m_Place.OZG_DeckClass(), m_Place.OZG_Mode());
        m_Game = null;
        if (broken)
            m_Note = OZG_Const.SAY_BROKEN;
        Save();
    }

    // ------------------------------------------------------------ small things

    protected void AddLog(string line)
    {
        m_R.Log.Insert(line);
        while (m_R.Log.Count() > OZG_Const.LOG_LINES)
            m_R.Log.RemoveOrdered(0);
    }

    int SeatOf(string uid)
    {
        if (uid == "")
            return -1;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.Uid != uid)
                continue;
            if (s.St == OZG_Const.SEAT_TAKEN || s.St == OZG_Const.SEAT_KEPT)
                return i;
        }
        return -1;
    }

    protected int CountOf(int st)
    {
        int n = 0;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            if (m_R.Seats[i].St == st)
                n++;
        }
        return n;
    }

    int Busy()
    {
        return m_R.Seats.Count() - CountOf(OZG_Const.SEAT_FREE);
    }

    bool Present(PlayerBase p)
    {
        if (!p || !m_Place)
            return false;
        if (!p.IsAlive() || p.IsUnconscious())
            return false;
        float dist = vector.Distance(p.GetPosition(), m_Place.GetPosition());
        return dist <= OZG_Cfg.Get().ActRadius;
    }

    protected bool InGame(int no)
    {
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game)
            return false;
        return m_R.Dealt.Find(no) >= 0;
    }

    bool IsHost(string uid)
    {
        return uid != "" && uid == m_R.HostUid;
    }

    // May the host's role be taken by another player now.
    protected bool HostFree()
    {
        if (m_R.HostUid == "")
            return true;
        return m_HostAway >= OZG_Cfg.Get().HostAwayMinutes * 60;
    }

    protected void Say(PlayerBase p, string op, string why)
    {
        if (!p)
            return;
        PlayerIdentity id = p.GetIdentity();
        if (id)
            OZ_Rpc.Notice(id, op, false, why);
    }

    protected void SendTo(PlayerBase p, string op, string body)
    {
        if (!p)
            return;
        PlayerIdentity id = p.GetIdentity();
        if (id)
            OZ_Rpc.ServiceRespond(id, OZG_Const.SERVICE, op, true, body, "");
    }

    void ShowTo(PlayerBase p)
    {
        if (!p)
            return;
        PlayerIdentity id = p.GetIdentity();
        if (id)
            OZ_Rpc.Show(id, OZG_Const.SHOW_OPEN + m_R.PlaceId);
    }

    // ------------------------------------------------------------ views

    protected OZG_WView ViewHead()
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        OZG_WView v = new OZG_WView();
        v.PlaceId = m_R.PlaceId;
        v.Rev = m_R.Rev;
        v.St = m_R.St;
        v.Style = OZG_Decks.StyleOf(m_R.DeckClass);
        v.GameId = m_R.GameId;
        v.OptIds.Copy(m_R.OptIds);
        v.OptVals.Copy(m_R.OptVals);
        v.HostSeat = SeatOf(m_R.HostUid);
        v.HostName = m_R.HostName;
        v.HostFree = 0;
        if (HostFree())
            v.HostFree = 1;
        v.Mode = m_R.Mode;
        v.Kind = m_R.Kind;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            OZG_WSeat w = new OZG_WSeat();
            w.No = i;
            w.Kind = s.Kind;
            w.St = s.St;
            w.Name = s.Name;
            w.X = s.X;
            w.Y = s.Y;
            w.Z = s.Z;
            w.Yaw = s.Yaw;
            w.PoseNo = s.PoseNo;
            if (Staked() && s.Uid != "")
            {
                int at = m_R.StakeUids.Find(s.Uid);
                if (at >= 0)
                {
                    w.Chips = m_R.StakeChips[at];
                    w.BroughtIn = m_R.StakeIn[at];
                }
                else
                {
                    w.Chips = 0;
                    w.BroughtIn = 0;
                }
            }
            v.Seats.Insert(w);
        }
        if (Staked())
            v.Stakes = 1;
        // The games the host may choose: the server's, of the place's set.
        for (int g = 0; g < cfg.GameIds.Count(); g++)
        {
            if (FitsSet(cfg.GameIds[g]))
                v.GameIds.Insert(cfg.GameIds[g]);
        }
        v.Dealt.Copy(m_R.Dealt);
        v.Log.Copy(m_R.Log);
        v.Hint = m_R.Hint;
        v.MarkSeconds = cfg.MarkSeconds;
        v.Reach = cfg.SeatReach + 0.4;
        return v;
    }

    // The three texts of a view for one client.
    protected string ViewBody(OZG_WView v, string uid)
    {
        int seat = SeatOf(uid);
        v.Mine = seat;
        v.IsHost = 0;
        if (IsHost(uid))
            v.IsHost = 1;
        v.Note = m_Note;
        // What this player reads and notices: only a player dealt into the
        // game, and only into the head made for that player (design §4.6).
        // One head serves every player of a push: both lists are emptied
        // first, so nobody is sent another's.
        v.Reads.Clear();
        v.Notices.Clear();
        bool dealt = seat >= 0 && m_R.Dealt.Find(seat) >= 0;
        bool reading = OZG_Cfg.Get().Cheating == 1;
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY && dealt && reading)
        {
            m_Game.ReadsFor(seat, uid, v.Reads);
            m_Game.NoticesFor(seat, uid, v.Notices);
        }
        // Sleights (card sharp's design §6, §10): in play, for a player dealt
        // in; between deals, for the dealer, who is the host on his seat, with
        // the cards of the deck he reads. Each head is made for its player
        // alone, as the reads are.
        SleightHead(v, uid, seat, dealt);
        StakeHead(v, uid);

        string head;
        string err;
        JsonFileLoader<OZG_WView>.MakeData(v, head, err, false);

        string shared = "";
        string own = "";
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY)
        {
            shared = m_Game.PublicView();
            if (seat >= 0 && m_R.Dealt.Find(seat) >= 0)
                own = m_Game.SeatView(seat);
        }
        return OZG_Text.Join3(head, shared, own);
    }

    void ViewTo(PlayerBase p)
    {
        string uid = OZG_Who.Uid(p);
        SendTo(p, OZG_Const.OP_VIEW, ViewBody(ViewHead(), uid));
        // Said once, to the first who looks.
        m_Note = "";
    }

    // For the stand: the head of the view this player would be sent now.
    string HeadFor(string uid)
    {
        string head;
        string shared;
        string own;
        OZG_Text.Cut3(ViewBody(ViewHead(), uid), head, shared, own);
        return head;
    }

    void Push()
    {
        OZG_WView v = ViewHead();
        int i;
        for (i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.St != OZG_Const.SEAT_TAKEN)
                continue;
            SendTo(OZG_Sessions.PlayerOf(s.Uid), OZG_Const.OP_VIEW, ViewBody(v, s.Uid));
        }
        for (i = 0; i < m_Watchers.Count(); i++)
            SendTo(OZG_Sessions.PlayerOf(m_Watchers[i]), OZG_Const.OP_VIEW, ViewBody(v, m_Watchers[i]));
    }

    void Pulse()
    {
        if (!m_Place)
            return;
        m_Place.OZG_Pulse(m_R.St, CountOf(OZG_Const.SEAT_TAKEN), Busy(), m_R.Seats.Count(), m_R.Rev);
    }

    void Changed()
    {
        SyncStacks();
        m_R.Rev = m_R.Rev + 1;
        Save();
        Pulse();
        PassHeard();
        Push();
    }

    // What the last change made heard goes onto the place, where every client
    // near it plays it (design §17): a deal, a move, a bout closed, a seat
    // dropped, an absent seat passed over.
    protected void PassHeard()
    {
        if (!m_Game || !m_Place)
            return;
        array<int> heard = new array<int>();
        m_Game.TakeHeard(heard);
        if (heard.Count() > 0)
            m_Place.OZG_Hear(heard);
    }

    // Everybody who looks at this place is told it is gone.
    void CloseAll(string why)
    {
        OZG_WClosed c = new OZG_WClosed();
        c.PlaceId = m_R.PlaceId;
        c.Why = why;
        string body;
        string err;
        JsonFileLoader<OZG_WClosed>.MakeData(c, body, err, false);
        int i;
        for (i = 0; i < m_R.Seats.Count(); i++)
        {
            if (m_R.Seats[i].Uid != "")
                SendTo(OZG_Sessions.PlayerOf(m_R.Seats[i].Uid), OZG_Const.OP_CLOSED, body);
        }
        for (i = 0; i < m_Watchers.Count(); i++)
            SendTo(OZG_Sessions.PlayerOf(m_Watchers[i]), OZG_Const.OP_CLOSED, body);
        m_Watchers.Clear();
    }

    protected void CloseFor(string uid, string why)
    {
        OZG_WClosed c = new OZG_WClosed();
        c.PlaceId = m_R.PlaceId;
        c.Why = why;
        string body;
        string err;
        JsonFileLoader<OZG_WClosed>.MakeData(c, body, err, false);
        SendTo(OZG_Sessions.PlayerOf(uid), OZG_Const.OP_CLOSED, body);
    }

    // ------------------------------------------------------------ the game goes on

    // The game ended: the result stands until the next deal.
    protected void Over()
    {
        // The deal's last stacks into the book, while it still counts as played.
        SyncStacks();
        m_R.St = OZG_Const.ST_FINISHED;
        m_R.Hint = m_Game.Outcome();
        m_R.HintUid = "";
        int eq = m_R.Hint.IndexOf("=");
        if (eq > 0)
        {
            string number = m_R.Hint.Substring(eq + 1, m_R.Hint.Length() - eq - 1);
            int no = number.ToInt();
            if (no >= 0 && no < m_R.Seats.Count())
                m_R.HintUid = m_R.Seats[no].Uid;
        }
        // A seat left with cards has nothing left to wait for.
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.St == OZG_Const.SEAT_LEFT)
            {
                s.St = OZG_Const.SEAT_FREE;
                s.Uid = "";
                s.Name = "";
            }
        }
        AddLog("end " + m_R.Hint);
    }

    // A move a seat MAY make but need not is passed over while its player is
    // not present, so one absent player does not stop every bout. Returns how
    // many were passed over.
    protected int Settle()
    {
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game)
            return 0;
        array<int> absent = new array<int>();
        for (int i = 0; i < m_R.Dealt.Count(); i++)
        {
            int no = m_R.Dealt[i];
            if (no < 0 || no >= m_R.Seats.Count())
                continue;
            int st = m_R.Seats[no].St;
            if (st == OZG_Const.SEAT_KEPT || st == OZG_Const.SEAT_LEFT)
                absent.Insert(no);
        }
        int skipped = 0;
        if (absent.Count() > 0)
            skipped = m_Game.SkipAbsent(absent);
        if (m_Game.IsOver())
            Over();
        return skipped;
    }

    // ------------------------------------------------------------ seats

    protected bool Takeable(OZG_SeatRec s)
    {
        if (s.St == OZG_Const.SEAT_LEFT)
            return true;
        if (s.St == OZG_Const.SEAT_FREE)
            return m_R.St != OZG_Const.ST_PLAYING;
        return false;
    }

    protected int NearestTakeable(vector at)
    {
        float reach = OZG_Cfg.Get().SeatReach + 0.4;
        int best = -1;
        float bestFar = reach;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (!Takeable(s))
                continue;
            float dist = vector.Distance(at, s.Where());
            if (dist <= bestFar)
            {
                bestFar = dist;
                best = i;
            }
        }
        return best;
    }

    protected void PutOn(PlayerBase player, OZG_SeatRec s)
    {
        player.SetPosition(s.Where());
        vector o = player.GetOrientation();
        o[0] = s.Yaw;
        player.SetOrientation(o);
    }

    void SetHost(PlayerBase player)
    {
        m_R.HostUid = OZG_Who.Uid(player);
        m_R.HostName = OZG_Who.Name(player);
        m_HostAway = 0;
    }

    // Server: the deck was asked out of the placer's hands.
    void AwaitDeck(PlayerBase by)
    {
        m_DeckDue = OZG_Sessions.Now() + OZG_Const.DECK_WAIT;
        m_DeckUid = OZG_Who.Uid(by);
    }

    bool Sit(PlayerBase player, int no, out string why)
    {
        why = OZG_Const.SAY_CANNOT;
        if (!player || !player.IsAlive() || player.IsUnconscious())
            return false;
        if (player.IsRestrained() || player.IsInVehicle())
            return false;
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return false;
        if (OZG_Sessions.SeatedIn(uid))
        {
            why = OZG_Const.SAY_OTHER_SEAT;
            return false;
        }
        if (no < 0 || no >= m_R.Seats.Count())
        {
            why = OZG_Const.SAY_NOT_NOW;
            return false;
        }
        OZG_SeatRec s = m_R.Seats[no];
        if (s.St == OZG_Const.SEAT_TAKEN || s.St == OZG_Const.SEAT_KEPT)
        {
            why = OZG_Const.SAY_SEAT_TAKEN;
            return false;
        }
        if (!Takeable(s))
        {
            why = OZG_Const.SAY_NOT_NOW;
            return false;
        }
        float reach = OZG_Cfg.Get().SeatReach + 0.4;
        if (vector.Distance(player.GetPosition(), s.Where()) > reach)
        {
            why = OZG_Const.SAY_TOO_FAR;
            return false;
        }
        // A game for stakes: the host lets a player in (design §3).
        if (Staked() && !m_Letting && m_R.HostUid != "" && uid != m_R.HostUid)
        {
            AskFor(uid, OZG_Who.Name(player), no);
            why = OZG_Const.SAY_ASKED;
            Changed();
            return false;
        }

        bool takesOver = s.St == OZG_Const.SEAT_LEFT;
        s.St = OZG_Const.SEAT_TAKEN;
        s.Uid = uid;
        s.Name = OZG_Who.Name(player);
        s.PoseNo = 0;
        PutOn(player, s);
        m_Watchers.RemoveItem(uid);
        if (m_R.HostUid == "")
            SetHost(player);
        // A player let in at a game for stakes gets one line, with his chips
        // (LetIn), not this one too.
        // The lines that name a seat's player carry the name: a window words
        // them later, when somebody else may sit there (2026-11-25).
        if (takesOver)
            AddLog("over " + no.ToString() + " " + s.Name);
        else if (!m_Letting)
            AddLog("sit " + no.ToString() + " " + s.Name);
        m_Idle = 0;
        Changed();
        why = "";
        return true;
    }

    // The role goes to the next present seated player around the mat; with
    // none, to the first who sits.
    protected void PassHost(int from)
    {
        m_R.HostUid = "";
        m_R.HostName = "";
        int n = m_R.Seats.Count();
        for (int k = 1; k <= n; k++)
        {
            OZG_SeatRec s = m_R.Seats[(from + k) % n];
            if (s.St != OZG_Const.SEAT_TAKEN)
                continue;
            m_R.HostUid = s.Uid;
            m_R.HostName = s.Name;
            m_HostAway = 0;
            AddLog("host " + s.No.ToString() + " " + s.Name);
            return;
        }
    }

    // A seat loses its player: by his own word, or the host's.
    protected bool Vacate(int no, string how, out string why)
    {
        why = "";
        OZG_SeatRec s = m_R.Seats[no];
        bool playing = InGame(no);
        // A seat of a game for stakes is not left with its cards for another
        // to play on: its chips are its player's.
        string way = how;
        if (playing && Staked())
            way = OZG_Const.HOW_DISCARD;
        if (playing)
        {
            if (way == OZG_Const.HOW_DISCARD)
            {
                m_Game.DropSeat(no);
                s.St = OZG_Const.SEAT_FREE;
                AddLog("drop " + no.ToString());
            }
            else if (way == OZG_Const.HOW_LEAVE)
            {
                s.St = OZG_Const.SEAT_LEFT;
                AddLog("left " + no.ToString());
            }
            else
            {
                why = OZG_Const.SAY_HOW;
                return false;
            }
        }
        else
        {
            s.St = OZG_Const.SEAT_FREE;
            AddLog("stand " + no.ToString());
        }

        string was = s.Uid;
        s.Uid = "";
        s.Name = "";
        s.PoseNo = 0;
        DropFan(was);
        if (was != "" && was == m_R.HostUid)
            PassHost(no);
        if (playing)
            Settle();
        return true;
    }

    // ------------------------------------------------------------ stakes

    // While the host lets a player in, Sit seats him without asking again.
    protected bool m_Letting;

    // Does the game at this place play for stakes (design §3)?
    bool Staked()
    {
        OZG_Game sample = OZG_Games.Sample(m_R.GameId);
        if (!sample)
            return false;
        return sample.UsesStakes();
    }

    int ChipsOf(string uid)
    {
        int at = m_R.StakeUids.Find(uid);
        if (at < 0 || uid == "")
            return 0;
        return m_R.StakeChips[at];
    }

    // A player's line of the book, made when he has none.
    protected int StakeLine(string uid, string name)
    {
        int at = m_R.StakeUids.Find(uid);
        if (at >= 0)
        {
            if (name != "")
                m_R.StakeNames[at] = name;
            return at;
        }
        m_R.StakeUids.Insert(uid);
        m_R.StakeNames.Insert(name);
        m_R.StakeIn.Insert(0);
        m_R.StakeChips.Insert(0);
        return m_R.StakeUids.Count() - 1;
    }

    protected void StakeAdd(string uid, string name, int amount)
    {
        int at = StakeLine(uid, name);
        m_R.StakeIn[at] = m_R.StakeIn[at] + amount;
        m_R.StakeChips[at] = m_R.StakeChips[at] + amount;
    }

    protected void AskFor(string uid, string name, int seat)
    {
        int at = m_R.ReqUids.Find(uid);
        if (at >= 0)
        {
            m_R.ReqNames[at] = name;
            m_R.ReqSeats[at] = seat;
            return;
        }
        m_R.ReqUids.Insert(uid);
        m_R.ReqNames.Insert(name);
        m_R.ReqSeats.Insert(seat);
    }

    protected void Unask(string uid)
    {
        int at = m_R.ReqUids.Find(uid);
        if (at < 0)
            return;
        m_R.ReqUids.RemoveOrdered(at);
        m_R.ReqNames.RemoveOrdered(at);
        m_R.ReqSeats.RemoveOrdered(at);
    }

    // The chips of every seat of the deal, as the game counts them now, into
    // the book: after every change while the deal is played, so a restart or
    // a fold finds them there, and once more as it ends (Over). Between deals
    // the book is the truth: an ended deal does not know the chips given
    // since, and copying its stacks again took them back (seen 2026-11-25: a
    // rebuy, and the 1000 of a bot seated again, gone at the next change).
    protected void SyncStacks()
    {
        if (!m_Game || m_R.St != OZG_Const.ST_PLAYING || !m_Game.UsesStakes())
            return;
        for (int k = 0; k < m_R.Dealt.Count() && k < m_R.DealtUids.Count(); k++)
        {
            int chips = m_Game.StackOf(m_R.Dealt[k]);
            string uid = m_R.DealtUids[k];
            if (chips < 0 || uid == "")
                continue;
            int at = StakeLine(uid, "");
            m_R.StakeChips[at] = chips;
        }
    }

    protected void StakeHead(OZG_WView v, string uid)
    {
        v.MyChips = 0;
        v.MyIn = 0;
        v.AskedChips = 0;
        v.ReqUids.Clear();
        v.ReqNames.Clear();
        v.ReqSeats.Clear();
        if (!Staked())
            return;
        int at = m_R.StakeUids.Find(uid);
        if (at >= 0)
        {
            v.MyChips = m_R.StakeChips[at];
            v.MyIn = m_R.StakeIn[at];
        }
        if (m_R.ReqUids.Find(uid) >= 0)
            v.AskedChips = 1;
        if (!IsHost(uid))
            return;
        v.ReqUids.Copy(m_R.ReqUids);
        v.ReqNames.Copy(m_R.ReqNames);
        v.ReqSeats.Copy(m_R.ReqSeats);
    }

    // The host lets a player in on the seat he asked for (or the free one
    // nearest him) with `amount` chips; or, for one seated, gives the chips
    // he asked for.
    protected string LetIn(string uid, int amount)
    {
        int r = m_R.ReqUids.Find(uid);
        if (r < 0)
            return OZG_Const.SAY_NOT_NOW;
        if (amount < 0 || amount > OZG_Const.STAKE_MOST)
            return OZG_Const.SAY_BAD_AMOUNT;
        if (m_R.St == OZG_Const.ST_PLAYING)
            return OZG_Const.SAY_PLAYING;
        string name = m_R.ReqNames[r];
        int wanted = m_R.ReqSeats[r];
        int mine = SeatOf(uid);
        if (wanted < 0 || mine >= 0)
        {
            if (mine < 0)
            {
                Unask(uid);
                Changed();
                return OZG_Const.SAY_GONE;
            }
            if (amount < 1)
                return OZG_Const.SAY_BAD_AMOUNT;
            StakeAdd(uid, name, amount);
            Unask(uid);
            string given = "chips " + mine.ToString() + " " + amount.ToString();
            AddLog(given + " " + name);
            Changed();
            return "";
        }
        PlayerBase p = OZG_Sessions.PlayerOf(uid);
        if (!p)
        {
            Unask(uid);
            Changed();
            return OZG_Const.SAY_GONE;
        }
        int no = wanted;
        bool free = no >= 0 && no < m_R.Seats.Count() && Takeable(m_R.Seats[no]);
        if (!free)
            no = NearestTakeable(p.GetPosition());
        if (no < 0)
            return OZG_Const.SAY_NO_SEATS;
        string why;
        if (!SeatLetIn(p, no, amount, why))
            return why;
        ShowTo(p);
        return "";
    }

    // Seats a player the host let in, with his chips; the stand's bots sit
    // this way too.
    bool SeatLetIn(PlayerBase p, int no, int amount, out string why)
    {
        m_Letting = true;
        bool sat = Sit(p, no, why);
        m_Letting = false;
        if (!sat)
            return false;
        string uid = OZG_Who.Uid(p);
        string who = OZG_Who.Name(p);
        StakeAdd(uid, who, amount);
        Unask(uid);
        string line = "let " + no.ToString() + " " + amount.ToString();
        AddLog(line + " " + who);
        Changed();
        return true;
    }

    protected string Refuse(string uid)
    {
        if (m_R.ReqUids.Find(uid) < 0)
            return OZG_Const.SAY_NOT_NOW;
        Unask(uid);
        PlayerBase p = OZG_Sessions.PlayerOf(uid);
        if (p)
            Say(p, OZG_Const.NOTICE_SIT, OZG_Const.SAY_REFUSED);
        Changed();
        return "";
    }

    // The host gives the player on seat `no` chips: a buy-in or a rebuy.
    protected string GiveChips(int no, int amount)
    {
        if (no < 0 || no >= m_R.Seats.Count())
            return OZG_Const.SAY_NOT_NOW;
        if (!Staked())
            return OZG_Const.SAY_CANNOT;
        if (m_R.St == OZG_Const.ST_PLAYING)
            return OZG_Const.SAY_PLAYING;
        if (amount < 1 || amount > OZG_Const.STAKE_MOST)
            return OZG_Const.SAY_BAD_AMOUNT;
        OZG_SeatRec s = m_R.Seats[no];
        if (s.Uid == "" || (s.St != OZG_Const.SEAT_TAKEN && s.St != OZG_Const.SEAT_KEPT))
            return OZG_Const.SAY_NOT_NOW;
        StakeAdd(s.Uid, s.Name, amount);
        Unask(s.Uid);
        string given = "chips " + no.ToString() + " " + amount.ToString();
        AddLog(given + " " + s.Name);
        Changed();
        return "";
    }

    // The host picks the game played at this place, between deals.
    protected string ChooseGame(string id)
    {
        if (m_R.St == OZG_Const.ST_PLAYING)
            return OZG_Const.SAY_PLAYING;
        if (!FitsSet(id) || !OZG_Cfg.Get().GameOn(id))
            return OZG_Const.NO_GAME;
        if (id == m_R.GameId)
            return "";
        m_R.GameId = id;
        m_R.OptIds.Clear();
        m_R.OptVals.Clear();
        DefaultOptions();
        m_R.Hint = "";
        m_R.HintUid = "";
        m_R.St = OZG_Const.ST_LOBBY;
        m_R.Dealt.Clear();
        m_R.DealtUids.Clear();
        m_Game = null;
        // A new game starts a new log. A line of a move names no game, and a
        // window words it by the game in play: the moves of the last one came
        // out in the new one's words (seen 2026-11-25, hold'em then preferans).
        m_R.Log.Clear();
        AddLog("game " + id);
        Changed();
        return "";
    }

    // The paper of a game for stakes (design §3), when the game is folded or
    // the deck gathered and somebody brought chips in.
    void MakePaper(PlayerBase by, vector at)
    {
        SyncStacks();
        OZG_StakePaper.Make(m_R, by, at);
    }

    // ------------------------------------------------------------ what players ask

    // The interaction with the place: what it means depends on the player.
    void SitAction(PlayerBase player)
    {
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return;
        if (SeatOf(uid) >= 0)
        {
            ViewTo(player);
            ShowTo(player);
            return;
        }
        if (OZG_Sessions.SeatedIn(uid))
        {
            Say(player, OZG_Const.NOTICE_SIT, OZG_Const.SAY_OTHER_SEAT);
            return;
        }

        int now = OZG_Sessions.Now();
        int asked;
        bool marked = false;
        if (m_Asked.Find(uid, asked))
            marked = now - asked <= OZG_Cfg.Get().MarkSeconds;

        if (marked)
        {
            int no = NearestTakeable(player.GetPosition());
            if (no >= 0)
            {
                string why;
                m_Asked.Remove(uid);
                if (Sit(player, no, why))
                    ShowTo(player);
                else
                    Say(player, OZG_Const.NOTICE_SIT, why);
                return;
            }
        }

        // The first step, or a player who stands on no mark: the seats. With
        // none to take now -- a game runs and no seat was left with cards --
        // there is nothing to mark, and the player is told why.
        bool any = false;
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            if (Takeable(m_R.Seats[i]))
                any = true;
        }
        if (!any)
        {
            Say(player, OZG_Const.NOTICE_SIT, OZG_Const.SAY_NO_SEATS);
            return;
        }
        m_Asked.Set(uid, now);
        SendTo(player, OZG_Const.OP_SEATS, ViewBody(ViewHead(), uid));
    }

    void WatchAction(PlayerBase player)
    {
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return;
        if (SeatOf(uid) < 0 && m_Watchers.Find(uid) < 0)
            m_Watchers.Insert(uid);
        ViewTo(player);
        ShowTo(player);
    }

    // One op of the service. Returns "" when it was taken; otherwise the key
    // of the refusal.
    string Ask(PlayerBase player, string op, OZG_WAsk ask)
    {
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return OZG_Const.SAY_CANNOT;
        int mine = SeatOf(uid);
        string why;

        if (op == OZG_Const.OP_OPEN)
        {
            if (mine < 0 && m_Watchers.Find(uid) < 0)
                return OZG_Const.SAY_NOT_SEATED;
            ViewTo(player);
            return "";
        }

        if (op == OZG_Const.OP_WATCH)
        {
            float away = vector.Distance(player.GetPosition(), m_Place.GetPosition());
            if (away > OZG_Cfg.Get().WatchRadius)
                return OZG_Const.SAY_TOO_FAR;
            if (mine < 0 && m_Watchers.Find(uid) < 0)
                m_Watchers.Insert(uid);
            ViewTo(player);
            return "";
        }

        if (op == OZG_Const.OP_UNWATCH)
        {
            m_Watchers.RemoveItem(uid);
            return "";
        }

        if (op == OZG_Const.OP_STAND)
        {
            if (mine < 0)
                return OZG_Const.SAY_NOT_SEATED;
            if (!Vacate(mine, ask.How, why))
                return why;
            Changed();
            CloseFor(uid, OZG_Const.SAY_STOOD);
            return "";
        }

        if (op == OZG_Const.OP_ACT)
            return Act(player, uid, mine, ask.Move);

        if (op == OZG_Const.OP_SLIP)
            return Slip(player, uid, mine, ask);
        if (op == OZG_Const.OP_SWAP)
            return Swap(player, uid, mine, ask);
        if (op == OZG_Const.OP_THROW)
            return Throw(player, uid, mine, ask);

        if (op == OZG_Const.OP_POSE)
        {
            if (mine < 0)
                return OZG_Const.SAY_NOT_SEATED;
            m_R.Seats[mine].PoseNo = ask.PoseNo;
            Save();
            return "";
        }

        if (op == OZG_Const.OP_HOST)
        {
            if (mine < 0 || !Present(player))
                return OZG_Const.SAY_NOT_SEATED;
            if (IsHost(uid))
                return "";
            if (!HostFree())
                return OZG_Const.SAY_HOST_HERE;
            SetHost(player);
            AddLog("host " + mine.ToString());
            Changed();
            return "";
        }

        if (op == OZG_Const.OP_REBUY)
        {
            if (mine < 0)
                return OZG_Const.SAY_NOT_SEATED;
            if (!Staked())
                return OZG_Const.SAY_CANNOT;
            AskFor(uid, OZG_Who.Name(player), -1);
            Changed();
            return "";
        }

        // The rest is the host's.
        if (!IsHost(uid))
            return OZG_Const.SAY_NOT_HOST;

        if (op == OZG_Const.OP_GAME)
            return ChooseGame(ask.OptValue);
        if (op == OZG_Const.OP_LET)
            return LetIn(ask.How, ask.OptValue.ToInt());
        if (op == OZG_Const.OP_REFUSE)
            return Refuse(ask.How);
        if (op == OZG_Const.OP_CHIPS)
            return GiveChips(ask.SeatNo, ask.OptValue.ToInt());

        if (op == OZG_Const.OP_OPT)
            return Opt(ask.OptId, ask.OptValue);

        if (op == OZG_Const.OP_DEAL)
            return Deal(player, uid, ask.Stack);

        if (op == OZG_Const.OP_KICK)
        {
            int no = ask.SeatNo;
            if (no < 0 || no >= m_R.Seats.Count())
                return OZG_Const.SAY_NOT_NOW;
            OZG_SeatRec s = m_R.Seats[no];
            if (s.St != OZG_Const.SEAT_TAKEN && s.St != OZG_Const.SEAT_KEPT)
                return OZG_Const.SAY_NOT_NOW;
            string gone = s.Uid;
            if (!Vacate(no, ask.How, why))
                return why;
            Changed();
            if (gone == uid)
                CloseFor(gone, OZG_Const.SAY_STOOD);
            else
                CloseFor(gone, OZG_Const.SAY_KICKED);
            return "";
        }

        if (op == OZG_Const.OP_DROP)
        {
            int left = ask.SeatNo;
            if (left < 0 || left >= m_R.Seats.Count())
                return OZG_Const.SAY_NOT_NOW;
            OZG_SeatRec ls = m_R.Seats[left];
            if (ls.St != OZG_Const.SEAT_LEFT)
                return OZG_Const.SAY_NOT_NOW;
            if (InGame(left))
                m_Game.DropSeat(left);
            ls.St = OZG_Const.SEAT_FREE;
            AddLog("drop " + left.ToString());
            Settle();
            Changed();
            return "";
        }

        return OZG_Const.SAY_CANNOT;
    }

    protected string Opt(string id, string value)
    {
        if (m_R.St == OZG_Const.ST_PLAYING)
            return OZG_Const.SAY_PLAYING;
        OZG_Game sample = OZG_Games.Sample(m_R.GameId);
        if (!sample)
            return OZG_Const.NO_GAME;
        array<ref OZG_Option> opts = new array<ref OZG_Option>();
        sample.OptionList(opts);
        for (int i = 0; i < opts.Count(); i++)
        {
            OZG_Option o = opts[i];
            if (o.OptionId != id)
                continue;
            if (o.ValueIds.Find(value) < 0)
                return OZG_Const.SAY_BAD_OPTION;
            int at = m_R.OptIds.Find(id);
            if (at < 0)
            {
                m_R.OptIds.Insert(id);
                m_R.OptVals.Insert(value);
            }
            else
            {
                m_R.OptVals[at] = value;
            }
            AddLog("opt " + id + "=" + value);
            Changed();
            return "";
        }
        return OZG_Const.SAY_BAD_OPTION;
    }

    protected string Deal(PlayerBase player, string uid, string stackText)
    {
        if (m_R.St == OZG_Const.ST_PLAYING)
            return OZG_Const.SAY_PLAYING;
        if (m_DeckDue > 0)
            return OZG_Const.SAY_NOT_NOW;
        if (!OZG_Cfg.Get().GameOn(m_R.GameId))
            return OZG_Const.NO_GAME;
        OZG_Game game = OZG_Games.Create(m_R.GameId);
        if (!game)
            return OZG_Const.NO_GAME;

        // A kept seat is not dealt in.
        array<int> seats = new array<int>();
        int i;
        for (i = 0; i < m_R.Seats.Count(); i++)
        {
            if (m_R.Seats[i].St == OZG_Const.SEAT_TAKEN)
                seats.Insert(i);
        }
        if (seats.Count() < game.MinPlayers() || seats.Count() > game.MaxPlayers())
            return OZG_Const.NO_PLAYERS;
        // A game for stakes deals the seats with chips (design §3).
        array<int> amounts = new array<int>();
        if (game.UsesStakes())
        {
            array<int> withChips = new array<int>();
            for (i = 0; i < seats.Count(); i++)
            {
                int chips = ChipsOf(m_R.Seats[seats[i]].Uid);
                if (chips <= 0)
                    continue;
                withChips.Insert(seats[i]);
                amounts.Insert(chips);
            }
            if (withChips.Count() < game.MinPlayers())
                return OZG_Const.SAY_NO_CHIPS;
            seats.Copy(withChips);
        }

        map<string, string> options = new map<string, string>();
        for (i = 0; i < m_R.OptIds.Count(); i++)
        {
            if (i < m_R.OptVals.Count())
                options.Set(m_R.OptIds[i], m_R.OptVals[i]);
        }

        // What the next deal should know: the host's seat, and what the game
        // that ended said -- unless the seat it named has another player now.
        string hint = "";
        int hostSeat = SeatOf(m_R.HostUid);
        if (hostSeat >= 0)
            hint = OZG_Const.HINT_HOST + "=" + hostSeat.ToString();
        if (m_R.Hint != "" && m_R.HintUid != "")
        {
            int named = SeatOf(m_R.HintUid);
            bool same = named >= 0 && seats.Find(named) >= 0;
            if (same)
            {
                int eq = m_R.Hint.IndexOf("=");
                same = eq > 0 && m_R.Hint.Substring(eq + 1, m_R.Hint.Length() - eq - 1).ToInt() == named;
            }
            if (same)
            {
                if (hint != "")
                    hint = hint + ";";
                hint = hint + m_R.Hint;
            }
        }

        // The cards of this deal are the cards the deck holds that are of the
        // game's set, however many of a face there are. A place that lost its
        // deck gets one of its class.
        if (m_Place && !m_Place.OZG_Deck())
            m_Place.OZG_MakeDeck(m_R.DeckClass);

        // A board game is dealt the pieces its board holds (boards and dice
        // design §2): no stack, no fans, no sleights of the card sharp.
        if (game.SetKind() != OZG_Const.SET_CARDS)
        {
            if (stackText != "")
                return OZG_Const.SAY_SLEIGHT_NOT_GAME;
            return DealPieces(game, seats, options, hint);
        }

        // The dealer's three (card sharp's design §6.5): checked whole and
        // the deal tried on paper before a single item moves, so that a deal
        // the stack would leave short is refused with nothing done.
        OZG_Stack stack = new OZG_Stack();
        stack.Read(stackText);
        int dealer = SeatOf(uid);
        if (!stack.Empty() && !game.HasSleights())
            return OZG_Const.SAY_SLEIGHT_NOT_GAME;
        if (!stack.Empty())
        {
            string why = StackCheck(player, uid, stack, game, seats, options, hint);
            if (why != "")
                return why;
            StackApply(player, stack);
        }

        array<EntityAI> things = new array<EntityAI>();
        OZG_DeckRead.ThingsOf(m_Place, things);
        array<ref OZG_DealCard> all = new array<ref OZG_DealCard>();
        array<string> keys = new array<string>();
        OZG_DeckRead.RecordsOf(things, all, keys);
        array<string> ofGame = new array<string>();
        game.CardsUsed(ofGame);
        array<ref OZG_DealCard> cards = new array<ref OZG_DealCard>();
        for (i = 0; i < all.Count(); i++)
        {
            if (ofGame.Find(OZG_Cards.FaceOf(all[i].Name)) >= 0)
                cards.Insert(all[i]);
        }
        game.TakeCards(cards);
        // The cards the dealer chose, by their names in this deal.
        if (stack.PickCards.Count() > 0)
            game.DealFirst(dealer, NamesOf(stack.PickCards, all, keys));

        if (game.UsesStakes())
            game.TakeStacks(seats, amounts);
        string refusal;
        OZG_Random chance = new OZG_Random();
        if (!game.Begin(seats, options, chance, hint, refusal))
            return refusal;

        m_Game = game;
        m_R.St = OZG_Const.ST_PLAYING;
        m_R.Dealt.Copy(seats);
        m_R.DealtUids.Clear();
        for (i = 0; i < seats.Count(); i++)
            m_R.DealtUids.Insert(m_R.Seats[seats[i]].Uid);
        m_R.Hint = "";
        m_R.HintUid = "";
        // A new deal: every player's sleights counted anew; the dealer's own
        // three belong to it.
        m_R.DealNo = m_R.DealNo + 1;
        m_R.SleightUids.Clear();
        m_R.SleightCounts.Clear();
        m_ThrowRev.Clear();
        m_ThrowCards.Clear();
        if (!stack.Empty())
            CountSleight(uid, stack.Sleights());
        AddLog("deal");
        Settle();
        // Noticing (design §5): after the deal and before anybody is sent it,
        // so that a card noticed now is in the first view of this deal.
        NoticeAfterDeal(seats, cards, chance);
        Changed();
        OZG_Log.Dbg("session " + m_R.PlaceId + " deal: " + m_Game.Brief());
        if (!stack.Empty())
            StackRolls(uid, dealer, stack);
        return "";
    }

    // A board game begun with the pieces the place's board holds.
    protected string DealPieces(OZG_Game game, array<int> seats, map<string, string> options, string hint)
    {
        array<string> held = new array<string>();
        OZG_BoardRead.FacesOf(m_Place, held);
        array<string> used = new array<string>();
        game.CardsUsed(used);
        array<string> faces = new array<string>();
        int i;
        for (i = 0; i < held.Count(); i++)
        {
            if (used.Find(held[i]) >= 0)
                faces.Insert(held[i]);
        }
        game.TakePieces(faces);
        string refusal;
        OZG_Random chance = new OZG_Random();
        if (!game.Begin(seats, options, chance, hint, refusal))
            return refusal;

        m_Game = game;
        m_R.St = OZG_Const.ST_PLAYING;
        m_R.Dealt.Copy(seats);
        m_R.DealtUids.Clear();
        for (i = 0; i < seats.Count(); i++)
            m_R.DealtUids.Insert(m_R.Seats[seats[i]].Uid);
        m_R.Hint = "";
        m_R.HintUid = "";
        m_R.DealNo = m_R.DealNo + 1;
        AddLog("deal");
        Settle();
        Changed();
        OZG_Log.Dbg("session " + m_R.PlaceId + " game: " + m_Game.Brief());
        return "";
    }

    // The names a deal gave the cards `chosen`: each item's key looked up
    // beside the records the deal was read into.
    protected array<string> NamesOf(array<OZG_Card_Base> chosen, array<ref OZG_DealCard> all, array<string> keys)
    {
        array<string> names = new array<string>();
        for (int i = 0; i < chosen.Count(); i++)
        {
            int at = keys.Find(OZG_Keys.Of(chosen[i]));
            if (at >= 0 && at < all.Count())
                names.Insert(all[at].Name);
        }
        return names;
    }

    // ------------------------------------------------------------ the dealer's three

    // The stack, checked whole (card sharp's design §6.5): sleights open to
    // the dealer, no more than his sleights and picks allow, every card where
    // the stack says -- a card put in in his pockets, a card taken out or
    // chosen in the place and read by him, a chosen card of the game's set --
    // no card named twice; a card out by chance picked now among the rest;
    // and the deal, with the deck as the stack will leave it, tried on a game
    // of paper. "" or the refusal.
    protected string StackCheck(PlayerBase player, string uid, OZG_Stack stack, OZG_Game game, array<int> seats, map<string, string> options, string hint)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        int mine = SeatOf(uid);
        if (mine < 0 || seats.Find(mine) < 0)
            return OZG_Const.SAY_NOT_SEATED;
        if (!Present(player))
            return OZG_Const.SAY_TOO_FAR;
        if (stack.Sleights() > cfg.SleightsPerDeal)
            return OZG_Const.SAY_SLEIGHT_NO_MORE;
        if (stack.PickKeys.Count() > cfg.DealerPicks)
            return OZG_Const.SAY_SLEIGHT_NO_MORE;

        array<string> ofGame = new array<string>();
        game.CardsUsed(ofGame);
        array<EntityAI> placed = new array<EntityAI>();
        OZG_DeckRead.ThingsOf(m_Place, placed);
        array<OZG_Card_Base> named = new array<OZG_Card_Base>();
        stack.InCards.Clear();
        stack.OutCards.Clear();
        stack.PickCards.Clear();
        int i;
        OZG_Card_Base card;
        for (i = 0; i < stack.InKeys.Count(); i++)
        {
            card = OZG_DeckRead.PocketCard(player, stack.InKeys[i]);
            if (!card || named.Find(card) >= 0)
                return OZG_Const.SAY_SLEIGHT_NO_CARD;
            named.Insert(card);
            stack.InCards.Insert(card);
        }
        for (i = 0; i < stack.PickKeys.Count(); i++)
        {
            card = OZG_Card_Base.Cast(OZG_Keys.Find(stack.PickKeys[i]));
            bool chosen = card && placed.Find(card) >= 0 && card.OZG_Reads(uid);
            if (!chosen || named.Find(card) >= 0)
                return OZG_Const.SAY_SLEIGHT_NO_CARD;
            if (ofGame.Find(card.OZG_Face()) < 0)
                return OZG_Const.SAY_SLEIGHT_NOT_GAME;
            named.Insert(card);
            stack.PickCards.Insert(card);
        }
        int byChance = 0;
        for (i = 0; i < stack.OutKeys.Count(); i++)
        {
            if (stack.OutKeys[i] == OZG_Const.ANY_CARD)
            {
                byChance++;
                continue;
            }
            card = OZG_Card_Base.Cast(OZG_Keys.Find(stack.OutKeys[i]));
            bool known = card && placed.Find(card) >= 0 && card.OZG_Reads(uid);
            if (!known || named.Find(card) >= 0)
                return OZG_Const.SAY_SLEIGHT_NO_CARD;
            named.Insert(card);
            stack.OutCards.Insert(card);
        }
        // Cards out by chance: any of the place's the stack does not name.
        array<OZG_Card_Base> rest = new array<OZG_Card_Base>();
        for (i = 0; i < placed.Count(); i++)
        {
            card = OZG_Card_Base.Cast(placed[i]);
            if (card && named.Find(card) < 0)
                rest.Insert(card);
        }
        OZG_Random dice = new OZG_Random();
        for (i = 0; i < byChance; i++)
        {
            if (rest.Count() == 0)
                return OZG_Const.SAY_SLEIGHT_NO_CARD;
            int at = dice.Below(rest.Count());
            stack.OutCards.Insert(rest[at]);
            rest.Remove(at);
        }

        // The deal on paper: the place's cards less those coming out, with
        // those going in.
        array<EntityAI> after = new array<EntityAI>();
        for (i = 0; i < placed.Count(); i++)
        {
            card = OZG_Card_Base.Cast(placed[i]);
            if (card && stack.OutCards.Find(card) < 0)
                after.Insert(card);
        }
        for (i = 0; i < stack.InCards.Count(); i++)
            after.Insert(stack.InCards[i]);
        array<ref OZG_DealCard> paperAll = new array<ref OZG_DealCard>();
        array<string> paperKeys = new array<string>();
        OZG_DeckRead.RecordsOf(after, paperAll, paperKeys);
        array<ref OZG_DealCard> paperCards = new array<ref OZG_DealCard>();
        for (i = 0; i < paperAll.Count(); i++)
        {
            if (ofGame.Find(OZG_Cards.FaceOf(paperAll[i].Name)) >= 0)
                paperCards.Insert(paperAll[i]);
        }
        OZG_Game paper = game.Fresh();
        paper.TakeCards(paperCards);
        if (stack.PickCards.Count() > 0)
            paper.DealFirst(mine, NamesOf(stack.PickCards, paperAll, paperKeys));
        string refusal;
        if (!paper.Begin(seats, options, new OZG_Random(), hint, refusal))
            return refusal;
        return "";
    }

    // The items the stack moves (§6.6): the cards taken out into the
    // dealer's pockets (the ground at his feet when they are full), the cards
    // put in into the deck (or the place's own holding).
    protected void StackApply(PlayerBase player, OZG_Stack stack)
    {
        int i;
        for (i = 0; i < stack.OutCards.Count(); i++)
            OZG_DeckRead.ToPlayer(stack.OutCards[i], player);
        for (i = 0; i < stack.InCards.Count(); i++)
        {
            if (!OZG_DeckRead.IntoPlace(m_Place, stack.InCards[i]))
                OZG_Log.Warn("sleight: a card put into the deck of place " + m_R.PlaceId + " found no room");
        }
    }

    // The rolls of the dealer's sleights, after everybody was sent the deal:
    // a card put in is a sleight of its own, a card taken out one, the cards
    // chosen one with a roll a card (§6.5, §7). Whoever saw any of them is
    // shown the deck shaken once, with the sound of the first he saw.
    protected void StackRolls(string uid, int dealer, OZG_Stack stack)
    {
        OZG_Random dice = new OZG_Random();
        map<string, string> shaken = new map<string, string>();
        int i;
        for (i = 0; i < stack.InCards.Count(); i++)
            StackRoll(uid, dealer, OZG_Const.SLEIGHT_DECK_IN, 1, stack.InCards[i].OZG_Face(), "deckin", dice, shaken);
        for (i = 0; i < stack.OutCards.Count(); i++)
            StackRoll(uid, dealer, OZG_Const.SLEIGHT_DECK_OUT, 1, stack.OutCards[i].OZG_Face(), "deckout", dice, shaken);
        if (stack.PickCards.Count() > 0)
            StackRoll(uid, dealer, OZG_Const.SLEIGHT_PICK, stack.PickCards.Count(), OZG_Stack.Faces(stack.PickCards), "pick", dice, shaken);
        array<string> one = new array<string>();
        for (i = 0; i < shaken.Count(); i++)
        {
            one.Clear();
            one.Insert(shaken.GetKey(i));
            ShakeFor(one, OZG_Regions.STOCK, shaken.GetElement(i));
        }
    }

    protected void StackRoll(string uid, int dealer, string kind, int cards, string what, string sound, OZG_Random dice, map<string, string> shaken)
    {
        array<string> seen = new array<string>();
        string said = Rolls(uid, kind, cards, dice, seen);
        SleightLine(uid, dealer, kind, what, said);
        for (int i = 0; i < seen.Count(); i++)
        {
            if (!shaken.Contains(seen[i]))
                shaken.Set(seen[i], sound);
        }
    }

    // ------------------------------------------------------------ sleights in play

    // How many sleights the player `uid` did in this deal.
    int SleightsOf(string uid)
    {
        int at = m_R.SleightUids.Find(uid);
        if (at < 0 || at >= m_R.SleightCounts.Count())
            return 0;
        return m_R.SleightCounts[at];
    }

    protected void CountSleight(string uid, int n)
    {
        int at = m_R.SleightUids.Find(uid);
        if (at < 0 || at >= m_R.SleightCounts.Count())
        {
            m_R.SleightUids.Insert(uid);
            m_R.SleightCounts.Insert(n);
            return;
        }
        m_R.SleightCounts[at] = m_R.SleightCounts[at] + n;
    }

    // What a head says of sleights to its player (card sharp's design §10):
    // in play, how many he did and may, when he was dealt in; between deals,
    // to the dealer -- the host on his own seat -- the same, how many cards he
    // may choose, and the cards of the deck he reads.
    protected void SleightHead(OZG_WView v, string uid, int seat, bool dealt)
    {
        v.SleightsUsed = 0;
        v.SleightsMost = 0;
        v.PicksMost = 0;
        v.DeckReads.Clear();
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || seat < 0 || uid == "")
            return;
        OZG_Game kind = OZG_Games.Sample(m_R.GameId);
        if (!kind || !kind.HasSleights())
            return;
        if (m_R.St == OZG_Const.ST_PLAYING)
        {
            if (!dealt || !m_Game)
                return;
            v.SleightsUsed = SleightsOf(uid);
            v.SleightsMost = cfg.SleightsPerDeal;
            return;
        }
        if (!IsHost(uid) || m_R.Seats[seat].St != OZG_Const.SEAT_TAKEN)
            return;
        v.SleightsMost = cfg.SleightsPerDeal;
        v.PicksMost = cfg.DealerPicks;
        DeckReadsFor(uid, v.DeckReads);
    }

    // The cards of the place the player reads and the game deals: what a
    // dealer may choose for his hand, or take out by name (§6.5). Read is
    // marked by him or studied closely (a reader of the mark); a card he only
    // noticed gives him its sign, never the right to pick it (the owner's
    // word, 2026-11-25), and is not among them.
    protected void DeckReadsFor(string uid, array<ref OZG_WDeckCard> into)
    {
        into.Clear();
        OZG_Game sample = OZG_Games.Sample(m_R.GameId);
        array<string> ofGame = new array<string>();
        if (sample)
            sample.CardsUsed(ofGame);
        array<EntityAI> things = new array<EntityAI>();
        OZG_DeckRead.ThingsOf(m_Place, things);
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card || !card.OZG_Reads(uid))
                continue;
            if (ofGame.Find(card.OZG_Face()) < 0)
                continue;
            into.Insert(OZG_WDeckCard.Make(OZG_Keys.Of(card), card.OZG_Face(), card.OZG_Look(), card.OZG_Sign()));
        }
    }

    // May this player do a sleight in play now (card sharp's design §6.1):
    // sleights on, a seat dealt into the deal in play, present, and sleights
    // left -- unless this is the second card of one throw. "" or the refusal.
    protected string SleightOpen(PlayerBase player, string uid, int mine, bool more)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        if (mine < 0)
            return OZG_Const.SAY_NOT_SEATED;
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game)
            return OZG_Const.NO_OVER;
        if (m_R.Dealt.Find(mine) < 0)
            return OZG_Const.NO_SEAT;
        if (!Present(player))
            return OZG_Const.SAY_TOO_FAR;
        if (!more && SleightsOf(uid) >= cfg.SleightsPerDeal)
            return OZG_Const.SAY_SLEIGHT_NO_MORE;
        return "";
    }

    // slip (§6.2): a card of his pockets joins his hand, the item into the
    // deck in the place.
    protected string Slip(PlayerBase player, string uid, int mine, OZG_WAsk ask)
    {
        string why = SleightOpen(player, uid, mine, false);
        if (why != "")
            return why;
        OZG_Card_Base card = OZG_DeckRead.PocketCard(player, ask.CardKey);
        if (!card)
            return OZG_Const.SAY_SLEIGHT_NO_CARD;
        string face = card.OZG_Face();
        if (!m_Game.SleightTakes(face))
            return OZG_Const.SAY_SLEIGHT_NOT_GAME;
        OZG_DealCard rec = OZG_DealCard.Make(m_Game.SleightName(face), card.OZG_Look());
        card.OZG_MarkInto(rec);
        if (!OZG_DeckRead.IntoPlace(m_Place, card))
            return OZG_Const.SAY_SLEIGHT_NO_ROOM;
        string refusal;
        if (!m_Game.SleightIn(mine, rec, refusal))
        {
            OZG_DeckRead.ToPlayer(card, player);
            return refusal;
        }
        m_ThrowRev.Remove(uid);
        Did(player, uid, mine, OZG_Const.SLEIGHT_SLIP, true, OZG_Regions.OfSeat(mine), "slip", rec.Name, "", face);
        return "";
    }

    // swap (§6.3): a card of his hand to his pockets, a card of his pockets
    // in its place; the count stays.
    protected string Swap(PlayerBase player, string uid, int mine, OZG_WAsk ask)
    {
        string why = SleightOpen(player, uid, mine, false);
        if (why != "")
            return why;
        OZG_Card_Base card = OZG_DeckRead.PocketCard(player, ask.CardKey);
        if (!card)
            return OZG_Const.SAY_SLEIGHT_NO_CARD;
        string face = card.OZG_Face();
        if (!m_Game.SleightTakes(face))
            return OZG_Const.SAY_SLEIGHT_NOT_GAME;
        // A local is never named `out`: the compiler takes it for the
        // keyword (dayz-modding, the language's iron rules).
        string leaving = ask.Move;
        if (!m_Game.SleightHolds(mine, leaving))
            return OZG_Const.NO_CARD;
        // The item of the card going out, found before the one coming in
        // lies beside it.
        OZG_Card_Base gone = OZG_DeckRead.ThingFor(m_Place, m_Game, leaving);
        if (!gone)
        {
            OZG_Log.Warn("sleight: the card " + leaving + " of place " + m_R.PlaceId + " is not in it");
            return OZG_Const.SAY_CANNOT;
        }
        OZG_DealCard rec = OZG_DealCard.Make(m_Game.SleightName(face), card.OZG_Look());
        card.OZG_MarkInto(rec);
        if (!OZG_DeckRead.IntoPlace(m_Place, card))
            return OZG_Const.SAY_SLEIGHT_NO_ROOM;
        string refusal;
        if (!m_Game.SleightSwap(mine, leaving, rec, refusal))
        {
            OZG_DeckRead.ToPlayer(card, player);
            return refusal;
        }
        string where = OZG_DeckRead.ToPlayer(gone, player);
        m_ThrowRev.Remove(uid);
        string what = OZG_Cards.FaceOf(leaving) + " for " + face + " (" + where + ")";
        Did(player, uid, mine, OZG_Const.SLEIGHT_SWAP, true, OZG_Regions.OfSeat(mine), "swap", rec.Name, leaving, what);
        return "";
    }

    // throw (§6.4): a card of his hand to the discards; one throw is up to
    // two cards, the second sent with How "more" straight after the first.
    // The item stays in the deck.
    protected string Throw(PlayerBase player, string uid, int mine, OZG_WAsk ask)
    {
        bool more = ask.How == OZG_Const.HOW_MORE && ThrowGoesOn(uid);
        string why = SleightOpen(player, uid, mine, more);
        if (why != "")
            return why;
        string card = ask.Move;
        if (!m_Game.SleightHolds(mine, card))
            return OZG_Const.NO_CARD;
        string refusal;
        if (!m_Game.SleightDiscard(mine, card, refusal))
            return refusal;
        int thrown = 1;
        if (more)
        {
            m_ThrowCards.Find(uid, thrown);
            thrown++;
        }
        string what = OZG_Cards.FaceOf(card) + " (card " + thrown.ToString() + ")";
        Did(player, uid, mine, OZG_Const.SLEIGHT_THROW, !more, OZG_Regions.DISCARDS, "toss", "", card, what);
        m_ThrowRev.Set(uid, m_R.Rev);
        m_ThrowCards.Set(uid, thrown);
        return "";
    }

    // Is a throw of this player still open for its second card: nothing has
    // changed since its first, and it threw fewer than two.
    protected bool ThrowGoesOn(string uid)
    {
        int rev;
        int cards;
        if (!m_ThrowRev.Find(uid, rev) || !m_ThrowCards.Find(uid, cards))
            return false;
        return rev == m_R.Rev && cards < OZG_Const.THROW_MOST;
    }

    // A sleight in play was done: counted, rolled (§6.1), written to the
    // server's log with the player's id -- never to the game's -- and every
    // view sent; then the shake to those who saw it, and the doer told what
    // his window flies, with its sound.
    protected void Did(PlayerBase player, string uid, int mine, string kind, bool counts, string region, string sound, string cameIn, string wentOut, string what)
    {
        if (counts)
            CountSleight(uid, 1);
        array<string> seen = new array<string>();
        string said = Rolls(uid, kind, 1, new OZG_Random(), seen);
        SleightLine(uid, mine, kind, what, said);
        Settle();
        Changed();
        ShakeFor(seen, region, sound);
        TellDoer(player, kind, cameIn, wentOut, sound);
    }

    // The two rolls of a sleight (§6.1): did it show at all, a roll a card at
    // its kind's percent; and when it did, every other seated player present
    // rolls his own SpotPercent. `seen` gets the ids that saw it; the answer
    // is the rolls, for the server's log.
    protected string Rolls(string uid, string kind, int cards, OZG_Random dice, array<string> seen)
    {
        seen.Clear();
        OZG_Settings cfg = OZG_Cfg.Get();
        int caught = cfg.CaughtOf(kind);
        bool showed = false;
        string rolled = "";
        for (int c = 0; c < cards; c++)
        {
            int r = dice.Below(100);
            if (c > 0)
                rolled = rolled + ",";
            rolled = rolled + r.ToString();
            if (r < caught)
                showed = true;
        }
        string said = "at " + caught.ToString() + "% rolled " + rolled;
        if (!showed)
            return said + ": not shown";
        said = said + ": shown; spot at " + cfg.SpotPercent.ToString() + "%";
        string who = "";
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.St != OZG_Const.SEAT_TAKEN || s.Uid == "" || s.Uid == uid)
                continue;
            int spot = dice.Below(100);
            said = said + " " + s.Uid + "=" + spot.ToString();
            if (spot >= cfg.SpotPercent)
                continue;
            seen.Insert(s.Uid);
            if (who != "")
                who = who + ",";
            who = who + s.Uid;
        }
        if (who == "")
            who = "nobody";
        return said + "; seen by " + who;
    }

    // Every sleight, caught or not, in the server's log with the player's id
    // (§6.1): an admin can always find out who did what.
    protected void SleightLine(string uid, int seat, string kind, string what, string said)
    {
        string line = "sleight: " + uid + " seat " + seat.ToString() + " " + kind + " " + what;
        line = line + ", place " + m_R.PlaceId + ", deal " + m_R.DealNo.ToString();
        OZG_Log.Info(line + ": " + said);
    }

    // The shake (§6.1, §9): to each who saw the sleight, and to him alone.
    protected void ShakeFor(array<string> seen, string region, string sound)
    {
        if (seen.Count() == 0)
            return;
        OZG_WShake w = new OZG_WShake();
        w.PlaceId = m_R.PlaceId;
        w.RegionName = region;
        w.SoundName = sound;
        string body;
        string err;
        JsonFileLoader<OZG_WShake>.MakeData(w, body, err, false);
        for (int i = 0; i < seen.Count(); i++)
            SendTo(OZG_Sessions.PlayerOf(seen[i]), OZG_Const.OP_SHAKE, body);
    }

    // The doer's own word (§9): what came into his hand and went out of it,
    // and the sound his client plays. Never who saw.
    protected void TellDoer(PlayerBase player, string kind, string cameIn, string wentOut, string sound)
    {
        OZG_WSleight w = new OZG_WSleight();
        w.PlaceId = m_R.PlaceId;
        w.Rev = m_R.Rev;
        w.Kind = kind;
        w.CameIn = cameIn;
        w.WentOut = wentOut;
        w.SoundName = sound;
        string body;
        string err;
        JsonFileLoader<OZG_WSleight>.MakeData(w, body, err, false);
        SendTo(player, OZG_Const.OP_SLEIGHT, body);
    }

    protected string Act(PlayerBase player, string uid, int mine, string move)
    {
        if (mine < 0)
            return OZG_Const.SAY_NOT_SEATED;
        if (m_R.St != OZG_Const.ST_PLAYING || !m_Game)
            return OZG_Const.NO_OVER;
        if (m_R.Dealt.Find(mine) < 0)
            return OZG_Const.NO_SEAT;
        if (!Present(player))
            return OZG_Const.SAY_TOO_FAR;

        // More than ten moves a second from one player are dropped.
        int now = OZG_Sessions.Now();
        if (now != m_MoveSecond)
        {
            m_MoveSecond = now;
            m_Moves.Clear();
        }
        int made = 0;
        m_Moves.Find(uid, made);
        made++;
        m_Moves.Set(uid, made);
        if (made > 10)
            return OZG_Const.SAY_SLOW;

        string refusal;
        if (!m_Game.Act(mine, move, refusal))
            return refusal;

        AddLog(mine.ToString() + " " + move);
        Settle();
        Changed();
        return "";
    }

    // ------------------------------------------------------------ noticing

    // The deals that counted towards noticing for a player so far.
    int NoticeDeals(string uid)
    {
        int n = 0;
        m_NoticeDeals.Find(uid, n);
        return n;
    }

    // After a deal (design §5): each player dealt in counts it when one of
    // the deal's cards is marked and he neither reads nor notices it, then
    // rolls the chance the settings give his count; on a success one such
    // card, picked at random, gets him as a noticer. Nothing is said to him
    // and nothing goes into the game's log.
    protected void NoticeAfterDeal(array<int> seats, array<ref OZG_DealCard> cards, OZG_Random dice)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || !m_Game)
            return;
        array<int> unseen = new array<int>();
        int i;
        int k;
        for (i = 0; i < seats.Count(); i++)
        {
            int no = seats[i];
            if (no < 0 || no >= m_R.Seats.Count())
                continue;
            string uid = m_R.Seats[no].Uid;
            if (uid == "")
                continue;
            unseen.Clear();
            for (k = 0; k < cards.Count(); k++)
            {
                OZG_DealCard c = cards[k];
                if (!c || c.Sign <= 0)
                    continue;
                if (c.ReadBy(uid) || c.NoticedBy(uid))
                    continue;
                unseen.Insert(k);
            }
            if (unseen.Count() == 0)
                continue;
            int deals = NoticeDeals(uid) + 1;
            m_NoticeDeals.Set(uid, deals);
            int chance = cfg.NoticeChance(deals);
            int rolled = dice.Below(100);
            string roll = "notice roll: " + uid + " deal " + deals.ToString();
            roll = roll + " at " + chance.ToString() + "% rolled " + rolled.ToString();
            OZG_Log.Dbg(roll);
            if (rolled >= chance)
                continue;
            OZG_DealCard picked = cards[unseen[dice.Below(unseen.Count())]];
            NoticeOne(uid, picked, deals, chance);
        }
    }

    // The player `uid` notices `picked`: on the card item, which keeps him
    // through restarts and games (§4.1), and on the game's record of it, so
    // that his view shows it at once. The server's log says so, with his id,
    // for the admin.
    protected void NoticeOne(string uid, OZG_DealCard picked, int deals, int chance)
    {
        OZG_Card_Base thing = OZG_DeckRead.CardNamed(m_Place, picked.Name);
        if (thing)
            thing.OZG_AddNoticer(uid);
        else
            OZG_Log.Warn("notice: the card " + picked.Name + " of place " + m_R.PlaceId + " is not in it");
        m_Game.AddNoticer(picked.Name, uid);
        string line = "notice: " + uid + " noticed " + OZG_Cards.FaceOf(picked.Name);
        line = line + ", sign " + picked.Sign.ToString() + ", deal " + deals.ToString();
        line = line + " at " + chance.ToString() + "%, place " + m_R.PlaceId;
        OZG_Log.Info(line);
    }

    // ------------------------------------------------------------ cards in hands

    // A present seated player who has cards and whose hands are empty holds a
    // fan of them, backs outwards. It is never in the way: it leaves with
    // whatever else the player takes into his hands, and comes back when they
    // are empty again. A game goes on without it.
    protected void Fans()
    {
        bool playing = false;
        if (m_Game && m_R.St == OZG_Const.ST_PLAYING)
            playing = true;
        int deckNo = OZG_Decks.NumberOf(m_R.DeckClass);
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.Uid == "")
                continue;
            PlayerBase p = OZG_Sessions.PlayerOf(s.Uid);
            if (!p || !p.IsAlive())
                continue;

            int want = 0;
            if (playing && s.St == OZG_Const.SEAT_TAKEN && m_R.Dealt.Find(i) >= 0)
                want = m_Game.HeldCount(i);
            if (want > 8)
                want = 8;

            EntityAI held = p.GetItemInHands();
            OZG_Hand fan = OZG_Hand.Cast(held);
            if (want > 0)
            {
                if (!fan && !held)
                    fan = OZG_Hand.Cast(p.GetHumanInventory().CreateInHands("OZG_Hand"));
                if (fan)
                    fan.OZG_Set(want, deckNo);
            }
            else if (fan)
            {
                GetGame().ObjectDelete(fan);
            }
        }
    }

    // A player who no longer sits here holds no fan of this game.
    protected void DropFan(string uid)
    {
        PlayerBase p = OZG_Sessions.PlayerOf(uid);
        if (!p)
            return;
        OZG_Hand fan = OZG_Hand.Cast(p.GetItemInHands());
        if (fan)
            GetGame().ObjectDelete(fan);
    }

    // The place is going: every fan of it goes too.
    void DropFans()
    {
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            if (m_R.Seats[i].Uid != "")
                DropFan(m_R.Seats[i].Uid);
        }
    }

    // ------------------------------------------------------------ the clock

    // Once a second.
    void Tick()
    {
        if (!m_Place)
            return;
        // A place the deck has not reached in time is no game.
        if (m_DeckDue > 0)
        {
            if (m_Place.OZG_Deck())
            {
                m_DeckDue = 0;
            }
            else if (OZG_Sessions.Now() >= m_DeckDue)
            {
                OZG_Sessions.Unlay(this, OZG_Sessions.PlayerOf(m_DeckUid));
                return;
            }
        }
        OZG_Settings cfg = OZG_Cfg.Get();
        bool changed = false;
        bool anyone = false;
        int i;

        // Taken and kept turn into each other as the player comes and goes.
        for (i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (s.Uid == "")
                continue;
            bool here = Present(OZG_Sessions.PlayerOf(s.Uid));
            if (here)
                anyone = true;
            if (here && s.St == OZG_Const.SEAT_KEPT)
            {
                s.St = OZG_Const.SEAT_TAKEN;
                changed = true;
            }
            else if (!here && s.St == OZG_Const.SEAT_TAKEN)
            {
                s.St = OZG_Const.SEAT_KEPT;
                changed = true;
            }
        }

        // The host's absence, in seconds in a row.
        if (m_R.HostUid != "" && Present(OZG_Sessions.PlayerOf(m_R.HostUid)))
            m_HostAway = 0;
        else
            m_HostAway++;
        bool free = HostFree();
        if (free != m_HostWasFree)
        {
            m_HostWasFree = free;
            changed = true;
        }

        // A watcher who walked off.
        for (i = m_Watchers.Count() - 1; i >= 0; i--)
        {
            PlayerBase w = OZG_Sessions.PlayerOf(m_Watchers[i]);
            bool inRange = w && w.IsAlive();
            if (inRange)
                inRange = vector.Distance(w.GetPosition(), m_Place.GetPosition()) <= cfg.WatchRadius;
            if (!inRange)
            {
                CloseFor(m_Watchers[i], OZG_Const.SAY_TOO_FAR);
                m_Watchers.Remove(i);
            }
        }

        if (changed)
        {
            Settle();
            Changed();
        }
        else if (Settle() > 0)
        {
            Changed();
        }

        Fans();

        // Idle: nobody seated has been here for a long while.
        if (anyone)
            m_Idle = 0;
        else
            m_Idle++;
        if (cfg.IdleMinutes > 0 && m_Idle >= cfg.IdleMinutes * 60)
            OZG_Sessions.Fold(this, null, OZG_Const.SAY_IDLE);
    }

    // For the stand: one line.
    string Describe()
    {
        string line = "session " + m_R.PlaceId + " st=" + m_R.St.ToString();
        line = line + " rev=" + m_R.Rev.ToString() + " host=" + SeatOf(m_R.HostUid).ToString();
        line = line + " seats=";
        for (int i = 0; i < m_R.Seats.Count(); i++)
        {
            OZG_SeatRec s = m_R.Seats[i];
            if (i > 0)
                line = line + ",";
            line = line + i.ToString() + ":" + s.St.ToString();
        }
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY)
            line = line + " | " + m_Game.Brief();
        return line;
    }
}
