// Who a player is to the mod, and what the placer has chosen while aiming.

// A player's id and name. A stand has characters the server made, with no
// identity behind them: the stand's own mod names them here, and nothing else
// ever calls Fake.
class OZG_Who
{
    protected static ref array<PlayerBase> s_Fakes = new array<PlayerBase>();
    protected static ref array<string> s_FakeUids = new array<string>();
    protected static ref array<string> s_FakeNames = new array<string>();

    static void Fake(PlayerBase body, string uid, string name)
    {
        if (!body || uid == "")
            return;
        int at = s_FakeUids.Find(uid);
        if (at >= 0)
        {
            s_Fakes[at] = body;
            s_FakeNames[at] = name;
            return;
        }
        s_Fakes.Insert(body);
        s_FakeUids.Insert(uid);
        s_FakeNames.Insert(name);
    }

    static void Unfake(string uid)
    {
        int at = s_FakeUids.Find(uid);
        if (at < 0)
            return;
        s_Fakes.Remove(at);
        s_FakeUids.Remove(at);
        s_FakeNames.Remove(at);
    }

    static void Reset()
    {
        s_Fakes.Clear();
        s_FakeUids.Clear();
        s_FakeNames.Clear();
    }

    static string Uid(PlayerBase p)
    {
        if (!p)
            return "";
        PlayerIdentity id = p.GetIdentity();
        if (id)
            return id.GetPlainId();
        int at = s_Fakes.Find(p);
        if (at >= 0)
            return s_FakeUids[at];
        return "";
    }

    static string Name(PlayerBase p)
    {
        if (!p)
            return "";
        PlayerIdentity id = p.GetIdentity();
        if (id)
            return id.GetName();
        int at = s_Fakes.Find(p);
        if (at >= 0)
            return s_FakeNames[at];
        return "";
    }

    // Everybody in the world by id: the connected players and the stand's
    // characters.
    static void Everyone(map<string, PlayerBase> into)
    {
        into.Clear();
        array<Man> men = new array<Man>();
        GetGame().GetPlayers(men);
        for (int i = 0; i < men.Count(); i++)
        {
            PlayerBase p = PlayerBase.Cast(men[i]);
            string uid = Uid(p);
            if (uid != "")
                into.Set(uid, p);
        }
        for (int f = 0; f < s_Fakes.Count(); f++)
        {
            if (s_Fakes[f])
                into.Set(s_FakeUids[f], s_Fakes[f]);
        }
    }
}

// What the placer sees and chooses while a deck's hologram is out.
//
// The client's half is written by the hologram every time it judges a spot and
// read by the mission's small panel; the mode is the placer's own choice and
// stays until he changes it. The server's half is the mode each placing
// player's deploy carried.
class OZG_Aim
{
    // Client.
    static int    s_Mode;       // OZG_Const.MODE_*
    static int    s_Stamp;      // when the hologram last judged, ms; 0 never
    static bool   s_Refused;
    static string s_Why;        // a string table key
    static int    s_Seats;
    static int    s_Layout;     // OZG_Const.MODE_STANDING, _SITTING or _FLOOR
    static bool   s_Guessed;

    // Server.
    protected static ref map<string, int> s_ServerMode = new map<string, int>();

    // The key's round: the mod's guess, standing, sitting, on the floor. Not
    // the order of the numbers: those are what was saved before there were
    // three.
    static void NextMode()
    {
        if (s_Mode == OZG_Const.MODE_AUTO)
            s_Mode = OZG_Const.MODE_STANDING;
        else if (s_Mode == OZG_Const.MODE_STANDING)
            s_Mode = OZG_Const.MODE_SITTING;
        else if (s_Mode == OZG_Const.MODE_SITTING)
            s_Mode = OZG_Const.MODE_FLOOR;
        else
            s_Mode = OZG_Const.MODE_AUTO;
    }

    // Is a hologram of ours being judged right now.
    static bool Live()
    {
        if (s_Stamp == 0)
            return false;
        return GetGame().GetTime() - s_Stamp < 400;
    }

    static void ServerSetMode(PlayerBase player, int mode)
    {
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return;
        if (mode < OZG_Const.MODE_AUTO || mode > OZG_Const.MODE_LAST)
            mode = OZG_Const.MODE_AUTO;
        s_ServerMode.Set(uid, mode);
    }

    static int ServerMode(PlayerBase player)
    {
        int mode;
        if (s_ServerMode.Find(OZG_Who.Uid(player), mode))
            return mode;
        return OZG_Const.MODE_AUTO;
    }

    static void Reset()
    {
        s_ServerMode.Clear();
        s_Stamp = 0;
    }
}
