// The sounds of the table (design §17) on a client. The server plays nothing.
// A place carries a number (OZG_Place.OZG_Hear) that tells every client near
// it what to play; a client plays a sound meant for its player alone through
// PlayLocal, the way a sleight is heard only by the one who does it and by
// those who saw it (card sharp's design §6.1, §9). The sounds, their sets and
// their lengths are written by tools/make_sounds.py (OZG_SoundIds.c,
// config.cpp).
class OZG_Sounds
{
    // How long a number carries a lot of sounds before it goes quiet, ms: a
    // client that sees the place later reads the count and no sounds, and so
    // never plays what was played before it came.
    static const int QUIET_MS = 1000;

    // One number carries up to SLOTS sounds and a count: five bits a sound,
    // the first played in the lowest bits, the count above them. The count
    // wraps at COUNT_ROOM, before count * COUNT_UNIT would pass 2^31.
    static const int SLOTS = 3;
    static const int ID_ROOM = 32;

    static const int COUNT_UNIT = 32768;
    static const int COUNT_ROOM = 65536;

    // A window that plays a thing's throws itself, in its own time. The dice tables'
    // window shows a throw a moment after the server made it (the dice gathered, the can
    // shaken and tipped) and sounds it with what it shows: the can rattles as it is shaken,
    // the dice land as they fly out, the game is over when its last throw has been seen.
    // So on that client the thing's lots leave out the throw (and the game over, when the
    // window asks) and leave them here for the window. The window says so every frame
    // (TakeOn): one gone without a word takes nothing more than TAKE_MS after.
    static const int TAKE_MS = 1000;
    static Object s_TakenOn;
    static bool s_TakenOver;
    static int s_TakenAt;
    static ref array<int> s_Taken;

    protected static ref array<string> s_Names;
    protected static ref array<string> s_Sets;
    protected static ref array<int> s_Lengths;

    // The window of `thing` plays its throws itself (`over`: the game over's sound too).
    static void TakeOn(Object thing, bool over)
    {
        if (!s_Taken)
            s_Taken = new array<int>();
        int now = GetGame().GetTime();
        // another thing's, or left by a window long gone: none of it is played now
        if (thing != s_TakenOn || now - s_TakenAt > TAKE_MS)
            s_Taken.Clear();
        s_TakenOn = thing;
        s_TakenOver = over;
        s_TakenAt = now;
    }

    // Is this sound of a lot of `thing` the window's to play?
    static bool Takes(Object thing, int id)
    {
        if (!thing || thing != s_TakenOn || !s_Taken)
            return false;
        int age = GetGame().GetTime() - s_TakenAt;
        if (age < 0 || age > TAKE_MS)
            return false;
        if (id == OZG_SoundIds.ROLL)
            return true;
        return id == OZG_SoundIds.OVER && s_TakenOver;
    }

    protected static void LoadList()
    {
        s_Names = new array<string>();
        s_Sets = new array<string>();
        s_Lengths = new array<int>();
        OZG_SoundIds.Fill(s_Names, s_Sets, s_Lengths);
    }

    // The id of a sound by its name ("lay"); 0 for a name no sound has.
    static int IdOf(string name)
    {
        if (!s_Names)
            LoadList();
        if (name == "")
            return 0;
        int at = s_Names.Find(name);
        if (at < 0)
            return 0;
        return at;
    }

    static string NameOf(int id)
    {
        if (!s_Names)
            LoadList();
        if (id <= 0 || id >= s_Names.Count())
            return "";
        return s_Names[id];
    }

    // The sound set config.cpp names for it; "" for no sound.
    static string SetOf(int id)
    {
        if (!s_Sets)
            LoadList();
        if (id <= 0 || id >= s_Sets.Count())
            return "";
        return s_Sets[id];
    }

    // How long it sounds, ms.
    static int LengthOf(int id)
    {
        if (!s_Lengths)
            LoadList();
        if (id <= 0 || id >= s_Lengths.Count())
            return 0;
        return s_Lengths[id];
    }

    // "shuffle,deal,trump": the names of a lot, for the logs and the stand;
    // "-" for none.
    static string NamesOf(array<int> ids)
    {
        if (!ids || ids.Count() == 0)
            return "-";
        string line = "";
        for (int i = 0; i < ids.Count(); i++)
        {
            if (i > 0)
                line = line + ",";
            line = line + NameOf(ids[i]);
        }
        return line;
    }

    // ------------------------------------------------------------ the number a thing carries

    // The number after `was` that carries `ids` (the first SLOTS of them, the
    // first played first) and the next count.
    static int PackLot(int was, array<int> ids)
    {
        int count = CountOf(was) + 1;
        if (count >= COUNT_ROOM)
            count = 0;
        int sounds = 0;
        int unit = 1;
        int n = ids.Count();
        if (n > SLOTS)
            n = SLOTS;
        for (int i = 0; i < n; i++)
        {
            int id = ids[i];
            if (id < 0 || id >= ID_ROOM)
                id = 0;
            sounds = sounds + id * unit;
            unit = unit * ID_ROOM;
        }
        return count * COUNT_UNIT + sounds;
    }

    // The same count with no sounds: what the number says once it is quiet.
    static int QuietOf(int packed)
    {
        return CountOf(packed) * COUNT_UNIT;
    }

    static int CountOf(int packed)
    {
        if (packed < 0)
            return 0;
        return packed / COUNT_UNIT;
    }

    // The sounds a number carries, the first played first.
    static void IdsOf(int packed, array<int> into)
    {
        into.Clear();
        if (packed < 0)
            return;
        int rest = packed % COUNT_UNIT;
        for (int i = 0; i < SLOTS; i++)
        {
            int id = rest % ID_ROOM;
            rest = rest / ID_ROOM;
            if (id > 0)
                into.Insert(id);
        }
    }

    // ------------------------------------------------------------ playing, on this client

    // The sound `name` played on `where` on this client and on no other: on
    // the place for a sound of the table; on this client's own player when
    // `where` is null. False when there is no such sound or nothing to play it
    // on. The card sharp's fourth phase plays a sleight's sound with this.
    static bool PlayLocal(string name, Object where)
    {
        int id = IdOf(name);
        if (id == 0)
        {
            OZG_Log.Dbg("sound: no sound named '" + name + "'");
            return false;
        }
        return PlayIdLocal(id, where, "local");
    }

    // The same by id; `why` names in the log what played it. Every sound
    // played is named in the log at debug level, with what SoundPlay said.
    static bool PlayIdLocal(int id, Object where, string why)
    {
        if (!GetGame() || GetGame().IsDedicatedServer())
            return false;
        string soundSet = SetOf(id);
        if (soundSet == "")
            return false;
        Object thing = where;
        if (!thing)
            thing = GetGame().GetPlayer();
        if (!thing)
            return false;
        // SEffectManager.PlaySoundOnObject, step by step (effectmanager.c:256),
        // so that what SoundPlay answers reaches the log.
        EffectSound sound = SEffectManager.CreateSound(soundSet, thing.GetPosition());
        sound.SetParent(thing);
        sound.SetLocalPosition(vector.Zero);
        bool started = sound.SoundPlay();
        sound.SetAutodestroy(true);
        string said = "0";
        if (started)
            said = "1";
        string line = "sound " + NameOf(id) + " on " + why;
        OZG_Log.Dbg(line + ": started=" + said);
        return started;
    }
}

// Sounds played one after another on one thing, on this client: a lot of a
// place's number. Each waits for the one before it to end
// (OZG_Sounds.LengthOf); a lot that comes while another plays does not wait
// for it, the two sound together.
class OZG_SoundRun
{
    protected Object m_Thing;
    protected string m_Why;
    // Sounds still to play and when, in this client's mission time (ms).
    protected ref array<int> m_Ids;
    protected ref array<int> m_At;

    void OZG_SoundRun(Object thing, string why)
    {
        m_Thing = thing;
        m_Why = why;
        m_Ids = new array<int>();
        m_At = new array<int>();
    }

    void ~OZG_SoundRun()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(PlayDue);
    }

    // `ids` from now on, each when the one before it ends.
    void AddLot(array<int> ids)
    {
        int at = GetGame().GetTime();
        for (int i = 0; i < ids.Count(); i++)
        {
            m_Ids.Insert(ids[i]);
            m_At.Insert(at);
            at = at + OZG_Sounds.LengthOf(ids[i]);
        }
        PlayDue();
    }

    // Plays what is due, and waits for the next.
    protected void PlayDue()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(PlayDue);
        int now = GetGame().GetTime();
        int soonest = -1;
        int i = 0;
        while (i < m_Ids.Count())
        {
            if (m_At[i] <= now)
            {
                if (m_Thing)
                    OZG_Sounds.PlayIdLocal(m_Ids[i], m_Thing, m_Why);
                m_Ids.RemoveOrdered(i);
                m_At.RemoveOrdered(i);
                continue;
            }
            if (soonest < 0 || m_At[i] < soonest)
                soonest = m_At[i];
            i++;
        }
        if (soonest >= 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PlayDue, soonest - now, false);
    }
}

// What one thing of the world does with its sound number. On the server: the
// next number for a lot of sounds, and the lot remembered for the stand. On a
// client: a new lot played once, as it comes; a number seen for the first
// time with no sounds in it only noted, so that a lot played before the client
// saw the thing is never played (OZG_Sounds.QUIET_MS).
class OZG_SoundPulse
{
    protected int m_Heard;
    protected string m_Last;
    protected ref OZG_SoundRun m_Run;

    void OZG_SoundPulse()
    {
        m_Heard = -1;
        m_Last = "-";
    }

    // Server: the number after `packed` that carries `ids`.
    int NextNumber(int packed, array<int> ids)
    {
        array<int> kept = new array<int>();
        for (int i = 0; i < ids.Count() && i < OZG_Sounds.SLOTS; i++)
            kept.Insert(ids[i]);
        m_Last = OZG_Sounds.NamesOf(kept);
        return OZG_Sounds.PackLot(packed, kept);
    }

    string LastLot()
    {
        return m_Last;
    }

    // Client: the number as it came, played on `thing`.
    void HeardNumber(int packed, Object thing, string why)
    {
        int count = OZG_Sounds.CountOf(packed);
        if (count == m_Heard)
            return;
        array<int> ids = new array<int>();
        OZG_Sounds.IdsOf(packed, ids);
        // When a client first reads the number: at the thing's coming into its
        // world, or at the first change after (the record says which).
        if (m_Heard < 0)
        {
            string first = why + ": first sound number, count " + count.ToString();
            OZG_Log.Dbg(first + ", sounds " + OZG_Sounds.NamesOf(ids));
        }
        m_Heard = count;
        if (ids.Count() == 0)
            return;
        // what a window on this client plays itself is left to it (OZG_Sounds.TakeOn)
        array<int> plays = new array<int>();
        for (int i = 0; i < ids.Count(); i++)
        {
            int id = ids[i];
            if (OZG_Sounds.Takes(thing, id))
            {
                OZG_Sounds.s_Taken.Insert(id);
                continue;
            }
            plays.Insert(id);
        }
        if (plays.Count() == 0)
            return;
        if (!m_Run)
            m_Run = new OZG_SoundRun(thing, why);
        m_Run.AddLot(plays);
    }
}
