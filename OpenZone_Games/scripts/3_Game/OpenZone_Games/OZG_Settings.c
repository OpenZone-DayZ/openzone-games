// The settings of OpenZone Games, with the values the design gives them.
//
// The file is $profile:OpenZone\OZ_Games_Settings.json, loaded and checked by
// the core's loader: a broken file gives a line in the log and the defaults,
// never a server that does not start. What a client needs of it (the poses,
// the radii) rides in the core's sync packet; until that arrives a client
// works with these defaults, which are the same numbers.

// One way of sitting or standing at a game. Poses are settings, not code: an
// admin whose server runs an animation mod adds its gestures by number. A pose
// is its gesture when the game knows that number (the mod that brings it is
// loaded), otherwise its stance: "squatting" is SFP's squat where SFP runs,
// a plain crouch where it does not.
class OZG_Pose
{
    string PoseId;
    string LabelKey;
    int    Kind;      // OZG_Const.KIND_STANDING or KIND_SITTING
    string Stance;    // "erect", "crouch", or "": the stance, or the fallback of a gesture
    int    Gesture;   // the number of a gesture, 0 when the pose is a stance only

    static OZG_Pose Make(string id, string label, int kind, string stance, int gesture)
    {
        OZG_Pose p = new OZG_Pose();
        p.PoseId = id;
        p.LabelKey = label;
        p.Kind = kind;
        p.Stance = stance;
        p.Gesture = gesture;
        return p;
    }

    // A pose made by script out of one the JSON loader allocated: the loader
    // runs no initialiser, and a member the file did not carry is raw memory
    // (the core's rule for every nested object of a config).
    OZG_Pose Copy()
    {
        return Make(PoseId, LabelKey, Kind, Stance, Gesture);
    }
}

class OZG_Settings : OZ_ConfigBase
{
    ref array<ref OZG_Pose> PoseList;
    int   IdleMinutes;      // 0: a place never folds by itself
    int   HostAwayMinutes;
    float ActRadius;        // being present
    float WatchRadius;
    float SeatSpacing;
    float SeatReach;
    float PlaceSpacing;
    int   MarkSeconds;
    int   MaxSeats;
    ref array<string> GameIds;
    int   DebugLog;
    // Marks and the loupe (card sharp's design §7). Cheating 0: nothing can
    // be marked, studied or read.
    int   Cheating;
    int   MarkUses;
    int   MarkSeconds2;
    int   LoupeSeconds;
    int   MarkKnowers;

    void OZG_Settings()
    {
        LoadDefaults();
    }

    // 2: the marks. 3: squatting is SFP's squat where SFP runs.
    override int LatestVersion()
    {
        return 3;
    }

    protected void DefaultPoses()
    {
        PoseList = new array<ref OZG_Pose>();
        PoseList.Insert(OZG_Pose.Make("standing", "STR_OZG_POSE_STANDING", OZG_Const.KIND_STANDING, "erect", 0));
        PoseList.Insert(OZG_Pose.Make("crosslegged", "STR_OZG_POSE_CROSSLEGGED", OZG_Const.KIND_SITTING, "", 14));
        PoseList.Insert(OZG_Pose.Make("legsout", "STR_OZG_POSE_LEGSOUT", OZG_Const.KIND_SITTING, "", 15));
        PoseList.Insert(OZG_Pose.Make("campfire", "STR_OZG_POSE_CAMPFIRE", OZG_Const.KIND_SITTING, "", 13));
        PoseList.Insert(OZG_Pose.Make("squatting", "STR_OZG_POSE_SQUATTING", OZG_Const.KIND_SITTING, "crouch", OZG_Const.GESTURE_SFP_SQUAT));
    }

    override void LoadDefaults()
    {
        Version = LatestVersion();
        DefaultPoses();
        IdleMinutes = 30;
        HostAwayMinutes = 3;
        ActRadius = 3.0;
        WatchRadius = 5.0;
        SeatSpacing = 0.7;
        SeatReach = 0.6;
        PlaceSpacing = 2.0;
        MarkSeconds = 20;
        MaxSeats = 8;
        GameIds = new array<string>();
        GameIds.Insert("durak");
        DebugLog = 0;
        Cheating = 1;
        MarkUses = 20;
        MarkSeconds2 = 8;
        LoupeSeconds = 5;
        MarkKnowers = 16;
    }

    override bool Migrate(int from)
    {
        // Version 1 had no marks: their defaults.
        if (from < 2)
        {
            Cheating = 1;
            MarkUses = 20;
            MarkSeconds2 = 8;
            LoupeSeconds = 5;
            MarkKnowers = 16;
        }
        // Version 2's squatting was a crouch alone: it takes SFP's squat,
        // the crouch staying its fallback.
        if (from < 3 && PoseList)
        {
            for (int i = 0; i < PoseList.Count(); i++)
            {
                OZG_Pose p = PoseList[i];
                if (p && p.PoseId == "squatting" && p.Gesture == 0)
                    p.Gesture = OZG_Const.GESTURE_SFP_SQUAT;
            }
        }
        Version = LatestVersion();
        return true;
    }

    // Called by the loader straight after the read, and by whoever builds a
    // settings object by hand: numbers a file may not carry are put right, and
    // everything nested is made anew by script.
    override void Validate(out int warnings)
    {
        warnings = 0;

        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        if (PoseList)
        {
            for (int i = 0; i < PoseList.Count(); i++)
            {
                OZG_Pose p = PoseList[i];
                if (!p)
                    continue;
                OZG_Pose made = p.Copy();
                bool known = made.Kind == OZG_Const.KIND_STANDING || made.Kind == OZG_Const.KIND_SITTING;
                if (made.PoseId == "" || !known)
                {
                    warnings++;
                    continue;
                }
                if (made.Gesture < 0)
                    made.Gesture = 0;
                poses.Insert(made);
            }
        }
        PoseList = poses;
        if (PoseList.Count() == 0)
        {
            warnings++;
            DefaultPoses();
        }

        array<string> games = new array<string>();
        if (GameIds)
        {
            for (int g = 0; g < GameIds.Count(); g++)
            {
                if (GameIds[g] != "")
                    games.Insert(GameIds[g]);
            }
        }
        GameIds = games;

        if (IdleMinutes < 0)
        {
            IdleMinutes = 0;
            warnings++;
        }
        if (HostAwayMinutes < 1)
        {
            HostAwayMinutes = 3;
            warnings++;
        }
        if (ActRadius < 1.5)
        {
            ActRadius = 3.0;
            warnings++;
        }
        if (WatchRadius < ActRadius)
        {
            WatchRadius = ActRadius + 2.0;
            warnings++;
        }
        if (SeatSpacing < 0.4)
        {
            SeatSpacing = 0.7;
            warnings++;
        }
        if (SeatReach < 0.3)
        {
            SeatReach = 0.6;
            warnings++;
        }
        if (PlaceSpacing < 1.0)
        {
            PlaceSpacing = 2.0;
            warnings++;
        }
        if (MarkSeconds < 5)
        {
            MarkSeconds = 20;
            warnings++;
        }
        if (MaxSeats < 2 || MaxSeats > 12)
        {
            MaxSeats = 8;
            warnings++;
        }
        if (Cheating != 0)
            Cheating = 1;
        if (MarkUses < 1)
        {
            MarkUses = 20;
            warnings++;
        }
        if (MarkSeconds2 < 1)
        {
            MarkSeconds2 = 8;
            warnings++;
        }
        if (LoupeSeconds < 1)
        {
            LoupeSeconds = 5;
            warnings++;
        }
        if (MarkKnowers < 2)
        {
            MarkKnowers = 16;
            warnings++;
        }
        if (DebugLog != 0)
            DebugLog = 1;
    }

    // The poses of one kind, in the order of the file.
    void PosesOf(int kind, array<ref OZG_Pose> into)
    {
        into.Clear();
        for (int i = 0; i < PoseList.Count(); i++)
        {
            OZG_Pose p = PoseList[i];
            if (p && p.Kind == kind)
                into.Insert(p);
        }
    }

    bool GameOn(string gameId)
    {
        return GameIds.Find(gameId) >= 0;
    }
}

// What a client is told of the settings: the numbers in one extra of the
// core's sync packet, and one extra a pose.
class OZG_ClientCfg
{
    float ActRadius;
    float WatchRadius;
    float SeatReach;
    int   MarkSeconds;
    int   PoseCount;
    int   Cheating;
    int   MarkSeconds2;
    int   LoupeSeconds;
}

// The settings in force on this machine.
class OZG_Cfg
{
    static const string PATH = "$profile:OpenZone\\OZ_Games_Settings.json";
    static const string TAG = "games settings";
    static const string ADMIN_NAME = "games_settings";
    static const string ADMIN_OWNER = "games";

    // The keys of the core's sync packet.
    static const string SYNC_CFG = "games.cfg";
    static const string SYNC_POSE = "games.pose.";

    private static ref OZG_Settings s_Now;
    private static int s_Problems;

    static OZG_Settings Get()
    {
        if (!s_Now)
            s_Now = new OZG_Settings();
        return s_Now;
    }

    static void Put(OZG_Settings fresh)
    {
        if (!fresh)
            return;
        s_Now = fresh;
    }

    static void Reset()
    {
        s_Now = null;
        s_Problems = 0;
    }

    // What the last load had to put right.
    static int Problems()
    {
        return s_Problems;
    }

    // Server: the file, read and checked by the core's loader.
    static void ServerLoad()
    {
        OZG_Settings fresh = new OZG_Settings();
        OZ_ConfigLoader<OZG_Settings>.Load(PATH, TAG, fresh);
        s_Problems = OZ_ConfigProblems.Of(TAG);
        Put(fresh);
    }

    // Server: what a client needs, into the core's sync packet.
    static void Fill(OZ_SyncPayload p)
    {
        OZG_Settings now = Get();
        OZG_ClientCfg c = new OZG_ClientCfg();
        c.ActRadius = now.ActRadius;
        c.WatchRadius = now.WatchRadius;
        c.SeatReach = now.SeatReach;
        c.MarkSeconds = now.MarkSeconds;
        c.PoseCount = now.PoseList.Count();
        c.Cheating = now.Cheating;
        c.MarkSeconds2 = now.MarkSeconds2;
        c.LoupeSeconds = now.LoupeSeconds;
        string text;
        string err;
        JsonFileLoader<OZG_ClientCfg>.MakeData(c, text, err, false);
        OZ_SyncExtras.Put(p, SYNC_CFG, text);
        for (int i = 0; i < now.PoseList.Count(); i++)
        {
            string pose;
            JsonFileLoader<OZG_Pose>.MakeData(now.PoseList[i], pose, err, false);
            OZ_SyncExtras.Put(p, SYNC_POSE + i.ToString(), pose);
        }
    }

    // Client: how many poses the numbers' extra names; -1 when it does not
    // read.
    static int PoseCountOf(string text)
    {
        if (text == "")
            return -1;
        OZG_ClientCfg c = new OZG_ClientCfg();
        string err;
        if (!JsonFileLoader<OZG_ClientCfg>.LoadData(text, c, err) || !c)
            return -1;
        return c.PoseCount;
    }

    // Client: the settings out of the core's sync packet -- the numbers'
    // extra, and one text a pose.
    static void TakeSync(string text, array<string> poseTexts)
    {
        if (text == "")
            return;
        OZG_ClientCfg c = new OZG_ClientCfg();
        string err;
        if (!JsonFileLoader<OZG_ClientCfg>.LoadData(text, c, err) || !c)
            return;

        OZG_Settings fresh = new OZG_Settings();
        fresh.ActRadius = c.ActRadius;
        fresh.WatchRadius = c.WatchRadius;
        fresh.SeatReach = c.SeatReach;
        fresh.MarkSeconds = c.MarkSeconds;
        fresh.Cheating = c.Cheating;
        fresh.MarkSeconds2 = c.MarkSeconds2;
        fresh.LoupeSeconds = c.LoupeSeconds;
        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        for (int i = 0; i < poseTexts.Count(); i++)
        {
            if (poseTexts[i] == "")
                continue;
            OZG_Pose pose = new OZG_Pose();
            if (JsonFileLoader<OZG_Pose>.LoadData(poseTexts[i], pose, err) && pose)
                poses.Insert(pose.Copy());
        }
        if (poses.Count() > 0)
            fresh.PoseList = poses;
        int warnings;
        fresh.Validate(warnings);
        Put(fresh);
    }
}
