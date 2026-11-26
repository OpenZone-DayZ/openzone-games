// The window of a game: one for players and watchers.
//
// It is the table of the design canvas of 2026-11-24 in the look of the deck
// the game was laid out from (OZG_TableLook: classic or stalker). The other
// seats sit round the table as fans of backs, to be counted by eye, every
// back in its own look; the stock lies with the trump turned under it, the
// discards as a heap, the bout as attack cards with the cards that beat them
// laid over; the player's hand is fanned along an arc. Right: the log, and
// what the player and the host may do. A watcher sees the same without a hand
// and without buttons, from the side of the first seat.
//
// The window shows what the server last said (OZG_Table) and asks for
// everything through the service: it decides nothing. A card is played with a
// click; when a click could mean more than one move the card is picked up
// first, and the next click says which. Cards are turned pictures that take
// no pointer: the cloth under them is one button (OZG_Hit), and the window
// finds the card under the pointer itself, turn and all.
//
// The table drawn here is durak's. Another game brings its own drawing of the
// two halves of a view; the frame around it -- seats, log, host -- is the
// same.
class OZG_GameMenu : UIScriptedMenu
{
    protected static OZG_GameMenu s_Current;
    protected static string s_ForId;
    // Layout paths the engine mixed up once this session: they are asked for
    // with backslashes from then on (MakeLayout).
    protected static ref array<string> s_Reslashed;

    protected static const string LAYOUT = "OpenZone_Games/gui/layouts/ozg_game.layout";
    protected static const string LAYOUT_CARD = "OpenZone_Games/gui/layouts/ozg_card.layout";
    // A turned picture is cut to its own unturned box (measured 2026-11-25:
    // the trump under the stock showed only as wide as it stood, the fanned
    // cards lost their corners). A card is therefore a square frame CARD_BOX
    // diagonals of the card wide, turned whole, its shadow, glow and face
    // inside it (tools/make_layouts.py, card()).
    protected static const float CARD_BOX = 1.1;
    protected static const float PLATE_PAD = 4;

    // The key the trump's card is drawn under (SlotOfFan and the rest below),
    // and the parts of the table a shake moves (ShakeRegion): the stock with
    // the trump under it (the card sharp's design calls it the deck), the
    // discards, a seat's fan (RegionOfSeat).
    static const string SLOT_TRUMP = "t";
    static const string REGION_STOCK = "stock";
    static const string REGION_DISCARDS = "discards";

    protected string m_PlaceId;
    protected string m_Style;
    protected ref OZG_TableLook m_Look;
    protected bool m_Claimed;
    protected bool m_Closing;
    protected int m_ShownRev;
    protected float m_K;
    protected float m_SaidLeft;
    protected float m_AskWait;
    protected float m_Recheck;

    protected Widget m_Win;
    protected Widget m_TableBox;
    protected Widget m_SeatsBox;
    protected Widget m_Hit;
    protected Widget m_MyPlate;
    protected Widget m_MyBg;
    protected Widget m_MyEdge;
    protected TextWidget m_MyName;
    protected TextWidget m_MyRole;
    protected TextWidget m_Title;
    protected TextWidget m_Sub;
    protected TextWidget m_Status;
    protected TextWidget m_Said;
    protected TextWidget m_Hint;
    protected TextWidget m_HostLine;
    protected TextWidget m_Result;
    protected TextWidget m_LogTitle;
    protected TextWidget m_StockLabel;
    protected TextWidget m_DiscardLabel;
    protected ref array<TextWidget> m_LogLines;

    protected Widget m_BtnClose;
    protected Widget m_BtnPass;
    protected Widget m_BtnTake;
    protected Widget m_BtnMove;
    protected Widget m_BtnVariant;
    protected Widget m_BtnDeal;
    protected Widget m_BtnTakeHost;
    protected Widget m_BtnKickDiscard;
    protected Widget m_BtnKickLeave;
    protected Widget m_BtnDropLeft;
    protected Widget m_BtnPose;
    protected Widget m_BtnStand;
    protected Widget m_BtnStandDiscard;
    protected Widget m_BtnStandLeave;
    protected Widget m_BtnFold;
    protected Widget m_BtnFoldYes;

    // What the script laid on the table, by the part that paints it.
    protected ref array<Widget> m_FanPics;
    protected ref array<Widget> m_TablePics;
    protected ref array<Widget> m_HandPics;
    protected ref array<Widget> m_SeatW;
    protected ref array<int> m_SeatOf;
    // The hand as drawn, for the pointer: each card's frame, where it rests
    // and how it is turned.
    protected ref array<Widget> m_HandW;
    protected ref array<string> m_HandCodes;
    protected ref array<float> m_HandX;
    protected ref array<float> m_HandY;
    protected ref array<float> m_HandA;
    protected int m_Hover;
    // The bout's attack cards as drawn: middle and size.
    protected ref array<float> m_AtkX;
    protected ref array<float> m_AtkY;
    protected ref array<float> m_AtkW;
    protected ref array<float> m_AtkH;

    // Every card drawn with a key or a region (Card), for the flights and the
    // shakes: its frame, the key a flight lands on it by, the region a shake
    // moves it with, and where it rests, in pixels of its parent.
    protected ref array<Widget> m_DrawnW;
    protected ref array<string> m_DrawnKey;
    protected ref array<string> m_DrawnRegion;
    protected ref array<float> m_DrawnX;
    protected ref array<float> m_DrawnY;
    // The keys whose card waits for a flight to land, drawn hidden until then:
    // a key is here once for every flight on its way to it.
    protected ref array<string> m_Hidden;
    // The layer the flying cards are made in (OZG_Air), and what moves them.
    protected Widget m_Air;
    protected ref OZG_Flights m_Flights;
    protected ref OZG_Shakes m_Shakes;

    // What the server last said.
    protected ref OZG_WView m_V;
    protected ref OZG_DurakPublic m_Pub;
    protected ref OZG_DurakPrivate m_Own;
    // The later games' halves of a view (stakes-and-games design §4).
    protected ref OZG_GPublic m_GPub;
    protected ref OZG_GPrivate m_GOwn;
    // What the script lays for them: the table's buttons and the move each
    // makes, the table's lines of text, the amount's box and what it was set
    // for; the column's buttons, lines and the chips' box; the faces of other
    // seats this player may click (the dummy).
    protected ref array<Widget> m_GenBtns;
    protected ref array<string> m_GenBtnMoves;
    protected ref array<Widget> m_GenTexts;
    protected Widget m_GenEdit;
    protected string m_GenEditMove;
    protected ref array<Widget> m_GenColBtns;
    protected ref array<string> m_GenColMoves;
    protected ref array<Widget> m_GenColTexts;
    protected Widget m_StakeEdit;
    protected ref array<string> m_GenFaceCodes;
    protected ref array<float> m_GenFaceX;
    protected ref array<float> m_GenFaceY;
    protected ref array<float> m_GenFaceW;
    protected ref array<float> m_GenFaceH;
    protected ref array<float> m_GenFaceA;

    // What the player is in the middle of.
    protected string m_Picked;
    protected int m_PickedSeat;
    protected bool m_AskStand;
    protected bool m_AskFold;

    // ---- the card sharp's phase 4 (design §6, §10) ---------------------------
    // The pockets' strip, the panel of sleights, the dealer's frame.
    protected Widget m_Pockets;
    protected Widget m_PocketsEdge;
    protected Widget m_PocketsEdgeHot;
    protected TextWidget m_PocketsNone;
    protected Widget m_SleightRule;
    protected Widget m_SleightRule2;
    protected TextWidget m_SleightTitle;
    protected TextWidget m_SleightCount;
    protected ref array<TextWidget> m_SleightHint;
    protected Widget m_BtnSlip;
    protected Widget m_BtnSwap;
    protected Widget m_BtnToss;
    protected Widget m_Deal;
    protected TextWidget m_DealTitle;
    protected TextWidget m_DealCount;
    protected TextWidget m_DealCaption;
    protected ref array<TextWidget> m_DealHint;
    protected Widget m_BtnDeckIn;
    protected Widget m_BtnDeckOut;
    // The right column's rule over the host's line, and whether the panel of
    // sleights shows under the log now (PaintColumn decides it).
    protected Widget m_ColRule;
    protected bool m_PanelShown;
    // What the script lays: the pockets' cards; the deck, the cards the
    // dealer reads and their ticks.
    protected ref array<Widget> m_PocketPics;
    protected ref array<Widget> m_DealPics;
    // The pockets as last read, and as drawn: each card's middle.
    protected ref array<ref OZG_PocketCard> m_PocketCards;
    protected ref array<ref OZG_PocketCard> m_PocketShown;
    protected ref array<float> m_PocketX;
    protected float m_PocketsAge;
    protected string m_PocketsLine;
    // The dealer's tiles as drawn: which read card, where.
    protected ref array<string> m_TileKeys;
    protected ref array<float> m_TileX;
    protected ref array<float> m_TileY;
    // The sleight armed: "" or one of OZG_Const.SLEIGHT_SLIP, _SWAP, _THROW,
    // _DECK_IN, _DECK_OUT; the card of the pockets a swap picked; the cards
    // one throw has sent.
    protected string m_Sleight;
    protected string m_PickKey;
    protected int m_Thrown;
    // The dealer's three as planned, sent with "Deal" (OZG_WAsk.Stack): keys
    // of pocket cards put in, of deck cards out (or OZG_Const.ANY_CARD), of
    // deck cards chosen; beside the cards out, their faces ("" by chance) and
    // looks, for the strip.
    protected ref array<string> m_PlanIn;
    protected ref array<string> m_PlanOut;
    protected ref array<string> m_PlanOutFace;
    protected ref array<int> m_PlanOutLook;
    protected ref array<string> m_PlanPick;
    // This player's sleights asked for and not yet answered, oldest first,
    // and an answer that came before the view it speaks of.
    protected ref array<ref OZG_SleightFly> m_Flying;
    protected ref OZG_WSleight m_Answer;

    void OZG_GameMenu()
    {
        m_GenBtns = new array<Widget>();
        m_GenBtnMoves = new array<string>();
        m_GenTexts = new array<Widget>();
        m_GenColBtns = new array<Widget>();
        m_GenColMoves = new array<string>();
        m_GenColTexts = new array<Widget>();
        m_GenFaceCodes = new array<string>();
        m_GenFaceX = new array<float>();
        m_GenFaceY = new array<float>();
        m_GenFaceW = new array<float>();
        m_GenFaceH = new array<float>();
        m_GenFaceA = new array<float>();
        m_SleightHint = new array<TextWidget>();
        m_DealHint = new array<TextWidget>();
        m_PocketPics = new array<Widget>();
        m_DealPics = new array<Widget>();
        m_PocketCards = new array<ref OZG_PocketCard>();
        m_PocketShown = new array<ref OZG_PocketCard>();
        m_PocketX = new array<float>();
        m_TileKeys = new array<string>();
        m_TileX = new array<float>();
        m_TileY = new array<float>();
        m_PlanIn = new array<string>();
        m_PlanOut = new array<string>();
        m_PlanOutFace = new array<string>();
        m_PlanOutLook = new array<int>();
        m_PlanPick = new array<string>();
        m_Flying = new array<ref OZG_SleightFly>();
        m_ShownRev = -1;
        m_K = 1;
        m_PickedSeat = -1;
        m_Hover = -1;
        m_LogLines = new array<TextWidget>();
        m_FanPics = new array<Widget>();
        m_TablePics = new array<Widget>();
        m_HandPics = new array<Widget>();
        m_SeatW = new array<Widget>();
        m_SeatOf = new array<int>();
        m_HandW = new array<Widget>();
        m_HandCodes = new array<string>();
        m_HandX = new array<float>();
        m_HandY = new array<float>();
        m_HandA = new array<float>();
        m_AtkX = new array<float>();
        m_AtkY = new array<float>();
        m_AtkW = new array<float>();
        m_AtkH = new array<float>();
        m_DrawnW = new array<Widget>();
        m_DrawnKey = new array<string>();
        m_DrawnRegion = new array<string>();
        m_DrawnX = new array<float>();
        m_DrawnY = new array<float>();
        m_Hidden = new array<string>();
        m_Flights = new OZG_Flights();
        m_Shakes = new OZG_Shakes();
        if (!s_Reslashed)
            s_Reslashed = new array<string>();
    }

    void ~OZG_GameMenu()
    {
        if (s_Current == this)
            s_Current = null;
    }

    static OZG_GameMenu Current()
    {
        return s_Current;
    }

    // The place the next window opens on.
    static void For(string placeId)
    {
        s_ForId = placeId;
    }

    string PlaceId()
    {
        return m_PlaceId;
    }

    // ------------------------------------------------------------ open and close

    // A layout's widgets under `parent`, checked by the root's name: the engine
    // can hand back a layout made earlier in the session for another path, and
    // asked again with backslashes it gives the right one (dayz-modding,
    // gui-layouts §32). A path that was mixed up once is asked for with
    // backslashes from then on.
    protected Widget MakeLayout(string path, Widget parent, string root)
    {
        string asked = path;
        if (s_Reslashed.Find(path) >= 0)
            asked.Replace("/", "\\");
        Widget w = GetGame().GetWorkspace().CreateWidgets(asked, parent);
        if (w && w.GetName() != root && asked == path)
        {
            OZG_Log.Warn("layout " + path + " came back as " + w.GetName() + "; asked again with backslashes");
            w.Unlink();
            s_Reslashed.Insert(path);
            asked.Replace("/", "\\");
            w = GetGame().GetWorkspace().CreateWidgets(asked, parent);
        }
        if (!w)
            OZG_Log.Error("the game window could not load " + path);
        return w;
    }

    protected Widget Find(string name)
    {
        Widget w = m_Win.FindAnyWidget(name);
        if (!w)
            OZG_Log.Error("the game window has no widget " + name);
        return w;
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT);
        if (!layoutRoot)
        {
            OZG_Log.Error("the game window's layout did not load");
            return null;
        }
        m_PlaceId = s_ForId;
        Build(StyleNow());
        return layoutRoot;
    }

    // The look of the place's deck: from the last view of it, or from the deck
    // that lies in the place while no view has come yet.
    protected string StyleNow()
    {
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (view && view.HeadPart && view.HeadPart.Style != "")
            return view.HeadPart.Style;
        array<OZG_Place> places = OZG_Place.All();
        for (int i = 0; i < places.Count(); i++)
        {
            OZG_Place place = places[i];
            if (!place || place.OZG_Id() != m_PlaceId)
                continue;
            OZG_Deck_Base deck = place.OZG_Deck();
            if (deck)
                return OZG_Decks.StyleOf(deck.GetType());
        }
        return "classic";
    }

    // The window in one look; a look already shown is put away first.
    protected void Build(string style)
    {
        if (style != "stalker")
            style = "classic";
        // Nothing of the old look flies or shakes on: its cards go with it.
        LandAll();
        m_Shakes.Clear();
        ClearPictures();
        ClearList(m_SeatW);
        m_SeatOf.Clear();
        m_LogLines.Clear();
        if (m_Win)
            m_Win.Unlink();
        m_Win = null;
        GenForget();
        m_Style = style;
        m_Look = OZG_TableLook.Of(style);

        string root = "OZG_WinClassic";
        if (style == "stalker")
            root = "OZG_WinStalker";
        m_Win = MakeLayout("OpenZone_Games/gui/layouts/ozg_win_" + style + ".layout", layoutRoot, root);
        if (!m_Win)
            return;

        m_TableBox = Find("OZG_Table");
        m_SeatsBox = Find("OZG_Seats");
        m_Hit = Find("OZG_Hit");
        m_MyPlate = Find("OZG_MyPlate");
        m_MyEdge = Find("OZG_MyEdge");
        m_MyBg = Find("OZG_MyBg");
        m_MyName = TextWidget.Cast(Find("OZG_MyName"));
        m_MyRole = TextWidget.Cast(Find("OZG_MyRole"));
        m_Title = TextWidget.Cast(Find("OZG_Title"));
        m_Sub = TextWidget.Cast(Find("OZG_Sub"));
        m_Status = TextWidget.Cast(Find("OZG_Status"));
        m_Said = TextWidget.Cast(Find("OZG_Said"));
        m_Hint = TextWidget.Cast(Find("OZG_Hint"));
        m_HostLine = TextWidget.Cast(Find("OZG_HostLine"));
        m_Result = TextWidget.Cast(Find("OZG_Result"));
        m_LogTitle = TextWidget.Cast(Find("OZG_LogTitle"));
        m_StockLabel = TextWidget.Cast(Find("OZG_StockLabel"));
        m_DiscardLabel = TextWidget.Cast(Find("OZG_DiscardLabel"));
        m_Air = Find("OZG_Air");
        // As many log lines as the look has.
        for (int i = 0; i < 40; i++)
        {
            TextWidget line = TextWidget.Cast(m_Win.FindAnyWidget("OZG_Log" + i.ToString()));
            if (!line)
                break;
            m_LogLines.Insert(line);
        }

        m_BtnClose = Find("OZG_BtnClose");
        m_BtnPass = Find("OZG_BtnPass");
        m_BtnTake = Find("OZG_BtnTake");
        m_BtnMove = Find("OZG_BtnMove");
        m_BtnVariant = Find("OZG_BtnVariant");
        m_BtnDeal = Find("OZG_BtnDeal");
        m_BtnTakeHost = Find("OZG_BtnTakeHost");
        m_BtnKickDiscard = Find("OZG_BtnKickDiscard");
        m_BtnKickLeave = Find("OZG_BtnKickLeave");
        m_BtnDropLeft = Find("OZG_BtnDropLeft");
        m_BtnPose = Find("OZG_BtnPose");
        m_BtnStand = Find("OZG_BtnStand");
        m_BtnStandDiscard = Find("OZG_BtnStandDiscard");
        m_BtnStandLeave = Find("OZG_BtnStandLeave");
        m_BtnFold = Find("OZG_BtnFold");
        m_BtnFoldYes = Find("OZG_BtnFoldYes");

        m_Pockets = Find("OZG_Pockets");
        m_PocketsEdge = Find("OZG_PocketsEdge");
        m_PocketsEdgeHot = Find("OZG_PocketsEdgeHot");
        m_PocketsNone = TextWidget.Cast(Find("OZG_PocketsNone"));
        m_SleightRule = Find("OZG_SleightRule");
        m_SleightRule2 = Find("OZG_SleightRule2");
        m_SleightTitle = TextWidget.Cast(Find("OZG_SleightTitle"));
        m_SleightCount = TextWidget.Cast(Find("OZG_SleightCount"));
        m_SleightHint.Clear();
        m_DealHint.Clear();
        for (int h = 0; h < 2; h++)
        {
            m_SleightHint.Insert(TextWidget.Cast(Find("OZG_SleightHint" + h.ToString())));
            m_DealHint.Insert(TextWidget.Cast(Find("OZG_DealHint" + h.ToString())));
        }
        m_BtnSlip = Find("OZG_BtnSlip");
        m_BtnSwap = Find("OZG_BtnSwap");
        m_BtnToss = Find("OZG_BtnToss");
        m_Deal = Find("OZG_Deal");
        m_DealTitle = TextWidget.Cast(Find("OZG_DealTitle"));
        m_DealCount = TextWidget.Cast(Find("OZG_DealCount"));
        m_DealCaption = TextWidget.Cast(Find("OZG_DealCaption"));
        m_BtnDeckIn = Find("OZG_BtnDeckIn");
        m_BtnDeckOut = Find("OZG_BtnDeckOut");
        m_ColRule = Find("OZG_ColRule");
        m_Sleight = "";
        m_PickKey = "";
        m_Thrown = 0;

        // The stalkers' table writes its labels in capitals: what the layout
        // wrote as a key is written again from the script.
        if (m_Look.Caps)
        {
            SetCaps(m_LogTitle, "STR_OZG_WIN_LOG");
            SetCaps(m_StockLabel, "STR_OZG_WIN_STOCK_LABEL");
            SetCaps(m_DiscardLabel, "STR_OZG_WIN_DISCARD_LABEL");
            SetLabel(m_BtnTake, Caps(Tr("STR_OZG_WIN_TAKE")));
            SetLabel(m_BtnPass, Caps(Tr("STR_OZG_WIN_PASS")));
            SetLabel(m_BtnMove, Caps(Tr("STR_OZG_WIN_MOVE_ON")));
            SetLabel(m_BtnDeal, Caps(Tr("STR_OZG_WIN_DEAL")));
            SetLabel(m_BtnTakeHost, Caps(Tr("STR_OZG_WIN_TAKE_HOST")));
            SetLabel(m_BtnKickDiscard, Caps(Tr("STR_OZG_WIN_KICK_DISCARD")));
            SetLabel(m_BtnKickLeave, Caps(Tr("STR_OZG_WIN_KICK_LEAVE")));
            SetLabel(m_BtnDropLeft, Caps(Tr("STR_OZG_WIN_DROP_LEFT")));
            SetLabel(m_BtnStand, Caps(Tr("STR_OZG_WIN_STAND")));
            SetLabel(m_BtnStandDiscard, Caps(Tr("STR_OZG_WIN_STAND_DISCARD")));
            SetLabel(m_BtnStandLeave, Caps(Tr("STR_OZG_WIN_STAND_LEAVE")));
            SetLabel(m_BtnFold, Caps(Tr("STR_OZG_WIN_FOLD")));
            SetLabel(m_BtnFoldYes, Caps(Tr("STR_OZG_WIN_FOLD_YES")));
            SetCaps(TextWidget.Cast(Find("OZG_PocketsTitle")), "STR_OZG_WIN_POCKETS");
            SetCaps(m_SleightTitle, "STR_OZG_WIN_SLEIGHTS");
            SetCaps(m_DealTitle, "STR_OZG_WIN_DEALING");
            SetLabel(m_BtnSlip, Caps(Tr("STR_OZG_WIN_SLIP")));
            SetLabel(m_BtnSwap, Caps(Tr("STR_OZG_WIN_SWAP")));
            SetLabel(m_BtnDeckIn, Caps(Tr("STR_OZG_WIN_DECK_IN")));
            SetLabel(m_BtnDeckOut, Caps(Tr("STR_OZG_WIN_DECK_OUT")));
        }
        Measure();
    }

    // What the window shuts out while it is open: the game's "menu" group
    // (bin\specific.xml) less its voice -- the inventory's group, the hotbar,
    // walking and the inventory key -- so push-to-talk, its toggle and the
    // voice level still work and the players talk while they play (the
    // owner's word, 2026-11-25). The game takes single inputs here as well as
    // groups (its script console does the same). A mod's own push-to-talk is
    // in none of them.
    protected static array<string> ShutOut()
    {
        array<string> groups = new array<string>();
        groups.Insert("inventory");
        groups.Insert("hotkey");
        groups.Insert("movement");
        groups.Insert("UAGear");
        return groups;
    }

    override void OnShow()
    {
        // The base locks the controls and shows the cursor; the excludes stop
        // the body from walking while the window is open.
        super.OnShow();
        if (!layoutRoot || !m_Win)
        {
            GetGame().GetUIManager().CloseMenu(OZG_Const.MENU_GAME);
            return;
        }
        s_Current = this;
        m_Claimed = true;
        // Every step through a local: at a death the mission or its HUD is
        // already gone when the engine hides the menu.
        Mission mission = GetGame().GetMission();
        if (mission)
        {
            mission.AddActiveInputExcludes(ShutOut());
            Hud hud = mission.GetHud();
            if (hud)
            {
                hud.ShowHudUI(false);
                hud.ShowQuickbarUI(false);
            }
        }
        Measure();
        Redraw();
        // The view may be older than the table: ask once on opening.
        Send(OZG_Const.OP_OPEN, "", -1, "", "", "", 0);
    }

    override void OnHide()
    {
        // Seated or not: the server keeps who has the window open (a sleight
        // on a board is told to them at once).
        if (m_V)
            Send(OZG_Const.OP_UNWATCH, "", -1, "", "", "", 0);
        // No card stays in the air, and no part of the table shaken.
        LandAll();
        m_Shakes.Clear();
        ClearPictures();
        ClearList(m_SeatW);
        m_SeatOf.Clear();
        if (m_Claimed)
        {
            m_Claimed = false;
            Mission mission = GetGame().GetMission();
            if (mission)
            {
                mission.RemoveActiveInputExcludes(ShutOut(), true);
                Hud hud = mission.GetHud();
                if (hud)
                {
                    hud.ShowHudUI(true);
                    hud.ShowQuickbarUI(true);
                }
            }
        }
        if (s_Current == this)
            s_Current = null;
        super.OnHide();
    }

    void Shut()
    {
        if (m_Closing)
            return;
        m_Closing = true;
        Close();
    }

    // The server took the place away from this client, and said why.
    void CloseBecause(string why)
    {
        if (why != "")
            NotificationSystem.AddNotificationExtended(4, "#STR_OZG_MODNAME", "#" + why, "");
        Shut();
    }

    // A place's pulse moved: a window open on an older revision asks again.
    void PulseOf(OZG_Place place)
    {
        if (!place || place.OZG_Id() != m_PlaceId)
            return;
        if (place.OZG_Rev() <= m_ShownRev || m_AskWait > 0)
            return;
        m_AskWait = 1.0;
        Send(OZG_Const.OP_OPEN, "", -1, "", "", "", 0);
    }

    void KeyPressed(int key)
    {
    }

    // What the server refused, or said.
    void Said(string text)
    {
        if (!m_Said)
            return;
        m_Said.SetText(text);
        m_SaidLeft = 6.0;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!layoutRoot || !m_Win || m_Closing)
            return;

        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
        {
            Shut();
            return;
        }

        if (m_AskWait > 0)
            m_AskWait = m_AskWait - timeslice;
        if (m_SaidLeft > 0)
        {
            m_SaidLeft = m_SaidLeft - timeslice;
            if (m_SaidLeft <= 0)
                m_Said.SetText("");
        }

        // Never over a player who is not there to play. (A hit closes the
        // window too: the mission hears it from the player's own hook --
        // health itself cannot be read on a client.)
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive() || player.IsUnconscious())
        {
            Shut();
            return;
        }

        TickAir(timeslice);
        TickShakes(timeslice);
        TrackHover();
        RefreshPockets(timeslice);

        // The pulse on the place says a newer revision than the one shown:
        // ask again. Looked at every two seconds, so a view that was lost, or
        // a pulse that came while an answer was on its way, is made up for.
        m_Recheck = m_Recheck + timeslice;
        if (m_Recheck >= 2.0)
        {
            m_Recheck = 0;
            array<OZG_Place> places = OZG_Place.All();
            for (int i = 0; i < places.Count(); i++)
            {
                OZG_Place place = places[i];
                if (!place || place.OZG_Id() != m_PlaceId)
                    continue;
                if (place.OZG_Rev() > m_ShownRev)
                    Send(OZG_Const.OP_OPEN, "", -1, "", "", "", 0);
                break;
            }
        }
    }

    // ------------------------------------------------------------ asking the server

    protected void Send(string op, string move, int seatNo, string how, string optId, string optValue, int poseNo)
    {
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = m_PlaceId;
        ask.Move = move;
        ask.SeatNo = seatNo;
        ask.How = how;
        ask.OptId = optId;
        ask.OptValue = optValue;
        ask.PoseNo = poseNo;
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, op, json);
    }

    protected void Play(string move)
    {
        m_Picked = "";
        Send(OZG_Const.OP_ACT, move, -1, "", "", "", 0);
    }

    // ------------------------------------------------------------ small things

    protected void ClearList(array<Widget> list)
    {
        for (int i = 0; i < list.Count(); i++)
        {
            if (list[i])
                list[i].Unlink();
        }
        list.Clear();
    }

    protected void ClearHandMarks()
    {
        m_HandW.Clear();
        m_HandCodes.Clear();
        m_HandX.Clear();
        m_HandY.Clear();
        m_HandA.Clear();
        m_Hover = -1;
    }

    protected void ClearPictures()
    {
        m_DrawnW.Clear();
        m_DrawnKey.Clear();
        m_DrawnRegion.Clear();
        m_DrawnX.Clear();
        m_DrawnY.Clear();
        ClearList(m_FanPics);
        ClearList(m_TablePics);
        ClearList(m_HandPics);
        ClearList(m_PocketPics);
        ClearList(m_DealPics);
        m_PocketShown.Clear();
        m_PocketX.Clear();
        m_TileKeys.Clear();
        m_TileX.Clear();
        m_TileY.Clear();
        ClearHandMarks();
        m_AtkX.Clear();
        m_AtkY.Clear();
        m_AtkW.Clear();
        m_AtkH.Clear();
    }

    // Real pixels in one layout unit, read off the window as it stands.
    protected void Measure()
    {
        m_K = 1;
        if (!m_Win)
            return;
        float w;
        float h;
        m_Win.GetScreenSize(w, h);
        float k = w / 1600.0;
        if (k > 0.2 && k < 6)
            m_K = k;
    }

    // Where the pointer is, in the window's layout units.
    protected void PointerAt(out float x, out float y)
    {
        int mx;
        int my;
        GetMousePos(mx, my);
        float wx;
        float wy;
        m_Win.GetScreenPos(wx, wy);
        x = (mx - wx) / m_K;
        y = (my - wy) / m_K;
    }

    protected string Tr(string key)
    {
        return Widget.TranslateString("#" + key);
    }

    protected string Tr1(string key, string value)
    {
        string s = Widget.TranslateString("#" + key);
        s.Replace("%1", value);
        return s;
    }

    protected string Tr2(string key, string first, string second)
    {
        string s = Widget.TranslateString("#" + key);
        s.Replace("%1", first);
        s.Replace("%2", second);
        return s;
    }

    // Capitals where the look writes in capitals.
    protected string Caps(string s)
    {
        if (m_Look && m_Look.Caps)
            return OZG_Text.Upper(s);
        return s;
    }

    protected void SetCaps(TextWidget w, string key)
    {
        if (w)
            w.SetText(Caps(Tr(key)));
    }

    protected string Picture(string name)
    {
        string style = m_Style;
        if (m_V && m_V.Style != "")
            style = m_V.Style;
        return "set:ozg_cards_" + style + " image:" + name;
    }

    // A picture in the art of a look: a card brought from another deck is
    // drawn as it is. A look of -1 (none said) is the deck's own.
    protected string PictureOf(string name, int look)
    {
        if (look < 0)
            return Picture(name);
        return "set:ozg_cards_" + OZG_Decks.StyleAt(look) + " image:" + name;
    }

    protected int LookAt(array<int> looks, int at)
    {
        if (!looks || at < 0 || at >= looks.Count())
            return -1;
        return looks[at];
    }

    protected void SetLabel(Widget button, string text)
    {
        if (!button)
            return;
        TextWidget label = TextWidget.Cast(button.FindAnyWidget(button.GetName() + "Text"));
        if (label)
            label.SetText(text);
    }

    // A card as the log names it: the rank and the suit's letter.
    protected string CardWord(string card)
    {
        if (card.Length() < 2)
            return card;
        string rank = card.Substring(0, 1);
        string suit = card.Substring(1, 1);
        string word = rank;
        if (rank == "T")
            word = "10";
        else if (rank == "J")
            word = Tr("STR_OZG_RANK_J");
        else if (rank == "Q")
            word = Tr("STR_OZG_RANK_Q");
        else if (rank == "K")
            word = Tr("STR_OZG_RANK_K");
        else if (rank == "A")
            word = Tr("STR_OZG_RANK_A");
        suit.ToUpper();
        return word + " " + Tr("STR_OZG_SUIT_" + suit);
    }

    protected string SeatName(int no)
    {
        if (m_V && no >= 0 && no < m_V.Seats.Count())
        {
            string name = m_V.Seats[no].Name;
            if (name != "")
                return name;
        }
        int shown = no + 1;
        return Tr1("STR_OZG_WIN_SEAT_NO", shown.ToString());
    }

    protected OZG_DurakSeatPublic GameSeat(int no)
    {
        if (!m_Pub || !m_Pub.SeatList)
            return null;
        for (int i = 0; i < m_Pub.SeatList.Count(); i++)
        {
            if (m_Pub.SeatList[i].SeatNo == no)
                return m_Pub.SeatList[i];
        }
        return null;
    }

    protected bool CanPlay(string move)
    {
        if (!m_Own || !m_Own.CanPlay)
            return false;
        return m_Own.CanPlay.Find(move) >= 0;
    }

    // The moves a hand card could make now.
    protected void MovesOf(string card, array<string> into)
    {
        into.Clear();
        if (!m_Own || !m_Own.CanPlay)
            return;
        for (int i = 0; i < m_Own.CanPlay.Count(); i++)
        {
            string move = m_Own.CanPlay[i];
            if (OZG_Cards.OfMove(move) == card)
                into.Insert(move);
        }
    }

    protected bool Playing()
    {
        return m_V && m_V.St == OZG_Const.ST_PLAYING;
    }

    protected bool MineInGame()
    {
        if (!m_V || m_V.Mine < 0)
            return false;
        return Playing() && m_V.Dealt.Find(m_V.Mine) >= 0;
    }

    // ------------------------------------------------------------ pictures on the table

    // The side of the square a card of w x h units turns in.
    protected float BoxOf(float w, float h)
    {
        return CARD_BOX * Math.Sqrt(w * w + h * h);
    }

    // A card's frame laid with its middle at (cx, cy), `side` units square,
    // turned clockwise by `turn` degrees about its middle.
    protected void Place(Widget w, float cx, float cy, float side, float turn)
    {
        float half = side * 0.5;
        w.SetSize(side * m_K, side * m_K);
        w.SetPos((cx - half) * m_K, (cy - half) * m_K);
        w.SetRotation(0, 0, turn);
    }

    // A card on the table: its picture with the middle at (cx, cy), w x h
    // units, turned clockwise by `turn` degrees, with its shadow under it when
    // `shadow`, and its glow in the colour `glow` when that is not 0. `slot`
    // is the key a flight lands on it by, `region` the part of the table a
    // shake moves it with; "" for none.
    protected Widget Card(array<Widget> into, string picture, float cx, float cy, float w, float h, float turn, int glow, bool shadow, string slot = "", string region = "")
    {
        Widget card = MakeLayout(LAYOUT_CARD, m_TableBox, "OZG_Card");
        if (!card)
            return null;
        ImageWidget face = ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace"));
        Widget halo = card.FindAnyWidget("OZG_CardGlow");
        Widget shade = card.FindAnyWidget("OZG_CardShadow");
        if (face)
        {
            face.LoadImageFile(0, picture);
            face.SetImage(0);
        }
        if (halo)
        {
            halo.Show(glow != 0);
            if (glow != 0)
                halo.SetColor(glow);
        }
        if (shade)
            shade.Show(shadow);
        Place(card, cx, cy, BoxOf(w, h), turn);
        into.Insert(card);
        KeepDrawn(card, slot, region, cx, cy, BoxOf(w, h));
        return card;
    }

    // Is (px, py) on a card of w x h units whose middle is (cx, cy), turned
    // clockwise by `turn` degrees?
    protected bool OnCard(float px, float py, float cx, float cy, float w, float h, float turn)
    {
        float r = turn * Math.DEG2RAD;
        float c = Math.Cos(r);
        float s = Math.Sin(r);
        float dx = px - cx;
        float dy = py - cy;
        float along = dx * c + dy * s;
        float across = dy * c - dx * s;
        return Math.AbsFloat(along) <= w * 0.5 && Math.AbsFloat(across) <= h * 0.5;
    }

    // ------------------------------------------------------------ what was drawn, for flights and shakes

    // The keys a flight lands on a card by: the index-th back of a seat's fan
    // (as PaintFan lays it), the index-th card of the player's hand (in the
    // order it is shown), an attack card of the bout or the card that beat
    // it, the k-th back of the discards' heap; the trump is SLOT_TRUMP.
    static string SlotOfFan(int seat, int index)
    {
        return "f" + seat.ToString() + "." + index.ToString();
    }

    static string SlotOfHand(int index)
    {
        return "h" + index.ToString();
    }

    static string SlotOfBout(int index, bool defence)
    {
        if (defence)
            return "d" + index.ToString();
        return "a" + index.ToString();
    }

    static string SlotOfHeap(int k)
    {
        return "x" + k.ToString();
    }

    // The region of a seat's fan, for ShakeRegion: the name the server's
    // `shake` gives it (OZG_Regions).
    static string RegionOfSeat(int seat)
    {
        return OZG_Regions.OfSeat(seat);
    }

    // A card drawn into the registry; hidden when a flight still has to land
    // on its key. Its place at rest is kept in pixels, for a shake.
    protected void KeepDrawn(Widget card, string slot, string region, float cx, float cy, float side)
    {
        if (!card)
            return;
        if (slot == "" && region == "")
            return;
        float half = side * 0.5;
        m_DrawnW.Insert(card);
        m_DrawnKey.Insert(slot);
        m_DrawnRegion.Insert(region);
        m_DrawnX.Insert((cx - half) * m_K);
        m_DrawnY.Insert((cy - half) * m_K);
        if (slot != "" && m_Hidden.Find(slot) >= 0)
            card.Show(false);
    }

    // The registry forgets the cards of one part of the table before that
    // part is drawn again and its pictures go.
    protected void ForgetDrawn(array<Widget> part)
    {
        for (int i = m_DrawnW.Count() - 1; i >= 0; i--)
        {
            Widget w = m_DrawnW[i];
            if (w && part.Find(w) < 0)
                continue;
            m_DrawnW.RemoveOrdered(i);
            m_DrawnKey.RemoveOrdered(i);
            m_DrawnRegion.RemoveOrdered(i);
            m_DrawnX.RemoveOrdered(i);
            m_DrawnY.RemoveOrdered(i);
        }
    }

    // ------------------------------------------------------------ where cards lie

    // The bottom seat of a view (BottomSeat for any view): the player's own;
    // for a watcher the first seat somebody holds; -1 for none.
    protected int BottomOf(OZG_WView v)
    {
        if (!v)
            return -1;
        if (v.Mine >= 0)
            return v.Mine;
        for (int i = 0; i < v.Seats.Count(); i++)
        {
            if (v.Seats[i].St != OZG_Const.SEAT_FREE)
                return i;
        }
        return -1;
    }

    // The seats round the table in a view, from the bottom one's left: every
    // seat somebody holds but the bottom one (PaintSeats).
    protected void RoundOf(OZG_WView v, int bottom, array<int> into)
    {
        into.Clear();
        int n = v.Seats.Count();
        for (int k = 1; k < n; k++)
        {
            int no = (bottom + k) % n;
            if (v.Seats[no].St != OZG_Const.SEAT_FREE)
                into.Insert(no);
        }
    }

    // The fan of the seat `no` as the view `v` lays it out: its middle, turn
    // and scale. False when that seat has no fan there: a free seat, or the
    // player's own (his hand is the faces at the bottom).
    protected bool FanOf(OZG_WView v, int no, out float x, out float y, out float turn, out float scale)
    {
        x = 0;
        y = 0;
        turn = 0;
        scale = 1;
        if (!v || no < 0 || no >= v.Seats.Count())
            return false;
        int bottom = BottomOf(v);
        if (no == bottom)
        {
            if (v.Mine >= 0)
                return false;
            x = m_Look.HandX;
            y = m_Look.HandY + 20;
            return true;
        }
        array<int> round = new array<int>();
        RoundOf(v, bottom, round);
        int at = round.Find(no);
        if (at < 0)
            return false;
        array<ref OZG_TableSpot> spots = new array<ref OZG_TableSpot>();
        m_Look.Spots(round.Count(), spots);
        OZG_TableSpot spot = spots[at];
        x = spot.X;
        y = spot.Y;
        turn = spot.Turn;
        scale = spot.Scale;
        return true;
    }

    // Where the i-th of `count` backs of a fan about (x, y), turned by `turn`,
    // rests: its middle and its own turn (PaintFan).
    protected void FanCardAt(float x, float y, float turn, float scale, int count, int i, out float cx, out float cy, out float a)
    {
        float spread = m_Look.FanSpread;
        if (count > 1 && spread * (count - 1) > m_Look.FanFan)
            spread = m_Look.FanFan / (count - 1);
        float r = m_Look.FanR * scale;
        float fx;
        float fy;
        float fturn;
        FanFacing(x, y, turn, r, spread, count, fx, fy, fturn);
        x = fx;
        y = fy;
        turn = fturn;
        float tr = turn * Math.DEG2RAD;
        float c = Math.Cos(tr);
        float s = Math.Sin(tr);
        float mid = (count - 1) * 0.5;
        float lean = (i - mid) * spread;
        float ar = lean * Math.DEG2RAD;
        float lx = r * Math.Sin(ar);
        float ly = r - r * Math.Cos(ar);
        cx = x + lx * c - ly * s;
        cy = y + lx * s + ly * c;
        a = lean + turn;
    }

    // Where the i-th of n cards of the player's hand rests before the pointer
    // lifts it: its middle and its turn (PaintHand).
    protected void HandCardAt(int n, int i, out float x, out float y, out float a)
    {
        float spread = m_Look.HandSpread;
        if (n > 1 && spread * (n - 1) > m_Look.HandFan)
            spread = m_Look.HandFan / (n - 1);
        float r = m_Look.HandR;
        float mid = (n - 1) * 0.5;
        a = (i - mid) * spread;
        float ar = a * Math.DEG2RAD;
        x = m_Look.HandX + r * Math.Sin(ar);
        y = m_Look.HandY + r - r * Math.Cos(ar);
    }

    // Where the index-th attack card of a bout of `n` lies, or the card that
    // beat it (`defence`: laid over it and turned a little): middle, size and
    // turn (PaintBout). One row of up to three; two rows of smaller cards from
    // four.
    protected void BoutCardAt(int n, int index, bool defence, out float cx, out float cy, out float w, out float h, out float turn)
    {
        float scale = 1;
        float pitch = 150;
        int perRow = n;
        int rows = 1;
        if (n == 3)
            pitch = 116;
        if (n >= 4)
        {
            rows = 2;
            perRow = (n + 1) / 2;
            scale = 0.82;
            pitch = 104;
            if (perRow > 3)
                pitch = 312.0 / (perRow - 1);
        }
        w = m_Look.BoutW * scale;
        h = m_Look.BoutH * scale;
        float ox = 18 * scale;
        float oy = 22 * scale;
        int row = 0;
        int col = index;
        int inRow = perRow;
        if (rows == 2 && index >= perRow)
        {
            row = 1;
            col = index - perRow;
            inRow = n - perRow;
        }
        float span = pitch * (inRow - 1) + w + ox;
        float left = m_Look.BoutX - span * 0.5 + col * pitch;
        float top = m_Look.BoutY;
        if (rows == 2)
            top = m_Look.BoutY - 66 + row * 118;
        cx = left + w * 0.5;
        cy = top + h * 0.5;
        turn = 0;
        if (defence)
        {
            cx = cx + ox;
            cy = cy + oy;
            turn = 12;
        }
    }

    // The middle of the stock's back number `k` from the bottom (PaintTable).
    protected void StockLayerAt(int k, out float sx, out float sy)
    {
        sx = m_Look.StockX - k * 1.2 + m_Look.PileW * 0.5;
        sy = m_Look.StockY - k * 1.6 + m_Look.PileH * 0.5;
    }

    // How many backs the stock shows for so many cards in it: the trump lies
    // apart, and six backs at most.
    protected int StockLayers(int stockCount)
    {
        int layers = stockCount - 1;
        if (layers > 6)
            layers = 6;
        return layers;
    }

    // A card of the seat `no`'s fan as the view `v` lays it out: the index-th
    // of `count` backs, or the fan's middle (index -1). Null when that seat has
    // no fan there.
    OZG_CardSpot SpotOfFan(OZG_WView v, int no, int index, int count)
    {
        float x;
        float y;
        float turn;
        float scale;
        if (!FanOf(v, no, x, y, turn, scale))
            return null;
        float w = m_Look.FanW * scale;
        float h = m_Look.FanH * scale;
        if (index < 0 || count < 1)
            return OZG_CardSpot.Make(x, y, w, h, turn);
        float cx;
        float cy;
        float a;
        FanCardAt(x, y, turn, scale, count, index, cx, cy, a);
        return OZG_CardSpot.Make(cx, cy, w, h, a);
    }

    // The index-th of `count` cards of the player's own hand, faces, in the
    // order the window shows them (SortHand).
    OZG_CardSpot SpotOfHand(int index, int count)
    {
        float x;
        float y;
        float a;
        HandCardAt(count, index, x, y, a);
        return OZG_CardSpot.Make(x, y, m_Look.CardW, m_Look.CardH, a);
    }

    // The top of a stock of `stockCount` cards: where a card dealt or drawn
    // leaves from; with no back left on it, the trump's place.
    OZG_CardSpot SpotOfStock(int stockCount)
    {
        int layers = StockLayers(stockCount);
        if (layers < 1)
            return SpotOfTrump();
        float sx;
        float sy;
        StockLayerAt(layers - 1, sx, sy);
        return OZG_CardSpot.Make(sx, sy, m_Look.PileW, m_Look.PileH, 0);
    }

    // The trump, turned under the stock.
    OZG_CardSpot SpotOfTrump()
    {
        float tx = m_Look.TrumpX + m_Look.PileW * 0.5;
        float ty = m_Look.TrumpY + m_Look.PileH * 0.5;
        return OZG_CardSpot.Make(tx, ty, m_Look.PileW, m_Look.PileH, 90);
    }

    // The k-th back of the discards' heap.
    OZG_CardSpot SpotOfDiscards(int k)
    {
        float dx;
        float dy;
        float dt;
        OZG_TableLook.HeapAt(k, dx, dy, dt);
        float hx = m_Look.DiscardX + dx;
        float hy = m_Look.DiscardY + dy;
        return OZG_CardSpot.Make(hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt);
    }

    // The index-th attack card of a bout of `count` cards, or the card that
    // beat it.
    OZG_CardSpot SpotOfBout(int count, int index, bool defence)
    {
        float cx;
        float cy;
        float w;
        float h;
        float turn;
        BoutCardAt(count, index, defence, cx, cy, w, h, turn);
        return OZG_CardSpot.Make(cx, cy, w, h, turn);
    }

    // How many backs the heap shows for so many cards discarded: a back for
    // every two, OZG_TableLook.HEAP at most.
    int HeapOf(int discardCount)
    {
        int heap = (discardCount + 1) / 2;
        if (heap > OZG_TableLook.HEAP)
            heap = OZG_TableLook.HEAP;
        return heap;
    }

    // The pictures a flight wears: a card's face in a look, a look's back
    // (-1: the deck's own).
    string PictureOfFace(string card, int look)
    {
        return PictureOf("c_" + OZG_Cards.FaceOf(card), look);
    }

    string PictureOfBack(int look)
    {
        return PictureOf("back", look);
    }

    // ------------------------------------------------------------ flights and shakes

    // A card flies over the table (OZG_Flights): `picture` on its face at
    // first, `pictureLand` from halfway ("" keeps it), from `from` to `to`,
    // after `delay` seconds, for `seconds`. The card drawn under the key
    // `reveal` is hidden until it lands. Nothing waits for it, and a newer
    // view lands it at once. The card sharp's fourth phase flies the sleights
    // with this.
    OZG_Flight Fly(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        if (!m_Air || !from || !to)
            return null;
        Widget card = MakeLayout(LAYOUT_CARD, m_Air, "OZG_Card");
        if (!card)
            return null;
        ImageWidget face = ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace"));
        Widget halo = card.FindAnyWidget("OZG_CardGlow");
        if (face)
        {
            face.LoadImageFile(0, picture);
            face.SetImage(0);
        }
        if (halo)
            halo.Show(false);
        if (reveal != "")
            HideSlot(reveal);
        return m_Flights.Launch(card, pictureLand, from, to, delay, seconds, reveal);
    }

    // The card drawn under `key` waits, hidden, for one more flight.
    void HideSlot(string key)
    {
        m_Hidden.Insert(key);
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            if (m_DrawnKey[i] == key && m_DrawnW[i])
                m_DrawnW[i].Show(false);
        }
    }

    // One flight onto `key` landed: the card drawn there shows once no other
    // is on its way to it.
    protected void RevealSlot(string key)
    {
        m_Hidden.RemoveItem(key);
        if (m_Hidden.Find(key) >= 0)
            return;
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            if (m_DrawnKey[i] == key && m_DrawnW[i])
                m_DrawnW[i].Show(true);
        }
    }

    // Every card in the air at once where it lands, and every hidden card
    // shown: a newer view, a look built anew, the window closing.
    protected void LandAll()
    {
        int n = m_Flights.Count();
        array<string> landed = new array<string>();
        m_Flights.Finish(landed);
        for (int i = 0; i < landed.Count(); i++)
            RevealSlot(landed[i]);
        m_Hidden.Clear();
        if (n > 0)
            OZG_Log.Dbg("flights: " + n.ToString() + " landed at once");
    }

    // A small shake of a region of the table: a seat's fan (RegionOfSeat),
    // the discards (REGION_DISCARDS) or the stock (REGION_STOCK). What a
    // player who saw a sleight is shown (card sharp's design §6.1).
    void ShakeRegion(string region)
    {
        if (region == "")
            return;
        int cards = 0;
        for (int i = 0; i < m_DrawnRegion.Count(); i++)
        {
            if (m_DrawnRegion[i] == region)
                cards++;
        }
        m_Shakes.Start(region);
        OZG_Log.Dbg("shake " + region + ": " + cards.ToString() + " cards");
    }

    // The cards in the air, one frame on; a landed card shows its key.
    protected void TickAir(float dt)
    {
        if (m_Flights.Count() == 0)
            return;
        array<string> landed = new array<string>();
        m_Flights.Tick(dt, m_K, landed);
        for (int i = 0; i < landed.Count(); i++)
            RevealSlot(landed[i]);
        if (m_Flights.Count() == 0)
            OZG_Log.Dbg("flights: all landed");
    }

    // The shaking regions, one frame on: their cards moved off their rest,
    // and put back when the shake ends.
    protected void TickShakes(float dt)
    {
        if (!m_Shakes.Busy())
            return;
        array<string> ended = new array<string>();
        m_Shakes.Tick(dt, ended);
        float dx;
        float dy;
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            string region = m_DrawnRegion[i];
            Widget w = m_DrawnW[i];
            if (region == "" || !w)
                continue;
            bool moving = m_Shakes.Offset(region, dx, dy);
            if (!moving && ended.Find(region) < 0)
                continue;
            w.SetPos(m_DrawnX[i] + dx * m_K, m_DrawnY[i] + dy * m_K);
        }
    }

    // ------------------------------------------------------------ marks

    // Which of this player's reads a back of the seat `seat` (-1: the top of
    // the stock) in the look `look` shows: the first that lies there in that
    // look and that no back drawn before took (`used`); -1: none. A fan shows
    // its backs by look and not in the hand's order, so a read lies on the
    // first free back of its look.
    protected int ReadAt(int seat, int look, array<int> used)
    {
        if (!m_V || !m_V.Reads)
            return -1;
        for (int i = 0; i < m_V.Reads.Count(); i++)
        {
            OZG_WRead r = m_V.Reads[i];
            if (r.Seat != seat || r.Look != look)
                continue;
            if (used.Find(i) >= 0)
                continue;
            used.Insert(i);
            return i;
        }
        return -1;
    }

    // Which of this player's notices a back of the seat `seat` (-1: the top
    // of the stock) in the look `look` shows: the first notice of a back
    // there in that look that no back drawn before took (`used`); -1: none.
    // As a read, a notice lies on the first free back of its look.
    protected int NoticeAt(int seat, int look, array<int> used)
    {
        if (!m_V || !m_V.Notices)
            return -1;
        for (int i = 0; i < m_V.Notices.Count(); i++)
        {
            OZG_WNotice n = m_V.Notices[i];
            if (n.CardName != "" || n.Seat != seat || n.Look != look)
                continue;
            if (used.Find(i) >= 0)
                continue;
            used.Insert(i);
            return i;
        }
        return -1;
    }

    // The notice on the face of the card `card`, wherever it lies face up
    // for this player; -1: none.
    protected int NoticeOn(string card)
    {
        if (!m_V || !m_V.Notices || card == "")
            return -1;
        for (int i = 0; i < m_V.Notices.Count(); i++)
        {
            if (m_V.Notices[i].CardName == card)
                return i;
        }
        return -1;
    }

    // A picture of set:ozg_marks laid into the frame of a card of w x h
    // units, at a box given in pixels of a 200 x 280 card (OZG_MarkPlaces,
    // tools/make_marks.py), so that it turns with the card. The face lies in
    // the middle of the frame, 5 : 7, as tools/make_layouts.py lays it.
    protected void LayMark(ImageWidget pic, string image, float w, float h, int x, int y, int bw, int bh)
    {
        float side = BoxOf(w, h) * m_K;
        float d = CARD_BOX * Math.Sqrt(74);
        float faceW = side * 5 / d;
        float faceH = side * 7 / d;
        float left = (side - faceW) * 0.5;
        float top = (side - faceH) * 0.5;
        float kx = faceW / OZG_MarkPlaces.CARD_W;
        float ky = faceH / OZG_MarkPlaces.CARD_H;
        pic.LoadImageFile(0, image);
        pic.SetImage(0);
        pic.SetPos(left + x * kx, top + y * ky);
        pic.SetSize(bw * kx, bh * ky);
        pic.Show(true);
    }

    // A read on a back of w x h units: the sign where its fault lies and the
    // face in pencil in the top corner (design §4.4), moved where the card is
    // seen when another lies over it (`cover`, OZG_Cover; null: nothing does).
    protected void MarkBack(Widget card, float w, float h, OZG_WRead r, OZG_Cover cover = null)
    {
        SignBack(card, w, h, r.Look, r.Sign);
        ImageWidget pencil = ImageWidget.Cast(card.FindAnyWidget("OZG_CardPencil"));
        if (!pencil)
            return;
        int x;
        int y;
        int bw;
        int bh;
        if (!OZG_MarkPlaces.PencilBox(x, y, bw, bh))
            return;
        if (cover)
            cover.PutSeen(x, y, bw, bh);
        LayMark(pencil, OZG_MarkPlaces.PencilImage(r.Face), w, h, x, y, bw, bh);
    }

    // A sign on a back of w x h units, the fault where it lies in the back's
    // pattern (design §4.5): beside a reader's pencil, and all that a noticer
    // sees there.
    protected void SignBack(Widget card, float w, float h, int look, int sign)
    {
        ImageWidget pic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        if (!pic)
            return;
        string style = OZG_Decks.StyleAt(look);
        int x;
        int y;
        int bw;
        int bh;
        if (OZG_MarkPlaces.SignBox(style, sign, x, y, bw, bh))
            LayMark(pic, OZG_MarkPlaces.SignImage(style, sign), w, h, x, y, bw, bh);
    }

    // A noticed card on its face of w x h units: a small dark speck in the
    // margin, where its sign puts it (design §4.5), in the frame's sign
    // picture -- a face shows no fault of the back. Moved where the card is
    // seen when another lies over it (`cover`, OZG_Cover).
    protected void SpeckFace(Widget card, float w, float h, OZG_WNotice n, OZG_Cover cover = null)
    {
        ImageWidget pic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        if (!pic)
            return;
        string style = OZG_Decks.StyleAt(n.Look);
        int x;
        int y;
        int bw;
        int bh;
        if (!OZG_MarkPlaces.SpeckBox(style, n.Sign, x, y, bw, bh))
            return;
        if (cover)
            cover.PutSeen(x, y, bw, bh);
        LayMark(pic, OZG_MarkPlaces.SpeckImage(), w, h, x, y, bw, bh);
    }

    // ------------------------------------------------------------ drawing

    void Redraw()
    {
        if (!layoutRoot)
            return;
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
        // The view shown until now, for the flights: in locals, so that it
        // lives through this call while the members take the new one.
        OZG_WView wasV = m_V;
        OZG_DurakPublic wasPub = m_Pub;
        OZG_DurakPrivate wasOwn = m_Own;
        bool newer = false;
        if (wasV && view.HeadPart.Rev > m_ShownRev)
            newer = true;
        m_V = view.HeadPart;
        m_ShownRev = m_V.Rev;
        m_Pub = null;
        m_Own = null;
        string err;
        bool durak = m_V.GameId == OZG_DurakConst.GAME_ID;
        if (durak && view.Shared != "")
            JsonFileLoader<OZG_DurakPublic>.LoadData(view.Shared, m_Pub, err);
        if (durak && view.Own != "")
            JsonFileLoader<OZG_DurakPrivate>.LoadData(view.Own, m_Own, err);
        m_GPub = null;
        m_GOwn = null;
        if (!durak && view.Shared != "")
            JsonFileLoader<OZG_GPublic>.LoadData(view.Shared, m_GPub, err);
        if (!durak && view.Own != "")
            JsonFileLoader<OZG_GPrivate>.LoadData(view.Own, m_GOwn, err);

        // The deck's look decides the window's.
        string style = m_V.Style;
        if (style != "stalker")
            style = "classic";
        bool rebuilt = false;
        if (style != m_Style || !m_Win)
        {
            Build(style);
            rebuilt = true;
        }
        if (!m_Win)
            return;
        // A newer view lands every card still in the air before it is drawn:
        // nothing flies on to a place the new view may have moved.
        if (newer)
            LandAll();

        if (m_Picked != "")
        {
            array<string> still = new array<string>();
            MovesOf(m_Picked, still);
            if (still.Count() < 2)
                m_Picked = "";
        }
        if (m_PickedSeat >= m_V.Seats.Count())
            m_PickedSeat = -1;
        if (!MineInGame())
            m_AskStand = false;
        if (!Playing())
            m_AskFold = false;
        if (m_V.Note != "")
            Said("#" + m_V.Note);

        // The dealer's plan lives between deals and on his seat only: a deal
        // dealt, or the dealing gone to another, leaves none.
        if (!Dealing())
            ClearPlan();
        if (m_PocketsLine == "")
        {
            OZG_PocketsRead.Into(m_PocketCards);
            m_PocketsLine = OZG_PocketsRead.Line(m_PocketCards);
        }

        Measure();
        GenHideTable();
        PaintTitle();
        PaintSeats();
        PaintTable();
        PaintHand();
        PaintMoment();
        PaintColumn();
        PaintLog();
        PaintSleights();

        // What moved since the view shown before flies to where it is now:
        // only for a newer view of the game in the same look.
        if (newer && !rebuilt && durak)
            OZG_FlightPlan.Make(this, wasV, wasPub, wasOwn, m_V, m_Pub, m_Own);
        // A sleight of this player's that the server answered before this
        // view came flies now.
        if (m_Answer && m_ShownRev >= m_Answer.Rev)
        {
            OZG_WSleight answer = m_Answer;
            m_Answer = null;
            FlySleight(answer);
        }
    }

    protected string VariantLabel()
    {
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (!sample)
            return "";
        array<ref OZG_Option> opts = new array<ref OZG_Option>();
        sample.OptionList(opts);
        for (int i = 0; i < opts.Count(); i++)
        {
            OZG_Option o = opts[i];
            int at = m_V.OptIds.Find(o.OptionId);
            string chosen = o.DefaultId;
            if (at >= 0 && at < m_V.OptVals.Count())
                chosen = m_V.OptVals[at];
            int v = o.ValueIds.Find(chosen);
            if (v >= 0 && v < o.ValueKeys.Count())
                return Tr(o.ValueKeys[v]);
        }
        return "";
    }

    protected void PaintTitle()
    {
        string title = "";
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (sample)
            title = Tr(sample.LabelKey());
        string variant = VariantLabel();
        if (variant != "")
            title = title + " — " + variant;
        m_Title.SetText(Caps(title));

        string sub = "";
        if (m_V.Mine < 0)
            sub = Tr("STR_OZG_ST_WATCHER");
        m_Sub.SetText(sub);
    }

    // The seat that sits at the bottom, where the hand is: the player's own;
    // a watcher looks from the side of the first seat somebody holds (-1:
    // nobody holds one).
    protected int BottomSeat()
    {
        return BottomOf(m_V);
    }

    // What the game says of a seat now, and whether it waits for that seat.
    protected string RoleOf(int i, out bool turn)
    {
        turn = false;
        string does = "";
        OZG_WSeat s = m_V.Seats[i];
        OZG_DurakSeatPublic g = GameSeat(i);
        // A seat that is free now says nothing of a game that has ended.
        if (g && m_Pub.Ended == 1 && s.St == OZG_Const.SEAT_FREE)
            g = null;
        if (g && m_V.St != OZG_Const.ST_LOBBY)
        {
            if (g.Gone == OZG_DurakConst.GONE_OUT)
                does = Tr("STR_OZG_WIN_ROLE_OUT");
            else if (g.Gone == OZG_DurakConst.GONE_DROPPED)
                does = Tr("STR_OZG_WIN_ROLE_DROPPED");
            else if (m_Pub.Ended == 1)
                does = "";
            else if (i == m_Pub.DefenderSeat && m_Pub.Taking == 1)
                does = Tr("STR_OZG_WIN_ROLE_TAKES");
            else if (i == m_Pub.DefenderSeat)
                does = Tr("STR_OZG_WIN_ROLE_DEFEND");
            else if (g.Passed == 1)
                does = Tr("STR_OZG_WIN_ROLE_PASSED");
            else if (i == m_Pub.OpenerSeat)
                does = Tr("STR_OZG_WIN_ROLE_ATTACK");
            if (m_Pub.Ended == 1 && m_Pub.DurakSeat == i)
                does = Tr("STR_OZG_WIN_ROLE_DURAK");
            if (m_Pub.Awaited && m_Pub.Awaited.Find(i) >= 0 && m_Pub.Ended != 1)
                turn = true;
        }
        else if (Playing() && s.St != OZG_Const.SEAT_FREE)
        {
            does = Tr("STR_OZG_WIN_ROLE_WATCH");
        }

        // The host's mark and a seat kept for one who walked away.
        if (i == m_V.HostSeat)
            does = Joined(does, Tr("STR_OZG_WIN_HOST_MARK"));
        if (s.St == OZG_Const.SEAT_KEPT)
            does = Joined(does, Tr("STR_OZG_WIN_AWAY"));
        return does;
    }

    protected string Joined(string a, string b)
    {
        if (a == "")
            return b;
        return a + " · " + b;
    }

    // The cards a seat holds in this game, by the look of each back; false when
    // it holds none on the table now.
    protected bool BacksOf(int i, out OZG_DurakSeatPublic g)
    {
        g = GameSeat(i);
        if (!g || !m_Pub || m_V.St == OZG_Const.ST_LOBBY)
            return false;
        if (m_Pub.Ended == 1 && m_V.Seats[i].St == OZG_Const.SEAT_FREE)
            return false;
        return g.CardCount > 0;
    }

    protected void PaintSeats()
    {
        ForgetDrawn(m_FanPics);
        ClearList(m_FanPics);
        ClearList(m_SeatW);
        m_SeatOf.Clear();
        m_GenFaceCodes.Clear();
        m_GenFaceX.Clear();
        m_GenFaceY.Clear();
        m_GenFaceW.Clear();
        m_GenFaceH.Clear();
        m_GenFaceA.Clear();
        int n = m_V.Seats.Count();
        if (n == 0)
        {
            m_MyPlate.Show(false);
            return;
        }

        // Round the table sit the seats somebody holds, in the ring's order
        // from the bottom one's left; a free seat takes no place there (the
        // world marks where one may sit).
        int bottom = BottomSeat();
        if (bottom < 0)
        {
            m_MyPlate.Show(false);
            return;
        }
        array<int> round = new array<int>();
        RoundOf(m_V, bottom, round);
        array<ref OZG_TableSpot> spots = new array<ref OZG_TableSpot>();
        m_Look.Spots(round.Count(), spots);
        for (int r = 0; r < round.Count(); r++)
            PaintSeat(round[r], spots[r]);
        PaintBottom(bottom);
    }

    // One other seat: its fan of backs and its name plate.
    protected void PaintSeat(int no, OZG_TableSpot spot)
    {
        OZG_WSeat s = m_V.Seats[no];
        OZG_DurakSeatPublic g;
        // Before the dealer's deal nothing of the game that ended lies on his
        // table (PaintTable): his frame takes it.
        if (Generic())
            GenSeatCards(no, spot.X, spot.Y, spot.Turn, spot.Scale);
        else if (BacksOf(no, g) && !Dealing())
            PaintFan(spot.X, spot.Y, spot.Turn, spot.Scale, g.CardCount, g.Backs, no);

        string root = "OZG_PlateClassic";
        if (m_Style == "stalker")
            root = "OZG_PlateStalker";
        Widget plate = MakeLayout("OpenZone_Games/gui/layouts/ozg_plate_" + m_Style + ".layout", m_SeatsBox, root);
        if (!plate)
            return;
        // The line under the name: the later games' says more (a blind, the
        // chips, the bet), and the plate grows to hold it.
        bool turn;
        string roleText;
        float plateW = spot.PlateW;
        if (Generic())
        {
            roleText = GenRole(no, turn);
            plateW = GenPlateWidth(roleText, plateW);
        }
        else
            roleText = RoleOf(no, turn);

        // The plate lies PLATE_PAD units inside its button, which leaves room
        // for the ring of a seat the host picked (tools/make_layouts.py).
        float h = 48;
        if (m_Style == "stalker")
            h = 46;
        float bw = plateW + 2 * PLATE_PAD;
        float bh = h + 2 * PLATE_PAD;
        float left = spot.PlateX - bw * 0.5;
        // A grown plate stays on the table: it slides in from the edge.
        if (Generic())
        {
            if (left < GEN_PLATE_MIN_X)
                left = GEN_PLATE_MIN_X;
            if (left + bw > GEN_PLATE_MAX_X)
                left = GEN_PLATE_MAX_X - bw;
        }
        float top = spot.PlateY - PLATE_PAD;
        plate.SetSize(bw * m_K, bh * m_K);
        plate.SetPos(left * m_K, top * m_K);
        m_SeatW.Insert(plate);
        m_SeatOf.Insert(no);

        TextWidget name = TextWidget.Cast(plate.FindAnyWidget("OZG_PlateName"));
        TextWidget role = TextWidget.Cast(plate.FindAnyWidget("OZG_PlateRole"));
        Widget edge = plate.FindAnyWidget("OZG_PlateEdge");
        Widget sel = plate.FindAnyWidget("OZG_PlateSel");

        string who = s.Name;
        if (s.St == OZG_Const.SEAT_LEFT)
            who = Tr("STR_OZG_WIN_SEAT_LEFT");
        name.SetText(Caps(who));

        role.SetText(roleText);
        if (turn)
        {
            edge.SetColor(m_Look.EdgeTurn);
            role.SetColor(m_Look.RoleTurn);
        }
        else
        {
            edge.SetColor(m_Look.EdgeIdle);
            role.SetColor(m_Look.RoleIdle);
        }
        sel.Show(no == m_PickedSeat);
    }

    // A fan a seat round the table holds bulges towards the table: the player
    // holds it in front of himself, the middle card nearest the table and the
    // outer ones curving back towards him, as the player's own hand does (the
    // owner, 2026-11-25: "they must bulge towards the table! a player holds his
    // fan in front of him" -- the side seats' fans bulged at their players).
    // The seats' spots turn the side fans the other way round, so a fan whose
    // outer cards would curve in towards the table is turned half round in the
    // same room: its middle card moves to where its outer ones were. The order
    // of the cards, and so what covers what, stays.
    protected void FanFacing(float x, float y, float turn, float r, float spread, int count, out float fx, out float fy, out float fturn)
    {
        fx = x;
        fy = y;
        fturn = turn;
        float tr = turn * Math.DEG2RAD;
        float dx = -Math.Sin(tr);
        float dy = Math.Cos(tr);
        float tx = m_Look.BoutX - x;
        float ty = m_Look.BoutY + m_Look.BoutH * 0.5 - y;
        float towards = dx * tx + dy * ty;
        if (towards <= 0)
            return;
        float half = spread * (count - 1) * 0.5 * Math.DEG2RAD;
        float most = r - r * Math.Cos(half);
        fx = x + dx * most;
        fy = y + dy * most;
        fturn = turn + 180;
        if (fturn >= 360)
            fturn = fturn - 360;
    }

    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first), the hand of the seat `seat`: a
    // back this player reads there carries its mark (design §4.4), a back he
    // notices its sign (§4.5).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks, int seat)
    {
        array<int> used = new array<int>();
        array<int> noticed = new array<int>();
        float spread = m_Look.FanSpread;
        if (count > 1 && spread * (count - 1) > m_Look.FanFan)
            spread = m_Look.FanFan / (count - 1);
        float r = m_Look.FanR * scale;
        float w = m_Look.FanW * scale;
        float h = m_Look.FanH * scale;
        float fx;
        float fy;
        float fturn;
        FanFacing(x, y, turn, r, spread, count, fx, fy, fturn);
        x = fx;
        y = fy;
        turn = fturn;
        float tr = turn * Math.DEG2RAD;
        float c = Math.Cos(tr);
        float s = Math.Sin(tr);
        float mid = (count - 1) * 0.5;
        for (int i = 0; i < count; i++)
        {
            float a = (i - mid) * spread;
            float ar = a * Math.DEG2RAD;
            float lx = r * Math.Sin(ar);
            float ly = r - r * Math.Cos(ar);
            float cx = x + lx * c - ly * s;
            float cy = y + lx * s + ly * c;
            int look = LookAt(looks, i);
            string back = PictureOf("back", look);
            // A flight's slot and the seat's region (OZG_Flights.c).
            Widget card = Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true, SlotOfFan(seat, i), RegionOfSeat(seat));
            int readNo = ReadAt(seat, look, used);
            int noticeNo = -1;
            if (readNo < 0)
                noticeNo = NoticeAt(seat, look, noticed);
            // Every card but the last has the next one over its right side.
            OZG_Cover over = null;
            if (i < count - 1)
                over = OZG_Cover.MakeFan(r, spread, w, h);
            if (card && readNo >= 0)
                MarkBack(card, w, h, m_V.Reads[readNo], over);
            if (card && noticeNo >= 0)
                SignBack(card, w, h, look, m_V.Notices[noticeNo].Sign);
        }
    }

    // The seat at the bottom: the player's own plate under the hand; a
    // watcher sees the first seat there, its fan of backs where a hand would be.
    protected void PaintBottom(int no)
    {
        m_MyPlate.Show(true);
        string name = SeatName(no);
        if (no == m_V.Mine)
            name = name + " (" + Tr("STR_OZG_WIN_YOU") + ")";
        m_MyName.SetText(Caps(name));
        bool turn;
        string roleText;
        float plateW = m_Look.MyPlateW;
        if (Generic())
        {
            roleText = GenRole(no, turn);
            plateW = GenPlateWidth(roleText, plateW);
        }
        else
            roleText = RoleOf(no, turn);
        m_MyRole.SetText(roleText);
        FitMyPlate(plateW);
        if (turn)
            m_MyEdge.SetColor(m_Look.EdgeTurn);
        else
            m_MyEdge.SetColor(m_Look.EdgeIdle);

        if (Generic())
        {
            if (m_V.Mine < 0)
                GenSeatCards(no, m_Look.HandX, m_Look.HandY + 20, 0, 1);
            return;
        }
        OZG_DurakSeatPublic g;
        if (m_V.Mine < 0 && BacksOf(no, g))
            PaintFan(m_Look.HandX, m_Look.HandY + 20, 0, 1, g.CardCount, g.Backs, no);
    }

    protected void PaintTable()
    {
        ForgetDrawn(m_TablePics);
        ClearList(m_TablePics);
        m_AtkX.Clear();
        m_AtkY.Clear();
        m_AtkW.Clear();
        m_AtkH.Clear();
        m_Result.Show(false);
        if (Generic())
        {
            GenPaintTable();
            return;
        }
        // Before the dealer's deal his frame lies on the table and the deck at
        // the stock's place (PaintDealer); nothing of the game that ended.
        if (Dealing())
        {
            m_StockLabel.Show(true);
            m_DiscardLabel.Show(false);
            return;
        }
        bool dealt = m_Pub && m_V.St != OZG_Const.ST_LOBBY;
        m_StockLabel.Show(dealt);
        m_DiscardLabel.Show(dealt);
        if (!dealt)
            return;

        float pw = m_Look.PileW;
        float ph = m_Look.PileH;

        // The stock, the trump turned under it; when the stock is gone the
        // trump's suit stays to be seen.
        float tx = m_Look.TrumpX + pw * 0.5;
        float ty = m_Look.TrumpY + ph * 0.5;
        if (m_Pub.TrumpCard != "")
        {
            if (m_Pub.StockCount > 0)
            {
                string trump = "c_" + OZG_Cards.FaceOf(m_Pub.TrumpCard);
                Widget trumpPic = Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true, SLOT_TRUMP, REGION_STOCK);
                // A trump this player notices shows its speck (design §4.5),
                // on the top end that comes out from under the stock (turned
                // a quarter clockwise, its top points right).
                int trumpNotice = NoticeOn(m_Pub.TrumpCard);
                OZG_Cover underStock = null;
                if (m_Pub.StockCount > 1)
                {
                    float sticks = tx + ph * 0.5 - (m_Look.StockX + pw);
                    underStock = OZG_Cover.MakeTop(sticks / ph);
                }
                if (trumpPic && trumpNotice >= 0)
                    SpeckFace(trumpPic, pw, ph, m_V.Notices[trumpNotice], underStock);
            }
            else
            {
                string suit = "suit_" + m_Pub.TrumpCard.Substring(1, 1);
                Card(m_TablePics, Picture(suit), tx, ty, 56, 78, 0, 0, false, "", REGION_STOCK);
            }
        }
        int layers = m_Pub.StockCount - 1;
        if (layers > 6)
            layers = 6;
        int k;
        array<int> stockUsed = new array<int>();
        array<int> stockNoticed = new array<int>();
        for (k = 0; k < layers; k++)
        {
            // The top card shows the look of the one drawn next.
            int look = -1;
            if (k == layers - 1)
                look = m_Pub.StockTop;
            float sx = m_Look.StockX - k * 1.2 + pw * 0.5;
            float sy = m_Look.StockY - k * 1.6 + ph * 0.5;
            Widget layer = Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true, "", REGION_STOCK);
            // The top card: a back this player reads carries its mark (design
            // §4.4), a back he notices its sign (§4.5).
            int topRead = -1;
            int topNotice = -1;
            if (k == layers - 1)
                topRead = ReadAt(-1, look, stockUsed);
            if (k == layers - 1 && topRead < 0)
                topNotice = NoticeAt(-1, look, stockNoticed);
            if (layer && topRead >= 0)
                MarkBack(layer, pw, ph, m_V.Reads[topRead]);
            if (layer && topNotice >= 0)
                SignBack(layer, pw, ph, look, m_V.Notices[topNotice].Sign);
        }

        // The discards: a careless heap, a back for every two cards.
        int heap = (m_Pub.DiscardCount + 1) / 2;
        if (heap > OZG_TableLook.HEAP)
            heap = OZG_TableLook.HEAP;
        float dx;
        float dy;
        float dt;
        for (k = 0; k < heap; k++)
        {
            OZG_TableLook.HeapAt(k, dx, dy, dt);
            float hx = m_Look.DiscardX + dx;
            float hy = m_Look.DiscardY + dy;
            Card(m_TablePics, PictureOf("back", -1), hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt, 0, true, SlotOfHeap(k), REGION_DISCARDS);
        }

        PaintBout();

        if (m_Pub.Ended == 1)
        {
            string over = Tr("STR_OZG_ST_OVER_NONE");
            if (m_Pub.Tie == 1)
                over = Tr("STR_OZG_ST_OVER_TIE");
            else if (m_Pub.DurakSeat >= 0)
                over = Tr1("STR_OZG_ST_OVER_DURAK", SeatName(m_Pub.DurakSeat));
            m_Result.SetText(Caps(over));
            PutAt(m_Result, m_Look.ResultX, m_Look.ResultY, m_Look.ResultW, m_Look.ResultH);
        }
    }

    // The bout: each attack card and the card that beat it laid over, turned a
    // little. One row of up to three; more go into two rows of smaller cards.
    protected void PaintBout()
    {
        int n = m_Pub.AttackCards.Count();
        if (n == 0)
            return;
        array<string> picked = new array<string>();
        if (m_Picked != "")
            MovesOf(m_Picked, picked);

        float scale = 1;
        float pitch = 150;
        int perRow = n;
        int rows = 1;
        if (n == 3)
            pitch = 116;
        if (n >= 4)
        {
            rows = 2;
            perRow = (n + 1) / 2;
            scale = 0.82;
            pitch = 104;
            if (perRow > 3)
                pitch = 312.0 / (perRow - 1);
        }
        float w = m_Look.BoutW * scale;
        float h = m_Look.BoutH * scale;
        float ox = 18 * scale;
        float oy = 22 * scale;

        for (int i = 0; i < n; i++)
        {
            int row = 0;
            int col = i;
            int inRow = perRow;
            if (rows == 2 && i >= perRow)
            {
                row = 1;
                col = i - perRow;
                inRow = n - perRow;
            }
            float span = pitch * (inRow - 1) + w + ox;
            float left = m_Look.BoutX - span * 0.5 + col * pitch;
            float top = m_Look.BoutY;
            if (rows == 2)
                top = m_Look.BoutY - 66 + row * 118;
            float cx = left + w * 0.5;
            float cy = top + h * 0.5;

            int number = i + 1;
            string target = "d:" + m_Picked + ":" + number.ToString();
            int glow = 0;
            if (picked.Find(target) >= 0)
                glow = m_Look.TargetGlow;
            // One step at a time: a long expression does not compile.
            string attackFace = "c_" + OZG_Cards.FaceOf(m_Pub.AttackCards[i]);
            int attackLook = LookAt(m_Pub.AttackLooks, i);
            Widget attackPic = Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true, SlotOfBout(i, false));
            string beaten = "";
            if (i < m_Pub.DefenceCards.Count())
                beaten = m_Pub.DefenceCards[i];
            // A card of the table this player notices shows its speck (design
            // §4.5), clear of the card that beat it.
            int attackNotice = NoticeOn(m_Pub.AttackCards[i]);
            OZG_Cover underBeat = null;
            if (beaten != "")
                underBeat = OZG_Cover.MakeBout(ox, oy, 12, w, h);
            if (attackPic && attackNotice >= 0)
                SpeckFace(attackPic, w, h, m_V.Notices[attackNotice], underBeat);
            m_AtkX.Insert(cx);
            m_AtkY.Insert(cy);
            m_AtkW.Insert(w);
            m_AtkH.Insert(h);
            if (beaten != "")
            {
                string beatenFace = "c_" + OZG_Cards.FaceOf(beaten);
                int beatenLook = LookAt(m_Pub.DefenceLooks, i);
                Widget beatenPic = Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true, SlotOfBout(i, true));
                int beatenNotice = NoticeOn(beaten);
                if (beatenPic && beatenNotice >= 0)
                    SpeckFace(beatenPic, w, h, m_V.Notices[beatenNotice]);
            }
        }
    }

    // The order a hand is shown in: suit by suit, the trumps last, low to high.
    protected int SortKey(string card, string trumpSuit)
    {
        string suit = card.Substring(1, 1);
        int group = OZG_Cards.SUITS.IndexOf(suit);
        if (suit == trumpSuit)
            group = 9;
        return group * 100 + OZG_Cards.RankOf(card);
    }

    // A hand in the order the window shows it (SortKey). The flights find a
    // card's place in it.
    void SortHand(array<string> held, string trumpSuit, array<string> into)
    {
        into.Clear();
        if (!held)
            return;
        into.Copy(held);
        int n = into.Count();
        for (int i = 1; i < n; i++)
        {
            string moved = into[i];
            int key = SortKey(moved, trumpSuit);
            int j = i - 1;
            while (j >= 0 && SortKey(into[j], trumpSuit) > key)
            {
                into[j + 1] = into[j];
                j--;
            }
            into[j + 1] = moved;
        }
    }

    protected void PaintHand()
    {
        ForgetDrawn(m_HandPics);
        ClearList(m_HandPics);
        ClearHandMarks();
        if (Generic())
        {
            GenPaintHand();
            return;
        }
        if (!m_Own || !m_Own.Held || m_V.St == OZG_Const.ST_LOBBY)
            return;
        // The hand the last game left lies under the dealer's frame: it is
        // not drawn before his deal (measured 2026-11-25: six cards over the
        // second row of his choice).
        if (Dealing())
            return;

        string trumpSuit = "";
        if (m_Pub)
            trumpSuit = OZG_Cards.SuitOf(m_Pub.TrumpCard);

        // The flights find a card's place by the same order (SortHand).
        array<string> cards = new array<string>();
        SortHand(m_Own.Held, trumpSuit, cards);
        int n = cards.Count();
        int i;

        float spread = m_Look.HandSpread;
        if (n > 1 && spread * (n - 1) > m_Look.HandFan)
            spread = m_Look.HandFan / (n - 1);
        float r = m_Look.HandR;
        float mid = (n - 1) * 0.5;
        array<string> moves = new array<string>();
        for (i = 0; i < n; i++)
        {
            string card = cards[i];
            MovesOf(card, moves);
            float a = (i - mid) * spread;
            float ar = a * Math.DEG2RAD;
            float x = m_Look.HandX + r * Math.Sin(ar);
            float y = m_Look.HandY + r - r * Math.Cos(ar);

            // A card that can be played glows; the card picked up glows
            // brighter and stands out of the fan. While a sleight that takes
            // a card of the hand is armed, every card glows its colour, and
            // nothing else.
            int glow = 0;
            bool sleightHand = m_Sleight == OZG_Const.SLEIGHT_THROW;
            if (m_Sleight == OZG_Const.SLEIGHT_SWAP && m_PickKey != "")
                sleightHand = true;
            if (sleightHand)
                glow = m_Look.SleightGlow;
            else if (m_Sleight != "")
                glow = 0;
            else if (card == m_Picked)
                glow = m_Look.PickGlow;
            else if (Playing() && moves.Count() > 0)
                glow = m_Look.Glow;

            // The hand is drawn sorted: a card's look is found by the card,
            // not by its place in the row.
            string cardFace = "c_" + OZG_Cards.FaceOf(card);
            int cardLook = LookAt(m_Own.HeldLooks, m_Own.Held.Find(card));
            Widget pic = Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true, SlotOfHand(i));
            if (!pic)
                continue;
            // A card of the hand this player notices shows its speck (design
            // §4.5), on the left side the next card leaves.
            int handNotice = NoticeOn(card);
            OZG_Cover nextOver = null;
            if (i < n - 1)
                nextOver = OZG_Cover.MakeFan(r, spread, m_Look.CardW, m_Look.CardH);
            if (handNotice >= 0)
                SpeckFace(pic, m_Look.CardW, m_Look.CardH, m_V.Notices[handNotice], nextOver);
            m_HandW.Insert(pic);
            m_HandCodes.Insert(card);
            m_HandX.Insert(x);
            m_HandY.Insert(y);
            m_HandA.Insert(a);
        }
        for (i = 0; i < m_HandW.Count(); i++)
            PlaceHand(i);
    }

    // How far a hand card stands out of the fan, along its own length.
    protected float LiftOf(int i)
    {
        if (m_HandCodes[i] == m_Picked)
            return m_Look.PickLift;
        if (i == m_Hover)
            return m_Look.HoverLift;
        return 0;
    }

    protected void HandMiddle(int i, out float x, out float y)
    {
        float lift = LiftOf(i);
        float ar = m_HandA[i] * Math.DEG2RAD;
        x = m_HandX[i] + lift * Math.Sin(ar);
        y = m_HandY[i] - lift * Math.Cos(ar);
    }

    protected void PlaceHand(int i)
    {
        float x;
        float y;
        HandMiddle(i, x, y);
        Place(m_HandW[i], x, y, BoxOf(m_Look.CardW, m_Look.CardH), m_HandA[i]);
    }

    // The hand card under (x, y), the uppermost first; -1 for none.
    protected int HandAt(float x, float y)
    {
        for (int i = m_HandW.Count() - 1; i >= 0; i--)
        {
            float cx;
            float cy;
            HandMiddle(i, cx, cy);
            if (OnCard(x, y, cx, cy, m_Look.CardW, m_Look.CardH, m_HandA[i]))
                return i;
        }
        return -1;
    }

    protected int AttackAt(float x, float y)
    {
        for (int i = m_AtkX.Count() - 1; i >= 0; i--)
        {
            if (OnCard(x, y, m_AtkX[i], m_AtkY[i], m_AtkW[i], m_AtkH[i], 0))
                return i;
        }
        return -1;
    }

    // The card under the pointer rises a little out of the fan.
    protected void TrackHover()
    {
        if (m_HandW.Count() == 0)
        {
            m_Hover = -1;
            return;
        }
        float x;
        float y;
        PointerAt(x, y);
        // Asked with the card under the pointer standing out: a card raised
        // keeps the pointer while the pointer stays on its raised place.
        int at = HandAt(x, y);
        if (at == m_Hover)
            return;
        int was = m_Hover;
        m_Hover = at;
        if (was >= 0 && was < m_HandW.Count())
            PlaceHand(was);
        if (at >= 0)
            PlaceHand(at);
    }

    // The buttons of the moment, and the line that says whose move it is.
    protected void PaintMoment()
    {
        if (Generic())
        {
            GenPaintMoment();
            return;
        }
        bool pass = CanPlay("pass");
        bool take = CanPlay("take");
        bool moveOn = m_Picked != "" && CanPlay("t:" + m_Picked);
        array<Widget> shown = new array<Widget>();
        if (take)
            shown.Insert(m_BtnTake);
        if (pass)
            shown.Insert(m_BtnPass);
        if (moveOn)
            shown.Insert(m_BtnMove);
        array<Widget> all = new array<Widget>();
        all.Insert(m_BtnTake);
        all.Insert(m_BtnPass);
        all.Insert(m_BtnMove);
        for (int b = 0; b < all.Count(); b++)
        {
            Widget btn = all[b];
            if (!btn)
                continue;
            int at = shown.Find(btn);
            btn.Show(at >= 0);
            if (at >= 0)
                btn.SetPos(m_Look.MomentX * m_K, (m_Look.MomentY - at * m_Look.MomentStep) * m_K);
        }

        string hint = "";
        if (m_Picked != "")
            hint = Tr("STR_OZG_ST_PICK");
        m_Hint.SetText(hint);

        string status = "";
        if (m_V.St == OZG_Const.ST_LOBBY)
        {
            int seated = 0;
            for (int i = 0; i < m_V.Seats.Count(); i++)
            {
                if (m_V.Seats[i].St == OZG_Const.SEAT_TAKEN)
                    seated++;
            }
            if (seated < 2)
                status = Tr("STR_OZG_ST_NEED_TWO");
            else if (m_V.IsHost == 1)
                status = Tr("STR_OZG_ST_LOBBY_HOST");
            else
                status = Tr("STR_OZG_ST_LOBBY");
        }
        else if (m_Pub && m_Pub.Ended == 1)
        {
            if (m_V.IsHost == 1)
                status = Tr("STR_OZG_ST_LOBBY_HOST");
            else
                status = Tr("STR_OZG_ST_LOBBY");
        }
        else if (m_Pub)
        {
            bool mine = false;
            if (m_V.Mine >= 0 && m_Pub.Awaited)
                mine = m_Pub.Awaited.Find(m_V.Mine) >= 0;
            if (mine)
            {
                if (m_V.Mine == m_Pub.DefenderSeat)
                    status = Tr("STR_OZG_ST_DEFEND");
                else if (m_Pub.AttackCards.Count() == 0)
                    status = Tr("STR_OZG_ST_ATTACK");
                else
                    status = Tr("STR_OZG_ST_THROW");
            }
            else
            {
                string names = "";
                if (m_Pub.Awaited)
                {
                    for (int a = 0; a < m_Pub.Awaited.Count(); a++)
                    {
                        if (names != "")
                            names = names + ", ";
                        names = names + SeatName(m_Pub.Awaited[a]);
                    }
                }
                status = Tr1("STR_OZG_ST_WAIT", names);
            }
        }
        // The dealer's frame says what he does (its title).
        if (Dealing())
            status = "";
        m_Status.SetText(Caps(status));
    }

    // The player a seat line of the log names: "<seat> <name>" since
    // 2026-11-25 (whoever sat there then), the seat's player now in an older
    // line that has the number alone.
    protected string LogWho(string rest)
    {
        int space = rest.IndexOf(" ");
        if (space <= 0)
            return SeatName(rest.ToInt());
        return rest.Substring(space + 1, rest.Length() - space - 1);
    }

    // One line of the log, worded.
    protected string LogWord(string line)
    {
        int space = line.IndexOf(" ");
        string head = line;
        string rest = "";
        if (space > 0)
        {
            head = line.Substring(0, space);
            rest = line.Substring(space + 1, line.Length() - space - 1);
        }

        string later = GenLogWord(head, rest);
        if (later != "")
            return later;
        if (head == "deal")
            return Tr("STR_OZG_LOG_DEAL");
        if (head == "sit")
            return Tr1("STR_OZG_LOG_SIT", LogWho(rest));
        if (head == "over")
            return Tr1("STR_OZG_LOG_OVER", LogWho(rest));
        int shownNo = rest.ToInt() + 1;
        if (head == "stand")
            return Tr1("STR_OZG_LOG_STAND", shownNo.ToString());
        if (head == "drop")
            return Tr1("STR_OZG_LOG_DROP", shownNo.ToString());
        if (head == "left")
            return Tr1("STR_OZG_LOG_LEFT", shownNo.ToString());
        if (head == "host")
            return Tr1("STR_OZG_LOG_HOST", LogWho(rest));
        if (head == "opt")
            return Tr1("STR_OZG_WIN_VARIANT", VariantLabel());
        if (head == "end")
        {
            if (rest == "durak=-1" || rest == "")
                return Tr("STR_OZG_ST_OVER_NONE");
            int eq = rest.IndexOf("=");
            if (eq > 0)
            {
                string named = rest.Substring(eq + 1, rest.Length() - eq - 1);
                return Tr1("STR_OZG_ST_OVER_DURAK", SeatName(named.ToInt()));
            }
            return Tr("STR_OZG_ST_OVER_NONE");
        }

        // A move: "<seat> <move>".
        string who = SeatName(head.ToInt());
        if (rest == "take")
            return Tr1("STR_OZG_LOG_TAKE", who);
        if (rest == "pass")
            return Tr1("STR_OZG_LOG_PASS", who);
        string card = OZG_Cards.OfMove(rest);
        if (card != "")
        {
            string kind = rest.Substring(0, 1);
            if (kind == "a")
                return Tr2("STR_OZG_LOG_ATTACK", who, CardWord(card));
            if (kind == "d")
                return Tr2("STR_OZG_LOG_BEAT", who, CardWord(card));
            if (kind == "t")
                return Tr2("STR_OZG_LOG_TRANSFER", who, CardWord(card));
        }
        return line;
    }

    protected void PaintLog()
    {
        // A shorter log while the panel of sleights shows under it.
        int lines = m_LogLines.Count();
        if (m_PanelShown && m_Look.LogShort < lines)
            lines = m_Look.LogShort;
        if (Generic())
        {
            GenPaintLog(lines);
            return;
        }
        int have = m_V.Log.Count();
        int from = have - lines;
        if (from < 0)
            from = 0;
        for (int i = 0; i < m_LogLines.Count(); i++)
        {
            string text = "";
            if (i < lines && from + i < have)
                text = LogWord(m_V.Log[from + i]);
            if (m_LogLines[i])
                m_LogLines[i].SetText(text);
        }
    }

    // The right column: what this player may do now, stacked with no gaps.
    protected void PaintColumn()
    {
        bool host = m_V.IsHost == 1;
        bool seated = m_V.Mine >= 0;
        bool playing = Playing();

        string hostLine = Tr("STR_OZG_WIN_NO_HOST");
        if (m_V.HostName != "")
            hostLine = Tr1("STR_OZG_WIN_HOST", m_V.HostName);
        m_HostLine.SetText(Caps(hostLine));

        // The seat the host picked by its plate.
        bool pickTaken = false;
        bool pickLeft = false;
        bool pickInGame = false;
        if (host && m_PickedSeat >= 0 && m_PickedSeat < m_V.Seats.Count())
        {
            int st = m_V.Seats[m_PickedSeat].St;
            pickTaken = st == OZG_Const.SEAT_TAKEN || st == OZG_Const.SEAT_KEPT;
            pickLeft = st == OZG_Const.SEAT_LEFT;
            pickInGame = playing && m_V.Dealt.Find(m_PickedSeat) >= 0;
        }

        array<Widget> shown = new array<Widget>();
        // A game with no variant (bridge) has no button for it.
        string variant = VariantLabel();
        SetLabel(m_BtnVariant, Caps(Tr1("STR_OZG_WIN_VARIANT", variant)));
        if (host && !playing)
        {
            if (variant != "")
                shown.Insert(m_BtnVariant);
            shown.Insert(m_BtnDeal);
        }
        if (!host && seated && m_V.HostFree == 1)
            shown.Insert(m_BtnTakeHost);
        if (pickTaken)
        {
            shown.Insert(m_BtnKickDiscard);
            if (pickInGame)
                shown.Insert(m_BtnKickLeave);
        }
        if (pickLeft)
            shown.Insert(m_BtnDropLeft);
        if (seated)
        {
            string pose = OZG_PoseDriver.LabelNow(m_V);
            if (pose != "")
            {
                SetLabel(m_BtnPose, Caps(Tr1("STR_OZG_WIN_POSE", pose)));
                shown.Insert(m_BtnPose);
            }
            if (m_AskStand)
            {
                shown.Insert(m_BtnStandDiscard);
                shown.Insert(m_BtnStandLeave);
            }
            else
            {
                shown.Insert(m_BtnStand);
            }
        }
        if (host)
        {
            if (m_AskFold)
                shown.Insert(m_BtnFoldYes);
            else
                shown.Insert(m_BtnFold);
        }

        array<Widget> all = new array<Widget>();
        all.Insert(m_BtnVariant);
        all.Insert(m_BtnDeal);
        all.Insert(m_BtnTakeHost);
        all.Insert(m_BtnKickDiscard);
        all.Insert(m_BtnKickLeave);
        all.Insert(m_BtnDropLeft);
        all.Insert(m_BtnPose);
        all.Insert(m_BtnStand);
        all.Insert(m_BtnStandDiscard);
        all.Insert(m_BtnStandLeave);
        all.Insert(m_BtnFold);
        all.Insert(m_BtnFoldYes);

        // The panel of sleights takes the column under a shorter log while a
        // deal is played (the canvas's boards); the host's line and the
        // buttons go under it. When the moment's buttons do not fit there --
        // the choices of standing up, the host's on a seat -- the panel gives
        // way for that moment.
        m_PanelShown = SleightsInPlay() && shown.Count() <= m_Look.StackFits2;
        float stackY = m_Look.StackY;
        float hostY = m_Look.HostLineY;
        if (m_PanelShown)
        {
            stackY = m_Look.StackY2;
            hostY = m_Look.HostLineY2;
        }
        m_HostLine.SetPos(m_Look.StackX * m_K, hostY * m_K);
        if (m_ColRule)
            m_ColRule.Show(!m_PanelShown);
        // The game the host picks, and at a game for stakes the chips, take
        // the first rows of the stack.
        int extra = GenColumn(stackY);

        for (int i = 0; i < all.Count(); i++)
        {
            Widget b = all[i];
            if (!b)
                continue;
            int at = shown.Find(b);
            b.Show(at >= 0);
            if (at >= 0)
                b.SetPos(m_Look.StackX * m_K, (stackY + (at + extra) * m_Look.StackStep) * m_K);
        }
    }

    // ------------------------------------------------------------ sleights (card sharp's design §6, §10)

    // Sleights in play are open to this player now: dealt in, a deal in play,
    // and the head says how many he may (none with Cheating 0).
    protected bool SleightsInPlay()
    {
        return m_V && m_V.SleightsMost > 0 && MineInGame();
    }

    // The dealer's frame shows: between deals, to the host on his own seat,
    // when the head says he may do sleights.
    protected bool Dealing()
    {
        if (!m_V || m_V.St == OZG_Const.ST_PLAYING)
            return false;
        return m_V.IsHost == 1 && m_V.Mine >= 0 && m_V.SleightsMost > 0;
    }

    protected bool PlayMode(string mode)
    {
        return mode == OZG_Const.SLEIGHT_SLIP || mode == OZG_Const.SLEIGHT_SWAP || mode == OZG_Const.SLEIGHT_THROW;
    }

    protected bool DealMode(string mode)
    {
        return mode == OZG_Const.SLEIGHT_DECK_IN || mode == OZG_Const.SLEIGHT_DECK_OUT;
    }

    // The dealer's sleights planned: a card in one, a card out one, the
    // chosen cards one (design §6.5).
    protected int Planned()
    {
        int n = m_PlanIn.Count() + m_PlanOut.Count();
        if (m_PlanPick.Count() > 0)
            n++;
        return n;
    }

    // How many sleights this player has left now.
    protected int SleightsLeft()
    {
        if (Dealing())
            return m_V.SleightsMost - Planned();
        if (SleightsInPlay())
            return m_V.SleightsMost - m_V.SleightsUsed;
        return 0;
    }

    protected void ClearPlan()
    {
        m_PlanIn.Clear();
        m_PlanOut.Clear();
        m_PlanOutFace.Clear();
        m_PlanOutLook.Clear();
        m_PlanPick.Clear();
    }

    // The deal's stack for the server (OZG_WAsk.Stack); "" with nothing
    // planned.
    protected string StackText()
    {
        if (Planned() == 0)
            return "";
        string text = "in=" + Joined3(m_PlanIn) + ";out=" + Joined3(m_PlanOut);
        return text + ";pick=" + Joined3(m_PlanPick);
    }

    protected string Joined3(array<string> keys)
    {
        string line = "";
        for (int i = 0; i < keys.Count(); i++)
        {
            if (i > 0)
                line = line + ",";
            line = line + keys[i];
        }
        return line;
    }

    // A hint of two lines: the string's "|" parts them.
    protected void Hint(array<TextWidget> lines, string text, bool hot)
    {
        string first = text;
        string second = "";
        int bar = text.IndexOf("|");
        if (bar >= 0)
        {
            first = text.Substring(0, bar);
            second = text.Substring(bar + 1, text.Length() - bar - 1);
        }
        int ink = m_Look.HintInk;
        if (hot)
            ink = m_Look.HintHot;
        for (int i = 0; i < lines.Count(); i++)
        {
            if (!lines[i])
                continue;
            if (i == 0)
                lines[i].SetText(first);
            else
                lines[i].SetText(second);
            lines[i].SetColor(ink);
        }
    }

    // A button of the sleights tinted: armed (its rim pink or rust), spent
    // (the deal's sleights used up), ready.
    protected void SleightButton(Widget b, bool armed, bool spent)
    {
        if (!b)
            return;
        string name = b.GetName();
        Widget ground = b.FindAnyWidget(name + "Bg");
        Widget rim = b.FindAnyWidget(name + "Edge");
        TextWidget label = TextWidget.Cast(b.FindAnyWidget(name + "Text"));
        int bg = m_Look.BtnBg;
        if (armed)
            bg = m_Look.BtnBgArmed;
        else if (spent)
            bg = m_Look.BtnBgSpent;
        if (ground)
            ground.SetColor(bg);
        if (rim && m_Look.Caps)
        {
            if (armed)
                rim.SetColor(m_Look.Hot);
            else
                rim.SetColor(m_Look.BtnEdge);
        }
        else if (rim)
        {
            rim.Show(armed);
        }
        if (label && spent && !armed)
            label.SetColor(m_Look.BtnInkSpent);
        else if (label)
            label.SetColor(m_Look.BtnInk);
    }

    protected void PaintSleights()
    {
        bool inPlay = SleightsInPlay();
        bool dealing = Dealing();
        if (!inPlay && PlayMode(m_Sleight))
            m_Sleight = "";
        if (!dealing && DealMode(m_Sleight))
            m_Sleight = "";
        if (m_Sleight != OZG_Const.SLEIGHT_SWAP)
            m_PickKey = "";
        if (m_Sleight != OZG_Const.SLEIGHT_THROW)
            m_Thrown = 0;
        PaintPockets(inPlay || dealing);
        PaintPanel(m_PanelShown);
        PaintDealer(dealing);
    }

    // The strip "in your pockets" (design §10): the cards the player carries,
    // faces up; less those the dealer plans into the deck, and with those he
    // plans out of it -- a back for a card by chance.
    protected void PaintPockets(bool show)
    {
        ForgetDrawn(m_PocketPics);
        ClearList(m_PocketPics);
        m_PocketShown.Clear();
        m_PocketX.Clear();
        if (m_Pockets)
            m_Pockets.Show(show);
        if (!show)
            return;
        int k;
        for (k = 0; k < m_PocketCards.Count(); k++)
        {
            if (m_PlanIn.Find(m_PocketCards[k].Key) < 0)
                m_PocketShown.Insert(m_PocketCards[k]);
        }
        for (k = 0; k < m_PlanOut.Count(); k++)
            m_PocketShown.Insert(OZG_PocketCard.Make(m_PlanOut[k], m_PlanOutFace[k], m_PlanOutLook[k], true));
        int n = m_PocketShown.Count();
        if (m_PocketsNone)
            m_PocketsNone.Show(n == 0);
        bool armed = m_Sleight == OZG_Const.SLEIGHT_SLIP || m_Sleight == OZG_Const.SLEIGHT_SWAP;
        if (m_Sleight == OZG_Const.SLEIGHT_DECK_IN)
            armed = true;
        if (m_PocketsEdgeHot)
            m_PocketsEdgeHot.Show(armed);
        if (m_PocketsEdge && !m_Look.Caps)
            m_PocketsEdge.Show(!armed);
        float step = PocketStepFor(n);
        for (k = 0; k < n; k++)
        {
            OZG_PocketCard c = m_PocketShown[k];
            float x = m_Look.PocketX + k * step;
            string picture = PictureOfBack(c.Look);
            if (c.Face != "")
                picture = PictureOfFace(c.Face, c.Look);
            int glow = 0;
            if (armed && !c.Planned)
                glow = m_Look.SleightGlow;
            bool picked = c.Key == m_PickKey;
            if (picked)
                glow = m_Look.SleightPick;
            Widget made = Card(m_PocketPics, picture, x, m_Look.PocketY, m_Look.PocketW, m_Look.PocketH, 0, glow, true, SlotOfPocket(k));
            // The card picked for a swap: the mock-up's solid ring round it.
            Widget ring = null;
            if (made)
                ring = made.FindAnyWidget("OZG_CardRing");
            if (ring && picked)
            {
                ring.SetColor(m_Look.Hot);
                ring.Show(true);
            }
            m_PocketX.Insert(x);
        }
    }

    // The step between two cards of the strip, when it holds n: the mock-up's
    // 68 units while three fit, closer when more must.
    protected float PocketStepFor(int n)
    {
        float step = m_Look.PocketStep;
        float room = m_Look.PocketRoom - m_Look.PocketW;
        if (n > 1 && step * (n - 1) > room)
            step = room / (n - 1);
        return step;
    }

    static string SlotOfPocket(int k)
    {
        return "p" + k.ToString();
    }

    // Where the k-th card of the strip lies, of n.
    OZG_CardSpot SpotOfPocket(int k, int n)
    {
        float x = m_Look.PocketX + k * PocketStepFor(n);
        return OZG_CardSpot.Make(x, m_Look.PocketY, m_Look.PocketW, m_Look.PocketH, 0);
    }

    // The deck the dealer's frame lays at the stock's place: its top card.
    OZG_CardSpot SpotOfDeck()
    {
        float sx;
        float sy;
        StockLayerAt(5, sx, sy);
        return OZG_CardSpot.Make(sx, sy, m_Look.PileW, m_Look.PileH, 0);
    }

    // The panel "Sleight of hand" (the canvas's boards): the count of the
    // deal, three buttons, the hint.
    protected void PaintPanel(bool show)
    {
        array<Widget> parts = new array<Widget>();
        parts.Insert(m_SleightRule);
        parts.Insert(m_SleightRule2);
        parts.Insert(m_SleightTitle);
        parts.Insert(m_SleightCount);
        parts.Insert(m_BtnSlip);
        parts.Insert(m_BtnSwap);
        parts.Insert(m_BtnToss);
        int i;
        for (i = 0; i < parts.Count(); i++)
        {
            if (parts[i])
                parts[i].Show(show);
        }
        for (i = 0; i < m_SleightHint.Count(); i++)
        {
            if (m_SleightHint[i])
                m_SleightHint[i].Show(show);
        }
        if (!show)
            return;
        int used = m_V.SleightsUsed;
        int most = m_V.SleightsMost;
        m_SleightCount.SetText(Tr2("STR_OZG_WIN_SLEIGHTS_USED", used.ToString(), most.ToString()));
        bool spent = used >= most;
        bool throwing = m_Sleight == OZG_Const.SLEIGHT_THROW;
        SleightButton(m_BtnSlip, m_Sleight == OZG_Const.SLEIGHT_SLIP, spent);
        SleightButton(m_BtnSwap, m_Sleight == OZG_Const.SLEIGHT_SWAP, spent);
        SleightButton(m_BtnToss, throwing, spent);
        if (throwing)
            SetLabel(m_BtnToss, Caps(Tr("STR_OZG_WIN_TOSS_DONE")));
        else
            SetLabel(m_BtnToss, Caps(Tr("STR_OZG_WIN_TOSS")));

        string key = "STR_OZG_HINT_SLEIGHT";
        if (m_Sleight == OZG_Const.SLEIGHT_SLIP)
            key = "STR_OZG_HINT_SLIP";
        else if (m_Sleight == OZG_Const.SLEIGHT_SWAP && m_PickKey == "")
            key = "STR_OZG_HINT_SWAP_POCKET";
        else if (m_Sleight == OZG_Const.SLEIGHT_SWAP)
            key = "STR_OZG_HINT_SWAP_HAND";
        else if (throwing)
            key = "STR_OZG_HINT_TOSS";
        else if (spent)
            key = "STR_OZG_HINT_NO_MORE";
        Hint(m_SleightHint, Tr(key), m_Sleight != "");
    }

    // The dealer's frame (design §6.5, §10; the mock-up's table_deal): put
    // into the deck, draw out of it, and the cards he reads with a tick to
    // deal them to himself; the deck lies at the stock's place.
    protected void PaintDealer(bool show)
    {
        ForgetDrawn(m_DealPics);
        ClearList(m_DealPics);
        m_TileKeys.Clear();
        m_TileX.Clear();
        m_TileY.Clear();
        if (m_Deal)
            m_Deal.Show(show);
        if (!show)
            return;
        int i;
        // What the plan names that is no longer there goes from it.
        array<string> reads = new array<string>();
        for (i = 0; i < m_V.DeckReads.Count(); i++)
            reads.Insert(m_V.DeckReads[i].Key);
        for (i = m_PlanPick.Count() - 1; i >= 0; i--)
        {
            if (reads.Find(m_PlanPick[i]) < 0)
                m_PlanPick.Remove(i);
        }

        int planned = Planned();
        int most = m_V.SleightsMost;
        m_DealCount.SetText(Tr2("STR_OZG_WIN_SLEIGHTS_USED", planned.ToString(), most.ToString()));
        bool spent = planned >= most;
        m_BtnDeckIn.Show(true);
        m_BtnDeckOut.Show(true);
        SleightButton(m_BtnDeckIn, m_Sleight == OZG_Const.SLEIGHT_DECK_IN, spent);
        SleightButton(m_BtnDeckOut, m_Sleight == OZG_Const.SLEIGHT_DECK_OUT, spent);

        // The deck, six backs, the top one lit while a card is to be drawn.
        bool drawing = m_Sleight == OZG_Const.SLEIGHT_DECK_OUT;
        for (int k = 0; k < 6; k++)
        {
            float sx;
            float sy;
            StockLayerAt(k, sx, sy);
            int deckGlow = 0;
            if (drawing && k == 5)
                deckGlow = m_Look.SleightGlow;
            Card(m_DealPics, PictureOfBack(-1), sx, sy, m_Look.PileW, m_Look.PileH, 0, deckGlow, true, "", REGION_STOCK);
        }

        // The cards he reads, ten at most, a tick under each.
        int n = m_V.DeckReads.Count();
        if (n > 2 * m_Look.TilesPerRow)
            n = 2 * m_Look.TilesPerRow;
        if (n == 0)
            m_DealCaption.SetText(Tr("STR_OZG_WIN_DEAL_NONE"));
        else
            m_DealCaption.SetText(Tr1("STR_OZG_WIN_DEAL_SELF", m_V.PicksMost.ToString()));
        string tickRoot = "OZG_TickClassic";
        if (m_Style == "stalker")
            tickRoot = "OZG_TickStalker";
        int shown = 0;
        for (i = 0; i < n; i++)
        {
            OZG_WDeckCard c = m_V.DeckReads[i];
            if (m_PlanOut.Find(c.Key) >= 0)
                continue;
            int row = shown / m_Look.TilesPerRow;
            int col = shown % m_Look.TilesPerRow;
            shown++;
            float x = m_Look.TileX + col * m_Look.TileStep;
            float y = m_Look.TileY + row * m_Look.TileRowStep;
            bool ticked = m_PlanPick.Find(c.Key) >= 0;
            int glow = 0;
            if (drawing)
                glow = m_Look.SleightGlow;
            Widget tile = Card(m_DealPics, PictureOfBack(c.Look), x, y, m_Look.FanW, m_Look.FanH, 0, glow, true, SlotOfTile(c.Key));
            // His mark in the back's pattern, and the face he knows written
            // on a label in its corner, as the mock-up of the deal drew it:
            // a choice he must read at a glance, not the table's faint pencil.
            if (tile)
                SignBack(tile, m_Look.FanW, m_Look.FanH, c.Look, c.Sign);
            FaceLabel(c.Face, x - m_Look.FanW * 0.5 + 2, y - m_Look.FanH * 0.5 + 3);
            Widget box = MakeLayout("OpenZone_Games/gui/layouts/ozg_tick_" + m_Style + ".layout", m_TableBox, tickRoot);
            if (box)
            {
                float side = 22;
                box.SetSize(side * m_K, side * m_K);
                box.SetPos((x - side * 0.5) * m_K, (y + m_Look.TickDown - side * 0.5) * m_K);
                Widget rim = box.FindAnyWidget("OZG_TickEdge");
                Widget mark = box.FindAnyWidget("OZG_TickMark");
                if (mark)
                    mark.Show(ticked);
                if (rim && ticked)
                    rim.SetColor(m_Look.TickOn);
                else if (rim)
                    rim.SetColor(m_Look.TickIdle);
                m_DealPics.Insert(box);
            }
            m_TileKeys.Insert(c.Key);
            m_TileX.Insert(x);
            m_TileY.Insert(y);
        }

        string key = "STR_OZG_HINT_DEAL";
        if (m_Sleight == OZG_Const.SLEIGHT_DECK_IN)
            key = "STR_OZG_HINT_DECK_IN";
        else if (drawing)
            key = "STR_OZG_HINT_DECK_OUT";
        else if (spent)
            key = "STR_OZG_HINT_NO_MORE";
        Hint(m_DealHint, Tr(key), m_Sleight != "");
    }

    static string SlotOfTile(string key)
    {
        return "r" + key;
    }

    // A face on a pale label, its top-left at (x, y): the rank as the cards
    // print it, the suit's own picture.
    protected void FaceLabel(string face, float x, float y)
    {
        string root = "OZG_LabelClassic";
        if (m_Style == "stalker")
            root = "OZG_LabelStalker";
        Widget label = MakeLayout("OpenZone_Games/gui/layouts/ozg_label_" + m_Style + ".layout", m_TableBox, root);
        if (!label)
            return;
        label.SetSize(36 * m_K, 22 * m_K);
        label.SetPos(x * m_K, y * m_K);
        TextWidget rank = TextWidget.Cast(label.FindAnyWidget("OZG_LabelRank"));
        ImageWidget suit = ImageWidget.Cast(label.FindAnyWidget("OZG_LabelSuit"));
        string r = face.Substring(0, 1);
        if (r == "T")
            r = "10";
        string s = face.Substring(1, 1);
        if (rank)
        {
            rank.SetText(r);
            if (s == "h" || s == "d")
                rank.SetColor(ARGB(255, 200, 26, 40));
            else
                rank.SetColor(ARGB(255, 24, 22, 24));
        }
        if (suit)
        {
            suit.LoadImageFile(0, Picture("suit_" + s));
            suit.SetImage(0);
        }
        m_DealPics.Insert(label);
    }

    // The pockets as they are now, read every half second: a change redraws
    // the strip.
    protected void RefreshPockets(float dt)
    {
        m_PocketsAge = m_PocketsAge + dt;
        if (m_PocketsAge < 0.5)
            return;
        m_PocketsAge = 0;
        OZG_PocketsRead.Into(m_PocketCards);
        // A card planned into the deck that left the pockets is out of the plan.
        for (int i = m_PlanIn.Count() - 1; i >= 0; i--)
        {
            bool still = false;
            for (int k = 0; k < m_PocketCards.Count(); k++)
            {
                if (m_PocketCards[k].Key == m_PlanIn[i])
                    still = true;
            }
            if (!still)
                m_PlanIn.Remove(i);
        }
        string line = OZG_PocketsRead.Line(m_PocketCards);
        if (line == m_PocketsLine)
            return;
        m_PocketsLine = line;
        if (m_V)
            PaintSleights();
    }

    // ---- the hit tests

    protected int PocketAt(float x, float y)
    {
        if (!m_Pockets || !m_Pockets.IsVisible())
            return -1;
        for (int k = m_PocketX.Count() - 1; k >= 0; k--)
        {
            if (OnCard(x, y, m_PocketX[k], m_Look.PocketY, m_Look.PocketW, m_Look.PocketH, 0))
                return k;
        }
        return -1;
    }

    protected int TileAt(float x, float y)
    {
        for (int k = m_TileKeys.Count() - 1; k >= 0; k--)
        {
            if (OnCard(x, y, m_TileX[k], m_TileY[k], m_Look.FanW, m_Look.FanH, 0))
                return k;
            float tx = m_TileX[k];
            float ty = m_TileY[k] + m_Look.TickDown;
            if (OnCard(x, y, tx, ty, 26, 26, 0))
                return k;
        }
        return -1;
    }

    protected bool OnDeck(float x, float y)
    {
        OZG_CardSpot top = SpotOfDeck();
        return OnCard(x, y, top.X, top.Y, top.W + 8, top.H + 8, 0);
    }

    // ---- what the player does with them

    // The sleight asked for (design §9): the card of his pockets by its key,
    // the card of his hand by its name.
    protected void SendSleight(string op, string key, string card, string how)
    {
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = m_PlaceId;
        ask.CardKey = key;
        ask.Move = card;
        ask.How = how;
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, op, json);
    }

    protected void ArmSleight(string mode)
    {
        if (m_Sleight == mode)
        {
            m_Sleight = "";
        }
        else
        {
            if (SleightsLeft() <= 0)
                return;
            m_Sleight = mode;
        }
        m_PickKey = "";
        m_Thrown = 0;
        m_Picked = "";
        RepaintSleights();
    }

    // "Throw to the discards" pressed again, as "Done": the throw ends.
    protected void TossButton()
    {
        if (m_Sleight == OZG_Const.SLEIGHT_THROW)
        {
            m_Sleight = "";
            m_Thrown = 0;
            RepaintSleights();
            return;
        }
        ArmSleight(OZG_Const.SLEIGHT_THROW);
    }

    protected void RepaintSleights()
    {
        PaintTable();
        PaintHand();
        PaintMoment();
        PaintSleights();
    }

    protected void ClickPocket(int k)
    {
        OZG_PocketCard c = m_PocketShown[k];
        if (c.Planned)
            return;
        OZG_CardSpot spot = SpotOfPocket(k, m_PocketShown.Count());
        string picture = PictureOfFace(c.Face, c.Look);
        if (m_Sleight == OZG_Const.SLEIGHT_SLIP)
        {
            OZG_SleightFly f = new OZG_SleightFly();
            f.Kind = OZG_Const.SLEIGHT_SLIP;
            f.PocketSpot = spot;
            f.PocketPicture = picture;
            m_Flying.Insert(f);
            SendSleight(OZG_Const.OP_SLIP, c.Key, "", "");
            m_Sleight = "";
        }
        else if (m_Sleight == OZG_Const.SLEIGHT_SWAP)
        {
            if (m_PickKey == c.Key)
                m_PickKey = "";
            else
                m_PickKey = c.Key;
            RepaintSleights();
            return;
        }
        else if (m_Sleight == OZG_Const.SLEIGHT_DECK_IN)
        {
            // Planned now, done with "Deal": the card flies into the deck
            // in his own window, with its sound (design §6.1).
            m_PlanIn.Insert(c.Key);
            Fly(picture, PictureOfBack(c.Look), spot, SpotOfDeck(), 0, 0.45, "");
            OZG_Sounds.PlayLocal("deckin", null);
            m_Sleight = "";
        }
        else
        {
            return;
        }
        RepaintSleights();
    }

    protected void ClickHandSleight(int i)
    {
        string card = m_HandCodes[i];
        float hx;
        float hy;
        HandMiddle(i, hx, hy);
        OZG_CardSpot spot = OZG_CardSpot.Make(hx, hy, m_Look.CardW, m_Look.CardH, m_HandA[i]);
        int look = LookAt(m_Own.HeldLooks, m_Own.Held.Find(card));
        string picture = PictureOfFace(card, look);
        OZG_SleightFly f = new OZG_SleightFly();
        f.HandSpot = spot;
        f.HandPicture = picture;
        f.HandLook = look;
        if (m_Sleight == OZG_Const.SLEIGHT_THROW)
        {
            f.Kind = OZG_Const.SLEIGHT_THROW;
            m_Flying.Insert(f);
            string how = "";
            if (m_Thrown > 0)
                how = OZG_Const.HOW_MORE;
            SendSleight(OZG_Const.OP_THROW, "", card, how);
            m_Thrown++;
            if (m_Thrown >= OZG_Const.THROW_MOST)
            {
                m_Sleight = "";
                m_Thrown = 0;
            }
        }
        else if (m_Sleight == OZG_Const.SLEIGHT_SWAP && m_PickKey != "")
        {
            int at = -1;
            for (int k = 0; k < m_PocketShown.Count(); k++)
            {
                if (m_PocketShown[k].Key == m_PickKey)
                    at = k;
            }
            if (at < 0)
                return;
            OZG_PocketCard pocket = m_PocketShown[at];
            f.Kind = OZG_Const.SLEIGHT_SWAP;
            f.PocketSpot = SpotOfPocket(at, m_PocketShown.Count());
            f.PocketPicture = PictureOfFace(pocket.Face, pocket.Look);
            m_Flying.Insert(f);
            SendSleight(OZG_Const.OP_SWAP, m_PickKey, card, "");
            m_Sleight = "";
            m_PickKey = "";
        }
        else
        {
            return;
        }
        RepaintSleights();
    }

    // A card the dealer reads: a tick (dealt to him first), or, while a card
    // is to be drawn, that card out of the deck.
    protected void ClickTile(int t)
    {
        string key = m_TileKeys[t];
        OZG_WDeckCard c = null;
        for (int i = 0; i < m_V.DeckReads.Count(); i++)
        {
            if (m_V.DeckReads[i].Key == key)
                c = m_V.DeckReads[i];
        }
        if (!c)
            return;
        OZG_CardSpot from = OZG_CardSpot.Make(m_TileX[t], m_TileY[t], m_Look.FanW, m_Look.FanH, 0);
        if (m_Sleight == OZG_Const.SLEIGHT_DECK_OUT)
        {
            if (SleightsLeft() <= 0)
                return;
            m_PlanPick.RemoveItem(key);
            m_PlanOut.Insert(key);
            m_PlanOutFace.Insert(c.Face);
            m_PlanOutLook.Insert(c.Look);
            int n = m_PocketShown.Count() + 1;
            Fly(PictureOfBack(c.Look), PictureOfFace(c.Face, c.Look), from, SpotOfPocket(n - 1, n), 0, 0.45, "");
            OZG_Sounds.PlayLocal("deckout", null);
            m_Sleight = "";
            RepaintSleights();
            return;
        }
        if (m_Sleight != "")
            return;
        if (m_PlanPick.Find(key) >= 0)
        {
            m_PlanPick.RemoveItem(key);
            RepaintSleights();
            return;
        }
        // The first card chosen is a sleight of its own; the others ride on it.
        if (m_PlanPick.Count() >= m_V.PicksMost)
            return;
        if (m_PlanPick.Count() == 0 && SleightsLeft() <= 0)
            return;
        m_PlanPick.Insert(key);
        Fly(PictureOfBack(c.Look), "", from, SpotOfDeck(), 0, 0.45, "");
        OZG_Sounds.PlayLocal("pick", null);
        RepaintSleights();
    }

    // The deck, while a card is to be drawn: a card by chance out of it.
    protected void ClickDeck()
    {
        if (m_Sleight != OZG_Const.SLEIGHT_DECK_OUT || SleightsLeft() <= 0)
            return;
        m_PlanOut.Insert(OZG_Const.ANY_CARD);
        m_PlanOutFace.Insert("");
        m_PlanOutLook.Insert(-1);
        int n = m_PocketShown.Count() + 1;
        Fly(PictureOfBack(-1), "", SpotOfDeck(), SpotOfPocket(n - 1, n), 0, 0.45, "");
        OZG_Sounds.PlayLocal("deckout", null);
        m_Sleight = "";
        RepaintSleights();
    }

    // The server did this player's sleight (design §6.1, §9): its sound for
    // him, and the card flown in his window from where he clicked it to where
    // the view now draws it. An answer that comes before its view waits for
    // it.
    void SleightDone(OZG_WSleight w)
    {
        if (!w || w.PlaceId != m_PlaceId)
            return;
        OZG_Sounds.PlayLocal(w.SoundName, OZG_Place.OZG_ById(m_PlaceId));
        if (m_ShownRev < w.Rev)
        {
            m_Answer = w;
            return;
        }
        FlySleight(w);
    }

    // A sleight of his the server refused: its flight is dropped.
    void SleightRefused()
    {
        if (m_Flying.Count() > 0)
            m_Flying.RemoveOrdered(0);
    }

    protected void FlySleight(OZG_WSleight w)
    {
        if (m_Flying.Count() == 0)
            return;
        OZG_SleightFly f = m_Flying[0];
        m_Flying.RemoveOrdered(0);
        if (!f || f.Kind != w.Kind || !m_Own)
            return;
        string trumpSuit = "";
        if (m_Pub)
            trumpSuit = OZG_Cards.SuitOf(m_Pub.TrumpCard);
        array<string> hand = new array<string>();
        SortHand(m_Own.Held, trumpSuit, hand);
        int flown = 0;
        if (w.CameIn != "" && f.PocketSpot)
        {
            int at = hand.Find(w.CameIn);
            if (at >= 0)
            {
                Fly(f.PocketPicture, "", f.PocketSpot, SpotOfHand(at, hand.Count()), 0, 0.45, SlotOfHand(at));
                flown++;
            }
        }
        if (w.Kind == OZG_Const.SLEIGHT_SWAP && f.HandSpot && f.PocketSpot)
        {
            Fly(f.HandPicture, "", f.HandSpot, f.PocketSpot, 0, 0.45, "");
            flown++;
        }
        if (w.Kind == OZG_Const.SLEIGHT_THROW && f.HandSpot && m_Pub)
        {
            int heap = HeapOf(m_Pub.DiscardCount) - 1;
            if (heap < 0)
                heap = 0;
            Fly(f.HandPicture, PictureOfBack(f.HandLook), f.HandSpot, SpotOfDiscards(heap), 0, 0.45, SlotOfHeap(heap));
            flown++;
        }
        OZG_Log.Dbg("sleight " + w.Kind + " flown in his window: " + flown.ToString() + " card(s), rev " + w.Rev.ToString());
    }

    // ------------------------------------------------------------ clicks

    protected void NextVariant()
    {
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (!sample)
            return;
        array<ref OZG_Option> opts = new array<ref OZG_Option>();
        sample.OptionList(opts);
        if (opts.Count() == 0)
            return;
        OZG_Option o = opts[0];
        int at = m_V.OptIds.Find(o.OptionId);
        string chosen = o.DefaultId;
        if (at >= 0 && at < m_V.OptVals.Count())
            chosen = m_V.OptVals[at];
        int v = o.ValueIds.Find(chosen) + 1;
        if (v >= o.ValueIds.Count())
            v = 0;
        Send(OZG_Const.OP_OPT, "", -1, "", o.OptionId, o.ValueIds[v], 0);
    }

    protected void ClickHand(string card)
    {
        if (Generic())
        {
            GenClickCard(card);
            return;
        }
        array<string> moves = new array<string>();
        MovesOf(card, moves);
        if (moves.Count() == 0)
            return;
        if (moves.Count() == 1)
        {
            Play(moves[0]);
            return;
        }
        // More than one move could be meant: the card is picked up, and the
        // next click says which.
        if (m_Picked == card)
            m_Picked = "";
        else
            m_Picked = card;
        PaintTable();
        PaintHand();
        PaintMoment();
    }

    // A click on the cloth: the card under the pointer, if any. The pockets
    // and the dealer's cards come first; while a sleight is armed the hand
    // serves it, and nothing is played.
    protected void ClickTable()
    {
        float x;
        float y;
        PointerAt(x, y);
        if (Generic())
        {
            int held = HandAt(x, y);
            if (held >= 0)
            {
                GenClickCard(m_HandCodes[held]);
                return;
            }
            int shown = GenFaceAt(x, y);
            if (shown >= 0)
                GenClickCard(m_GenFaceCodes[shown]);
            return;
        }
        int pocket = PocketAt(x, y);
        if (pocket >= 0)
        {
            ClickPocket(pocket);
            return;
        }
        if (Dealing())
        {
            int tile = TileAt(x, y);
            if (tile >= 0)
            {
                ClickTile(tile);
                return;
            }
            if (OnDeck(x, y))
            {
                ClickDeck();
                return;
            }
        }
        int at = HandAt(x, y);
        if (at >= 0 && m_Sleight != "")
        {
            ClickHandSleight(at);
            return;
        }
        if (at >= 0)
        {
            ClickHand(m_HandCodes[at]);
            return;
        }
        if (m_Sleight != "")
            return;
        at = AttackAt(x, y);
        if (at >= 0 && m_Picked != "")
        {
            int number = at + 1;
            string move = "d:" + m_Picked + ":" + number.ToString();
            if (CanPlay(move))
                Play(move);
        }
    }

    // ------------------------------------------------------------ the later games (stakes-and-games design §4)
    //
    // Ochko, hold'em, preferans and bridge describe their table in one shape
    // (OZG_GPublic, OZG_GPrivate); this part draws it. A seat is drawn as
    // durak's is -- a fan of backs and a plate -- and the cards it shows face
    // up as a fan of faces; the piles lie in the middle; the hand is fanned as
    // durak's; the moves that name no card are buttons, the bids a grid; an
    // amount is typed into a box. The right column gets the game the host
    // picks and, at a game for stakes, the requests and the chips.

    protected static const float GEN_BTN_W = 170;
    // A later game's plate grows to hold its line, at most this wide, and stays
    // between these edges of the table (layout units).
    protected static const float GEN_PLATE_MOST = 330;
    protected static const float GEN_PLATE_MIN_X = 14;
    protected static const float GEN_PLATE_MAX_X = 1166;
    protected static const float GEN_BTN_H = 38;
    protected static const float GEN_GRID_W = 98;
    protected static const float GEN_GRID_H = 30;
    protected static const float GEN_GRID_GAP = 6;
    protected static const float GEN_AMOUNT_X = 76;
    protected static const float GEN_AMOUNT_W = 210;

    protected bool Generic()
    {
        if (!m_V || m_V.GameId == "")
            return false;
        return m_V.GameId != OZG_DurakConst.GAME_ID;
    }

    // ---- words ---------------------------------------------------------------

    // Words of a view: a key and its arguments (OZG_Words), in this client's
    // language.
    protected string Words(string words)
    {
        if (words == "")
            return "";
        array<string> parts = new array<string>();
        OZG_Words.Parts(words, parts);
        if (parts.Count() == 0)
            return "";
        string s = Tr(parts[0]);
        for (int i = 1; i < parts.Count(); i++)
        {
            string arg = ArgWord(parts[i]);
            s.Replace("%" + i.ToString(), arg);
        }
        return s;
    }

    protected string ArgWord(string arg)
    {
        int n = arg.Length();
        if (n < 2 || arg.Substring(1, 1) != ":")
            return arg;
        string kind = arg.Substring(0, 1);
        string rest = arg.Substring(2, n - 2);
        if (kind == "c")
            return CardWord(rest);
        if (kind == "s")
            return SeatName(rest.ToInt());
        if (kind == "k")
            return Tr(rest);
        if (kind == "b")
            return BidWord(rest);
        return arg;
    }

    // A call of the auction: "1C".."7N", "6S".."10N", "P", "X", "XX", "M".
    protected string BidWord(string bid)
    {
        if (bid == "P")
            return Tr("STR_OZG_CALL_PASS");
        if (bid == "X")
            return Tr("STR_OZG_CALL_DBL");
        if (bid == "XX")
            return Tr("STR_OZG_CALL_RDBL");
        if (bid == "M")
            return Tr("STR_OZG_CALL_MISERE");
        int n = bid.Length();
        if (n < 2)
            return bid;
        string level = bid.Substring(0, n - 1);
        string strain = bid.Substring(n - 1, 1);
        // The strain agrees with the number: "1 club", "2 clubs"; Ukrainian has
        // a third form from five on ("1 трефа", "2 трефи", "5 треф").
        int count = level.ToInt();
        string key = "STR_OZG_STRAIN_" + strain;
        if (count == 1)
            key = "STR_OZG_STRAIN1_" + strain;
        else if (count >= 2 && count <= 4)
            key = "STR_OZG_STRAIN2_" + strain;
        return level + " " + Tr(key);
    }

    protected string GameLabel(string id)
    {
        OZG_Game sample = OZG_Games.Sample(id);
        if (!sample)
            return id;
        return Tr(sample.LabelKey());
    }

    // ---- what the view says ----------------------------------------------------

    protected OZG_GSeat GenSeat(int no)
    {
        if (!m_GPub || !m_GPub.SeatList)
            return null;
        for (int i = 0; i < m_GPub.SeatList.Count(); i++)
        {
            if (m_GPub.SeatList[i].SeatNo == no)
                return m_GPub.SeatList[i];
        }
        return null;
    }

    // The moves a card makes now: from the hand, or from the faces of a seat
    // this player plays for (the dummy, an open whist's partner).
    protected void GenMovesOf(string card, array<string> into)
    {
        into.Clear();
        if (!m_GOwn || !m_GOwn.CanPlay)
            return;
        for (int i = 0; i < m_GOwn.CanPlay.Count(); i++)
        {
            string move = m_GOwn.CanPlay[i];
            if (OZG_Cards.OfMove(move) == card)
                into.Insert(move);
        }
    }

    // What a plate says of a seat, and whether the game waits for it.
    protected string GenRole(int no, out bool turn)
    {
        turn = false;
        OZG_WSeat s = m_V.Seats[no];
        string does = "";
        OZG_GSeat g = GenSeat(no);
        if (g && m_V.St != OZG_Const.ST_LOBBY)
        {
            does = Words(g.Role);
            string line = Words(g.Line);
            if (line != "")
                does = Joined(does, line);
            if (g.Mark == 1)
            {
                string mark = Tr("STR_OZG_G_MARK_" + m_V.GameId);
                if (does == "")
                    does = mark;
                else
                    does = mark + " · " + does;
            }
            if (m_GPub.Ended != 1 && m_GPub.Awaited && m_GPub.Awaited.Find(no) >= 0)
                turn = true;
        }
        else if (m_V.Stakes == 1 && s.Chips >= 0 && s.St != OZG_Const.SEAT_FREE)
        {
            does = Tr1("STR_OZG_G_LINE_CHIPS", s.Chips.ToString());
        }
        if (no == m_V.HostSeat)
            does = Joined(does, Tr("STR_OZG_WIN_HOST_MARK"));
        if (s.St == OZG_Const.SEAT_KEPT)
            does = Joined(does, Tr("STR_OZG_WIN_AWAY"));
        return does;
    }

    // ---- the widgets the script lays -----------------------------------------

    protected Widget GenMake(string kind)
    {
        string look = "Classic";
        if (m_Style == "stalker")
            look = "Stalker";
        string root = "OZG_Gen" + kind + look;
        string file = "ozg_g" + kind + "_" + m_Style + ".layout";
        file.ToLower();
        return MakeLayout("OpenZone_Games/gui/layouts/" + file, m_Win, root);
    }

    protected Widget GenButton(array<Widget> pool, array<string> moves, int i)
    {
        while (pool.Count() <= i)
        {
            Widget made = GenMake("Btn");
            if (!made)
                return null;
            made.Show(false);
            pool.Insert(made);
            moves.Insert("");
        }
        return pool[i];
    }

    protected TextWidget GenText(array<Widget> pool, int i)
    {
        while (pool.Count() <= i)
        {
            Widget made = GenMake("Text");
            if (!made)
                return null;
            made.Show(false);
            pool.Insert(made);
        }
        return TextWidget.Cast(pool[i]);
    }

    protected EditBoxWidget GenEditBox(Widget frame)
    {
        if (!frame)
            return null;
        return EditBoxWidget.Cast(frame.FindAnyWidget(frame.GetName() + "Box"));
    }

    protected void GenHot(Widget button, bool hot)
    {
        if (!button)
            return;
        Widget edge = button.FindAnyWidget(button.GetName() + "Edge");
        if (m_Style == "stalker")
            edge = button.FindAnyWidget(button.GetName() + "Hot");
        if (edge)
            edge.Show(hot);
    }

    protected void PutAt(Widget w, float x, float y, float width, float height)
    {
        if (!w)
            return;
        w.SetPos(x * m_K, y * m_K);
        w.SetSize(width * m_K, height * m_K);
        w.Show(true);
    }

    protected void HideAll(array<Widget> pool)
    {
        for (int i = 0; i < pool.Count(); i++)
        {
            if (pool[i])
                pool[i].Show(false);
        }
    }

    // The widgets of the later games' table out of sight: before every
    // drawing, and for durak.
    protected void GenHideTable()
    {
        HideAll(m_GenBtns);
        HideAll(m_GenTexts);
        if (m_GenEdit)
            m_GenEdit.Show(false);
    }

    // Everything made for the later games goes with the look it was made in.
    protected void GenForget()
    {
        m_GenBtns.Clear();
        m_GenBtnMoves.Clear();
        m_GenTexts.Clear();
        m_GenColBtns.Clear();
        m_GenColMoves.Clear();
        m_GenColTexts.Clear();
        m_GenEdit = null;
        m_StakeEdit = null;
        m_GenEditMove = "";
        m_GenFaceCodes.Clear();
        m_GenFaceX.Clear();
        m_GenFaceY.Clear();
        m_GenFaceW.Clear();
        m_GenFaceH.Clear();
        m_GenFaceA.Clear();
    }

    // A whole number typed into a box, "" for none.
    protected string BoxNumber(Widget frame)
    {
        EditBoxWidget box = GenEditBox(frame);
        if (!box)
            return "";
        string text = box.GetText();
        string digits = "";
        string nums = "0123456789";
        for (int i = 0; i < text.Length(); i++)
        {
            string c = text.Substring(i, 1);
            if (nums.IndexOf(c) >= 0)
                digits = digits + c;
        }
        if (digits.Length() > 7)
            digits = digits.Substring(0, 7);
        return digits;
    }

    // ---- seats -----------------------------------------------------------------

    // A seat's cards: its faces as a fan when it shows any, else its backs.
    protected void GenSeatCards(int no, float x, float y, float turn, float scale)
    {
        OZG_GSeat g = GenSeat(no);
        if (!g || m_V.St == OZG_Const.ST_LOBBY)
            return;
        if (g.Faces.Count() > 0)
            GenFaces(x, y, turn, scale, g.Faces, g.FaceLooks, no);
        else if (g.Backs.Count() > 0)
            PaintFan(x, y, turn, scale, g.Backs.Count(), g.Backs, no);
    }

    // A fan of faces, as a fan of backs is laid; every face is kept for the
    // pointer, a face this player may play lit.
    protected void GenFaces(float x, float y, float turn, float scale, array<string> faces, array<int> looks, int seat)
    {
        int count = faces.Count();
        float spread = m_Look.FanSpread;
        if (count > 1 && spread * (count - 1) > m_Look.FanFan)
            spread = m_Look.FanFan / (count - 1);
        float r = m_Look.FanR * scale;
        float w = m_Look.FanW * scale * 1.25;
        float h = m_Look.FanH * scale * 1.25;
        float fx;
        float fy;
        float fturn;
        FanFacing(x, y, turn, r, spread, count, fx, fy, fturn);
        x = fx;
        y = fy;
        turn = fturn;
        float tr = turn * Math.DEG2RAD;
        float c = Math.Cos(tr);
        float s = Math.Sin(tr);
        float mid = (count - 1) * 0.5;
        array<string> can = new array<string>();
        for (int i = 0; i < count; i++)
        {
            float a = (i - mid) * spread;
            float ar = a * Math.DEG2RAD;
            float lx = r * Math.Sin(ar);
            float ly = r - r * Math.Cos(ar);
            float cx = x + lx * c - ly * s;
            float cy = y + lx * s + ly * c;
            string card = faces[i];
            string face = "c_" + OZG_Cards.FaceOf(card);
            int look = LookAt(looks, i);
            GenMovesOf(card, can);
            int glow = 0;
            if (can.Count() > 0)
                glow = m_Look.Glow;
            Card(m_FanPics, PictureOf(face, look), cx, cy, w, h, a + turn, glow, true, "", RegionOfSeat(seat));
            m_GenFaceCodes.Insert(card);
            m_GenFaceX.Insert(cx);
            m_GenFaceY.Insert(cy);
            m_GenFaceW.Insert(w);
            m_GenFaceH.Insert(h);
            m_GenFaceA.Insert(a + turn);
        }
    }

    protected int GenFaceAt(float x, float y)
    {
        for (int i = m_GenFaceCodes.Count() - 1; i >= 0; i--)
        {
            if (OnCard(x, y, m_GenFaceX[i], m_GenFaceY[i], m_GenFaceW[i], m_GenFaceH[i], m_GenFaceA[i]))
                return i;
        }
        return -1;
    }

    // ---- the middle of the table -----------------------------------------------

    protected void GenLine(int i, string text, float x, float y, float w, float h)
    {
        TextWidget t = GenText(m_GenTexts, i);
        if (!t)
            return;
        t.SetText(Caps(text));
        PutAt(t, x, y, w, h);
    }

    protected void GenPaintTable()
    {
        m_StockLabel.Show(false);
        m_DiscardLabel.Show(false);
        if (!m_GPub || m_V.St == OZG_Const.ST_LOBBY)
            return;
        int texts = 0;
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            OZG_GZone zone = m_GPub.Zones[z];
            if (zone.Id == "bank" || zone.Id == "pot")
            {
                // One line under the row of cards, where the result stands
                // when the deal is over (the pot is taken by then).
                if (m_GPub.Ended == 1)
                    continue;
                string amount = Words(zone.Line);
                string sum = Words(zone.Label) + " " + amount;
                GenLine(texts, sum, m_Look.BoutX - 150, m_Look.BoutY + m_Look.BoutH + 12, 300, 26);
                texts++;
                continue;
            }
            if (zone.Id == "stock" || zone.Id == "talon")
            {
                texts = GenPile(zone, texts);
                continue;
            }
            texts = GenRow(zone, texts);
        }
        if (m_GPub.Ended == 1 && m_GPub.Result != "")
        {
            m_Result.SetText(Caps(Words(m_GPub.Result)));
            float rw = m_Look.ResultW;
            PutAt(m_Result, m_Look.BoutX - rw * 0.5, m_Look.BoutY + m_Look.BoutH + 6, rw, 44);
        }
    }

    // The player's own plate `w` units wide round the layout's middle, its
    // ground, frame and two lines with it: the layout's own size for durak.
    protected void FitMyPlate(float w)
    {
        float h = m_Look.MyPlateH;
        float middle = m_Look.MyPlateX + m_Look.MyPlateW * 0.5;
        PutAt(m_MyPlate, middle - w * 0.5, m_Look.MyPlateY, w, h);
        PutAt(m_MyBg, 0, 0, w, h);
        PutAt(m_MyEdge, 0, 0, w, h);
        float textW = w - m_Look.MyTextX - m_Look.MyTextRight;
        PutAt(m_MyName, m_Look.MyTextX, m_Look.MyNameY, textW, m_Look.MyNameH);
        PutAt(m_MyRole, m_Look.MyTextX, m_Look.MyRoleY, textW, m_Look.MyRoleH);
    }

    // How wide a later game's plate must be for its line: the line's letters
    // at about the width of the role's 13-unit face (capitals wider), never
    // narrower than the look's plate.
    protected float GenPlateWidth(string text, float least)
    {
        // Measured on the stand: a Ukrainian line ran past a plate sized at
        // 6.4 a letter, its letters being wider than the English ones.
        float per = 7.2;
        if (m_Look.Caps)
            per = 8.2;
        int letters = text.LengthUtf8();
        float need = letters * per + 18;
        if (need < least)
            return least;
        if (need > GEN_PLATE_MOST)
            return GEN_PLATE_MOST;
        return need;
    }

    // Whether the table shows a pile at the stock's place now (the stock, the
    // talon): something the bid grid must not lie over.
    protected bool GenHasPile()
    {
        if (!m_GPub)
            return false;
        OZG_GZone zone;
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            zone = m_GPub.Zones[z];
            if (zone.Id != "stock" && zone.Id != "talon")
                continue;
            if (zone.Backs > 0 || zone.Faces.Count() > 0)
                return true;
        }
        return false;
    }

    // A pile at the stock's place: its backs, the faces turned up beside it.
    protected int GenPile(OZG_GZone zone, int texts)
    {
        float pw = m_Look.PileW;
        float ph = m_Look.PileH;
        int layers = zone.Backs;
        if (layers > 6)
            layers = 6;
        int k;
        for (k = 0; k < layers; k++)
        {
            int look = -1;
            if (k == layers - 1)
                look = zone.BackLook;
            float sx = m_Look.StockX - k * 1.2 + pw * 0.5;
            float sy = m_Look.StockY - k * 1.6 + ph * 0.5;
            Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true, "", REGION_STOCK);
        }
        float fx = m_Look.StockX + pw * 0.5;
        if (layers > 0)
            fx = fx + pw + 10;
        for (k = 0; k < zone.Faces.Count(); k++)
        {
            string face = "c_" + OZG_Cards.FaceOf(zone.Faces[k]);
            int faceLook = LookAt(zone.Looks, k);
            Card(m_TablePics, PictureOf(face, faceLook), fx + k * (pw + 8), m_Look.StockY + ph * 0.5, pw, ph, 0, 0, true, "", REGION_STOCK);
        }
        if (layers > 0 || zone.Faces.Count() > 0)
        {
            GenLine(texts, Words(zone.Label), m_Look.StockX - 30, m_Look.StockY + ph + 8, pw + 60, 22);
            texts++;
        }
        return texts;
    }

    // A row at the bout's place: the board, the trick with who laid each card.
    protected int GenRow(OZG_GZone zone, int texts)
    {
        int faces = zone.Faces.Count();
        int n = faces + zone.Backs;
        if (n == 0)
            return texts;
        float w = m_Look.BoutW;
        float h = m_Look.BoutH;
        float pitch = w + 14;
        if (n > 5)
            pitch = (5 * (w + 14)) / n;
        float left = m_Look.BoutX - pitch * (n - 1) * 0.5;
        float cy = m_Look.BoutY + h * 0.5;
        GenLine(texts, Words(zone.Label), m_Look.BoutX - 150, m_Look.BoutY - 28, 300, 22);
        texts++;
        for (int k = 0; k < n; k++)
        {
            float cx = left + k * pitch;
            if (k < faces)
            {
                string face = "c_" + OZG_Cards.FaceOf(zone.Faces[k]);
                int look = LookAt(zone.Looks, k);
                Widget pic = Card(m_TablePics, PictureOf(face, look), cx, cy, w, h, 0, 0, true);
                int notice = NoticeOn(zone.Faces[k]);
                if (pic && notice >= 0)
                    SpeckFace(pic, w, h, m_V.Notices[notice]);
                int by = -1;
                if (k < zone.By.Count())
                    by = zone.By[k];
                if (by >= 0)
                {
                    GenLine(texts, SeatName(by), cx - 60, cy + h * 0.5 + 4, 120, 20);
                    texts++;
                }
            }
            else
            {
                Card(m_TablePics, PictureOf("back", zone.BackLook), cx, cy, w, h, 0, 0, true);
            }
        }
        return texts;
    }

    // ---- the hand ----------------------------------------------------------------

    protected void GenPaintHand()
    {
        if (!m_GOwn || !m_GOwn.Held || m_V.St == OZG_Const.ST_LOBBY)
            return;
        int n = m_GOwn.Held.Count();
        int i;
        float spread = m_Look.HandSpread;
        if (n > 1 && spread * (n - 1) > m_Look.HandFan)
            spread = m_Look.HandFan / (n - 1);
        float r = m_Look.HandR;
        float mid = (n - 1) * 0.5;
        array<string> moves = new array<string>();
        for (i = 0; i < n; i++)
        {
            string card = m_GOwn.Held[i];
            GenMovesOf(card, moves);
            float a = (i - mid) * spread;
            float ar = a * Math.DEG2RAD;
            float x = m_Look.HandX + r * Math.Sin(ar);
            float y = m_Look.HandY + r - r * Math.Cos(ar);
            int glow = 0;
            if (Playing() && moves.Count() > 0)
                glow = m_Look.Glow;
            string cardFace = "c_" + OZG_Cards.FaceOf(card);
            int cardLook = LookAt(m_GOwn.HeldLooks, i);
            Widget pic = Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true, SlotOfHand(i));
            if (!pic)
                continue;
            int handNotice = NoticeOn(card);
            OZG_Cover nextOver = null;
            if (i < n - 1)
                nextOver = OZG_Cover.MakeFan(r, spread, m_Look.CardW, m_Look.CardH);
            if (handNotice >= 0)
                SpeckFace(pic, m_Look.CardW, m_Look.CardH, m_V.Notices[handNotice], nextOver);
            m_HandW.Insert(pic);
            m_HandCodes.Insert(card);
            m_HandX.Insert(x);
            m_HandY.Insert(y);
            m_HandA.Insert(a);
        }
        for (i = 0; i < m_HandW.Count(); i++)
            PlaceHand(i);
    }

    protected void GenClickCard(string card)
    {
        array<string> moves = new array<string>();
        GenMovesOf(card, moves);
        if (moves.Count() > 0)
            Play(moves[0]);
    }

    // ---- the moment: whose move, the buttons, the amount -------------------------

    protected void GenPaintMoment()
    {
        if (m_BtnTake)
            m_BtnTake.Show(false);
        if (m_BtnPass)
            m_BtnPass.Show(false);
        if (m_BtnMove)
            m_BtnMove.Show(false);
        HideAll(m_GenBtns);
        if (m_GenEdit)
            m_GenEdit.Show(false);

        string own = "";
        if (m_GOwn && m_V.St != OZG_Const.ST_LOBBY)
            own = Words(m_GOwn.Status);
        m_Hint.SetText(own);

        string status = "";
        if (m_V.St == OZG_Const.ST_LOBBY || (m_GPub && m_GPub.Ended == 1) || !m_GPub)
        {
            int seated = 0;
            for (int i = 0; i < m_V.Seats.Count(); i++)
            {
                if (m_V.Seats[i].St == OZG_Const.SEAT_TAKEN)
                    seated++;
            }
            OZG_Game sample = OZG_Games.Sample(m_V.GameId);
            int least = 2;
            int most = 8;
            if (sample)
            {
                least = sample.MinPlayers();
                most = sample.MaxPlayers();
            }
            // Too many at the table is a refused deal too (preferans is for
            // three): the host is told before he presses Deal.
            if (seated < least)
                status = Tr1("STR_OZG_G_ST_NEED", least.ToString());
            else if (seated > most)
                status = Tr1("STR_OZG_G_ST_TOO_MANY", most.ToString());
            else if (m_V.IsHost == 1 && VariantLabel() == "")
                status = Tr("STR_OZG_G_ST_DEAL");
            else if (m_V.IsHost == 1)
                status = Tr("STR_OZG_ST_LOBBY_HOST");
            else
                status = Tr("STR_OZG_ST_LOBBY");
        }
        else
        {
            status = Words(m_GPub.Status);
            if (m_GPub.Trump != "")
                status = status + " · " + Tr("STR_OZG_G_TRUMP_" + m_GPub.Trump);
        }
        m_Status.SetText(Caps(status));

        if (!m_GOwn || !Playing())
            return;
        int used = 0;
        int actions = 0;
        int rows = 0;
        int b;
        for (b = 0; b < m_GOwn.Buttons.Count(); b++)
        {
            if (m_GOwn.Buttons[b].Row + 1 > rows)
                rows = m_GOwn.Buttons[b].Row + 1;
        }
        float gridW = 5 * GEN_GRID_W + 4 * GEN_GRID_GAP;
        float gridLeft = m_Look.BoutX - gridW * 0.5;
        // A pile at the stock's place (preferans' talon) stays in sight: the
        // grid starts right of it (seen 2026-11-25: the grid lay over it).
        float pileRight = m_Look.StockX + m_Look.PileW + 16;
        if (GenHasPile() && gridLeft < pileRight)
            gridLeft = pileRight;
        float gridTop = m_Look.BoutY + 60 - rows * (GEN_GRID_H + GEN_GRID_GAP) * 0.5;
        for (b = 0; b < m_GOwn.Buttons.Count(); b++)
        {
            OZG_GButton gb = m_GOwn.Buttons[b];
            Widget w = GenButton(m_GenBtns, m_GenBtnMoves, used);
            if (!w)
                break;
            m_GenBtnMoves[used] = gb.Move;
            used++;
            SetLabel(w, Caps(Words(gb.Label)));
            GenHot(w, gb.Hot == 1);
            if (gb.Row < 0)
            {
                PutAt(w, m_Look.MomentX, m_Look.MomentY - actions * m_Look.MomentStep, GEN_BTN_W, GEN_BTN_H);
                actions++;
            }
            else
            {
                float gx = gridLeft + gb.Col * (GEN_GRID_W + GEN_GRID_GAP);
                float gy = gridTop + gb.Row * (GEN_GRID_H + GEN_GRID_GAP);
                PutAt(w, gx, gy, GEN_GRID_W, GEN_GRID_H);
            }
        }
        // The amount: its box and the button that sends it, low on the left.
        if (m_GOwn.AmountMove == "")
            return;
        if (!m_GenEdit)
            m_GenEdit = GenMake("Edit");
        EditBoxWidget box = GenEditBox(m_GenEdit);
        string bounds = m_GOwn.AmountMove + m_GOwn.AmountMin.ToString() + "-" + m_GOwn.AmountMax.ToString();
        if (box && bounds != m_GenEditMove)
        {
            box.SetText(m_GOwn.AmountStart.ToString());
            m_GenEditMove = bounds;
        }
        PutAt(m_GenEdit, GEN_AMOUNT_X, m_Look.MomentY, GEN_AMOUNT_W, GEN_BTN_H);
        Widget send = GenButton(m_GenBtns, m_GenBtnMoves, used);
        if (send)
        {
            m_GenBtnMoves[used] = "#amount";
            string range = " (" + m_GOwn.AmountMin.ToString() + "-" + m_GOwn.AmountMax.ToString() + ")";
            SetLabel(send, Caps(Words(m_GOwn.AmountLabel)) + range);
            GenHot(send, false);
            PutAt(send, GEN_AMOUNT_X, m_Look.MomentY - m_Look.MomentStep, GEN_AMOUNT_W, GEN_BTN_H);
        }
    }

    // ---- the right column: the game, the chips -----------------------------------

    protected Widget GenColButton(int i, string move, string label)
    {
        Widget w = GenButton(m_GenColBtns, m_GenColMoves, i);
        if (!w)
            return null;
        m_GenColMoves[i] = move;
        SetLabel(w, label);
        GenHot(w, false);
        return w;
    }

    protected void GenColLine(int i, string text, float y)
    {
        TextWidget t = GenText(m_GenColTexts, i);
        if (!t)
            return;
        t.SetText(text);
        PutAt(t, m_Look.StackX, y, 352, GEN_BTN_H);
    }

    // What the column holds above its stack for the later games and for
    // stakes; returns the rows it took from `top` down.
    protected int GenColumn(float top)
    {
        HideAll(m_GenColBtns);
        HideAll(m_GenColTexts);
        if (m_StakeEdit)
            m_StakeEdit.Show(false);
        if (!m_V)
            return 0;
        bool host = m_V.IsHost == 1;
        bool playing = Playing();
        float step = m_Look.StackStep;
        float colW = 352;
        int row = 0;
        int b = 0;
        int t = 0;
        // The host picks the game between deals.
        if (host && !playing && m_V.GameIds.Count() > 1)
        {
            Widget game = GenColButton(b, "#game", Caps(Tr1("STR_OZG_WIN_GAME", GameLabel(m_V.GameId))));
            b++;
            PutAt(game, m_Look.StackX, top + row * step, colW, GEN_BTN_H);
            row++;
        }
        if (m_V.Stakes != 1)
            return row;
        // A seated player's own chips, between deals: during one his plate
        // says what is behind him, and this line would count his bet too.
        if (m_V.Mine >= 0 && !playing)
        {
            GenColLine(t, Tr2("STR_OZG_WIN_MY_CHIPS", m_V.MyChips.ToString(), m_V.MyIn.ToString()), top + row * step);
            t++;
            row++;
        }
        // The first request waiting for the host: its line, the amount and
        // "Let in" (or "Give") on one row, "Refuse" under them.
        bool asking = host && m_V.ReqUids.Count() > 0;
        bool picked = host && !playing && m_PickedSeat >= 0 && m_PickedSeat < m_V.Seats.Count();
        if (picked)
            picked = m_V.Seats[m_PickedSeat].St == OZG_Const.SEAT_TAKEN || m_V.Seats[m_PickedSeat].St == OZG_Const.SEAT_KEPT;
        if (asking || picked)
        {
            if (!m_StakeEdit)
                m_StakeEdit = GenMake("Edit");
            string ask;
            string sendMove = "#give";
            string sendLabel = Tr("STR_OZG_WIN_GIVE");
            if (asking)
            {
                ask = Tr1("STR_OZG_WIN_REQ_CHIPS", m_V.ReqNames[0]);
                if (m_V.ReqSeats[0] >= 0)
                    ask = Tr1("STR_OZG_WIN_REQ_SIT", m_V.ReqNames[0]);
                if (m_V.ReqUids.Count() > 1)
                {
                    int more = m_V.ReqUids.Count() - 1;
                    ask = ask + " (+" + more.ToString() + ")";
                }
                sendMove = "#let";
                if (m_V.ReqSeats[0] >= 0)
                    sendLabel = Tr("STR_OZG_WIN_LET");
            }
            else
            {
                ask = Tr1("STR_OZG_WIN_CHIPS_FOR", SeatName(m_PickedSeat));
            }
            GenColLine(t, ask, top + row * step);
            t++;
            row++;
            float half = (colW - 8) * 0.5;
            PutAt(m_StakeEdit, m_Look.StackX, top + row * step, half, GEN_BTN_H);
            Widget send = GenColButton(b, sendMove, Caps(sendLabel));
            b++;
            PutAt(send, m_Look.StackX + half + 8, top + row * step, half, GEN_BTN_H);
            row++;
            if (asking)
            {
                Widget refuse = GenColButton(b, "#refuse", Caps(Tr("STR_OZG_WIN_REFUSE")));
                b++;
                PutAt(refuse, m_Look.StackX, top + row * step, colW, GEN_BTN_H);
                row++;
            }
        }
        // A seated player who is not the host asks for chips.
        if (!host && m_V.Mine >= 0)
        {
            if (m_V.AskedChips == 1)
            {
                GenColLine(t, Tr("STR_OZG_WIN_ASKED_CHIPS"), top + row * step);
                t++;
            }
            else
            {
                Widget askChips = GenColButton(b, "#askchips", Caps(Tr("STR_OZG_WIN_ASK_CHIPS")));
                b++;
                PutAt(askChips, m_Look.StackX, top + row * step, colW, GEN_BTN_H);
            }
            row++;
        }
        return row;
    }

    protected void NextGame()
    {
        if (!m_V || m_V.GameIds.Count() == 0)
            return;
        int at = m_V.GameIds.Find(m_V.GameId);
        int next = at + 1;
        if (next >= m_V.GameIds.Count())
            next = 0;
        Send(OZG_Const.OP_GAME, "", -1, "", "", m_V.GameIds[next], 0);
    }

    // A button of the later games: a move of the game, or one of the window's
    // own (`#...`).
    protected void GenClick(string move)
    {
        if (move == "#game")
        {
            NextGame();
            return;
        }
        if (move == "#askchips")
        {
            Send(OZG_Const.OP_REBUY, "", -1, "", "", "", 0);
            return;
        }
        if (move == "#let" || move == "#refuse")
        {
            if (!m_V || m_V.ReqUids.Count() == 0)
                return;
            string uid = m_V.ReqUids[0];
            if (move == "#refuse")
            {
                Send(OZG_Const.OP_REFUSE, "", -1, uid, "", "", 0);
                return;
            }
            string amount = BoxNumber(m_StakeEdit);
            if (amount == "")
                amount = "0";
            Send(OZG_Const.OP_LET, "", -1, uid, "", amount, 0);
            return;
        }
        if (move == "#give")
        {
            string chips = BoxNumber(m_StakeEdit);
            if (chips == "")
            {
                Said(Tr(OZG_Const.SAY_BAD_AMOUNT));
                return;
            }
            Send(OZG_Const.OP_CHIPS, "", m_PickedSeat, "", "", chips, 0);
            return;
        }
        if (move == "#amount")
        {
            if (!m_GOwn || m_GOwn.AmountMove == "")
                return;
            string n = BoxNumber(m_GenEdit);
            if (n == "")
            {
                Said(Tr(OZG_Const.SAY_BAD_AMOUNT));
                return;
            }
            Play(m_GOwn.AmountMove + n);
            return;
        }
        Play(move);
    }

    // ---- the log and the score sheet ---------------------------------------------

    // A line of the log of a later game, or "" for one the window words as
    // before.
    protected string GenLogWord(string head, string rest)
    {
        if (head == "game")
            return Tr1("STR_OZG_LOG_GAME", GameLabel(rest));
        if (head == "let" || head == "chips")
        {
            int space = rest.IndexOf(" ");
            if (space <= 0)
                return "";
            int seat = rest.Substring(0, space).ToInt();
            string amount = rest.Substring(space + 1, rest.Length() - space - 1);
            // "<seat> <amount> <name>": the name of whoever it was then.
            string who = SeatName(seat);
            int named = amount.IndexOf(" ");
            if (named > 0)
            {
                who = amount.Substring(named + 1, amount.Length() - named - 1);
                amount = amount.Substring(0, named);
            }
            if (head == "let")
                return Tr2("STR_OZG_LOG_LET", who, amount);
            return Tr2("STR_OZG_LOG_CHIPS", who, amount);
        }
        if (!Generic())
            return "";
        if (head == "end")
            return Tr("STR_OZG_LOG_END");
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (!sample)
            return "";
        string first = head.Substring(0, 1);
        string digits = "0123456789";
        if (digits.IndexOf(first) < 0)
            return "";
        array<string> parts = new array<string>();
        sample.MoveWords(rest, parts);
        if (parts.Count() == 0)
            return "";
        string words = parts[0] + OZG_Words.SEP + OZG_Words.SeatArg(head.ToInt());
        for (int i = 1; i < parts.Count(); i++)
            words = words + OZG_Words.SEP + parts[i];
        return Words(words);
    }

    protected void GenPaintLog(int lines)
    {
        array<string> rows = new array<string>();
        int k;
        if (m_GPub && m_V.St != OZG_Const.ST_LOBBY)
        {
            for (k = 0; k < m_GPub.Score.Count(); k++)
                rows.Insert(Words(m_GPub.Score[k]));
            if (rows.Count() > 0 && rows.Count() < lines - 1)
                rows.Insert("");
        }
        int room = lines - rows.Count();
        int have = m_V.Log.Count();
        int from = have - room;
        if (from < 0)
            from = 0;
        for (k = from; k < have && rows.Count() < lines; k++)
            rows.Insert(LogWord(m_V.Log[k]));
        for (int i = 0; i < m_LogLines.Count(); i++)
        {
            string text = "";
            if (i < lines && i < rows.Count())
                text = rows[i];
            if (m_LogLines[i])
                m_LogLines[i].SetText(text);
        }
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w || button != MouseState.LEFT || !m_V)
            return false;

        if (w == m_Hit)
        {
            ClickTable();
            return true;
        }
        if (w == m_BtnClose)
        {
            Shut();
            return true;
        }
        if (w == m_BtnPass)
        {
            Play("pass");
            return true;
        }
        if (w == m_BtnTake)
        {
            Play("take");
            return true;
        }
        if (w == m_BtnMove)
        {
            if (m_Picked != "")
                Play("t:" + m_Picked);
            return true;
        }
        if (w == m_BtnVariant)
        {
            NextVariant();
            return true;
        }
        if (w == m_BtnDeal)
        {
            // The dealer's three ride in the deal (design §9).
            OZG_WAsk deal = new OZG_WAsk();
            deal.PlaceId = m_PlaceId;
            if (Dealing())
                deal.Stack = StackText();
            string dealJson;
            string dealErr;
            JsonFileLoader<OZG_WAsk>.MakeData(deal, dealJson, dealErr, false);
            OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_DEAL, dealJson);
            m_Sleight = "";
            return true;
        }
        if (w == m_BtnSlip)
        {
            ArmSleight(OZG_Const.SLEIGHT_SLIP);
            return true;
        }
        if (w == m_BtnSwap)
        {
            ArmSleight(OZG_Const.SLEIGHT_SWAP);
            return true;
        }
        if (w == m_BtnToss)
        {
            TossButton();
            return true;
        }
        if (w == m_BtnDeckIn)
        {
            ArmSleight(OZG_Const.SLEIGHT_DECK_IN);
            return true;
        }
        if (w == m_BtnDeckOut)
        {
            ArmSleight(OZG_Const.SLEIGHT_DECK_OUT);
            return true;
        }
        if (w == m_BtnTakeHost)
        {
            Send(OZG_Const.OP_HOST, "", -1, "", "", "", 0);
            return true;
        }
        if (w == m_BtnKickDiscard)
        {
            Send(OZG_Const.OP_KICK, "", m_PickedSeat, OZG_Const.HOW_DISCARD, "", "", 0);
            m_PickedSeat = -1;
            return true;
        }
        if (w == m_BtnKickLeave)
        {
            Send(OZG_Const.OP_KICK, "", m_PickedSeat, OZG_Const.HOW_LEAVE, "", "", 0);
            m_PickedSeat = -1;
            return true;
        }
        if (w == m_BtnDropLeft)
        {
            Send(OZG_Const.OP_DROP, "", m_PickedSeat, "", "", "", 0);
            m_PickedSeat = -1;
            return true;
        }
        if (w == m_BtnPose)
        {
            int next = OZG_PoseDriver.Next(m_V);
            Send(OZG_Const.OP_POSE, "", -1, "", "", "", next);
            PaintColumn();
            return true;
        }
        if (w == m_BtnStand)
        {
            // In mid-game the player chooses what becomes of his cards.
            if (MineInGame())
            {
                m_AskStand = true;
                PaintColumn();
            }
            else
            {
                Send(OZG_Const.OP_STAND, "", -1, "", "", "", 0);
            }
            return true;
        }
        if (w == m_BtnStandDiscard)
        {
            Send(OZG_Const.OP_STAND, "", -1, OZG_Const.HOW_DISCARD, "", "", 0);
            return true;
        }
        if (w == m_BtnStandLeave)
        {
            Send(OZG_Const.OP_STAND, "", -1, OZG_Const.HOW_LEAVE, "", "", 0);
            return true;
        }
        if (w == m_BtnFold)
        {
            if (Playing())
            {
                m_AskFold = true;
                PaintColumn();
            }
            else
            {
                Send(OZG_Const.OP_FOLD, "", -1, "", "", "", 0);
            }
            return true;
        }
        if (w == m_BtnFoldYes)
        {
            Send(OZG_Const.OP_FOLD, "", -1, "", "", "", 0);
            return true;
        }

        int gen = m_GenBtns.Find(w);
        if (gen >= 0)
        {
            GenClick(m_GenBtnMoves[gen]);
            return true;
        }
        int col = m_GenColBtns.Find(w);
        if (col >= 0)
        {
            GenClick(m_GenColMoves[col]);
            return true;
        }

        // A seat's plate: the host picks the seat to remove or clear.
        int at = m_SeatW.Find(w);
        if (at >= 0)
        {
            if (m_V.IsHost == 1)
            {
                int no = m_SeatOf[at];
                if (m_PickedSeat == no)
                    m_PickedSeat = -1;
                else
                    m_PickedSeat = no;
                PaintSeats();
                PaintColumn();
            }
            return true;
        }
        return false;
    }
}
