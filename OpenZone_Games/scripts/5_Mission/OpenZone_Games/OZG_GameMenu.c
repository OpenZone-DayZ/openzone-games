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

    // What the server last said.
    protected ref OZG_WView m_V;
    protected ref OZG_DurakPublic m_Pub;
    protected ref OZG_DurakPrivate m_Own;

    // What the player is in the middle of.
    protected string m_Picked;
    protected int m_PickedSeat;
    protected bool m_AskStand;
    protected bool m_AskFold;

    void OZG_GameMenu()
    {
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
        ClearPictures();
        ClearList(m_SeatW);
        m_SeatOf.Clear();
        m_LogLines.Clear();
        if (m_Win)
            m_Win.Unlink();
        m_Win = null;
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
        }
        Measure();
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
            mission.AddActiveInputExcludes({"menu"});
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
        if (m_V && m_V.Mine < 0)
            Send(OZG_Const.OP_UNWATCH, "", -1, "", "", "", 0);
        ClearPictures();
        ClearList(m_SeatW);
        m_SeatOf.Clear();
        if (m_Claimed)
        {
            m_Claimed = false;
            Mission mission = GetGame().GetMission();
            if (mission)
            {
                mission.RemoveActiveInputExcludes({"menu"}, true);
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

        TrackHover();

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
        ClearList(m_FanPics);
        ClearList(m_TablePics);
        ClearList(m_HandPics);
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
    // `shadow`, and its glow in the colour `glow` when that is not 0.
    protected Widget Card(array<Widget> into, string picture, float cx, float cy, float w, float h, float turn, int glow, bool shadow)
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

    // A read on a back of w x h units: the sign where its fault lies and the
    // face in pencil in the top corner (design §4.4), inside the card's frame
    // so that they turn with it. The places are pixels of a 200 x 280 card
    // (OZG_MarkPlaces, tools/make_marks.py); the face lies in the middle of
    // the frame, 5 : 7, as tools/make_layouts.py lays it.
    protected void MarkBack(Widget card, float w, float h, OZG_WRead r)
    {
        ImageWidget sign = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        ImageWidget pencil = ImageWidget.Cast(card.FindAnyWidget("OZG_CardPencil"));
        if (!sign || !pencil)
            return;
        float side = BoxOf(w, h) * m_K;
        float d = CARD_BOX * Math.Sqrt(74);
        float faceW = side * 5 / d;
        float faceH = side * 7 / d;
        float left = (side - faceW) * 0.5;
        float top = (side - faceH) * 0.5;
        float kx = faceW / OZG_MarkPlaces.CARD_W;
        float ky = faceH / OZG_MarkPlaces.CARD_H;
        int x;
        int y;
        int bw;
        int bh;
        string style = OZG_Decks.StyleAt(r.Look);
        if (OZG_MarkPlaces.SignBox(style, r.Sign, x, y, bw, bh))
        {
            sign.LoadImageFile(0, OZG_MarkPlaces.SignImage(style, r.Sign));
            sign.SetImage(0);
            sign.SetPos(left + x * kx, top + y * ky);
            sign.SetSize(bw * kx, bh * ky);
            sign.Show(true);
        }
        if (OZG_MarkPlaces.PencilBox(x, y, bw, bh))
        {
            pencil.LoadImageFile(0, OZG_MarkPlaces.PencilImage(r.Face));
            pencil.SetImage(0);
            pencil.SetPos(left + x * kx, top + y * ky);
            pencil.SetSize(bw * kx, bh * ky);
            pencil.Show(true);
        }
    }

    // ------------------------------------------------------------ drawing

    void Redraw()
    {
        if (!layoutRoot)
            return;
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
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

        // The deck's look decides the window's.
        string style = m_V.Style;
        if (style != "stalker")
            style = "classic";
        if (style != m_Style || !m_Win)
            Build(style);
        if (!m_Win)
            return;

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

        Measure();
        PaintTitle();
        PaintSeats();
        PaintTable();
        PaintHand();
        PaintMoment();
        PaintLog();
        PaintColumn();
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
        if (m_V.Mine >= 0)
            return m_V.Mine;
        for (int i = 0; i < m_V.Seats.Count(); i++)
        {
            if (m_V.Seats[i].St != OZG_Const.SEAT_FREE)
                return i;
        }
        return -1;
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
        ClearList(m_FanPics);
        ClearList(m_SeatW);
        m_SeatOf.Clear();
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
        for (int k = 1; k < n; k++)
        {
            int no = (bottom + k) % n;
            if (m_V.Seats[no].St != OZG_Const.SEAT_FREE)
                round.Insert(no);
        }
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
        if (BacksOf(no, g))
            PaintFan(spot.X, spot.Y, spot.Turn, spot.Scale, g.CardCount, g.Backs, no);

        string root = "OZG_PlateClassic";
        if (m_Style == "stalker")
            root = "OZG_PlateStalker";
        Widget plate = MakeLayout("OpenZone_Games/gui/layouts/ozg_plate_" + m_Style + ".layout", m_SeatsBox, root);
        if (!plate)
            return;
        // The plate lies PLATE_PAD units inside its button, which leaves room
        // for the ring of a seat the host picked (tools/make_layouts.py).
        float h = 48;
        if (m_Style == "stalker")
            h = 46;
        float bw = spot.PlateW + 2 * PLATE_PAD;
        float bh = h + 2 * PLATE_PAD;
        float left = spot.PlateX - bw * 0.5;
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

        bool turn;
        role.SetText(RoleOf(no, turn));
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

    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first), the hand of the seat `seat`: a
    // back this player reads there carries its mark (design §4.4).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks, int seat)
    {
        array<int> used = new array<int>();
        float spread = m_Look.FanSpread;
        if (count > 1 && spread * (count - 1) > m_Look.FanFan)
            spread = m_Look.FanFan / (count - 1);
        float r = m_Look.FanR * scale;
        float w = m_Look.FanW * scale;
        float h = m_Look.FanH * scale;
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
            Widget card = Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true);
            int readNo = ReadAt(seat, look, used);
            if (card && readNo >= 0)
                MarkBack(card, w, h, m_V.Reads[readNo]);
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
        m_MyRole.SetText(RoleOf(no, turn));
        if (turn)
            m_MyEdge.SetColor(m_Look.EdgeTurn);
        else
            m_MyEdge.SetColor(m_Look.EdgeIdle);

        OZG_DurakSeatPublic g;
        if (m_V.Mine < 0 && BacksOf(no, g))
            PaintFan(m_Look.HandX, m_Look.HandY + 20, 0, 1, g.CardCount, g.Backs, no);
    }

    protected void PaintTable()
    {
        ClearList(m_TablePics);
        m_AtkX.Clear();
        m_AtkY.Clear();
        m_AtkW.Clear();
        m_AtkH.Clear();
        m_Result.Show(false);
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
                Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true);
            }
            else
            {
                string suit = "suit_" + m_Pub.TrumpCard.Substring(1, 1);
                Card(m_TablePics, Picture(suit), tx, ty, 56, 78, 0, 0, false);
            }
        }
        int layers = m_Pub.StockCount - 1;
        if (layers > 6)
            layers = 6;
        int k;
        array<int> stockUsed = new array<int>();
        for (k = 0; k < layers; k++)
        {
            // The top card shows the look of the one drawn next.
            int look = -1;
            if (k == layers - 1)
                look = m_Pub.StockTop;
            float sx = m_Look.StockX - k * 1.2 + pw * 0.5;
            float sy = m_Look.StockY - k * 1.6 + ph * 0.5;
            Widget layer = Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true);
            // The top card: a back this player reads carries its mark (design §4.4).
            int topRead = -1;
            if (k == layers - 1)
                topRead = ReadAt(-1, look, stockUsed);
            if (layer && topRead >= 0)
                MarkBack(layer, pw, ph, m_V.Reads[topRead]);
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
            Card(m_TablePics, PictureOf("back", -1), hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt, 0, true);
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
            m_Result.Show(true);
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
            Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true);
            m_AtkX.Insert(cx);
            m_AtkY.Insert(cy);
            m_AtkW.Insert(w);
            m_AtkH.Insert(h);

            string beaten = "";
            if (i < m_Pub.DefenceCards.Count())
                beaten = m_Pub.DefenceCards[i];
            if (beaten != "")
            {
                string beatenFace = "c_" + OZG_Cards.FaceOf(beaten);
                int beatenLook = LookAt(m_Pub.DefenceLooks, i);
                Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true);
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

    protected void PaintHand()
    {
        ClearList(m_HandPics);
        ClearHandMarks();
        if (!m_Own || !m_Own.Held || m_V.St == OZG_Const.ST_LOBBY)
            return;

        string trumpSuit = "";
        if (m_Pub)
            trumpSuit = OZG_Cards.SuitOf(m_Pub.TrumpCard);

        array<string> cards = new array<string>();
        cards.Copy(m_Own.Held);
        int n = cards.Count();
        int i;
        int j;
        for (i = 1; i < n; i++)
        {
            string moved = cards[i];
            int key = SortKey(moved, trumpSuit);
            j = i - 1;
            while (j >= 0 && SortKey(cards[j], trumpSuit) > key)
            {
                cards[j + 1] = cards[j];
                j--;
            }
            cards[j + 1] = moved;
        }

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
            // brighter and stands out of the fan.
            int glow = 0;
            if (card == m_Picked)
                glow = m_Look.PickGlow;
            else if (Playing() && moves.Count() > 0)
                glow = m_Look.Glow;

            // The hand is drawn sorted: a card's look is found by the card,
            // not by its place in the row.
            string cardFace = "c_" + OZG_Cards.FaceOf(card);
            int cardLook = LookAt(m_Own.HeldLooks, m_Own.Held.Find(card));
            Widget pic = Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true);
            if (!pic)
                continue;
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
        m_Status.SetText(Caps(status));
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

        if (head == "deal")
            return Tr("STR_OZG_LOG_DEAL");
        if (head == "sit")
            return Tr1("STR_OZG_LOG_SIT", SeatName(rest.ToInt()));
        if (head == "over")
            return Tr1("STR_OZG_LOG_OVER", SeatName(rest.ToInt()));
        int shownNo = rest.ToInt() + 1;
        if (head == "stand")
            return Tr1("STR_OZG_LOG_STAND", shownNo.ToString());
        if (head == "drop")
            return Tr1("STR_OZG_LOG_DROP", shownNo.ToString());
        if (head == "left")
            return Tr1("STR_OZG_LOG_LEFT", shownNo.ToString());
        if (head == "host")
            return Tr1("STR_OZG_LOG_HOST", SeatName(rest.ToInt()));
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
        int lines = m_LogLines.Count();
        int have = m_V.Log.Count();
        int from = have - lines;
        if (from < 0)
            from = 0;
        for (int i = 0; i < lines; i++)
        {
            string text = "";
            if (from + i < have)
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
        SetLabel(m_BtnVariant, Caps(Tr1("STR_OZG_WIN_VARIANT", VariantLabel())));
        if (host && !playing)
        {
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
        for (int i = 0; i < all.Count(); i++)
        {
            Widget b = all[i];
            if (!b)
                continue;
            int at = shown.Find(b);
            b.Show(at >= 0);
            if (at >= 0)
                b.SetPos(m_Look.StackX * m_K, (m_Look.StackY + at * m_Look.StackStep) * m_K);
        }
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

    // A click on the cloth: the card under the pointer, if any.
    protected void ClickTable()
    {
        float x;
        float y;
        PointerAt(x, y);
        int at = HandAt(x, y);
        if (at >= 0)
        {
            ClickHand(m_HandCodes[at]);
            return;
        }
        at = AttackAt(x, y);
        if (at >= 0 && m_Picked != "")
        {
            int number = at + 1;
            string move = "d:" + m_Picked + ":" + number.ToString();
            if (CanPlay(move))
                Play(move);
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
            Send(OZG_Const.OP_DEAL, "", -1, "", "", "", 0);
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
