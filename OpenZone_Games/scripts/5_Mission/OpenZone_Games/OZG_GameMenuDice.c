// The dice games' window (boards and dice design §6, §7, §8, §9.2): the design canvas's
// boards of thousand, dice poker, pig, high roll and perudo as redone on 2026-11-27, their
// shared frame built of ozg_dwin.layout by tools/make_dice_win.py. A header bar: the game,
// its rules, the game and whose dice. The felt tray: the players' plates round it, a can of
// dice at the seat that throws, the dice thrown out of it. Right: what this player may do,
// "Підмінити кістку", the log. Every game's own parts are its face (OZG_GameMenuDiceGames.c);
// the cans, the dice and the swap's window are OZG_GameMenuCans.c's.
//
// The games tell it everything in the later games' view (OZG_GPublic, OZG_GPrivate): a
// seat's line and role, the zones, the buttons. A text of dice is "STR_OZG_DICE_SHOWN|1 3
// 5". A die is drawn in its own look (a red one swapped into a white set stays red wherever
// it lies) by the seat that throws it and its place among that seat's dice.
modded class OZG_GameMenu
{
    protected static const string DW_TRAY = "OpenZone_Games/gui/boards/ozg_dice_tray_co.paa";
    protected static const string DW_SHOWN = "STR_OZG_DICE_SHOWN";
    // the dice this player may pick on the felt: their middles (the window's units), how far
    // from its middle a die is hit, the ones picked (their numbers in the list)
    protected ref array<float> m_DWDieX;
    protected ref array<float> m_DWDieY;
    protected float m_DWDieHit = 40;
    protected ref array<int> m_DWPicked;
    // thousand: the dice of the throw to pick from
    protected ref array<int> m_DWDice;
    // the action panel's buttons and their moves
    protected ref array<string> m_DWActMoves;
    // this player's own can on the felt (the window's units): a click on it throws
    protected float m_DWCanX;
    protected float m_DWCanY;
    protected float m_DWCanW;
    protected float m_DWCanH;
    protected string m_DWCanMove = "";

    // ------------------------------------------------------------ the later file's part

    // Every dice game has a face of its own (the canvas's boards), painted by
    // OZG_GameMenuDiceGames.c, which loads after this file: true when it painted the window.
    protected bool DGPaint()
    {
        return false;
    }

    protected bool DGClick(string n)
    {
        return false;
    }

    override protected bool DiceWindow()
    {
        if (!m_V)
            return false;
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (sample)
            return sample.SetKind() == OZG_Const.SET_DICE;
        array<OZG_Place> places = OZG_Place.All();
        for (int i = 0; i < places.Count(); i++)
        {
            OZG_Place place = places[i];
            if (!place || place.OZG_Id() != m_PlaceId)
                continue;
            OZG_Deck_Base deck = place.OZG_Deck();
            return deck && deck.IsInherited(OZG_DiceCan_Base);
        }
        return false;
    }

    protected void DWInit()
    {
        if (m_DWDieX)
            return;
        m_DWDieX = new array<float>();
        m_DWDieY = new array<float>();
        m_DWPicked = new array<int>();
        m_DWDice = new array<int>();
        m_DWActMoves = new array<string>();
    }

    // ------------------------------------------------------------ words

    // The arguments of words "KEY|a|b".
    protected void DWArgs(string words, array<string> into)
    {
        into.Clear();
        array<string> parts = new array<string>();
        words.Split(OZG_Words.SEP, parts);
        for (int i = 1; i < parts.Count(); i++)
            into.Insert(parts[i]);
    }

    // The dice of words "STR_OZG_DICE_SHOWN|1 3 5"; false for other words.
    protected bool DWDiceOf(string words, array<int> into)
    {
        into.Clear();
        if (words.IndexOf(DW_SHOWN) != 0)
            return false;
        array<string> args = new array<string>();
        DWArgs(words, args);
        if (args.Count() > 0)
            OZG_DiceText.Parse(args[0], into);
        return true;
    }

    // ------------------------------------------------------------ painting

    override protected void DWPaint()
    {
        DWInit();
        // a view that tells of a throw waits until the throw is seen (OZG_GameMenuCans.c)
        DTAdopt();
        BWPoolsBegin();
        DTBegin();
        m_DWDieX.Clear();
        m_DWDieY.Clear();
        m_DWCanMove = "";
        DWPaintHead();
        ImageWidget tray = ImageWidget.Cast(BW("OZG_DTrayPic"));
        if (tray)
        {
            tray.LoadImageFile(0, DW_TRAY);
            tray.SetImage(0);
        }
        if (!DGPaint())
            DWPaintBare();
        DWPaintLog();
        DTSwapKeep();
        DTEnd();
        BWPoolsEnd();
        DTPaintSwapDlg();
        DTPaintPickDlg();
        BWPaintDialogs();
    }

    // A can laid out with no game chosen that has a face: the table's own buttons, the log.
    protected void DWPaintBare()
    {
        BWShow("OZG_DTHint", false);
        BWShow("OZG_DScore", false);
        BWShow("OZG_DSwapBtn", false);
        BWShow("OZG_DAct", true);
        m_DWScoreH = -10;
        DWPaintActions();
        Widget logW = BW("OZG_DLog");
        float top = m_DWActBottom + 10;
        if (logW)
        {
            logW.SetPos(12 * m_K, top * m_K);
            logW.SetSize(360 * m_K, (762 - top) * m_K);
        }
        BWFit(BW("OZG_DLogBg"), 360, 762 - top, 6);
    }

    override protected bool DWPlaying()
    {
        return m_GPub && m_V.St == OZG_Const.ST_PLAYING && m_GPub.Ended != 1;
    }

    // A dice game over (the canvas's "Гру закінчено" on every dice board): its last view
    // says so, until another is dealt or chosen. The box waits for the last throw to be seen.
    override protected bool BWOver()
    {
        if (!DiceWindow())
            return super.BWOver();
        if (!m_GPub || !m_V)
            return false;
        if (DTBusy() || m_DTOverWait > 0)
            return false;
        bool ended = m_GPub.Ended == 1 && m_V.St != OZG_Const.ST_PLAYING && m_GPub.GameId == m_V.GameId;
        return ended;
    }

    // Its words: how the game ended, as the game says it; a new game for the host, a look
    // at the table for everybody (the canvas has the one button: its board is one player's).
    override protected void BWPaintOver()
    {
        if (!DiceWindow())
        {
            super.BWPaintOver();
            return;
        }
        BWSetText("OZG_BOverTitle", Tr("STR_OZG_DWIN_OVER_TITLE"));
        BWSetText("OZG_BOverBtnNewText", Tr("STR_OZG_DWIN_NEW"));
        BWSetText("OZG_BOverBtnLookText", Tr("STR_OZG_DWIN_LOOK"));
        string text = "";
        if (m_GPub && m_GPub.Result != "")
            text = Words(m_GPub.Result) + ".";
        // the canvas's dice boards dim what is under the header (40, 74, 1520 x 798) and
        // lay the box in its middle: 23 lower than a board's
        Widget dim = BW("OZG_BOverDim");
        if (dim)
        {
            dim.SetPos(40 * m_K, 74 * m_K);
            dim.SetSize(1520 * m_K, 798 * m_K);
        }
        BWOverLay(text, 23);
    }

    // A note of the server, a refusal: the note over the felt (the canvas's).
    override protected void BWToast(string text, bool hot)
    {
        if (m_Style == "dice")
        {
            DTToast(text, "", "", hot);
            return;
        }
        super.BWToast(text, hot);
    }

    // The header: the game and its rules, the game's button and whose dice, from the right.
    protected void DWPaintHead()
    {
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        string game = "";
        if (sample)
            game = Tr(sample.LabelKey());
        TextWidget title = BWText("OZG_DTitle");
        if (title)
        {
            title.SetText(game);
            BWMove(BW("OZG_DSub"), 60 + BWTextW(title) + 10, 42.5);
        }
        string sub = "";
        if (m_V.GameId != "")
        {
            string key = "STR_OZG_DWIN_SUB_" + BWUpper(m_V.GameId);
            sub = Tr(key);
            if (sub == key || sub == "")
                sub = "";
            if (sub != "" && m_V.GameId == OZG_DPokerConst.GAME_ID)
            {
                string stake = "10";
                int stakeAt = m_V.OptIds.Find(OZG_DiceConst.OPT_STAKE);
                if (stakeAt >= 0 && stakeAt < m_V.OptVals.Count())
                    stake = m_V.OptVals[stakeAt];
                sub.Replace("%1", stake);
            }
        }
        BWSetText("OZG_DSub", sub);
        float right = 1508;
        TextWidget dice = BWText("OZG_DBtnDiceText");
        Widget diceBtn = BW("OZG_DBtnDice");
        string variant = VariantLabel();
        if (dice && diceBtn)
        {
            dice.SetText(Tr1("STR_OZG_BWIN_DICE_CHIP", OZG_Text.LowerFirst(variant)));
            float dw = BWTextW(dice) + 26;
            DWButtonAt(diceBtn, "OZG_DBtnDice", right - dw, 35.5, dw, 30);
            diceBtn.Show(variant != "");
            if (variant != "")
                right = right - dw - 10;
        }
        TextWidget gameText = BWText("OZG_DBtnGameText");
        Widget gameBtn = BW("OZG_DBtnGame");
        if (gameText && gameBtn)
        {
            gameText.SetText(Tr1("STR_OZG_BWIN_GAME", game));
            float gw = BWTextW(gameText) + 26;
            DWButtonAt(gameBtn, "OZG_DBtnGame", right - gw, 35.5, gw, 30);
            right = right - gw;
        }
        BWMove(BW("OZG_DSep"), right - 15, 37.5);
    }

    // A button of the layout moved and sized, its box's shapes and its words with it.
    protected void DWButtonAt(Widget b, string name, float x, float y, float w, float h)
    {
        if (!b)
            return;
        b.SetPos(x * m_K, y * m_K);
        b.SetSize(w * m_K, h * m_K);
        Widget boxW = BW(name + "Box");
        if (boxW)
            boxW.SetSize(w * m_K, h * m_K);
        BWFit(BW(name + "BoxEdge"), w, h, 4);
        Widget bg = BW(name + "BoxBg");
        if (bg)
        {
            bool ringed = BW(name + "BoxEdge") != null;
            if (ringed)
            {
                bg.SetPos(1 * m_K, 1 * m_K);
                BWFit(bg, w - 2, h - 2, 3);
            }
            else
            {
                BWFit(bg, w, h, 4);
            }
        }
        Widget label = BW(name + "Text");
        if (label)
            label.SetSize(w * m_K, h * m_K);
    }

    // The score (the canvas's "РАХУНОК"): a row a seat, the row of the seat whose turn it
    // is lit and in bold; a seat's note (`notes`, beside the seats in their order), its
    // total right.
    protected void DWPaintScore(array<string> notes)
    {
        BWShow("OZG_DScore", true);
        BWSetText("OZG_DScoreTitle", OZG_Text.Upper(Tr("STR_OZG_DWIN_SCORE")));
        int turn = -1;
        if (DWPlaying() && m_GPub.Awaited.Count() > 0)
            turn = m_GPub.Awaited[0];
        int shown = 0;
        if (m_GPub)
        {
            for (int s = 0; s < m_GPub.SeatList.Count() && shown < 6; s++)
            {
                OZG_GSeat g = m_GPub.SeatList[s];
                if (!g)
                    continue;
                string i = shown.ToString();
                bool lit = g.SeatNo == turn;
                string name = SeatName(g.SeatNo);
                if (g.SeatNo == m_V.Mine)
                    name = name + " " + Tr("STR_OZG_BWIN_YOU");
                BWShow("OZG_DScoreRow" + i, lit);
                BWShow("OZG_DScoreName" + i, !lit);
                BWShow("OZG_DScoreMe" + i, lit);
                BWSetText("OZG_DScoreName" + i, name);
                BWSetText("OZG_DScoreMe" + i, name);
                string note = "";
                if (notes && s < notes.Count())
                    note = notes[s];
                if (g.Out == 1)
                    note = Tr("STR_OZG_DWIN_OUT");
                BWSetText("OZG_DScoreNote" + i, note);
                BWShow("OZG_DScoreNote" + i, true);
                array<string> args = new array<string>();
                DWArgs(g.Line, args);
                string total = "";
                if (args.Count() > 0)
                    total = args[0];
                BWSetText("OZG_DScoreTotal" + i, total);
                BWShow("OZG_DScoreTotal" + i, true);
                shown++;
            }
        }
        for (int k = shown; k < 6; k++)
        {
            string j = k.ToString();
            BWShow("OZG_DScoreRow" + j, false);
            BWShow("OZG_DScoreName" + j, false);
            BWShow("OZG_DScoreMe" + j, false);
            BWShow("OZG_DScoreNote" + j, false);
            BWShow("OZG_DScoreTotal" + j, false);
        }
        float h = 30 + shown * 32 + 4;
        if (h < 70)
            h = 70;
        BWFit(BW("OZG_DScoreBg"), 360, h, 6);
        m_DWScoreH = h;
    }

    protected float m_DWScoreH = 130;

    // Between games: what the table offers -- begin (the host), the host's role taken, the
    // pose, leaving the seat, folding the can -- as buttons of the window's own panel, two a
    // row, under the score (`m_DWScoreH` high); under them "Банка й кості" for a player with
    // a can or dice to choose before the deal.
    protected void DWPaintActions()
    {
        float top = 12 + m_DWScoreH + 10;
        Widget panel = BW("OZG_DAct");
        if (panel)
            panel.SetPos(12 * m_K, top * m_K);
        BWSetText("OZG_DActTitle", Tr("STR_OZG_DWIN_TABLE"));
        BWSetText("OZG_DActInfo", "");
        BWShow("OZG_DActTitle", true);
        BWShow("OZG_DActInfo", true);
        BWShow("OZG_DAmount", false);
        BWPaintNamed("OZG_DActBoxEdge", ARGB(255, 201, 164, 92));
        m_DWActMoves.Clear();
        array<ref OZG_GButton> main = new array<ref OZG_GButton>();
        DWTableButtons(main);
        float y = 44;
        int used = 0;
        int i;
        // two a row; one alone (or the last of an odd number) across the panel
        for (i = 0; i < main.Count() && used < 16; i++)
        {
            bool wide = false;
            if (main.Count() == 1)
                wide = true;
            if (i == main.Count() - 1 && main.Count() % 2 == 1)
                wide = true;
            float bw = 163;
            float bx = 12;
            if (wide)
                bw = 336;
            else if (i % 2 == 1)
                bx = 185;
            DWActButton(used, main[i], bx, y, bw, 42);
            if (wide || i % 2 == 1)
                y = y + 50;
            used++;
        }
        while (used < 16)
        {
            BWShow("OZG_DAct" + used.ToString(), false);
            used++;
        }
        // "Банка й кості" (the column's own button, laid into the panel): 38 high, the
        // panel's 12 under it
        bool picks = DTPickButton(24, top + y, 336, true);
        if (picks)
            y = y + 46;
        float h = y + 4;
        if (h < 100)
            h = 100;
        Widget boxW = BW("OZG_DActBox");
        if (boxW)
            boxW.SetSize(360 * m_K, h * m_K);
        BWFit(BW("OZG_DActBoxEdge"), 360, h, 6);
        BWFit(BW("OZG_DActBoxBg"), 358, h - 2, 5);
        m_DWActBottom = top + h;
    }

    protected float m_DWActBottom = 300;

    // The faces a "set:" move of thousand names, by its places (from 1) among the dice thrown.
    protected void DWSetFaces(string move, array<int> into)
    {
        into.Clear();
        string list = move.Substring(4, move.Length() - 4);
        array<string> places = new array<string>();
        list.Split(",", places);
        for (int i = 0; i < places.Count(); i++)
        {
            int at = places[i].ToInt() - 1;
            if (at >= 0 && at < m_DWDice.Count())
                into.Insert(m_DWDice[at]);
        }
    }

    protected string DWFacesKey(array<int> faces)
    {
        string key = "";
        for (int i = 0; i < faces.Count(); i++)
        {
            int face = faces[i];
            key = key + face.ToString();
        }
        return key;
    }

    protected void DWActButton(int i, OZG_GButton data, float x, float y, float w, float h)
    {
        string name = "OZG_DAct" + i.ToString();
        Widget b = BW(name);
        if (!b)
            return;
        m_DWActMoves.Insert(data.Move);
        DWButtonAt(b, name, x, y, w, h);
        BWSetText(name + "Text", Words(data.Label));
        int bg = ARGB(255, 51, 37, 26);
        int ink = ARGB(255, 239, 228, 204);
        int ring = ARGB(255, 107, 82, 51);
        if (data.Hot == 1)
        {
            bg = ARGB(255, 201, 164, 92);
            ink = ARGB(255, 26, 18, 10);
            ring = bg;
        }
        BWPaintNamed(name + "BoxBg", bg);
        BWPaintNamed(name + "BoxEdge", ring);
        BWInk(name + "Text", ink);
        b.Show(true);
    }

    // Between games: begin (the host), the host's role taken, the pose; as buttons of the
    // action panel.
    protected void DWTableButtons(array<ref OZG_GButton> into)
    {
        bool host = m_V.IsHost == 1;
        bool seated = m_V.Mine >= 0;
        if (host)
        {
            OZG_GButton begin = OZG_GButton.Make("#deal", "STR_OZG_BWIN_BEGIN", -1, 0);
            begin.Hot = 1;
            into.Insert(begin);
        }
        if (!host && seated && m_V.HostFree == 1)
            into.Insert(OZG_GButton.Make("#host", "STR_OZG_WIN_TAKE_HOST", -1, 0));
        if (seated)
        {
            string pose = OZG_PoseDriver.LabelNow(m_V);
            if (pose != "")
                into.Insert(OZG_GButton.Make("#pose", OZG_Words.W1("STR_OZG_WIN_POSE", pose), -1, 0));
            into.Insert(OZG_GButton.Make("#stand", "STR_OZG_DWIN_STAND", -1, 0));
        }
        if (host)
            into.Insert(OZG_GButton.Make("#fold", "STR_OZG_BWIN_FOLD", -1, 0));
    }

    // The log (the canvas: lines of 17, 4 between its entries): the newest entries that
    // fit, the oldest of them at the top; an entry longer than the column wraps, three
    // lines at most.
    protected void DWPaintLog()
    {
        BWSetText("OZG_DLogTitle", OZG_Text.Upper(Tr("STR_OZG_BWIN_LOG")));
        Widget logW = BW("OZG_DLog");
        float h = 265;
        if (logW)
        {
            float lw;
            logW.GetSize(lw, h);
            h = h / m_K;
        }
        float room = h - 28 - 8;
        int lastRow = BW_LOG - 1;
        TextWidget probe = BWText("OZG_DLog" + lastRow.ToString());
        // from the newest back, while they fit
        array<ref array<string>> picked = new array<ref array<string>>();
        float used = 0;
        int count = 0;
        for (int k = m_V.Log.Count() - 1; k >= 0; k--)
        {
            string entry = LogWord(m_V.Log[k]);
            array<string> entryLines = new array<string>();
            if (probe && entry != "")
            {
                probe.SetText(entry);
                if (BWTextW(probe) > 336)
                    BWWrap(probe, entry, 336, 3, entryLines);
            }
            if (entryLines.Count() == 0)
                entryLines.Insert(entry);
            float cost = entryLines.Count() * 17 + 4;
            if (used + cost > room + 4 || count + entryLines.Count() > BW_LOG)
                break;
            used = used + cost;
            count = count + entryLines.Count();
            picked.Insert(entryLines);
        }
        int at = 0;
        float y = 28;
        for (int e = picked.Count() - 1; e >= 0; e--)
        {
            array<string> shownLines = picked[e];
            for (int j = 0; j < shownLines.Count(); j++)
            {
                TextWidget line = BWText("OZG_DLog" + at.ToString());
                if (line)
                {
                    line.SetText(shownLines[j]);
                    line.SetPos(12 * m_K, y * m_K);
                    line.Show(true);
                }
                at++;
                y = y + 17;
            }
            y = y + 4;
        }
        while (at < BW_LOG)
        {
            BWShow("OZG_DLog" + at.ToString(), false);
            at++;
        }
    }

    // ------------------------------------------------------------ clicks

    override protected bool DWClick(string n)
    {
        if (n == "OZG_DBtnClose")
        {
            Shut();
            return true;
        }
        // the swap's button and its window: while it is open nothing under it is pressed
        if (DTClick(n))
            return true;
        if (DGClick(n))
            return true;
        if (n == "OZG_DHit")
        {
            DWFeltClick();
            return true;
        }
        if (n.IndexOf("OZG_DAct") == 0 && n.Length() <= 10)
        {
            int k = n.Substring(8, n.Length() - 8).ToInt();
            if (k >= 0 && k < m_DWActMoves.Count())
                DWMove(m_DWActMoves[k]);
            return true;
        }
        if (n == "OZG_DBtnGame")
        {
            if (BWMenuAllowed())
            {
                m_BWMenuOpen = true;
                BoardWinPaint();
            }
            return true;
        }
        if (n == "OZG_DBtnDice")
        {
            if (m_V.IsHost == 1 && !DWPlaying())
                NextVariant();
            return true;
        }
        return false;
    }

    // A click on the felt: a die this player may pick, or his own can, which throws.
    protected void DWFeltClick()
    {
        if (DTBusy())
            return;
        float x;
        float y;
        PointerAt(x, y);
        for (int i = 0; i < m_DWDieX.Count() && i < m_DWDieY.Count(); i++)
        {
            bool onX = Math.AbsFloat(x - m_DWDieX[i]) <= m_DWDieHit;
            bool onY = Math.AbsFloat(y - m_DWDieY[i]) <= m_DWDieHit + 10;
            if (onX && onY)
            {
                int at = m_DWPicked.Find(i);
                if (at >= 0)
                    m_DWPicked.Remove(at);
                else
                    m_DWPicked.Insert(i);
                OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
                BoardWinPaint();
                return;
            }
        }
        if (m_DWCanMove == "")
            return;
        bool inX = x >= m_DWCanX && x <= m_DWCanX + m_DWCanW;
        bool inY = y >= m_DWCanY && y <= m_DWCanY + m_DWCanH;
        if (inX && inY)
            DWAct(m_DWCanMove);
    }

    // A move of the game asked for, unless a throw is still being played.
    protected void DWAct(string move)
    {
        if (move == "" || DTBusy())
            return;
        m_DWPicked.Clear();
        m_DTSwapOpen = false;
        m_DTPickOpen = false;
        Play(move);
        BoardWinPaint();
    }

    // A button's move: the game's, or the table's own ("#...").
    protected void DWMove(string move)
    {
        m_DWPicked.Clear();
        if (move == "#deal")
            Send(OZG_Const.OP_DEAL, "", -1, "", "", "", 0);
        else if (move == "#host")
            Send(OZG_Const.OP_HOST, "", -1, "", "", "", 0);
        else if (move == "#pose")
            Send(OZG_Const.OP_POSE, "", -1, "", "", "", OZG_PoseDriver.Next(m_V));
        else if (move == "#stand")
            Send(OZG_Const.OP_STAND, "", -1, OZG_Const.HOW_LEAVE, "", "", 0);
        else if (move == "#fold")
            m_BWFoldAsk = true;
        else if (move != "#none")
            GenClick(move);
        BoardWinPaint();
    }
}
