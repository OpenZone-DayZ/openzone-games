// Table Games Probe -- what the bridge verb `tgp` can ask for.
//
//   world_exec verb=tgp args={"op":"status"}
//   world_exec verb=tgp args={"op":"catalog","prefix":"StaticObj_Furniture_","limit":"0","spot":"4690 10190"}
//   world_exec verb=tgp args={"op":"area","pos":"x z","r":"30","pitch":"0.25","up":"25"}
//   world_exec verb=tgp args={"op":"col","pos":"x z","up":"25"}
//   world_exec verb=tgp args={"op":"seat","pos":"x y z","label":"runway","repeat":"20"}
//   world_exec verb=tgp args={"op":"cancel"}
//
// Every value of args is a string. A long job answers at once and is then
// read back with `status`; its rows are in $profile:TableGamesProbe/.

class TGP_Verb
{
    static const string AREA_HITS = "area_hits.csv";
    static const string AREA_OBJS = "area_objs.csv";
    static const string AREA_META = "area_meta.csv";

    protected static bool s_Said;
    protected static int s_AreaLabel;

    // The last line a client said back, and how many there have been.
    protected static string s_Told;
    protected static int s_ToldCount;

    static void Told(string line)
    {
        s_Told = line;
        s_ToldCount++;
        TGP_Sweep.SweepTold(line);
    }

    static int ToldCount()
    {
        return s_ToldCount;
    }

    // Called every server frame.
    static void Pump()
    {
        if (!s_Said)
        {
            s_Said = true;
            Print("[TableGamesProbe] probe loaded");
        }

        if (TGP_Sweep.s_On)
        {
            TGP_Sweep.SweepPump();
            return;
        }

        if (TGP_Catalog.s_On)
        {
            TGP_Catalog.CatPump(6);
            return;
        }

        if (TGP_Area.s_On)
            TGP_Area.AreaPump(6);
    }

    protected static string Arg(map<string, string> args, string key, string fallback)
    {
        string value;
        if (args && args.Find(key, value))
            return value;
        return fallback;
    }

    // "x z" or "x y z" into numbers; false when it is neither.
    protected static bool Numbers(string text, out array<float> nums)
    {
        nums = new array<float>;
        TStringArray parts = new TStringArray;
        text.Split(" ", parts);

        int n = parts.Count();
        string part;
        for (int i = 0; i < n; i++)
        {
            part = parts.Get(i);
            if (part != "")
                nums.Insert(part.ToFloat());
        }

        return nums.Count() == 2 || nums.Count() == 3;
    }

    protected static string Busy()
    {
        string s = "";
        if (TGP_Sweep.s_On)
        {
            s = "sweep " + TGP_Sweep.s_At.ToString() + "/" + TGP_Sweep.SweepCount().ToString();
            s = s + " asked=" + TGP_Sweep.s_Asked.ToString();
        }
        else if (TGP_Catalog.s_On)
        {
            s = "catalog " + TGP_Catalog.s_At.ToString() + "/" + TGP_Catalog.CatCount().ToString();
            s = s + " made=" + TGP_Catalog.s_Made.ToString() + " skipped=" + TGP_Catalog.s_Skipped.ToString();
        }
        else if (TGP_Area.s_On)
        {
            int total = TGP_Area.s_NX * TGP_Area.s_NZ;
            s = "area columns " + TGP_Area.s_Columns.ToString() + "/" + total.ToString() + " rows=" + TGP_Area.s_Rows.ToString();
        }
        return s;
    }

    static bool Run(string op, map<string, string> args, out string detail)
    {
        array<float> nums;
        string busy = Busy();

        if (op == "status")
        {
            if (busy == "")
            {
                detail = "idle; last catalog made=" + TGP_Catalog.s_Made.ToString() + " skipped=" + TGP_Catalog.s_Skipped.ToString();
                detail = detail + "; last area columns=" + TGP_Area.s_Columns.ToString() + " rows=" + TGP_Area.s_Rows.ToString();
                detail = detail + "; rays=" + TGP_Scan.s_Rays.ToString() + " roads=" + TGP_Scan.s_Roads.ToString();
            }
            else
            {
                detail = "busy: " + busy;
            }
            return true;
        }

        if (op == "cancel")
        {
            TGP_Sweep.SweepCancel();
            TGP_Catalog.CatCancel();
            TGP_Area.AreaCancel();
            detail = "stopped: " + busy;
            return true;
        }

        if (op == "catalog" || op == "area" || op == "sweep")
        {
            if (busy != "")
            {
                detail = "a job is running (" + busy + "); wait for it or send op=cancel";
                return false;
            }
        }

        if (op == "catalog")
        {
            if (!Numbers(Arg(args, "spot", "4690 10190"), nums))
            {
                detail = "catalog: spot must be 'x z'";
                return false;
            }

            string prefix = Arg(args, "prefix", "StaticObj_Furniture_");
            int limit = Arg(args, "limit", "0").ToInt();
            int found = TGP_Catalog.CatBegin(prefix, limit, nums.Get(0), nums.Get(nums.Count() - 1));
            detail = "catalog started: " + found.ToString() + " classes named " + prefix + "*";
            return found > 0;
        }

        if (op == "area")
        {
            if (!Numbers(Arg(args, "pos", ""), nums))
            {
                detail = "area: pos must be 'x z'";
                return false;
            }

            float cx = nums.Get(0);
            float cz = nums.Get(nums.Count() - 1);
            float radius = Arg(args, "r", "30").ToFloat();
            float pitch = Arg(args, "pitch", "0.25").ToFloat();
            float up = Arg(args, "up", "25").ToFloat();
            if (radius < 1 || radius > 150 || pitch < 0.05 || pitch > 2)
            {
                detail = "area: r is 1 to 150 and pitch 0.05 to 2";
                return false;
            }

            if (Arg(args, "fresh", "1") == "1")
            {
                TGP_Out.Fresh(AREA_HITS, "label;ix;iz;kind;y;rise;gap;obj;walk");
                TGP_Out.Fresh(AREA_OBJS, "label;obj;type;model;building;plain;item;plant;x;y;z");
                TGP_Out.Fresh(AREA_META, "label;x0;z0;pitch;nx;nz");
                s_AreaLabel = 0;
            }

            TGP_Area.AreaBegin(AREA_HITS, AREA_OBJS, s_AreaLabel, cx - radius, cz - radius, cx + radius, cz + radius, pitch, up, 1, null, -99999);

            array<string> meta = new array<string>;
            string row = s_AreaLabel.ToString() + ";" + TGP_Out.F2(cx - radius) + ";" + TGP_Out.F2(cz - radius) + ";" + pitch.ToString();
            row = row + ";" + TGP_Area.s_NX.ToString() + ";" + TGP_Area.s_NZ.ToString();
            meta.Insert(row);
            TGP_Out.Flush(AREA_META, meta);

            int columns = TGP_Area.s_NX * TGP_Area.s_NZ;
            detail = "area " + s_AreaLabel.ToString() + " started: " + columns.ToString() + " columns";
            s_AreaLabel++;
            return true;
        }

        if (op == "col")
        {
            if (!Numbers(Arg(args, "pos", ""), nums))
            {
                detail = "col: pos must be 'x z'";
                return false;
            }

            float x = nums.Get(0);
            float z = nums.Get(nums.Count() - 1);
            float ground = GetGame().SurfaceY(x, z);
            float top = ground + Arg(args, "up", "25").ToFloat();
            string names = "FGV";
            int count;
            float y;
            Object o;

            detail = "ground=" + TGP_Out.F2(ground);
            for (int kind = 0; kind < 3; kind++)
            {
                TGP_Scan.ColumnDown(x, z, top, ground - 1, kind, null);
                detail = detail + " " + names.Get(kind) + ":";
                count = TGP_Scan.s_Ys.Count();
                for (int i = 0; i < count; i++)
                {
                    y = TGP_Scan.s_Ys.Get(i);
                    o = TGP_Scan.s_Objs.Get(i);
                    detail = detail + " " + TGP_Out.F2(y) + "/" + TGP_Out.F2(TGP_Scan.RiseOf(x, y, z));
                    if (o)
                        detail = detail + "/" + o.GetType();
                }
            }
            return true;
        }

        if (op == "seat")
        {
            if (!Numbers(Arg(args, "pos", ""), nums))
            {
                detail = "seat: pos must be 'x z' (on the floor there) or 'x y z'";
                return false;
            }

            vector anchor;
            if (nums.Count() == 3)
            {
                anchor = Vector(nums.Get(0), nums.Get(1), nums.Get(2));
            }
            else
            {
                float terrain = GetGame().SurfaceY(nums.Get(0), nums.Get(1));
                float road = GetGame().SurfaceRoadY3D(nums.Get(0), terrain + 3, nums.Get(1), RoadSurfaceDetection.UNDER);
                anchor = Vector(nums.Get(0), road, nums.Get(1));
            }

            if (Arg(args, "fresh", "0") == "1")
                TGP_Seat.SeatFresh();

            int repeat = Arg(args, "repeat", "20").ToInt();
            detail = TGP_Seat.SeatRun(anchor, Arg(args, "label", "x"), repeat);
            return true;
        }

        if (op == "sweep")
        {
            if (!Numbers(Arg(args, "spot", "4690 10190"), nums))
            {
                detail = "sweep: spot must be 'x z'";
                return false;
            }

            string swPrefix = Arg(args, "prefix", "StaticObj_Furniture_");
            int swLimit = Arg(args, "limit", "0").ToInt();
            int swFound = TGP_Sweep.SweepBegin(swPrefix, swLimit, nums.Get(0), nums.Get(nums.Count() - 1));
            detail = "sweep started: " + swFound.ToString() + " classes named " + swPrefix + "*; the first player needs the deck in hands";
            return swFound > 0;
        }

        if (op == "show")
        {
            if (!Numbers(Arg(args, "spot", "4690 10190"), nums))
            {
                detail = "show: spot must be 'x z'";
                return false;
            }
            detail = TGP_Sweep.ShowMake(Arg(args, "class", ""), nums.Get(0), nums.Get(nums.Count() - 1));
            return true;
        }

        if (op == "top")
        {
            detail = TGP_Sweep.ShowTop(Arg(args, "move", "0") == "1");
            return true;
        }

        // To the first player's client: its answer comes back as a line.
        if (op == "ask")
        {
            array<Man> players = new array<Man>;
            GetGame().GetPlayers(players);
            if (players.Count() == 0)
            {
                detail = "ask: nobody is connected";
                return false;
            }

            PlayerBase target = PlayerBase.Cast(players.Get(0));
            if (!target || !target.GetIdentity())
            {
                detail = "ask: the first player has no identity yet";
                return false;
            }

            string what = Arg(args, "do", "state");
            ScriptRPC rpc = new ScriptRPC();
            rpc.Write(what);
            rpc.Write(Arg(args, "text", ""));
            rpc.Send(target, TGP_Rpc.ASK, true, target.GetIdentity());

            detail = "asked '" + what + "'; told so far " + s_ToldCount.ToString();
            return true;
        }

        if (op == "told")
        {
            detail = s_ToldCount.ToString() + " " + s_Told;
            return true;
        }

        // The slot probe: slots (what the engine knows), slotfill pos="x y z"
        // (two boxes of fifty-two cards there), slotclear pos="x y z".
        if (op == "slots")
        {
            detail = TGP_Slots.Count();
            return true;
        }

        if (op == "slotfill" || op == "slotclear")
        {
            if (!Numbers(Arg(args, "pos", ""), nums) || nums.Count() != 3)
            {
                detail = op + ": pos must be 'x y z'";
                return false;
            }
            vector spot = Vector(nums.Get(0), nums.Get(1), nums.Get(2));
            if (op == "slotfill")
                detail = TGP_Slots.Fill(spot);
            else
                detail = TGP_Slots.Clear(spot);
            return true;
        }

        detail = "tgp knows the ops: status, catalog, area, col, seat, sweep, ask, told, slots, slotfill, slotclear, cancel";
        return false;
    }
}
