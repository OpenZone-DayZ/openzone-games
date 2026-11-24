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
    protected bool m_WantsWindow;
    protected float m_NearWait;
    protected bool m_Near;
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

        string kind = "#STR_OZG_AIM_FLOOR";
        if (OZG_Aim.s_Layout == OZG_Const.MODE_STANDING)
            kind = "#STR_OZG_AIM_STANDING";
        else if (OZG_Aim.s_Layout == OZG_Const.MODE_SITTING)
            kind = "#STR_OZG_AIM_SITTING";
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

    // Did the player ask for the game's window with the key since the last
    // call. The mission opens it.
    bool TakeWindowAsk()
    {
        bool asked = m_WantsWindow;
        m_WantsWindow = false;
        return asked;
    }

    // Is this client seated at a place it stands near. Looked up twice a
    // second, not every frame.
    protected bool SeatedNear(float timeslice)
    {
        if (OZG_Mine.s_PlaceId == "" || OZG_Mine.s_Seat < 0)
            return false;
        m_NearWait = m_NearWait - timeslice;
        if (m_NearWait > 0)
            return m_Near;
        m_NearWait = 0.5;
        m_Near = false;
        Man me = GetGame().GetPlayer();
        if (!me)
            return false;
        array<OZG_Place> places = OZG_Place.All();
        for (int i = 0; i < places.Count(); i++)
        {
            OZG_Place place = places[i];
            if (!place || place.OZG_Id() != OZG_Mine.s_PlaceId)
                continue;
            float dist = vector.Distance(me.GetPosition(), place.GetPosition());
            if (dist <= OZG_Cfg.Get().ActRadius)
                m_Near = true;
            break;
        }
        return m_Near;
    }

    // Every frame.
    void Update(float timeslice)
    {
        if (!m_Root)
            return;

        bool menu = GetGame().GetUIManager().GetMenu() != null;
        bool live = OZG_Aim.Live();
        // Not aiming: a seated player near his game is told the key that
        // opens its window. The game allows no action while a gesture plays,
        // so a player sitting at a mat could not reach "To the game" without
        // standing up first; the key asks nothing of the body.
        bool seated = false;
        if (!live)
            seated = SeatedNear(timeslice);
        bool show = live || seated;
        if (menu)
            show = false;
        if (show != m_Root.IsVisible())
            m_Root.Show(show);
        if (!show)
            return;

        // The key: while aiming it steps the mode, the placer's to change
        // then and at no other time; at a game it opens the window.
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
                if (!typing && live)
                    OZG_Aim.NextMode();
                else if (!typing)
                    m_WantsWindow = true;
            }
        }

        if (!live)
        {
            string at = Widget.TranslateString("#STR_OZG_SEATED");
            if (at != m_Shown)
            {
                m_Shown = at;
                m_Line.SetText(at);
                m_Line.SetColor(ARGB(255, 255, 255, 255));
            }
            string seatedKey = "seated:" + KeyName();
            if (seatedKey != m_KeyShown)
            {
                m_KeyShown = seatedKey;
                string opens = Widget.TranslateString("#STR_OZG_SEATED_KEY");
                opens.Replace("%1", "[" + KeyName() + "]");
                m_Key.SetText(opens);
            }
            return;
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
            // The mark's arrow head points along the model's back (seen on
            // the stand 2026-11-24): half a turn makes it point to the mat.
            mark.SetOrientation(Vector(s.Yaw + 180, 0, 0));
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
