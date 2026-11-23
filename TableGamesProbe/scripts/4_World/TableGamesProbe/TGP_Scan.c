// Table Games Probe -- the rays.
//
// A surface is found the way the game's placement hologram finds one: a ray
// against fire geometry. The same column is read with the two other geometry
// kinds as well, so that a table top which is there for the eye and for the
// body, but not for the hologram's ray, shows up as a difference.
//
// Results are semicolon-separated lines in $profile:TableGamesProbe/; the
// arithmetic on them is done outside the game (tools/analyze.py).

class TGP_Out
{
    static const string DIR = "$profile:TableGamesProbe";

    static string PathOf(string name)
    {
        return DIR + "/" + name;
    }

    static void Fresh(string name, string header)
    {
        if (!FileExist(DIR))
            MakeDirectory(DIR);

        FileHandle fh = OpenFile(PathOf(name), FileMode.WRITE);
        if (fh == 0)
            return;

        FPrintln(fh, header);
        CloseFile(fh);
    }

    static void Flush(string name, array<string> lines)
    {
        int n = lines.Count();
        if (n == 0)
            return;

        FileHandle fh = OpenFile(PathOf(name), FileMode.APPEND);
        if (fh != 0)
        {
            for (int i = 0; i < n; i++)
                FPrintln(fh, lines.Get(i));
            CloseFile(fh);
        }

        lines.Clear();
    }

    // Two decimals are enough for a height and keep the files small.
    static string F2(float v)
    {
        float r = Math.Round(v * 100);
        r = r / 100;
        return r.ToString();
    }

    static string B(bool v)
    {
        if (v)
            return "1";
        return "0";
    }
}

class TGP_Scan
{
    static const int KIND_FIRE = 0;
    static const int KIND_GEOM = 1;
    static const int KIND_VIEW = 2;

    // What a table top can be: this far above the walkable surface under it.
    static const float RISE_MIN = 0.3;
    static const float RISE_MAX = 1.35;

    // Counters for the cost of a scan.
    static int s_Rays;
    static int s_Roads;

    protected static ref set<Object> s_Res = new set<Object>;

    // One column's hits of one geometry kind, top to bottom.
    static ref array<float> s_Ys = new array<float>;
    static ref array<float> s_Gaps = new array<float>;
    static ref array<Object> s_Objs = new array<Object>;

    static int IntersectOf(int kind)
    {
        if (kind == KIND_FIRE)
            return ObjIntersectFire;
        if (kind == KIND_GEOM)
            return ObjIntersectGeom;
        return ObjIntersectView;
    }

    // Every surface a ray going straight down meets between yTop and yBottom.
    // The ray starts again just under each hit. A ray that hits at once has
    // started inside something solid: that is stepped through, not recorded.
    static void ColumnDown(float x, float z, float yTop, float yBottom, int kind, Object ignore)
    {
        s_Ys.Clear();
        s_Gaps.Clear();
        s_Objs.Clear();

        int iType = IntersectOf(kind);
        vector to = Vector(x, yBottom, z);
        vector from;
        vector cpos;
        vector cdir;
        int comp;
        float y = yTop;
        float above = yTop;     // where the free space over the next hit begins
        float hy;
        int guard = 0;
        Object hit;
        bool more = true;

        while (more && guard < 60 && y > yBottom + 0.05)
        {
            guard++;
            from = Vector(x, y, z);
            s_Res.Clear();
            s_Rays++;

            if (!DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, s_Res, null, ignore, false, false, iType))
            {
                more = false;
            }
            else
            {
                hy = cpos[1];
                if (y - hy < 0.03)
                {
                    y = y - 0.2;
                    above = y;
                }
                else
                {
                    hit = null;
                    if (s_Res.Count() > 0)
                        hit = s_Res.Get(0);

                    s_Ys.Insert(hy);
                    s_Gaps.Insert(above - hy);
                    s_Objs.Insert(hit);

                    // The ground itself ends the column.
                    if (!hit)
                        more = false;

                    y = hy - 0.05;
                    above = y;
                }
            }
        }
    }

    // Height of a point over the surface a player can walk on under it. A
    // floor answers with nothing, a table top with its own height.
    static float RiseOf(float x, float y, float z)
    {
        s_Roads++;
        float road = GetGame().SurfaceRoadY3D(x, y + 0.05, z, RoadSurfaceDetection.UNDER);
        return y - road;
    }
}

// A rectangle of the world read column by column, a few milliseconds a frame.
class TGP_Area
{
    static bool s_On;
    static string s_File;
    static string s_ObjFile;
    static int s_Label;
    static float s_X0;
    static float s_Z0;
    static float s_Pitch;
    static int s_NX;
    static int s_NZ;
    static int s_IX;
    static int s_IZ;
    static float s_Up;
    static float s_Down;
    static Object s_Ignore;
    static int s_Rows;
    static int s_Columns;
    static float s_BaseY;

    protected static ref array<string> s_Lines = new array<string>;
    protected static ref array<Object> s_Seen = new array<Object>;

    // baseY above -9000 measures every height from that level instead of from
    // the walkable surface under the hit: a bed is walkable on top, and on the
    // stand's flat spot its height is still wanted.
    static void AreaBegin(string file, string objFile, int label, float x0, float z0, float x1, float z1, float pitch, float up, float down, Object ignore, float baseY)
    {
        s_BaseY = baseY;
        s_File = file;
        s_ObjFile = objFile;
        s_Label = label;
        s_X0 = x0;
        s_Z0 = z0;
        s_Pitch = pitch;
        int nx = Math.Floor((x1 - x0) / pitch);
        int nz = Math.Floor((z1 - z0) / pitch);
        s_NX = nx + 1;
        s_NZ = nz + 1;
        s_IX = 0;
        s_IZ = 0;
        s_Up = up;
        s_Down = down;
        s_Ignore = ignore;
        s_Rows = 0;
        s_Columns = 0;
        s_Lines.Clear();
        s_Seen.Clear();
        s_On = true;
    }

    protected static int SeenIndex(Object o)
    {
        if (!o)
            return -1;

        int i = s_Seen.Find(o);
        if (i < 0)
        {
            s_Seen.Insert(o);
            i = s_Seen.Count() - 1;
        }
        return i;
    }

    protected static void AreaColumn()
    {
        float x = s_X0 + s_IX * s_Pitch;
        float z = s_Z0 + s_IZ * s_Pitch;
        float ground = GetGame().SurfaceY(x, z);
        float yTop = ground + s_Up;
        float yBottom = ground - s_Down;
        float y;
        float rise;
        bool walk;
        int n;
        int i;
        int idx;
        string line;

        for (int kind = 0; kind < 3; kind++)
        {
            TGP_Scan.ColumnDown(x, z, yTop, yBottom, kind, s_Ignore);
            n = TGP_Scan.s_Ys.Count();
            for (i = 0; i < n; i++)
            {
                y = TGP_Scan.s_Ys.Get(i);
                rise = TGP_Scan.RiseOf(x, y, z);
                walk = rise < 0.08 && rise > -0.08;
                if (s_BaseY > -9000)
                    rise = y - s_BaseY;

                if (rise >= TGP_Scan.RISE_MIN && rise <= TGP_Scan.RISE_MAX)
                {
                    idx = SeenIndex(TGP_Scan.s_Objs.Get(i));
                    line = s_Label.ToString() + ";" + s_IX.ToString() + ";" + s_IZ.ToString() + ";" + kind.ToString();
                    line = line + ";" + TGP_Out.F2(y) + ";" + TGP_Out.F2(rise);
                    line = line + ";" + TGP_Out.F2(TGP_Scan.s_Gaps.Get(i)) + ";" + idx.ToString();
                    line = line + ";" + TGP_Out.B(walk);
                    s_Lines.Insert(line);
                    s_Rows++;
                }
            }
        }

        s_Columns++;
    }

    protected static void AreaEnd()
    {
        s_On = false;
        TGP_Out.Flush(s_File, s_Lines);

        array<string> rows = new array<string>;
        int n = s_Seen.Count();
        Object o;
        string row;
        vector p;

        for (int i = 0; i < n; i++)
        {
            o = s_Seen.Get(i);
            row = s_Label.ToString() + ";" + i.ToString();
            if (o)
            {
                p = o.GetPosition();
                row = row + ";" + o.GetType() + ";" + o.GetShapeName();
                row = row + ";" + TGP_Out.B(o.IsBuilding()) + ";" + TGP_Out.B(o.IsPlainObject());
                row = row + ";" + TGP_Out.B(o.IsInherited(InventoryItem)) + ";" + TGP_Out.B(o.IsTree() || o.IsBush());
                row = row + ";" + TGP_Out.F2(p[0]) + ";" + TGP_Out.F2(p[1]) + ";" + TGP_Out.F2(p[2]);
            }
            else
            {
                row = row + ";;;0;0;0;0;0;0;0";
            }
            rows.Insert(row);
        }

        TGP_Out.Flush(s_ObjFile, rows);
        s_Seen.Clear();
    }

    // True while there is work left.
    static bool AreaPump(float budgetMs)
    {
        if (!s_On)
            return false;

        float t0 = GetGame().GetTickTime();
        float spent = 0;

        while (s_On && spent < budgetMs)
        {
            AreaColumn();

            s_IX++;
            if (s_IX >= s_NX)
            {
                s_IX = 0;
                s_IZ++;
                if (s_IZ >= s_NZ)
                    AreaEnd();
            }

            spent = (GetGame().GetTickTime() - t0) * 1000;
        }

        if (s_On && s_Lines.Count() > 400)
            TGP_Out.Flush(s_File, s_Lines);

        return s_On;
    }

    static void AreaCancel()
    {
        if (s_On)
            AreaEnd();
    }
}

// Every config class of a name prefix, stood on the stand's flat spot one at
// a time and read from above: which of the furniture an admin can map has a
// top the hologram's ray stops on.
class TGP_Catalog
{
    static const string HITS = "catalog_hits.csv";
    static const string OBJS = "catalog_objs.csv";
    static const string META = "catalog_classes.csv";

    static bool s_On;
    static int s_At;
    static int s_Made;
    static int s_Skipped;

    protected static ref array<string> s_Classes = new array<string>;
    protected static ref array<string> s_Meta = new array<string>;
    protected static Object s_Obj;
    protected static float s_ReadyAt;
    protected static float s_Ground;
    protected static int s_Phase;       // 0 make, 1 let it settle, 2 read
    protected static float s_SpotX;
    protected static float s_SpotZ;

    static int CatCount()
    {
        return s_Classes.Count();
    }

    static int CatBegin(string prefix, int limit, float spotX, float spotZ)
    {
        s_Classes.Clear();
        s_Meta.Clear();

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

        TGP_Out.Fresh(HITS, "label;ix;iz;kind;y;rise;gap;obj;walk");
        TGP_Out.Fresh(OBJS, "label;obj;type;model;building;plain;item;plant;x;y;z");
        TGP_Out.Fresh(META, "label;class;x0;z0;pitch;nx;nz;ground;bottom;top;building;plain");

        s_SpotX = spotX;
        s_SpotZ = spotZ;
        s_At = 0;
        s_Made = 0;
        s_Skipped = 0;
        s_Phase = 0;
        s_Obj = null;

        int found = s_Classes.Count();
        bool any = found > 0;
        s_On = any;
        return found;
    }

    protected static float GroundAt(float x, float z)
    {
        float terrain = GetGame().SurfaceY(x, z);
        return GetGame().SurfaceRoadY3D(x, terrain + 3, z, RoadSurfaceDetection.UNDER);
    }

    protected static void CatMake()
    {
        string cls = s_Classes.Get(s_At);

        // Read BEFORE the thing stands there: a bed is walkable on top, and
        // asked afterwards the ground under it would be the bed.
        float ground = GroundAt(s_SpotX, s_SpotZ);
        s_Ground = ground;
        vector pos = Vector(s_SpotX, ground + 0.5, s_SpotZ);

        s_Obj = GetGame().CreateObjectEx(cls, pos, ECE_PLACE_ON_SURFACE);
        if (!s_Obj)
        {
            s_Skipped++;
            s_At++;
            return;
        }

        s_Obj.SetOrientation("0 0 0");

        // Whatever the placing made of it, the thing is to stand ON the
        // concrete: a half-sunk table would read as a low one.
        vector mm[2];
        s_Obj.ClippingInfo(mm);
        pos = s_Obj.GetPosition();
        float bottom = pos[1] + mm[0][1];
        float off = ground - bottom;
        if (off > 0.02 || off < -0.02)
        {
            pos[1] = pos[1] + off;
            s_Obj.SetPosition(pos);
        }

        s_Made++;
        s_ReadyAt = GetGame().GetTickTime() + 0.2;
        s_Phase = 1;
    }

    protected static void CatRead()
    {
        vector mm[2];
        s_Obj.ClippingInfo(mm);
        vector pos = s_Obj.GetPosition();
        float ground = s_Ground;

        float x0 = pos[0] + mm[0][0] - 0.2;
        float x1 = pos[0] + mm[1][0] + 0.2;
        float z0 = pos[2] + mm[0][2] - 0.2;
        float z1 = pos[2] + mm[1][2] + 0.2;

        // A wardrobe is furniture too; a hangar is not what this is for.
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

        float bottom = pos[1] + mm[0][1];
        float top = pos[1] + mm[1][1];
        float up = top - ground + 0.6;
        if (up < 1)
            up = 1;
        if (up > 8)
            up = 8;

        float pitch = 0.1;
        TGP_Area.AreaBegin(HITS, OBJS, s_At, x0, z0, x1, z1, pitch, up, 0.3, null, ground);

        string row = s_At.ToString() + ";" + s_Classes.Get(s_At) + ";" + TGP_Out.F2(x0) + ";" + TGP_Out.F2(z0);
        row = row + ";" + pitch.ToString() + ";" + TGP_Area.s_NX.ToString() + ";" + TGP_Area.s_NZ.ToString();
        row = row + ";" + TGP_Out.F2(ground) + ";" + TGP_Out.F2(bottom) + ";" + TGP_Out.F2(top);
        row = row + ";" + TGP_Out.B(s_Obj.IsBuilding()) + ";" + TGP_Out.B(s_Obj.IsPlainObject());
        s_Meta.Insert(row);

        s_Phase = 2;
    }

    // True while there is work left.
    static bool CatPump(float budgetMs)
    {
        if (!s_On)
            return false;

        if (s_Phase == 2)
        {
            if (TGP_Area.AreaPump(budgetMs))
                return true;

            if (s_Obj)
                GetGame().ObjectDelete(s_Obj);
            s_Obj = null;
            s_At++;
            s_Phase = 0;
            return true;
        }

        if (s_Phase == 1)
        {
            if (GetGame().GetTickTime() >= s_ReadyAt)
                CatRead();
            return true;
        }

        if (s_At >= s_Classes.Count())
        {
            s_On = false;
            TGP_Out.Flush(META, s_Meta);
            return false;
        }

        CatMake();
        return true;
    }

    static void CatCancel()
    {
        if (!s_On)
            return;

        TGP_Area.AreaCancel();
        if (s_Obj)
            GetGame().ObjectDelete(s_Obj);
        s_Obj = null;
        s_On = false;
        TGP_Out.Flush(META, s_Meta);
    }
}
