// The window of a game: one for players and watchers.
//
// Middle: the table -- the stock with the trump face up, the discards as a
// count, the attack cards with the cards that beat them laid over. Top: the
// seats in their order around the mat. Bottom: the player's hand, the cards
// that can be played now lit, and the buttons of the moment. Right: the log,
// and what the player and the host may do. A watcher sees the same without a
// hand and without buttons.
//
// The window shows what the server last said (OZG_Table) and asks for
// everything through the service: it decides nothing. A card is played with a
// click; when a click could mean more than one move the card is picked up
// first, and the next click says which.
//
// The table drawn here is durak's. Another game brings its own drawing of the
// two halves of a view; the frame around it -- seats, log, host -- is the
// same.
class OZG_GameMenu : UIScriptedMenu
{
    protected static OZG_GameMenu s_Current;
    protected static string s_ForId;

    protected static const string LAYOUT = "OpenZone_Games/gui/layouts/ozg_game.layout";
    protected static const string LAYOUT_CARD = "OpenZone_Games/gui/layouts/ozg_card.layout";
    protected static const string LAYOUT_SEAT = "OpenZone_Games/gui/layouts/ozg_seat.layout";

    // Layout units of the things a script places itself.
    protected static const float HAND_W = 110;
    protected static const float HAND_H = 154;
    protected static const float TABLE_W = 100;
    protected static const float TABLE_H = 140;
    protected static const float BOX_W = 1188;
    protected static const float COL_X = 1256;
    protected static const float STACK_Y = 410;
    protected static const float STACK_STEP = 42;

    protected string m_PlaceId;
    protected bool m_Claimed;
    protected bool m_Closing;
    protected int m_ShownRev;
    protected float m_K;
    protected float m_SaidLeft;
    protected float m_AskWait;
    protected float m_Health;

    protected Widget m_Win;
    protected Widget m_SeatsBox;
    protected Widget m_Bout;
    protected Widget m_HandBox;
    protected TextWidget m_Title;
    protected TextWidget m_Sub;
    protected TextWidget m_Status;
    protected TextWidget m_Said;
    protected TextWidget m_Foot;
    protected TextWidget m_Hint;
    protected TextWidget m_StockCount;
    protected TextWidget m_Discards;
    protected TextWidget m_Limit;
    protected TextWidget m_HostLine;
    protected TextWidget m_Result;
    protected ImageWidget m_Trump;
    protected ImageWidget m_StockBack;
    protected ImageWidget m_TrumpSuit;
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

    // What a script made, and what each of them stands for.
    protected ref array<Widget> m_HandW;
    protected ref array<string> m_HandCodes;
    protected ref array<Widget> m_AttackW;
    protected ref array<Widget> m_DefenceW;
    protected ref array<Widget> m_SeatW;

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
        m_LogLines = new array<TextWidget>();
        m_HandW = new array<Widget>();
        m_HandCodes = new array<string>();
        m_AttackW = new array<Widget>();
        m_DefenceW = new array<Widget>();
        m_SeatW = new array<Widget>();
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

    protected Widget Find(string name)
    {
        Widget w = layoutRoot.FindAnyWidget(name);
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

        m_Win = Find("OZG_Win");
        m_SeatsBox = Find("OZG_Seats");
        m_Bout = Find("OZG_Bout");
        m_HandBox = Find("OZG_Hand");
        m_Title = TextWidget.Cast(Find("OZG_Title"));
        m_Sub = TextWidget.Cast(Find("OZG_Sub"));
        m_Status = TextWidget.Cast(Find("OZG_Status"));
        m_Said = TextWidget.Cast(Find("OZG_Said"));
        m_Foot = TextWidget.Cast(Find("OZG_Foot"));
        m_Hint = TextWidget.Cast(Find("OZG_Hint"));
        m_StockCount = TextWidget.Cast(Find("OZG_StockCount"));
        m_Discards = TextWidget.Cast(Find("OZG_Discards"));
        m_Limit = TextWidget.Cast(Find("OZG_Limit"));
        m_HostLine = TextWidget.Cast(Find("OZG_HostLine"));
        m_Result = TextWidget.Cast(Find("OZG_Result"));
        m_Trump = ImageWidget.Cast(Find("OZG_Trump"));
        m_StockBack = ImageWidget.Cast(Find("OZG_StockBack"));
        m_TrumpSuit = ImageWidget.Cast(Find("OZG_TrumpSuit"));
        for (int i = 0; i < 14; i++)
            m_LogLines.Insert(TextWidget.Cast(Find("OZG_Log" + i.ToString())));

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
        return layoutRoot;
    }

    override void OnShow()
    {
        // The base locks the controls and shows the cursor; the excludes stop
        // the body from walking while the window is open.
        super.OnShow();
        if (!layoutRoot)
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
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (player)
            m_Health = player.GetHealth("", "");

        Measure();
        Redraw();
        // The view may be older than the table: ask once on opening.
        Send(OZG_Const.OP_OPEN, "", -1, "", "", "", 0);
    }

    override void OnHide()
    {
        if (m_V && m_V.Mine < 0)
            Send(OZG_Const.OP_UNWATCH, "", -1, "", "", "", 0);
        ClearList(m_HandW);
        ClearList(m_AttackW);
        ClearList(m_DefenceW);
        ClearList(m_SeatW);
        m_HandCodes.Clear();
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
        if (!layoutRoot || m_Closing)
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

        // A player who is hurt has to be able to answer: the window closes,
        // the seat is his.
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive() || player.IsUnconscious())
        {
            Shut();
            return;
        }
        float health = player.GetHealth("", "");
        if (health < m_Health - 0.5)
        {
            Shut();
            return;
        }
        m_Health = health;
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

    protected string Picture(string name)
    {
        string style = "classic";
        if (m_V && m_V.Style != "")
            style = m_V.Style;
        return "set:ozg_cards_" + style + " image:" + name;
    }

    protected void SetPicture(ImageWidget img, string name)
    {
        if (!img)
            return;
        img.LoadImageFile(0, Picture(name));
        img.SetImage(0);
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
            if (move.IndexOf(":" + card) >= 0)
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
        m_Title.SetText(title);

        string sub = "";
        if (m_V.Mine < 0)
            sub = Tr("STR_OZG_ST_WATCHER");
        m_Sub.SetText(sub);
        m_Foot.SetText(Tr("STR_OZG_WIN_FOOT"));
    }

    protected void PaintSeats()
    {
        ClearList(m_SeatW);
        int n = m_V.Seats.Count();
        if (n == 0)
            return;
        float pitch = BOX_W / n;
        if (pitch > 148)
            pitch = 148;

        for (int i = 0; i < n; i++)
        {
            OZG_WSeat s = m_V.Seats[i];
            Widget chip = GetGame().GetWorkspace().CreateWidgets(LAYOUT_SEAT, m_SeatsBox);
            if (!chip)
                continue;
            chip.SetPos(i * pitch * m_K, 0);
            m_SeatW.Insert(chip);

            TextWidget name = TextWidget.Cast(chip.FindAnyWidget("OZG_SeatName"));
            TextWidget count = TextWidget.Cast(chip.FindAnyWidget("OZG_SeatCount"));
            TextWidget role = TextWidget.Cast(chip.FindAnyWidget("OZG_SeatRole"));
            TextWidget note = TextWidget.Cast(chip.FindAnyWidget("OZG_SeatNote"));
            ImageWidget back = ImageWidget.Cast(chip.FindAnyWidget("OZG_SeatBack"));
            Widget bg = chip.FindAnyWidget("OZG_SeatBg");
            Widget sel = chip.FindAnyWidget("OZG_SeatSel");

            string who = s.Name;
            if (s.St == OZG_Const.SEAT_FREE)
                who = Tr("STR_OZG_WIN_SEAT_FREE");
            else if (s.St == OZG_Const.SEAT_LEFT)
                who = Tr("STR_OZG_WIN_SEAT_LEFT");
            name.SetText(who);

            // What the game says of the seat.
            string does = "";
            string cards = "";
            OZG_DurakSeatPublic g = GameSeat(i);
            bool shown = false;
            // A seat that is free now says nothing of a game that has ended.
            if (g && m_Pub.Ended == 1 && s.St == OZG_Const.SEAT_FREE)
                g = null;
            if (g && m_V.St != OZG_Const.ST_LOBBY)
            {
                cards = g.CardCount.ToString();
                shown = true;
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
                    does = does + " ...";
            }
            else if (Playing() && s.St != OZG_Const.SEAT_FREE)
            {
                does = Tr("STR_OZG_WIN_ROLE_WATCH");
            }
            count.SetText(cards);
            role.SetText(does);
            back.Show(shown);
            if (shown)
                SetPicture(back, "back");

            string marks = "";
            if (i == m_V.Mine)
                marks = Tr("STR_OZG_WIN_YOU");
            if (i == m_V.HostSeat)
            {
                if (marks != "")
                    marks = marks + ", ";
                marks = marks + Tr("STR_OZG_WIN_HOST_MARK");
            }
            if (s.St == OZG_Const.SEAT_KEPT)
            {
                if (marks != "")
                    marks = marks + ", ";
                marks = marks + Tr("STR_OZG_WIN_AWAY");
            }
            note.SetText(marks);

            int tint = ARGB(235, 13, 31, 23);
            if (i == m_V.Mine)
                tint = ARGB(245, 28, 62, 44);
            else if (s.St == OZG_Const.SEAT_LEFT)
                tint = ARGB(245, 74, 60, 20);
            else if (s.St == OZG_Const.SEAT_FREE)
                tint = ARGB(160, 10, 20, 16);
            else if (s.St == OZG_Const.SEAT_KEPT)
                tint = ARGB(235, 30, 30, 30);
            bg.SetColor(tint);
            sel.Show(i == m_PickedSeat);
        }
    }

    protected Widget MakeCard(Widget parent, string picture, float x, float y, float w, float h)
    {
        Widget card = GetGame().GetWorkspace().CreateWidgets(LAYOUT_CARD, parent);
        if (!card)
            return null;
        card.SetPos(x * m_K, y * m_K);
        card.SetSize(w * m_K, h * m_K);
        SetPicture(ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace")), picture);
        return card;
    }

    protected void PaintTable()
    {
        ClearList(m_AttackW);
        ClearList(m_DefenceW);
        m_Result.Show(false);
        m_Trump.Show(false);
        m_StockBack.Show(false);
        m_TrumpSuit.Show(false);
        m_StockCount.SetText("");
        m_Discards.SetText("");
        m_Limit.SetText("");
        Widget trumpLabel = layoutRoot.FindAnyWidget("OZG_TrumpLabel");
        bool dealt = m_Pub && m_V.St != OZG_Const.ST_LOBBY;
        if (trumpLabel)
            trumpLabel.Show(dealt);
        if (!dealt)
            return;

        // The stock: the trump face up under it; when the stock is gone the
        // trump's suit stays to be seen.
        if (m_Pub.TrumpCard != "")
        {
            if (m_Pub.StockCount > 0)
            {
                m_Trump.Show(true);
                SetPicture(m_Trump, "c_" + m_Pub.TrumpCard);
            }
            else
            {
                m_TrumpSuit.Show(true);
                SetPicture(m_TrumpSuit, "suit_" + m_Pub.TrumpCard.Substring(1, 1));
            }
        }
        if (m_Pub.StockCount > 1)
        {
            m_StockBack.Show(true);
            SetPicture(m_StockBack, "back");
        }
        m_StockCount.SetText(m_Pub.StockCount.ToString());
        m_Discards.SetText(Tr1("STR_OZG_WIN_DISCARDS", m_Pub.DiscardCount.ToString()));
        if (m_Pub.Ended != 1)
            m_Limit.SetText(Tr1("STR_OZG_WIN_LIMIT", m_Pub.AttackLimit.ToString()));

        // The bout: each attack card, and the card that beat it laid over.
        array<string> picked = new array<string>();
        if (m_Picked != "")
            MovesOf(m_Picked, picked);
        int n = m_Pub.AttackCards.Count();
        float pitch = 142;
        if (n > 6)
            pitch = 852.0 / n;
        for (int i = 0; i < n; i++)
        {
            Widget attack = MakeCard(m_Bout, "c_" + m_Pub.AttackCards[i], i * pitch, 16, TABLE_W, TABLE_H);
            m_AttackW.Insert(attack);
            int number = i + 1;
            bool target = picked.Find("d:" + m_Picked + ":" + number.ToString()) >= 0;
            if (attack && target)
                attack.FindAnyWidget("OZG_CardGlow").Show(true);

            string beaten = "";
            if (i < m_Pub.DefenceCards.Count())
                beaten = m_Pub.DefenceCards[i];
            if (beaten != "")
                m_DefenceW.Insert(MakeCard(m_Bout, "c_" + beaten, i * pitch + 30, 70, TABLE_W, TABLE_H));
        }

        if (m_Pub.Ended == 1)
        {
            string over = Tr("STR_OZG_ST_OVER_NONE");
            if (m_Pub.Tie == 1)
                over = Tr("STR_OZG_ST_OVER_TIE");
            else if (m_Pub.DurakSeat >= 0)
                over = Tr1("STR_OZG_ST_OVER_DURAK", SeatName(m_Pub.DurakSeat));
            m_Result.SetText(over);
            m_Result.Show(true);
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
        ClearList(m_HandW);
        m_HandCodes.Clear();
        if (!m_Own || !m_Own.Held || m_V.St == OZG_Const.ST_LOBBY)
            return;

        string trumpSuit = "";
        if (m_Pub && m_Pub.TrumpCard.Length() == 2)
            trumpSuit = m_Pub.TrumpCard.Substring(1, 1);

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

        float step = HAND_W + 8;
        if (n > 1)
        {
            float fit = (BOX_W - HAND_W) / (n - 1);
            if (fit < step)
                step = fit;
        }
        float span = HAND_W;
        if (n > 1)
            span = step * (n - 1) + HAND_W;
        float x0 = (BOX_W - span) * 0.5;

        array<string> moves = new array<string>();
        for (i = 0; i < n; i++)
        {
            string card = cards[i];
            MovesOf(card, moves);
            float y = 22;
            if (card == m_Picked)
                y = 2;
            Widget w = MakeCard(m_HandBox, "c_" + card, x0 + i * step, y, HAND_W, HAND_H);
            if (!w)
                continue;
            // A card that cannot be played now is dimmed by its own colour.
            bool lit = moves.Count() > 0;
            if (!lit && Playing())
                w.FindAnyWidget("OZG_CardFace").SetColor(ARGB(255, 150, 150, 150));
            w.FindAnyWidget("OZG_CardGlow").Show(card == m_Picked);
            m_HandW.Insert(w);
            m_HandCodes.Insert(card);
        }
    }

    // The buttons of the moment, and the line that says whose move it is.
    protected void PaintMoment()
    {
        bool pass = CanPlay("pass");
        bool take = CanPlay("take");
        bool moveOn = m_Picked != "" && CanPlay("t:" + m_Picked);
        m_BtnPass.Show(pass);
        m_BtnTake.Show(take);
        m_BtnMove.Show(moveOn);

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
        m_Status.SetText(status);
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
        if (rest.Length() >= 4)
        {
            string kind = rest.Substring(0, 1);
            string card = rest.Substring(2, 2);
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
        m_HostLine.SetText(hostLine);

        // The seat the host picked on the strip.
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
        SetLabel(m_BtnVariant, Tr1("STR_OZG_WIN_VARIANT", VariantLabel()));
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
                SetLabel(m_BtnPose, Tr1("STR_OZG_WIN_POSE", pose));
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
                b.SetPos(COL_X * m_K, (STACK_Y + at * STACK_STEP) * m_K);
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

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w || button != MouseState.LEFT || !m_V)
            return false;

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

        int at = m_HandW.Find(w);
        if (at >= 0)
        {
            ClickHand(m_HandCodes[at]);
            return true;
        }

        at = m_AttackW.Find(w);
        if (at >= 0 && m_Picked != "")
        {
            int number = at + 1;
            string move = "d:" + m_Picked + ":" + number.ToString();
            if (CanPlay(move))
                Play(move);
            return true;
        }

        at = m_SeatW.Find(w);
        if (at >= 0)
        {
            if (m_V.IsHost == 1)
            {
                if (m_PickedSeat == at)
                    m_PickedSeat = -1;
                else
                    m_PickedSeat = at;
                PaintSeats();
                PaintColumn();
            }
            return true;
        }
        return false;
    }
}
