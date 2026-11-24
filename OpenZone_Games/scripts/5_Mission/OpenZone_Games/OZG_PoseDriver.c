// How this client's body sits or stands at a game.
//
// Poses are settings, not code (OZG_Settings.PoseList): each is a stance or
// the number of a gesture. The client applies the first pose of its seat's
// kind when it sits; the window's switch steps through the others. A pose
// pins nobody: a seat is kept by the session's rules, not by where the body
// is.
class OZG_PoseDriver
{
    protected static int s_PoseNo;

    protected static void PosesFor(OZG_WView view, array<ref OZG_Pose> into)
    {
        into.Clear();
        if (!view || view.Mine < 0 || view.Mine >= view.Seats.Count())
            return;
        OZG_Cfg.Get().PosesOf(view.Seats[view.Mine].Kind, into);
    }

    // The label of the pose in use; "" when the seat's kind has one pose or
    // none, and there is nothing to switch.
    static string LabelNow(OZG_WView view)
    {
        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        PosesFor(view, poses);
        if (poses.Count() < 2)
            return "";
        if (s_PoseNo >= poses.Count())
            s_PoseNo = 0;
        return Widget.TranslateString("#" + poses[s_PoseNo].LabelKey);
    }

    // The next pose of the seat's kind, applied; its number is returned for
    // the server to remember.
    static int Next(OZG_WView view)
    {
        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        PosesFor(view, poses);
        if (poses.Count() == 0)
            return 0;
        s_PoseNo = s_PoseNo + 1;
        if (s_PoseNo >= poses.Count())
            s_PoseNo = 0;
        Apply(poses[s_PoseNo]);
        return s_PoseNo;
    }

    // The first pose of the seat's kind, when the player has just sat down.
    static void SitDown(OZG_WView view)
    {
        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        PosesFor(view, poses);
        s_PoseNo = 0;
        if (poses.Count() > 0)
            Apply(poses[0]);
    }

    static void Apply(OZG_Pose pose)
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !pose)
            return;

        if (pose.Gesture > 0)
        {
            EmoteManager emotes = player.GetEmoteManager();
            if (emotes)
                emotes.CreateEmoteCBFromMenu(pose.Gesture, true);
            return;
        }

        HumanCommandMove move = player.GetCommand_Move();
        if (!move)
            return;
        if (pose.Stance == "crouch")
            move.ForceStance(DayZPlayerConstants.STANCEIDX_CROUCH);
        else if (pose.Stance == "erect")
            move.ForceStance(DayZPlayerConstants.STANCEIDX_ERECT);
    }
}
