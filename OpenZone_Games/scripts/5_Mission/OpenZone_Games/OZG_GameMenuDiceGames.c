// The five dice games' own faces (boards and dice design §6, §15): the design canvas's
// boards "Тисяча", "Покер на кістках", "Свиня", "Хто більше" and "Перудо" as redone on
// 2026-11-27, measured to the unit. Each game's parts are layouts of their own
// (tools/make_dice_win.py: ozg_dg_<game>.layout on the felt, ozg_dg_<game>col.layout in the
// right column), made into the dice window (ozg_dwin.layout) when its game is played and
// dropped when another is.
//
// A table: the players' plates round the felt -- this player's own under its right, the
// others along its top --, a can of dice at the seat that throws (the table's one can when
// the dice are the host's: it stands at the thrower's and goes to the next; a can each
// when they are their own), the dice thrown out of it. The cans and the dice are
// OZG_GameMenuCans.c's; this file says where each stands and what it shows.
//
// The games tell what their faces need in their views: zones "throw", "aside", "lie" and
// "turn" (thousand), "dpoker" (phase, round, the raise and its seat, the stake), "pig" (the
// last die, the turn's points, the goal, the throws), "highroll" (the dice each, the stake,
// every seat's chips and stake), "perudo" (the bid, its seat, palifico, the dice on the
// table) and "perudo_open" (the last showdown); the view's head says what this player may
// swap and what he knows of every die (OZG_WView.DiceSwap*, SeatDiceMarks).
modded class OZG_GameMenu
{
    protected static const string DG_LAYOUTS = "OpenZone_Games/gui/layouts/";
    // the window the faces were made in, the face made, its two parts; painted now
    protected Widget m_DGWin;
    protected string m_DGFace = "";
    protected Widget m_DGTray;
    protected Widget m_DGCol;
    protected bool m_DGActive;
    // the moves of a face's two buttons (throw / stop, bid / do not believe, put aside /
    // write)
    protected string m_DGMoveA = "";
    protected string m_DGMoveB = "";
    protected string m_DGMoveC = "";
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
    // perudo's showdown: the dice lie open DG_REVEAL seconds (the canvas: 3), then the cans
    // are shaken and stand down again; the showdown it runs for (its zone's line), the time
    // left
    protected static const float DG_REVEAL = 3.0;
    protected static const float DG_SHAKE = 0.66;
    protected string m_DGOpenSeen = "";
    protected float m_DGOpenLeft = 0;
    // perudo's cans being shaken (a new round), seconds left
    protected float m_DGShakeLeft = 0;
    protected int m_DGRoundSeen = -1;
    // dice poker's round that was decided: shown DG_REVEAL_POKER seconds as it lay (the
    // canvas waits for "Наступний раунд"; at a table nobody holds the others up), kept
    // here because the server's own copy goes with the next round's first throw
    protected static const float DG_REVEAL_POKER = 4.0;
    protected string m_DGPokerOpen = "";
    // no game dealt yet: the face is drawn from the seats (DGLobbyView)
    protected bool m_DGLobby;
    // the box sized next is drawn through (a dimmed card): fitted snug, in whole pixels
    protected bool m_DGSnug;
    // the seats as the last paint laid them, and the tags of the plate being laid
    protected ref array<ref OZG_DTSeat> m_DGGeo;
    protected string m_DGTagP = "";
    protected int m_DGTagN;
    protected float m_DGTagX;
    protected float m_DGTagY;
    protected float m_DGTagX0;
    protected float m_DGTagRoom;

    // ------------------------------------------------------------ the faces

    protected int DGC(int r, int g, int b)
    {
        return ARGB(255, r, g, b);
    }

    // The face of a game: its layouts' name; "" for a game with none.
    protected string DGFaceOf(string gameId)
    {
        if (gameId == "thousand")
            return "thousand";
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
        // the dice a plate of the old face held went with it
        DGPlateDiceForget();
        return m_DGTray != null && m_DGCol != null;
    }

    // The window built anew: the faces went with the old one.
    protected void DGWindowCheck()
    {
        if (!m_DGGeo)
            m_DGGeo = new array<ref OZG_DTSeat>();
        if (m_DGWin == m_Win)
            return;
        m_DGWin = m_Win;
        m_DGFace = "";
        m_DGTray = null;
        m_DGCol = null;
        m_DGOpenSeen = "";
        m_DGOpenLeft = 0;
        m_DGShakeLeft = 0;
        m_DGRoundSeen = -1;
        m_DGPokerOpen = "";
    }

    // The dice made into a plate's row are forgotten with the plate (their ids begin "L").
    protected void DGPlateDiceForget()
    {
        if (!m_DTDice)
            return;
        for (int i = m_DTDice.Count() - 1; i >= 0; i--)
        {
            if (m_DTDice[i].m_Id.IndexOf("L") == 0)
                m_DTDice.RemoveOrdered(i);
        }
    }

    override protected bool DGPaint()
    {
        DGWindowCheck();
        m_DGActive = false;
        m_DGLobby = false;
        m_DTBlocked = false;
        string face = "";
        if (m_V)
            face = DGFaceOf(m_V.GameId);
        // no game dealt yet, or another's last view: the face is drawn from the seats, as
        // the canvas's boards stand before their first throw
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
        m_DGTray.Show(true);
        m_DGCol.Show(true);
        m_DGMoveA = "";
        m_DGMoveB = "";
        m_DGMoveC = "";
        m_DWActMoves.Clear();
        // through a local: a comparison is never written straight into a field
        bool under = face == "perudo";
        m_DTSwapUnder = under;
        BWShow("OZG_DScore", false);
        BWShow("OZG_DAct", false);
        BWShow("OZG_DTHint", false);
        if (face == "thousand")
            DGPaintThousand();
        else if (face == "poker")
            DGPaintPoker();
        else if (face == "pig")
            DGPaintPig();
        else if (face == "high")
            DGPaintHigh();
        else
            DGPaintPerudo();
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
            // put (dice poker); the total, the bolts and the tries (thousand); the score
            // (pig); the dice left (perudo)
            if (face == "poker")
                g.Line = "x|" + chipsText + "|0|0";
            else if (face == "perudo")
                g.Line = "x|" + OZG_PerudoConst.DICE.ToString();
            else if (face == "thousand")
                g.Line = "x|0|0|0";
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
        if (face == "pig")
        {
            OZG_GZone pig = new OZG_GZone();
            pig.Id = "pig";
            pig.Line = "0|0|" + OZG_PigConst.GOAL.ToString() + "|0";
            v.Zones.Insert(pig);
        }
        return v;
    }

    // A can with no game that has a face: the faces hidden.
    protected void DGHideFaces()
    {
        if (m_DGTray)
            m_DGTray.Show(false);
        if (m_DGCol)
            m_DGCol.Show(false);
    }

    // ------------------------------------------------------------ small things

    // A zone of the game's view by its id, its line split at "|".
    protected bool DGZone(string id, array<string> into)
    {
        into.Clear();
        OZG_GZone zone = DGZoneOf(id);
        if (!zone)
            return false;
        zone.Line.Split("|", into);
        return true;
    }

    protected OZG_GZone DGZoneOf(string id)
    {
        if (!m_GPub)
            return null;
        for (int z = 0; z < m_GPub.Zones.Count(); z++)
        {
            OZG_GZone zone = m_GPub.Zones[z];
            if (zone && zone.Id == id)
                return zone;
        }
        return null;
    }

    // A zone's line as the game wrote it ("" when it has none).
    protected string DGZoneLine(string id)
    {
        OZG_GZone zone = DGZoneOf(id);
        if (!zone)
            return "";
        return zone.Line;
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
        OZG_GZone zone = DGZoneOf("pot");
        if (!zone)
            return 0;
        int v = DGArgInt(zone.Line, 0);
        if (v < 0)
            v = 0;
        return v;
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
    // units: a player's name is as long as he made it, and what stands after it on a plate
    // does not move. The width it took.
    protected float DGNameIn(string widget, string name, float room)
    {
        TextWidget t = BWText(widget);
        if (!t)
            return 0;
        if (room < 20)
            room = 20;
        t.SetSize(room * m_K + 2, 20 * m_K);
        t.SetText(name);
        float w = BWTextW(t);
        if (w <= room)
            return w;
        int n = name.LengthUtf8();
        while (n > 1)
        {
            n--;
            string cut = name.SubstringUtf8(0, n);
            cut = cut.Trim();
            t.SetText(cut + "…");
            w = BWTextW(t);
            if (w <= room)
                return w;
        }
        return w;
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

    // Words wrapped into `most` lines "<name>0", "<name>1" ..; the lines they took (at
    // least `least`: the canvas keeps room for two).
    protected int DGLinesN(string name, string text, float w, int most, int least)
    {
        array<string> lines = new array<string>();
        BWWrap(BWText(name + "0"), text, w, most, lines);
        for (int i = 0; i < most; i++)
        {
            string line = "";
            if (i < lines.Count())
                line = lines[i];
            BWSetText(name + i.ToString(), line);
        }
        int n = lines.Count();
        if (n < least)
            n = least;
        return n;
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

    // The pot as the canvas draws it: the pill's chip, "БАНК" and the number, centred; at
    // x, y of the window.
    protected void DGPill(string name, int pot, float wx, float wy)
    {
        Widget pill = BW(name);
        if (!pill)
            return;
        pill.Show(true);
        pill.SetPos((wx - 52) * m_K, (wy - 86) * m_K);
        float lw = DGText(name + "Label", OZG_Text.Upper(Tr("STR_OZG_DICE_POT")));
        float vw = DGText(name + "Value", pot.ToString());
        float total = 30 + 12 + lw + 12 + vw;
        float x0 = DGSnap(132 - total / 2);
        BWMove(BW(name + "Icon"), x0, 20);
        BWMove(BW(name + "Label"), x0 + 42, 27);
        BWMove(BW(name + "Value"), x0 + 42 + lw + 12, 15);
    }

    // "ЗА ХІД" and the turn's points (the canvas: a pill 50 high in a box 252 wide at 476,
    // 576), as wide as its words, in the middle.
    protected void DGTurnPill(string name, int points)
    {
        float lw = DGText(name + "Label", OZG_Text.Upper(Tr("STR_OZG_DWIN_PIG_TURN")));
        float vw = DGText(name + "Value", points.ToString());
        float pw = 20 + lw + 10 + vw + 20;
        Widget pill = BW(name);
        if (pill)
        {
            pill.SetPos(DGSnap(550 - pw / 2) * m_K, 490 * m_K);
            pill.SetSize(pw * m_K, 50 * m_K);
        }
        DGBox(name + "Box", pw, 50, 25, DGC(201, 164, 92), 1);
        BWMove(BW(name + "Value"), 20 + lw + 10, 7.5);
    }

    // Words in the middle of the felt (the canvas: 500 wide at 352 of the window).
    protected void DGFeltHint(string text, float wy)
    {
        BWShow("OZG_DTHint", text != "");
        BWSetText("OZG_DTHint", text);
        BWMove(BW("OZG_DTHint"), 300, wy - 86);
    }

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

    // Between games the window's own action panel (the table's buttons) at `top`; its
    // bottom.
    protected float DGTableActs(float top)
    {
        BWShow("OZG_DAct", true);
        m_DWScoreH = top - 22;
        DWPaintActions();
        return m_DWActBottom;
    }

    // A panel of the column sized: its box `h` high, its ring gold while it is this
    // player's turn.
    protected void DGPanel(string name, float y, float h, bool mine)
    {
        Widget panel = BW(name);
        if (panel)
        {
            panel.SetPos(12 * m_K, y * m_K);
            panel.SetSize(360 * m_K, h * m_K);
        }
        int ring = DGC(107, 82, 51);
        if (mine)
            ring = DGC(201, 164, 92);
        m_DGSnug = false;
        DGBox(name + "Box", 360, h, 6, ring, 1);
    }

    // ------------------------------------------------------------ seats and plates

    // Where things stand at this player's own seat (the canvas's SEAT[0]): the tables of
    // three -- thousand, pig, dice poker ("th") --, high roll's ("hr") and perudo's ("pe").
    protected OZG_DTSeat DGGeoMe(string family, int seat, float plateY)
    {
        OZG_DTSeat g = new OZG_DTSeat();
        g.m_Seat = seat;
        if (family == "th")
        {
            g.m_PlateX = 790;
            g.m_PlateY = plateY;
            g.m_PlateW = 280;
            g.m_CanX = 128;
            g.m_CanY = 628;
            g.m_CanK = 1.5;
            g.m_MouthX = 262;
            g.m_MouthY = 660;
            g.m_Tip = 78;
            g.m_HandX = 168;
            g.m_HandY = 790;
            g.m_RowX = 332;
            g.m_RowY = 708;
            g.m_RowStep = 96;
            g.m_RowScale = 1;
            g.m_AsideX = 296;
            g.m_AsideY = 722;
            g.m_AsideStep = 52;
            g.m_AsideScale = 0.6;
            return g;
        }
        g.m_PlateX = 820;
        g.m_PlateY = 692;
        g.m_PlateW = 260;
        g.m_CanK = 1.25;
        g.m_Tip = 78;
        g.m_CanX = 150;
        if (family == "hr")
        {
            g.m_CanY = 632;
            g.m_MouthX = 264;
            g.m_MouthY = 668;
            g.m_HandX = 186;
            g.m_HandY = 772;
            g.m_RowX = 330;
            g.m_RowY = 692;
            g.m_RowStep = 96;
            g.m_RowScale = 1;
            return g;
        }
        g.m_CanY = 628;
        g.m_MouthX = 197.5;
        g.m_MouthY = 698;
        g.m_HandX = 186;
        g.m_HandY = 770;
        g.m_RowX = 300;
        g.m_RowY = 690;
        g.m_RowStep = 88;
        g.m_RowScale = 1;
        return g;
    }

    // The same for seat `i` of the `n` along the felt's top. The canvas has two there (its
    // tables of three: the left can tips right, the right one left) or three (its tables of
    // four); with more the plates share the row and the cans and the dice are smaller.
    protected OZG_DTSeat DGGeoTop(string family, int seat, int i, int n)
    {
        OZG_DTSeat g = new OZG_DTSeat();
        g.m_Seat = seat;
        float x;
        float w;
        DTTopPlate(i, n, family == "th", x, w);
        g.m_PlateX = x;
        g.m_PlateY = 160;
        g.m_PlateW = w;
        if (family == "th")
        {
            float k = 1.5;
            if (n == 3)
                k = 1.25;
            if (n == 4)
                k = 0.9;
            if (n == 5)
                k = 0.72;
            if (n == 6)
                k = 0.6;
            if (n >= 7)
                k = 0.5;
            float s = k / 1.5;
            g.m_CanK = k;
            g.m_CanY = 282;
            g.m_RowStep = 50 * s;
            // three along the top: the middle seat's row of five and the mirrored right
            // one's lie side by side, a little closer each
            if (n == 3)
                g.m_RowStep = 46 * s;
            g.m_RowScale = 0.5 * s;
            g.m_AsideScale = 0.42 * s;
            // the seat at the felt's right end is the canvas's mirrored one: its can at the
            // plate's right, its dice to the left of it (a row of five would leave the felt)
            bool mirrored = n >= 2 && i == n - 1;
            if (!mirrored)
            {
                g.m_CanX = x + 10 * s;
                g.m_MouthX = g.m_CanX + 134 * s;
                g.m_Tip = 108;
                g.m_HandX = g.m_CanX + 30 * s;
                g.m_AsideX = g.m_CanX + 138 * s;
                g.m_AsideStep = 36 * s;
                g.m_RowX = g.m_CanX + 150 * s;
            }
            else
            {
                g.m_CanX = x + w - 10 * s - 76 * k;
                g.m_MouthX = g.m_CanX - 20 * s;
                g.m_Tip = -108;
                g.m_HandX = g.m_CanX + 70 * s;
                g.m_AsideX = g.m_CanX - 24 * s;
                g.m_AsideStep = -36 * s;
                // its row ends by the can (the canvas: its fifth die 34 from it); with
                // three along the top it lies nearer, clear of the middle seat's row
                float byCan = 34 * s;
                if (n == 3)
                    byCan = 22 * s;
                g.m_RowX = g.m_CanX - byCan - 4 * g.m_RowStep;
            }
            g.m_MouthY = g.m_CanY + 50 * s;
            g.m_HandY = g.m_CanY - 38 * s;
            g.m_AsideY = g.m_CanY + 104 * s;
            g.m_RowY = g.m_CanY + 74 * s;
            return g;
        }
        float kk = 1.25;
        if (n == 4)
            kk = 1.0;
        if (n == 5)
            kk = 0.85;
        if (n == 6)
            kk = 0.72;
        if (n >= 7)
            kk = 0.6;
        float ss = kk / 1.25;
        g.m_CanK = kk;
        g.m_Tip = 108;
        if (family == "hr")
        {
            g.m_CanX = x;
            g.m_CanY = 252;
            g.m_MouthX = g.m_CanX + 106 * ss;
            g.m_MouthY = g.m_CanY + 48 * ss;
            g.m_HandX = g.m_CanX + 30 * ss;
            g.m_HandY = g.m_CanY - 26 * ss;
            g.m_RowX = g.m_CanX + 138 * ss;
            g.m_RowY = g.m_CanY + 66 * ss;
            g.m_RowStep = 58 * ss;
            // the canvas's last seat of three keeps its third die off the felt's edge
            if (n == 3 && i == 2)
                g.m_RowStep = 56;
            g.m_RowScale = 0.667 * ss;
            return g;
        }
        g.m_CanX = x + 10 * ss;
        g.m_CanY = 250;
        g.m_MouthX = g.m_CanX + 38 * kk;
        g.m_MouthY = g.m_CanY + 56 * kk;
        g.m_HandX = g.m_CanX + 30 * ss;
        g.m_HandY = g.m_CanY - 24 * ss;
        g.m_RowX = g.m_CanX + 20 * ss;
        g.m_RowY = g.m_CanY + 146 * ss;
        g.m_RowStep = 44 * ss;
        g.m_RowScale = 0.513 * ss;
        return g;
    }

    // The seats of the game round the felt: this player's own first when he has one, then
    // the others in their order from his on. `others`: how many lie along the top.
    protected void DGLaySeats(string family, float meY, out int others)
    {
        m_DGGeo.Clear();
        others = 0;
        if (!m_GPub)
            return;
        int n = m_GPub.SeatList.Count();
        int mineAt = -1;
        int i;
        for (i = 0; i < n; i++)
        {
            OZG_GSeat g = m_GPub.SeatList[i];
            if (g && g.SeatNo == m_V.Mine)
                mineAt = i;
        }
        others = n;
        if (mineAt >= 0)
        {
            others = n - 1;
            m_DGGeo.Insert(DGGeoMe(family, m_V.Mine, meY));
        }
        int at = 0;
        for (i = 1; i <= n; i++)
        {
            int j = i - 1;
            if (mineAt >= 0)
                j = (mineAt + i) % n;
            if (j == mineAt)
                continue;
            OZG_GSeat other = m_GPub.SeatList[j];
            if (!other)
                continue;
            int otherSeat = other.SeatNo;
            m_DGGeo.Insert(DGGeoTop(family, otherSeat, at, others));
            at++;
        }
    }

    protected OZG_DTSeat DGGeoOf(int seat)
    {
        if (!m_DGGeo)
            return null;
        for (int i = 0; i < m_DGGeo.Count(); i++)
        {
            if (m_DGGeo[i].m_Seat == seat)
                return m_DGGeo[i];
        }
        return null;
    }

    // The name of the plate a seat of the row laid has: this player's own "<face>Me", the
    // others' "<face>P0" .. along the top.
    protected string DGPlateOf(string prefix, int geoAt)
    {
        OZG_DTSeat at = m_DGGeo[geoAt];
        if (at.m_Seat == m_V.Mine)
            return prefix + "Me";
        int k = geoAt;
        if (m_DGGeo.Count() > 0 && m_DGGeo[0].m_Seat == m_V.Mine)
            k = geoAt - 1;
        return prefix + "P" + k.ToString();
    }

    // The plates of a face that no seat of this paint took are hidden.
    protected void DGPlatesHide(string prefix, int others, bool mine)
    {
        for (int i = others; i < 7; i++)
            BWShow(prefix + "P" + i.ToString(), false);
        if (!mine)
            BWShow(prefix + "Me", false);
    }

    // A plate at its seat: placed, `h` high, its ring.
    protected void DGPlateAt(string p, OZG_DTSeat at, float h, int ring, float ringW)
    {
        Widget plate = BW(p);
        if (!plate)
            return;
        plate.Show(true);
        plate.SetPos((at.m_PlateX - 52) * m_K, (at.m_PlateY - 86) * m_K);
        plate.SetSize(at.m_PlateW * m_K, h * m_K);
        DGBox(p + "Box", at.m_PlateW, h, 8, ring, ringW);
    }

    // A plate's name, cut to `room` units; on this player's own "(ви)" after it, lighter.
    protected void DGPlateName(string p, int seat, float room)
    {
        string name = SeatName(seat);
        bool me = seat == m_V.Mine;
        TextWidget you = BWText(p + "You");
        float youW = 0;
        if (you && me)
        {
            you.SetText(Tr("STR_OZG_BWIN_YOU"));
            youW = BWTextW(you) + 5;
        }
        float nw = DGNameIn(p + "Name", name, room - youW);
        if (you)
        {
            you.Show(me);
            Widget nameW = BW(p + "Name");
            float nx;
            float ny;
            if (nameW)
            {
                nameW.GetPos(nx, ny);
                you.SetPos(nx + (nw + 5) * m_K, ny);
            }
        }
    }

    // A text of a plate whose right edge lies `pad` from the plate's (its box is as the
    // layout made it, its words to the right in it).
    protected void DGRight(string name, float plateW, float pad, float y)
    {
        Widget t = BW(name);
        if (!t)
            return;
        float w;
        float h;
        t.GetSize(w, h);
        t.SetPos((plateW - pad) * m_K - w, y * m_K);
    }

    // The tags of a plate (the canvas: pills 22 high, 6 apart, wrapping): begun at x, y of
    // the plate with `room` units a row.
    protected void DGTagsBegin(string p, float x, float y, float room)
    {
        m_DGTagP = p;
        m_DGTagN = 0;
        m_DGTagX = x;
        m_DGTagX0 = x;
        m_DGTagY = y;
        m_DGTagRoom = room;
    }

    protected void DGTag(string text, int fill, int ink)
    {
        string name = m_DGTagP + "Tag" + m_DGTagN.ToString();
        Widget chip = BW(name);
        TextWidget t = BWText(name + "Text");
        if (!chip || !t)
            return;
        m_DGTagN++;
        t.SetText(text);
        float tw = BWTextW(t);
        float cw = tw + 16;
        if (cw > m_DGTagRoom)
            cw = m_DGTagRoom;
        if (m_DGTagX > m_DGTagX0 && m_DGTagX + cw > m_DGTagX0 + m_DGTagRoom)
        {
            m_DGTagX = m_DGTagX0;
            m_DGTagY = m_DGTagY + 28;
        }
        chip.Show(true);
        chip.SetPos(m_DGTagX * m_K, m_DGTagY * m_K);
        chip.SetSize(cw * m_K, 22 * m_K);
        t.SetSize((cw - 16) * m_K + 2, 15 * m_K);
        Widget bg = BW(name + "Bg");
        BWFit(bg, cw, 22, 11);
        BWPaint(bg, fill);
        t.SetColor(ink);
        m_DGTagX = m_DGTagX + cw + 6;
    }

    // The rest of the plate's tags hidden; where the tags end.
    protected float DGTagsEnd(int most)
    {
        for (int i = m_DGTagN; i < most; i++)
            BWShow(m_DGTagP + "Tag" + i.ToString(), false);
        if (m_DGTagN == 0)
            return m_DGTagY - 7;
        return m_DGTagY + 22;
    }

    // The canvas's tags: whose turn (green), a mark of the game (gold), a note (grey), whose
    // dice (faint).
    protected void DGTagTurn(bool me)
    {
        string text = Tr("STR_OZG_DWIN_TAG_THROWS");
        if (me)
            text = Tr("STR_OZG_DWIN_TAG_YOURS");
        DGTag(text, DGC(32, 36, 22), DGC(166, 217, 160));
    }

    protected void DGTagGold(string text)
    {
        DGTag(text, DGC(54, 41, 23), DGC(231, 194, 122));
    }

    protected void DGTagGrey(string text)
    {
        DGTag(text, DGC(40, 32, 23), DGC(210, 195, 164));
    }

    protected void DGTagWhose(int seat, bool one)
    {
        string key = "STR_OZG_DWIN_HR_HOST";
        if (DTOwnCan(seat))
            key = "STR_OZG_DWIN_HR_OWN";
        if (one)
        {
            key = "STR_OZG_DWIN_ONE_HOST";
            if (DTOwnCan(seat))
                key = "STR_OZG_DWIN_ONE_OWN";
        }
        DGTag(Tr(key), DGC(31, 24, 17), DGC(184, 165, 133));
    }

    // ------------------------------------------------------------ dice and cans on the felt

    // A die of a seat lying on the table: its face, as it was thrown (its look then, what
    // this player knows of that die), its middle (the window's units), how big it lies
    // (`nominal` at full size, times `scale`), its turn, its alpha.
    protected OZG_DTDie DGDieLie(int seat, int place, int face, float nominal, float cx, float cy, float scale, float rot, float alpha, bool marks)
    {
        OZG_DTDie d = DTDieOf(DTDieId(seat, place));
        if (!d)
            return null;
        string look = DTLookAt(seat, place, true);
        DTDieFace(d, face, look, nominal, false);
        int mark = 0;
        if (marks)
            mark = DTMarkAt(seat, place, true);
        DTDieMarked(d, mark, 0, false);
        DTDieGo(d, cx, cy, nominal * scale, rot, alpha, 0);
        if (d.m_Root)
            d.m_Root.SetSort(30);
        return d;
    }

    // A die of a seat in its can: at the can's mouth, small, unseen. A die made by this
    // paint is there at once; one that lay on the table flies back.
    protected OZG_DTDie DGDieHome(int seat, int place, float nominal, OZG_DTSeat at, float scale)
    {
        OZG_DTDie d = DTDieOf(DTDieId(seat, place));
        if (!d || !at)
            return d;
        if (d.m_Fresh)
        {
            string look = DTLookAt(seat, place, false);
            DTDieFace(d, 0, look, nominal, false);
        }
        DTDieGo(d, at.m_MouthX, at.m_MouthY, nominal * scale, 0, 0, 0);
        return d;
    }

    // Does any die of a seat lie outside its can (as the last paint laid them)?
    protected bool DGSeatOut(int seat)
    {
        if (!m_DTDice || seat < 0)
            return false;
        string head = "s" + seat.ToString() + "_";
        for (int i = 0; i < m_DTDice.Count(); i++)
        {
            OZG_DTDie d = m_DTDice[i];
            if (d.m_Id.IndexOf(head) == 0 && d.m_Alpha > 0.5)
                return true;
        }
        return false;
    }

    override protected bool DTAnyOut(int seat)
    {
        for (int i = 0; i < m_DTThrowAt.Count(); i++)
        {
            int place = m_DTThrowAt[i];
            OZG_DTDie d = DTDieFind(DTDieId(seat, place));
            if (d && d.m_Alpha > 0.5)
                return true;
        }
        return false;
    }

    // A seat's can's mouth on the table laid (a die of its that lies nowhere flies there).
    override protected bool DTHomeOf(int seat, out float x, out float y)
    {
        if (m_Style != "dice")
            return super.DTHomeOf(seat, x, y);
        x = 0;
        y = 0;
        OZG_DTSeat at = DGGeoOf(seat);
        if (!at || !m_DGActive)
            return false;
        x = at.m_MouthX;
        y = at.m_MouthY;
        return true;
    }

    // The can at a seat, in a state; with the dice `looks` lying in it and, `glow`, the
    // ring of whose throw it is.
    protected OZG_DTCan DGCanAt(int seat, OZG_DTSeat at, string state, array<string> looks, bool glow)
    {
        if (!at)
            return null;
        OZG_DTCan c = DTCanOf(DTCanId(seat));
        DTCanSet(c, at.m_CanX, at.m_CanY, at.m_CanK, state, at.m_Tip, DTArmy(seat), looks, glow);
        return c;
    }

    // Where the table's one can stands at another's throw: a faint oval on the felt (the
    // canvas: 98 x 28 at 8, 108 of a can of 114; smaller with the can).
    protected void DGSpot(OZG_DTSeat at)
    {
        if (!at)
            return;
        float s = at.m_CanK / 1.5;
        float x = at.m_CanX + 7 * s - 52;
        float y = at.m_CanY + 107 * s - 86;
        BWPic(BW_FIELD, BW_TEX + "ozg_seat_spot_ca.paa", x, y, 100 * s, 30 * s, 2, ARGB(255, 255, 255, 255));
    }

    // The seats that throw the table's dice, of the seats laid.
    protected void DGHostSeats(array<int> into)
    {
        into.Clear();
        for (int i = 0; i < m_DGGeo.Count(); i++)
        {
            int seat = m_DGGeo[i].m_Seat;
            if (!DTOwnCan(seat))
                into.Insert(seat);
        }
    }

    // This player's own can takes a click: `move` is made (a throw).
    protected void DGCanClick(OZG_DTSeat at, string move)
    {
        if (!at)
            return;
        m_DWCanX = at.m_CanX;
        m_DWCanY = at.m_CanY;
        m_DWCanW = 76 * at.m_CanK;
        m_DWCanH = 92 * at.m_CanK;
        m_DWCanMove = move;
    }

    // The buttons of the can and dice and of the swap in the panel that lies at `panelY`
    // of the column, `h` down it (what the panel holds ends 10 above), one under the other
    // 10 apart, each there while it has something to offer; the panel's height with them
    // (38 each and 12 under the last) or without (its bottom's 12).
    protected float DGSwapRow(float panelY, float h, bool live)
    {
        float y = h;
        bool picks = DTPickButton(24, panelY + y, 336, live);
        if (picks)
            y = y + 48;
        bool swaps = DTSwapButton(24, panelY + y, 336, live);
        if (swaps)
            y = y + 48;
        return y + 2;
    }

    // ------------------------------------------------------------ thousand

    // A seat's numbers at thousand: its total, its bolts in a row, its tries on the barrel;
    // whether it stands on the barrel.
    protected void DGThSeat(OZG_GSeat g, out int total, out int bolts, out int tries, out bool barrel)
    {
        total = 0;
        bolts = 0;
        tries = 0;
        barrel = false;
        if (!g)
            return;
        string line = g.Line;
        total = DGArgInt(line, 0);
        bolts = DGArgInt(line, 1);
        tries = DGArgInt(line, 2);
        if (total < -100000)
            total = 0;
        if (bolts < 0)
            bolts = 0;
        if (tries < 0)
            tries = 0;
        barrel = line.IndexOf("STR_OZG_THOUSAND_SEAT_BARREL") == 0;
    }

    protected bool DGThPit(int total)
    {
        if (DGOption(OZG_ThousandConst.OPT_PITS, "on") != "on")
            return false;
        bool low = total >= 200 && total <= 300;
        bool high = total >= 600 && total <= 700;
        return low || high;
    }

    // The canvas's five spots for dice thrown onto the felt (their middles, the turn).
    protected void DGThSpot(int i, out float x, out float y, out float rot)
    {
        x = 577;
        y = 447;
        rot = -3;
        if (i == 0)
        {
            x = 257;
            y = 457;
            rot = -8;
        }
        else if (i == 1)
        {
            x = 417;
            y = 487;
            rot = 6;
        }
        else if (i == 3)
        {
            x = 737;
            y = 497;
            rot = 9;
        }
        else if (i >= 4)
        {
            x = 897;
            y = 462;
            rot = -6;
        }
    }

    protected void DGPaintThousand()
    {
        bool playing = DWPlaying();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        bool busy = DTBusy();
        int mineNo = m_V.Mine;
        int i;
        int k;
        // what lies where: the dice of the throw, the ones put aside, the rest that lies
        array<int> thrown = new array<int>();
        array<int> thrownAt = new array<int>();
        array<int> aside = new array<int>();
        array<int> asideAt = new array<int>();
        array<int> lie = new array<int>();
        array<int> lieAt = new array<int>();
        int thrower = -1;
        OZG_GZone zThrow = DGZoneOf("throw");
        if (zThrow)
        {
            DWDiceOf(zThrow.Line, thrown);
            if (zThrow.DicePlaces)
                thrownAt.Copy(zThrow.DicePlaces);
            thrower = zThrow.DiceSeat;
        }
        OZG_GZone zAside = DGZoneOf("aside");
        if (zAside)
        {
            DWDiceOf(zAside.Line, aside);
            if (zAside.DicePlaces)
                asideAt.Copy(zAside.DicePlaces);
        }
        OZG_GZone zLie = DGZoneOf("lie");
        if (zLie)
        {
            DWDiceOf(zLie.Line, lie);
            if (zLie.DicePlaces)
                lieAt.Copy(zLie.DicePlaces);
        }
        m_DWDice.Copy(thrown);
        int turnPts = 0;
        OZG_GZone zTurn = DGZoneOf("turn");
        if (zTurn)
            turnPts = DGArgInt(zTurn.Line, 0);
        if (turnPts < 0)
            turnPts = 0;
        // whose dice are on the table: the seat's in turn; while a throw the view has no
        // more on the table is seen (a bolt: the turn has passed), the one's who threw
        bool flown = m_DTFx == "fly" || m_DTFx == "linger";
        bool bolt = flown && thrown.Count() == 0 && m_DTThrower >= 0;
        int shown = turn;
        if (shown < 0)
            shown = thrower;
        if (bolt)
            shown = m_DTThrower;
        // and whose turn the plates and the can's ring show: his, until his dice go
        int turnShown = turn;
        if (bolt)
            turnShown = m_DTThrower;
        // this player's move: pick dice to put aside, or throw (and maybe write)
        bool hasSet = false;
        bool hasThrow = false;
        bool hasWrite = false;
        array<string> sets = new array<string>();
        if (m_GOwn && mine)
        {
            for (i = 0; i < m_GOwn.Buttons.Count(); i++)
            {
                OZG_GButton b = m_GOwn.Buttons[i];
                if (!b)
                    continue;
                string bm = b.Move;
                if (bm.IndexOf(OZG_ThousandConst.M_SET) == 0)
                {
                    hasSet = true;
                    sets.Insert(bm);
                }
                if (bm == OZG_DiceGameConst.M_THROW)
                    hasThrow = true;
                if (bm == OZG_ThousandConst.M_WRITE)
                    hasWrite = true;
            }
        }
        bool picking = mine && hasSet && !busy;
        if (!picking)
            m_DWPicked.Clear();
        for (i = m_DWPicked.Count() - 1; i >= 0; i--)
        {
            if (m_DWPicked[i] >= thrown.Count())
                m_DWPicked.Remove(i);
        }
        bool canThrow = mine && hasThrow && !busy;
        bool canWrite = mine && hasWrite && !busy;
        int toThrow = 5 - aside.Count();
        if (toThrow <= 0)
            toThrow = 5;

        // ---- the seats, their plates
        int others;
        DGLaySeats("th", 696, others);
        array<string> notes = new array<string>();
        for (i = 0; i < m_GPub.SeatList.Count(); i++)
        {
            int noteTotal;
            int noteBolts;
            int noteTries;
            bool noteBarrel;
            OZG_GSeat noteSeat = m_GPub.SeatList[i];
            DGThSeat(noteSeat, noteTotal, noteBolts, noteTries, noteBarrel);
            string note = "";
            if (noteBarrel)
                note = Tr("STR_OZG_DWIN_TH_NOTE_BARREL");
            else if (DGThPit(noteTotal))
                note = Tr("STR_OZG_DWIN_TH_PIT");
            else if (noteBolts > 0)
                note = Tr1("STR_OZG_DWIN_TH_NOTE_BOLTS", noteBolts.ToString());
            notes.Insert(note);
        }
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat at = m_DGGeo[i];
            int seat = at.m_Seat;
            string p = DGPlateOf("OZG_DTh", i);
            OZG_GSeat g = DGSeat(seat);
            int total;
            int bolts;
            int tries;
            bool barrel;
            DGThSeat(g, total, bolts, tries, barrel);
            bool me = seat == mineNo;
            bool current = seat == turnShown;
            float pw = at.m_PlateW;
            float totalW = DGText(p + "Total", total.ToString());
            DGRight(p + "Total", pw, 12, 10);
            DGPlateName(p, seat, pw - 24 - totalW - 10);
            DGTagsBegin(p, 12, 49, pw - 24);
            if (barrel)
            {
                int tryNo = tries + 1;
                DGTagGold(Tr1("STR_OZG_DWIN_TH_BARREL", tryNo.ToString()));
            }
            else if (DGThPit(total))
            {
                DGTagGrey(Tr("STR_OZG_DWIN_TH_PIT"));
            }
            if (bolts > 0)
                DGTagGrey(Tr1("STR_OZG_DWIN_TH_BOLTS", bolts.ToString()));
            if (current)
                DGTagTurn(me);
            DGTagWhose(seat, false);
            float bottom = DGTagsEnd(5);
            // the dice it threw last, each as it was thrown (not while they lie on the felt)
            Widget row = BW(p + "Dice");
            int lastN = 0;
            bool lastShown = false;
            if (g && !me && row)
            {
                lastN = g.Faces.Count();
                if (lastN > 5)
                    lastN = 5;
                lastShown = lastN > 0 && !current && seat != shown;
                row.Show(lastShown);
                row.SetPos(12 * m_K, (bottom + 7) * m_K);
                for (k = 0; k < 5; k++)
                {
                    OZG_DTDie mini = DTDieFind("L" + seat.ToString() + "_" + k.ToString());
                    bool has = lastShown && k < lastN;
                    if (!mini && has)
                    {
                        mini = DTDieMake("L" + seat.ToString() + "_" + k.ToString(), row);
                        if (mini)
                            m_DTDice.Insert(mini);
                    }
                    if (!mini)
                        continue;
                    mini.m_Used = has;
                    if (!has)
                        continue;
                    string lastFace = g.Faces[k];
                    int lastPlace = k;
                    if (g.FacePlaces && k < g.FacePlaces.Count())
                        lastPlace = g.FacePlaces[k];
                    string lastLook = DTLookAt(seat, lastPlace, true);
                    int lastMark = DTMarkAt(seat, lastPlace, true);
                    DTDieFace(mini, lastFace.ToInt(), lastLook, 26, false);
                    DTDieMarked(mini, lastMark, 0, false);
                    mini.m_Fresh = true;
                    DTDieGoIn(mini, 13 + k * 32, 13, 26, 0, 1, 0);
                }
                TextWidget lastNote = BWText(p + "Note");
                if (lastNote)
                {
                    lastNote.SetText(Tr("STR_OZG_DWIN_LAST"));
                    float noteX = 12 + lastN * 32;
                    bool fits = noteX + BWTextW(lastNote) <= pw - 12;
                    lastNote.Show(lastShown && fits);
                    lastNote.SetPos(noteX * m_K, (bottom + 7 + 5.5) * m_K);
                }
                if (lastShown)
                    bottom = bottom + 7 + 26;
            }
            int ring = DGC(74, 55, 38);
            if (me)
                ring = DGC(107, 82, 51);
            if (current)
                ring = DGC(201, 164, 92);
            m_DGSnug = false;
            DGPlateAt(p, at, bottom + 10, ring, 1);
        }
        DGPlatesHide("OZG_DTh", others, mineNo >= 0 && DGGeoOf(mineNo) != null);

        // ---- the felt: whose throw, "ЗА ХІД", "ВІДКЛАДЕНО"
        string feltHint = "";
        bool inCanAll = thrown.Count() == 0 && lie.Count() == 0 && aside.Count() == 0;
        if (playing && !busy && inCanAll && turn >= 0)
        {
            feltHint = Tr1("STR_OZG_DWIN_THROWS", SeatName(turn));
            if (mine)
                feltHint = Tr("STR_OZG_DWIN_YOUR_THROW");
        }
        DGFeltHint(feltHint, 448);
        DGTurnPill("OZG_DThPill", turnPts);

        // ---- the dice of the seat whose throw lies on the table
        OZG_DTSeat sat = DGGeoOf(shown);
        array<string> inCan = new array<string>();
        int asideLies = aside.Count();
        if (sat)
        {
            for (k = 0; k < 5; k++)
            {
                string id = DTDieId(shown, k);
                if (DTHeld(shown, k))
                {
                    DGDieHome(shown, k, 74, sat, 0.35);
                    inCan.Insert(DTLookAt(shown, k, false));
                    if (asideAt.Find(k) >= 0)
                        asideLies--;
                    continue;
                }
                float fx;
                float fy;
                float fr;
                if (bolt)
                {
                    int flownFace = DTFlown(shown, k);
                    if (flownFace > 0)
                    {
                        DGThSpot(m_DTThrowAt.Find(k), fx, fy, fr);
                        DGDieLie(shown, k, flownFace, 74, fx, fy, 1, fr, 1, true);
                        continue;
                    }
                    // what was put aside before the bolt lies there until the dice go
                    OZG_DTDie kept = DTDieFind(id);
                    if (kept && kept.m_ToAlpha > 0.5)
                    {
                        kept.m_Used = true;
                        asideLies++;
                        continue;
                    }
                    DGDieHome(shown, k, 74, sat, 0.35);
                    continue;
                }
                int a = asideAt.Find(k);
                if (a >= 0 && a < aside.Count())
                {
                    float ax = sat.m_AsideX + a * sat.m_AsideStep;
                    DGDieLie(shown, k, aside[a], 74, ax, sat.m_AsideY, sat.m_AsideScale, 0, 1, true);
                    continue;
                }
                int t = thrownAt.Find(k);
                if (t >= 0 && t < thrown.Count())
                {
                    DGThSpot(t, fx, fy, fr);
                    OZG_DTDie lying = DGDieLie(shown, k, thrown[t], 74, fx, fy, 1, fr, 1, true);
                    bool on = picking && m_DWPicked.Find(t) >= 0;
                    if (lying && on)
                    {
                        int mark = DTMarkAt(shown, k, true);
                        DTDieMarked(lying, mark, DGC(231, 194, 122), true);
                        DTDieGo(lying, fx, fy, 74, fr, 1, -14);
                        if (lying.m_Root)
                            lying.m_Root.SetSort(31);
                    }
                    continue;
                }
                int l = lieAt.Find(k);
                if (l >= 0 && l < lie.Count())
                {
                    // the rest of the throw lies where it fell
                    OZG_DTDie rest = DTDieFind(id);
                    if (rest && !rest.m_Fresh && rest.m_ToAlpha > 0.5)
                    {
                        rest.m_Used = true;
                        DTDieMarked(rest, DTMarkAt(shown, k, true), 0, false);
                        DTDieGoIn(rest, rest.m_ToX, rest.m_ToY, 74, rest.m_ToRot, 1, 0);
                        continue;
                    }
                    DGThSpot(l, fx, fy, fr);
                    DGDieLie(shown, k, lie[l], 74, fx, fy, 1, fr, 1, true);
                    continue;
                }
                DGDieHome(shown, k, 74, sat, 0.35);
                inCan.Insert(DTLookAt(shown, k, false));
            }
        }
        BWShow("OZG_DThAside", shown == mineNo && asideLies > 0 && mineNo >= 0);
        BWSetText("OZG_DThAside", OZG_Text.Upper(Tr("STR_OZG_THOUSAND_ASIDE")));
        // the dice he may pick: the throw's, by their numbers in it
        if (picking)
        {
            m_DWDieHit = 40;
            for (k = 0; k < thrown.Count(); k++)
            {
                float px;
                float py;
                float pr;
                DGThSpot(k, px, py, pr);
                m_DWDieX.Insert(px);
                m_DWDieY.Insert(py);
            }
        }

        // ---- the cans: its own at every seat that throws its own dice, the table's one
        // at the seat that throws
        array<int> hostSeats = new array<int>();
        DGHostSeats(hostSeats);
        int want = m_DTHostAt;
        if (shown >= 0 && hostSeats.Find(shown) >= 0)
            want = shown;
        if (hostSeats.Find(want) < 0)
        {
            want = -1;
            if (hostSeats.Count() > 0)
                want = hostSeats[0];
        }
        int hostAt = -1;
        if (hostSeats.Count() > 0)
            hostAt = DTHostSeat(want, DGSeatOut(m_DTHostAt));
        array<string> full = new array<string>();
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat canAt = m_DGGeo[i];
            int canSeat = canAt.m_Seat;
            bool own = DTOwnCan(canSeat);
            if (!own && canSeat != hostAt)
            {
                DGSpot(canAt);
                continue;
            }
            string state = DTCanFx(canSeat);
            array<string> looks = full;
            if (canSeat == shown)
            {
                looks = inCan;
                if (state == "")
                {
                    state = "empty";
                    if (inCan.Count() > 0)
                        state = "full";
                }
            }
            else
            {
                full.Clear();
                for (k = 0; k < 5; k++)
                    full.Insert(DTLookAt(canSeat, k, false));
                state = "full";
            }
            bool glow = playing && canSeat == turnShown;
            DGCanAt(canSeat, canAt, state, looks, glow);
            if (canSeat == mineNo && canThrow)
                DGCanClick(canAt, OZG_DiceGameConst.M_THROW);
        }

        // ---- the right column: the score, the panel, the swap, the log
        DWPaintScore(notes);
        float top = 12 + m_DWScoreH + 10;
        BWShow("OZG_DThAct", playing);
        float bottomY = top;
        if (!playing)
        {
            bottomY = DGTableActs(top);
            DTSwapButton(24, 0, 336, false);
        }
        else
        {
            array<int> pickedFaces = new array<int>();
            array<int> pickedAt = new array<int>();
            pickedAt.Copy(m_DWPicked);
            pickedAt.Sort();
            string asideMove = OZG_ThousandConst.M_SET;
            for (i = 0; i < pickedAt.Count(); i++)
            {
                int at2 = pickedAt[i];
                if (i > 0)
                    asideMove = asideMove + ",";
                int place = at2 + 1;
                asideMove = asideMove + place.ToString();
                if (at2 >= 0 && at2 < thrown.Count())
                    pickedFaces.Insert(thrown[at2]);
            }
            int pickPts = -1;
            if (pickedFaces.Count() > 0)
                pickPts = OZG_Thousand.ThPoints(pickedFaces);
            pickedFaces.Sort();
            string pickedKey = DWFacesKey(pickedFaces);
            bool asideOk = false;
            for (i = 0; i < sets.Count() && pickedAt.Count() > 0; i++)
            {
                array<int> optionFaces = new array<int>();
                DWSetFaces(sets[i], optionFaces);
                optionFaces.Sort();
                if (DWFacesKey(optionFaces) == pickedKey)
                    asideOk = true;
            }
            string title = Tr("STR_OZG_DWIN_YOUR_THROW");
            string hint = "";
            string turnName = SeatName(turn);
            if (!mine)
            {
                title = Tr1("STR_OZG_DWIN_TURN", turnName);
                hint = Tr1("STR_OZG_DWIN_TH_HINT_HOST", turnName);
                if (DTOwnCan(turn))
                    hint = Tr1("STR_OZG_DWIN_TH_HINT_OWN", turnName);
            }
            else if (hasSet)
            {
                title = Tr("STR_OZG_DWIN_TH_PICK_TITLE");
                hint = Tr("STR_OZG_DWIN_TH_HINT_PICK");
                if (pickedAt.Count() > 0 && asideOk)
                    hint = Tr1("STR_OZG_DWIN_TH_HINT_GIVES", pickPts.ToString());
                else if (pickedAt.Count() > 0)
                    hint = Tr("STR_OZG_DWIN_TH_HINT_BAD");
            }
            else if (turnPts > 0 || aside.Count() > 0)
            {
                title = Tr("STR_OZG_DWIN_TH_CHOOSE_TITLE");
                hint = Tr2("STR_OZG_DWIN_TH_HINT_CHOOSE", turnPts.ToString(), toThrow.ToString());
                if (toThrow == 1)
                    hint = Tr1("STR_OZG_DWIN_TH_HINT_CHOOSE_ONE", turnPts.ToString());
                int mineTotal;
                int mineBolts;
                int mineTries;
                bool mineBarrel;
                DGThSeat(DGSeat(mineNo), mineTotal, mineBolts, mineTries, mineBarrel);
                if (!mineBarrel && DGThPit(mineTotal + turnPts))
                    hint = hint + " " + Tr("STR_OZG_DWIN_TH_HINT_PIT");
            }
            else
            {
                hint = Tr("STR_OZG_DWIN_TH_HINT_THROW");
            }
            if (bolt && m_DTThrower == mineNo)
            {
                title = Tr("STR_OZG_DWIN_TH_BOLT_TITLE");
                hint = Tr("STR_OZG_DWIN_TH_HINT_BOLT");
            }
            else if (bolt)
            {
                title = Tr1("STR_OZG_DWIN_TURN", SeatName(m_DTThrower));
            }
            BWSetText("OZG_DThTitle", title);
            string leftText = toThrow.ToString();
            BWSetText("OZG_DThLeft", Tr1("STR_OZG_DWIN_TH_IN_THROW", leftText));
            int lines = DGLinesN("OZG_DThHint", hint, 336, 3, 2);
            float y = 44 + lines * 19 + 10;
            string asideLabel = Tr("STR_OZG_DWIN_TH_ASIDE");
            int asideFill = DGC(51, 37, 26);
            int asideInk = DGC(160, 143, 115);
            if (picking && asideOk)
            {
                asideLabel = Tr1("STR_OZG_DWIN_TH_ASIDE_PTS", pickPts.ToString());
                asideFill = DGC(201, 164, 92);
                asideInk = DGC(26, 18, 10);
                m_DGMoveC = asideMove;
            }
            DGButton("OZG_DThAsideBtn", asideLabel, asideFill, DGC(107, 82, 51), asideInk);
            BWMove(BW("OZG_DThAsideBtn"), 12, y);
            y = y + 42 + 10;
            int throwFill = DGC(51, 37, 26);
            int throwInk = DGC(160, 143, 115);
            if (canThrow)
            {
                throwFill = DGC(201, 164, 92);
                throwInk = DGC(26, 18, 10);
                m_DGMoveA = OZG_DiceGameConst.M_THROW;
            }
            DGButton("OZG_DThThrow", Tr1("STR_OZG_DWIN_HR_THROW", toThrow.ToString()), throwFill, throwFill, throwInk);
            BWMove(BW("OZG_DThThrow"), 12, y);
            int writeFill = DGC(36, 26, 18);
            int writeInk = DGC(160, 143, 115);
            int writeRing = DGC(61, 45, 29);
            if (canWrite)
            {
                writeFill = DGC(45, 33, 23);
                writeInk = DGC(239, 228, 204);
                writeRing = DGC(201, 164, 92);
                m_DGMoveB = OZG_ThousandConst.M_WRITE;
            }
            DGButton("OZG_DThWrite", Tr1("STR_OZG_DWIN_TH_WRITE", turnPts.ToString()), writeFill, writeRing, writeInk);
            BWMove(BW("OZG_DThWrite"), 184, y);
            y = y + 44 + 10;
            float h = DGSwapRow(top, y, true);
            DGPanel("OZG_DThAct", top, h, mine);
            bottomY = top + h;
        }
        DGLog(bottomY + 10, 762);
    }

    // ------------------------------------------------------------ dice poker

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
        bool busy = DTBusy();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        int mineNo = m_V.Mine;
        m_DGPhase = phase;
        m_DGMine = mine;
        int i;
        int k;
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
            if (open.Count() > 2)
                open[2].Split(",", openFolded);
        }
        bool picking = playing && mine && phase == 2 && !reveal && !busy;
        if (!picking)
            m_DWPicked.Clear();
        m_DTBlocked = reveal;

        // ---- the seats: the plates, the dice, the cans
        int others;
        DGLaySeats("th", 694, others);
        array<int> hostSeats = new array<int>();
        DGHostSeats(hostSeats);
        // the table's can stands at the seat that throws next (the throw, the throw again);
        // while the stakes are raised it stays where it threw last
        bool throwing = playing && !reveal && (phase == 0 || phase == 2);
        int want = m_DTHostAt;
        if (throwing && turn >= 0 && hostSeats.Find(turn) >= 0)
            want = turn;
        if (hostSeats.Find(want) < 0)
        {
            want = -1;
            if (hostSeats.Count() > 0)
                want = hostSeats[0];
        }
        int hostAt = -1;
        if (hostSeats.Count() > 0)
            hostAt = DTHostSeat(want, false);
        float rots[5] = {-6.0, 4.0, -2.0, 7.0, -4.0};
        array<int> myDice = new array<int>();
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat at = m_DGGeo[i];
            int seat = at.m_Seat;
            string p = DGPlateOf("OZG_DP", i);
            OZG_GSeat g = DGSeat(seat);
            if (!g)
                continue;
            string gLine = g.Line;
            string gRole = g.Role;
            bool me = seat == mineNo;
            bool current = seat == turn && playing && !reveal;
            array<int> dice = new array<int>();
            DWDiceOf(gRole, dice);
            bool folded = g.Out == 1;
            if (reveal)
            {
                // as the round lay: its dice, and whether it had folded
                DGOpenDice(open, 3, seat, dice);
                string seatText = seat.ToString();
                folded = openFolded.Find(seatText) >= 0;
            }
            if (me)
                myDice.Copy(dice);
            int chips = DGArgInt(gLine, 0);
            int wins = DGArgInt(gLine, 1);
            int put = DGArgInt(gLine, 2);
            float pw = at.m_PlateW;
            float chipsW = DGText(p + "Chips", Tr1("STR_OZG_DICE_POT_LINE", chips.ToString()));
            DGRight(p + "Chips", pw, 12, 12);
            float winsX = pw - 12 - chipsW - 10 - 29;
            DGPip(p + "Win0", wins >= 1, winsX, 14);
            DGPip(p + "Win1", wins >= 2, winsX + 17, 14);
            DGPlateName(p, seat, winsX - 10 - 12);
            bool thrownAll = dice.Count() >= 5;
            string hand = Tr("STR_OZG_DWIN_NOT_THROWN");
            int handInk = DGC(143, 125, 98);
            if (thrownAll)
            {
                hand = DGHandName(dice);
                handInk = DGC(239, 228, 204);
            }
            else if (folded)
            {
                hand = "";
            }
            BWSetText(p + "Hand", hand);
            BWInk(p + "Hand", handInk);
            string state = "";
            int stateInk = DGC(184, 165, 133);
            if (folded)
            {
                state = Tr("STR_OZG_DWIN_DP_FOLDED");
            }
            else if (DTCanFx(seat) != "" || (m_DTFx == "gather" && seat == m_DTThrower))
            {
                state = Tr("STR_OZG_DWIN_TAG_THROWS");
                stateInk = DGC(166, 217, 160);
            }
            else if (put > 0)
            {
                state = Tr1("STR_OZG_DWIN_DP_PUT", put.ToString());
            }
            BWSetText(p + "State", state);
            BWInk(p + "State", stateInk);
            DGRight(p + "State", pw, 12, 44.5);
            int ring = DGC(107, 82, 51);
            Widget plate = BW(p);
            if (folded)
            {
                ring = DGC(61, 45, 29);
                DGDim(plate, p, 0.63, 0.7);
            }
            else
            {
                DGDim(plate, p, 1, 1);
            }
            if (current || seat == m_DTThrower && m_DTFx != "" && m_DTFx != "home")
                ring = DGC(201, 164, 92);
            m_DGSnug = folded;
            DGPlateAt(p, at, 73, ring, 1);
            m_DGSnug = false;

            // its five dice: lying in its row, or in its can (the table's dice are where
            // the can is: another seat's row is what it threw with them, dimmed -- but for
            // the throw made last, which lies as it fell until the next one, for this
            // player's own hand, which he picks from, and at the showdown, where every
            // hand is looked at)
            bool own = DTOwnCan(seat);
            bool ghost = !own && seat != hostAt && seat != m_GPub.ThrowSeat && !me && !reveal;
            if (m_GPub.Ended == 1)
                ghost = false;
            float alpha = 1;
            if (ghost)
                alpha = 0.55;
            if (folded)
                alpha = 0.45;
            array<string> inCan = new array<string>();
            for (k = 0; k < 5; k++)
            {
                bool lies = k < dice.Count() && !DTHeld(seat, k);
                if (!lies)
                {
                    DGDieHome(seat, k, 84, at, 0.3);
                    inCan.Insert(DTLookAt(seat, k, false));
                    continue;
                }
                float dx = at.m_RowX + k * at.m_RowStep;
                float dy = at.m_RowY;
                float rot = rots[k];
                OZG_DTDie d = DGDieLie(seat, k, dice[k], 84, dx, dy, at.m_RowScale, rot, alpha, !ghost);
                bool on = me && picking && m_DWPicked.Find(k) >= 0;
                if (d && on)
                {
                    int mark = DTMarkAt(seat, k, true);
                    DTDieMarked(d, mark, DGC(231, 194, 122), true);
                    DTDieGo(d, dx, dy, 84 * at.m_RowScale, rot, alpha, -18);
                    if (d.m_Root)
                        d.m_Root.SetSort(31);
                }
            }
            if (!own && seat != hostAt)
            {
                DGSpot(at);
                continue;
            }
            string canState = DTCanFx(seat);
            if (canState == "")
            {
                canState = "empty";
                if (inCan.Count() > 0)
                    canState = "full";
            }
            bool glow = seat == m_DTThrower && m_DTFx != "" && m_DTFx != "home";
            if (current && throwing)
                glow = true;
            DGCanAt(seat, at, canState, inCan, glow);
            if (me && playing && mine && phase == 0 && !reveal && !busy)
                DGCanClick(at, OZG_DiceGameConst.M_THROW);
        }
        DGPlatesHide("OZG_DP", others, mineNo >= 0 && DGGeoOf(mineNo) != null);
        for (k = m_DWPicked.Count() - 1; k >= 0; k--)
        {
            if (m_DWPicked[k] >= myDice.Count())
                m_DWPicked.Remove(k);
        }
        // the dice he may pick to throw again: his own five
        OZG_DTSeat mySeat = DGGeoOf(mineNo);
        if (picking && mySeat)
        {
            m_DWDieHit = 46;
            for (k = 0; k < myDice.Count() && k < 5; k++)
            {
                m_DWDieX.Insert(mySeat.m_RowX + k * mySeat.m_RowStep);
                m_DWDieY.Insert(mySeat.m_RowY);
            }
        }
        DGPill("OZG_DPBank", DGPot(), 468, 474);

        // ---- the right column: the round, what he may do, the swap, the log
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
        // the round's box keeps room for two lines of words and grows for a third
        int rlines = DGLinesN("OZG_DPRHint", rhint, 336, 3, 2);
        float roundH = 94 + (rlines - 2) * 19;
        Widget roundBox = BW("OZG_DPRound");
        if (roundBox)
            roundBox.SetSize(360 * m_K, roundH * m_K);
        BWFit(BW("OZG_DPRoundBg"), 360, roundH, 6);
        float top = 12 + roundH + 10;
        float bottom = top;
        if (playing)
        {
            BWShow("OZG_DAct", true);
            BWShow("OZG_DActTitle", false);
            BWShow("OZG_DActInfo", false);
            BWShow("OZG_DAmount", false);
            BWMove(BW("OZG_DAct"), 12, top);
            int used = 0;
            int haveChips = 0;
            OZG_GSeat meSeat = DGSeat(mineNo);
            if (meSeat)
                haveChips = DGArgInt(meSeat.Line, 0);
            // nothing is asked of the game while a throw is still seen
            int gold = 0;
            int plain = 1;
            if (busy)
            {
                gold = 2;
                plain = 2;
            }
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
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_THROW"), OZG_DiceGameConst.M_THROW, gold, 12);
                used++;
            }
            else if (phase == 1 && raise > 0)
            {
                DGActBtn(used, Tr1("STR_OZG_DWIN_DP_CALL", raise.ToString()), OZG_DPokerConst.M_CALL, gold, 12);
                used++;
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_FOLD"), OZG_DPokerConst.M_FOLD, plain, 62);
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
                    DGActBtn(used, Tr1("STR_OZG_DWIN_DP_RAISE", up.ToString()), OZG_DPokerConst.M_RAISE + up.ToString(), gold, y1);
                    used++;
                    y1 = 62;
                }
                DGActBtn(used, Tr("STR_OZG_DWIN_DP_NO_RAISE"), OZG_DPokerConst.M_CHECK, plain, y1);
                used++;
            }
            else
            {
                string again = Tr("STR_OZG_DWIN_DP_KEEP");
                int againN = m_DWPicked.Count();
                if (againN > 0)
                    again = Tr1("STR_OZG_DWIN_DP_AGAIN", againN.ToString());
                DGActBtn(used, again, "#again", gold, 12);
                used++;
            }
            for (k = used; k < 16; k++)
                BWShow("OZG_DAct" + k.ToString(), false);
            float h = 12 + used * 50;
            float whole = DGSwapRow(top, h, true);
            Widget boxW = BW("OZG_DActBox");
            if (boxW)
                boxW.SetSize(360 * m_K, whole * m_K);
            BWFit(BW("OZG_DActBoxEdge"), 360, whole, 6);
            BWFit(BW("OZG_DActBoxBg"), 358, whole - 2, 5);
            BWPaintNamed("OZG_DActBoxEdge", DGC(201, 164, 92));
            bottom = top + whole;
        }
        else
        {
            bottom = DGTableActs(top);
            DTSwapButton(24, 0, 336, false);
        }
        DGLog(bottom + 10, 762);
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
            move = "";
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
        bool playing = DWPlaying();
        bool busy = DTBusy();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        int mineNo = m_V.Mine;
        int i;
        // whose die is on the table: the seat's in turn; while a one is seen (the turn has
        // passed with it), the one's who threw it
        bool flown = m_DTFx == "fly" || m_DTFx == "linger";
        bool burnt = flown && m_DTThrower >= 0 && (m_DTThrower != turn || turnPts <= 0);
        int shown = turn;
        if (burnt)
            shown = m_DTThrower;
        int turnShown = shown;

        int others;
        DGLaySeats("th", 688, others);
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat at = m_DGGeo[i];
            int seat = at.m_Seat;
            string p = DGPlateOf("OZG_DPg", i);
            OZG_GSeat g = DGSeat(seat);
            if (!g)
                continue;
            bool me = seat == mineNo;
            bool current = seat == turnShown;
            float pw = at.m_PlateW;
            int score = DGArgInt(g.Line, 0);
            if (score < 0)
                score = 0;
            float scoreW = DGText(p + "Score", score.ToString());
            DGRight(p + "Score", pw, 12, 10);
            DGPlateName(p, seat, pw - 24 - scoreW - 10);
            // the bar to the goal, the turn's points after it
            float barW = pw - 24;
            int held = score;
            if (held > goal)
                held = goal;
            float fw = barW * held / goal;
            int extra = 0;
            if (current)
            {
                extra = turnPts;
                if (extra > goal - held)
                    extra = goal - held;
            }
            float tw = barW * extra / goal;
            Widget bar = BW(p + "Bar");
            if (bar)
                bar.SetSize(barW * m_K, 10 * m_K);
            BWFit(BW(p + "BarTrack"), barW, 10, 5);
            BWFit(BW(p + "BarFillShape"), barW, 10, 5);
            BWFit(BW(p + "BarTurnShape"), barW, 10, 5);
            Widget fill = BW(p + "BarFill");
            if (fill)
                fill.SetSize(fw * m_K, 10 * m_K);
            Widget turnW = BW(p + "BarTurn");
            if (turnW)
            {
                turnW.SetPos(fw * m_K, 0);
                turnW.SetSize(tw * m_K, 10 * m_K);
            }
            BWMove(BW(p + "BarTurnShape"), -fw, 0);
            DGTagsBegin(p, 12, 68, pw - 24);
            if (current)
                DGTagTurn(me);
            DGTagWhose(seat, true);
            DGTagsEnd(2);
            int ring = DGC(74, 55, 38);
            if (current)
                ring = DGC(201, 164, 92);
            bool gone = g.Out == 1;
            Widget plate = BW(p);
            if (gone)
                DGDim(plate, p, 0.53, 0.62);
            else
                DGDim(plate, p, 1, 1);
            m_DGSnug = gone;
            DGPlateAt(p, at, 100, ring, 1);
            m_DGSnug = false;
        }
        DGPlatesHide("OZG_DPg", others, mineNo >= 0 && DGGeoOf(mineNo) != null);

        // ---- the die of the seat whose turn it is: on the felt, or in its can
        OZG_DTSeat sat = DGGeoOf(shown);
        bool lies = false;
        int face = die;
        if (sat)
        {
            if (DTHeld(shown, 0))
            {
                lies = false;
            }
            else if (burnt)
            {
                face = DTFlown(shown, 0);
                lies = face > 0;
            }
            else
            {
                lies = turnPts > 0 && die > 0 && playing;
                if (!playing && m_GPub.Ended == 1 && die > 0)
                    lies = true;
            }
            if (lies)
            {
                OZG_DTDie d = DGDieLie(shown, 0, face, 128, 577, 458, 1, -6, 1, true);
                if (d)
                {
                    string look = DTLookAt(shown, 0, true);
                    DTDieFace(d, face, look, 128, face == 1);
                    int mark = DTMarkAt(shown, 0, true);
                    int oneRing = 0;
                    if (face == 1)
                        oneRing = DGC(142, 42, 30);
                    DTDieMarked(d, mark, oneRing, false);
                }
            }
            else
            {
                DGDieHome(shown, 0, 128, sat, 0.25);
            }
        }
        string feltHint = "";
        if (playing && !busy && !lies && turn >= 0)
        {
            feltHint = Tr1("STR_OZG_DWIN_THROWS", SeatName(turn));
            if (mine)
                feltHint = Tr("STR_OZG_DWIN_YOUR_THROW");
        }
        DGFeltHint(feltHint, 430);
        DGTurnPill("OZG_DPgPill", turnPts);

        // ---- what this player may do
        bool canThrow = false;
        bool canHold = false;
        if (m_GOwn && mine && !busy)
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

        // ---- the cans
        array<int> hostSeats = new array<int>();
        DGHostSeats(hostSeats);
        int want = m_DTHostAt;
        if (shown >= 0 && hostSeats.Find(shown) >= 0)
            want = shown;
        if (hostSeats.Find(want) < 0)
        {
            want = -1;
            if (hostSeats.Count() > 0)
                want = hostSeats[0];
        }
        int hostAt = -1;
        if (hostSeats.Count() > 0)
            hostAt = DTHostSeat(want, DGSeatOut(m_DTHostAt));
        array<string> looks = new array<string>();
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat canAt = m_DGGeo[i];
            int canSeat = canAt.m_Seat;
            if (!DTOwnCan(canSeat) && canSeat != hostAt)
            {
                DGSpot(canAt);
                continue;
            }
            looks.Clear();
            looks.Insert(DTLookAt(canSeat, 0, false));
            string state = DTCanFx(canSeat);
            if (state == "")
            {
                state = "full";
                if (canSeat == shown && lies)
                    state = "empty";
            }
            DGCanAt(canSeat, canAt, state, looks, playing && canSeat == turnShown);
            if (canSeat == mineNo && canThrow)
                DGCanClick(canAt, OZG_DiceGameConst.M_THROW);
        }

        // ---- the right column: the turn, the swap, the log
        float bottom = 12;
        BWShow("OZG_DPgAct", playing);
        if (playing)
        {
            string turnName = SeatName(turn);
            string title = Tr1("STR_OZG_DWIN_TURN", turnName);
            string hint = Tr1("STR_OZG_DWIN_PIG_HINT_HOST", turnName);
            if (DTOwnCan(turn))
                hint = Tr1("STR_OZG_DWIN_PIG_HINT_OWN", turnName);
            if (mine)
            {
                title = Tr("STR_OZG_DICE_YOUR_TURN");
                hint = Tr("STR_OZG_DWIN_PIG_HINT_FIRST");
                if (turnPts > 0)
                    hint = Tr1("STR_OZG_DWIN_PIG_HINT_MORE", turnPts.ToString());
            }
            if (burnt && m_DTThrower == mineNo)
            {
                title = Tr("STR_OZG_DWIN_PIG_ONE_TITLE");
                hint = Tr("STR_OZG_DWIN_PIG_HINT_ONE");
            }
            else if (burnt)
            {
                title = Tr1("STR_OZG_DWIN_TURN", SeatName(m_DTThrower));
            }
            BWSetText("OZG_DPgTitle", title);
            int lines = DGLinesN("OZG_DPgHint", hint, 336, 3, 2);
            float y = 44 + lines * 19 + 10;
            if (canThrow)
            {
                m_DGMoveA = OZG_DiceGameConst.M_THROW;
                DGButton("OZG_DPgThrow", Tr("STR_OZG_DICE_BTN_THROW"), DGC(201, 164, 92), DGC(201, 164, 92), DGC(26, 18, 10));
            }
            else
            {
                DGButton("OZG_DPgThrow", Tr("STR_OZG_DICE_BTN_THROW"), DGC(51, 37, 26), DGC(51, 37, 26), DGC(160, 143, 115));
            }
            BWMove(BW("OZG_DPgThrow"), 12, y);
            y = y + 46 + 10;
            int holdRing = DGC(61, 45, 29);
            int holdInk = DGC(160, 143, 115);
            if (canHold)
            {
                m_DGMoveB = OZG_PigConst.M_HOLD;
                holdRing = DGC(201, 164, 92);
                holdInk = DGC(239, 228, 204);
            }
            string holdLabel = Tr("STR_OZG_DWIN_PIG_HOLD_NONE");
            if (mine && turnPts > 0)
                holdLabel = Tr1("STR_OZG_DWIN_PIG_HOLD", turnPts.ToString());
            DGButton("OZG_DPgHold", holdLabel, DGC(45, 33, 23), holdRing, holdInk);
            BWMove(BW("OZG_DPgHold"), 12, y);
            y = y + 46 + 10;
            float h = DGSwapRow(12, y, true);
            DGPanel("OZG_DPgAct", 12, h, mine);
            bottom = 12 + h;
        }
        else
        {
            bottom = DGTableActs(12);
            DTSwapButton(24, 0, 336, false);
        }
        DGLog(bottom + 10, 762);
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
        bool busy = DTBusy();
        bool mine = DGMyTurn();
        int turn = DGTurn();
        int mineNo = m_V.Mine;
        int i;
        int k;
        int others;
        DGLaySeats("hr", 692, others);
        array<int> hostSeats = new array<int>();
        DGHostSeats(hostSeats);
        // the table's can stands at the seat whose throw it is: a table of people waits for
        // a throw as the canvas's bots do not, and the can says whose it is (the dice of
        // the throw before lie on at their seat, dimmed)
        int want = m_DTHostAt;
        if (playing && turn >= 0 && hostSeats.Find(turn) >= 0)
            want = turn;
        if (hostSeats.Find(want) < 0)
        {
            want = -1;
            if (hostSeats.Count() > 0)
                want = hostSeats[0];
        }
        int hostAt = -1;
        if (hostSeats.Count() > 0)
            hostAt = DTHostSeat(want, false);
        float rots[3] = {-7.0, 5.0, -3.0};
        bool canThrow = playing && mine && !busy;
        bool flown = m_DTFx == "fly" || m_DTFx == "linger";
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat at = m_DGGeo[i];
            int seat = at.m_Seat;
            string p = DGPlateOf("OZG_DH", i);
            OZG_GSeat g = DGSeat(seat);
            if (!g)
                continue;
            // the seat's place in the game's own order: its chips and its stake lie there
            int order = -1;
            for (k = 0; k < m_GPub.SeatList.Count(); k++)
            {
                OZG_GSeat listed = m_GPub.SeatList[k];
                if (listed && listed.SeatNo == seat)
                    order = k;
            }
            bool me = seat == mineNo;
            bool won = g.Mark == 1;
            bool gone = g.Out == 1 && !won && playing;
            bool throwsNow = seat == m_DTThrower && m_DTFx != "" && m_DTFx != "home" && m_DTFx != "linger";
            float pw = at.m_PlateW;
            string chips = "0";
            if (order >= 0 && order < chipsOf.Count())
                chips = chipsOf[order];
            float chipsW = DGText(p + "Chips", Tr1("STR_OZG_DICE_POT_LINE", chips));
            DGRight(p + "Chips", pw, 12, 11);
            DGPlateName(p, seat, pw - 24 - chipsW - 8);
            array<int> dice = new array<int>();
            DWDiceOf(g.Role, dice);
            // a throw the view has no more on the table (those tied throw again): it lies
            // a moment as it fell
            if (dice.Count() == 0 && flown && seat == m_DTThrower)
            {
                for (k = 0; k < m_DTThrowFaces.Count(); k++)
                    dice.Insert(m_DTThrowFaces[k]);
            }
            int sum = 0;
            for (k = 0; k < dice.Count(); k++)
                sum = sum + dice[k];
            string sumText = "";
            if (dice.Count() > 0 && !DTHeld(seat, 0))
                sumText = sum.ToString();
            BWSetText(p + "Sum", sumText);
            DGRight(p + "Sum", pw, 12, 30.5);
            int sumInk = DGC(239, 228, 204);
            if (won)
                sumInk = DGC(231, 194, 122);
            BWInk(p + "Sum", sumInk);
            string state = Tr("STR_OZG_DWIN_HR_HOST");
            if (DTOwnCan(seat))
                state = Tr("STR_OZG_DWIN_HR_OWN");
            int stateInk = DGC(184, 165, 133);
            if (m_DGLobby && order >= 0 && order < putsOf.Count() && putsOf[order].ToInt() > 0)
                state = Tr1("STR_OZG_DWIN_HR_STAKE", putsOf[order]) + " · " + state;
            if (won)
            {
                state = Tr("STR_OZG_DWIN_HR_WON");
                stateInk = DGC(231, 194, 122);
            }
            else if (gone)
            {
                state = Tr("STR_OZG_DWIN_HR_OUT");
            }
            else if (throwsNow)
            {
                state = Tr("STR_OZG_DWIN_TAG_THROWS");
                stateInk = DGC(166, 217, 160);
            }
            BWSetText(p + "State", state);
            BWInk(p + "State", stateInk);
            int ring = DGC(74, 55, 38);
            float ringW = 1;
            if (won)
            {
                ring = DGC(231, 194, 122);
                ringW = 3;
            }
            else if (throwsNow || (playing && seat == turn))
            {
                ring = DGC(201, 164, 92);
            }
            Widget plate = BW(p);
            if (gone)
                DGDim(plate, p, 0.53, 0.62);
            else
                DGDim(plate, p, 1, 1);
            m_DGSnug = gone;
            DGPlateAt(p, at, 73, ring, ringW);
            m_DGSnug = false;

            // its dice: lying in its row, or in its can. The table's dice are where the can
            // is: a row of another seat is what that seat threw with them, dimmed -- but
            // for the throw made last, which lies as it fell until the next one, and for
            // this player's own
            bool own = DTOwnCan(seat);
            bool ghost = !own && seat != hostAt && seat != m_GPub.ThrowSeat && !me;
            // the pot is played out: every throw is looked at
            if (m_GPub.Ended == 1)
                ghost = false;
            float alpha = 1;
            if (ghost)
                alpha = 0.5;
            if (gone)
                alpha = 0.45;
            array<string> inCan = new array<string>();
            bool anyLies = false;
            for (k = 0; k < 3; k++)
            {
                bool lies = k < dice.Count() && !DTHeld(seat, k);
                if (!lies)
                {
                    DGDieHome(seat, k, 78, at, 0.3);
                    if (k < count)
                        inCan.Insert(DTLookAt(seat, k, false));
                    continue;
                }
                anyLies = true;
                float dx = at.m_RowX + k * at.m_RowStep;
                float rot = rots[k];
                DGDieLie(seat, k, dice[k], 78, dx, at.m_RowY, at.m_RowScale, rot, alpha, !ghost);
            }
            if (!own && seat != hostAt)
            {
                DGSpot(at);
                continue;
            }
            string canState = DTCanFx(seat);
            if (canState == "")
            {
                canState = "full";
                if (anyLies)
                    canState = "empty";
            }
            bool glow = throwsNow || (playing && seat == turn);
            DGCanAt(seat, at, canState, inCan, glow);
            if (me && canThrow)
                DGCanClick(at, OZG_DiceGameConst.M_THROW);
        }
        DGPlatesHide("OZG_DH", others, mineNo >= 0 && DGGeoOf(mineNo) != null);
        DGPill("OZG_DHBank", pot, 468, 470);

        // ---- the right column: the pot or the throw, the dice a throw, the swap, the log
        float bottom = 12;
        BWShow("OZG_DHAct", playing);
        if (playing)
        {
            string title = Tr1("STR_OZG_DWIN_HR_POT", pot.ToString());
            string hint = Tr("STR_OZG_DWIN_HR_HINT_OTHERS");
            if (hostSeats.Count() == 0)
                hint = Tr("STR_OZG_DWIN_HR_HINT_WAIT");
            if (mine)
            {
                title = Tr("STR_OZG_DWIN_YOUR_THROW");
                hint = Tr("STR_OZG_DWIN_HR_HINT_MINE");
            }
            BWSetText("OZG_DHTitle", title);
            int lines = DGLinesN("OZG_DHHint", hint, 336, 3, 2);
            float y = 44 + lines * 19 + 10;
            if (canThrow)
            {
                m_DGMoveA = OZG_DiceGameConst.M_THROW;
                DGButton("OZG_DHThrow", Tr1("STR_OZG_DWIN_HR_THROW", count.ToString()), DGC(201, 164, 92), DGC(201, 164, 92), DGC(26, 18, 10));
            }
            else
            {
                string waits = Tr("STR_OZG_DWIN_HR_WAITING");
                if (mine)
                    waits = Tr1("STR_OZG_DWIN_HR_THROW", count.ToString());
                DGButton("OZG_DHThrow", waits, DGC(51, 37, 26), DGC(51, 37, 26), DGC(160, 143, 115));
            }
            BWShow("OZG_DHTitle", true);
            BWShow("OZG_DHThrow", true);
            BWMove(BW("OZG_DHThrow"), 12, y);
            y = y + 46 + 10;
            DGHighCount(count, y, false);
            y = y + 38 + 10;
            float h = DGSwapRow(12, y, true);
            DGPanel("OZG_DHAct", 12, h, true);
            bottom = 12 + h;
        }
        else
        {
            // between games: the table's buttons, and under them the dice a throw in a
            // box of its own (the host steps it there)
            bottom = DGTableActs(12);
            DTSwapButton(24, 0, 336, false);
            BWShow("OZG_DHAct", true);
            BWShow("OZG_DHTitle", false);
            BWShow("OZG_DHThrow", false);
            DGLinesN("OZG_DHHint", "", 336, 3, 0);
            DGHighCount(count, 12, m_V.IsHost == 1);
            DGPanel("OZG_DHAct", bottom + 10, 62, false);
            bottom = bottom + 10 + 62;
        }
        DGLog(bottom + 10, 762);
    }

    // High roll's row "dice a throw" at `y` of its panel: lit while the host may step it.
    protected void DGHighCount(int count, float y, bool lit)
    {
        BWSetText("OZG_DHCountValue", count.ToString());
        BWMove(BW("OZG_DHCount"), 12, y);
        int edge = DGC(61, 45, 29);
        int ink = DGC(160, 143, 115);
        if (lit)
        {
            edge = DGC(107, 82, 51);
            ink = DGC(239, 228, 204);
        }
        BWPaintNamed("OZG_DHCountBoxEdge", edge);
        BWInk("OZG_DHCountText", ink);
        BWInk("OZG_DHCountValue", ink);
    }

    // ------------------------------------------------------------ perudo

    // The dice of words "KEY|1 3 5" (perudo's own: "STR_OZG_PERUDO_YOURS|..").
    protected void DGDiceArg(string words, array<int> into)
    {
        into.Clear();
        array<string> args = new array<string>();
        DWArgs(words, args);
        if (args.Count() > 0)
            OZG_DiceText.Parse(args[0], into);
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
        int mineNo = m_V.Mine;
        // the last showdown (its bid, wild ones, the loser, whose bid it was, every seat's
        // dice): the dice lie open DG_REVEAL seconds after it, as the canvas shows them, and
        // for good once the game is over; then the cans are shaken and the new round is on
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
        // a round thrown: the cans are shaken -- after the showdown that came with it
        if (m_DGRoundSeen < 0 || !playing)
            m_DGRoundSeen = m_GPub.Throws;
        if (playing && m_GPub.Throws != m_DGRoundSeen && !reveal && m_DGShakeLeft <= 0)
        {
            m_DGRoundSeen = m_GPub.Throws;
            m_DGShakeLeft = DG_SHAKE;
            DTThrowSound(OZG_SoundIds.RATTLE);
        }
        bool shaking = m_DGShakeLeft > 0 && !reveal;
        bool blocked = reveal || shaking;
        m_DTBlocked = blocked;
        int openN = DGIntAt(open, 0);
        int openF = DGIntAt(open, 1);
        bool openPal = DGIntAt(open, 2) == 1;
        int openLoser = -1;
        if (open.Count() > 3)
            openLoser = open[3].ToInt();
        int openBidder = -1;
        if (open.Count() > 4)
            openBidder = open[4].ToInt();
        int turn = DGTurn();
        bool mine = DGMyTurn();
        // while the dice lie open or the cans are shaken nobody bids
        bool myTurn = mine;
        if (reveal || shaking)
            myTurn = false;
        int shownBidder = bidder;
        if (reveal)
            shownBidder = openBidder;
        int i;
        int k;
        int others;
        DGLaySeats("pe", 692, others);
        array<int> myDice = new array<int>();
        for (i = 0; i < m_DGGeo.Count(); i++)
        {
            OZG_DTSeat at = m_DGGeo[i];
            int seat = at.m_Seat;
            string p = DGPlateOf("OZG_DR", i);
            OZG_GSeat g = DGSeat(seat);
            if (!g)
                continue;
            bool me = seat == mineNo;
            int have = DGArgInt(g.Line, 0);
            if (have < 0)
                have = 0;
            bool left = g.Out == 1 || have <= 0;
            // out of the game: his plate dims once the dice are covered again
            bool gone = left && !reveal;
            bool thinks = seat == turn && playing && !reveal && !shaking;
            bool hisBid = seat == shownBidder;
            float pw = at.m_PlateW;
            float countW = DGText(p + "Count", Tr1("STR_OZG_DWIN_PR_DICE", have.ToString()));
            DGRight(p + "Count", pw, 12, 11);
            DGPlateName(p, seat, pw - 24 - countW - 8);
            string line = "";
            int lineInk = DGC(184, 165, 133);
            if (gone)
            {
                line = Tr("STR_OZG_DWIN_OUT");
            }
            else if (reveal && seat == openLoser)
            {
                line = Tr("STR_OZG_DWIN_PR_LOSES");
                lineInk = DGC(240, 154, 134);
            }
            else if (hisBid)
            {
                line = Tr("STR_OZG_DWIN_PR_HIS_BID");
                if (me)
                    line = Tr("STR_OZG_DWIN_PR_MY_BID");
            }
            else if (thinks)
            {
                line = Tr("STR_OZG_DWIN_PR_THINKS");
                if (me)
                    line = Tr("STR_OZG_DWIN_PR_MY_TURN");
                lineInk = DGC(166, 217, 160);
            }
            BWSetText(p + "Line", line);
            BWInk(p + "Line", lineInk);
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
            Widget plate = BW(p);
            if (gone)
                DGDim(plate, p, 0.48, 0.58);
            else
                DGDim(plate, p, 1, 1);
            m_DGSnug = gone;
            DGPlateAt(p, at, 59, ring, ringW);
            m_DGSnug = false;

            // its dice: this player's own under his can, seen by him alone; everybody's
            // at the showdown, what counts ringed
            array<int> dice = new array<int>();
            if (reveal)
                DGOpenDice(open, 5, seat, dice);
            else if (me && m_GOwn && !ended)
                DGDiceArg(m_GOwn.Status, dice);
            if (me)
                myDice.Copy(dice);
            bool seen = dice.Count() > 0 && !shaking && (me || reveal);
            array<string> looks = new array<string>();
            for (k = 0; k < 5; k++)
            {
                if (k < have)
                    looks.Insert(DTLookAt(seat, k, false));
                if (!seen || k >= dice.Count())
                {
                    DGDieHome(seat, k, 78, at, 0.3);
                    continue;
                }
                int v = dice[k];
                float dx = at.m_RowX + k * at.m_RowStep;
                OZG_DTDie d = DGDieLie(seat, k, v, 78, dx, at.m_RowY, at.m_RowScale, 0, 1, true);
                bool wild = v == 1 && !openPal && openF != 1;
                bool counts = reveal && (v == openF || wild);
                if (d && counts)
                {
                    int mark = DTMarkAt(seat, k, true);
                    DTDieMarked(d, mark, DGC(231, 194, 122), false);
                }
                if (d)
                {
                    d.m_MoveLen = 0.4;
                    d.m_FadeLen = 0.2;
                }
            }
            // its can: shaken with a new round, lifted for him who looks under it and for
            // everybody at the showdown, gone with its last die
            string canState = "down";
            bool looksUnder = me && myDice.Count() > 0;
            if (looksUnder || reveal)
                canState = "peek";
            if (shaking)
                canState = "shake";
            if (have <= 0 && !reveal)
                canState = "up";
            bool glow = thinks;
            DGCanAt(seat, at, canState, looks, glow);
        }
        DGPlatesHide("OZG_DR", others, mineNo >= 0 && DGGeoOf(mineNo) != null);
        bool under = !reveal && !shaking && myDice.Count() > 0 && playing;
        BWShow("OZG_DRUnder", under);
        BWSetText("OZG_DRUnder", Tr("STR_OZG_DWIN_PR_UNDER"));

        // ---- the bid in the middle
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
        // the bid being made: picked again from the bid standing whenever it changes
        string seenBid = bidN.ToString() + ":" + bidF.ToString() + ":" + bidder.ToString();
        if (seenBid != m_DGBidSeen)
        {
            m_DGBidSeen = seenBid;
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
        float bottom = 12;
        if (playing)
        {
            string title = Tr1("STR_OZG_DWIN_PR_BIDS", SeatName(turn));
            if (reveal)
                title = Tr("STR_OZG_DWIN_PR_OPEN");
            else if (shaking)
                title = Tr("STR_OZG_DWIN_PR_SHAKING");
            else if (myTurn)
                title = Tr("STR_OZG_DWIN_PR_YOUR_BID");
            BWSetText("OZG_DRAskTitle", title);
            // "палифіко" beside it: a pill as wide as its word, at the panel's right
            Widget palW = BW("OZG_DRPal");
            if (palW)
            {
                palW.Show(pal);
                TextWidget palT = BWText("OZG_DRPalText");
                float palTw = 60;
                if (palT)
                    palTw = BWTextW(palT);
                float palCw = palTw + 16;
                palW.SetPos((348 - palCw) * m_K, 12 * m_K);
                palW.SetSize(palCw * m_K, 22 * m_K);
                BWFit(BW("OZG_DRPalBg"), palCw, 22, 11);
            }
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
            OZG_GSeat meSeat = DGSeat(mineNo);
            if (meSeat && DGArgInt(meSeat.Line, 0) <= 0 && !reveal)
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
            DGLinesN("OZG_DRNote", note, 336, 2, 2);
            BWInk("OZG_DRNote0", noteInk);
            BWInk("OZG_DRNote1", noteInk);
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
            float h = DGSwapRow(12, 273, true);
            Widget ask = BW("OZG_DRAsk");
            if (ask)
                ask.SetSize(360 * m_K, h * m_K);
            int askRing = DGC(107, 82, 51);
            if (myTurn)
                askRing = DGC(201, 164, 92);
            m_DGSnug = false;
            DGBox("OZG_DRAskBox", 360, h, 6, askRing, 1);
            bottom = 12 + h;
        }
        else
        {
            bottom = DGTableActs(12);
            DTSwapButton(24, 0, 336, false);
        }
        DGLog(bottom + 10, 762);
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

    // The showdown's time runs out and the cans are shaken; the shake runs out and they
    // stand down.
    override protected void DGTick(float dt)
    {
        if (m_DGOpenLeft > 0)
        {
            m_DGOpenLeft = m_DGOpenLeft - dt;
            if (m_DGOpenLeft <= 0)
            {
                m_DGOpenLeft = 0;
                if (m_DGActive)
                    BoardWinPaint();
            }
        }
        if (m_DGShakeLeft > 0)
        {
            m_DGShakeLeft = m_DGShakeLeft - dt;
            if (m_DGShakeLeft <= 0)
            {
                m_DGShakeLeft = 0;
                if (m_DGActive)
                {
                    BoardWinPaint();
                    DGPerudoTumble();
                    // the cans come down on the felt (the canvas's 'land')
                    DTThrowSound(OZG_SoundIds.LAND);
                }
            }
        }
    }

    // Perudo sounds its throw itself: the cans rattle once the showdown that came with the
    // throw has been seen.
    override protected bool DTThrowSounding()
    {
        if (m_DGFace == "perudo")
        {
            if (m_DGShakeLeft > 0 || m_DGOpenLeft > 0)
                return true;
        }
        return super.DTThrowSounding();
    }

    // Perudo's dice under this player's can tumble as the can comes down on them.
    protected void DGPerudoTumble()
    {
        if (!m_V || m_V.Mine < 0 || m_DGFace != "perudo")
            return;
        for (int k = 0; k < 5; k++)
        {
            OZG_DTDie d = DTDieFind(DTDieId(m_V.Mine, k));
            if (d && d.m_Used && d.m_ToAlpha > 0.5)
                d.m_TumbleT = 0;
        }
    }

    // ------------------------------------------------------------ what the cans ask

    // Does the view shown have the dice of the last throw on the table (else they lie a
    // moment as they fell: a bolt, a one, a tie thrown again)?
    override protected bool DTThrowLies()
    {
        if (!m_V || !m_GPub)
            return true;
        if (m_V.GameId == OZG_ThousandConst.GAME_ID)
        {
            array<int> thrown = new array<int>();
            OZG_GZone zone = DGZoneOf("throw");
            if (zone)
                DWDiceOf(zone.Line, thrown);
            return thrown.Count() > 0;
        }
        if (m_V.GameId == OZG_PigConst.GAME_ID)
        {
            array<string> st = new array<string>();
            DGZone("pig", st);
            int turn = DGTurn();
            if (m_GPub.Ended == 1)
                return true;
            return turn == m_DTThrower && DGIntAt(st, 1) > 0;
        }
        if (m_V.GameId == OZG_HighRollConst.GAME_ID)
        {
            OZG_GSeat g = DGSeat(m_DTThrower);
            array<int> dice = new array<int>();
            if (g)
                DWDiceOf(g.Role, dice);
            return dice.Count() > 0;
        }
        return true;
    }

    // Is a throw this player's next move (a die may be swapped before it)?
    override protected bool DTMyThrowNext()
    {
        if (!m_GOwn || !DGMyTurn())
            return false;
        for (int i = 0; i < m_GOwn.Buttons.Count(); i++)
        {
            OZG_GButton b = m_GOwn.Buttons[i];
            if (b && b.Move == OZG_DiceGameConst.M_THROW)
                return true;
        }
        return false;
    }

    override protected string DTSwapWhenKey()
    {
        if (m_V && m_V.GameId == OZG_PerudoConst.GAME_ID)
            return "STR_OZG_DWIN_SWAP_WHEN_PERUDO_SHORT";
        return "STR_OZG_SAY_DSWAP_NOT_NOW";
    }

    // A die swapped (the server's word): a die flies from the hand at the seat into its
    // can -- where the can stands now --, and the one taken out flies back. Who noticed is
    // told who did it; the one who did it, what he slipped in.
    override protected void DTSawPlay(OZG_WDiceSaw saw)
    {
        if (m_Style != "dice")
        {
            super.DTSawPlay(saw);
            return;
        }
        OZG_DTSeat at = DGGeoOf(saw.SeatNo);
        if (!at || !m_DTCans)
            return;
        float bx = at.m_CanX + 38 * at.m_CanK - 17;
        float by = at.m_CanY - 2;
        OZG_DTCan can = null;
        string id = DTCanId(saw.SeatNo);
        for (int i = 0; i < m_DTCans.Count(); i++)
        {
            if (m_DTCans[i].m_Id == id && m_DTCans[i].m_Used)
                can = m_DTCans[i];
        }
        if (can)
        {
            bx = can.m_X + m_DTOX + 38 * can.m_Scale - 17;
            by = can.m_Y + m_DTOY - 2;
        }
        bool noticed = saw.Noticed == 1;
        DTSwapPlay(saw.SeatNo, saw.InLook, saw.OutLook, noticed, at.m_HandX, at.m_HandY, bx, by, can);
        bool under = m_V.GameId == OZG_PerudoConst.GAME_ID;
        if (noticed)
        {
            string who = Tr1("STR_OZG_SAY_DSWAP_SEEN", SeatName(saw.SeatNo));
            string seenSub = Tr("STR_OZG_DWIN_SWAP_SEEN_SUB");
            if (under)
                seenSub = Tr("STR_OZG_DWIN_SWAP_SEEN_SUB_UNDER");
            DTToast(who, "", seenSub, true);
            return;
        }
        int placeNo = saw.Place + 1;
        string what = DTDieNameAcc(saw.InLook);
        string did = Tr2("STR_OZG_DWIN_SWAP_DID", what, placeNo.ToString());
        string didSub = Tr("STR_OZG_DWIN_SWAP_DID_SUB");
        if (under)
            didSub = Tr("STR_OZG_DWIN_SWAP_DID_SUB_UNDER");
        DTToast(did, "", didSub, false);
    }

    // ------------------------------------------------------------ clicks

    override protected bool DGClick(string n)
    {
        // high roll's dice a throw: the host's between games, one to three
        if (n == "OZG_DHCount" && m_V && m_V.GameId == "highroll")
        {
            if (m_V.IsHost == 1 && !DWPlaying())
            {
                string now = DGOption(OZG_HighRollConst.OPT_DICE, "2");
                int more = now.ToInt() + 1;
                if (more > 3)
                    more = 1;
                OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
                Send(OZG_Const.OP_OPT, "", -1, "", OZG_HighRollConst.OPT_DICE, more.ToString(), 0);
            }
            else
            {
                DTToast(Tr("STR_OZG_DWIN_HR_COUNT_WHEN"), "", "", false);
            }
            return true;
        }
        if (!m_DGActive)
            return false;
        if (n == "OZG_DPgThrow" || n == "OZG_DHThrow" || n == "OZG_DRBidBtn" || n == "OZG_DThThrow")
        {
            DWAct(m_DGMoveA);
            return true;
        }
        if (n == "OZG_DPgHold" || n == "OZG_DRDudoBtn" || n == "OZG_DThWrite")
        {
            DWAct(m_DGMoveB);
            return true;
        }
        if (n == "OZG_DThAsideBtn")
        {
            DWAct(m_DGMoveC);
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
            DWAct(move);
            return true;
        }
        return false;
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
