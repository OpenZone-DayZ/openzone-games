// What a placer sees while a deck's hologram is out, and the seats' marks.
//
// Both live on one client only. The panel reads what the hologram last judged
// (OZG_Aim) and the key steps the mode; the marks are local things the server
// never hears of.

class OZG_AimHud
{
    protected static const string LAYOUT = "OpenZone_Games/gui/layouts/ozg_aim.layout";

    protected Widget m_Root;
    protected TextWidget m_Line;
    protected TextWidget m_Key;
    protected string m_Shown;
    protected string m_KeyShown;
    // No ref: the wrapper is a native object the script does not own (the
    // game keeps it the same way, radialmenu.c).
    protected UAIDWrapper m_Input;

    void Init()
    {
        if (!m_Root)
        {
            m_Root = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
            if (!m_Root)
            {
                OZG_Log.Error("the aiming panel did not load: " + LAYOUT);
                return;
            }
            m_Line = TextWidget.Cast(m_Root.FindAnyWidget("OZG_AimLine"));
            m_Key = TextWidget.Cast(m_Root.FindAnyWidget("OZG_AimKey"));
            m_Root.Show(false);
        }

        UAInput input = GetUApi().GetInputByName(OZG_Const.INPUT_MODE);
        if (input)
            m_Input = input.GetPersistentWrapper();
        else
            OZG_Log.Error("the input " + OZG_Const.INPUT_MODE + " is not there: is data/inputs.xml loaded?");
    }

    void Destroy()
    {
        if (m_Root)
            m_Root.Unlink();
        m_Root = null;
        m_Line = null;
        m_Key = null;
        m_Input = null;
    }

    protected string KeyName()
    {
        if (!m_Input)
            return "";
        UAInput input = m_Input.InputP();
        if (!input || input.BindKeyCount() < 1)
            return "";
        return GetUApi().GetButtonName(input.GetBindKey(0));
    }

    protected string LineNow()
    {
        if (OZG_Aim.s_Refused && OZG_Aim.s_Why != OZG_Const.LAY_NO_ROOM)
            return "#" + OZG_Aim.s_Why;

        string kind = "#STR_OZG_AIM_GROUND";
        if (OZG_Aim.s_Kind == OZG_Const.KIND_STANDING)
            kind = "#STR_OZG_AIM_STANDING";
        string how = "#STR_OZG_AIM_CHOSEN";
        if (OZG_Aim.s_Guessed)
            how = "#STR_OZG_AIM_GUESSED";

        string seats = OZG_Aim.s_Seats.ToString();
        string line = Widget.TranslateString("#STR_OZG_AIM_SEATS");
        line.Replace("%1", seats);
        line = line + " — " + Widget.TranslateString(kind);
        line = line + ", " + Widget.TranslateString(how);
        if (OZG_Aim.s_Refused)
            line = Widget.TranslateString("#" + OZG_Aim.s_Why) + ". " + line;
        return line;
    }

    // Every frame.
    void Update()
    {
        if (!m_Root)
            return;

        bool live = OZG_Aim.Live();
        if (live && GetGame().GetUIManager().GetMenu() != null)
            live = false;
        if (live != m_Root.IsVisible())
            m_Root.Show(live);
        if (!live)
            return;

        // The key: the mode is the placer's to change while aiming, and at no
        // other time.
        if (m_Input)
        {
            UAInput input = m_Input.InputP();
            if (input && input.LocalPress())
            {
                // A key is heard whatever holds the keyboard: not while a
                // text box does.
                Widget focused = GetFocus();
                bool typing = false;
                if (focused && EditBoxWidget.Cast(focused))
                    typing = true;
                if (focused && MultilineEditBoxWidget.Cast(focused))
                    typing = true;
                if (!typing)
                    OZG_Aim.NextMode();
            }
        }

        string line = LineNow();
        if (line != m_Shown)
        {
            m_Shown = line;
            m_Line.SetText(line);
            int tint = ARGB(255, 255, 255, 255);
            if (OZG_Aim.s_Refused)
                tint = ARGB(255, 255, 150, 130);
            m_Line.SetColor(tint);
        }

        string keyName = KeyName();
        if (keyName != m_KeyShown)
        {
            m_KeyShown = keyName;
            string hint = Widget.TranslateString("#STR_OZG_AIM_KEY");
            hint.Replace("%1", "[" + keyName + "]");
            m_Key.SetText(hint);
        }
    }
}

// The marks of a place's seats, for the seconds the server allows.
class OZG_Marks
{
    protected ref array<Object> m_Made = new array<Object>();

    void Clear()
    {
        for (int i = 0; i < m_Made.Count(); i++)
        {
            if (m_Made[i])
                GetGame().ObjectDelete(m_Made[i]);
        }
        m_Made.Clear();
        OZG_Mine.s_MarkPlace = "";
        OZG_Mine.s_MarkUntil = 0;
        OZG_Mine.s_MarkAt.Clear();
    }

    // The server answered "Sit down to the game" with the seats.
    void Show(OZG_WView view)
    {
        Clear();
        if (!view)
            return;
        bool playing = view.St == OZG_Const.ST_PLAYING;
        for (int i = 0; i < view.Seats.Count(); i++)
        {
            OZG_WSeat s = view.Seats[i];
            string type = "";
            if (s.St == OZG_Const.SEAT_LEFT)
                type = "OZG_SeatMark_Left";
            else if (s.St == OZG_Const.SEAT_FREE && !playing)
                type = "OZG_SeatMark_Free";
            if (type == "")
                continue;

            vector at = Vector(s.X, s.Y, s.Z);
            Object mark = GetGame().CreateObjectEx(type, at, ECE_LOCAL);
            if (!mark)
                continue;
            mark.SetPosition(at);
            mark.SetOrientation(Vector(s.Yaw, 0, 0));
            m_Made.Insert(mark);
            OZG_Mine.s_MarkAt.Insert(at);
        }
        OZG_Mine.s_MarkPlace = view.PlaceId;
        OZG_Mine.s_MarkUntil = GetGame().GetTime() + view.MarkSeconds * 1000;
        OZG_Mine.s_Reach = view.Reach;
    }

    // Every frame: the marks go when their time is up.
    void Update()
    {
        if (m_Made.Count() == 0)
            return;
        if (GetGame().GetTime() > OZG_Mine.s_MarkUntil)
            Clear();
    }
}
