// Table Games Probe -- the places around a laid-out mat.
//
// A first cut of "where can players be seated", written to be TIMED, not to
// be kept: sixteen directions around the mat, and in each the nearest spot
// where the floor is the mat's own floor, a body-sized box is free and the
// mat can be seen. The box test is the game's own for a spot beside a car
// door (Transport.IsAreaAtDoorFree): an oriented box lifted off the floor,
// blocked by anything that has a collision box.

class TGP_Seat
{
    static const int DIRS = 16;
    static const string FILE_ROWS = "seats.csv";

    static int s_Boxes;
    static int s_Rays;
    static int s_Roads;

    // The places of the last kept pass, for whoever wants to draw them.
    static ref array<vector> s_Places = new array<vector>;
    static bool s_Standing;

    protected static ref array<string> s_Rows = new array<string>;
    protected static ref array<Object> s_None = new array<Object>;
    protected static ref array<Object> s_Hit = new array<Object>;
    protected static ref set<Object> s_Res = new set<Object>;

    static void SeatFresh()
    {
        TGP_Out.Fresh(FILE_ROWS, "label;mode;dir;r;x;y;z");
    }

    // Is this spot a place: its floor is the anchor's floor, a body-sized box
    // over it is free, and from eye height the point `to` can be seen.
    static bool SpotOk(float px, float pz, float floorY, float ang, vector ext, float eye, vector to, out float fy)
    {
        s_Roads++;
        fy = GetGame().SurfaceRoadY3D(px, floorY + 0.5, pz, RoadSurfaceDetection.UNDER);
        float dy = fy - floorY;
        if (dy >= 0.35 || dy <= -0.35)
            return false;

        vector center = Vector(px, fy + 0.15 + ext[1] * 0.5, pz);
        vector orient = Vector(ang, 0, 0);
        vector mm[2];
        Object o;

        s_Hit.Clear();
        s_Boxes++;
        GetGame().IsBoxColliding(center, orient, ext, s_None, s_Hit);

        int n = s_Hit.Count();
        for (int i = 0; i < n; i++)
        {
            o = s_Hit.Get(i);
            if (o && !o.IsInherited(Man) && o.GetCollisionBox(mm))
                return false;
        }

        vector from = Vector(px, fy + eye, pz);
        vector cpos;
        vector cdir;
        int comp;
        s_Res.Clear();
        s_Rays++;
        if (DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, s_Res, null, null, false, false, ObjIntersectView))
        {
            if (vector.Distance(cpos, to) > 0.25)
                return false;
        }

        return true;
    }

    // One pass around the anchor. Returns how many of the sixteen directions
    // gave a place; `keep` writes them down.
    protected static int SeatRing(vector anchor, string label, bool keep, out bool standing, out float rise)
    {
        float floorY = GetGame().SurfaceRoadY3D(anchor[0], anchor[1] + 0.05, anchor[2], RoadSurfaceDetection.UNDER);
        s_Roads++;
        rise = anchor[1] - floorY;
        standing = rise >= 0.4;

        float rMin = 0.7;
        float rMax = 1.4;
        float eye = 0.9;
        vector ext = "0.7 1.0 0.7";
        string mode = "sit";
        if (standing)
        {
            rMin = 0.5;
            rMax = 1.6;
            eye = 1.55;
            ext = "0.5 1.7 0.5";
            mode = "stand";
        }

        vector to = Vector(anchor[0], anchor[1] + 0.1, anchor[2]);
        int seats = 0;
        float ang;
        float dx;
        float dz;
        float r;
        float px;
        float pz;
        float fy;
        bool found;
        string row;

        for (int k = 0; k < DIRS; k++)
        {
            ang = k * 360.0 / DIRS;
            dx = Math.Sin(ang * Math.DEG2RAD);
            dz = Math.Cos(ang * Math.DEG2RAD);
            found = false;
            r = rMin;

            while (!found && r <= rMax + 0.001)
            {
                px = anchor[0] + dx * r;
                pz = anchor[2] + dz * r;

                if (SpotOk(px, pz, floorY, ang, ext, eye, to, fy))
                    found = true;
                else
                    r = r + 0.1;
            }

            if (found)
            {
                seats++;
                if (keep)
                {
                    row = label + ";" + mode + ";" + k.ToString() + ";" + TGP_Out.F2(r);
                    row = row + ";" + TGP_Out.F2(px) + ";" + TGP_Out.F2(fy) + ";" + TGP_Out.F2(pz);
                    s_Rows.Insert(row);
                    s_Places.Insert(Vector(px, fy, pz));
                }
            }
        }

        return seats;
    }

    // Runs the ring `repeat` times and says what one pass costs.
    static string SeatRun(vector anchor, string label, int repeat)
    {
        if (repeat < 1)
            repeat = 1;

        bool standing;
        float rise;
        int seats = 0;
        int best = 0;
        int total = 0;
        int t0;
        int dt;

        s_Boxes = 0;
        s_Rays = 0;
        s_Roads = 0;
        s_Places.Clear();

        for (int pass = 0; pass < repeat; pass++)
        {
            t0 = TickCount(0);
            seats = SeatRing(anchor, label, pass == 0, standing, rise);
            dt = TickCount(t0);

            total = total + dt;
            if (pass == 0 || dt < best)
                best = dt;
        }

        TGP_Out.Flush(FILE_ROWS, s_Rows);

        float avgMs = total / 10000.0;
        avgMs = avgMs / repeat;
        float bestMs = best / 10000.0;
        int boxes = s_Boxes / repeat;
        int rays = s_Rays / repeat;
        int roads = s_Roads / repeat;

        string mode = "sit";
        if (standing)
            mode = "stand";
        s_Standing = standing;

        int dirs = DIRS;
        string s = "seat " + label + ": mode=" + mode + " rise=" + TGP_Out.F2(rise);
        s = s + " places=" + seats.ToString() + "/" + dirs.ToString();
        s = s + " per pass: boxes=" + boxes.ToString() + " rays=" + rays.ToString() + " roads=" + roads.ToString();
        s = s + " ms avg=" + avgMs.ToString() + " best=" + bestMs.ToString() + " passes=" + repeat.ToString();
        return s;
    }
}
