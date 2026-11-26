// The card sharp's kit's window (boards and dice design §10): the design canvas's board
// "Шулерський набір", built of ozg_kwin.layout (its cards ozg_kcard.layout, its dice
// ozg_kdie.layout) by tools/make_board_win.py. Left the kit and its uses; in the middle
// this player's cards and dice where he carries them, each with what he knows of it (his
// mark, a mark he reads, a load he knows); right the one he picked -- a card to mark, a
// die to load on a face and a weight. The kit works on it a moment (the canvas: 2.3 s, a
// bar filling), then the server does it. Another's mark or load opens to him when he
// picks the thing. The window decides nothing: the server says what he carries and
// knows, and does what he asks (OZG_KitService).
class OZG_KitMenu : UIScriptedMenu
{
    protected static OZG_KitMenu s_Current;
    protected static ref OZG_WKit s_Next;
    protected static ref array<string> s_Reslashed;

    protected static const string LAYOUT_ROOT = "OpenZone_Games/gui/layouts/ozg_game.layout";
    protected static const string LAYOUT_WIN = "OpenZone_Games/gui/layouts/ozg_kwin.layout";
    protected static const string LAYOUT_CARD = "OpenZone_Games/gui/layouts/ozg_kcard.layout";
    protected static const string LAYOUT_DIE = "OpenZone_Games/gui/layouts/ozg_kdie.layout";
    protected static const string TEX = "OpenZone_Games/gui/textures/";
    // How long the kit works (the canvas: 2.3 s, its bar full at 2).
    protected static const float WORK_TIME = 2.3;
    protected static const float WORK_FULL = 2.0;
    // The middle panel's things, in its scroll (which lies 1 inside the panel's edge): from
    // the canvas's padding to its padding (16 at the sides, 14 at the top), 12 apart; the
    // button that picks one stops short of the scroll's bar.
    protected static const float THINGS_X = 15;
    protected static const float THINGS_TOP = 13;
    protected static const float THINGS_RIGHT = 731;
    protected static const float THINGS_GAP = 12;
    protected static const float SCROLL_W = 746;
    protected static const float SCROLL_H = 772;
    protected static const float HIT_W = 734;
    protected static const float CARD_W = 86;
    protected static const float CARD_H = 129;
    protected static const float DIE_SIDE = 64;
    // The right panel: a card's back beside its face, 112 x 168.
    protected static const float BACK_X = 138;
    protected static const float PICK_Y = 55;
    protected static const float PICK_W = 112;
    protected static const float PICK_H = 168;

    protected Widget m_Win;
    protected float m_K = 1;
    protected bool m_Claimed;
    protected ref OZG_WKit m_Kit;
    // the thing picked (its key), the face and the weight chosen for a die
    protected string m_Pick = "";
    protected int m_Face = 6;
    protected int m_Step = 2;
    // the kit at work: time gone (below 0: idle), and what it asks when done
    protected float m_Work = -1;
    protected string m_WorkOp = "";
    protected string m_WorkKey = "";
    protected int m_WorkFace;
    protected int m_WorkStep;
    protected string m_WorkText = "";
    // the things already looked at (another's work on one opens on the first look)
    protected ref array<string> m_Looked;
    // the cards and dice laid, reused paint to paint
    protected ref array<Widget> m_Cards;
    protected ref array<Widget> m_Dice;
    protected int m_CardsUsed;
    protected int m_DiceUsed;
    // how tall the middle's things are (its scroll's content)
    protected float m_ThingsH;
    // the things as laid: key and box in the middle panel's units
    protected ref array<string> m_HitKeys;
    protected ref array<float> m_HitX;
    protected ref array<float> m_HitY;
    protected ref array<float> m_HitW;
    protected ref array<float> m_HitH;

    void OZG_KitMenu()
    {
        m_Looked = new array<string>();
        m_Cards = new array<Widget>();
        m_Dice = new array<Widget>();
        m_HitKeys = new array<string>();
        m_HitX = new array<float>();
        m_HitY = new array<float>();
        m_HitW = new array<float>();
        m_HitH = new array<float>();
        if (!s_Reslashed)
            s_Reslashed = new array<string>();
    }

    void ~OZG_KitMenu()
    {
        if (s_Current == this)
            s_Current = null;
    }

    static OZG_KitMenu Current()
    {
        return s_Current;
    }

    // The server's answer (OZG_MissionGameplay.OZG_SvcRes): the action's view opens the
    // window; an answer to the window refreshes it, or says why not.
    static void Answer(string op, bool ok, string json, string error)
    {
        if (!ok)
        {
            string text = Words(error);
            if (s_Current)
                s_Current.Refused(text);
            else if (text != "")
                NotificationSystem.AddNotificationExtended(4, "#STR_OZG_MODNAME", text, "");
            return;
        }
        OZG_WKit kit;
        string bad;
        if (!JsonFileLoader<OZG_WKit>.LoadData(json, kit, bad) || !kit)
            return;
        if (s_Current)
        {
            // the kit's work done, or another's work opened: its click (the canvas's sound)
            bool worked = op == OZG_Const.OP_KIT_MARK || op == OZG_Const.OP_KIT_LOAD;
            if (worked || kit.Said != "")
                OZG_Sounds.PlayLocal("click", null);
            s_Current.Use(kit);
            return;
        }
        if (op != OZG_Const.OP_KIT_VIEW)
            return;
        // never over a player who is not there to use it
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive() || player.IsUnconscious())
            return;
        s_Next = kit;
        GetGame().GetUIManager().EnterScriptedMenu(OZG_Const.MENU_KIT, null);
    }

    // Words the server said (OZG_Words), in this client's language: "k:" an argument that
    // is a key, "l:" a key whose first letter is written small (a name inside a sentence).
    static string Words(string words)
    {
        array<string> parts = new array<string>();
        OZG_Words.Parts(words, parts);
        if (parts.Count() == 0)
            return "";
        string s = Widget.TranslateString("#" + parts[0]);
        for (int i = 1; i < parts.Count(); i++)
        {
            string arg = parts[i];
            int n = arg.Length();
            if (n > 2 && arg.IndexOf("k:") == 0)
                arg = Widget.TranslateString("#" + arg.Substring(2, n - 2));
            else if (n > 2 && arg.IndexOf("l:") == 0)
                arg = OZG_Text.LowerFirst(Widget.TranslateString("#" + arg.Substring(2, n - 2)));
            s.Replace("%" + i.ToString(), arg);
        }
        return s;
    }

    // A layout's widgets under `parent`, checked by the root's name (OZG_GameMenu.MakeLayout:
    // the engine can hand back a layout made earlier for another path).
    protected static Widget MakeLayout(string path, Widget parent, string root)
    {
        string asked = path;
        if (s_Reslashed.Find(path) >= 0)
            asked.Replace("/", "\\");
        Widget w = GetGame().GetWorkspace().CreateWidgets(asked, parent);
        if (w && w.GetName() != root && asked == path)
        {
            w.Unlink();
            s_Reslashed.Insert(path);
            asked.Replace("/", "\\");
            w = GetGame().GetWorkspace().CreateWidgets(asked, parent);
        }
        if (!w)
            OZG_Log.Error("the kit's window could not load " + path);
        return w;
    }

    override Widget Init()
    {
        layoutRoot = GetGame().GetWorkspace().CreateWidgets(LAYOUT_ROOT);
        if (!layoutRoot)
            return null;
        m_Win = MakeLayout(LAYOUT_WIN, layoutRoot, "OZG_KWin");
        return layoutRoot;
    }

    override void OnShow()
    {
        super.OnShow();
        if (!layoutRoot || !m_Win)
        {
            Close();
            return;
        }
        s_Current = this;
        m_Claimed = true;
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
        // the captions the canvas writes in capitals
        SetText("OZG_KFaceLabel", OZG_Text.Upper(Tr("STR_OZG_KWIN_FACE")));
        SetText("OZG_KWeightLabel", OZG_Text.Upper(Tr("STR_OZG_KWIN_WEIGHT")));
        Use(s_Next);
        s_Next = null;
    }

    override void OnHide()
    {
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

    // What the window shuts out (as the game window does): voice stays.
    protected static array<string> ShutOut()
    {
        array<string> groups = new array<string>();
        groups.Insert("inventory");
        groups.Insert("hotkey");
        groups.Insert("movement");
        groups.Insert("UAGear");
        return groups;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!layoutRoot || !m_Win)
            return;
        if (GetUApi().GetInputByID(UAUIBack).LocalPress())
        {
            Close();
            return;
        }
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.IsAlive() || player.IsUnconscious())
        {
            Close();
            return;
        }
        if (m_Work < 0)
            return;
        m_Work = m_Work + timeslice;
        if (m_Work < WORK_TIME)
        {
            PaintWork();
            return;
        }
        m_Work = -1;
        OZG_WAsk ask = new OZG_WAsk();
        ask.CardKey = m_WorkKey;
        ask.SeatNo = m_WorkFace;
        ask.PoseNo = m_WorkStep;
        Ask(m_WorkOp, ask);
        Paint();
    }

    protected void Ask(string op, OZG_WAsk ask)
    {
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, op, json);
    }

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

    protected void Use(OZG_WKit kit)
    {
        if (kit)
            m_Kit = kit;
        if (!m_Kit)
            m_Kit = new OZG_WKit();
        Measure();
        Paint();
    }

    // A refusal the server gave: at the foot of the right panel.
    void Refused(string text)
    {
        if (!m_Kit)
            return;
        m_Kit.Said = "";
        m_Kit.SaidKey = text;
        Paint();
    }

    // ------------------------------------------------------------ helpers

    protected Widget W(string name)
    {
        if (!m_Win)
            return null;
        return m_Win.FindAnyWidget(name);
    }

    protected void SetText(string name, string text)
    {
        TextWidget t = TextWidget.Cast(W(name));
        if (t)
            t.SetText(text);
    }

    protected void Ink(string name, int colour)
    {
        Widget w = W(name);
        if (w)
            w.SetColor(colour);
    }

    protected void ShowW(string name, bool show)
    {
        Widget w = W(name);
        if (w)
            w.Show(show);
    }

    protected void MoveW(string name, float x, float y)
    {
        Widget w = W(name);
        if (w)
            w.SetPos(x * m_K, y * m_K);
    }

    protected string Tr(string key)
    {
        return Widget.TranslateString("#" + key);
    }

    protected float TextW(TextWidget t)
    {
        if (!t || m_K <= 0)
            return 0;
        int w;
        int h;
        t.Update();
        t.GetTextSize(w, h);
        return w / m_K;
    }

    // Words in lines no wider than `w` units, into the text widgets `name`0 .. count-1;
    // how many lines they took.
    protected int Lines(string name, int count, string text, float w)
    {
        TextWidget probe = TextWidget.Cast(W(name + "0"));
        array<string> lines = new array<string>();
        if (probe && text != "")
        {
            array<string> words = new array<string>();
            text.Split(" ", words);
            string line = "";
            for (int i = 0; i < words.Count(); i++)
            {
                string tried = words[i];
                if (line != "")
                    tried = line + " " + words[i];
                probe.SetText(tried);
                if (line != "" && TextW(probe) > w)
                {
                    lines.Insert(line);
                    line = words[i];
                }
                else
                {
                    line = tried;
                }
            }
            if (line != "")
                lines.Insert(line);
        }
        for (int k = 0; k < count; k++)
        {
            string shown = "";
            if (k < lines.Count())
                shown = lines[k];
            if (k == count - 1 && lines.Count() > count)
                shown = shown + "…";
            SetText(name + k.ToString(), shown);
        }
        int used = lines.Count();
        if (used > count)
            used = count;
        return used;
    }

    // A shape's parts recoloured (tools/make_board_win.py shape()).
    protected void Paint7(Widget shape, int colour)
    {
        if (!shape)
            return;
        Widget child = shape.GetChildren();
        if (!child)
        {
            shape.SetColor(colour);
            return;
        }
        while (child)
        {
            child.SetColor(colour);
            child = child.GetSibling();
        }
    }

    protected void PaintShape(string name, int colour)
    {
        Paint7(W(name), colour);
    }

    // A shape's seven parts laid out again for another width or height, in units: in
    // whole pixels of the shape, the bands a pixel over the corners and each other (the
    // game window's BWFit says why).
    protected void Fit7(Widget shape, float w, float h, float r)
    {
        if (!shape)
            return;
        string n = shape.GetName();
        float pw = Math.Round(w * m_K);
        float ph = Math.Round(h * m_K);
        float pr = Math.Round(r * m_K);
        if (pr * 2 > pw)
            pr = Math.Floor(pw / 2);
        if (pr * 2 > ph)
            pr = Math.Floor(ph / 2);
        if (pr < 0)
            pr = 0;
        float o = 1;
        if (o > pr)
            o = pr;
        shape.SetSize(pw, ph);
        Part(shape, n + "TL", 0, 0, pr, pr);
        Part(shape, n + "TR", pw - pr, 0, pr, pr);
        Part(shape, n + "BL", 0, ph - pr, pr, pr);
        Part(shape, n + "BR", pw - pr, ph - pr, pr, pr);
        Part(shape, n + "M", 0, pr - o, pw, ph - 2 * pr + 2 * o);
        Part(shape, n + "T", pr - o, 0, pw - 2 * pr + 2 * o, pr + o);
        Part(shape, n + "B", pr - o, ph - pr - o, pw - 2 * pr + 2 * o, pr + o);
    }

    // A part's place in its shape, in pixels.
    protected void Part(Widget shape, string name, float x, float y, float w, float h)
    {
        Widget p = shape.FindAnyWidget(name);
        if (!p)
            return;
        p.SetPos(x, y);
        p.SetSize(w, h);
    }

    protected void Picture(Widget holder, string name, string picture, int colour)
    {
        if (!holder)
            return;
        ImageWidget pic = ImageWidget.Cast(holder.FindAnyWidget(name));
        if (!pic)
            return;
        pic.LoadImageFile(0, picture);
        pic.SetImage(0);
        pic.SetColor(colour);
    }

    // A card of a deck's look: "c_Qh", "back".
    protected string CardPicture(string look, string name)
    {
        string style = look;
        if (style == "")
            style = "classic";
        return "set:ozg_cards_" + style + " image:" + name;
    }

    // "Qh" -> the card's name in the stringtable (STR_OZG_CARD_QH).
    protected string CardName(string face)
    {
        string up = face;
        up.ToUpper();
        return Tr("STR_OZG_CARD_" + up);
    }

    // A count and its noun in the plural it takes (Ukrainian: 1, 2-4, 5 and up).
    protected string Counted(string key, int n)
    {
        int ten = n % 10;
        int hundred = n % 100;
        string form = "5";
        bool teens = hundred >= 12 && hundred <= 14;
        if (ten == 1 && hundred != 11)
            form = "1";
        else if (ten >= 2 && ten <= 4 && !teens)
            form = "2";
        string s = Tr(key + form);
        s.Replace("%1", n.ToString());
        return s;
    }

    protected int Uses()
    {
        if (!m_Kit || m_Kit.Uses < 0)
            return 0;
        return m_Kit.Uses;
    }

    // ------------------------------------------------------------ painting

    protected void Paint()
    {
        if (!m_Win || !m_Kit)
            return;
        m_CardsUsed = 0;
        m_DiceUsed = 0;
        PaintLeft();
        PaintThings();
        PaintRight();
        int i;
        for (i = m_CardsUsed; i < m_Cards.Count(); i++)
        {
            if (m_Cards[i])
                m_Cards[i].Show(false);
        }
        for (i = m_DiceUsed; i < m_Dice.Count(); i++)
        {
            if (m_Dice[i])
                m_Dice[i].Show(false);
        }
    }

    protected void PaintLeft()
    {
        int uses = Uses();
        for (int i = 0; i < 3; i++)
        {
            ImageWidget pip = ImageWidget.Cast(W("OZG_KUse" + i.ToString()));
            if (!pip)
                continue;
            if (i < uses)
                pip.LoadImageFile(0, TEX + "ozg_disc_ca.paa");
            else
                pip.LoadImageFile(0, TEX + "ozg_ring6_ca.paa");
            pip.SetImage(0);
        }
        Lines("OZG_KNoteA", 3, Tr("STR_OZG_KWIN_NOTE_A"), 312);
        Lines("OZG_KNoteB", 4, Tr("STR_OZG_KWIN_NOTE_B"), 312);
        Lines("OZG_KFoot", 2, Tr("STR_OZG_KWIN_FOOT"), 312);
    }

    // The things, group by group: the pockets, the deck in the hands, a can of dice.
    protected void PaintThings()
    {
        m_HitKeys.Clear();
        m_HitX.Clear();
        m_HitY.Clear();
        m_HitW.Clear();
        m_HitH.Clear();
        Widget layer = W("OZG_KThings");
        array<string> wheres = new array<string>();
        wheres.Insert("pocket");
        wheres.Insert("deck");
        wheres.Insert("can");
        array<string> titles = new array<string>();
        titles.Insert("STR_OZG_KWIN_POCKET");
        titles.Insert("STR_OZG_KWIN_DECK");
        titles.Insert("STR_OZG_KWIN_CAN");
        float y = THINGS_TOP;
        int groups = 0;
        for (int g = 0; g < 3; g++)
        {
            int cards = 0;
            int dice = 0;
            string look = "";
            int t;
            for (t = 0; t < m_Kit.Things.Count(); t++)
            {
                OZG_WKitThing c = m_Kit.Things[t];
                if (c.Where != wheres[g])
                    continue;
                if (c.Kind == "card")
                {
                    cards++;
                    look = c.Look;
                }
                else
                {
                    dice++;
                }
            }
            string gi = g.ToString();
            bool shown = cards + dice > 0;
            ShowW("OZG_KGroup" + gi, shown);
            ShowW("OZG_KGroupNote" + gi, shown);
            if (!shown)
                continue;
            groups++;
            TextWidget label = TextWidget.Cast(W("OZG_KGroup" + gi));
            if (label)
            {
                label.SetText(OZG_Text.Upper(Tr(titles[g])));
                label.SetPos(THINGS_X * m_K, y * m_K);
                TextWidget note = TextWidget.Cast(W("OZG_KGroupNote" + gi));
                if (note)
                {
                    note.SetText(GroupNote(wheres[g], cards, dice, look));
                    note.SetPos((THINGS_X + TextW(label) + 10) * m_K, y * m_K);
                }
            }
            y = y + 25;
            float x = THINGS_X;
            float rowH = 0;
            for (t = 0; t < m_Kit.Things.Count(); t++)
            {
                OZG_WKitThing th = m_Kit.Things[t];
                if (th.Where != wheres[g])
                    continue;
                float w = DIE_SIDE;
                float h = DIE_SIDE;
                if (th.Kind == "card")
                {
                    w = CARD_W;
                    h = CARD_H;
                }
                if (x > THINGS_X && x + w > THINGS_RIGHT)
                {
                    x = THINGS_X;
                    y = y + rowH + THINGS_GAP;
                    rowH = 0;
                }
                if (th.Kind == "card")
                    LayCard(layer, th, x, y);
                else
                    LayDie(layer, th, x, y);
                m_HitKeys.Insert(th.Key);
                m_HitX.Insert(x);
                m_HitY.Insert(y);
                m_HitW.Insert(w);
                m_HitH.Insert(h);
                x = x + w + THINGS_GAP;
                if (h > rowH)
                    rowH = h;
            }
            y = y + rowH + 14;
        }
        m_ThingsH = y;
        ShowW("OZG_KEmpty", groups == 0);
        FitScroll();
    }

    // The scroll's content the height of the things, and the scroll told so: it notices a
    // content that grew only on Update (dayz-modding gui-layouts §31). Fewer things than when
    // it was scrolled: no further than they reach. Its positions are pixels.
    protected void FitScroll()
    {
        Widget things = W("OZG_KThings");
        ScrollWidget scroll = ScrollWidget.Cast(W("OZG_KScroll"));
        if (!things || !scroll)
            return;
        float h = m_ThingsH;
        things.SetSize(SCROLL_W * m_K, h * m_K);
        Widget hit = W("OZG_KHit");
        if (hit)
            hit.SetSize(HIT_W * m_K, h * m_K);
        things.Update();
        scroll.Update();
        float most = (h - SCROLL_H) * m_K;
        if (most < 0)
            most = 0;
        float now = scroll.GetVScrollPos();
        if (now > most)
            scroll.VScrollToPos(most);
    }

    // What a group holds, in words: "2 карти й 2 кістки"; a deck's look before its count.
    protected string GroupNote(string where, int cards, int dice, string look)
    {
        string s = "";
        if (cards > 0)
            s = Counted("STR_OZG_KWIN_CARDS_", cards);
        if (dice > 0)
        {
            string d = Counted("STR_OZG_KWIN_DICE_", dice);
            if (s == "")
            {
                s = d;
            }
            else
            {
                string both = Tr("STR_OZG_KWIN_AND");
                both.Replace("%1", s);
                both.Replace("%2", d);
                s = both;
            }
        }
        bool named = look == "classic" || look == "stalker";
        if (where == "deck" && named)
        {
            string style = look;
            style.ToUpper();
            string withLook = Tr("STR_OZG_KWIN_DECK_NOTE");
            withLook.Replace("%1", Tr("STR_OZG_KWIN_LOOK_" + style));
            withLook.Replace("%2", s);
            s = withLook;
        }
        return s;
    }

    // A widget of the pool (cards or dice) for the next thing, made when short.
    protected Widget Pooled(array<Widget> pool, int at, string path, string root, Widget parent)
    {
        Widget w;
        if (at < pool.Count())
            w = pool[at];
        if (w && w.GetParent() == parent)
            return w;
        if (w)
            w.Unlink();
        w = MakeLayout(path, parent, root);
        if (at < pool.Count())
            pool[at] = w;
        else
            pool.Insert(w);
        return w;
    }

    protected void LayCard(Widget layer, OZG_WKitThing th, float x, float y)
    {
        Widget w = Pooled(m_Cards, m_CardsUsed, LAYOUT_CARD, "OZG_KCard", layer);
        m_CardsUsed++;
        if (!w)
            return;
        w.SetPos(x * m_K, y * m_K);
        w.Show(true);
        Picture(w, "OZG_KCardPic", CardPicture(th.Look, "c_" + th.Face), ARGB(255, 255, 255, 255));
        Widget ring = w.FindAnyWidget("OZG_KCardRing");
        if (ring)
            ring.Show(th.Key == m_Pick);
        Widget tag = w.FindAnyWidget("OZG_KCardTag");
        if (!tag)
            return;
        tag.Show(th.Mark > 0);
        if (th.Mark <= 0)
            return;
        TextWidget text = TextWidget.Cast(w.FindAnyWidget("OZG_KCardTagText"));
        if (!text)
            return;
        text.SetText(TagWord(th.Mark));
        float tw = TextW(text) + 10;
        text.SetSize(tw * m_K, 16 * m_K);
        tag.SetSize(tw * m_K, 16 * m_K);
        Fit7(w.FindAnyWidget("OZG_KCardTagBg"), tw, 16, 3);
    }

    protected string TagWord(int mark)
    {
        if (mark == 1)
            return Tr("STR_OZG_KWIN_TAG_MINE");
        return Tr("STR_OZG_KWIN_TAG_READ");
    }

    // A die's face up in the window: the one it is loaded on when he knows it, else one
    // its key gives (the same each time it is drawn).
    protected int FaceUp(OZG_WKitThing th)
    {
        if (th.LoadFace > 0)
            return th.LoadFace;
        int sum = 0;
        string key = th.Key;
        int n = key.Length();
        for (int i = 0; i < n; i++)
        {
            string ch = key.Substring(i, 1);
            sum = sum + ch.ToAscii();
        }
        return 1 + sum % 6;
    }

    protected string DieLook(OZG_WKitThing th)
    {
        if (th.Look == "")
            return "white";
        return th.Look;
    }

    protected void LayDie(Widget layer, OZG_WKitThing th, float x, float y)
    {
        Widget w = Pooled(m_Dice, m_DiceUsed, LAYOUT_DIE, "OZG_KDie", layer);
        m_DiceUsed++;
        if (!w)
            return;
        w.SetPos(x * m_K, y * m_K);
        w.Show(true);
        int face = FaceUp(th);
        Picture(w, "OZG_KDieSh", "set:ozg_dice image:die_s", ARGB(255, 255, 255, 255));
        Picture(w, "OZG_KDiePic", "set:ozg_dice image:" + DieLook(th) + "_" + face.ToString(), ARGB(255, 255, 255, 255));
        Widget ring = w.FindAnyWidget("OZG_KDieRing");
        if (ring)
            ring.Show(th.Key == m_Pick);
        Widget badge = w.FindAnyWidget("OZG_KDieBadge");
        if (badge)
            badge.Show(th.LoadFace > 0);
        TextWidget number = TextWidget.Cast(w.FindAnyWidget("OZG_KDieBadgeText"));
        if (number && th.LoadFace > 0)
            number.SetText(th.LoadFace.ToString());
    }

    protected OZG_WKitThing Picked()
    {
        if (!m_Kit || m_Pick == "")
            return null;
        for (int i = 0; i < m_Kit.Things.Count(); i++)
        {
            if (m_Kit.Things[i].Key == m_Pick)
                return m_Kit.Things[i];
        }
        return null;
    }

    // The right panel: what to pick; the card picked; the die picked; the kit at work.
    protected void PaintRight()
    {
        OZG_WKitThing th = Picked();
        if (!th)
            m_Pick = "";
        bool working = m_Work >= 0;
        string kind = "";
        if (th && !working)
            kind = th.Kind;
        ShowW("OZG_KPickTitle", !working);
        ShowW("OZG_KWork", working);
        ShowW("OZG_KPickCard", kind == "card");
        bool die = kind == "die";
        for (int f = 0; f < 6; f++)
            ShowW("OZG_KFace" + f.ToString(), die);
        for (int s = 0; s < 3; s++)
            ShowW("OZG_KWeight" + s.ToString(), die);
        ShowW("OZG_KFaceLabel", die);
        ShowW("OZG_KWeightLabel", die);
        ShowW("OZG_KDo", kind != "");
        for (int p = 0; p < 3; p++)
            ShowW("OZG_KPickText" + p.ToString(), !working);
        if (working)
        {
            SetText("OZG_KWorkText", m_WorkText);
            PaintWork();
        }
        else if (kind == "card")
        {
            PaintCard(th);
        }
        else if (kind == "die")
        {
            PaintDie(th);
        }
        else
        {
            SetText("OZG_KPickTitle", Tr("STR_OZG_KWIN_PICK"));
            PickTextAt(PICK_Y, ARGB(255, 184, 165, 133));
            Lines("OZG_KPickText", 3, Tr("STR_OZG_KWIN_PICK_TEXT"), 356);
        }
        // what was just done, or refused
        string said = "";
        if (m_Kit.SaidKey != "")
            said = m_Kit.SaidKey;
        else if (m_Kit.Said != "")
            said = Words(m_Kit.Said);
        Lines("OZG_KSaid", 2, said, 356);
    }

    protected void PickTextAt(float y, int colour)
    {
        for (int i = 0; i < 3; i++)
        {
            string name = "OZG_KPickText" + i.ToString();
            MoveW(name, 14, y + i * 20);
            Ink(name, colour);
        }
    }

    // The action's button: gold while the kit has a use, dark when it has none.
    protected void PaintDo(string label, bool can)
    {
        int fill = ARGB(255, 201, 164, 92);
        int ink = ARGB(255, 26, 18, 10);
        if (!can)
        {
            fill = ARGB(255, 51, 37, 26);
            ink = ARGB(255, 160, 143, 115);
        }
        PaintShape("OZG_KDoBoxBg", fill);
        SetText("OZG_KDoText", label);
        Ink("OZG_KDoText", ink);
    }

    protected void PaintCard(OZG_WKitThing th)
    {
        SetText("OZG_KPickTitle", CardName(th.Face));
        Widget box = W("OZG_KPickCard");
        Picture(box, "OZG_KPickFace", CardPicture(th.Look, "c_" + th.Face), ARGB(255, 255, 255, 255));
        Picture(box, "OZG_KPickBack", CardPicture(th.Look, "back"), ARGB(255, 255, 255, 255));
        string style = th.Look;
        if (style == "")
            style = "classic";
        bool knows = th.Mark > 0;
        int sx;
        int sy;
        int sw;
        int sh;
        bool spot = false;
        if (knows)
            spot = OZG_MarkPlaces.SignBox(style, th.Sign, sx, sy, sw, sh);
        ShowW("OZG_KPickSign", spot);
        ShowW("OZG_KPickGlow", spot);
        ShowW("OZG_KPickTag", knows);
        if (spot)
        {
            float kx = PICK_W / OZG_MarkPlaces.CARD_W;
            float ky = PICK_H / OZG_MarkPlaces.CARD_H;
            float x = BACK_X + sx * kx;
            float y = PICK_Y + sy * ky;
            float w = sw * kx;
            float h = sh * ky;
            ImageWidget sign = ImageWidget.Cast(W("OZG_KPickSign"));
            if (sign)
            {
                sign.LoadImageFile(0, OZG_MarkPlaces.SignImage(style, th.Sign));
                sign.SetImage(0);
                sign.SetPos(x * m_K, y * m_K);
                sign.SetSize(w * m_K, h * m_K);
            }
            // the canvas rings the mark: 7 of gold round it
            float d = w;
            if (h > d)
                d = h;
            d = d + 14;
            Widget glow = W("OZG_KPickGlow");
            if (glow)
            {
                glow.SetPos((x + w * 0.5 - d * 0.5) * m_K, (y + h * 0.5 - d * 0.5) * m_K);
                glow.SetSize(d * m_K, d * m_K);
            }
        }
        if (knows)
        {
            TextWidget tag = TextWidget.Cast(W("OZG_KPickTagText"));
            if (tag)
            {
                tag.SetText(TagWord(th.Mark));
                float tw = TextW(tag) + 10;
                tag.SetSize(tw * m_K, 16 * m_K);
                Widget tagBox = W("OZG_KPickTag");
                if (tagBox)
                {
                    tagBox.SetSize(tw * m_K, 16 * m_K);
                    tagBox.SetPos((BACK_X + PICK_W - 6 - tw) * m_K, (PICK_Y + 6) * m_K);
                }
                Fit7(W("OZG_KPickTagBg"), tw, 16, 3);
            }
        }
        string note = Tr("STR_OZG_KWIN_CARD_CLEAN");
        if (th.Mark == 1)
            note = Tr("STR_OZG_KWIN_CARD_MINE");
        else if (th.Mark == 2)
            note = Tr("STR_OZG_KWIN_CARD_READ");
        float top = PICK_Y + PICK_H + 14;
        PickTextAt(top, ARGB(255, 230, 216, 188));
        int lines = Lines("OZG_KPickText", 3, note, 356);
        MoveW("OZG_KDo", 14, top + lines * 20 + 14);
        int uses = Uses();
        string label = Tr("STR_OZG_KWIN_EMPTY_KIT");
        if (th.Mark > 0)
            label = Tr("STR_OZG_KWIN_MARKED");
        else if (uses > 0)
            label = Tr("STR_OZG_KWIN_MARK");
        PaintDo(label, uses > 0);
    }

    protected void PaintDie(OZG_WKitThing th)
    {
        string title = Tr("STR_OZG_KWIN_DIE_POCKET");
        if (th.Where == "can")
            title = Tr("STR_OZG_KWIN_DIE_CAN");
        SetText("OZG_KPickTitle", title);
        string note = Tr("STR_OZG_KWIN_DIE_FAIR");
        if (th.LoadFace > 0)
        {
            note = Tr("STR_OZG_KWIN_DIE_MINE");
            if (th.Mark == 2)
                note = Tr("STR_OZG_KWIN_DIE_OTHER");
            note.Replace("%1", th.LoadFace.ToString());
            note.Replace("%2", Tr("STR_OZG_LOAD_STEP_" + th.LoadStep.ToString()));
        }
        PickTextAt(PICK_Y, ARGB(255, 230, 216, 188));
        int lines = Lines("OZG_KPickText", 3, note, 356);
        // what follows the note stands lower by its lines past the first
        float dy = (lines - 1) * 20;
        if (dy < 0)
            dy = 0;
        MoveW("OZG_KFaceLabel", 14, 89 + dy);
        MoveW("OZG_KWeightLabel", 14, 182 + dy);
        // the faces as the canvas draws them: light tiles, the picked one lighter and
        // ringed; the dice's own pips (the stalker sign on stalker dice)
        string look = "white";
        if (DieLook(th).IndexOf("stalker") == 0)
            look = "stalker";
        for (int i = 0; i < 6; i++)
        {
            string fn = "OZG_KFace" + i.ToString();
            int n = i + 1;
            bool on = n == m_Face;
            MoveW(fn, 14 + i * 60.25, 118 + dy);
            int tile = ARGB(255, 207, 197, 173);
            int tint = ARGB(255, 218, 215, 205);
            if (on)
            {
                tile = ARGB(255, 242, 234, 215);
                tint = ARGB(255, 255, 255, 255);
            }
            Picture(m_Win, fn + "Pic", "set:ozg_dice image:" + look + "_" + n.ToString(), tint);
            PaintShape(fn + "Bg", tile);
            ShowW(fn + "Ring", on);
        }
        OZG_Settings cfg = OZG_Cfg.Get();
        for (int s = 0; s < 3; s++)
        {
            string wn = "OZG_KWeight" + s.ToString();
            int step = s + 1;
            MoveW(wn, 14, 211 + s * 52 + dy);
            SetText(wn + "Name", Tr("STR_OZG_KWIN_STEP_" + step.ToString()));
            string wnote = Tr("STR_OZG_KWIN_WEIGHT_NOTE_" + step.ToString());
            int pct = 0;
            if (cfg.LoadWeights && s < cfg.LoadWeights.Count())
                pct = cfg.LoadWeights[s];
            wnote.Replace("%1", pct.ToString());
            SetText(wn + "Note", wnote);
            bool chosen = step == m_Step;
            int fill = ARGB(255, 36, 26, 18);
            int edge = ARGB(255, 74, 55, 38);
            if (chosen)
            {
                fill = ARGB(255, 58, 44, 28);
                edge = ARGB(255, 201, 164, 92);
            }
            PaintShape(wn + "BoxBg", fill);
            PaintShape(wn + "BoxEdge", edge);
        }
        MoveW("OZG_KDo", 14, 375 + dy);
        int uses = Uses();
        string label = Tr("STR_OZG_KWIN_EMPTY_KIT");
        if (uses > 0)
        {
            label = Tr("STR_OZG_KWIN_LOAD");
            label.Replace("%1", m_Face.ToString());
            label.Replace("%2", Tr("STR_OZG_LOAD_STEP_" + m_Step.ToString()));
        }
        PaintDo(label, uses > 0);
    }

    // The kit at work: its bar filled to the time gone.
    protected void PaintWork()
    {
        float part = m_Work / WORK_FULL;
        if (part < 0)
            part = 0;
        if (part > 1)
            part = 1;
        Widget fill = W("OZG_KWorkFill");
        if (fill)
            fill.SetSize(356 * part * m_K, 10 * m_K);
    }

    // ------------------------------------------------------------ clicks

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!w || button != MouseState.LEFT)
            return false;
        string n = w.GetName();
        if (n == "OZG_KBtnClose")
        {
            Close();
            return true;
        }
        if (m_Work >= 0)
            return true;
        if (n == "OZG_KHit")
        {
            PickAtPointer();
            return true;
        }
        if (n.IndexOf("OZG_KFace") == 0 && n.Length() == 10)
        {
            m_Face = n.Substring(9, 1).ToInt() + 1;
            Paint();
            return true;
        }
        if (n.IndexOf("OZG_KWeight") == 0 && n.Length() == 12)
        {
            m_Step = n.Substring(11, 1).ToInt() + 1;
            Paint();
            return true;
        }
        if (n == "OZG_KDo")
        {
            Work();
            return true;
        }
        return false;
    }

    protected void PickAtPointer()
    {
        ScrollWidget scroll = ScrollWidget.Cast(W("OZG_KScroll"));
        if (!scroll || !m_Kit)
            return;
        int mx;
        int my;
        GetMousePos(mx, my);
        float px;
        float py;
        scroll.GetScreenPos(px, py);
        float top = scroll.GetVScrollPos();
        float ux = (mx - px) / m_K;
        float uy = (my - py + top) / m_K;
        for (int i = 0; i < m_HitKeys.Count(); i++)
        {
            bool inX = ux >= m_HitX[i] && ux <= m_HitX[i] + m_HitW[i];
            bool inY = uy >= m_HitY[i] && uy <= m_HitY[i] + m_HitH[i];
            if (inX && inY)
            {
                Pick(m_HitKeys[i]);
                return;
            }
        }
    }

    // A thing picked: shown on the right; asked about once, if he knows nothing of it
    // (another's mark or load on it opens to him then).
    protected void Pick(string key)
    {
        m_Pick = key;
        m_Kit.Said = "";
        m_Kit.SaidKey = "";
        OZG_WKitThing th = Picked();
        if (!th)
            return;
        if (th.Kind == "die" && th.LoadFace > 0)
        {
            m_Face = th.LoadFace;
            if (th.LoadStep >= 1 && th.LoadStep <= 3)
                m_Step = th.LoadStep;
        }
        bool unknown = th.Mark == 0;
        if (unknown && m_Looked.Find(key) < 0)
        {
            m_Looked.Insert(key);
            OZG_WAsk ask = new OZG_WAsk();
            ask.CardKey = key;
            Ask(OZG_Const.OP_KIT_LOOK, ask);
        }
        Paint();
    }

    // The button: a card marked, a die loaded, after the kit's moment of work.
    protected void Work()
    {
        OZG_WKitThing th = Picked();
        if (!th || Uses() <= 0)
            return;
        m_WorkKey = th.Key;
        m_WorkFace = 0;
        m_WorkStep = 0;
        if (th.Kind == "card")
        {
            if (th.Mark > 0)
                return;
            m_WorkOp = OZG_Const.OP_KIT_MARK;
            string marking = Tr("STR_OZG_KWIN_WORK_MARK");
            marking.Replace("%1", OZG_Text.LowerFirst(CardName(th.Face)));
            m_WorkText = marking;
        }
        else
        {
            m_WorkOp = OZG_Const.OP_KIT_LOAD;
            m_WorkFace = m_Face;
            m_WorkStep = m_Step;
            string loading = Tr("STR_OZG_KWIN_WORK_LOAD");
            loading.Replace("%1", m_Face.ToString());
            m_WorkText = loading;
        }
        m_Kit.Said = "";
        m_Kit.SaidKey = "";
        m_Work = 0;
        OZG_Sounds.PlayLocal("click", null);
        Paint();
    }
}
