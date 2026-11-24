// The ring of seats around a place.
//
// One function for both sides: the server runs it when a place is made (and
// when a place comes back from storage with no session file), a client runs it
// while aiming, at most four times a second. Measured on the stand
// (2026-11-23): 0.13 ms in the open, 0.37 ms beside one table, up to 9 ms
// among houses.
//
// THE FLOOR IS READ AROUND THE MAT, NOT UNDER IT. Measured: 18 of 82 furniture
// tops are walkable, the main tables among them, so the walkable level under
// the mat is the table top itself and says nothing about where a player
// stands.

// One seat: a point, a facing towards the mat and a kind.
class OZG_SeatSpot
{
    vector Xyz;
    float  Yaw;
    int    Kind;
    int    Heading;   // which of the sixteen directions it lies in
}

class OZG_RingResult
{
    ref array<ref OZG_SeatSpot> Seats;
    int   Kind;       // the kind of every seat of this ring
    bool  Guessed;    // the mod chose the kind, not the placer
    bool  Raised;     // the mat lies on a raised top
    float FloorY;

    void OZG_RingResult()
    {
        Seats = new array<ref OZG_SeatSpot>();
        Kind = OZG_Const.KIND_GROUND;
    }

    int Count()
    {
        return Seats.Count();
    }

    // The seat nearest to a point, and how far it is; -1 when there is none.
    int Nearest(vector at, out float distance)
    {
        int best = -1;
        distance = 100000;
        for (int i = 0; i < Seats.Count(); i++)
        {
            float d = vector.Distance(at, Seats[i].Xyz);
            if (d < distance)
            {
                distance = d;
                best = i;
            }
        }
        return best;
    }
}

class OZG_Ring
{
    static const int   DIRS        = 16;
    // How the floor is read around the mat.
    static const float READ_STEP   = 0.2;
    static const int   READ_STEPS  = 8;      // out to 1.6 m
    static const float READ_ABOVE  = 0.3;
    static const float DROP_MIN    = 0.4;
    static const float DROP_MAX    = 1.3;
    static const int   RAISED_DIRS = 9;
    // A candidate.
    static const float STEP        = 0.1;
    static const float LEVEL_SLACK = 0.35;
    static const float BOX_LIFT    = 0.15;
    static const float SEEN_SLACK  = 0.25;

    protected static ref array<Object> s_Excluded = new array<Object>();
    protected static ref array<Object> s_Hit = new array<Object>();
    protected static ref set<Object> s_Seen = new set<Object>();

    // The commonest of the levels, to a decimetre.
    protected static bool Commonest(array<float> levels, out float level)
    {
        level = 0;
        int total = levels.Count();
        if (total == 0)
            return false;

        int best = 0;
        int bestCount = 0;
        float d;
        int count;
        int i;
        int j;
        for (i = 0; i < total; i++)
        {
            count = 0;
            for (j = 0; j < total; j++)
            {
                d = levels[j] - levels[i];
                if (d < 0.1 && d > -0.1)
                    count++;
            }
            if (count > bestCount)
            {
                bestCount = count;
                best = i;
            }
        }

        float sum = 0;
        int n = 0;
        for (j = 0; j < total; j++)
        {
            d = levels[j] - levels[best];
            if (d < 0.1 && d > -0.1)
            {
                sum = sum + levels[j];
                n++;
            }
        }
        level = sum / n;
        return true;
    }

    // Does this thing stand in a seat's way. Anything with a collision box
    // does; players do not, nor what they carry, nor the mod's own things.
    protected static bool Blocks(Object o, Object ignore)
    {
        if (!o)
            return false;
        if (o == ignore)
            return false;
        if (o.IsInherited(Man))
            return false;
        if (o.IsInherited(OZG_Place))
            return false;
        if (o.IsInherited(OZG_PlacePlacing))
            return false;
        if (o.IsInherited(OZG_SeatMark))
            return false;
        EntityAI e = EntityAI.Cast(o);
        if (e && e.GetHierarchyRootPlayer())
            return false;
        vector box[2];
        return o.GetCollisionBox(box);
    }

    protected static bool SpotOk(float px, float pz, float floorY, float ang, vector ext, float eye, vector to, Object ignore, out float fy)
    {
        fy = GetGame().SurfaceRoadY3D(px, floorY + 0.5, pz, RoadSurfaceDetection.UNDER);
        float dy = fy - floorY;
        if (dy >= LEVEL_SLACK || dy <= -LEVEL_SLACK)
            return false;

        vector center = Vector(px, fy + BOX_LIFT + ext[1] * 0.5, pz);
        vector orient = Vector(ang, 0, 0);

        s_Excluded.Clear();
        if (ignore)
            s_Excluded.Insert(ignore);
        s_Hit.Clear();
        GetGame().IsBoxColliding(center, orient, ext, s_Excluded, s_Hit);

        int n = s_Hit.Count();
        for (int i = 0; i < n; i++)
        {
            if (Blocks(s_Hit.Get(i), ignore))
                return false;
        }

        // The mat is seen from eye height.
        vector from = Vector(px, fy + eye, pz);
        vector cpos;
        vector cdir;
        int comp;
        s_Seen.Clear();
        if (DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, s_Seen, null, ignore, false, false, ObjIntersectView))
        {
            if (vector.Distance(cpos, to) > SEEN_SLACK)
            {
                // A player standing between is not a wall.
                bool wall = true;
                if (s_Seen.Count() > 0)
                {
                    Object first = s_Seen.Get(0);
                    if (first && first.IsInherited(Man))
                        wall = false;
                }
                if (wall)
                    return false;
            }
        }

        return true;
    }

    // p is the middle of the mat, placer where the player who lays it out
    // stands (the seats are counted from his side), ignore the thing that
    // stands for the mat itself.
    static OZG_RingResult Compute(vector p, int mode, vector placer, Object ignore)
    {
        OZG_RingResult res = new OZG_RingResult();
        OZG_Settings cfg = OZG_Cfg.Get();

        // 1. The floor around the mat.
        array<float> firstDrops = new array<float>();
        array<float> allDrops = new array<float>();
        int dropDirs = 0;
        float ang;
        float dx;
        float dz;
        float r;
        float lvl;
        float drop;
        bool dropped;
        int k;
        int s;
        for (k = 0; k < DIRS; k++)
        {
            ang = k * 360.0 / DIRS;
            dx = Math.Sin(ang * Math.DEG2RAD);
            dz = Math.Cos(ang * Math.DEG2RAD);
            dropped = false;
            for (s = 1; s <= READ_STEPS; s++)
            {
                r = s * READ_STEP;
                lvl = GetGame().SurfaceRoadY3D(p[0] + dx * r, p[1] + READ_ABOVE, p[2] + dz * r, RoadSurfaceDetection.UNDER);
                drop = p[1] - lvl;
                if (drop >= DROP_MIN && drop <= DROP_MAX)
                {
                    allDrops.Insert(lvl);
                    if (!dropped)
                    {
                        dropped = true;
                        firstDrops.Insert(lvl);
                    }
                }
            }
            if (dropped)
                dropDirs++;
        }
        bool raised = dropDirs >= RAISED_DIRS;
        res.Raised = raised;

        // 2. The kind of the seats and the floor they stand on.
        float floorY = p[1];
        float found;
        int kind = OZG_Const.KIND_GROUND;
        if (mode == OZG_Const.MODE_STANDING)
        {
            kind = OZG_Const.KIND_STANDING;
            if (!Commonest(allDrops, found))
            {
                // Nothing lower anywhere around: nowhere to stand.
                res.Kind = kind;
                res.FloorY = floorY;
                return res;
            }
            floorY = found;
        }
        else if (mode == OZG_Const.MODE_AUTO)
        {
            res.Guessed = true;
            if (res.Raised && Commonest(firstDrops, found))
            {
                kind = OZG_Const.KIND_STANDING;
                floorY = found;
            }
        }
        res.Kind = kind;
        res.FloorY = floorY;

        // 3. A candidate in each direction, taken in turn from the placer's
        // side; 4. kept when far enough from every kept one.
        // A ground seat no nearer than 0.95 m: at the design's 0.7 m the ring
        // holds five seats 0.7 m apart and no more (measured 2026-11-24), and
        // durak is a game for up to six. At 0.95 m every second direction is
        // far enough from the last, which gives eight.
        float rMin = 0.95;
        float rMax = 1.5;
        float eye = 0.9;
        vector ext = "0.7 1.0 0.7";
        if (kind == OZG_Const.KIND_STANDING)
        {
            rMin = 0.5;
            rMax = 1.6;
            eye = 1.55;
            ext = "0.5 1.7 0.5";
        }

        vector to = Vector(p[0], p[1] + 0.1, p[2]);
        float bearing = Math.Atan2(placer[0] - p[0], placer[2] - p[2]) * Math.RAD2DEG;
        if (bearing < 0)
            bearing = bearing + 360.0;
        int k0 = Math.Round(bearing * DIRS / 360.0);
        k0 = k0 % DIRS;

        float px;
        float pz;
        float fy;
        bool ok;
        bool clear;
        int heading;
        int kept;
        vector at;
        vector back;
        OZG_SeatSpot spot;
        for (k = 0; k < DIRS; k++)
        {
            if (res.Seats.Count() >= cfg.MaxSeats)
                break;

            heading = (k0 + k) % DIRS;
            ang = heading * 360.0 / DIRS;
            dx = Math.Sin(ang * Math.DEG2RAD);
            dz = Math.Cos(ang * Math.DEG2RAD);

            ok = false;
            r = rMin;
            while (!ok && r <= rMax + 0.001)
            {
                px = p[0] + dx * r;
                pz = p[2] + dz * r;
                if (SpotOk(px, pz, floorY, ang, ext, eye, to, ignore, fy))
                    ok = true;
                else
                    r = r + STEP;
            }
            if (!ok)
                continue;

            at = Vector(px, fy, pz);
            clear = true;
            for (kept = 0; kept < res.Seats.Count(); kept++)
            {
                if (vector.Distance(at, res.Seats[kept].Xyz) < cfg.SeatSpacing)
                {
                    clear = false;
                    break;
                }
            }
            if (!clear)
                continue;

            spot = new OZG_SeatSpot();
            spot.Xyz = at;
            back = Vector(p[0] - px, 0, p[2] - pz);
            spot.Yaw = back.VectorToAngles()[0];
            spot.Kind = kind;
            spot.Heading = heading;
            res.Seats.Insert(spot);
        }

        return res;
    }
}
