// The dice games' window (boards and dice design §6, §7, §8, §9.2): the design canvas's
// boards of thousand, dice poker, pig, high roll and perudo, their shared frame built of
// ozg_dwin.layout by tools/make_board_win.py. A header bar: the game, its rules, the game
// and whose dice. The felt tray: the other players' panels along its top, this player's
// dice in its middle (picked by a click where the game keeps some), the game's zones in a
// row under them. Right: the score, what this player may do (the game's own buttons, a
// number for a bet or a raise), swapping a die before his throw, the log.
//
// The games tell it everything in the later games' view (OZG_GPublic, OZG_GPrivate): a
// seat's line and role, the zones, the buttons. A text of dice is "STR_OZG_DICE_SHOWN|1 3
// 5" and is drawn as dice, each in the look of the set's dice.
modded class OZG_GameMenu
{
    protected static const string DW_TRAY = "OpenZone_Games/gui/boards/ozg_dice_tray_co.paa";
    protected static const string DW_SHOWN = "STR_OZG_DICE_SHOWN";
    // the dice of this player's throw as drawn: their middles (units of the tray), picked
    protected ref array<float> m_DWDieX;
    protected ref array<float> m_DWDieY;
    protected ref array<int> m_DWPicked;
    protected ref array<int> m_DWDice;
    // the action panel's buttons and their moves; the swap: the die carried picked
    protected ref array<string> m_DWActMoves;

    // ------------------------------------------------------------ the later file's part

    // Dice poker, pig, high roll and perudo have faces of their own (the canvas's boards),
    // painted by OZG_GameMenuDiceGames.c, which loads after this file: true when it painted
    // the window; false for a game this file draws (that file hides its faces then).
    protected bool DGPaint()
    {
        return false;
    }

    protected bool DGClick(string n)
    {
        return false;
    }

    // The swap panel at `top` (units of the column), the later file's; its bottom, or
    // `top` when it is not shown.
    protected float DWSwapPanel(float top)
    {
        return top;
    }

    // The look die `k` of a seat's last throw had (the later file reads the view's own
    // list: a die swapped in keeps its colour on the felt).
    protected string DWThrownLook(int seatNo, int k)
    {
        return DWLook();
    }

    // The header's chip, when the game says something else there (high roll: the dice each).
    protected string DGDiceChip()
    {
        return "";
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

    protected string DWLook()
    {
        string look = BWLookOf("Di");
        if (look == "classic" || look == "")
            look = "white";
        return look;
    }

    // ------------------------------------------------------------ painting

    override protected void DWPaint()
    {
        DWInit();
        BWPoolsBegin();
        DWPaintHead();
        ImageWidget tray = ImageWidget.Cast(BW("OZG_DTrayPic"));
        if (tray)
        {
            tray.LoadImageFile(0, DW_TRAY);
            tray.SetImage(0);
        }
        if (DGPaint())
        {
            DWPaintLog();
            BWPoolsEnd();
            BWPaintDialogs();
            return;
        }
        DWPaintSeats();
        DWPaintMine();
        DWPaintZones();
        DWPaintScore();
        DWPaintActions();
        DWPaintSwap();
        DWPaintLog();
        BWPoolsEnd();
        BWPaintDialogs();
    }

    override protected bool DWPlaying()
    {
        return m_GPub && m_V.St == OZG_Const.ST_PLAYING && m_GPub.Ended != 1;
    }

    // A dice game over (the canvas's "Гру закінчено" on every dice board): its last view
    // says so, until another is dealt or chosen.
    override protected bool BWOver()
    {
        if (!DiceWindow())
            return super.BWOver();
        if (!m_GPub || !m_V)
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
                sub = sub + Tr1("STR_OZG_DWIN_SUB_STAKE", stake);
            }
        }
        BWSetText("OZG_DSub", sub);
        float right = 1508;
        TextWidget dice = BWText("OZG_DBtnDiceText");
        Widget diceBtn = BW("OZG_DBtnDice");
        string variant = VariantLabel();
        string chip = DGDiceChip();
        if (dice && diceBtn)
        {
            if (chip != "")
            {
                variant = chip;
                dice.SetText(chip);
            }
            else
            {
                dice.SetText(Tr1("STR_OZG_BWIN_DICE_CHIP", OZG_Text.LowerFirst(variant)));
            }
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

    // The other players' panels along the tray's top: name, the score, the seat's line and
    // role, his dice when the game shows them.
    protected void DWPaintSeats()
    {
        array<ref OZG_GSeat> others = new array<ref OZG_GSeat>();
        if (m_GPub)
        {
            for (int s = 0; s < m_GPub.SeatList.Count(); s++)
            {
                OZG_GSeat g = m_GPub.SeatList[s];
                if (g && g.SeatNo != m_V.Mine)
                    others.Insert(g);
            }
        }
        int n = others.Count();
        if (n > 6)
            n = 6;
        float w = 300;
        if (n > 3)
            w = (964 - (n - 1) * 16.0) / n;
        float gap = 0;
        if (n > 1)
            gap = (964 - n * w) / (n - 1);
        for (int i = 0; i < 6; i++)
        {
            string p = "OZG_DSeat" + i.ToString();
            Widget panel = BW(p);
            if (!panel)
                continue;
            bool shown = i < n;
            panel.Show(shown);
            if (!shown)
                continue;
            OZG_GSeat seat = others[i];
            float x = 68 + i * (w + gap);
            panel.SetPos(x * m_K, 64 * m_K);
            panel.SetSize(w * m_K, 148 * m_K);
            BWFit(BW(p + "BoxEdge"), w, 148, 8);
            Widget bg = BW(p + "BoxBg");
            if (bg)
                BWFit(bg, w - 2, 146, 7);
            BWSetText(p + "Name", SeatName(seat.SeatNo));
            // the score: the line's first number
            array<string> args = new array<string>();
            DWArgs(seat.Line, args);
            string score = "";
            if (args.Count() > 0)
            {
                int num = args[0].ToInt();
                if (num.ToString() == args[0])
                    score = args[0];
            }
            BWSetText(p + "Score", score);
            BWMove(BW(p + "Score"), w - 150, 10);
            // the line and the role as chips; dice drawn as dice
            bool waits = m_GPub.Awaited.Find(seat.SeatNo) >= 0;
            DWChip(p + "Chip0", Words(seat.Line), waits, w - 24);
            array<int> dice = new array<int>();
            bool hasDice = DWDiceOf(seat.Role, dice);
            if (!hasDice && seat.Role != "")
                DWChip(p + "Chip1", Words(seat.Role), false, w - 24);
            else
                BWShow(p + "Chip1", false);
            // the dice it threw last, when the role holds none (the canvas: "останній кидок")
            if (!hasDice && seat.Faces)
            {
                for (int f = 0; f < seat.Faces.Count(); f++)
                    dice.Insert(seat.Faces[f].ToInt());
            }
            DWSeatDice(p, dice, w);
        }
    }

    protected void DWChip(string name, string text, bool hot, float most)
    {
        Widget chip = BW(name);
        TextWidget t = BWText(name + "Text");
        if (!chip || !t)
            return;
        chip.Show(text != "");
        if (text == "")
            return;
        t.SetText(text);
        float tw = BWTextW(t);
        float cw = tw + 16;
        if (cw > most)
            cw = most;
        t.SetSize((cw - 16) * m_K, 15 * m_K);
        chip.SetSize(cw * m_K, 22 * m_K);
        Widget bg = BW(name + "Bg");
        BWFit(bg, cw, 22, 11);
        int fill = ARGB(255, 31, 24, 17);
        int ink = ARGB(255, 184, 165, 133);
        if (hot)
        {
            fill = ARGB(255, 54, 41, 23);
            ink = ARGB(255, 231, 194, 122);
        }
        BWPaint(bg, fill);
        t.SetColor(ink);
    }

    // A seat's dice, 30 a die, 6 apart, its "last throw" after them.
    protected void DWSeatDice(string p, array<int> dice, float w)
    {
        Widget row = BW(p + "Dice");
        if (!row)
            return;
        Widget child = row.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        string look = DWLook();
        // 30 a die, 6 apart (the canvas); smaller on a narrow plate, so that they fit
        int n = dice.Count();
        if (n > 8)
            n = 8;
        float step = 36;
        float room = w - 24;
        if (n > 0 && n * step - 6 > room)
            step = (room + 6) / n;
        float size = step - 6;
        if (size < 14)
            size = 14;
        for (int i = 0; i < n; i++)
        {
            float top = (30 - size) * 0.5;
            string seatDie = "set:ozg_dice image:" + look + "_" + dice[i].ToString();
            if (!BWTurnIn(row, seatDie, i * step, top, size))
                return;
        }
        TextWidget note = BWText(p + "Note");
        if (note)
        {
            note.SetText(Tr("STR_OZG_DWIN_LAST"));
            float noteX = 12 + n * step + 4;
            // the words only where they fit beside the dice
            bool fits = noteX + BWTextW(note) <= w - 12;
            note.Show(n > 0 && fits);
            note.SetPos(noteX * m_K, 115.5 * m_K);
        }
    }

    // The canvas's five spots for thrown dice (x, y in its box, the turn in degrees).
    protected void DWSpots(array<float> xs, array<float> ys, array<float> turns)
    {
        xs.Insert(60);
        ys.Insert(40);
        turns.Insert(-8);
        xs.Insert(220);
        ys.Insert(70);
        turns.Insert(6);
        xs.Insert(380);
        ys.Insert(30);
        turns.Insert(-3);
        xs.Insert(540);
        ys.Insert(80);
        turns.Insert(9);
        xs.Insert(700);
        ys.Insert(45);
        turns.Insert(-6);
    }

    // This player's dice in the tray's middle, 74 a die; picked ones lifted and ringed; the
    // game's words over them.
    protected void DWPaintMine()
    {
        m_DWDice.Clear();
        m_DWDieX.Clear();
        m_DWDieY.Clear();
        if (m_GPub)
        {
            for (int z = 0; z < m_GPub.Zones.Count(); z++)
            {
                OZG_GZone zone = m_GPub.Zones[z];
                if (zone && (zone.Id == "throw" || zone.Id == "dice"))
                    DWDiceOf(zone.Line, m_DWDice);
            }
            // dice poker and perudo: the hand is the seat's own role
            if (m_DWDice.Count() == 0)
            {
                for (int s = 0; s < m_GPub.SeatList.Count(); s++)
                {
                    OZG_GSeat g = m_GPub.SeatList[s];
                    if (g && g.SeatNo == m_V.Mine)
                        DWDiceOf(g.Role, m_DWDice);
                }
            }
            if (m_DWDice.Count() == 0 && m_GOwn && m_GOwn.Held.Count() > 0)
            {
                for (int h = 0; h < m_GOwn.Held.Count(); h++)
                    m_DWDice.Insert(m_GOwn.Held[h].ToInt());
            }
        }
        for (int p = m_DWPicked.Count() - 1; p >= 0; p--)
        {
            if (m_DWPicked[p] >= m_DWDice.Count())
                m_DWPicked.Remove(p);
        }
        // whose throw lies there: the seat awaited, else this player's
        int thrower = m_V.Mine;
        if (m_GPub && m_GPub.Awaited.Count() > 0)
            thrower = m_GPub.Awaited[0];
        int n = m_DWDice.Count();
        float size = 74;
        if (n == 1)
            size = 128;
        float step = size + 14;
        float x0 = 550 - (n * step - 14) / 2;
        float y0 = 488;
        if (n == 1)
            y0 = 314;
        // the canvas throws up to five dice onto five spots of a box at 108, 294 of the
        // felt, each a little turned; more go in a row
        array<float> spotX = new array<float>();
        array<float> spotY = new array<float>();
        array<float> spotR = new array<float>();
        DWSpots(spotX, spotY, spotR);
        bool spots = n >= 2 && n <= 5;
        for (int i = 0; i < n; i++)
        {
            float x = x0 + i * step;
            float y = y0;
            float turn = 0;
            if (spots)
            {
                x = 108 + spotX[i];
                y = 294 + spotY[i];
                turn = spotR[i];
            }
            bool picked = m_DWPicked.Find(i) >= 0;
            if (picked)
                y = y - 14;
            m_DWDieX.Insert(x + size / 2);
            m_DWDieY.Insert(y + size / 2);
            // every die in its own colour: a red one swapped in shows red to everybody
            int thrownFace = m_DWDice[i];
            string thrownLook = DWThrownLook(thrower, i);
            string thrownDie = "set:ozg_dice image:" + thrownLook + "_" + thrownFace.ToString();
            // a picked die as the canvas has it: lifted, its shadow further down, an outline
            // of its own shape four units wide in gold (a round ring stood here until
            // 2026-11-26: a circle about a square die)
            float drop = 6;
            if (picked)
                drop = 14;
            BWTurnPic(BW_FIELD, "set:ozg_dice image:die_s", x, y + drop, size, 29, ARGB(255, 255, 255, 255), turn);
            if (picked)
                BWTurnPic(BW_FIELD, "set:ozg_dice image:white_0", x - 4, y - 4, size + 8, 30, ARGB(255, 231, 194, 122), turn);
            BWTurnPic(BW_FIELD, thrownDie, x, y, size, 31, ARGB(255, 255, 255, 255), turn);
        }
        // the game's words: whose turn, what to do
        string status = "";
        if (m_GOwn && m_GOwn.Status != "")
            status = Words(m_GOwn.Status);
        else if (m_GPub && m_GPub.Status != "")
            status = Words(m_GPub.Status);
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_DCentre0"), status, 620, 2, lines);
        string c0 = "";
        string c1 = "";
        if (lines.Count() > 0)
            c0 = lines[0];
        if (lines.Count() > 1)
            c1 = lines[1];
        // the canvas says it only while no throw lies on the felt (its dice are where the
        // words would be)
        if (spots)
        {
            c0 = "";
            c1 = "";
        }
        BWSetText("OZG_DCentre0", c0);
        BWSetText("OZG_DCentre1", c1);
        float cy = 364;
        if (n == 1)
            cy = 260;
        BWMove(BW("OZG_DCentre0"), 240, cy);
        BWMove(BW("OZG_DCentre1"), 240, cy + 20);
    }

    // The game's other zones in a row under the dice: what is set aside, the pot, the bid,
    // the last throw; dice drawn as dice, a number big, other words as they are.
    protected void DWPaintZones()
    {
        int at = 0;
        float x = 68;
        if (m_GPub)
        {
            for (int z = 0; z < m_GPub.Zones.Count() && at < 3; z++)
            {
                OZG_GZone zone = m_GPub.Zones[z];
                if (!zone || zone.Id == "throw" || zone.Id == "dice")
                    continue;
                string p = "OZG_DZone" + at.ToString();
                Widget box = BW(p);
                if (!box)
                    continue;
                box.Show(true);
                TextWidget label = BWText(p + "Label");
                string labelText = OZG_Text.Upper(Words(zone.Label));
                if (label)
                    label.SetText(labelText);
                float lw = 0;
                if (label)
                    lw = BWTextW(label);
                array<int> dice = new array<int>();
                bool hasDice = DWDiceOf(zone.Line, dice);
                string value = "";
                if (!hasDice)
                {
                    array<string> args = new array<string>();
                    DWArgs(zone.Line, args);
                    if (args.Count() > 0)
                        value = args[args.Count() - 1];
                    if (args.Count() != 1)
                        value = Words(zone.Line);
                }
                TextWidget val = BWText(p + "Value");
                float vw = 0;
                if (val)
                {
                    val.SetText(value);
                    vw = BWTextW(val);
                }
                float w = 14 + lw + 10 + vw + 14;
                if (hasDice)
                    w = 14 + lw + 10 + dice.Count() * 36 + 10;
                if (w < 112)
                    w = 112;
                box.SetPos(x * m_K, 554 * m_K);
                box.SetSize(w * m_K, 64 * m_K);
                BWFit(BW(p + "BoxEdge"), w, 64, 8);
                Widget bg = BW(p + "BoxBg");
                if (bg)
                    BWFit(bg, w - 2, 62, 7);
                if (val)
                {
                    val.SetPos((14 + lw + 10) * m_K, 12 * m_K);
                    val.SetSize((vw + 4) * m_K, 40 * m_K);
                    val.Show(!hasDice);
                }
                Widget diceRow = BW(p + "Dice");
                if (diceRow)
                {
                    diceRow.SetPos((14 + lw + 10) * m_K, 17 * m_K);
                    DWZoneDice(diceRow, dice);
                }
                x = x + w + 14;
                at++;
            }
        }
        while (at < 3)
        {
            BWShow("OZG_DZone" + at.ToString(), false);
            at++;
        }
        string rule = "";
        if (m_V.GameId != "")
        {
            string key = "STR_OZG_DWIN_RULE_" + BWUpper(m_V.GameId);
            rule = Tr(key);
            if (rule == key)
                rule = "";
        }
        TextWidget ruleW = BWText("OZG_DRule");
        if (ruleW)
        {
            ruleW.SetText(rule);
            ruleW.SetPos((x + 4) * m_K, 577.5 * m_K);
            ruleW.SetSize((1080 - x) * m_K, 17 * m_K);
        }
    }

    protected void DWZoneDice(Widget row, array<int> dice)
    {
        Widget child = row.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        string look = DWLook();
        for (int i = 0; i < dice.Count() && i < 6; i++)
        {
            string zoneDie = "set:ozg_dice image:" + look + "_" + dice[i].ToString();
            if (!BWTurnIn(row, zoneDie, i * 36, 0, 30))
                return;
        }
    }

    // The score: a row a seat, this player's lit, his total right.
    protected void DWPaintScore()
    {
        BWSetText("OZG_DScoreTitle", OZG_Text.Upper(Tr("STR_OZG_DWIN_SCORE")));
        int shown = 0;
        if (m_GPub)
        {
            for (int s = 0; s < m_GPub.SeatList.Count() && shown < 6; s++)
            {
                OZG_GSeat g = m_GPub.SeatList[s];
                if (!g)
                    continue;
                string i = shown.ToString();
                bool mine = g.SeatNo == m_V.Mine;
                string name = SeatName(g.SeatNo);
                if (mine)
                    name = name + " " + Tr("STR_OZG_BWIN_YOU");
                BWShow("OZG_DScoreRow" + i, mine);
                BWShow("OZG_DScoreName" + i, !mine);
                BWShow("OZG_DScoreMe" + i, mine);
                BWSetText("OZG_DScoreName" + i, name);
                BWSetText("OZG_DScoreMe" + i, name);
                string note = "";
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

    // What this player may do: the game's buttons -- the main ones wide (two a row), a grid
    // of the small ones (perudo's bids: a row a face) -- and a number for a bet or a raise.
    protected void DWPaintActions()
    {
        float top = 12 + m_DWScoreH + 10;
        Widget panel = BW("OZG_DAct");
        if (panel)
            panel.SetPos(12 * m_K, top * m_K);
        bool myTurn = DWPlaying() && m_GPub.Awaited.Find(m_V.Mine) >= 0;
        string title = Tr("STR_OZG_DWIN_YOUR_THROW");
        if (!myTurn && DWPlaying() && m_GPub.Awaited.Count() > 0)
            title = Tr1("STR_OZG_DWIN_TURN", SeatName(m_GPub.Awaited[0]));
        if (!DWPlaying())
            title = Tr("STR_OZG_DWIN_TABLE");
        BWSetText("OZG_DActTitle", title);
        BWSetText("OZG_DActInfo", "");
        m_DWActMoves.Clear();
        array<ref OZG_GButton> main = new array<ref OZG_GButton>();
        array<ref OZG_GButton> grid = new array<ref OZG_GButton>();
        if (m_GOwn)
        {
            for (int b = 0; b < m_GOwn.Buttons.Count(); b++)
            {
                OZG_GButton btnData = m_GOwn.Buttons[b];
                if (!btnData)
                    continue;
                if (btnData.Row >= 0)
                    grid.Insert(btnData);
                else
                    main.Insert(btnData);
            }
        }
        // the table's own when no game is played: begin, the host's choices
        if (!DWPlaying())
            DWTableButtons(main);
        // thousand's dice to put aside are picked on the felt, as its canvas has it, and
        // one button puts them aside: live when what is picked scores. (Its moves came as
        // a row of small buttons, a move each, their words cut to nothing.)
        bool asides = false;
        bool asideOk = false;
        string asideMove = "set:";
        int g;
        for (g = 0; g < grid.Count(); g++)
        {
            OZG_GButton option = grid[g];
            if (option.Move.IndexOf("set:") == 0)
                asides = true;
        }
        if (asides)
        {
            array<int> pickedAt = new array<int>();
            pickedAt.Copy(m_DWPicked);
            pickedAt.Sort();
            array<int> pickedFaces = new array<int>();
            for (g = 0; g < pickedAt.Count(); g++)
            {
                int at = pickedAt[g];
                if (g > 0)
                    asideMove = asideMove + ",";
                int place = at + 1;
                asideMove = asideMove + place.ToString();
                if (at >= 0 && at < m_DWDice.Count())
                    pickedFaces.Insert(m_DWDice[at]);
            }
            pickedFaces.Sort();
            string pickedKey = DWFacesKey(pickedFaces);
            for (g = 0; g < grid.Count() && pickedAt.Count() > 0; g++)
            {
                string optionMove = grid[g].Move;
                if (optionMove.IndexOf("set:") != 0)
                    continue;
                array<int> optionFaces = new array<int>();
                DWSetFaces(optionMove, optionFaces);
                optionFaces.Sort();
                if (DWFacesKey(optionFaces) == pickedKey)
                    asideOk = true;
            }
            grid.Clear();
            string asideGoes = "#none";
            if (asideOk)
                asideGoes = asideMove;
            OZG_GButton aside = OZG_GButton.Make(asideGoes, "STR_OZG_DWIN_TH_ASIDE", -1, 0);
            if (asideOk)
                aside.Hot = 1;
            main.InsertAt(aside, 0);
            BWSetText("OZG_DActInfo", Tr("STR_OZG_DWIN_TH_PICK"));
        }
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
        // the grid: a row of small buttons a Row value
        int rowAt = -1;
        float gx = 12;
        for (i = 0; i < grid.Count() && used < 16; i++)
        {
            OZG_GButton gb = grid[i];
            if (gb.Row != rowAt)
            {
                if (rowAt >= 0)
                    y = y + 34;
                rowAt = gb.Row;
                gx = 12;
            }
            DWActButton(used, gb, gx, y, 52, 28);
            gx = gx + 56;
            used++;
        }
        if (grid.Count() > 0)
            y = y + 34;
        while (used < 16)
        {
            BWShow("OZG_DAct" + used.ToString(), false);
            used++;
        }
        // a number to type: a bet, a raise
        bool amount = m_GOwn && m_GOwn.AmountMove != "";
        BWShow("OZG_DAmount", amount);
        if (amount)
        {
            BWMove(BW("OZG_DAmount"), 12, y);
            BWSetText("OZG_DAmountBtnText", Words(m_GOwn.AmountLabel));
            EditBoxWidget edit = EditBoxWidget.Cast(BW("OZG_DAmountEdit"));
            if (edit && edit.GetText() == "")
                edit.SetText(m_GOwn.AmountStart.ToString());
            y = y + 50;
        }
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
        if (data.Hot == 1)
        {
            bg = ARGB(255, 201, 164, 92);
            ink = ARGB(255, 26, 18, 10);
        }
        BWPaintNamed(name + "BoxBg", bg);
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

    // Swapping a die before this player's throw: the panel the four faces have (the later
    // file's DGPaintSwap: every die he carries as a picture in its colour, the dice he
    // throws, one of each picked by him), under the actions; the log under it. Until
    // 2026-11-26 thousand had a panel of its own with room for four pocket dice, each
    // drawn as a six: a fifth die carried could not be picked at all.
    protected void DWPaintSwap()
    {
        float top = m_DWActBottom + 10;
        float bottom = DWSwapPanel(top);
        float logTop = top;
        if (bottom > top)
            logTop = bottom + 10;
        Widget logW = BW("OZG_DLog");
        if (logW)
        {
            logW.SetPos(12 * m_K, logTop * m_K);
            logW.SetSize(360 * m_K, (762 - logTop) * m_K);
        }
        BWFit(BW("OZG_DLogBg"), 360, 762 - logTop, 6);
    }

    // The log, the newest lines that fit, 21 a line.
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
        if (DGClick(n))
            return true;
        if (n == "OZG_DBtnClose")
        {
            Shut();
            return true;
        }
        if (n == "OZG_DHit")
        {
            float x;
            float y;
            PointerAt(x, y);
            float tx = x - 52;
            float ty = y - 86;
            for (int i = 0; i < m_DWDieX.Count(); i++)
            {
                bool onX = Math.AbsFloat(tx - m_DWDieX[i]) <= 40;
                bool onY = i >= m_DWDieY.Count() || Math.AbsFloat(ty - m_DWDieY[i]) <= 70;
                if (onX && onY)
                {
                    int at = m_DWPicked.Find(i);
                    if (at >= 0)
                        m_DWPicked.Remove(at);
                    else
                        m_DWPicked.Insert(i);
                    BoardWinPaint();
                    return true;
                }
            }
            return true;
        }
        if (n.IndexOf("OZG_DAct") == 0 && n.Length() <= 10)
        {
            int k = n.Substring(8, n.Length() - 8).ToInt();
            if (k >= 0 && k < m_DWActMoves.Count())
                DWMove(m_DWActMoves[k]);
            return true;
        }
        if (n == "OZG_DAmountBtn")
        {
            EditBoxWidget edit = EditBoxWidget.Cast(BW("OZG_DAmountEdit"));
            if (m_GOwn && edit)
            {
                string typed = edit.GetText();
                int amount = typed.ToInt();
                Play(m_GOwn.AmountMove + amount.ToString());
            }
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
