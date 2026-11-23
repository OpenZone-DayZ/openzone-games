// Table Games Probe -- the live hologram against every piece of furniture.
//
// Each config class of a name prefix is stood on the stand's flat spot. If
// the hologram's ray finds a top on it, the first player is put at a place
// beside it, the client's hologram is aimed at the top, and the game's own
// verdict -- which check refused, if any -- is written down.

class TGP_Sweep
{
    static const string FILE_ROWS = "holo_sweep.csv";

    static bool s_On;
    static int s_At;
    static int s_Asked;

    protected static ref array<string> s_Classes = new array<string>;
    protected static ref array<string> s_Rows = new array<string>;
    protected static Object s_Obj;
    protected static int s_Phase;       // 0 make, 1 find the top, 2 ask the client, 3 wait for its line
    protected static float s_Until;
    protected static int s_ToldAt;
    protected static string s_Head;
    protected static float s_SpotX;
    protected static float s_SpotZ;
    protected static float s_Ground;

    // Scratch of TopOf: the cells the ray met in range.
    protected static ref array<float> s_Xs = new array<float>;
    protected static ref array<float> s_Zs = new array<float>;
    protected static ref array<float> s_Hs = new array<float>;
    protected static ref array<int> s_Keys = new array<int>;

    static int SweepCount()
    {
        return s_Classes.Count();
    }

    static int SweepBegin(string prefix, int limit, float spotX, float spotZ)
    {
        s_Classes.Clear();
        s_Rows.Clear();

        int n = GetGame().ConfigGetChildrenCount("CfgVehicles");
        string name;
        bool room = true;

        for (int i = 0; i < n && room; i++)
        {
            GetGame().ConfigGetChildName("CfgVehicles", i, name);
            if (name.IndexOf(prefix) == 0)
            {
                s_Classes.Insert(name);
                if (limit > 0 && s_Classes.Count() >= limit)
                    room = false;
            }
        }

        TGP_Out.Fresh(FILE_ROWS, "label;class;ax;ay;az;rise;cells;places;r;line");

        s_SpotX = spotX;
        s_SpotZ = spotZ;
        s_At = 0;
        s_Asked = 0;
        s_Phase = 0;
        s_Obj = null;

        // Once, before anything stands there. Measured again for each thing
        // it came out 0.4 m high after a sofa or a bed: a deleted object is
        // still in the world until the frame ends, and those are walkable on
        // top (run of 2026-11-23, nine classes lost that way).
        float terrain = GetGame().SurfaceY(spotX, spotZ);
        s_Ground = GetGame().SurfaceRoadY3D(spotX, terrain + 3, spotZ, RoadSurfaceDetection.UNDER);

        int found = s_Classes.Count();
        bool any = found > 0;
        s_On = any;
        return found;
    }

    protected static PlayerBase FirstPlayer()
    {
        array<Man> players = new array<Man>;
        GetGame().GetPlayers(players);
        if (players.Count() == 0)
            return null;
        return PlayerBase.Cast(players.Get(0));
    }

    protected static void Ask(PlayerBase player, string what, string text)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(what);
        rpc.Write(text);
        rpc.Send(player, TGP_Rpc.ASK, true, player.GetIdentity());
    }

    // The largest flat patch the hologram's ray meets on the thing, 0.35 to
    // 1.25 m up and with room above; `aim` is its cell nearest the middle.
    protected static bool TopOf(Object obj, float ground, out vector aim, out int cells)
    {
        vector mm[2];
        obj.ClippingInfo(mm);
        vector pos = obj.GetPosition();

        float x0 = pos[0] + mm[0][0];
        float x1 = pos[0] + mm[1][0];
        float z0 = pos[2] + mm[0][2];
        float z1 = pos[2] + mm[1][2];
        if (x1 - x0 > 6)
        {
            x0 = pos[0] - 3;
            x1 = pos[0] + 3;
        }
        if (z1 - z0 > 6)
        {
            z0 = pos[2] - 3;
            z1 = pos[2] + 3;
        }

        // Well above the thing: a table's top is its highest surface, and the
        // room over it is measured from where the ray starts.
        float top = pos[1] + mm[1][1] + 1.5;
        if (top > ground + 9)
            top = ground + 9;

        s_Xs.Clear();
        s_Zs.Clear();
        s_Hs.Clear();
        s_Keys.Clear();

        map<int, int> counts = new map<int, int>;
        float x;
        float z;
        float y;
        float rise;
        int n;
        int i;
        int key;
        int have;

        for (x = x0; x <= x1; x = x + 0.1)
        {
            for (z = z0; z <= z1; z = z + 0.1)
            {
                TGP_Scan.ColumnDown(x, z, top, ground - 0.2, TGP_Scan.KIND_FIRE, null);
                n = TGP_Scan.s_Ys.Count();
                for (i = 0; i < n; i++)
                {
                    y = TGP_Scan.s_Ys.Get(i);
                    rise = y - ground;
                    if (rise >= 0.35 && rise <= 1.25 && TGP_Scan.s_Gaps.Get(i) >= 0.6)
                    {
                        key = Math.Round(y * 25);
                        s_Xs.Insert(x);
                        s_Zs.Insert(z);
                        s_Hs.Insert(y);
                        s_Keys.Insert(key);

                        have = 0;
                        counts.Find(key, have);
                        counts.Set(key, have + 1);
                    }
                }
            }
        }

        int bestKey = 0;
        cells = 0;
        for (i = 0; i < counts.Count(); i++)
        {
            if (counts.GetElement(i) > cells)
            {
                cells = counts.GetElement(i);
                bestKey = counts.GetKey(i);
            }
        }

        // 0.12 m2 at least: room for a mat.
        if (cells < 12)
            return false;

        float sx = 0;
        float sz = 0;
        int total = s_Keys.Count();
        for (i = 0; i < total; i++)
        {
            if (s_Keys.Get(i) == bestKey)
            {
                sx = sx + s_Xs.Get(i);
                sz = sz + s_Zs.Get(i);
            }
        }
        sx = sx / cells;
        sz = sz / cells;

        float bestD = 99999;
        float d;
        float dx;
        float dz;
        for (i = 0; i < total; i++)
        {
            if (s_Keys.Get(i) == bestKey)
            {
                dx = s_Xs.Get(i) - sx;
                dz = s_Zs.Get(i) - sz;
                d = dx * dx + dz * dz;
                if (d < bestD)
                {
                    bestD = d;
                    aim = Vector(s_Xs.Get(i), s_Hs.Get(i), s_Zs.Get(i));
                }
            }
        }

        return true;
    }

    // A place to stand beside the top. The hologram wants its point 1 to 2 m
    // from the feet, so of the ring's places the one whose distance comes
    // nearest to 1.4 m.
    protected static bool PlaceFor(vector aim, float rise, out vector best, out float bestR, out int places)
    {
        // The ring, for the count only: its places are the NEAREST free ones,
        // often closer than the hologram's own minimum.
        TGP_Seat.SeatRun(aim, "sweep", 1);
        places = TGP_Seat.s_Places.Count();
        bestR = 0;

        float floorY = aim[1] - rise;
        float want = 1.96 - rise * rise;
        float r0 = 0.9;
        if (want > 0.81)
            r0 = Math.Sqrt(want);

        vector ext = "0.5 1.7 0.5";
        vector to = Vector(aim[0], aim[1] + 0.1, aim[2]);
        float ang;
        float r;
        float px;
        float pz;
        float fy;

        // The wanted distance first, then a little further and a little nearer.
        for (int pass = 0; pass < 3; pass++)
        {
            r = r0;
            if (pass == 1)
                r = r0 + 0.2;
            if (pass == 2)
                r = r0 - 0.2;

            for (int k = 0; k < 16; k++)
            {
                ang = k * 22.5;
                px = aim[0] + Math.Sin(ang * Math.DEG2RAD) * r;
                pz = aim[2] + Math.Cos(ang * Math.DEG2RAD) * r;
                if (TGP_Seat.SpotOk(px, pz, floorY, ang, ext, 1.55, to, fy))
                {
                    best = Vector(px, fy, pz);
                    bestR = r;
                    return true;
                }
            }
        }

        return false;
    }

    // One thing stood on the spot and left there, for a look by hand.
    protected static Object s_Shown;

    static string ShowMake(string cls, float spotX, float spotZ)
    {
        if (s_Shown)
            GetGame().ObjectDelete(s_Shown);
        s_Shown = null;

        if (cls == "")
            return "show: cleared";

        float terrain = GetGame().SurfaceY(spotX, spotZ);
        s_Ground = GetGame().SurfaceRoadY3D(spotX, terrain + 3, spotZ, RoadSurfaceDetection.UNDER);
        vector pos = Vector(spotX, s_Ground + 0.5, spotZ);

        s_Shown = GetGame().CreateObjectEx(cls, pos, ECE_PLACE_ON_SURFACE);
        if (!s_Shown)
            return "show: the game made nothing of " + cls;

        s_Shown.SetOrientation("0 0 0");

        vector mm[2];
        s_Shown.ClippingInfo(mm);
        pos = s_Shown.GetPosition();
        float off = s_Ground - (pos[1] + mm[0][1]);
        if (off > 0.02 || off < -0.02)
        {
            pos[1] = pos[1] + off;
            s_Shown.SetPosition(pos);
        }

        return "show: " + cls + " stands; ask op=top next";
    }

    // The top of the shown thing and a place beside it; `move` puts the first
    // player there.
    static string ShowTop(bool move)
    {
        if (!s_Shown)
            return "top: nothing is shown";

        vector aim;
        int cells;
        if (!TopOf(s_Shown, s_Ground, aim, cells))
            return "top: no top, cells=" + cells.ToString();

        float rise = aim[1] - s_Ground;
        vector best;
        float bestR;
        int places;
        bool havePlace = PlaceFor(aim, rise, best, bestR, places);

        string s = "top: aim=" + TGP_Out.F2(aim[0]) + " " + TGP_Out.F2(aim[1]) + " " + TGP_Out.F2(aim[2]);
        s = s + " rise=" + TGP_Out.F2(rise) + " cells=" + cells.ToString() + " places=" + places.ToString();
        if (havePlace)
        {
            s = s + " stand=" + TGP_Out.F2(best[0]) + " " + TGP_Out.F2(best[1]) + " " + TGP_Out.F2(best[2]);
            s = s + " r=" + TGP_Out.F2(bestR);

            PlayerBase player = FirstPlayer();
            if (move && player)
            {
                player.SetPosition(best);
                s = s + " moved";
            }
        }
        return s;
    }

    protected static void SweepMake()
    {
        string cls = s_Classes.Get(s_At);
        vector pos = Vector(s_SpotX, s_Ground + 0.5, s_SpotZ);

        s_Obj = GetGame().CreateObjectEx(cls, pos, ECE_PLACE_ON_SURFACE);
        if (!s_Obj)
        {
            s_At++;
            return;
        }

        s_Obj.SetOrientation("0 0 0");

        vector mm[2];
        s_Obj.ClippingInfo(mm);
        pos = s_Obj.GetPosition();
        float off = s_Ground - (pos[1] + mm[0][1]);
        if (off > 0.02 || off < -0.02)
        {
            pos[1] = pos[1] + off;
            s_Obj.SetPosition(pos);
        }

        s_Until = GetGame().GetTickTime() + 0.3;
        s_Phase = 1;
    }

    protected static void SweepNext(string tail)
    {
        if (tail != "")
            s_Rows.Insert(s_Head + ";" + tail);

        if (s_Obj)
            GetGame().ObjectDelete(s_Obj);
        s_Obj = null;
        s_At++;
        s_Phase = 0;

        if (s_Rows.Count() > 20)
            TGP_Out.Flush(FILE_ROWS, s_Rows);
    }

    protected static void SweepAim()
    {
        vector aim;
        int cells;
        string cls = s_Classes.Get(s_At);
        s_Head = s_At.ToString() + ";" + cls;

        if (!TopOf(s_Obj, s_Ground, aim, cells))
        {
            SweepNext(";;;;" + cells.ToString() + ";;;no top");
            return;
        }

        float rise = aim[1] - s_Ground;
        s_Head = s_Head + ";" + TGP_Out.F2(aim[0]) + ";" + TGP_Out.F2(aim[1]) + ";" + TGP_Out.F2(aim[2]);
        s_Head = s_Head + ";" + TGP_Out.F2(rise) + ";" + cells.ToString();

        vector best;
        float bestR;
        int places;
        bool havePlace = PlaceFor(aim, rise, best, bestR, places);
        s_Head = s_Head + ";" + places.ToString();

        PlayerBase player = FirstPlayer();
        if (!havePlace || !player)
        {
            SweepNext(";no place or no player");
            return;
        }

        s_Head = s_Head + ";" + TGP_Out.F2(bestR);

        player.SetPosition(best);
        float ax = aim[0];
        float ay = aim[1];
        float az = aim[2];
        string text = ax.ToString() + " " + ay.ToString() + " " + az.ToString();
        Ask(player, "aimplace", text);
        s_Asked++;

        s_Until = GetGame().GetTickTime() + 0.9;
        s_Phase = 2;
    }

    // True while there is work left.
    static bool SweepPump()
    {
        if (!s_On)
            return false;

        float now = GetGame().GetTickTime();
        PlayerBase player;

        if (s_Phase == 0)
        {
            if (s_At >= s_Classes.Count())
            {
                s_On = false;
                TGP_Out.Flush(FILE_ROWS, s_Rows);
                player = FirstPlayer();
                if (player)
                    Ask(player, "unplace", "");
                return false;
            }
            SweepMake();
            return true;
        }

        if (now < s_Until)
            return true;

        if (s_Phase == 1)
        {
            SweepAim();
            return true;
        }

        if (s_Phase == 2)
        {
            player = FirstPlayer();
            if (!player)
            {
                SweepNext("the player left");
                return true;
            }
            s_ToldAt = TGP_Verb.ToldCount();
            Ask(player, "holo", "");
            s_Until = now + 2.0;
            s_Phase = 3;
            return true;
        }

        // Phase 3 ran out of time: the line never came.
        SweepNext("no answer");
        return true;
    }

    // Called when a client's line arrives.
    static void SweepTold(string line)
    {
        if (!s_On || s_Phase != 3)
            return;
        if (line.IndexOf("holo:") != 0)
            return;

        SweepNext(line);
    }

    static void SweepCancel()
    {
        if (!s_On)
            return;

        if (s_Obj)
            GetGame().ObjectDelete(s_Obj);
        s_Obj = null;
        s_On = false;
        TGP_Out.Flush(FILE_ROWS, s_Rows);
    }
}
