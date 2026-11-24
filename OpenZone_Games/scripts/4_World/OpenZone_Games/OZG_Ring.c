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
//
// A FLOOR IS A ROAD SURFACE OR THE TOP OF A THING THAT STANDS STILL. Measured
// 2026-11-24 on a concrete wall of a third-party terrain: the mat lay on the
// wall's top, the placer stood on the wall's own step 0.86 m below it, and the
// game's road surface under that step was the ground another 1.36 m down. The
// step is geometry and no road surface, so a floor read as a road surface
// alone found nowhere to stand. LevelUnder asks both.
//
// THREE LAYOUTS (the owner, 2026-11-24). STANDING and SITTING seat the players
// on the floor below the mat, around the raised thing it lies on; FLOOR seats
// them on the level of the mat. A seat's KIND is only what its player does
// there, standing or sitting.

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
    int   LayMode;    // where the seats are: MODE_STANDING, _SITTING or _FLOOR
    bool  Guessed;    // the mod chose the layout, not the placer
    bool  Raised;     // the mat lies on a raised top
    float FloorY;

    void OZG_RingResult()
    {
        Seats = new array<ref OZG_SeatSpot>();
        Kind = OZG_Const.KIND_SITTING;
        LayMode = OZG_Const.MODE_FLOOR;
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
    // A fall of this much between two reads 0.2 m apart is an edge, not a
    // slope.
    static const float EDGE_MIN    = 0.35;
    // The mod's own guess: from this drop up the players stand, under it they
    // sit.
    static const float STAND_FROM  = 0.6;
    // How far under its start a floor is looked for, and how near a road
    // surface must be to be taken for the floor without a ray.
    static const float FLOOR_REACH = 2.0;
    static const float ROAD_NEAR   = 0.56;
    // A candidate.
    static const float STEP        = 0.1;
    static const float LEVEL_SLACK = 0.35;
    static const float BOX_LIFT    = 0.15;
    static const float SEEN_SLACK  = 0.25;

    protected static ref array<Object> s_Excluded = new array<Object>();
    protected static ref array<Object> s_Hit = new array<Object>();
    protected static ref set<Object> s_Seen = new set<Object>();
    protected static ref set<Object> s_Under = new set<Object>();

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

    // What is neither a floor nor in a seat's way: a body, what a body
    // carries, and the mod's own things.
    protected static bool LooksThrough(Object o, Object ignore)
    {
        if (o == ignore)
            return true;
        if (o.IsInherited(Man))
            return true;
        if (o.IsInherited(OZG_Place))
            return true;
        if (o.IsInherited(OZG_PlacePlacing))
            return true;
        if (o.IsInherited(OZG_SeatMark))
            return true;
        EntityAI e = EntityAI.Cast(o);
        if (e && e.GetHierarchyRootPlayer())
            return true;
        return false;
    }

    // Does this thing stand in a seat's way. Anything with a collision box
    // does, but for what LooksThrough names.
    protected static bool Blocks(Object o, Object ignore)
    {
        if (!o)
            return false;
        if (LooksThrough(o, ignore))
            return false;
        vector box[2];
        return o.GetCollisionBox(box);
    }

    // The level a body rests on at a point, looked for from `top` down: the
    // game's road surface, or the top of a thing under the ray, the higher of
    // the two. A ray that begins inside something solid answers `top` itself,
    // which no caller takes for a floor.
    static float LevelUnder(float x, float top, float z, Object ignore)
    {
        float road = GetGame().SurfaceRoadY3D(x, top, z, RoadSurfaceDetection.UNDER);
        // A road surface right under the start is the floor, and no ray is
        // spent on it: every caller starts no more than half a metre over the
        // level it expects. A ray costs: measured 2026-11-24 on a wall whose
        // step is no road surface, a ring took 9 ms with a ray at every read
        // where it had taken 2 to 3 ms with none.
        if (top - road < ROAD_NEAR)
            return road;
        float bottom = top - FLOOR_REACH;
        if (road > bottom)
            bottom = road;

        vector to = Vector(x, bottom, z);
        vector from;
        vector cpos;
        vector cdir;
        int comp;
        float y = top;
        float hy;
        Object hit;
        int guard = 0;
        while (guard < 6 && y > bottom + 0.05)
        {
            guard++;
            from = Vector(x, y, z);
            s_Under.Clear();
            if (!DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, s_Under, null, ignore, false, false, ObjIntersectGeom))
                break;
            // The ground itself: the road surface has said it already.
            if (s_Under.Count() == 0)
                break;
            hit = s_Under.Get(0);
            if (!hit)
                break;
            hy = cpos[1];
            if (LooksThrough(hit, ignore))
            {
                y = hy - 0.05;
                continue;
            }
            if (y - hy < 0.03)
                return top;
            if (hy > road)
                return hy;
            break;
        }
        return road;
    }

    // Is there floor under the whole of a seat and not under its middle
    // alone: four points around it, towards the mat, away from it and to both
    // sides. Without this a seat on the level of the mat was offered on the
    // brink of a wall's top, 2.5 m over the ground (2026-11-24).
    protected static bool Rests(float px, float pz, float fy, float dx, float dz, float half, Object ignore)
    {
        float top = fy + 0.5;
        float lvl;
        float d;

        lvl = LevelUnder(px + dx * half, top, pz + dz * half, ignore);
        d = lvl - fy;
        if (d >= LEVEL_SLACK || d <= -LEVEL_SLACK)
            return false;

        lvl = LevelUnder(px - dx * half, top, pz - dz * half, ignore);
        d = lvl - fy;
        if (d >= LEVEL_SLACK || d <= -LEVEL_SLACK)
            return false;

        lvl = LevelUnder(px - dz * half, top, pz + dx * half, ignore);
        d = lvl - fy;
        if (d >= LEVEL_SLACK || d <= -LEVEL_SLACK)
            return false;

        lvl = LevelUnder(px + dz * half, top, pz - dx * half, ignore);
        d = lvl - fy;
        if (d >= LEVEL_SLACK || d <= -LEVEL_SLACK)
            return false;

        return true;
    }

    protected static bool SpotOk(float px, float pz, float floorY, float ang, vector ext, float eye, vector to, Object ignore, out float fy)
    {
        fy = LevelUnder(px, floorY + 0.5, pz, ignore);
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

        float dx = Math.Sin(ang * Math.DEG2RAD);
        float dz = Math.Cos(ang * Math.DEG2RAD);
        float half = ext[0] * 0.5 - 0.05;
        if (!Rests(px, pz, fy, dx, dz, half, ignore))
            return false;

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
    // stands (the seats are counted from his side, and where he stands is one
    // of the two signs of a raised top), ignore the thing that stands for the
    // mat itself.
    static OZG_RingResult Compute(vector p, int mode, vector placer, Object ignore)
    {
        OZG_RingResult res = new OZG_RingResult();
        OZG_Settings cfg = OZG_Cfg.Get();

        float bearing = Math.Atan2(placer[0] - p[0], placer[2] - p[2]) * Math.RAD2DEG;
        if (bearing < 0)
            bearing = bearing + 360.0;
        int k0 = Math.Round(bearing * DIRS / 360.0);
        k0 = k0 % DIRS;

        // 1. The floor around the mat.
        array<float> firstDrops = new array<float>();
        array<float> allDrops = new array<float>();
        array<bool> edged = new array<bool>();
        int dropDirs = 0;
        float ang;
        float dx;
        float dz;
        float r;
        float lvl;
        float before;
        float drop;
        bool dropped;
        bool edge;
        int k;
        int s;
        for (k = 0; k < DIRS; k++)
        {
            ang = k * 360.0 / DIRS;
            dx = Math.Sin(ang * Math.DEG2RAD);
            dz = Math.Cos(ang * Math.DEG2RAD);
            dropped = false;
            edge = false;
            before = p[1];
            for (s = 1; s <= READ_STEPS; s++)
            {
                r = s * READ_STEP;
                lvl = LevelUnder(p[0] + dx * r, p[1] + READ_ABOVE, p[2] + dz * r, ignore);
                drop = p[1] - lvl;
                if (drop >= DROP_MIN && drop <= DROP_MAX)
                {
                    allDrops.Insert(lvl);
                    if (!dropped)
                    {
                        dropped = true;
                        firstDrops.Insert(lvl);
                    }
                    if (before - lvl >= EDGE_MIN)
                        edge = true;
                }
                before = lvl;
            }
            if (dropped)
                dropDirs++;
            edged.Insert(edge);
        }

        // A raised top: the floor is lower most of the way round, or the
        // placer himself stands lower, over an edge on his side. The second
        // sign is what a long thing gives, a wall's top or a bench, whose
        // floor is lower on one side or two.
        float under = p[1] - placer[1];
        bool below = under >= DROP_MIN && under <= DROP_MAX;
        bool edgeNear = false;
        if (edged[k0])
            edgeNear = true;
        if (edged[(k0 + 1) % DIRS])
            edgeNear = true;
        if (edged[(k0 + DIRS - 1) % DIRS])
            edgeNear = true;
        bool raised = dropDirs >= RAISED_DIRS;
        if (below && edgeNear)
            raised = true;
        res.Raised = raised;

        // 2. Where the seats are, and the floor they are on.
        float floorY = p[1];
        float found;
        int lay = OZG_Const.MODE_FLOOR;
        bool lower = false;
        if (mode == OZG_Const.MODE_STANDING || mode == OZG_Const.MODE_SITTING)
        {
            lay = mode;
            if (Commonest(allDrops, found))
            {
                floorY = found;
                lower = true;
            }
            else if (below)
            {
                floorY = placer[1];
                lower = true;
            }
        }
        else if (mode == OZG_Const.MODE_AUTO)
        {
            res.Guessed = true;
            if (raised)
            {
                if (Commonest(firstDrops, found))
                {
                    floorY = found;
                    lower = true;
                }
                else if (below)
                {
                    floorY = placer[1];
                    lower = true;
                }
            }
            if (lower)
            {
                lay = OZG_Const.MODE_SITTING;
                if (p[1] - floorY >= STAND_FROM)
                    lay = OZG_Const.MODE_STANDING;
            }
        }

        int kind = OZG_Const.KIND_SITTING;
        if (lay == OZG_Const.MODE_STANDING)
            kind = OZG_Const.KIND_STANDING;
        res.Kind = kind;
        res.LayMode = lay;
        res.FloorY = floorY;

        // Asked for seats below the mat, and nothing lower anywhere around.
        if (lay != OZG_Const.MODE_FLOOR && !lower)
            return res;

        // 3. A candidate in each direction, taken in turn from the placer's
        // side; 4. kept when far enough from every kept one.
        // A seat on the level of the mat no nearer than 0.95 m: at the
        // design's 0.7 m the ring holds five seats 0.7 m apart and no more
        // (measured 2026-11-24), and durak is a game for up to six. At 0.95 m
        // every second direction is far enough from the last, which gives
        // eight.
        float rMin = 0.95;
        float rMax = 1.5;
        float eye = 0.9;
        vector ext = "0.7 1.0 0.7";
        if (lay == OZG_Const.MODE_STANDING)
        {
            rMin = 0.5;
            rMax = 1.6;
            eye = 1.55;
            ext = "0.5 1.7 0.5";
        }
        else if (lay == OZG_Const.MODE_SITTING)
        {
            // A sitting body beside the raised thing, as near as its box
            // lets it.
            rMin = 0.6;
            rMax = 1.6;
        }

        vector to = Vector(p[0], p[1] + 0.1, p[2]);

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
