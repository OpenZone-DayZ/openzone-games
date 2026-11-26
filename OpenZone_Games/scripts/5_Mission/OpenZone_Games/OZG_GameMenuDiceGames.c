// The four dice games' own faces (boards and dice design §6, §15): the design canvas's
// boards "Покер на кістках", "Свиня", "Хто більше" and "Перудо", measured to the unit. Each
// game's parts are layouts of their own (tools/make_board_win.py: ozg_dg_<game>.layout on
// the felt, ozg_dg_<game>col.layout in the right column), made into the dice window
// (ozg_dwin.layout) when its game is played and dropped when another is. The dice are
// pictures laid on the felt's field as the window lays its other dice, and tumble with a
// throw; the swap panel of the four (OZG_DSwapS) and the log are the window's own.
//
// The games tell what their faces need in their views: zones "dpoker" (phase, round, the
// raise and its seat, the stake), "pig" (the last die, the turn's points, the goal, the
// throws), "highroll" (the dice each, the stake, every seat's chips and stake), "perudo"
// (the bid, its seat, palifico, the dice on the table) and "perudo_open" (the last
// showdown); the view's head says what this player may swap (OZG_WView.DiceSwap*).
modded class OZG_GameMenu
{
    protected static const string DG_LAYOUTS = "OpenZone_Games/gui/layouts/";
    // the window the faces were made in, the face made, its two parts; painted now
    protected Widget m_DGWin;
    protected string m_DGFace = "";
    protected Widget m_DGTray;
    protected Widget m_DGCol;
    protected bool m_DGActive;
    protected ref array<Widget> m_DGBadges;
    protected int m_DGBadgesUsed;
    // the swap button's die from the pocket and the die of the throw it replaces; the
    // pocket die picked in the panel (by its key: it stays picked as the view changes)
    protected string m_DGSwapKey = "";
    protected int m_DGSwapAt = -1;
    protected string m_DGSwapPickKey = "";
    protected int m_DGSwapTarget = -1;
    // the moves of a face's two buttons (throw / stop, bid / do not believe)
    protected string m_DGMoveA = "";
    protected string m_DGMoveB = "";
    // dice poker: the phase as the view said it, this player's turn
    protected int m_DGPhase = -1;
    protected bool m_DGMine;
    // perudo: the bid made in the panel, the bid it was set from, the dice on the table,
    // the face the palifico keeps (0: any)
    protected int m_DGCount = 1;
    protected int m_DGPick = 2;
    protected string m_DGBidSeen = "";
    protected int m_DGInPlay = 1;
    protected int m_DGLocked = 0;
    // perudo's showdown: the dice lie open DG_REVEAL seconds (the canvas: 2.6), then the
    // cans are down again; the showdown it runs for (its zone's line), the time left
    protected static const float DG_REVEAL = 2.6;
    protected string m_DGOpenSeen = "";
    protected float m_DGOpenLeft = 0;
    // dice poker's round that was decided: shown DG_REVEAL_POKER seconds as it lay (the
    // canvas waits for "Наступний раунд"; at a table nobody holds the others up), kept
    // here because the server's own copy goes with the next round's first throw
    protected static const float DG_REVEAL_POKER = 4.0;
    protected string m_DGPokerOpen = "";
    // the swap panel's pocket dice: a button a die he carries, made as they are needed
    protected ref array<Widget> m_DGPocketW;
    // how see-through the dice laid now are (a seat that folded: the canvas dims its card)
    protected int m_DGDieAlpha = 255;
    // no game dealt yet: the face is drawn from the seats (DGLobbyView)
    protected bool m_DGLobby;
    // the box sized next is drawn through (a dimmed card): fitted snug, in whole pixels
    protected bool m_DGSnug;

    // ------------------------------------------------------------ the faces

    protected int DGC(int r, int g, int b)
    {
        return ARGB(255, r, g, b);
    }

    // The face of a game: its layouts' name; "" for a game the window draws itself.
    protected string DGFaceOf(string gameId)
    {
        if (gameId == "dicepoker")
            return "poker";
        if (gameId == "pig")
            return "pig";
        if (gameId == "highroll")
            return "high";
        if (gameId == "perudo")
            return "perudo";
        return "";
    }

    // The game's faces made into the window, once a window and a game; another game's
    // dropped.
    protected bool DGEnsure(string face)
    {
        DGWindowCheck();
        if (m_DGFace == face && m_DGTray)
            return true;
        if (m_DGTray)
            m_DGTray.Unlink();
        if (m_DGCol)
            m_DGCol.Unlink();
        m_DGTray = null;
        m_DGCol = null;
        m_DGFace = face;
        Widget tray = BW("OZG_DTray");
        Widget col = BW("OZG_DCol");
        if (!tray || !col)
            return false;
        m_DGTray = MakeLayout(DG_LAYOUTS + "ozg_dg_" + face + ".layout", tray, "OZG_DG_" + face);
        m_DGCol = MakeLayout(DG_LAYOUTS + "ozg_dg_" + face + "col.layout", col, "OZG_DGC_" + face);
        return m_DGTray != null && m_DGCol != null;
    }

    // The window built anew: the faces and the badges went with the old one.
    protected void DGWindowCheck()
    {
        if (m_DGWin == m_Win)
            return;
        m_DGWin = m_Win;
        m_DGFace = "";
        m_DGTray = null;
        m_DGCol = null;
        m_DGBadges = new array<Widget>();
        m_DGBadgesUsed = 0;
        m_DGPocketW = new array<Widget>();
    }

    override protected bool DGPaint()
    {
        DGWindowCheck();
        m_DGActive = false;
        m_DGLobby = false;
        m_DGDieAlpha = 255;
        string face = "";
        if (m_V)
            face = DGFaceOf(m_V.GameId);
        // no game dealt yet, or another's last view: the face is drawn from the seats, as
        // the canvas's boards stand before their first stake
        bool lobby = false;
        if (face != "" && m_V.St != OZG_Const.ST_PLAYING)
        {
            if (!m_GPub || m_GPub.GameId != m_V.GameId)
                lobby = true;
        }
        OZG_GPublic real = m_GPub;
        if (lobby)
            m_GPub = DGLobbyView(face);
        if (!m_GPub || m_GPub.GameId != m_V.GameId)
            face = "";
        if (face == "" || !DGEnsure(face))
        {
            m_GPub = real;
            DGHideFaces();
            return false;
        }
        m_DGActive = true;
        m_DGLobby = lobby;
        DGHideGeneric();
        m_DGTray.Show(true);
        m_DGCol.Show(true);
        m_DGBadgesUsed = 0;
        m_DGMoveA = "";
        m_DGMoveB = "";
        m_DWDieX.Clear();
        m_DWDieY.Clear();
        m_DWActMoves.Clear();
        if (face == "poker")
            DGPaintPoker();
        else if (face == "pig")
            DGPaintPig();
        else if (face == "high")
            DGPaintHigh();
        else
            DGPaintPerudo();
        DGBadgesEnd();
        m_GPub = real;
        return true;
    }

    // The seats as a game's view would have them before anything is thrown: every seat
    // taken, its chips, nothing in the pot. The faces draw it as they draw a game.
    protected OZG_GPublic DGLobbyView(string face)
    {
        OZG_GPublic v = new OZG_GPublic();
        v.GameId = m_V.GameId;
        string chipsOf = "";
        string putsOf = "";
        int stake = DGOption(OZG_DiceConst.OPT_STAKE, "10").ToInt();
        int seated = 0;
        for (int i = 0; i < m_V.Seats.Count(); i++)
        {
            OZG_WSeat s = m_V.Seats[i];
            if (!s || s.St != OZG_Const.SEAT_TAKEN)
                continue;
            int seatNo = s.No;
            int chips = s.Chips;
            if (chips < 0)
                chips = 0;
            int put = stake;
            if (put > chips)
                put = chips;
            string chipsText = chips.ToString();
            string putText = put.ToString();
            OZG_GSeat g = new OZG_GSeat();
            g.SeatNo = seatNo;
            // the numbers a face reads off a seat's line: chips, rounds won and what it
            // put (dice poker); the score (pig); the dice left (perudo)
            if (face == "poker")
                g.Line = "x|" + chipsText + "|0|0";
            else if (face == "perudo")
                g.Line = "x|" + OZG_PerudoConst.DICE.ToString();
            else
                g.Line = "x|0";
            v.SeatList.Insert(g);
            if (seated > 0)
            {
                chipsOf = chipsOf + ",";
                putsOf = putsOf + ",";
            }
            chipsOf = chipsOf + chipsText;
            putsOf = putsOf + putText;
            seated++;
        }
        if (face == "high")
        {
            OZG_GZone state = new OZG_GZone();
            state.Id = "highroll";
            state.Line = DGOption(OZG_HighRollConst.OPT_DICE, "2") + "|" + stake.ToString() + "|" + chipsOf + "|" + putsOf;
            v.Zones.Insert(state);
        }
        if (face == "perudo")
        {
            int inPlay = seated * OZG_PerudoConst.DICE;
            OZG_GZone table = new OZG_GZone();
            table.Id = "perudo";
            table.Line = "0|0|-1|0|" + inPlay.ToString();
            v.Zones.Insert(table);
        }
        if (face == "poker")
        {
            OZG_GZone round = new OZG_GZone();
            round.Id = "dpoker";
            round.Line = "-1|1|0|-1|" + stake.ToString();
            v.Zones.Insert(round);
        }
        return v;
    }

    // A game the window draws itself: the faces hidden, the window's own parts back.
    protected void DGHideFaces()
    {
        if (m_DGTray)
            m_DGTray.Show(false);
        if (m_DGCol)
            m_DGCol.Show(false);
        m_DGBadgesUsed = 0;
        DGBadgesEnd();
        BWShow("OZG_DSwapS", false);
        BWShow("OZG_DActHint0", false);
        BWShow("OZG_DActHint1", false);
        BWShow("OZG_DCentre0", true);
        BWShow("OZG_DCentre1", true);
        BWShow("OZG_DRule", true);
        BWShow("OZG_DScore", true);
        BWShow("OZG_DAct", true);
        BWShow("OZG_DActTitle", true);
        BWShow("OZG_DActInfo", true);
        BWPaintNamed("OZG_DActBoxEdge", DGC(201, 164, 92));
    }

    // The window's own parts the faces stand in for.
    protected void DGHideGeneric()
    {
        int i;
        for (i = 0; i < 6; i++)
            BWShow("OZG_DSeat" + i.ToString(), false);
        for (i = 0; i < 3; i++)
            BWShow("OZG_DZone" + i.ToString(), false);
        BWShow("OZG_DCentre0", false);
        BWShow("OZG_DCentre1", false);
        BWShow("OZG_DRule", false);
        BWShow("OZG_DScore", false);
        BWShow("OZG_DAct", false);
        BWShow("OZG_DActHint0", false);
        BWShow("OZG_DActHint1", false);
    }

    // ------------------------------------------------------------ small things

    // A zone of the game's view by its id, its line split at "|".
    protected bool DGZone(string id, array<string> into)
    {
        into.Clear();
        if (!m_GPub)
            return false;
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            OZG_GZone zone = m_GPub.Zones[z];
            if (zone && zone.Id == id)
            {
                zone.Line.Split("|", into);
                return true;
            }
        }
        return false;
    }

    protected int DGIntAt(array<string> parts, int n)
    {
        if (n < 0 || n >= parts.Count())
            return 0;
        return parts[n].ToInt();
    }

    // The n-th argument of words "KEY|a|b" as a number (-1 when there is none).
    protected int DGArgInt(string words, int n)
    {
        array<string> args = new array<string>();
        DWArgs(words, args);
        if (n < 0 || n >= args.Count())
            return -1;
        return args[n].ToInt();
    }

    // The pot: the zone "pot"'s number.
    protected int DGPot()
    {
        if (!m_GPub)
            return 0;
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            OZG_GZone zone = m_GPub.Zones[z];
            if (zone && zone.Id == "pot")
            {
                int v = DGArgInt(zone.Line, 0);
                if (v < 0)
                    v = 0;
                return v;
            }
        }
        return 0;
    }

    protected OZG_GSeat DGSeat(int seatNo)
    {
        if (!m_GPub || seatNo < 0)
            return null;
        for (int s = 0; s < m_GPub.SeatList.Count(); s++)
        {
            OZG_GSeat g = m_GPub.SeatList[s];
            if (g && g.SeatNo == seatNo)
                return g;
        }
        return null;
    }

    // The seats on the cards: all but this player's (a watcher sees them all), six at most.
    protected void DGOthers(array<ref OZG_GSeat> into)
    {
        into.Clear();
        if (!m_GPub)
            return;
        for (int s = 0; s < m_GPub.SeatList.Count(); s++)
        {
            OZG_GSeat g = m_GPub.SeatList[s];
            if (g && g.SeatNo != m_V.Mine && into.Count() < 6)
                into.Insert(g);
        }
    }

    // Whose turn it is (-1: nobody's), and whether it is this player's.
    protected int DGTurn()
    {
        if (!DWPlaying() || m_GPub.Awaited.Count() == 0)
            return -1;
        return m_GPub.Awaited[0];
    }

    protected bool DGMyTurn()
    {
        int t = DGTurn();
        if (t < 0)
            return false;
        return t == m_V.Mine;
    }

    // A game's option as the host set it.
    protected string DGOption(string optId, string fallback)
    {
        if (!m_V)
            return fallback;
        int at = m_V.OptIds.Find(optId);
        if (at >= 0 && at < m_V.OptVals.Count())
            return m_V.OptVals[at];
        return fallback;
    }

    // A box of a face sized again: its ring `ringW` wide (1, or the canvas's 3) in `ring`.
    protected void DGBox(string name, float w, float h, float r, int ring, float ringW)
    {
        if (m_DGSnug)
        {
            DGBoxSnug(name, w, h, r, ring, ringW);
            return;
        }
        Widget boxW = BW(name);
        if (boxW)
            boxW.SetSize(w * m_K, h * m_K);
        Widget edge = BW(name + "Edge");
        // where a snug fit may have moved it
        if (edge)
            edge.SetPos(0, 0);
        BWFit(edge, w, h, r);
        BWPaint(edge, ring);
        Widget bg = BW(name + "Bg");
        if (!bg)
            return;
        float inner = r - ringW;
        if (inner < 0)
            inner = 0;
        bg.SetPos(ringW * m_K, ringW * m_K);
        BWFit(bg, w - 2 * ringW, h - 2 * ringW, inner);
    }

    // The same box drawn through: its ring and its ground fitted with no overlap, every
    // edge a whole pixel (BWFitPx says why), and the box itself moved to a quarter of a
    // pixel on the screen. A panel fills whole pixels from the floor of its place, a
    // picture is drawn at the place itself: past half a pixel the two part by one pixel
    // (the stand at 3840 x 1600, 2026-11-26: a card at 1392.59, 355.56 -- its bands began
    // at pixel 1392, 355, its corner pictures at 1393, 356, and the felt showed along two
    // sides of three corners). At a quarter they agree, with room for the sums' rounding.
    // The box lies at its card's corner; the card's place on the screen is its parent's as
    // it lies and the place just given to the card (asked of the card itself, the screen
    // may still know the old one).
    protected void DGBoxSnug(string name, float w, float h, float r, int ring, float ringW)
    {
        float pw = Math.Round(w * m_K);
        float ph = Math.Round(h * m_K);
        float pr = Math.Round(r * m_K);
        float pring = Math.Round(ringW * m_K);
        if (pring < 1)
            pring = 1;
        float dx = 0;
        float dy = 0;
        Widget boxW = BW(name);
        if (boxW)
        {
            boxW.SetSize(pw, ph);
            Widget card = boxW.GetParent();
            Widget root;
            if (card)
                root = card.GetParent();
            if (root)
            {
                float rx;
                float ry;
                float cardX;
                float cardY;
                root.GetScreenPos(rx, ry);
                card.GetPos(cardX, cardY);
                float sx = rx + cardX;
                float sy = ry + cardY;
                dx = Math.Round(sx - 0.25) + 0.25 - sx;
                dy = Math.Round(sy - 0.25) + 0.25 - sy;
            }
        }
        Widget edge = BW(name + "Edge");
        if (edge)
            edge.SetPos(dx, dy);
        BWFitPx(edge, pw, ph, pr);
        BWPaint(edge, ring);
        Widget bg = BW(name + "Bg");
        if (!bg)
            return;
        float inner = pr - pring;
        if (inner < 0)
            inner = 0;
        bg.SetPos(pring + dx, pring + dy);
        BWFitPx(bg, pw - 2 * pring, ph - 2 * pring, inner);
    }

    // A name put into its text and cut with an ellipsis when it is wider than `room`
    // units: a player's name is as long as he made it, and what stands after it on a card
    // does not move (the stand, 2026-11-26: "Survivor (ви)" ran into "кості господаря" on
    // a card of six). `tail` (DGYou) is kept whole: the name before it gives way.
    protected void DGNameIn(string widget, string name, string tail, float room)
    {
        TextWidget t = BWText(widget);
        if (!t)
            return;
        t.SetText(name + tail);
        if (room <= 0 || BWTextW(t) <= room)
            return;
        int n = name.LengthUtf8();
        while (n > 1)
        {
            n--;
            string cut = name.SubstringUtf8(0, n);
            cut = cut.Trim();
            t.SetText(cut + "…" + tail);
            if (BWTextW(t) <= room)
                return;
        }
    }

    // " (ви)" after the name on this player's own seat, nothing on another's.
    protected string DGYou(int seatNo)
    {
        if (seatNo == m_V.Mine)
            return " " + Tr("STR_OZG_BWIN_YOU");
        return "";
    }

    // A card dimmed as the canvas dims it (`opacity` there on the whole card): its box
    // takes `boxAlpha`, what lies on it follows the card's own `inkAlpha` (the layouts'
    // inheritalpha). The two differ because the engine blends in linear light and words
    // came out darker than a picture at the same alpha (the stand, 2026-11-26: at 0.55 a
    // name read 127 where the canvas has 159, a die 181 where it has 162).
    protected void DGDim(Widget card, string name, float boxAlpha, float inkAlpha)
    {
        if (card)
            card.SetAlpha(inkAlpha);
        Widget boxW = BW(name + "Box");
        if (boxW)
            boxW.SetAlpha(boxAlpha);
    }

    // A button of a face: its words, its fill, its ring and its ink.
    protected void DGButton(string name, string label, int fill, int ring, int ink)
    {
        BWSetText(name + "Text", label);
        BWPaintNamed(name + "BoxBg", fill);
        BWPaintNamed(name + "BoxEdge", ring);
        BWInk(name + "Text", ink);
    }

    // Words wrapped into two lines of a face.
    protected void DGLines(string first, string second, string text, float w)
    {
        array<string> lines = new array<string>();
        BWWrap(BWText(first), text, w, 2, lines);
        string a = "";
        string b = "";
        if (lines.Count() > 0)
            a = lines[0];
        if (lines.Count() > 1)
            b = lines[1];
        BWSetText(first, a);
        BWSetText(second, b);
    }

    // A text set and its width in units.
    protected float DGText(string name, string text)
    {
        TextWidget t = BWText(name);
        if (!t)
            return 0;
        t.SetText(text);
        return BWTextW(t);
    }

    // Units that land on a whole pixel: a part centred by the width of its words would
    // otherwise sit on half a pixel, and a picture there takes its opposite edge into its
    // first column (seen on pig's pill, 2026-11-26).
    protected float DGSnap(float units)
    {
        if (m_K <= 0)
            return units;
        return Math.Round(units * m_K) / m_K;
    }

    // A pip of the rounds won: a gold disc, or its ring.
    protected void DGPip(string name, bool won, float x, float y)
    {
        ImageWidget pip = ImageWidget.Cast(BW(name));
        if (!pip)
            return;
        if (won)
            pip.LoadImageFile(0, BW_TEX + "ozg_disc_ca.paa");
        else
            pip.LoadImageFile(0, BW_TEX + "ozg_ring6_ca.paa");
        pip.SetImage(0);
        pip.SetPos(x * m_K, y * m_K);
        pip.Show(true);
    }

    // ------------------------------------------------------------ dice on the felt

    // A die laid on the felt (units of the tray): its shadow `drop` lower, turned `turn`; it
    // tumbles with a throw. Face 0 is the die with no pips; `red`: a one with a red pip
    // (pig's, on white dice).
    protected void DGDie(int face, string look, float x, float y, float size, float turn, float drop, int prio, bool red)
    {
        if (look == "")
            look = DWLook();
        string pic = look + "_" + face.ToString();
        bool white = look == "white" || look == "stalker";
        if (red && face == 1 && white)
            pic = look + "_1r";
        int tint = ARGB(m_DGDieAlpha, 255, 255, 255);
        BWTurnPic(BW_FIELD, "set:ozg_dice image:die_s", x, y + drop, size, prio, tint, turn);
        BWTurnPic(BW_FIELD, "set:ozg_dice image:" + pic, x, y, size, prio + 1, tint, turn);
    }

    // A ring round a die (picked, counted, a one thrown): the die's shape `gap` wider each
    // side, in `colour`, under it.
    protected void DGRing(float x, float y, float size, float gap, float turn, int colour, int prio)
    {
        float s = size + 2 * gap;
        BWTurnPic(BW_FIELD, "set:ozg_dice image:white_0", x - gap, y - gap, s, prio, colour, turn);
    }

    // The face a die of this player's is loaded on, over its corner (the canvas: a grey
    // disc in a dark ring).
    protected void DGBadge(int face, float x, float y)
    {
        if (!m_DGBadges)
            m_DGBadges = new array<Widget>();
        Widget b;
        if (m_DGBadgesUsed < m_DGBadges.Count())
        {
            b = m_DGBadges[m_DGBadgesUsed];
        }
        else
        {
            Widget parent = m_BWPoolParent[BW_FIELD];
            if (!parent)
                return;
            b = MakeLayout(DG_LAYOUTS + "ozg_dbadge.layout", parent, "OZG_DBadge");
            if (!b)
                return;
            m_DGBadges.Insert(b);
        }
        m_DGBadgesUsed++;
        TextWidget t = TextWidget.Cast(b.FindAnyWidget("OZG_DBadgeText"));
        if (t)
            t.SetText(face.ToString());
        b.SetSort(60);
        b.SetPos(x * m_K, y * m_K);
        b.Show(true);
    }

    protected void DGBadgesEnd()
    {
        if (!m_DGBadges)
            return;
        for (int i = m_DGBadgesUsed; i < m_DGBadges.Count(); i++)
        {
            if (m_DGBadges[i])
                m_DGBadges[i].Show(false);
        }
    }

    // The look of die `k` a seat throws (the view's SeatDiceLooks); "" for the set's.
    protected string DGLookOf(int seatNo, int k)
    {
        array<string> looks = new array<string>();
        DGSeatLooks(seatNo, looks);
        if (k >= 0 && k < looks.Count())
            return looks[k];
        return "";
    }

    // The look die `k` of a seat had when the seat threw it last (the view's
    // ThrownDiceLooks); the look of the die it throws now when that is not known.
    protected string DGThrownLookOf(int seatNo, int k)
    {
        if (m_V && seatNo >= 0 && k >= 0)
        {
            string head = seatNo.ToString() + "=";
            int headLen = head.Length();
            for (int i = 0; i < m_V.ThrownDiceLooks.Count(); i++)
            {
                string line = m_V.ThrownDiceLooks[i];
                if (line.IndexOf(head) != 0)
                    continue;
                array<string> looks = new array<string>();
                string rest = line.Substring(headLen, line.Length() - headLen);
                rest.Split(",", looks);
                if (k < looks.Count() && looks[k] != "")
                    return looks[k];
                break;
            }
        }
        return DGLookOf(seatNo, k);
    }

    // The load this player knows on die `i` of his throw (0: none).
    protected int DGMyLoad(int i)
    {
        if (!m_V || i < 0 || i >= m_V.MyDiceLoads.Count())
            return 0;
        return m_V.MyDiceLoads[i];
    }

    // The pot as the canvas draws it: the pill's chip, "БАНК" and the number, centred.
    protected void DGPill(string name, int pot, float x, float y)
    {
        Widget pill = BW(name);
        if (!pill)
            return;
        pill.Show(true);
        pill.SetPos(x * m_K, y * m_K);
        float lw = DGText(name + "Label", OZG_Text.Upper(Tr("STR_OZG_DICE_POT")));
        float vw = DGText(name + "Value", pot.ToString());
        float total = 30 + 12 + lw + 12 + vw;
        float x0 = DGSnap(132 - total / 2);
        BWMove(BW(name + "Icon"), x0, 20);
        BWMove(BW(name + "Label"), x0 + 42, 27);
        BWMove(BW(name + "Value"), x0 + 42 + lw + 12, 15);
    }

    // ------------------------------------------------------------ the right column

    // The log from `top` to `bottom` (units of the column).
    protected void DGLog(float top, float bottom)
    {
        Widget logW = BW("OZG_DLog");
        if (logW)
        {
            logW.SetPos(12 * m_K, top * m_K);
            logW.SetSize(360 * m_K, (bottom - top) * m_K);
        }
        BWFit(BW("OZG_DLogBg"), 360, bottom - top, 6);
    }

    // Between games the window's own action panel (the table's buttons) at `top`, the
    // face's own hidden by the caller; its bottom.
    protected float DGTableActs(float top)
    {
        BWShow("OZG_DAct", true);
        BWShow("OZG_DActTitle", true);
        BWShow("OZG_DActInfo", true);
        BWPaintNamed("OZG_DActBoxEdge", DGC(201, 164, 92));
        m_DWScoreH = top - 22;
        DWPaintActions();
        return m_DWActBottom;
    }

    // The swap panel of the four at `top` (units of the column): the swaps left; the dice
    // he carries and the dice he is about to throw, one of each picked by him and by nobody
    // else (the owner's word: the game does not decide for the person); what the picked die
    // is; the button, live once both are picked and a swap may be done. He may carry any
    // number of dice: seven a row in the engine's scroll, two rows in sight, the wheel and
    // the gold bar for the rest; what lies under them is placed by their height. Its
    // bottom, or `top` when it is not shown.
    protected float DGPaintSwap(float top, string face)
    {
        Widget panel = BW("OZG_DSwapS");
        m_DGSwapKey = "";
        m_DGSwapAt = -1;
        // only for a player with dice to throw: one out of perudo has none to swap
        int mineSeat = m_V.Mine;
        array<string> mineLooks = new array<string>();
        DGSeatLooks(mineSeat, mineLooks);
        bool throws = m_V.MyDiceLoads.Count() > 0 || mineLooks.Count() > 0;
        bool shown = DWPlaying() && m_V.DiceSwapsMost > 0 && throws;
        if (panel)
            panel.Show(shown);
        if (!panel || !shown)
            return top;
        panel.SetPos(12 * m_K, top * m_K);
        string leftText = m_V.DiceSwapsLeft.ToString();
        string mostText = m_V.DiceSwapsMost.ToString();
        BWSetText("OZG_DSwapSInfo", Tr2("STR_OZG_DWIN_SWAP_LEFT", leftText, mostText));
        BWSetText("OZG_DSwapSFrom", OZG_Text.Upper(Tr("STR_OZG_DWIN_SWAP_FROM")));
        BWSetText("OZG_DSwapSFor", OZG_Text.Upper(Tr("STR_OZG_DWIN_SWAP_FOR")));
        // the pocket die he picked, by its key; none until he picks one
        int i;
        int pick = -1;
        int pockets = m_V.PocketDice.Count();
        for (i = 0; i < pockets; i++)
        {
            if (m_V.PocketDice[i] == m_DGSwapPickKey)
                pick = i;
        }
        if (pick < 0)
            m_DGSwapPickKey = "";
        // a button a die carried, made as they are needed, named by its place
        Widget inside = BW("OZG_DSPocketsIn");
        if (!m_DGPocketW)
            m_DGPocketW = new array<Widget>();
        while (inside && m_DGPocketW.Count() < pockets)
        {
            Widget made = MakeLayout(DG_LAYOUTS + "ozg_dspocket.layout", inside, "OZG_DSPocket");
            if (!made)
                break;
            int madeNo = m_DGPocketW.Count();
            made.SetName("OZG_DSPocket" + madeNo.ToString());
            m_DGPocketW.Insert(made);
        }
        for (i = 0; i < m_DGPocketW.Count(); i++)
        {
            Widget pocket = m_DGPocketW[i];
            if (!pocket)
                continue;
            bool there = i < pockets;
            pocket.Show(there);
            if (!there)
                continue;
            int col = i % 7;
            int row = i / 7;
            float px = 4 + col * 46;
            float py = 8 + row * 46;
            pocket.SetPos(px * m_K, py * m_K);
            string look = "white";
            if (i < m_V.PocketLooks.Count() && m_V.PocketLooks[i] != "")
                look = m_V.PocketLooks[i];
            int loaded = 0;
            if (i < m_V.PocketLoads.Count())
                loaded = m_V.PocketLoads[i];
            string pocketKey = m_V.PocketDice[i];
            int up = DGFaceOfKey(pocketKey);
            if (loaded > 0)
                up = loaded;
            ImageWidget pic = ImageWidget.Cast(pocket.FindAnyWidget("OZG_DSPocketPic"));
            if (pic)
            {
                pic.LoadImageFile(0, "set:ozg_dice image:" + look + "_" + up.ToString());
                pic.SetImage(0);
            }
            Widget pocketRing = pocket.FindAnyWidget("OZG_DSPocketRing");
            if (pocketRing)
                pocketRing.Show(i == pick);
            Widget pocketBadge = pocket.FindAnyWidget("OZG_DSPocketBadge");
            if (pocketBadge)
                pocketBadge.Show(loaded > 0);
            TextWidget pocketFace = TextWidget.Cast(pocket.FindAnyWidget("OZG_DSPocketBadgeText"));
            if (pocketFace && loaded > 0)
                pocketFace.SetText(loaded.ToString());
        }
        // the rows: two in sight, the scroll for more (its positions are pixels)
        int rows = (pockets + 6) / 7;
        if (rows < 1)
            rows = 1;
        int rowsSeen = rows;
        if (rowsSeen > 2)
            rowsSeen = 2;
        // the rows are 46 apart and the scroll runs whole rows: at its end the row above
        // ends exactly at its edge
        float scrollH = 3 + rowsSeen * 46;
        float insideH = 3 + rows * 46;
        ScrollWidget scroll = ScrollWidget.Cast(BW("OZG_DSPockets"));
        if (scroll)
            scroll.SetSize(344 * m_K, scrollH * m_K);
        if (inside)
        {
            inside.SetSize(334 * m_K, insideH * m_K);
            inside.Update();
        }
        if (scroll)
        {
            scroll.Update();
            float most = (insideH - scrollH) * m_K;
            if (most < 0)
                most = 0;
            if (scroll.GetVScrollPos() > most)
                scroll.VScrollToPos(most);
        }
        float y = 46 + scrollH + 5;
        BWMove(BW("OZG_DSwapSFor"), 12, y);
        y = y + 18;
        // the dice he is about to throw: each in its look, its place on it
        int mineNo = m_V.Mine;
        array<string> looks = new array<string>();
        DGSeatLooks(mineNo, looks);
        int targets = m_V.MyDiceLoads.Count();
        if (targets == 0)
            targets = looks.Count();
        if (targets > 5)
            targets = 5;
        if (m_DGSwapTarget >= targets)
            m_DGSwapTarget = -1;
        for (i = 0; i < 5; i++)
        {
            string tn = "OZG_DSTarget" + i.ToString();
            bool thrown = i < targets;
            BWShow(tn, thrown);
            if (!thrown)
                continue;
            BWMove(BW(tn), 12 + i * 46, y);
            string tlook = DWLook();
            if (i < looks.Count() && looks[i] != "")
                tlook = looks[i];
            ImageWidget tpic = ImageWidget.Cast(BW(tn + "Pic"));
            if (tpic)
            {
                tpic.LoadImageFile(0, "set:ozg_dice image:" + tlook + "_0");
                tpic.SetImage(0);
            }
            int place = i + 1;
            BWSetText(tn + "Num", place.ToString());
            int numInk = DGC(29, 23, 18);
            bool dark = tlook == "black" || tlook == "red" || tlook == "stalker_black" || tlook == "stalker_red";
            if (dark)
                numInk = DGC(246, 239, 226);
            BWInk(tn + "Num", numInk);
            BWShow(tn + "Ring", i == m_DGSwapTarget);
            int tload = DGMyLoad(i);
            BWShow(tn + "Badge", tload > 0);
            if (tload > 0)
                BWSetText(tn + "BadgeText", tload.ToString());
        }
        y = y + 38 + 8;
        int load = 0;
        int step = 0;
        if (pick >= 0 && pick < m_V.PocketLoads.Count() && pick < m_V.PocketSteps.Count())
        {
            load = m_V.PocketLoads[pick];
            step = m_V.PocketSteps[pick];
        }
        // one of his own loaded dice in play
        int playing = 0;
        int playStep = 0;
        for (i = 0; i < m_V.MyDiceLoads.Count(); i++)
        {
            if (m_V.MyDiceLoads[i] > 0)
            {
                playing = m_V.MyDiceLoads[i];
                if (i < m_V.MyDiceSteps.Count())
                    playStep = m_V.MyDiceSteps[i];
                break;
            }
        }
        string when = Tr("STR_OZG_DWIN_SWAP_WHEN_THROW");
        string notNow = Tr("STR_OZG_DWIN_SWAP_NOT_THROW");
        if (face == "pig")
        {
            when = Tr("STR_OZG_DWIN_SWAP_WHEN_PIG");
            notNow = Tr("STR_OZG_DWIN_SWAP_NOT_PIG");
        }
        else if (face == "perudo")
        {
            when = Tr("STR_OZG_DWIN_SWAP_WHEN_PERUDO");
            notNow = Tr("STR_OZG_DWIN_SWAP_NOT_PERUDO");
        }
        // the words made into locals before a call takes them (perudo's PeShow says why)
        string playFace = playing.ToString();
        string playWord = Tr(OZG_Const.LOAD_STEP + playStep.ToString());
        string loadFace = load.ToString();
        string loadWord = Tr(OZG_Const.LOAD_STEP + step.ToString());
        string afterWords = when;
        if (m_DGSwapTarget < 0)
            afterWords = Tr("STR_OZG_DWIN_SWAP_CHOOSE_FOR");
        string text;
        if (pick >= 0 && load > 0)
            text = Tr2("STR_OZG_DWIN_SWAP_PICK_LOADED", loadFace, loadWord) + " " + afterWords;
        else if (pick >= 0)
            text = Tr("STR_OZG_DWIN_SWAP_PICK_PLAIN") + " " + afterWords;
        else if (playing > 0)
            text = Tr2("STR_OZG_DWIN_SWAP_IN_PLAY", playFace, playWord);
        else if (pockets == 0)
            text = Tr("STR_OZG_DWIN_SWAP_NONE");
        else
            text = Tr("STR_OZG_DWIN_SWAP_CHOOSE") + " " + when;
        // the words: two lines as the canvas has them, a third when they need it
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_DSwapSText0"), text, 336, 3, lines);
        int lineRoom = lines.Count();
        if (lineRoom < 2)
            lineRoom = 2;
        for (i = 0; i < 3; i++)
        {
            string ln = "OZG_DSwapSText" + i.ToString();
            string lineWords = "";
            if (i < lines.Count())
                lineWords = lines[i];
            BWSetText(ln, lineWords);
            BWMove(BW(ln), 12, y + i * 17);
            BWShow(ln, i < lineRoom);
        }
        y = y + lineRoom * 17 + 6;
        // the button: live once both are picked and a swap may be done now
        bool both = pick >= 0 && m_DGSwapTarget >= 0;
        bool can = m_V.DiceSwapNow == 1 && both;
        string label;
        int targetNo = m_DGSwapTarget + 1;
        string targetText = targetNo.ToString();
        if (m_V.DiceSwapsLeft <= 0)
            label = Tr("STR_OZG_DWIN_SWAP_NO_MORE");
        else if (pockets == 0)
            label = Tr("STR_OZG_DWIN_SWAP_NOTHING");
        else if (m_V.DiceSwapNow != 1)
            label = notNow;
        else if (!both)
            label = Tr("STR_OZG_DWIN_SWAP_PICK_BOTH");
        else if (face == "perudo")
            label = Tr1("STR_OZG_DWIN_SWAP_DO_UNDER", targetText);
        else
            label = Tr1("STR_OZG_DWIN_SWAP_DO", targetText);
        if (can)
        {
            m_DGSwapKey = m_V.PocketDice[pick];
            m_DGSwapAt = m_DGSwapTarget;
        }
        int fill = DGC(36, 26, 18);
        if (can)
            fill = DGC(58, 44, 28);
        DGButton("OZG_DSwapSBtn", label, fill, DGC(107, 82, 51), DGC(239, 228, 204));
        BWMove(BW("OZG_DSwapSBtn"), 12, y);
        y = y + 36 + 10;
        panel.SetSize(360 * m_K, y * m_K);
        DGBox("OZG_DSwapSBox", 360, y, 6, DGC(107, 82, 51), 1);
        return top + y;
    }

    // The looks of the dice a seat throws (the view's SeatDiceLooks), in their order.
    protected void DGSeatLooks(int seatNo, array<string> into)
    {
        into.Clear();
        if (!m_V || seatNo < 0)
            return;
        string head = seatNo.ToString() + "=";
        for (int i = 0; i < m_V.SeatDiceLooks.Count(); i++)
        {
            string line = m_V.SeatDiceLooks[i];
            if (line.IndexOf(head) != 0)
                continue;
            line.Substring(head.Length(), line.Length() - head.Length()).Split(",", into);
            return;
        }
    }

    // A face for a die whose load he does not know: the same each time it is drawn (the
    // kit's window draws it so).
    protected int DGFaceOfKey(string key)
    {
        int sum = 0;
        int n = key.Length();
        for (int i = 0; i < n; i++)
        {
            string ch = key.Substring(i, 1);
            sum = sum + ch.ToAscii();
        }
        return 1 + sum % 6;
    }

    // ------------------------------------------------------------ dice poker

    // Where dice poker's card `i` of `n` lies and how wide (the canvas: two of 360 over 984
    // from 110; three of 310; four of 234; five and six in two rows of three).
    protected void DGPokerSpot(int i, int n, out float x, out float y, out float w)
    {
        y = 54;
        w = 360;
        x = 58;
        if (n == 1)
        {
            x = 370;
            return;
        }
        if (n == 2)
        {
            x = 58 + i * 624;
            return;
        }
        if (n == 3)
        {
            w = 310;
            x = 58 + i * 337;
            return;
        }
        if (n == 4)
        {
            w = 234;
            x = 58 + i * 250;
            return;
        }
        w = 310;
        int col = i % 3;
        int row = i / 3;
        x = 58 + col * 337;
        y = 54 + row * 136;
    }

    protected string DGHandName(array<int> dice)
    {
        if (dice.Count() == 0)
            return "";
        int kind = OZG_DicePoker.DpRank(dice) / 10000;
        return OZG_Text.UpperFirst(Tr("STR_OZG_DPOKER_HAND_" + kind.ToString()));
    }

    protected void DGPaintPoker()
    {
        array<string> st = new array<string>();
        DGZone("dpoker", st);
        int phase = DGIntAt(st, 0);
        int round = DGIntAt(st, 1);
        int raise = DGIntAt(st, 2);
        int raiser = -1;
        if (st.Count() > 3)
            raiser = st[3].ToInt();
        int stake = DGIntAt(st, 4);
        bool playing = DWPlaying();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        m_DGPhase = phase;
        m_DGMine = mine;
        if (!playing)
            m_DWPicked.Clear();
        // the round before as it lay when it was decided (the canvas's step "розкриття"):
        // a new one is shown DG_REVEAL_POKER seconds, then the round that is on
        string openLine = DGZoneLine("dpoker_open");
        if (!playing)
        {
            m_DGOpenSeen = "";
            m_DGOpenLeft = 0;
            m_DGPokerOpen = "";
        }
        else if (openLine != "" && openLine != m_DGOpenSeen)
        {
            m_DGOpenSeen = openLine;
            m_DGPokerOpen = openLine;
            m_DGOpenLeft = DG_REVEAL_POKER;
        }
        bool reveal = playing && m_DGOpenLeft > 0 && m_DGPokerOpen != "";
        array<string> open = new array<string>();
        if (reveal)
            m_DGPokerOpen.Split("|", open);
        int openRound = DGIntAt(open, 0);
        array<string> openFolded = new array<string>();
        if (reveal)
        {
            m_DGPhase = -1;
            m_DWPicked.Clear();
            if (open.Count() > 2)
                open[2].Split(",", openFolded);
        }
        array<ref OZG_GSeat> others = new array<ref OZG_GSeat>();
        DGOthers(others);
        int n = others.Count();
        int i;
        int k;
        for (i = 0; i < 6; i++)
        {
            string c = "OZG_DPOpp" + i.ToString();
            Widget card = BW(c);
            if (!card)
                continue;
            bool shown = i < n;
            card.Show(shown);
            if (!shown)
                continue;
            OZG_GSeat g = others[i];
            int gSeat = g.SeatNo;
            string gLine = g.Line;
            string gRole = g.Role;
            int gOut = g.Out;
            int gMark = g.Mark;
            float cx;
            float cy;
            float cw;
            DGPokerSpot(i, n, cx, cy, cw);
            card.SetPos(cx * m_K, cy * m_K);
            card.SetSize(cw * m_K, 124 * m_K);
            array<int> dice = new array<int>();
            DWDiceOf(gRole, dice);
            bool folded = gOut == 1;
            if (reveal)
            {
                // as the round lay: its dice, and whether it had folded
                DGOpenDice(open, 3, gSeat, dice);
                string gSeatText = gSeat.ToString();
                folded = openFolded.Find(gSeatText) >= 0;
            }
            int ring = DGC(107, 82, 51);
            if (folded)
            {
                ring = DGC(61, 45, 29);
                DGDim(card, c, 0.59, 0.66);
            }
            else
            {
                DGDim(card, c, 1, 1);
            }
            m_DGSnug = folded;
            DGBox(c + "Box", cw, 124, 8, ring, 1);
            m_DGSnug = false;
            int chips = DGArgInt(gLine, 0);
            int wins = DGArgInt(gLine, 1);
            int put = DGArgInt(gLine, 2);
            Widget chipsW = BW(c + "Chips");
            float chipsW0 = DGText(c + "Chips", Tr1("STR_OZG_DICE_POT_LINE", chips.ToString()));
            if (chipsW)
            {
                chipsW.SetPos((cw - 12 - 198) * m_K, 13.5 * m_K);
            }
            float chipsLeft = cw - 12 - chipsW0;
            // the name up to the rounds' marks
            string pokerName = SeatName(gSeat);
            DGNameIn(c + "Name", pokerName, "", chipsLeft - 40 - 8 - 12);
            DGPip(c + "Win0", wins >= 1, chipsLeft - 40, 15);
            DGPip(c + "Win1", wins >= 2, chipsLeft - 23, 15);
            float ds = 42;
            float dstep = 50;
            if (cw < 300)
            {
                ds = 36;
                dstep = 42;
            }
            if (folded)
                m_DGDieAlpha = 109;
            // a die's face and its look into locals before the call takes them: an element
            // read in an argument list is lost to a call made after it in the same list
            // (dice poker's view of 2026-11-26: a seat came out as its last die)
            for (k = 0; k < dice.Count() && k < 5; k++)
            {
                int oppFace = dice[k];
                string oppLook = DGThrownLookOf(gSeat, k);
                DGDie(oppFace, oppLook, cx + 12 + k * dstep, cy + 42, ds, 0, 3, 31, false);
            }
            m_DGDieAlpha = 255;
            string hand = Tr("STR_OZG_DWIN_NOT_THROWN");
            if (dice.Count() > 0)
                hand = DGHandName(dice);
            BWSetText(c + "Hand", hand);
            string state = "";
            if (folded)
                state = Tr("STR_OZG_DWIN_DP_FOLDED");
            else if (put > 0)
                state = Tr1("STR_OZG_DWIN_DP_PUT", put.ToString());
            Widget stateW = BW(c + "State");
            if (stateW)
                stateW.SetPos((cw - 12 - 198) * m_K, 96 * m_K);
            BWSetText(c + "State", state);
        }
        // the pot under the cards
        float bankY = 286;
        if (n >= 5)
            bankY = 334;
        DGPill("OZG_DPBank", DGPot(), 418, bankY);

        // this player's dice, his hand, his rounds and chips
        int mineNo = m_V.Mine;
        OZG_GSeat me = DGSeat(mineNo);
        string meRole = "";
        string meLine = "";
        if (me)
        {
            meRole = me.Role;
            meLine = me.Line;
        }
        array<int> myDice = new array<int>();
        if (me)
            DWDiceOf(meRole, myDice);
        if (me && reveal)
            DGOpenDice(open, 3, mineNo, myDice);
        for (k = m_DWPicked.Count() - 1; k >= 0; k--)
        {
            if (m_DWPicked[k] >= myDice.Count())
                m_DWPicked.Remove(k);
        }
        array<float> rots = new array<float>();
        rots.Insert(-6);
        rots.Insert(4);
        rots.Insert(-2);
        rots.Insert(7);
        rots.Insert(-4);
        for (k = 0; k < myDice.Count() && k < 5; k++)
        {
            float dx = 296 + k * 106;
            float dy = 438;
            bool picked = m_DWPicked.Find(k) >= 0;
            float drop = 7;
            if (picked)
            {
                dy = dy - 18;
                drop = 16;
                float ringDeg = rots[k];
                int ringGold = DGC(231, 194, 122);
                DGRing(dx, dy, 84, 4, ringDeg, ringGold, 30);
            }
            int myFace = myDice[k];
            float myTurnDeg = rots[k];
            string myLook = DGThrownLookOf(mineNo, k);
            DGDie(myFace, myLook, dx, dy, 84, myTurnDeg, drop, 31, false);
            m_DWDieX.Insert(dx + 42);
            m_DWDieY.Insert(dy + 42);
            int load = DGMyLoad(k);
            if (load > 0)
                DGBadge(load, dx + 68, dy - 10);
        }
        string hint = "";
        if (myDice.Count() == 0 && playing)
        {
            if (mine && phase == 0)
                hint = Tr("STR_OZG_DWIN_DP_YOUR_THROW");
            else if (turn >= 0)
                hint = Tr1("STR_OZG_DWIN_DP_THROWS", SeatName(turn));
        }
        if (m_DGLobby)
            hint = Tr("STR_OZG_DWIN_DP_FIRST_STAKES");
        if (reveal)
            hint = "";
        BWSetText("OZG_DPHint", hint);
        bool seated = me != null;
        BWShow("OZG_DPYou", seated);
        BWShow("OZG_DPMyWin0", seated);
        BWShow("OZG_DPMyWin1", seated);
        BWShow("OZG_DPMyChips", seated);
        if (seated)
        {
            float lw = DGText("OZG_DPYouLabel", OZG_Text.Upper(Tr("STR_OZG_DWIN_DP_YOU")));
            string myHand = "—";
            if (myDice.Count() > 0)
                myHand = DGHandName(myDice);
            float hw = DGText("OZG_DPYouHand", myHand);
            float bw = 18 + lw + 10 + hw + 18;
            int myChips = DGArgInt(meLine, 0);
            int myWins = DGArgInt(meLine, 1);
            float mcw = DGText("OZG_DPMyChips", Tr1("STR_OZG_DICE_POT_LINE", myChips.ToString()));
            float total = bw + 14 + 33 + 14 + mcw;
            float x0 = DGSnap(550 - total / 2);
            BWMove(BW("OZG_DPYou"), x0, 564);
            int youRing = DGC(107, 82, 51);
            if (myDice.Count() > 0)
                youRing = DGC(201, 164, 92);
            DGBox("OZG_DPYouBox", bw, 48, 8, youRing, 1);
            Widget youW = BW("OZG_DPYou");
            if (youW)
                youW.SetSize(bw * m_K, 48 * m_K);
            BWMove(BW("OZG_DPYouHand"), 18 + lw + 10, 9);
            DGPip("OZG_DPMyWin0", myWins >= 1, x0 + bw + 14, 581);
            DGPip("OZG_DPMyWin1", myWins >= 2, x0 + bw + 14 + 19, 581);
            BWMove(BW("OZG_DPMyChips"), x0 + bw + 14 + 33 + 14, 579.5);
        }

        // the right column: the round, what he may do, the swap, the log, the hands
        int shownPhase = phase + 1;
        if (m_GPub.Ended == 1)
            shownPhase = 4;
        int shownRound = round;
        if (reveal)
        {
            shownPhase = 4;
            shownRound = openRound;
        }
        string phaseWord = Tr("STR_OZG_DWIN_DP_PHASE_" + shownPhase.ToString());
        BWSetText("OZG_DPRoundTitle", OZG_Text.Upper(Tr2("STR_OZG_DWIN_DP_ROUND", shownRound.ToString(), phaseWord)));
        for (i = 0; i < 5; i++)
        {
            int seg = DGC(58, 44, 28);
            if (i <= shownPhase)
                seg = DGC(201, 164, 92);
            BWPaintNamed("OZG_DPSeg" + i.ToString(), seg);
        }
        string rhint = "";
        if (m_DGLobby)
            rhint = Tr("STR_OZG_DWIN_DP_HINT_STAKES");
        if (playing)
        {
            if (phase == 0)
            {
                rhint = Tr1("STR_OZG_DWIN_DP_HINT_THROWS", SeatName(turn));
                if (mine)
                    rhint = Tr("STR_OZG_DWIN_DP_HINT_THROW");
            }
            else if (phase == 1)
            {
                rhint = Tr1("STR_OZG_DWIN_DP_HINT_BETS", SeatName(turn));
                if (mine && raise > 0)
                    rhint = Tr2("STR_OZG_DWIN_DP_HINT_CALL", SeatName(raiser), raise.ToString());
                else if (mine)
                    rhint = Tr("STR_OZG_DWIN_DP_HINT_RAISE");
            }
            else
            {
                rhint = Tr1("STR_OZG_DWIN_DP_HINT_AGAINS", SeatName(turn));
                int pickedN = m_DWPicked.Count();
                if (mine && pickedN > 0)
                    rhint = Tr1("STR_OZG_DWIN_DP_HINT_PICKED", pickedN.ToString());
                else if (mine)
                    rhint = Tr("STR_OZG_DWIN_DP_HINT_PICK");
            }
        }
        if (reveal)
            rhint = Tr("STR_OZG_DWIN_DP_HINT_SHOWN");
        DGLines("OZG_DPRHint0", "OZG_DPRHint1", rhint, 336);
        float bottom = 116;
        if (playing)
        {
            BWShow("OZG_DAct", true);
            BWShow("OZG_DActTitle", false);
            BWShow("OZG_DActInfo", false);
            BWShow("OZG_DAmount", false);
            BWMove(BW("OZG_DAct"), 12, 116);
            BWPaintNamed("OZG_DActBoxEdge", DGC(201, 164, 92));
            int used = 0;
            int haveChips = 0;
            if (me)
                haveChips = DGArgInt(meLine, 0);
            if (reveal)
            {
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_NEXT"), "", 2, 12);
                used++;
            }
            else if (!mine)
            {
                DGActBtn(used, Tr1("STR_OZG_DWIN_TURN_OF", SeatName(turn)), "", 2, 12);
                used++;
            }
            else if (phase == 0)
            {
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_THROW"), OZG_DiceGameConst.M_THROW, 0, 12);
                used++;
            }
            else if (phase == 1 && raise > 0)
            {
                DGActBtn(used, Tr1("STR_OZG_DWIN_DP_CALL", raise.ToString()), OZG_DPokerConst.M_CALL, 0, 12);
                used++;
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_FOLD"), OZG_DPokerConst.M_FOLD, 1, 62);
                used++;
            }
            else if (phase == 1)
            {
                int up = stake;
                if (up > haveChips)
                    up = haveChips;
                float y1 = 12;
                if (up > 0)
                {
                    DGActBtn(used, Tr1("STR_OZG_DWIN_DP_RAISE", up.ToString()), OZG_DPokerConst.M_RAISE + up.ToString(), 0, y1);
                    used++;
                    y1 = 62;
                }
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_NO_RAISE"), OZG_DPokerConst.M_CHECK, 1, y1);
                used++;
            }
            else
            {
                string again = Tr("STR_OZG_DWIN_DP_KEEP");
                int againN = m_DWPicked.Count();
                if (againN > 0)
                    again = Tr1("STR_OZG_DWIN_DP_AGAIN", againN.ToString());
                DGActBtn(used, again, "#again", 0, 12);
                used++;
            }
            for (k = used; k < 16; k++)
                BWShow("OZG_DAct" + k.ToString(), false);
            float h = 12 + used * 42 + (used - 1) * 8 + 12;
            Widget boxW = BW("OZG_DActBox");
            if (boxW)
                boxW.SetSize(360 * m_K, h * m_K);
            BWFit(BW("OZG_DActBoxEdge"), 360, h, 6);
            BWFit(BW("OZG_DActBoxBg"), 358, h - 2, 5);
            bottom = 116 + h;
        }
        else
        {
            bottom = DGTableActs(116);
        }
        float swapBottom = DGPaintSwap(bottom + 10, "poker");
        BWSetText("OZG_DPHandsTitle", OZG_Text.Upper(Tr("STR_OZG_DWIN_DP_HANDS_TITLE")));
        DGLines("OZG_DPHandsText0", "OZG_DPHandsText1", Tr("STR_OZG_DWIN_DP_HANDS"), 336);
        DGLog(swapBottom + 10, 683);
    }

    // A button of the action panel (dice poker's): style 0 gold, 1 plain, 2 not now.
    protected void DGActBtn(int k, string label, string move, int style, float y)
    {
        string name = "OZG_DAct" + k.ToString();
        Widget b = BW(name);
        if (!b)
            return;
        DWButtonAt(b, name, 12, y, 336, 42);
        int fill = DGC(201, 164, 92);
        int ring = fill;
        int ink = DGC(26, 18, 10);
        if (style == 1)
        {
            fill = DGC(45, 33, 23);
            ring = DGC(107, 82, 51);
            ink = DGC(239, 228, 204);
        }
        else if (style == 2)
        {
            fill = DGC(51, 37, 26);
            ring = fill;
            ink = DGC(160, 143, 115);
        }
        DGButton(name, label, fill, ring, ink);
        b.Show(true);
        while (m_DWActMoves.Count() <= k)
            m_DWActMoves.Insert("");
        m_DWActMoves[k] = move;
    }

    // ------------------------------------------------------------ pig

    protected void DGPaintPig()
    {
        array<string> st = new array<string>();
        DGZone("pig", st);
        int die = DGIntAt(st, 0);
        int turnPts = DGIntAt(st, 1);
        int goal = DGIntAt(st, 2);
        if (goal <= 0)
            goal = 100;
        int throws = DGIntAt(st, 3);
        bool playing = DWPlaying();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        int n = m_GPub.SeatList.Count();
        if (n > 6)
            n = 6;
        int i;
        for (i = 0; i < 6; i++)
        {
            string r = "OZG_DPgRow" + i.ToString();
            Widget row = BW(r);
            if (!row)
                continue;
            bool shown = i < n;
            row.Show(shown);
            if (!shown)
                continue;
            OZG_GSeat g = m_GPub.SeatList[i];
            int gSeat = g.SeatNo;
            string gLine = g.Line;
            string gRole = g.Role;
            int gOut = g.Out;
            int gMark = g.Mark;
            row.SetPos(78 * m_K, (64 + 66 * i) * m_K);
            bool current = gSeat == turn;
            bool me = gSeat == m_V.Mine;
            string name = SeatName(gSeat);
            string nameTail = DGYou(gSeat);
            DGNameIn(r + "Name", name, nameTail, 140);
            DGNameIn(r + "NameB", name, nameTail, 140);
            BWShow(r + "Name", !current);
            BWShow(r + "NameB", current);
            int ring = DGC(74, 55, 38);
            if (current)
                ring = DGC(201, 164, 92);
            // into a local first: a compound condition is never written straight into a field
            bool snugRow = gOut == 1;
            m_DGSnug = snugRow;
            DGBox(r + "Box", 944, 54, 8, ring, 1);
            m_DGSnug = false;
            if (gOut == 1)
                DGDim(row, r, 0.53, 0.62);
            else
                DGDim(row, r, 1, 1);
            int score = DGArgInt(gLine, 0);
            if (score < 0)
                score = 0;
            int held = score;
            if (held > goal)
                held = goal;
            float fw = 674.0 * held / goal;
            Widget fill = BW(r + "BarFill");
            if (fill)
                fill.SetSize(fw * m_K, 14 * m_K);
            int bar = DGC(143, 125, 98);
            if (me)
                bar = DGC(201, 164, 92);
            BWPaintNamed(r + "BarFillShape", bar);
            int extra = 0;
            if (current)
            {
                extra = turnPts;
                if (extra > goal - held)
                    extra = goal - held;
            }
            float tw = 674.0 * extra / goal;
            Widget turnW = BW(r + "BarTurn");
            if (turnW)
            {
                turnW.SetPos(fw * m_K, 0);
                turnW.SetSize(tw * m_K, 14 * m_K);
            }
            BWMove(BW(r + "BarTurnShape"), -fw, 0);
            BWSetText(r + "Score", score.ToString());
        }
        // the die under the rows (lower when they are many), the pill, the words. The
        // canvas has four seats; with six the rows leave the felt 237 units, and the
        // canvas's die with its gaps wants 297: the gap above the die gives way first,
        // then the die itself (the stand, 2026-11-26: the words lay on the tray's rim)
        float rowsBottom = 52 + n * 66;
        float gap = 64;
        float pigDie = 128;
        float room = 685 - rowsBottom;
        float need = gap + pigDie + 105;
        if (need > room)
        {
            gap = 20;
            need = gap + pigDie + 105;
        }
        if (need > room)
        {
            pigDie = pigDie - (need - room);
            if (pigDie < 80)
                pigDie = 80;
        }
        float dieX = 550 - pigDie / 2;
        float dieY = 314;
        if (rowsBottom + gap > dieY)
            dieY = rowsBottom + gap;
        float turnDeg = 0;
        if (throws > 0)
        {
            int spin = (throws * 37) % 31;
            turnDeg = spin - 15;
        }
        bool one = die == 1;
        int oneRed = DGC(142, 42, 30);
        if (one)
            DGRing(dieX, dieY, pigDie, 4, turnDeg, oneRed, 30);
        string pigLook = DGLookOf(turn, 0);
        DGDie(die, pigLook, dieX, dieY, pigDie, turnDeg, 12, 31, one);
        int load = DGMyLoad(0);
        if (mine && load > 0)
            DGBadge(load, dieX + pigDie - 12, dieY - 12);
        float lw = DGText("OZG_DPgPillLabel", OZG_Text.Upper(Tr("STR_OZG_DWIN_PIG_TURN")));
        float vw = DGText("OZG_DPgPillValue", turnPts.ToString());
        float pw = 20 + lw + 10 + vw + 20;
        BWMove(BW("OZG_DPgPill"), DGSnap(550 - pw / 2), dieY + pigDie + 18);
        Widget pill = BW("OZG_DPgPill");
        if (pill)
            pill.SetSize(pw * m_K, 50 * m_K);
        DGBox("OZG_DPgPillBox", pw, 50, 25, DGC(201, 164, 92), 1);
        BWMove(BW("OZG_DPgPillValue"), 20 + lw + 10, 7.5);
        string say = "";
        if (playing && mine && turnPts > 0)
            say = Tr1("STR_OZG_DWIN_PIG_MORE", turnPts.ToString());
        else if (playing && mine)
            say = Tr("STR_OZG_DWIN_PIG_FIRST");
        else if (playing && turn >= 0)
            say = Tr1("STR_OZG_DWIN_PIG_THROWS", SeatName(turn));
        BWSetText("OZG_DPgSay", say);
        BWMove(BW("OZG_DPgSay"), 250, dieY + pigDie + 86);

        // the right column: the turn, the swap, the log
        float bottom = 170;
        BWShow("OZG_DPgAct", playing);
        if (playing)
        {
            string title = Tr1("STR_OZG_DWIN_TURN_OF", SeatName(turn));
            if (mine)
                title = Tr("STR_OZG_DICE_YOUR_TURN");
            BWSetText("OZG_DPgActTitle", title);
            int edge = DGC(107, 82, 51);
            if (mine)
                edge = DGC(201, 164, 92);
            BWPaintNamed("OZG_DPgActBoxEdge", edge);
            bool canThrow = false;
            bool canHold = false;
            if (m_GOwn)
            {
                for (i = 0; i < m_GOwn.Buttons.Count(); i++)
                {
                    OZG_GButton b = m_GOwn.Buttons[i];
                    if (b && b.Move == OZG_DiceGameConst.M_THROW)
                        canThrow = true;
                    if (b && b.Move == OZG_PigConst.M_HOLD)
                        canHold = true;
                }
            }
            if (mine && canThrow)
            {
                m_DGMoveA = OZG_DiceGameConst.M_THROW;
                DGButton("OZG_DPgThrow", Tr("STR_OZG_DICE_BTN_THROW"), DGC(201, 164, 92), DGC(201, 164, 92), DGC(26, 18, 10));
            }
            else
            {
                DGButton("OZG_DPgThrow", Tr("STR_OZG_DICE_BTN_THROW"), DGC(51, 37, 26), DGC(51, 37, 26), DGC(160, 143, 115));
            }
            int holdRing = DGC(61, 45, 29);
            int holdInk = DGC(160, 143, 115);
            if (mine && canHold)
            {
                m_DGMoveB = OZG_PigConst.M_HOLD;
                holdRing = DGC(201, 164, 92);
                holdInk = DGC(239, 228, 204);
            }
            DGButton("OZG_DPgHold", Tr1("STR_OZG_DWIN_PIG_HOLD", turnPts.ToString()), DGC(45, 33, 23), holdRing, holdInk);
        }
        else
        {
            bottom = DGTableActs(12);
        }
        float swapBottom = DGPaintSwap(bottom + 10, "pig");
        DGLog(swapBottom + 10, 762);
    }

    // ------------------------------------------------------------ high roll

    protected void DGPaintHigh()
    {
        array<string> st = new array<string>();
        DGZone("highroll", st);
        int count = DGIntAt(st, 0);
        if (count <= 0)
            count = 2;
        array<string> chipsOf = new array<string>();
        array<string> putsOf = new array<string>();
        if (st.Count() > 2)
            st[2].Split(",", chipsOf);
        if (st.Count() > 3)
            st[3].Split(",", putsOf);
        int pot = DGPot();
        bool playing = DWPlaying();
        bool mine = DGMyTurn();
        int n = m_GPub.SeatList.Count();
        if (n > 8)
            n = 8;
        int cols = 2;
        if (n > 4)
            cols = 3;
        if (n > 6)
            cols = 4;
        float cw = (984 - (cols - 1) * 18.0) / cols;
        array<float> rots = new array<float>();
        rots.Insert(-7);
        rots.Insert(5);
        rots.Insert(-3);
        int i;
        int k;
        for (i = 0; i < 8; i++)
        {
            string c = "OZG_DHCard" + i.ToString();
            Widget card = BW(c);
            if (!card)
                continue;
            bool shown = i < n;
            card.Show(shown);
            if (!shown)
                continue;
            OZG_GSeat g = m_GPub.SeatList[i];
            int gSeat = g.SeatNo;
            string gLine = g.Line;
            string gRole = g.Role;
            int gOut = g.Out;
            int gMark = g.Mark;
            int col = i % cols;
            int row = i / cols;
            float cx = 58 + col * (cw + 18);
            float cy = 64 + row * 214;
            card.SetPos(cx * m_K, cy * m_K);
            card.SetSize(cw * m_K, 196 * m_K);
            bool won = gMark == 1;
            bool gone = gOut == 1 && !won;
            int ring = DGC(107, 82, 51);
            float ringW = 1;
            if (won)
            {
                ring = DGC(231, 194, 122);
                ringW = 3;
            }
            else if (gone)
            {
                ring = DGC(61, 45, 29);
            }
            bool snugCard = gone && playing;
            m_DGSnug = snugCard;
            DGBox(c + "Box", cw, 196, 10, ring, ringW);
            m_DGSnug = false;
            float cardAlpha = 1;
            if (gone && playing)
            {
                cardAlpha = 0.5;
                DGDim(card, c, 0.53, 0.62);
            }
            else
            {
                DGDim(card, c, 1, 1);
            }
            string chips = "0";
            if (i < chipsOf.Count())
                chips = chipsOf[i];
            float chw = DGText(c + "Chips", Tr1("STR_OZG_DICE_POT_LINE", chips));
            BWMove(BW(c + "Chips"), cw - 16 - 167, 15.5);
            string whose = Tr("STR_OZG_DWIN_HR_HOST");
            if (m_V.OwnDiceSeats.Find(gSeat) >= 0)
                whose = Tr("STR_OZG_DWIN_HR_OWN");
            float ww = DGText(c + "Whose", whose);
            Widget whoseW = BW(c + "Whose");
            if (whoseW)
            {
                whoseW.SetPos((cw - 16 - chw - 10 - 120) * m_K, 16.5 * m_K);
            }
            // the name up to whose dice he throws (the canvas's gap of 10)
            string highName = SeatName(gSeat);
            string highTail = DGYou(gSeat);
            DGNameIn(c + "Name", highName, highTail, cw - 16 - chw - 10 - ww - 10 - 16);
            array<int> dice = new array<int>();
            DWDiceOf(gRole, dice);
            bool thrown = dice.Count() > 0;
            BWShow(c + "Empty", !thrown);
            BWSetText(c + "Empty", Tr("STR_OZG_DWIN_NOT_THROWN"));
            float ds = 70;
            float room = cw - 32 - 70;
            if (dice.Count() > 0 && dice.Count() * 70 + (dice.Count() - 1) * 14 > room)
                ds = (room - (dice.Count() - 1) * 14) / dice.Count();
            int sum = 0;
            if (cardAlpha < 1)
                m_DGDieAlpha = 96;
            for (k = 0; k < dice.Count() && k < 3; k++)
            {
                float dx = cx + 16 + k * (ds + 14);
                float dy = cy + 46 + (80 - ds) / 2;
                int hrFace = dice[k];
                float hrDeg = rots[k];
                string hrLook = DGThrownLookOf(gSeat, k);
                DGDie(hrFace, hrLook, dx, dy, ds, hrDeg, 6, 31, false);
                sum = sum + hrFace;
                if (gSeat == m_V.Mine && k == 0)
                {
                    int load = DGMyLoad(0);
                    if (load > 0)
                        DGBadge(load, dx + ds + 10 - 24, dy - 10);
                }
            }
            m_DGDieAlpha = 255;
            string sumText = "";
            if (thrown)
                sumText = sum.ToString();
            BWSetText(c + "Sum", sumText);
            BWMove(BW(c + "Sum"), cw - 16 - 167, 59.5);
            int sumInk = DGC(239, 228, 204);
            if (won)
                sumInk = DGC(231, 194, 122);
            BWInk(c + "Sum", sumInk);
            string state = "";
            int stateInk = DGC(184, 165, 133);
            if (won)
            {
                state = Tr("STR_OZG_DWIN_HR_WON");
                stateInk = DGC(231, 194, 122);
            }
            else if (gone && playing)
            {
                state = Tr("STR_OZG_DWIN_HR_OUT");
            }
            else if (m_DGLobby && i < putsOf.Count() && putsOf[i].ToInt() > 0)
            {
                // the canvas says a seat's stake while the stakes are made, not after
                state = Tr1("STR_OZG_DWIN_HR_STAKE", putsOf[i]);
            }
            BWSetText(c + "State", state);
            BWInk(c + "State", stateInk);
        }
        DGPill("OZG_DHBank", pot, 418, 514);

        // the right column: the pot or the throw, the swap, the log
        float bottom = 143;
        BWShow("OZG_DHAct", playing);
        if (playing)
        {
            string title = Tr1("STR_OZG_DWIN_HR_POT", pot.ToString());
            string hint = Tr("STR_OZG_DWIN_HR_HINT_WAIT");
            if (mine)
            {
                title = Tr("STR_OZG_DWIN_YOUR_THROW");
                hint = Tr("STR_OZG_DWIN_HR_HINT_THROW");
            }
            BWSetText("OZG_DHActTitle", title);
            BWSetText("OZG_DHActHint", hint);
            if (mine)
            {
                m_DGMoveA = OZG_DiceGameConst.M_THROW;
                DGButton("OZG_DHThrow", Tr1("STR_OZG_DWIN_HR_THROW", count.ToString()), DGC(201, 164, 92), DGC(201, 164, 92), DGC(26, 18, 10));
            }
            else
            {
                DGButton("OZG_DHThrow", Tr("STR_OZG_DWIN_HR_WAITING"), DGC(51, 37, 26), DGC(51, 37, 26), DGC(160, 143, 115));
            }
        }
        else
        {
            bottom = DGTableActs(12);
        }
        float swapBottom = DGPaintSwap(bottom + 10, "high");
        DGLog(swapBottom + 10, 762);
    }

    override protected string DGDiceChip()
    {
        if (!m_V || m_V.GameId != "highroll")
            return "";
        return Tr1("STR_OZG_DWIN_HR_COUNT", DGOption(OZG_HighRollConst.OPT_DICE, "2"));
    }

    // ------------------------------------------------------------ perudo

    // Where perudo's card `i` of `n` lies and how wide (the canvas: three of 310 over 1004
    // from 100; more narrower in the same row).
    protected void DGPerudoSpot(int i, int n, out float x, out float w)
    {
        w = 310;
        x = 48;
        if (n == 1)
        {
            x = 395;
            return;
        }
        if (n == 2)
        {
            x = 48 + i * 694;
            return;
        }
        if (n == 3)
        {
            x = 48 + i * 347;
            return;
        }
        float gap = 12;
        if (n == 5)
            gap = 10;
        if (n >= 6)
            gap = 8;
        w = (1004 - (n - 1) * gap) / n;
        x = 48 + i * (w + gap);
    }

    // The dice of words "KEY|1 3 5" (perudo's own: "STR_OZG_PERUDO_YOURS|..").
    protected void DGDiceArg(string words, array<int> into)
    {
        into.Clear();
        array<string> args = new array<string>();
        DWArgs(words, args);
        if (args.Count() > 0)
            OZG_DiceText.Parse(args[0], into);
    }

    // A zone's line as the game wrote it ("" when it has none).
    protected string DGZoneLine(string id)
    {
        if (!m_GPub)
            return "";
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            OZG_GZone zone = m_GPub.Zones[z];
            if (zone && zone.Id == id)
                return zone.Line;
        }
        return "";
    }

    protected void DGPaintPerudo()
    {
        array<string> st = new array<string>();
        DGZone("perudo", st);
        int bidN = DGIntAt(st, 0);
        int bidF = DGIntAt(st, 1);
        int bidder = -1;
        if (st.Count() > 2)
            bidder = st[2].ToInt();
        bool pal = DGIntAt(st, 3) == 1;
        int inPlay = DGIntAt(st, 4);
        if (inPlay < 1)
            inPlay = 1;
        m_DGInPlay = inPlay;
        bool playing = DWPlaying();
        bool ended = m_GPub.Ended == 1;
        // the last showdown (its bid, wild ones, the loser, whose bid it was, every seat's
        // dice): the dice lie open DG_REVEAL seconds after it, as the canvas shows them, and
        // for good once the game is over; then the cans are down and the new round is on
        string openLine = DGZoneLine("perudo_open");
        array<string> open = new array<string>();
        if (openLine != "")
            openLine.Split("|", open);
        bool opened = openLine != "";
        if (opened && !ended && bidN > 0)
            opened = false;
        if (!opened)
        {
            m_DGOpenSeen = "";
            m_DGOpenLeft = 0;
        }
        else if (openLine != m_DGOpenSeen)
        {
            m_DGOpenSeen = openLine;
            m_DGOpenLeft = DG_REVEAL;
        }
        bool reveal = false;
        if (opened)
        {
            if (ended || m_DGOpenLeft > 0)
                reveal = true;
        }
        int openN = DGIntAt(open, 0);
        int openF = DGIntAt(open, 1);
        bool openPal = DGIntAt(open, 2) == 1;
        int openBidder = -1;
        if (open.Count() > 4)
            openBidder = open[4].ToInt();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        // while the dice lie open nobody bids
        bool myTurn = mine;
        if (reveal)
            myTurn = false;
        int shownBidder = bidder;
        if (reveal)
            shownBidder = openBidder;
        array<ref OZG_GSeat> others = new array<ref OZG_GSeat>();
        DGOthers(others);
        int n = others.Count();
        int i;
        int k;
        for (i = 0; i < 6; i++)
        {
            string c = "OZG_DRCard" + i.ToString();
            Widget card = BW(c);
            if (!card)
                continue;
            bool shown = i < n;
            card.Show(shown);
            if (!shown)
                continue;
            OZG_GSeat g = others[i];
            int gSeat = g.SeatNo;
            string gLine = g.Line;
            string gRole = g.Role;
            int gOut = g.Out;
            int gMark = g.Mark;
            float cx;
            float cw;
            DGPerudoSpot(i, n, cx, cw);
            card.SetPos(cx * m_K, 54 * m_K);
            card.SetSize(cw * m_K, 132 * m_K);
            // out of the game: his card dims once the dice are covered again
            bool gone = gOut == 1 && !reveal;
            bool thinks = gSeat == turn && playing && !reveal;
            bool hisBid = gSeat == shownBidder;
            int ring = DGC(74, 55, 38);
            float ringW = 1;
            if (thinks)
            {
                ring = DGC(201, 164, 92);
                ringW = 3;
            }
            else if (hisBid)
            {
                ring = DGC(138, 106, 62);
            }
            m_DGSnug = gone;
            DGBox(c + "Box", cw, 132, 10, ring, ringW);
            m_DGSnug = false;
            if (gone)
                DGDim(card, c, 0.48, 0.58);
            else
                DGDim(card, c, 1, 1);
            int have = DGArgInt(gLine, 0);
            if (have < 0)
                have = 0;
            float countW = DGText(c + "Count", Tr1("STR_OZG_DWIN_PR_DICE", have.ToString()));
            BWMove(BW(c + "Count"), cw - 12 - 148, 14);
            // the name up to how many dice he has
            string perudoName = SeatName(gSeat);
            DGNameIn(c + "Name", perudoName, "", cw - 12 - countW - 8 - 12);
            // the dice shown at the showdown, or the can
            array<int> shownDice = new array<int>();
            if (reveal)
                DGOpenDice(open, 5, gSeat, shownDice);
            bool showsDice = reveal && shownDice.Count() > 0;
            bool cup = !showsDice && gOut != 1;
            BWShow(c + "Cup", cup);
            BWShow(c + "CupNote", cup && cw >= 250);
            BWSetText(c + "CupNote", Tr("STR_OZG_DWIN_PR_CUP"));
            if (showsDice)
            {
                float ds = 40;
                if (shownDice.Count() * 48 - 8 > cw - 24)
                    ds = (cw - 24 + 8) / shownDice.Count() - 8;
                for (k = 0; k < shownDice.Count(); k++)
                {
                    int v = shownDice[k];
                    float dx = cx + 12 + k * (ds + 8);
                    float dy = 54 + 42 + (52 - ds) / 2;
                    bool wild = v == 1 && !openPal && openF != 1;
                    int shownGold = DGC(231, 194, 122);
                    if (v == openF || wild)
                        DGRing(dx, dy, ds, 3, 0, shownGold, 30);
                    string shownLook = DGThrownLookOf(gSeat, k);
                    DGDie(v, shownLook, dx, dy, ds, 0, 2, 31, false);
                }
            }
            // the canvas: "вибув", or whose the bid is, or who thinks now
            string line = "";
            int lineInk = DGC(184, 165, 133);
            if (gone)
            {
                line = Tr("STR_OZG_DWIN_OUT");
            }
            else if (hisBid)
            {
                line = Tr("STR_OZG_DWIN_PR_HIS_BID");
            }
            else if (thinks)
            {
                line = Tr("STR_OZG_DWIN_PR_THINKS");
                lineInk = DGC(166, 217, 160);
            }
            BWSetText(c + "Line", line);
            BWInk(c + "Line", lineInk);
        }
        // the bid in the middle
        string table = Tr1("STR_OZG_DWIN_PR_TABLE_" + CountForm(inPlay), inPlay.ToString());
        if (pal)
            table = table + Tr("STR_OZG_DWIN_PR_NOT_WILD");
        else
            table = table + Tr("STR_OZG_DWIN_PR_WILD");
        if (reveal)
        {
            string openLabel = Tr("STR_OZG_PERUDO_BID_NOW");
            if (openBidder >= 0)
                openLabel = Tr1("STR_OZG_DWIN_PR_BID_BY", SeatName(openBidder));
            BWSetText("OZG_DRBidLabel", OZG_Text.Upper(openLabel));
            BWSetText("OZG_DRBidText", FaceCount(openN, openF));
            BWSetText("OZG_DRBidSub", Tr("STR_OZG_DWIN_PR_OPEN_SUB"));
        }
        else if (bidN > 0)
        {
            BWSetText("OZG_DRBidLabel", OZG_Text.Upper(Tr1("STR_OZG_DWIN_PR_BID_BY", SeatName(bidder))));
            BWSetText("OZG_DRBidText", FaceCount(bidN, bidF));
            BWSetText("OZG_DRBidSub", table);
        }
        else
        {
            BWSetText("OZG_DRBidLabel", OZG_Text.Upper(Tr("STR_OZG_PERUDO_BID_NOW")));
            BWSetText("OZG_DRBidText", Tr("STR_OZG_DWIN_PR_NO_BID"));
            BWSetText("OZG_DRBidSub", table);
        }
        // this player's own box and dice: while the dice lie open, the ones he showed, what
        // counts ringed; then the ones under his can now
        int mineNo = m_V.Mine;
        OZG_GSeat me = DGSeat(mineNo);
        bool seated = me != null;
        string meLine = "";
        if (seated)
            meLine = me.Line;
        BWShow("OZG_DRMe", seated);
        if (seated)
        {
            int myCount = DGArgInt(meLine, 0);
            if (myCount < 0)
                myCount = 0;
            string perudoMine = SeatName(mineNo);
            string perudoTail = DGYou(mineNo);
            DGNameIn("OZG_DRMeName", perudoMine, perudoTail, 150);
            BWSetText("OZG_DRMeCount", Tr1("STR_OZG_DWIN_PR_MINE", myCount.ToString()));
            int meRing = DGC(74, 55, 38);
            float meRingW = 1;
            if (myTurn && playing)
            {
                meRing = DGC(201, 164, 92);
                meRingW = 3;
            }
            DGBox("OZG_DRMeBox", 684, 102, 10, meRing, meRingW);
            array<int> myDice = new array<int>();
            if (reveal)
                DGOpenDice(open, 5, mineNo, myDice);
            else if (m_GOwn && !ended)
                DGDiceArg(m_GOwn.Status, myDice);
            for (k = 0; k < myDice.Count() && k < 5; k++)
            {
                float mx = 386 + k * 88;
                int mv = myDice[k];
                bool mwild = mv == 1 && !openPal && openF != 1;
                int mineGold = DGC(231, 194, 122);
                if (reveal && (mv == openF || mwild))
                    DGRing(mx, 488, 74, 4, 0, mineGold, 30);
                string mineLook = DGThrownLookOf(mineNo, k);
                DGDie(mv, mineLook, mx, 488, 74, 0, 6, 31, false);
                int load = DGMyLoad(k);
                if (load > 0 && !reveal)
                    DGBadge(load, mx + 74 + 9 - 24, 488 - 9);
            }
        }
        // the bid being made: picked again from the bid standing whenever it changes
        string seen = bidN.ToString() + ":" + bidF.ToString() + ":" + bidder.ToString();
        if (seen != m_DGBidSeen)
        {
            m_DGBidSeen = seen;
            m_DGCount = bidN;
            if (m_DGCount < 1)
                m_DGCount = 1;
            m_DGPick = bidF;
            if (m_DGPick < 1)
                m_DGPick = 2;
        }
        if (m_DGCount > inPlay)
            m_DGCount = inPlay;
        m_DGLocked = 0;
        if (pal && bidN > 0)
        {
            m_DGLocked = bidF;
            m_DGPick = bidF;
        }
        BWShow("OZG_DRAsk", playing);
        float bottom = 272;
        if (playing)
        {
            string title = Tr1("STR_OZG_DWIN_PR_BIDS", SeatName(turn));
            if (reveal)
                title = Tr("STR_OZG_DWIN_PR_OPEN");
            else if (myTurn)
                title = Tr("STR_OZG_DWIN_PR_YOUR_BID");
            BWSetText("OZG_DRAskTitle", title);
            BWSetText("OZG_DRValue", m_DGCount.ToString());
            for (i = 0; i < 6; i++)
            {
                string f = "OZG_DRFace" + i.ToString();
                int face = i + 1;
                bool on = face == m_DGPick;
                ImageWidget pic = ImageWidget.Cast(BW(f + "Pic"));
                if (pic)
                {
                    pic.LoadImageFile(0, "set:ozg_dice image:white_" + face.ToString());
                    pic.SetImage(0);
                    int tint = ARGB(255, 218, 215, 205);
                    if (on)
                        tint = ARGB(255, 255, 255, 255);
                    pic.SetColor(tint);
                }
                BWShow(f + "Ring", on);
                // the canvas's 0.35 of a light die over the dark panel, in linear light
                float faceAlpha = 1;
                if (m_DGLocked > 0 && face != m_DGLocked)
                    faceAlpha = 0.15;
                Widget fw = BW(f);
                if (fw)
                    fw.SetAlpha(faceAlpha);
            }
            string move = OZG_PerudoConst.M_BID + m_DGCount.ToString() + ":" + m_DGPick.ToString();
            bool valid = myTurn && m_GOwn && m_GOwn.CanPlay.Find(move) >= 0;
            string note = Tr("STR_OZG_DWIN_PR_WAIT");
            // out of the game with others still playing: the canvas's words for it
            if (seated && DGArgInt(meLine, 0) <= 0 && !reveal)
                note = Tr("STR_OZG_DWIN_PR_YOU_OUT");
            int noteInk = DGC(184, 165, 133);
            if (valid)
            {
                note = Tr1("STR_OZG_DWIN_PR_OK", FaceCount(m_DGCount, m_DGPick));
            }
            else if (myTurn)
            {
                note = Tr("STR_OZG_DWIN_PR_LOW");
                if (bidF == 1 && m_DGPick != 1)
                    note = Tr("STR_OZG_DWIN_PR_LOW_ONES");
                noteInk = DGC(240, 154, 134);
            }
            BWSetText("OZG_DRNote", note);
            BWInk("OZG_DRNote", noteInk);
            if (valid)
            {
                m_DGMoveA = move;
                DGButton("OZG_DRBidBtn", Tr("STR_OZG_DWIN_PR_BID"), DGC(201, 164, 92), DGC(201, 164, 92), DGC(26, 18, 10));
            }
            else
            {
                DGButton("OZG_DRBidBtn", Tr("STR_OZG_DWIN_PR_BID"), DGC(51, 37, 26), DGC(51, 37, 26), DGC(160, 143, 115));
            }
            int doubt = DGC(58, 38, 32);
            if (myTurn && bidN > 0)
            {
                m_DGMoveB = OZG_PerudoConst.M_DUDO;
                doubt = DGC(124, 50, 38);
            }
            DGButton("OZG_DRDudoBtn", Tr("STR_OZG_DWIN_PR_DUDO"), doubt, doubt, DGC(251, 233, 228));
        }
        else
        {
            bottom = DGTableActs(12);
        }
        float swapBottom = DGPaintSwap(bottom + 10, "perudo");
        DGLog(swapBottom + 10, 762);
    }

    // The dice a seat showed at the last showdown ("<seat>=1 3 5" among the zone's parts
    // from `first` on).
    protected void DGOpenDice(array<string> open, int first, int seatNo, array<int> into)
    {
        into.Clear();
        string key = seatNo.ToString() + "=";
        int keyLen = key.Length();
        for (int k = first; k < open.Count(); k++)
        {
            string part = open[k];
            if (part.IndexOf(key) != 0)
                continue;
            string laid = part.Substring(keyLen, part.Length() - keyLen);
            OZG_DiceText.Parse(laid, into);
            return;
        }
    }

    // The showdown's time runs out: the cans go down.
    override protected void DGTick(float dt)
    {
        if (m_DGOpenLeft <= 0)
            return;
        m_DGOpenLeft = m_DGOpenLeft - dt;
        if (m_DGOpenLeft > 0)
            return;
        m_DGOpenLeft = 0;
        if (m_DGActive)
            BoardWinPaint();
    }

    override protected float DWSwapPanel(float top)
    {
        return DGPaintSwap(top, "thousand");
    }

    override protected string DWThrownLook(int seatNo, int k)
    {
        return DGThrownLookOf(seatNo, k);
    }

    // ------------------------------------------------------------ clicks

    override protected bool DGClick(string n)
    {
        // high roll's header: the dice each, the host's between games
        if (n == "OZG_DBtnDice" && m_V && m_V.GameId == "highroll")
        {
            if (m_V.IsHost == 1 && !DWPlaying())
            {
                string now = DGOption(OZG_HighRollConst.OPT_DICE, "2");
                int more = now.ToInt() + 1;
                if (more > 3)
                    more = 1;
                Send(OZG_Const.OP_OPT, "", -1, "", OZG_HighRollConst.OPT_DICE, more.ToString(), 0);
            }
            return true;
        }
        // the swap panel is thousand's too, which has no face
        if (DGSwapClick(n))
            return true;
        if (!m_DGActive)
            return false;
        if (n == "OZG_DPgThrow" || n == "OZG_DHThrow" || n == "OZG_DRBidBtn")
        {
            if (m_DGMoveA != "")
                Play(m_DGMoveA);
            return true;
        }
        return DGFaceClick(n);
    }

    // The swap panel's own clicks: a die of the pocket, a die of the throw, the button.
    protected bool DGSwapClick(string n)
    {
        if (n == "OZG_DSwapSBtn")
        {
            if (m_DGSwapKey != "" && m_DGSwapAt >= 0)
            {
                m_DGSwapPickKey = "";
                m_DGSwapTarget = -1;
                OZG_WAsk ask = new OZG_WAsk();
                ask.PlaceId = m_PlaceId;
                ask.CardKey = m_DGSwapKey;
                ask.SeatNo = m_DGSwapAt;
                string json;
                string err;
                JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
                OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_DSWAP, json);
                // the picks are let go at once: a swap refused sends no new view
                BoardWinPaint();
            }
            return true;
        }
        if (n.IndexOf("OZG_DSPocket") == 0 && n.Length() > 12)
        {
            // "OZG_DSPocket<place>": the row's scroll and its inside are named alike
            string placeText = n.Substring(12, n.Length() - 12);
            int p = placeText.ToInt();
            if (placeText != p.ToString())
                return false;
            if (m_V && p >= 0 && p < m_V.PocketDice.Count())
            {
                string key = m_V.PocketDice[p];
                if (key == m_DGSwapPickKey)
                    m_DGSwapPickKey = "";
                else
                    m_DGSwapPickKey = key;
            }
            BoardWinPaint();
            return true;
        }
        if (n.IndexOf("OZG_DSTarget") == 0 && n.Length() == 13)
        {
            int t = n.Substring(12, 1).ToInt();
            if (t == m_DGSwapTarget)
                m_DGSwapTarget = -1;
            else
                m_DGSwapTarget = t;
            BoardWinPaint();
            return true;
        }
        return false;
    }

    // A face's own buttons.
    protected bool DGFaceClick(string n)
    {
        if (n == "OZG_DPgHold" || n == "OZG_DRDudoBtn")
        {
            if (m_DGMoveB != "")
                Play(m_DGMoveB);
            return true;
        }
        if (n == "OZG_DRMinus")
        {
            if (m_DGCount > 1)
                m_DGCount--;
            BoardWinPaint();
            return true;
        }
        if (n == "OZG_DRPlus")
        {
            if (m_DGCount < m_DGInPlay)
                m_DGCount++;
            BoardWinPaint();
            return true;
        }
        if (n.IndexOf("OZG_DRFace") == 0 && n.Length() == 11)
        {
            int face = n.Substring(10, 1).ToInt() + 1;
            if (m_DGLocked == 0 || face == m_DGLocked)
                m_DGPick = face;
            BoardWinPaint();
            return true;
        }
        if (n == "OZG_DHit")
        {
            // dice poker's dice are picked in his second throw only
            if (m_DGFace == "poker" && m_DGPhase == 2 && m_DGMine)
                DGPickAtPointer();
            return true;
        }
        if (m_DGFace == "poker" && n.IndexOf("OZG_DAct") == 0 && n.Length() <= 10 && DWPlaying())
        {
            int k = n.Substring(8, n.Length() - 8).ToInt();
            if (k < 0 || k >= m_DWActMoves.Count())
                return true;
            string move = m_DWActMoves[k];
            if (move == "")
                return true;
            if (move == "#again")
                move = DGAgainMove();
            m_DWPicked.Clear();
            Play(move);
            BoardWinPaint();
            return true;
        }
        return false;
    }

    // The die of dice poker's under the pointer picked or let go.
    protected void DGPickAtPointer()
    {
        float x;
        float y;
        PointerAt(x, y);
        float tx = x - 52;
        float ty = y - 86;
        for (int i = 0; i < m_DWDieX.Count(); i++)
        {
            bool onX = Math.AbsFloat(tx - m_DWDieX[i]) <= 46;
            bool onY = i < m_DWDieY.Count() && Math.AbsFloat(ty - m_DWDieY[i]) <= 46;
            if (onX && onY)
            {
                int at = m_DWPicked.Find(i);
                if (at >= 0)
                    m_DWPicked.Remove(at);
                else
                    m_DWPicked.Insert(i);
                BoardWinPaint();
                return;
            }
        }
    }

    // "again:1,3" of the dice picked (their places from 1), "again:0" for none.
    protected string DGAgainMove()
    {
        string list = "";
        for (int p = 1; p <= 5; p++)
        {
            if (m_DWPicked.Find(p - 1) < 0)
                continue;
            if (list != "")
                list = list + ",";
            list = list + p.ToString();
        }
        if (list == "")
            list = "0";
        return OZG_DPokerConst.M_AGAIN + list;
    }
}
