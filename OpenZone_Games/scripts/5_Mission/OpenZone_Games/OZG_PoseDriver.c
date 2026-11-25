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

    // The pose being taken, and how many steps it has been waited for. The
    // body may be busy when a pose is asked for: the sit action still on, a
    // gesture still ending. The game drops a gesture it refuses without a word
    // (emotemanager.c:991-997), so every step looks at what the body does.
    protected static ref OZG_Pose s_Want;
    protected static int s_Tries;
    protected static bool s_Ended;
    static const int STEP_MS = 250;
    static const int STEPS = 16;

    static void Apply(OZG_Pose pose)
    {
        if (!pose)
            return;
        s_Want = pose.Copy();
        s_Tries = 0;
        s_Ended = false;
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OZG_PoseDriver.Step);
        Step();
    }

    static void Step()
    {
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !s_Want)
            return;
        string why = Take(player, s_Want);
        if (why == "")
        {
            OZG_Log.Dbg("pose " + s_Want.PoseId + " taken");
            return;
        }
        s_Tries++;
        if (s_Tries >= STEPS)
        {
            OZG_Log.Warn("pose " + s_Want.PoseId + " not taken: " + why);
            return;
        }
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OZG_PoseDriver.Step, STEP_MS, false);
    }

    // "" when the body holds the pose; otherwise what is still in the way.
    protected static string Take(PlayerBase player, OZG_Pose pose)
    {
        EmoteManager emotes = player.GetEmoteManager();
        if (!emotes)
            return "no gesture manager";
        int gesture = pose.Gesture;
        if (gesture > 0 && Knows(emotes, gesture))
        {
            if (emotes.IsEmotePlaying() && emotes.OZG_CurrentGesture() == gesture)
                return "";
            if (emotes.GetEmoteLauncher() || emotes.OZG_Pending())
                return "gesture " + gesture.ToString() + " on its way";
            // A gesture on is ended by the manager itself before the next one
            // starts (emotemanager.c:435); with none on, the game must allow it.
            if (!emotes.IsEmotePlaying() && !emotes.CanPlayEmote(gesture))
                return "gesture " + gesture.ToString() + " refused by the game now";
            emotes.CreateEmoteCBFromMenu(gesture, true);
            return "gesture " + gesture.ToString() + " asked for";
        }

        // A stance: out of any gesture first (the movement command takes a
        // stance, and a gesture is not that command).
        if (emotes.IsEmotePlaying())
        {
            if (!s_Ended)
            {
                emotes.OZG_EndGesture();
                s_Ended = true;
            }
            return "a gesture still on";
        }
        if (pose.Stance == "")
            return "";
        int mask = DayZPlayerConstants.STANCEMASK_ERECT;
        int index = DayZPlayerConstants.STANCEIDX_ERECT;
        if (pose.Stance == "crouch")
        {
            mask = DayZPlayerConstants.STANCEMASK_CROUCH;
            index = DayZPlayerConstants.STANCEIDX_CROUCH;
        }
        if (player.IsPlayerInStance(mask))
            return "";
        HumanCommandMove move = player.GetCommand_Move();
        if (!move)
            return "not in the movement command";
        move.ForceStance(index);
        return "stance " + pose.Stance + " asked for";
    }

    // Whether the game has this gesture: the mod that brings it is loaded.
    protected static bool Knows(EmoteManager emotes, int gesture)
    {
        map<int, ref EmoteBase> known = emotes.GetNameEmoteMap();
        if (!known)
            return false;
        return known.Contains(gesture);
    }
}
