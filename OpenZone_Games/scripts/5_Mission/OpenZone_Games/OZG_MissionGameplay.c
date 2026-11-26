// The client's entry of OpenZone Games: the aiming panel, the seats' marks,
// what the server says, and the window.
//
// A dedicated server makes no MissionGameplay, so none of this runs there.
modded class MissionGameplay
{
    protected ref OZG_AimHud m_OZG_Aim;
    protected ref OZG_Marks m_OZG_Marks;
    protected string m_OZG_OpenId;
    protected bool m_OZG_Asked;
    protected bool m_OZG_Talking;

    override void OnInit()
    {
        super.OnInit();

        m_OZG_Aim = new OZG_AimHud();
        m_OZG_Aim.Init();
        m_OZG_Marks = new OZG_Marks();
        OZG_Mine.Forget();
        OZG_Table.Reset();

        OZ_Show.OnShow.Insert(OZG_Show);
        OZ_ClientState.ServiceWatch().Insert(OZG_SvcRes);
        OZ_ClientState.SyncWatch().Insert(OZG_Sync);
        OZ_Notice.OnAnswer.Insert(OZG_Notice);
        OZG_PlaceWatch.OnMoved.Insert(OZG_PlaceMoved);
        OZG_PlaceWatch.OnGone.Insert(OZG_PlaceGone);
        OZG_PlaceWatch.OnHit.Insert(OZG_Hit);
        if (OZ_ClientState.Ready())
        {
            OZG_TakeSettings();
            OZG_AskMine();
        }
    }

    // What the server sent of its settings: the radii and the poses.
    protected void OZG_TakeSettings()
    {
        string text = OZ_ClientState.Extra(OZG_Cfg.SYNC_CFG, "");
        int count = OZG_Cfg.PoseCountOf(text);
        if (count < 0)
            return;
        array<string> poses = new array<string>();
        for (int i = 0; i < count; i++)
            poses.Insert(OZ_ClientState.Extra(OZG_Cfg.SYNC_POSE + i.ToString(), ""));
        OZG_Cfg.TakeSync(text, poses);
    }

    // The core's packet has come: the server knows this client, and can say
    // whether it holds a seat somewhere from before it connected.
    void OZG_Sync(OZ_SyncPayload p)
    {
        OZG_TakeSettings();
        OZG_AskMine();
    }

    protected void OZG_AskMine()
    {
        if (m_OZG_Asked)
            return;
        m_OZG_Asked = true;
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_MINE, "");
    }

    // A refusal out in the world, where no window is open to show it.
    void OZG_Notice(string op, bool ok, string why)
    {
        if (op.IndexOf(OZG_Const.NOTICE) != 0)
            return;
        // Words with their arguments ("key|arg|.."), or a key alone.
        string text = OZ_Notice.Text();
        if (why.IndexOf(OZG_Words.SEP) > 0)
            text = OZG_WordsText.Of(why);
        OZG_GameMenu menu = OZG_GameMenu.Current();
        if (menu)
        {
            menu.Said(text);
            return;
        }
        NotificationSystem.AddNotificationExtended(4, "#STR_OZG_MODNAME", text, "");
    }

    void OZG_Show(string what)
    {
        if (what.IndexOf(OZG_Const.SHOW_OPEN) != 0)
            return;
        string prefix = OZG_Const.SHOW_OPEN;
        int skip = prefix.Length();
        m_OZG_OpenId = what.Substring(skip, what.Length() - skip);
        m_OZG_Marks.Clear();
        // Not in this frame: at the moment of an action the menu stack is
        // still busy (the way the series opens its screens).
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(OZG_OpenGame, 100, false);
    }

    void OZG_OpenGame()
    {
        UIManager ui = GetGame().GetUIManager();
        if (OZG_GameMenu.Current())
            return;
        // Never over a player who is not there to play (a menu open at the
        // moment of a death takes the client down).
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive() || player.IsUnconscious())
            return;
        OZG_GameMenu.For(m_OZG_OpenId);
        ui.EnterScriptedMenu(OZG_Const.MENU_GAME, null);
    }

    void OZG_SvcRes(string serviceId, string op, bool ok, string json, string error)
    {
        if (serviceId != OZG_Const.SERVICE)
            return;

        OZG_GameMenu menu = OZG_GameMenu.Current();

        if (!ok)
        {
            string text = error;
            if (error.IndexOf("STR_") == 0)
                text = "#" + error;
            // A sleight refused flies nothing (card sharp's design §6.1).
            bool sleight = op == OZG_Const.OP_SLIP || op == OZG_Const.OP_SWAP || op == OZG_Const.OP_THROW;
            if (menu && sleight)
                menu.SleightRefused();
            if (menu)
                menu.Said(text);
            else
                NotificationSystem.AddNotificationExtended(4, "#STR_OZG_MODNAME", text, "");
            return;
        }

        // This player's own sleight, done (design §6.1, §9): its sound, and
        // the card flown in his window.
        if (op == OZG_Const.OP_SLEIGHT)
        {
            OZG_WSleight done;
            string bad2;
            if (!JsonFileLoader<OZG_WSleight>.LoadData(json, done, bad2) || !done)
                return;
            if (menu && menu.PlaceId() == done.PlaceId)
                menu.SleightDone(done);
            else
                OZG_Sounds.PlayLocal(done.SoundName, OZG_Place.OZG_ById(done.PlaceId));
            return;
        }

        if (op == OZG_Const.OP_MINE)
        {
            OZG_WMine mine;
            string bad;
            if (JsonFileLoader<OZG_WMine>.LoadData(json, mine, bad) && mine)
            {
                OZG_Mine.s_PlaceId = mine.PlaceId;
                OZG_Mine.s_Seat = mine.SeatNo;
            }
            return;
        }

        if (op == OZG_Const.OP_CLOSED)
        {
            OZG_WClosed closed;
            string err;
            if (!JsonFileLoader<OZG_WClosed>.LoadData(json, closed, err) || !closed)
                return;
            if (closed.PlaceId == OZG_Mine.s_PlaceId)
            {
                OZG_Mine.s_PlaceId = "";
                OZG_Mine.s_Seat = -1;
            }
            OZG_Table.Forget(closed.PlaceId);
            if (menu && menu.PlaceId() == closed.PlaceId)
                menu.CloseBecause(closed.Why);
            else if (closed.Why != "")
                NotificationSystem.AddNotificationExtended(4, "#STR_OZG_MODNAME", "#" + closed.Why, "");
            return;
        }

        // A sleight on the board seen (boards and dice design §9.1).
        if (op == OZG_Const.OP_BSAW)
        {
            OZG_WBoardSaw saw;
            string wrongSaw;
            if (!JsonFileLoader<OZG_WBoardSaw>.LoadData(json, saw, wrongSaw) || !saw)
                return;
            if (menu && menu.PlaceId() == saw.PlaceId)
                menu.BoardSaw(saw);
            return;
        }

        if (op == OZG_Const.OP_SHAKE)
        {
            OZG_WShake shake;
            string wrong;
            if (!JsonFileLoader<OZG_WShake>.LoadData(json, shake, wrong) || !shake)
                return;
            // Heard by this player alone: played here, on the place when this
            // client has it, never through the place's own number.
            OZG_Place where = OZG_Place.OZG_ById(shake.PlaceId);
            if (shake.SoundName != "")
                OZG_Sounds.PlayLocal(shake.SoundName, where);
            if (menu && menu.PlaceId() == shake.PlaceId && shake.RegionName != "")
                menu.ShakeRegion(shake.RegionName);
            return;
        }

        if (op == OZG_Const.OP_SEATS || op == OZG_Const.OP_VIEW)
        {
            OZG_TableView view = OZG_Table.Take(json);
            if (!view)
                return;
            if (op == OZG_Const.OP_SEATS)
            {
                m_OZG_Marks.Show(view.HeadPart);
                return;
            }
            // A view says where this client sits.
            if (view.HeadPart.Mine >= 0)
            {
                // Newly seated here: the first pose of the seat's kind.
                bool fresh = OZG_Mine.s_Seat != view.HeadPart.Mine || OZG_Mine.s_PlaceId != view.HeadPart.PlaceId;
                OZG_Mine.s_PlaceId = view.HeadPart.PlaceId;
                OZG_Mine.s_Seat = view.HeadPart.Mine;
                if (fresh)
                {
                    m_OZG_Marks.Clear();
                    // Facing the mat, then the first pose of the seat's kind.
                    PlayerBase me = PlayerBase.Cast(GetGame().GetPlayer());
                    int seatNo = view.HeadPart.Mine;
                    if (me && seatNo < view.HeadPart.Seats.Count())
                        me.OZG_Face(view.HeadPart.Seats[seatNo].Yaw);
                    OZG_PoseDriver.SitDown(view.HeadPart);
                }
            }
            else if (OZG_Mine.s_PlaceId == view.HeadPart.PlaceId)
            {
                OZG_Mine.s_PlaceId = "";
                OZG_Mine.s_Seat = -1;
            }
            if (menu && menu.PlaceId() == view.HeadPart.PlaceId)
                menu.Redraw();
        }
    }

    // This client's player was hit: the window closes, the seat is his.
    void OZG_Hit()
    {
        OZG_GameMenu menu = OZG_GameMenu.Current();
        if (menu)
            menu.Shut();
    }

    // A place's pulse moved: a window open on an older revision asks again,
    // which covers a lost message.
    void OZG_PlaceMoved(OZG_Place place)
    {
        OZG_GameMenu menu = OZG_GameMenu.Current();
        if (menu && place)
            menu.PulseOf(place);
    }

    // A place is gone from this client's world: the marks of its seats go
    // with it. Until 2026-11-24 only their clock took them away, so a player
    // who asked for the seats and then gathered the deck stood among the marks
    // of a game that was no longer there.
    void OZG_PlaceGone(string placeId)
    {
        if (m_OZG_Marks && placeId == OZG_Mine.s_MarkPlace)
            m_OZG_Marks.Clear();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        if (m_OZG_Aim)
        {
            m_OZG_Aim.Update(timeslice);
            // A seated player pressed the mod's key: his game's window.
            if (m_OZG_Aim.TakeWindowAsk() && OZG_Mine.s_PlaceId != "")
            {
                m_OZG_OpenId = OZG_Mine.s_PlaceId;
                OZG_OpenGame();
            }
        }
        if (m_OZG_Marks)
            m_OZG_Marks.Update();
        OZG_WatchVoice();
    }

    // The player's own voice going on and off, in the client's log with
    // whether the game's window was open: how push-to-talk under the window
    // is checked (the owner's word, 2026-11-25: the players talk while they
    // play).
    protected void OZG_WatchVoice()
    {
        bool talking = IsVoNActive();
        if (talking == m_OZG_Talking)
            return;
        m_OZG_Talking = talking;
        string state = "off";
        if (talking)
            state = "on";
        string window = "closed";
        if (OZG_GameMenu.Current())
            window = "open";
        OZG_Log.Dbg("voice " + state + ", the game's window " + window);
    }

    override void OnKeyPress(int key)
    {
        super.OnKeyPress(key);
        OZG_GameMenu menu = OZG_GameMenu.Current();
        if (menu)
            menu.KeyPressed(key);
    }

    override UIScriptedMenu CreateScriptedMenu(int id)
    {
        // super first, and out at once when it made something: that is how
        // other mods that touch this class live beside us.
        UIScriptedMenu menu = super.CreateScriptedMenu(id);
        if (menu)
            return menu;
        if (id == OZG_Const.MENU_GAME)
        {
            menu = new OZG_GameMenu();
            menu.SetID(id);
            return menu;
        }
        return null;
    }

    override void OnMissionFinish()
    {
        // The mirror of OnInit: the core's invokers are static and outlive
        // the mission.
        OZ_Show.OnShow.Remove(OZG_Show);
        OZ_ClientState.ServiceWatch().Remove(OZG_SvcRes);
        OZ_ClientState.SyncWatch().Remove(OZG_Sync);
        OZ_Notice.OnAnswer.Remove(OZG_Notice);
        OZG_PlaceWatch.OnMoved.Remove(OZG_PlaceMoved);
        OZG_PlaceWatch.OnGone.Remove(OZG_PlaceGone);
        OZG_PlaceWatch.OnHit.Remove(OZG_Hit);
        GetGame().GetCallQueue(CALL_CATEGORY_GUI).Remove(OZG_OpenGame);
        if (m_OZG_Marks)
            m_OZG_Marks.Clear();
        if (m_OZG_Aim)
            m_OZG_Aim.Destroy();
        m_OZG_Aim = null;
        m_OZG_Marks = null;
        OZG_Table.Reset();
        super.OnMissionFinish();
    }
}
