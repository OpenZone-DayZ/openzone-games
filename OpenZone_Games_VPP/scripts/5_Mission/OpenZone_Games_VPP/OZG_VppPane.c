// OpenZone Games in the OpenZone window of VPP (2026-11-25): the module "games" on
// the window's rail, three pages over one file, OZ_Games_Settings.json -- the games
// register it in the core's admin console as "games_settings" (OZG_Cfg.ADMIN_NAME):
//   Налаштування -- which games are played, the table and its seats, presence, time,
//                   the mod's detailed log;
//   Шулерство    -- marks, noticing and sleight of hand (card sharp's design §7), and
//                   since 2026-11-27 the numbers of boards and dice (boards and dice
//                   design §11): the loads of a die and how they are noticed, the swap,
//                   the wait between sleights on a board;
//   Пози         -- how a player sits or stands at a game (PoseList).
// Файли конфігів joins the module by itself: the core's window adds it under every
// module that owns a file of its console.
//
// The layouts are converted from the mockup's boards (css2layout, 2026-11-25;
// docs/specs/2026-11-13-openzone-vpp-page-guide.md): GamesSettings.dc.html,
// GamesSharp.dc.html and GamesPoses.dc.html under
// docs/specs/2026-11-12-openzone-vpp-mockup/project. They carry their own colours and
// follow the window by themselves; what this file names is in the reports
// ui/css2layout/ozg_vpp_*.txt. The games and the poses are lists of rows made from
// two templates.
//
// ONE FILE, ONE SAVE. The three pages are one form: the bar at the bottom of each
// names what is not saved on any of them, and saving writes them all after the
// window's confirm strip (design 3.4). The boxes refuse what the server's own check
// (OZG_Settings.Validate) would put right without a word, and name the rule: what the
// admin saves is what the server keeps. The answer reads the file back
// (OnCfgApplied), and the server sends what a client holds of it to everyone online.
//
// Guards: NO_GUI -- the server compiles Mission without UI; AVPPAdminTools and
// OpenZone_VPP -- names of CfgMods classes, defined by the engine.
//
// Members of this layer start with Ozg (fields m_Ozg): every glue modifies the same
// class, and a name two of them have does not compile. Widget names are unique in the
// whole window. Fields are declared bare and set in OnCreate.

#ifdef AVPPAdminTools
#ifdef OpenZone_VPP
#ifndef NO_GUI

modded class OZ_VppAdminMenu
{
    static const string OZG_MOD        = "games";
    static const string OZG_PAGE_SET   = "games.settings";
    static const string OZG_PAGE_SHARP = "games.sharp";
    static const string OZG_PAGE_POSES = "games.poses";

    // Numbers of the converted layouts, in layout units. The engine does not read the
    // boards, so they live twice: convert a page again, check these against its report.
    // BAR: the bar at the bottom of every page (the confirm strip stands over it).
    // LEG_*: a caption over a box's border -- its room left and right of the words, the
    // height of its ground and of its line. Rows: a game 393 x 38, a pose 487 x 46.
    static const float OZG_BAR_H     = 61;
    static const float OZG_LEG_PAD   = 6;
    static const float OZG_LEG_BAR_H = 8;
    static const float OZG_LEG_H     = 15;
    static const float OZG_GAME_W    = 393;
    static const float OZG_GAME_H    = 38;
    static const float OZG_POSE_W    = 487;
    static const float OZG_POSE_H    = 46;

    // The pages, as created.
    private Widget m_OzgSetPane;
    private Widget m_OzgShPane;
    private Widget m_OzgPoPane;

    // What is fitted to its words waits for the page to be on screen: frames left.
    private int m_OzgFitTries;
    // The script writes the boxes: an OnChange that comes of it is not an edit.
    private bool m_OzgFill;

    // The file as the server last sent it (the boxes are compared with it), and the
    // shipped values (the line under each box).
    private ref OZG_Settings m_OzgCfg;
    private ref OZG_Settings m_OzgDef;

    // The number boxes as a table: a field of OZG_Settings, its caption's key, its
    // group's key, whether it is a fraction, and the server's own bounds (a max below
    // 0: none).
    private ref array<string> m_OzgKeys;
    private ref array<string> m_OzgLabels;
    private ref array<string> m_OzgGroups;
    private ref array<int>    m_OzgFloats;
    private ref array<float>  m_OzgMins;
    private ref array<float>  m_OzgMaxs;

    // The switches as the admin has set them: the games ticked, the log, card sharping.
    private ref array<string> m_OzgGamesOn;
    private bool m_OzgDebugOn;
    private bool m_OzgCheatOn;

    // The two lists, and the poses as the admin is editing them (copies made by
    // script), with the one on the card.
    private ref OZ_VppFlexRows m_OzgGameRows;
    private ref OZ_VppFlexRows m_OzgPoseRows;
    private ref array<ref OZG_Pose> m_OzgPoses;
    private int m_OzgPosePick;

    override void OnCreate(Widget RootW)
    {
        super.OnCreate(RootW);
        if (!M_SUB_WIDGET)
            return;

        m_OzgFitTries = 0;
        m_OzgFill = false;
        m_OzgDebugOn = false;
        m_OzgCheatOn = true;
        m_OzgPosePick = 0;

        m_OzgKeys = new array<string>();
        m_OzgLabels = new array<string>();
        m_OzgGroups = new array<string>();
        m_OzgFloats = new array<int>();
        m_OzgMins = new array<float>();
        m_OzgMaxs = new array<float>();
        m_OzgGamesOn = new array<string>();
        m_OzgPoses = new array<ref OZG_Pose>();
        OzgTable();

        // The constructor fills the shipped values.
        m_OzgDef = new OZG_Settings();

        m_OzgSetPane = OzgAddPage(OZG_PAGE_SET, "#STR_OZG_VPP_PAGE_SET", "ozg_vpp_settings", "OzgSetPane");
        m_OzgShPane = OzgAddPage(OZG_PAGE_SHARP, "#STR_OZG_VPP_PAGE_SHARP", "ozg_vpp_sharp", "OzgShPane");
        m_OzgPoPane = OzgAddPage(OZG_PAGE_POSES, "#STR_OZG_VPP_PAGE_POSES", "ozg_vpp_poses", "OzgPoPane");

        if (m_OzgSetPane)
        {
            Widget gameHost = m_OzgSetPane.FindAnyWidget("OzgGamesRows");
            if (gameHost)
                m_OzgGameRows = new OZ_VppFlexRows(m_Flex, gameHost, "OpenZone_Games_VPP/gui/layouts/ozg_vpp_game_row.layout", "OZG_VppGameRow", OZG_GAME_W, OZG_GAME_H);
            else
                OZ_Log.Error("games vpp page: the games list has no OzgGamesRows");
        }
        if (m_OzgPoPane)
        {
            Widget poseHost = m_OzgPoPane.FindAnyWidget("OzgPoListRows");
            if (poseHost)
                m_OzgPoseRows = new OZ_VppFlexRows(m_Flex, poseHost, "OpenZone_Games_VPP/gui/layouts/ozg_vpp_pose_row.layout", "OZG_VppPoseRow", OZG_POSE_W, OZG_POSE_H);
            else
                OZ_Log.Error("games vpp page: the poses list has no OzgPoListRows");
        }

        // The captions and the empty form, before any file arrives.
        OzgCaptions();
        OzgPaintAll();
    }

    // A page of the module: its layout under the window's root, registered on the rail.
    // No hint label of its own: a page's lines go to the journal, and the status bar
    // shows the newest.
    private Widget OzgAddPage(string id, string label, string layout, string rootName)
    {
        string path = "OpenZone_Games_VPP/gui/layouts/" + layout + ".layout";
        Widget pane = OZ_VppFlex.Create(path, M_SUB_WIDGET, rootName);
        if (!pane)
        {
            OZ_Log.Error("games vpp page: layout failed to load: " + path);
            return null;
        }
        RegisterPage(OZG_MOD, "#STR_OZG_VPP_MOD", id, label, pane, "");
        return pane;
    }

    private bool OzgIsPage(string id)
    {
        if (id == OZG_PAGE_SET || id == OZG_PAGE_SHARP)
            return true;
        return id == OZG_PAGE_POSES;
    }

    private bool OzgShown()
    {
        return OzgIsPage(CurrentPane());
    }

    // ================================================= the window's hooks

    // A page of the module is shown: its lists made for the size it has now, and the
    // file asked for unless the form holds the admin's work.
    override void OnPaneShown(string id)
    {
        super.OnPaneShown(id);
        if (!OzgIsPage(id))
            return;

        m_OzgFitTries = 30;
        if (id == OZG_PAGE_SET)
            OzgBuildGames();
        if (id == OZG_PAGE_POSES)
            OzgBuildPoses();

        array<string> changed = new array<string>();
        OzgChanged(changed, false);
        if (changed.Count() == 0)
            AskCfg(OZG_Cfg.ADMIN_NAME);
        OzgPaintDirty();
    }

    // Every name goes on to super: Ядро › Файли конфігів shows the same file raw.
    override void OnCfgText(string name, string body)
    {
        if (name == OZG_Cfg.ADMIN_NAME)
            OzgTakeText(body);
        super.OnCfgText(name, body);
    }

    // The file was applied: a page of the module on screen reads it back as the server
    // keeps it.
    override void OnCfgApplied()
    {
        super.OnCfgApplied();
        if (OzgShown())
            AskCfg(OZG_Cfg.ADMIN_NAME);
    }

    // The confirm strip's "yes" (design 3.4). The module's actions start with "ozg:".
    override void OnConfirmed(string action)
    {
        if (action.IndexOf("ozg:") != 0)
        {
            super.OnConfirmed(action);
            return;
        }
        if (action == "ozg:save")
        {
            OzgSaveNow();
            return;
        }
        if (action == "ozg:drop")
        {
            OzgFillFromCfg();
            Hint("#STR_OZG_VPP_H_DROPPED");
        }
    }

    // Every page has its bar at the bottom: the confirm strip stands over it.
    override float ConfirmLift()
    {
        if (OzgIsPage(m_Shown))
            return OZG_BAR_H;
        return super.ConfirmLift();
    }

    // Every frame while the window is open: what waits to be fitted to its words.
    override void Tick()
    {
        super.Tick();
        if (m_OzgFitTries > 0)
            OzgFitTick();
    }

    // The window changed size: the lists' rows as wide as their lists again.
    override void OnWindowResized(float w, float h)
    {
        super.OnWindowResized(w, h);
        if (!M_SUB_WIDGET)
            return;
        if (m_OzgGameRows)
            m_OzgGameRows.Fit();
        if (m_OzgPoseRows)
            m_OzgPoseRows.Fit();
    }

    // The captions over the boxes' borders are as wide as their words in the game's
    // language. A text measures only once it is on screen, so the open page is tried
    // for a few frames after it is shown.
    private void OzgFitTick()
    {
        m_OzgFitTries = m_OzgFitTries - 1;

        bool done = true;
        int g;
        if (m_Shown == OZG_PAGE_SET)
        {
            for (g = 0; g < 5; g++)
            {
                if (!OzgFitLegend(m_OzgSetPane, "OzgSetLeg" + g.ToString()))
                    done = false;
            }
        }
        else if (m_Shown == OZG_PAGE_SHARP)
        {
            for (g = 0; g < 7; g++)
            {
                if (!OzgFitLegend(m_OzgShPane, "OzgShLeg" + g.ToString()))
                    done = false;
            }
        }
        else if (m_Shown == OZG_PAGE_POSES)
        {
            for (g = 0; g < 2; g++)
            {
                if (!OzgFitLegend(m_OzgPoPane, "OzgPoLeg" + g.ToString()))
                    done = false;
            }
        }

        if (done)
            m_OzgFitTries = 0;
    }

    // One caption over a border: its ground (<name>Bar hides the border under the words)
    // and its line (<name>Text) as wide as the words. False: the words do not measure
    // yet.
    private bool OzgFitLegend(Widget pane, string name)
    {
        if (!pane)
            return true;
        Widget bar = pane.FindAnyWidget(name + "Bar");
        TextWidget t = TextWidget.Cast(pane.FindAnyWidget(name + "Text"));
        if (!bar || !t)
            return true;
        if (!t.IsVisibleHierarchy())
            return true;

        int tw = OZ_VppFlex.TextWidth(t);
        if (tw <= 0)
            return false;

        float s = UnitScale();
        bar.SetSize(tw + 2 * OZG_LEG_PAD * s, OZG_LEG_BAR_H * s);
        t.SetSize(tw + 2, OZG_LEG_H * s);
        return true;
    }

    // ================================================= the table of boxes

    // The boxes in the order of the pages, with the bounds OZG_Settings.Validate puts on
    // them (WatchRadius has one more: not under ActRadius).
    private void OzgTable()
    {
        OzgRow("MaxSeats",        "MAXSEATS",     "#STR_OZG_VPP_G_TABLE",    false, 2, 12);
        OzgRow("SeatSpacing",     "SEATSPACING",  "#STR_OZG_VPP_G_TABLE",    true, 0.4, -1);
        OzgRow("SeatReach",       "SEATREACH",    "#STR_OZG_VPP_G_TABLE",    true, 0.3, -1);
        OzgRow("PlaceSpacing",    "PLACESPACING", "#STR_OZG_VPP_G_TABLE",    true, 1, -1);
        OzgRow("ActRadius",       "ACTRADIUS",    "#STR_OZG_VPP_G_PRESENCE", true, 1.5, -1);
        OzgRow("WatchRadius",     "WATCHRADIUS",  "#STR_OZG_VPP_G_PRESENCE", true, 1.5, -1);
        OzgRow("IdleMinutes",     "IDLE",         "#STR_OZG_VPP_G_TIME",     false, 0, -1);
        OzgRow("HostAwayMinutes", "HOSTAWAY",     "#STR_OZG_VPP_G_TIME",     false, 1, -1);
        OzgRow("MarkSeconds",     "MARKSECONDS",  "#STR_OZG_VPP_G_TIME",     false, 5, -1);
        OzgRow("MarkUses",        "MARKUSES",     "#STR_OZG_VPP_G_SHARP",    false, 1, -1);
        OzgRow("MarkSeconds2",    "MARKSECONDS2", "#STR_OZG_VPP_G_SHARP",    false, 1, -1);
        OzgRow("StudySeconds",    "STUDY",        "#STR_OZG_VPP_G_SHARP",    false, 1, -1);
        OzgRow("MarkKnowers",     "KNOWERS",      "#STR_OZG_VPP_G_SHARP",    false, 2, -1);
        OzgRow("NoticeGrace",     "GRACE",        "#STR_OZG_VPP_G_NOTICE",   false, 0, -1);
        OzgRow("NoticeStep",      "STEP",         "#STR_OZG_VPP_G_NOTICE",   false, 0, 100);
        OzgRow("NoticeMax",       "NOTICEMAX",    "#STR_OZG_VPP_G_NOTICE",   false, 0, 100);
        OzgRow("SleightsPerDeal", "SLEIGHTS",     "#STR_OZG_VPP_G_SLEIGHT",  false, 0, -1);
        OzgRow("DealerPicks",     "PICKS",        "#STR_OZG_VPP_G_SLEIGHT",  false, 0, -1);
        OzgRow("SpotPercent",     "SPOT",         "#STR_OZG_VPP_G_SLEIGHT",  false, 0, 100);
        OzgRow("CaughtSlip",      "C_SLIP",       "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        OzgRow("CaughtReplace",   "C_SWAP",       "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        OzgRow("CaughtThrow",     "C_THROW",      "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        OzgRow("CaughtDeckIn",    "C_DECKIN",     "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        OzgRow("CaughtDeckOut",   "C_DECKOUT",    "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        OzgRow("CaughtPick",      "C_PICK",       "#STR_OZG_VPP_G_CAUGHT",   false, 0, 100);
        // Boards and dice. LoadWeights is a list of three, one a step of the load: a box
        // for each place (OzgLoad, OzgSetLoad).
        OzgRow("LoadWeight1",     "LOAD1",        "#STR_OZG_VPP_G_LOADED",   false, 1, 100);
        OzgRow("LoadWeight2",     "LOAD2",        "#STR_OZG_VPP_G_LOADED",   false, 1, 100);
        OzgRow("LoadWeight3",     "LOAD3",        "#STR_OZG_VPP_G_LOADED",   false, 1, 100);
        OzgRow("NoticeLoaded",    "NOTICELOADED", "#STR_OZG_VPP_G_LOADED",   false, 0, 100);
        OzgRow("DiceSwaps",       "DICESWAPS",    "#STR_OZG_VPP_G_DSWAP",    false, 0, -1);
        OzgRow("ChanceDiceSwap",  "C_DSWAP",      "#STR_OZG_VPP_G_DSWAP",    false, 0, 100);
        OzgRow("BoardSleightMinutes", "BOARDSLEIGHT", "#STR_OZG_VPP_G_BOARDS", false, 0, -1);
    }

    private void OzgRow(string key, string code, string group, bool fraction, float min, float max)
    {
        m_OzgKeys.Insert(key);
        m_OzgLabels.Insert("#STR_OZG_VPP_F_" + code);
        m_OzgGroups.Insert(group);
        int f = 0;
        if (fraction)
            f = 1;
        m_OzgFloats.Insert(f);
        m_OzgMins.Insert(min);
        m_OzgMaxs.Insert(max);
    }

    // A place of LoadWeights (from 0); 0 when the list is not the three it must be.
    private float OzgLoad(OZG_Settings c, int at)
    {
        if (!c.LoadWeights || at < 0 || at >= c.LoadWeights.Count())
            return 0;
        int weight = c.LoadWeights.Get(at);
        return weight;
    }

    private void OzgSetLoad(OZG_Settings c, int at, int weight)
    {
        if (!c.LoadWeights || at < 0 || at >= c.LoadWeights.Count())
            return;
        c.LoadWeights.Set(at, weight);
    }

    // A field of OZG_Settings by its name, as a number. The class has no reflection to
    // lean on, so the thirty-two are named here once.
    private float OzgGet(OZG_Settings c, string key)
    {
        if (key == "MaxSeats")
            return c.MaxSeats;
        if (key == "SeatSpacing")
            return c.SeatSpacing;
        if (key == "SeatReach")
            return c.SeatReach;
        if (key == "PlaceSpacing")
            return c.PlaceSpacing;
        if (key == "ActRadius")
            return c.ActRadius;
        if (key == "WatchRadius")
            return c.WatchRadius;
        if (key == "IdleMinutes")
            return c.IdleMinutes;
        if (key == "HostAwayMinutes")
            return c.HostAwayMinutes;
        if (key == "MarkSeconds")
            return c.MarkSeconds;
        if (key == "MarkUses")
            return c.MarkUses;
        if (key == "MarkSeconds2")
            return c.MarkSeconds2;
        if (key == "StudySeconds")
            return c.StudySeconds;
        if (key == "MarkKnowers")
            return c.MarkKnowers;
        if (key == "NoticeGrace")
            return c.NoticeGrace;
        if (key == "NoticeStep")
            return c.NoticeStep;
        if (key == "NoticeMax")
            return c.NoticeMax;
        if (key == "SleightsPerDeal")
            return c.SleightsPerDeal;
        if (key == "DealerPicks")
            return c.DealerPicks;
        if (key == "SpotPercent")
            return c.SpotPercent;
        if (key == "CaughtSlip")
            return c.CaughtSlip;
        if (key == "CaughtReplace")
            return c.CaughtReplace;
        if (key == "CaughtThrow")
            return c.CaughtThrow;
        if (key == "CaughtDeckIn")
            return c.CaughtDeckIn;
        if (key == "CaughtDeckOut")
            return c.CaughtDeckOut;
        if (key == "CaughtPick")
            return c.CaughtPick;
        if (key == "LoadWeight1")
            return OzgLoad(c, 0);
        if (key == "LoadWeight2")
            return OzgLoad(c, 1);
        if (key == "LoadWeight3")
            return OzgLoad(c, 2);
        if (key == "NoticeLoaded")
            return c.NoticeLoaded;
        if (key == "DiceSwaps")
            return c.DiceSwaps;
        if (key == "ChanceDiceSwap")
            return c.ChanceDiceSwap;
        if (key == "BoardSleightMinutes")
            return c.BoardSleightMinutes;
        return 0;
    }

    // A field set from a box's words; the words are a number of the field's kind
    // (OzgFieldBad says so first).
    private void OzgSet(OZG_Settings c, string key, string typed)
    {
        int n = typed.ToInt();
        float f = typed.ToFloat();
        if (key == "MaxSeats")
            c.MaxSeats = n;
        else if (key == "SeatSpacing")
            c.SeatSpacing = f;
        else if (key == "SeatReach")
            c.SeatReach = f;
        else if (key == "PlaceSpacing")
            c.PlaceSpacing = f;
        else if (key == "ActRadius")
            c.ActRadius = f;
        else if (key == "WatchRadius")
            c.WatchRadius = f;
        else if (key == "IdleMinutes")
            c.IdleMinutes = n;
        else if (key == "HostAwayMinutes")
            c.HostAwayMinutes = n;
        else if (key == "MarkSeconds")
            c.MarkSeconds = n;
        else if (key == "MarkUses")
            c.MarkUses = n;
        else if (key == "MarkSeconds2")
            c.MarkSeconds2 = n;
        else if (key == "StudySeconds")
            c.StudySeconds = n;
        else if (key == "MarkKnowers")
            c.MarkKnowers = n;
        else if (key == "NoticeGrace")
            c.NoticeGrace = n;
        else if (key == "NoticeStep")
            c.NoticeStep = n;
        else if (key == "NoticeMax")
            c.NoticeMax = n;
        else if (key == "SleightsPerDeal")
            c.SleightsPerDeal = n;
        else if (key == "DealerPicks")
            c.DealerPicks = n;
        else if (key == "SpotPercent")
            c.SpotPercent = n;
        else if (key == "CaughtSlip")
            c.CaughtSlip = n;
        else if (key == "CaughtReplace")
            c.CaughtReplace = n;
        else if (key == "CaughtThrow")
            c.CaughtThrow = n;
        else if (key == "CaughtDeckIn")
            c.CaughtDeckIn = n;
        else if (key == "CaughtDeckOut")
            c.CaughtDeckOut = n;
        else if (key == "CaughtPick")
            c.CaughtPick = n;
        else if (key == "LoadWeight1")
            OzgSetLoad(c, 0, n);
        else if (key == "LoadWeight2")
            OzgSetLoad(c, 1, n);
        else if (key == "LoadWeight3")
            OzgSetLoad(c, 2, n);
        else if (key == "NoticeLoaded")
            c.NoticeLoaded = n;
        else if (key == "DiceSwaps")
            c.DiceSwaps = n;
        else if (key == "ChanceDiceSwap")
            c.ChanceDiceSwap = n;
        else if (key == "BoardSleightMinutes")
            c.BoardSleightMinutes = n;
    }

    // A field as its box shows it: a fraction as the engine writes it, a whole number
    // without a point.
    private string OzgText(OZG_Settings c, int i)
    {
        float v = OzgGet(c, m_OzgKeys[i]);
        if (m_OzgFloats[i] == 1)
            return v.ToString();
        int n = Math.Round(v);
        return n.ToString();
    }

    // A box's words, trimmed, with a decimal comma read as a point.
    private string OzgTyped(int i)
    {
        string t = GetEdit("Ozg_" + m_OzgKeys[i]);
        t = t.Trim();
        t.Replace(",", ".");
        return t;
    }

    // A number as an admin types it: digits, a minus in front, and for a fraction one
    // point.
    private bool OzgIsNumber(string s, bool fraction)
    {
        int n = s.Length();
        if (n == 0)
            return false;
        string digits = "0123456789";
        int points = 0;
        int seen = 0;
        string ch;
        for (int i = 0; i < n; i++)
        {
            ch = s.Substring(i, 1);
            if (i == 0 && ch == "-" && n > 1)
                continue;
            if (ch == ".")
            {
                points++;
                continue;
            }
            if (digits.IndexOf(ch) == -1)
                return false;
            seen++;
        }
        if (seen == 0)
            return false;
        if (points == 0)
            return true;
        return fraction && points == 1;
    }

    // Whether a box differs from the file. A box that holds no number of its kind
    // counts as changed: saving names it instead of writing a 0 the admin never typed.
    private bool OzgFieldChanged(int i)
    {
        if (!m_OzgCfg)
            return false;
        string typed = OzgTyped(i);
        bool fraction = m_OzgFloats[i] == 1;
        if (!OzgIsNumber(typed, fraction))
            return true;
        float was = OzgGet(m_OzgCfg, m_OzgKeys[i]);
        if (fraction)
        {
            float now = typed.ToFloat();
            return Math.AbsFloat(now - was) > 0.0005;
        }
        int whole = typed.ToInt();
        int wasWhole = Math.Round(was);
        return whole != wasWhole;
    }

    // What is wrong with a box, in the admin's words; "" when nothing is. The bounds
    // are the server's own (OZG_Settings.Validate).
    private string OzgFieldBad(int i)
    {
        string typed = OzgTyped(i);
        bool fraction = m_OzgFloats[i] == 1;
        if (!OzgIsNumber(typed, fraction))
        {
            if (fraction)
                return Widget.TranslateString("#STR_OZG_VPP_H_NUMBER");
            return Widget.TranslateString("#STR_OZG_VPP_H_WHOLE");
        }
        float v = typed.ToFloat();
        float lo = m_OzgMins[i];
        float hi = m_OzgMaxs[i];
        string low = OzgBound(lo, fraction);
        if (hi >= 0)
        {
            string high = OzgBound(hi, fraction);
            if (v < lo || v > hi)
                return string.Format(Widget.TranslateString("#STR_OZG_VPP_H_RANGE"), low, high);
            return "";
        }
        if (v < lo)
            return string.Format(Widget.TranslateString("#STR_OZG_VPP_H_ATLEAST"), low);

        // Who sees a game stands at least as far as who plays it.
        if (m_OzgKeys[i] == "WatchRadius")
        {
            int act = m_OzgKeys.Find("ActRadius");
            string actTyped = OzgTyped(act);
            if (OzgIsNumber(actTyped, true) && v < actTyped.ToFloat())
                return Widget.TranslateString("#STR_OZG_VPP_H_WATCH");
        }
        return "";
    }

    private string OzgBound(float v, bool fraction)
    {
        if (fraction)
            return v.ToString();
        int n = Math.Round(v);
        return n.ToString();
    }

    // ================================================= the file

    // The file arrived. A form with the admin's unsaved changes is left as it is: the
    // same file arrives for Ядро › Файли конфігів and after a page is shown again.
    private void OzgTakeText(string body)
    {
        if (m_OzgCfg)
        {
            array<string> changed = new array<string>();
            OzgChanged(changed, false);
            if (changed.Count() > 0)
            {
                if (OzgShown())
                    Hint("#STR_OZG_VPP_H_KEPT");
                return;
            }
        }

        // The root made by the script, the way the core's OZ_ConfigBase says: a field
        // the file lacks keeps its default. What the serializer allocated under it --
        // the poses and the list of games -- moves into script-made copies at once,
        // while the read is honest.
        OZG_Settings t = new OZG_Settings();
        string err;
        if (!JsonFileLoader<OZG_Settings>.LoadData(body, t, err) || !t)
        {
            if (OzgShown())
            {
                string bad = Widget.TranslateString("#STR_OZG_VPP_H_BAD_FILE");
                Hint("OZ_Games_Settings.json " + bad + " " + err);
            }
            return;
        }
        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        int i;
        if (t.PoseList)
        {
            for (i = 0; i < t.PoseList.Count(); i++)
            {
                if (t.PoseList[i])
                    poses.Insert(t.PoseList[i].Copy());
            }
        }
        t.PoseList = poses;
        array<string> games = new array<string>();
        if (t.GameIds)
        {
            for (i = 0; i < t.GameIds.Count(); i++)
                games.Insert(t.GameIds[i]);
        }
        t.GameIds = games;
        // The loads: three numbers, or the shipped three when the file holds another
        // count (the server's own check puts the same in their place).
        array<int> loads = new array<int>();
        bool three = t.LoadWeights && t.LoadWeights.Count() == 3;
        for (i = 0; i < 3; i++)
        {
            int weight = OzgLoad(m_OzgDef, i);
            if (three)
                weight = t.LoadWeights.Get(i);
            loads.Insert(weight);
        }
        t.LoadWeights = loads;

        m_OzgCfg = t;
        OzgFillFromCfg();
    }

    // The form as the file is: the boxes, the switches, the poses.
    private void OzgFillFromCfg()
    {
        if (!m_OzgCfg)
            return;

        m_OzgFill = true;
        for (int i = 0; i < m_OzgKeys.Count(); i++)
            SetEdit("Ozg_" + m_OzgKeys[i], OzgText(m_OzgCfg, i));
        m_OzgFill = false;

        m_OzgGamesOn.Clear();
        for (int g = 0; g < m_OzgCfg.GameIds.Count(); g++)
            m_OzgGamesOn.Insert(m_OzgCfg.GameIds[g]);
        bool debugOn = m_OzgCfg.DebugLog == 1;
        m_OzgDebugOn = debugOn;
        bool cheatOn = m_OzgCfg.Cheating == 1;
        m_OzgCheatOn = cheatOn;

        m_OzgPoses = new array<ref OZG_Pose>();
        for (int p = 0; p < m_OzgCfg.PoseList.Count(); p++)
            m_OzgPoses.Insert(m_OzgCfg.PoseList[p].Copy());
        if (m_OzgPosePick >= m_OzgPoses.Count())
            m_OzgPosePick = 0;

        OzgPaintAll();
    }

    // ================================================= painting

    private void OzgPaintAll()
    {
        OzgPaintNumbers();
        OzgPaintSwitches();
        OzgBuildGames();
        OzgBuildPoses();
        OzgPaintCard();
        OzgPaintNotice();
        OzgPaintLoads();
        OzgPaintDirty();
    }

    // The captions of the boxes: the layout leaves them to the script, which words
    // them in the admin's language.
    private void OzgCaptions()
    {
        int cInk = ARGB(255, 224, 223, 218);
        for (int i = 0; i < m_OzgKeys.Count(); i++)
            OzgSay("OzgL_" + m_OzgKeys[i], m_OzgLabels[i], cInk);
    }

    // Under each caption the field's default, in the alert colour with "changed" while
    // the box differs from the file, and the box's frame with it.
    private void OzgPaintNumbers()
    {
        if (!m_OzgKeys)
            return;

        int cMuted = ARGB(255, 160, 159, 153);
        int cAlert = ARGB(255, 245, 199, 64);
        int cEdge = ARGB(255, 60, 60, 58);
        string defWord = Widget.TranslateString("#STR_OZG_VPP_DEFAULT");
        string changedWord = Widget.TranslateString("#STR_OZG_VPP_CHANGED");
        string neverWord = Widget.TranslateString("#STR_OZG_VPP_NEVER");
        string noWaitWord = Widget.TranslateString("#STR_OZG_VPP_ZERO_NOWAIT");
        string noSwapsWord = Widget.TranslateString("#STR_OZG_VPP_ZERO_NONE");

        string key;
        string line;
        string shipped;
        bool dirty;
        int ink;
        int edge;
        for (int i = 0; i < m_OzgKeys.Count(); i++)
        {
            key = m_OzgKeys[i];
            dirty = OzgFieldChanged(i);
            shipped = OzgText(m_OzgDef, i);
            line = defWord + " " + shipped;
            if (key == "IdleMinutes")
                line = line + " · " + neverWord;
            if (key == "BoardSleightMinutes")
                line = line + " · " + noWaitWord;
            if (key == "DiceSwaps")
                line = line + " · " + noSwapsWord;
            ink = cMuted;
            edge = cEdge;
            if (dirty)
            {
                line = line + " · " + changedWord;
                ink = cAlert;
                edge = cAlert;
            }
            OzgSay("OzgD_" + key, line, ink);
            OzgPaintIn(M_SUB_WIDGET, "OzgBox_" + key + "Edge", edge);
        }
    }

    // The two switches of the form: the tick, and under it the default.
    private void OzgPaintSwitches()
    {
        bool fileDebug = false;
        bool fileCheat = true;
        if (m_OzgCfg)
        {
            fileDebug = m_OzgCfg.DebugLog == 1;
            fileCheat = m_OzgCfg.Cheating == 1;
        }
        OzgTick(m_OzgSetPane, "OzgDebugTick", m_OzgDebugOn);
        OzgTick(m_OzgShPane, "OzgCheatTick", m_OzgCheatOn);
        OzgSwitchLine("OzgD_DebugLog", "#STR_OZG_VPP_OFF_DEFAULT", m_OzgDebugOn != fileDebug);
        OzgSwitchLine("OzgD_Cheating", "#STR_OZG_VPP_ON_DEFAULT", m_OzgCheatOn != fileCheat);
    }

    private void OzgSwitchLine(string name, string key, bool dirty)
    {
        string line = Widget.TranslateString(key);
        int ink = ARGB(255, 160, 159, 153);
        if (dirty)
        {
            string changedWord = Widget.TranslateString("#STR_OZG_VPP_CHANGED");
            line = line + " · " + changedWord;
            ink = ARGB(255, 245, 199, 64);
        }
        OzgSay(name, line, ink);
    }

    // A checkbox of a converted page: the mark while it is on, the accent's frame and
    // ground on, a faint frame on the cell's ground off.
    private void OzgTick(Widget root, string tick, bool lit)
    {
        if (!root)
            return;
        Widget box = root.FindAnyWidget(tick);
        if (!box)
            return;
        Widget mark = box.FindAnyWidget(tick + "Mark");
        if (mark)
            mark.Show(lit);
        int edge = ARGB(255, 92, 90, 88);
        int fill = ARGB(255, 7, 7, 7);
        if (lit)
        {
            edge = ARGB(255, 232, 120, 43);
            fill = edge;
        }
        OZ_VppFlex.Paint(root, tick + "Edge", edge);
        OZ_VppFlex.Paint(root, tick + "Bg", fill);
    }

    // Шулерство: what the three boxes of noticing make, as the server rolls it -- the
    // chance min(most, max(0, deals - grace) * step) (card sharp's design §5).
    private void OzgPaintNotice()
    {
        if (!m_OzgShPane)
            return;
        int grace = OzgTypedInt("NoticeGrace");
        int perDeal = OzgTypedInt("NoticeStep");
        int most = OzgTypedInt("NoticeMax");
        int cMuted = ARGB(255, 160, 159, 153);
        if (grace < 0 || perDeal <= 0 || most <= 0)
        {
            OzgSay("OzgShNotice", "#STR_OZG_VPP_NOTICE_OFF", cMuted);
            return;
        }
        int deals = grace + 5;
        int chance = 5 * perDeal;
        if (chance > most)
            chance = most;
        int need = most / perDeal;
        if (need * perDeal < most)
            need++;
        int full = grace + need;
        // Put in by hand: string.Format drops a percent sign that no digit follows
        // ("%2 %;" came out "20 ;", measured 2026-11-25), and the line has two.
        string line = Widget.TranslateString("#STR_OZG_VPP_NOTICE_LINE");
        line.Replace("%1", deals.ToString());
        line.Replace("%2", chance.ToString());
        line.Replace("%3", most.ToString());
        line.Replace("%4", full.ToString());
        OzgSay("OzgShNotice", line, cMuted);
    }

    // Шулерство › Заряджені кості: what the box of noticing makes for each step of a
    // load, as the server rolls it -- NoticeLoaded times the step (1, 2, 3) at every throw
    // that shows the loaded face (boards and dice design §10); over 100 is 100.
    private void OzgPaintLoads()
    {
        if (!m_OzgShPane)
            return;
        int perStep = OzgTypedInt("NoticeLoaded");
        int cMuted = ARGB(255, 160, 159, 153);
        if (perStep <= 0)
        {
            OzgSay("OzgShLoads", "#STR_OZG_VPP_LOADS_OFF", cMuted);
            return;
        }
        int little = perStep;
        int well = perStep * 2;
        int heavy = perStep * 3;
        if (little > 100)
            little = 100;
        if (well > 100)
            well = 100;
        if (heavy > 100)
            heavy = 100;
        // Put in by hand, as the noticing line is: string.Format drops a percent sign
        // that no digit follows.
        string littleText = little.ToString();
        string wellText = well.ToString();
        string heavyText = heavy.ToString();
        string line = Widget.TranslateString("#STR_OZG_VPP_LOADS_LINE");
        line.Replace("%1", littleText);
        line.Replace("%2", wellText);
        line.Replace("%3", heavyText);
        OzgSay("OzgShLoads", line, cMuted);
    }

    // A whole-number box's value; the file's when the box holds no whole number.
    private int OzgTypedInt(string key)
    {
        int i = m_OzgKeys.Find(key);
        if (i < 0)
            return 0;
        string typed = OzgTyped(i);
        if (OzgIsNumber(typed, false))
            return typed.ToInt();
        OZG_Settings c = m_OzgCfg;
        if (!c)
            c = m_OzgDef;
        int v = Math.Round(OzgGet(c, key));
        return v;
    }

    // ================================================= Налаштування › Ігри

    // A row for every game this machine knows, the games of another mod too.
    private void OzgBuildGames()
    {
        if (!m_OzgGameRows)
            return;
        m_OzgGameRows.Clear();
        array<string> ids = new array<string>();
        OzgGameOrder(ids);
        Widget row;
        for (int i = 0; i < ids.Count(); i++)
        {
            row = m_OzgGameRows.Add(ids[i]);
            if (row)
                OzgPaintGameRow(row, ids[i]);
        }
        m_OzgGameRows.Fit();
    }

    private void OzgPaintGameRow(Widget row, string id)
    {
        int cInk = ARGB(255, 224, 223, 218);
        int cMuted = ARGB(255, 160, 159, 153);
        bool ticked = m_OzgGamesOn.Find(id) >= 0;
        int ink = cMuted;
        if (ticked)
            ink = cInk;
        OzgRowSay(row, "OzgGameName", OzgGameName(id), ink);
        OzgRowSay(row, "OzgGameNote", OzgGameNote(id), cMuted);
        OzgTick(row, "OzgGameTick", ticked);
    }

    private string OzgGameName(string id)
    {
        OZG_Game sample = OZG_Games.Sample(id);
        if (!sample || sample.LabelKey() == "")
            return id;
        return Widget.TranslateString("#" + sample.LabelKey());
    }

    // "гравців 2–6", "гравців 3".
    private string OzgGameNote(string id)
    {
        OZG_Game sample = OZG_Games.Sample(id);
        if (!sample)
            return "";
        int lo = sample.MinPlayers();
        int hi = sample.MaxPlayers();
        string fmt;
        if (lo == hi)
        {
            fmt = Widget.TranslateString("#STR_OZG_VPP_PLAYERS_ONE");
            return string.Format(fmt, lo.ToString());
        }
        fmt = Widget.TranslateString("#STR_OZG_VPP_PLAYERS_RANGE");
        return string.Format(fmt, lo.ToString(), hi.ToString());
    }

    private void OzgToggleGame(string id)
    {
        int at = m_OzgGamesOn.Find(id);
        if (at >= 0)
            m_OzgGamesOn.RemoveOrdered(at);
        else
            m_OzgGamesOn.Insert(id);
        if (m_OzgGameRows)
        {
            int r = m_OzgGameRows.Find(id);
            Widget row = m_OzgGameRows.Row(r);
            if (row)
                OzgPaintGameRow(row, id);
        }
        OzgPaintDirty();
    }

    // The games in the order of the file -- the order the host's button steps through,
    // and its first is the game a new place starts with -- then those the file does not
    // name, in the order of the registry.
    private void OzgGameOrder(array<string> into)
    {
        into.Clear();
        array<string> ids = new array<string>();
        OZG_Games.Ids(ids);
        int i;
        string id;
        if (m_OzgCfg)
        {
            for (i = 0; i < m_OzgCfg.GameIds.Count(); i++)
            {
                id = m_OzgCfg.GameIds[i];
                if (ids.Find(id) >= 0 && into.Find(id) < 0)
                    into.Insert(id);
            }
        }
        for (i = 0; i < ids.Count(); i++)
        {
            if (into.Find(ids[i]) < 0)
                into.Insert(ids[i]);
        }
    }

    // The games on, for the file: the file's own order kept, a game ticked anew after
    // them, and every id of the file this machine does not know kept where it was (a
    // game of a mod that is not loaded here).
    private void OzgGamesInto(array<string> into)
    {
        into.Clear();
        array<string> ids = new array<string>();
        OZG_Games.Ids(ids);
        array<string> order = new array<string>();
        OzgGameOrder(order);
        int i;
        string id;
        if (m_OzgCfg)
        {
            for (i = 0; i < m_OzgCfg.GameIds.Count(); i++)
            {
                id = m_OzgCfg.GameIds[i];
                if (ids.Find(id) < 0 || m_OzgGamesOn.Find(id) >= 0)
                    into.Insert(id);
            }
        }
        for (i = 0; i < order.Count(); i++)
        {
            id = order[i];
            if (m_OzgGamesOn.Find(id) >= 0 && into.Find(id) < 0)
                into.Insert(id);
        }
    }

    // ================================================= Пози

    private OZG_Pose OzgPicked()
    {
        if (m_OzgPosePick < 0 || m_OzgPosePick >= m_OzgPoses.Count())
            return null;
        return m_OzgPoses[m_OzgPosePick];
    }

    // A pose's name as this client words it: its key translated, its id when it has
    // no key.
    private string OzgPoseName(OZG_Pose p)
    {
        if (!p)
            return "";
        if (p.LabelKey == "")
            return p.PoseId;
        return Widget.TranslateString("#" + p.LabelKey);
    }

    private string OzgKindWord(int kind)
    {
        if (kind == OZG_Const.KIND_STANDING)
            return "#STR_OZG_VPP_KIND_STAND";
        return "#STR_OZG_VPP_KIND_SIT";
    }

    private string OzgStanceWord(string stance)
    {
        if (stance == "erect")
            return "#STR_OZG_VPP_ST_ERECT";
        if (stance == "crouch")
            return "#STR_OZG_VPP_ST_CROUCH";
        return "#STR_OZG_VPP_ST_KEEP";
    }

    private void OzgBuildPoses()
    {
        if (!m_OzgPoseRows)
            return;
        m_OzgPoseRows.Clear();
        Widget row;
        for (int i = 0; i < m_OzgPoses.Count(); i++)
        {
            row = m_OzgPoseRows.Add(i.ToString());
            if (row)
                OzgPaintPoseRow(row, i);
        }
        m_OzgPoseRows.Fit();
    }

    private void OzgPaintPoseRow(Widget row, int i)
    {
        OZG_Pose p = m_OzgPoses[i];
        if (!p)
            return;
        int cInk = ARGB(255, 224, 223, 218);
        int cMuted = ARGB(255, 160, 159, 153);
        OzgRowSay(row, "OzgPoRowName", OzgPoseName(p), cInk);
        OzgRowSay(row, "OzgPoRowId", p.PoseId, cMuted);
        OzgRowSay(row, "OzgPoRowKind", OzgKindWord(p.Kind), cMuted);
        string gesture = "—";
        if (p.Gesture > 0)
            gesture = p.Gesture.ToString();
        OzgRowSay(row, "OzgPoRowGesture", gesture, cInk);

        bool picked = i == m_OzgPosePick;
        if (picked)
        {
            OZ_VppFlex.Paint(row, "OZG_VppPoseRowEdge", ARGB(255, 232, 120, 43));
            OZ_VppFlex.Paint(row, "OZG_VppPoseRowBg", ARGB(255, 47, 29, 17));
            return;
        }
        OZ_VppFlex.Paint(row, "OZG_VppPoseRowEdge", ARGB(255, 19, 19, 18));
        OZ_VppFlex.Paint(row, "OZG_VppPoseRowBg", ARGB(255, 19, 19, 18));
    }

    private void OzgRepaintPoseRows()
    {
        if (!m_OzgPoseRows)
            return;
        Widget row;
        for (int i = 0; i < m_OzgPoseRows.Count(); i++)
        {
            row = m_OzgPoseRows.Row(i);
            if (row && i < m_OzgPoses.Count())
                OzgPaintPoseRow(row, i);
        }
    }

    // The card of the picked pose: its boxes, its words and the checks.
    private void OzgPaintCard()
    {
        if (!m_OzgPoPane)
            return;
        OZG_Pose p = OzgPicked();
        m_OzgFill = true;
        if (p)
        {
            SetEdit("OzgPoF_Id", p.PoseId);
            SetEdit("OzgPoF_Label", p.LabelKey);
            SetEdit("OzgPoF_Gesture", p.Gesture.ToString());
        }
        else
        {
            SetEdit("OzgPoF_Id", "");
            SetEdit("OzgPoF_Label", "");
            SetEdit("OzgPoF_Gesture", "");
        }
        m_OzgFill = false;
        OzgPaintCardWords();
        OzgPaintChecks();
    }

    // The card's words that follow its boxes: the title, how this client words the
    // key, the two selects.
    private void OzgPaintCardWords()
    {
        int cInk = ARGB(255, 224, 223, 218);
        int cMuted = ARGB(255, 160, 159, 153);
        OZG_Pose p = OzgPicked();
        if (!p)
        {
            OzgSay("OzgPoTitle", "#STR_OZG_VPP_NO_POSE", cMuted);
            OzgSay("OzgPoF_Says", "", cMuted);
            OzgSay("OzgPoF_KindText", "", cInk);
            OzgSay("OzgPoF_StanceText", "", cInk);
            return;
        }
        string name = OzgPoseName(p);
        OzgSay("OzgPoTitle", name, cInk);
        string fmt = Widget.TranslateString("#STR_OZG_VPP_SAYS");
        OzgSay("OzgPoF_Says", string.Format(fmt, name), cMuted);
        OzgSay("OzgPoF_KindText", OzgKindWord(p.Kind), cInk);
        OzgSay("OzgPoF_StanceText", OzgStanceWord(p.Stance), cInk);
    }

    // The checks of the list and of the picked pose: how many of each kind, whether an
    // id is empty or twice, whether this client has the gesture.
    private void OzgPaintChecks()
    {
        if (!m_OzgPoPane)
            return;

        int standing = 0;
        int sitting = 0;
        string empty = "";
        string twice = "";
        array<string> seen = new array<string>();
        OZG_Pose p;
        for (int i = 0; i < m_OzgPoses.Count(); i++)
        {
            p = m_OzgPoses[i];
            if (p.Kind == OZG_Const.KIND_STANDING)
                standing++;
            else
                sitting++;
            if (p.PoseId == "")
            {
                if (empty == "")
                    empty = OzgPoseName(p);
                continue;
            }
            if (seen.Find(p.PoseId) >= 0 && twice == "")
                twice = p.PoseId;
            seen.Insert(p.PoseId);
        }

        string fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_COUNT");
        string counts = string.Format(fmt, standing.ToString(), sitting.ToString());
        if (standing == 0 || sitting == 0)
            OzgCheck(0, counts, 1);
        else
            OzgCheck(0, counts, 0);

        if (empty != "")
        {
            fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_ID_EMPTY");
            OzgCheck(1, string.Format(fmt, empty), 2);
        }
        else if (twice != "")
        {
            fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_ID_TWICE");
            OzgCheck(1, string.Format(fmt, twice), 2);
        }
        else
            OzgCheck(1, "#STR_OZG_VPP_CHK_IDS_OK", 0);

        OZG_Pose picked = OzgPicked();
        if (!picked)
        {
            OzgCheck(2, "", 0);
            return;
        }
        if (picked.Gesture <= 0)
        {
            OzgCheck(2, "#STR_OZG_VPP_CHK_GEST_NONE", 3);
            return;
        }
        string number = picked.Gesture.ToString();
        if (OZG_PoseDriver.GestureKnown(picked.Gesture))
        {
            fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_GEST_OK");
            OzgCheck(2, string.Format(fmt, number), 0);
            return;
        }
        fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_GEST_NO");
        OzgCheck(2, string.Format(fmt, number), 1);
    }

    // One check: its words and its sign. state 0: right (a check, ok), 1: to look at (a
    // ring, alert), 2: wrong (a cross, bad), 3: a plain fact (a dot, muted).
    private void OzgCheck(int i, string text, int state)
    {
        int tint = ARGB(255, 102, 199, 92);
        int ink = ARGB(255, 224, 223, 218);
        string image = "check";
        if (state == 1)
        {
            tint = ARGB(255, 245, 199, 64);
            ink = tint;
            image = "ring";
        }
        else if (state == 2)
        {
            tint = ARGB(255, 224, 82, 74);
            ink = tint;
            image = "cross";
        }
        else if (state == 3)
        {
            tint = ARGB(255, 160, 159, 153);
            image = "dot";
        }
        OzgSay("OzgPoChk" + i.ToString(), text, ink);

        if (!m_OzgPoPane)
            return;
        ImageWidget sign = ImageWidget.Cast(m_OzgPoPane.FindAnyWidget("OzgPoDot" + i.ToString()));
        if (!sign)
            return;
        sign.Show(text != "");
        sign.LoadImageFile(0, "set:ozv_ui image:" + image);
        sign.SetImage(0);
        sign.SetColor(tint);
    }

    private void OzgPickPose(int i)
    {
        if (i < 0 || i >= m_OzgPoses.Count())
            return;
        m_OzgPosePick = i;
        OzgRepaintPoseRows();
        OzgPaintCard();
    }

    // A box of the card changed: the pose follows it at once, and the list with it.
    private void OzgPoseEdited(string name)
    {
        OZG_Pose p = OzgPicked();
        if (!p)
            return;
        string typed = GetEdit(name);
        typed = typed.Trim();
        if (name == "OzgPoF_Id")
            p.PoseId = typed;
        else if (name == "OzgPoF_Label")
            p.LabelKey = typed;
        else if (name == "OzgPoF_Gesture")
        {
            if (!OzgIsNumber(typed, false) || typed.ToInt() < 0)
            {
                OzgPaintDirty();
                return;
            }
            p.Gesture = typed.ToInt();
        }
        OzgRepaintPoseRows();
        OzgPaintCardWords();
        OzgPaintChecks();
        OzgPaintDirty();
    }

    private void OzgPoseKind()
    {
        OZG_Pose p = OzgPicked();
        if (!p)
            return;
        if (p.Kind == OZG_Const.KIND_STANDING)
            p.Kind = OZG_Const.KIND_SITTING;
        else
            p.Kind = OZG_Const.KIND_STANDING;
        OzgRepaintPoseRows();
        OzgPaintCardWords();
        OzgPaintChecks();
        OzgPaintDirty();
    }

    // The stance steps: leave the body as it is, stand up straight, crouch.
    private void OzgPoseStance()
    {
        OZG_Pose p = OzgPicked();
        if (!p)
            return;
        if (p.Stance == "")
            p.Stance = "erect";
        else if (p.Stance == "erect")
            p.Stance = "crouch";
        else
            p.Stance = "";
        OzgPaintCardWords();
        OzgPaintDirty();
    }

    // A new pose at the end of the list: sitting, no key, no gesture, an id nobody has.
    private void OzgPoseNew()
    {
        string id;
        int n = 1;
        while (true)
        {
            id = "pose" + n.ToString();
            if (!OzgPoseIdTaken(id))
                break;
            n++;
        }
        m_OzgPoses.Insert(OZG_Pose.Make(id, "", OZG_Const.KIND_SITTING, "", 0));
        m_OzgPosePick = m_OzgPoses.Count() - 1;
        OzgBuildPoses();
        OzgPaintCard();
        OzgPaintDirty();
    }

    private bool OzgPoseIdTaken(string id)
    {
        for (int i = 0; i < m_OzgPoses.Count(); i++)
        {
            if (m_OzgPoses[i] && m_OzgPoses[i].PoseId == id)
                return true;
        }
        return false;
    }

    // The picked pose one place up (by < 0) or down; the order is the order a player
    // steps through the poses of his kind.
    private void OzgPoseMove(int by)
    {
        int src = m_OzgPosePick;
        int dst = src + by;
        if (src < 0 || src >= m_OzgPoses.Count())
            return;
        if (dst < 0 || dst >= m_OzgPoses.Count())
            return;
        OZG_Pose a = m_OzgPoses[src];
        OZG_Pose b = m_OzgPoses[dst];
        m_OzgPoses.Set(dst, a);
        m_OzgPoses.Set(src, b);
        m_OzgPosePick = dst;
        OzgRepaintPoseRows();
        OzgPaintCard();
        OzgPaintDirty();
    }

    private void OzgPoseDelete()
    {
        if (m_OzgPosePick < 0 || m_OzgPosePick >= m_OzgPoses.Count())
            return;
        m_OzgPoses.RemoveOrdered(m_OzgPosePick);
        if (m_OzgPosePick >= m_OzgPoses.Count())
            m_OzgPosePick = m_OzgPoses.Count() - 1;
        if (m_OzgPosePick < 0)
            m_OzgPosePick = 0;
        OzgBuildPoses();
        OzgPaintCard();
        OzgPaintDirty();
    }

    // Whether the poses differ from the file's, in anything or in their order.
    private bool OzgPosesChanged()
    {
        if (!m_OzgCfg)
            return false;
        array<ref OZG_Pose> was = m_OzgCfg.PoseList;
        if (was.Count() != m_OzgPoses.Count())
            return true;
        OZG_Pose a;
        OZG_Pose b;
        for (int i = 0; i < was.Count(); i++)
        {
            a = was[i];
            b = m_OzgPoses[i];
            if (a.PoseId != b.PoseId || a.LabelKey != b.LabelKey)
                return true;
            if (a.Kind != b.Kind || a.Stance != b.Stance)
                return true;
            if (a.Gesture != b.Gesture)
                return true;
        }
        return false;
    }

    // ================================================= saving

    // What differs from the file, on all three pages, in the order of the pages.
    // withValues: "СТІЛ: між місцями, м 0.7 -> 0.8".
    private void OzgChanged(array<string> items, bool withValues)
    {
        items.Clear();
        if (!m_OzgCfg)
            return;

        string group;
        string item;
        string was;
        string now;
        string onWord = Widget.TranslateString("#STR_OZG_VPP_ON");
        string offWord = Widget.TranslateString("#STR_OZG_VPP_OFF");

        // Налаштування: the games, the boxes, the log.
        array<string> ids = new array<string>();
        OzgGameOrder(ids);
        group = Widget.TranslateString("#STR_OZG_VPP_G_GAMES");
        int i;
        bool fileOn;
        bool formOn;
        for (i = 0; i < ids.Count(); i++)
        {
            fileOn = m_OzgCfg.GameOn(ids[i]);
            formOn = m_OzgGamesOn.Find(ids[i]) >= 0;
            if (fileOn == formOn)
                continue;
            item = group + ": " + OzgGameName(ids[i]);
            if (withValues)
                item = item + " " + OzgOnOff(fileOn, onWord, offWord) + " -> " + OzgOnOff(formOn, onWord, offWord);
            items.Insert(item);
        }
        for (i = 0; i < 9; i++)
            OzgChangedBox(i, items, withValues);
        fileOn = m_OzgCfg.DebugLog == 1;
        if (fileOn != m_OzgDebugOn)
        {
            group = Widget.TranslateString("#STR_OZG_VPP_G_LOG");
            item = group + ": " + Widget.TranslateString("#STR_OZG_VPP_DEBUG");
            if (withValues)
                item = item + " " + OzgOnOff(fileOn, onWord, offWord) + " -> " + OzgOnOff(m_OzgDebugOn, onWord, offWord);
            items.Insert(item);
        }

        // Шулерство: the switch, then its boxes.
        fileOn = m_OzgCfg.Cheating == 1;
        if (fileOn != m_OzgCheatOn)
        {
            group = Widget.TranslateString("#STR_OZG_VPP_G_SHARP");
            item = group + ": " + Widget.TranslateString("#STR_OZG_VPP_CHEAT");
            if (withValues)
                item = item + " " + OzgOnOff(fileOn, onWord, offWord) + " -> " + OzgOnOff(m_OzgCheatOn, onWord, offWord);
            items.Insert(item);
        }
        for (i = 9; i < m_OzgKeys.Count(); i++)
            OzgChangedBox(i, items, withValues);

        // Пози.
        if (OzgPosesChanged())
        {
            group = Widget.TranslateString("#STR_OZG_VPP_PAGE_POSES");
            now = Widget.TranslateString("#STR_OZG_VPP_LIST_CHANGED");
            item = group + ": " + now;
            if (withValues)
            {
                was = m_OzgCfg.PoseList.Count().ToString();
                now = m_OzgPoses.Count().ToString();
                item = item + " " + was + " -> " + now;
            }
            items.Insert(item);
        }
    }

    private void OzgChangedBox(int i, array<string> items, bool withValues)
    {
        if (!OzgFieldChanged(i))
            return;
        string item = OzgBoxName(i);
        if (withValues)
        {
            string was = OzgText(m_OzgCfg, i);
            string now = OzgTyped(i);
            item = item + " " + was + " -> " + now;
        }
        items.Insert(item);
    }

    // "СТІЛ: між місцями, м". The two halves are worded one at a time: a translation
    // left of a `+` whose right side translates again loses its words.
    private string OzgBoxName(int i)
    {
        string group = Widget.TranslateString(m_OzgGroups[i]);
        string label = Widget.TranslateString(m_OzgLabels[i]);
        return group + ": " + label;
    }

    private string OzgOnOff(bool lit, string onWord, string offWord)
    {
        if (lit)
            return onWord;
        return offWord;
    }

    // The items one after another; `most` > 0 names that many and counts the rest.
    private string OzgList(array<string> items, int most)
    {
        string list = "";
        int named = items.Count();
        if (most > 0 && named > most)
            named = most;
        for (int i = 0; i < named; i++)
        {
            if (list != "")
                list = list + "; ";
            list = list + items[i];
        }
        int rest = items.Count() - named;
        if (rest > 0)
            list = list + "; +" + rest.ToString();
        return list;
    }

    // The bar of every page: what is not saved, on any of them.
    private void OzgPaintDirty()
    {
        int cMuted = ARGB(255, 160, 159, 153);
        int cAlert = ARGB(255, 245, 199, 64);
        array<string> items = new array<string>();
        OzgChanged(items, false);
        string line = Widget.TranslateString("#STR_OZG_VPP_NO_CHANGES");
        int ink = cMuted;
        if (items.Count() > 0)
        {
            string unsaved = Widget.TranslateString("#STR_OZG_VPP_UNSAVED");
            string list = OzgList(items, 4);
            line = unsaved + " " + list;
            ink = cAlert;
        }
        OzgSay("OzgSetDirty", line, ink);
        OzgSay("OzgShDirty", line, ink);
        OzgSay("OzgPoDirty", line, ink);
    }

    // "Зберегти": what is wrong is named and nothing is sent; otherwise the strip asks.
    private void OzgAskSave()
    {
        if (!m_OzgCfg)
        {
            Hint("#STR_OZG_VPP_H_NOT_LOADED");
            return;
        }
        array<string> items = new array<string>();
        OzgChanged(items, false);
        if (items.Count() == 0)
        {
            Hint("#STR_OZG_VPP_NO_CHANGES");
            return;
        }

        // Every box, not only the changed ones: a rule between two boxes (who sees a
        // game, who plays it) can break by the other box.
        string bad;
        string name;
        for (int i = 0; i < m_OzgKeys.Count(); i++)
        {
            bad = OzgFieldBad(i);
            if (bad == "")
                continue;
            name = OzgBoxName(i);
            Hint(name + " — " + bad);
            return;
        }

        string why = OzgPosesBad();
        if (why != "")
        {
            Hint(why);
            return;
        }

        string ask = Widget.TranslateString("#STR_OZG_VPP_ASK_SAVE");
        OzgChanged(items, true);
        string list = OzgList(items, 0);
        ConfirmApply(ask + " " + list, "#STR_OZG_VPP_SAVE", "ozg:save");
    }

    // What the server would drop from the poses without a word (OZG_Settings.Validate
    // skips a pose with no id), and what it would keep but no player could tell apart.
    private string OzgPosesBad()
    {
        array<string> seen = new array<string>();
        OZG_Pose p;
        string fmt;
        for (int i = 0; i < m_OzgPoses.Count(); i++)
        {
            p = m_OzgPoses[i];
            if (p.PoseId == "")
            {
                fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_ID_EMPTY");
                return string.Format(fmt, OzgPoseName(p));
            }
            if (seen.Find(p.PoseId) >= 0)
            {
                fmt = Widget.TranslateString("#STR_OZG_VPP_CHK_ID_TWICE");
                return string.Format(fmt, p.PoseId);
            }
            seen.Insert(p.PoseId);
        }
        string gesture = GetEdit("OzgPoF_Gesture");
        gesture = gesture.Trim();
        if (OzgPicked() && !OzgIsNumber(gesture, false))
        {
            string label = Widget.TranslateString("#STR_OZG_VPP_PF_GESTURE");
            string must = Widget.TranslateString("#STR_OZG_VPP_H_WHOLE");
            return label + " — " + must;
        }
        return "";
    }

    private void OzgAskDrop()
    {
        array<string> items = new array<string>();
        OzgChanged(items, false);
        if (items.Count() == 0)
        {
            Hint("#STR_OZG_VPP_NO_CHANGES");
            return;
        }
        string ask = Widget.TranslateString("#STR_OZG_VPP_ASK_DROP");
        OzgChanged(items, true);
        string list = OzgList(items, 0);
        Confirm(ask + " " + list, "#STR_OZG_VPP_CANCEL", "ozg:drop");
    }

    // The strip's "yes": the form as it is now goes into the LOADED object -- Version
    // and every field the form does not show survive the save untouched -- and the file
    // goes to the server whole (cfg_set). The answer reads it back (OnCfgApplied).
    private void OzgSaveNow()
    {
        if (!m_OzgCfg)
        {
            Hint("#STR_OZG_VPP_H_NOT_LOADED");
            return;
        }

        string typed;
        for (int i = 0; i < m_OzgKeys.Count(); i++)
        {
            typed = OzgTyped(i);
            if (OzgIsNumber(typed, m_OzgFloats[i] == 1))
                OzgSet(m_OzgCfg, m_OzgKeys[i], typed);
        }

        array<string> games = new array<string>();
        OzgGamesInto(games);
        m_OzgCfg.GameIds = games;

        int logOn = 0;
        if (m_OzgDebugOn)
            logOn = 1;
        m_OzgCfg.DebugLog = logOn;
        int cheat = 0;
        if (m_OzgCheatOn)
            cheat = 1;
        m_OzgCfg.Cheating = cheat;

        array<ref OZG_Pose> poses = new array<ref OZG_Pose>();
        for (int p = 0; p < m_OzgPoses.Count(); p++)
            poses.Insert(m_OzgPoses[p].Copy());
        m_OzgCfg.PoseList = poses;

        string body;
        string err;
        if (!JsonFileLoader<OZG_Settings>.MakeData(m_OzgCfg, body, err, false))
        {
            Hint("#STR_OZG_VPP_H_NO_JSON");
            return;
        }
        SendCfg(OZG_Cfg.ADMIN_NAME, body);
        OzgPaintAll();
    }

    // "Відкрити як файл": the module's Файли конфігів with this file picked.
    private void OzgOpenFile()
    {
        string page = OZG_MOD + ".files";
        if (m_Panes.Contains(page))
            ShowPane(page);
        else if (m_Panes.Contains("raw"))
            ShowPane("raw");
        else
            return;
        RawPick(OZG_Cfg.ADMIN_NAME);
    }

    // ================================================= pieces

    // A text of the window by its name: its words (a key is translated by SetText) and
    // its colour.
    private void OzgSay(string name, string text, int ink)
    {
        if (!M_SUB_WIDGET)
            return;
        TextWidget t = TextWidget.Cast(M_SUB_WIDGET.FindAnyWidget(name));
        if (!t)
            return;
        t.SetText(text);
        t.SetColor(ink);
    }

    // A text of a list's row.
    private void OzgRowSay(Widget row, string name, string text, int ink)
    {
        TextWidget t = TextWidget.Cast(row.FindAnyWidget(name));
        if (!t)
            return;
        t.SetText(text);
        t.SetColor(ink);
    }

    private void OzgPaintIn(Widget root, string name, int color)
    {
        if (root)
            OZ_VppFlex.Paint(root, name, color);
    }

    // ================================================= input

    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        if (w && M_SUB_WIDGET && !m_OzgFill)
        {
            string nm = w.GetName();
            if (nm.IndexOf("Ozg_") == 0)
            {
                OzgPaintNumbers();
                OzgPaintNotice();
                OzgPaintLoads();
                OzgPaintDirty();
                return true;
            }
            if (nm.IndexOf("OzgPoF_") == 0)
            {
                OzgPoseEdited(nm);
                return true;
            }
        }
        return super.OnChange(w, x, y, finished);
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w || !M_SUB_WIDGET)
            return super.OnClick(w, x, y, button);

        string nm = w.GetName();

        if (nm == "BtnOzgSetSave" || nm == "BtnOzgShSave" || nm == "BtnOzgPoSave")
        {
            OzgAskSave();
            return true;
        }
        if (nm == "BtnOzgSetDrop" || nm == "BtnOzgShDrop" || nm == "BtnOzgPoDrop")
        {
            OzgAskDrop();
            return true;
        }
        if (nm == "BtnOzgSetOpen" || nm == "BtnOzgShOpen" || nm == "BtnOzgPoOpen")
        {
            OzgOpenFile();
            return true;
        }
        if (nm == "BtnOzgDebug" || nm == "OzgDebugTick")
        {
            bool debugOn = !m_OzgDebugOn;
            m_OzgDebugOn = debugOn;
            OzgPaintSwitches();
            OzgPaintDirty();
            return true;
        }
        if (nm == "BtnOzgCheat" || nm == "OzgCheatTick")
        {
            bool cheatOn = !m_OzgCheatOn;
            m_OzgCheatOn = cheatOn;
            OzgPaintSwitches();
            OzgPaintDirty();
            return true;
        }
        if (nm == "BtnOzgPoNew")
        {
            OzgPoseNew();
            return true;
        }
        if (nm == "BtnOzgPoUp")
        {
            OzgPoseMove(-1);
            return true;
        }
        if (nm == "BtnOzgPoDown")
        {
            OzgPoseMove(1);
            return true;
        }
        if (nm == "BtnOzgPoDel")
        {
            OzgPoseDelete();
            return true;
        }
        if (nm == "OzgPoF_Kind")
        {
            OzgPoseKind();
            return true;
        }
        if (nm == "OzgPoF_Stance")
        {
            OzgPoseStance();
            return true;
        }

        // A row of the games: its tick.
        if (m_OzgGameRows)
        {
            int game = m_OzgGameRows.IndexOf(w);
            if (game != -1)
            {
                OzgToggleGame(m_OzgGameRows.Key(game));
                return true;
            }
        }
        // A row of the poses: its pose on the card.
        if (m_OzgPoseRows)
        {
            int pose = m_OzgPoseRows.IndexOf(w);
            if (pose != -1)
            {
                OzgPickPose(pose);
                return true;
            }
        }

        return super.OnClick(w, x, y, button);
    }
}

#endif
#endif
#endif
