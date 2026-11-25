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

    void OZG_SessionRec()
    {
        Version = OZG_Const.FILE_VERSION;
        OptIds = new array<string>();
        OptVals = new array<string>();
        Seats = new array<ref OZG_SeatRec>();
        Dealt = new array<int>();
        GameParts = new array<string>();
        Log = new array<string>();
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

    void OZG_Session(OZG_Place place)
    {
        m_Place = place;
        m_R = new OZG_SessionRec();
        m_Watchers = new array<string>();
        m_Asked = new map<string, int>();
        m_Moves = new map<string, int>();
        m_NoticeDeals = new map<string, int>();
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
            if (OZG_Games.Sample(cfg.GameIds[i]))
            {
                m_R.GameId = cfg.GameIds[i];
                break;
            }
        }
        DefaultOptions();
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

        OZG_Game game;
        if (rec.St != OZG_Const.ST_LOBBY)
        {
            game = OZG_Games.Create(rec.GameId);
            string state = OZG_Text.Glue(rec.GameParts);
            string why;
            bool good = game && game.LoadState(state);
            if (good)
                good = game.Sound(why);
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
            v.Seats.Insert(w);
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
        // What this player reads: only a player dealt into the game, and only
        // into the head made for that player (design §4.6).
        v.Reads.Clear();
        bool dealt = seat >= 0 && m_R.Dealt.Find(seat) >= 0;
        bool reading = OZG_Cfg.Get().Cheating == 1;
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY && dealt && reading)
            m_Game.ReadsFor(seat, uid, v.Reads);

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
        m_R.Rev = m_R.Rev + 1;
        Save();
        Pulse();
        Push();
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

        bool takesOver = s.St == OZG_Const.SEAT_LEFT;
        s.St = OZG_Const.SEAT_TAKEN;
        s.Uid = uid;
        s.Name = OZG_Who.Name(player);
        s.PoseNo = 0;
        PutOn(player, s);
        m_Watchers.RemoveItem(uid);
        if (m_R.HostUid == "")
            SetHost(player);
        if (takesOver)
            AddLog("over " + no.ToString());
        else
            AddLog("sit " + no.ToString());
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
            AddLog("host " + s.No.ToString());
            return;
        }
    }

    // A seat loses its player: by his own word, or the host's.
    protected bool Vacate(int no, string how, out string why)
    {
        why = "";
        OZG_SeatRec s = m_R.Seats[no];
        bool playing = InGame(no);
        if (playing)
        {
            if (how == OZG_Const.HOW_DISCARD)
            {
                m_Game.DropSeat(no);
                s.St = OZG_Const.SEAT_FREE;
                AddLog("drop " + no.ToString());
            }
            else if (how == OZG_Const.HOW_LEAVE)
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

        // The rest is the host's.
        if (!IsHost(uid))
            return OZG_Const.SAY_NOT_HOST;

        if (op == OZG_Const.OP_OPT)
            return Opt(ask.OptId, ask.OptValue);

        if (op == OZG_Const.OP_DEAL)
            return Deal();

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

    protected string Deal()
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
        array<ref OZG_DealCard> all = new array<ref OZG_DealCard>();
        OZG_DeckRead.CardsOf(m_Place, all);
        array<string> ofGame = new array<string>();
        game.CardsUsed(ofGame);
        array<ref OZG_DealCard> cards = new array<ref OZG_DealCard>();
        for (i = 0; i < all.Count(); i++)
        {
            if (ofGame.Find(OZG_Cards.FaceOf(all[i].Name)) >= 0)
                cards.Insert(all[i]);
        }
        game.TakeCards(cards);

        string refusal;
        OZG_Random chance = new OZG_Random();
        if (!game.Begin(seats, options, chance, hint, refusal))
            return refusal;

        m_Game = game;
        m_R.St = OZG_Const.ST_PLAYING;
        m_R.Dealt.Copy(seats);
        m_R.Hint = "";
        m_R.HintUid = "";
        AddLog("deal");
        Settle();
        // Noticing (design §5): after the deal and before anybody is sent it,
        // so that a card noticed now is in the first view of this deal.
        NoticeAfterDeal(seats, cards, chance);
        Changed();
        OZG_Log.Dbg("session " + m_R.PlaceId + " deal: " + m_Game.Brief());
        return "";
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
