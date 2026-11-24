// The settings of OpenZone Games, with the values the design gives them.
//
// The file is $profile:OpenZone\OZ_Games_Settings.json. What a client needs of
// it (the poses, the radii) is sent by the server; until that arrives a client
// works with these defaults, which are the same numbers.

// One way of sitting or standing at a game. Poses are settings, not code: an
// admin whose server runs an animation mod adds its gestures by number.
class OZG_Pose
{
    string PoseId;
    string LabelKey;
    int    Kind;      // OZG_Const.KIND_STANDING or KIND_GROUND
    string Stance;    // "erect", "crouch", or "" when the pose is a gesture
    int    Gesture;   // the number of a gesture, 0 when the pose is a stance

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
}

class OZG_Settings
{
    int Version;
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

    void OZG_Settings()
    {
        Version = 1;
        PoseList = new array<ref OZG_Pose>();
        PoseList.Insert(OZG_Pose.Make("standing", "STR_OZG_POSE_STANDING", OZG_Const.KIND_STANDING, "erect", 0));
        PoseList.Insert(OZG_Pose.Make("crosslegged", "STR_OZG_POSE_CROSSLEGGED", OZG_Const.KIND_GROUND, "", 14));
        PoseList.Insert(OZG_Pose.Make("legsout", "STR_OZG_POSE_LEGSOUT", OZG_Const.KIND_GROUND, "", 15));
        PoseList.Insert(OZG_Pose.Make("campfire", "STR_OZG_POSE_CAMPFIRE", OZG_Const.KIND_GROUND, "", 13));
        PoseList.Insert(OZG_Pose.Make("squatting", "STR_OZG_POSE_SQUATTING", OZG_Const.KIND_GROUND, "crouch", 0));
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

    // Numbers a file may not carry: a radius of nothing would lock every game.
    void Mend()
    {
        if (!PoseList)
            PoseList = new array<ref OZG_Pose>();
        if (!GameIds)
            GameIds = new array<string>();
        if (IdleMinutes < 0)
            IdleMinutes = 0;
        if (HostAwayMinutes < 1)
            HostAwayMinutes = 1;
        if (ActRadius < 1.5)
            ActRadius = 1.5;
        if (WatchRadius < ActRadius)
            WatchRadius = ActRadius;
        if (SeatSpacing < 0.4)
            SeatSpacing = 0.4;
        if (SeatReach < 0.3)
            SeatReach = 0.3;
        if (PlaceSpacing < 1.0)
            PlaceSpacing = 1.0;
        if (MarkSeconds < 5)
            MarkSeconds = 5;
        if (MaxSeats < 2)
            MaxSeats = 2;
        if (MaxSeats > 12)
            MaxSeats = 12;
    }
}

// The settings in force on this machine.
class OZG_Cfg
{
    private static ref OZG_Settings s_Now;

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
        fresh.Mend();
        s_Now = fresh;
    }

    static void Reset()
    {
        s_Now = null;
    }
}
