// Nardy in the board's window (boards and dice design §5.5 to §5.7, §7): the design
// canvas's boards "Довгі нарди" and "Короткі нарди", built of ozg_bwin_nardy.layout by
// tools/make_board_win.py. The board's other side: 24 points, the bar between the halves,
// the dice of the throw on the right half; left the game, the other player and the
// throw (its button, swapping a die before it); right the log, the discs borne off of
// each side, sleight of hand, the draw and the resignation, the player himself.
//
// The game counts the points 1 to 24 the way White does, White moving down to 1; the
// canvas lays a point q = 24 - point out from the top right corner leftwards and back
// along the bottom (Black sees the board turned half round). A move is picked a disc, then
// a point; bearing off is the player's own tray, right.
modded class OZG_GameMenu
{
    // The board's measures in the window (the canvas): its corner, the frame, a half, the
    // bar, a point's width, a checker.
    protected static const float BN_X = 360.0;
    protected static const float BN_Y = 10.0;
    protected static const float BN_EDGE = 55.0;
    protected static const float BN_HALF = 363.0;
    protected static const float BN_BAR = 44.0;
    protected static const float BN_PW = 60.5;
    protected static const float BN_CK = 54.0;

    // The position as read: the discs on each point (White up, Black down; point 1 at
    // [0]), on the bar and borne off, White first.
    protected ref array<int> m_BNPts;
    protected int m_BNBarW;
    protected int m_BNBarB;
    protected int m_BNOffW;
    protected int m_BNOffB;
    // The point picked to move from (1 .. 24, 25 the bar), -1 for none.
    protected int m_BNSel = -1;
    // Swapping a die (§9.2): 0 shut, 1 choosing the die carried, 2 choosing the die it
    // replaces; the key of the die carried.
    protected int m_BNSwapStep;
    protected string m_BNSwapKey = "";
    protected ref array<string> m_BNSwapKeys;
    // "Put back" at nardy picks the tray first: "w" or "b".
    protected string m_BNSlWho = "";

    protected bool BWNardy()
    {
        return m_V && (m_V.GameId == OZG_NardyConst.LONG_ID || m_V.GameId == OZG_NardyConst.SHORT_ID);
    }

    // ------------------------------------------------------------ reading

    protected void BNRead()
    {
        if (!m_BNPts)
            m_BNPts = new array<int>();
        if (!m_BNSwapKeys)
            m_BNSwapKeys = new array<string>();
        m_BNPts.Clear();
        m_BNBarW = 0;
        m_BNBarB = 0;
        m_BNOffW = 0;
        m_BNOffB = 0;
        for (int z = 0; z < 24; z++)
            m_BNPts.Insert(0);
        if (!m_BPub || m_BPub.Points == "")
            return;
        array<string> parts = new array<string>();
        m_BPub.Points.Split("|", parts);
        if (parts.Count() < 3)
            return;
        array<string> pts = new array<string>();
        parts[0].Split(",", pts);
        for (int i = 0; i < 24 && i < pts.Count(); i++)
            m_BNPts[i] = pts[i].ToInt();
        array<string> bar = new array<string>();
        parts[1].Split(",", bar);
        if (bar.Count() == 2)
        {
            m_BNBarW = bar[0].ToInt();
            m_BNBarB = bar[1].ToInt();
        }
        array<string> off = new array<string>();
        parts[2].Split(",", off);
        if (off.Count() == 2)
        {
            m_BNOffW = off[0].ToInt();
            m_BNOffB = off[1].ToInt();
        }
    }

    // The position in the game's own class, for the pips left to go.
    protected OZG_NardyBoard BNBoard()
    {
        OZG_NardyBoard b = new OZG_NardyBoard();
        b.SetUp(m_V.GameId == OZG_NardyConst.LONG_ID);
        for (int i = 0; i < 24; i++)
            b.Pts[i] = m_BNPts[i];
        b.Bar[0] = m_BNBarW;
        b.Bar[1] = m_BNBarB;
        b.Off[0] = m_BNOffW;
        b.Off[1] = m_BNOffB;
        return b;
    }

    protected int BNPips(int side)
    {
        OZG_NardyBoard b = BNBoard();
        int pips = 0;
        for (int g = 1; g <= 24; g++)
            pips = pips + b.PosOf(side, g) * b.Mine(side, g);
        if (side == 0)
            pips = pips + m_BNBarW * OZG_NardyConst.BAR_POS;
        else
            pips = pips + m_BNBarB * OZG_NardyConst.BAR_POS;
        return pips;
    }

    // This seat's steps: "m:<from>:<to>:<die>".
    protected void BNSteps(array<string> into)
    {
        into.Clear();
        if (!m_BOwn || !m_BOwn.Buttons)
            return;
        for (int i = 0; i < m_BOwn.Buttons.Count(); i++)
        {
            OZG_GButton b = m_BOwn.Buttons[i];
            if (b && b.Move.IndexOf("m:") == 0)
                into.Insert(b.Move);
        }
    }

    // A step's two ends: a point (1 .. 24), 25 the bar, 0 off.
    protected void BNEnds(string step, out int from, out int to)
    {
        from = -1;
        to = -1;
        array<string> p = new array<string>();
        step.Split(":", p);
        if (p.Count() != 4)
            return;
        if (p[1] == OZG_NardyConst.BAR)
            from = OZG_NardyConst.BAR_POS;
        else
            from = p[1].ToInt();
        if (p[2] == OZG_NardyConst.OFF)
            to = 0;
        else
            to = p[2].ToInt();
    }

    // ------------------------------------------------------------ where things are

    // The canvas's place of a point: its column (0 .. 11, left to right) and its row.
    protected void BNWhere(int point, out int col, out bool top)
    {
        int q = 24 - point;
        if (m_BFlip)
            q = (q + 12) % 24;
        if (q < 12)
        {
            col = 11 - q;
            top = true;
            return;
        }
        col = q - 12;
        top = false;
    }

    protected float BNColX(int col)
    {
        if (col < 6)
            return BN_X + BN_EDGE + col * BN_PW;
        return BN_X + BN_EDGE + BN_HALF + BN_BAR + (col - 6) * BN_PW;
    }

    // The point under a place of the window (units): 1 .. 24, 25 the bar, -1 none.
    protected int BNPointAt(float x, float y)
    {
        bool top;
        if (y >= BN_Y + BN_EDGE && y <= BN_Y + BN_EDGE + 360)
            top = true;
        else if (y >= BN_Y + 880 - BN_EDGE - 360 && y <= BN_Y + 880 - BN_EDGE)
            top = false;
        else
            return -1;
        float barX = BN_X + BN_EDGE + BN_HALF;
        if (x >= barX && x < barX + BN_BAR)
            return OZG_NardyConst.BAR_POS;
        int col = -1;
        if (x >= BN_X + BN_EDGE && x < barX)
            col = (x - BN_X - BN_EDGE) / BN_PW;
        else if (x >= barX + BN_BAR && x < barX + BN_BAR + 6 * BN_PW)
            col = 6 + (x - barX - BN_BAR) / BN_PW;
        if (col < 0 || col > 11)
            return -1;
        int q = 12 + col;
        if (top)
            q = 11 - col;
        if (m_BFlip)
            q = (q + 12) % 24;
        return 24 - q;
    }

    // ------------------------------------------------------------ painting

    protected void BWNardyPaint()
    {
        BNRead();
        if (!BWPlaying())
        {
            m_BNSel = -1;
            m_BNSwapStep = 0;
        }
        BWPoolsBegin();
        m_BNBadgesUsed = 0;
        BNPaintTitle();
        BNPaintPlate("OZG_BOpp", BWTopSeat(), false);
        BNPaintPlate("OZG_BMe", BWBottomSeat(), true);
        BWPaintLog();
        BNPaintThrow();
        BNPaintOff();
        BNPaintActions();
        BNPaintField();
        BWPoolsEnd();
        if (m_BNBadges)
        {
            for (int b = m_BNBadgesUsed; b < m_BNBadges.Count(); b++)
            {
                if (m_BNBadges[b])
                    m_BNBadges[b].Show(false);
            }
        }
        BWPaintDialogs();
    }

    protected void BNPaintTitle()
    {
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        string game = "";
        if (sample)
            game = Tr(sample.LabelKey());
        BWSetText("OZG_BTitle", game);
        string sub = Tr("STR_OZG_BWIN_NARDY_SUB_LONG");
        if (m_V.GameId == OZG_NardyConst.SHORT_ID)
            sub = Tr("STR_OZG_BWIN_NARDY_SUB_SHORT");
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_BSub0"), sub, 302, 2, lines);
        string s0 = "";
        string s1 = "";
        if (lines.Count() > 0)
            s0 = lines[0];
        if (lines.Count() > 1)
            s1 = lines[1];
        BWSetText("OZG_BSub0", s0);
        BWSetText("OZG_BSub1", s1);
        TextWidget label = BWText("OZG_BBtnGameText");
        if (label)
        {
            label.SetText(Tr1("STR_OZG_BWIN_GAME", game));
            float tw = BWTextW(label);
            Widget chev = BW("OZG_BBtnGameChev");
            if (chev)
            {
                chev.Show(BWMenuAllowed());
                BWMove(chev, 30 + 65.5 + tw / 2 + 6, 122);
            }
        }
        // whose dice: the game's first option; the host changes it between games
        BWSetText("OZG_BBtnDiceText", Tr1("STR_OZG_BWIN_DICE_CHIP", VariantLabel()));
    }

    protected void BNPaintPlate(string p, int seat, bool bottom)
    {
        bool mine = bottom && seat >= 0 && seat == m_V.Mine;
        bool taken = BWTaken(seat);
        string name = Tr("STR_OZG_BWIN_FREE_SEAT");
        if (taken)
            name = SeatName(seat);
        TextWidget nameW = BWText(p + "Name");
        if (nameW)
            nameW.SetText(name);
        BWShow(p + "You", mine);
        if (mine && nameW)
            BWMove(BW(p + "You"), 68 + BWTextW(nameW) + 4, 14.5);
        bool white = bottom;
        if (m_BPub && m_BPub.White >= 0)
            white = seat == m_BPub.White;
        int side = 1;
        if (white)
            side = 0;
        string colour = Tr("STR_OZG_BWIN_SIDE_BLACK");
        if (white)
            colour = Tr("STR_OZG_BWIN_SIDE_WHITE");
        string line = colour;
        if (m_BPub && m_BPub.White >= 0)
            line = Tr2("STR_OZG_BWIN_TO_GO", colour, BNPips(side).ToString());
        BWSetText(p + "Side", line);
        ImageWidget av = ImageWidget.Cast(BW(p + "AvPic"));
        if (av)
        {
            av.LoadImageFile(0, BNTop(white, false));
            av.SetImage(0);
            av.Show(taken);
        }
        // the chip and the ring, as at chess
        bool turn = BWPlaying() && m_BPub.Turn == seat;
        string chip = Tr("STR_OZG_BWIN_WATCHING");
        int chipBg = ARGB(255, 40, 43, 28);
        int chipFg = ARGB(255, 166, 217, 160);
        int ring = ARGB(255, 46, 34, 23);
        if (!taken)
        {
            chip = Tr("STR_OZG_BWIN_WAITING");
            chipBg = ARGB(255, 45, 36, 27);
            chipFg = ARGB(255, 184, 165, 133);
        }
        else if (mine)
        {
            chip = Tr("STR_OZG_BWIN_THEIR_MOVE");
            chipBg = ARGB(255, 45, 36, 27);
            chipFg = ARGB(255, 184, 165, 133);
            if (BWOver())
                chip = Tr("STR_OZG_BWIN_OVER");
            else if (!BWPlaying())
                chip = Tr("STR_OZG_BWIN_WAITING");
            else if (turn)
            {
                chip = Tr("STR_OZG_BWIN_YOUR_MOVE");
                chipBg = ARGB(255, 61, 48, 29);
                chipFg = ARGB(255, 231, 194, 122);
                ring = ARGB(255, 201, 164, 92);
            }
        }
        else if (!BWLooks(seat))
        {
            chip = Tr("STR_OZG_BWIN_AWAY");
            chipBg = ARGB(255, 57, 42, 21);
            chipFg = ARGB(255, 240, 196, 106);
        }
        else if (turn)
        {
            chip = Tr("STR_OZG_BWIN_THINKING");
            ring = ARGB(255, 138, 106, 62);
        }
        BWPaintNamed(p + "PlateEdge", ring);
        TextWidget chipText = BWText(p + "ChipText");
        Widget chipBox = BW(p + "Chip");
        if (chipText && chipBox)
        {
            chipText.SetText(chip);
            chipText.SetColor(chipFg);
            float tw = BWTextW(chipText);
            float cw = 10 + tw + 10;
            chipText.SetSize((tw + 2) * m_K, 15 * m_K);
            chipBox.SetPos((318 - cw) * m_K, 21 * m_K);
            chipBox.SetSize(cw * m_K, 24 * m_K);
            Widget pill = BW(p + "ChipBg");
            BWFit(pill, cw, 24, 12);
            BWPaint(pill, chipBg);
        }
    }

    // A checker seen from above: the disc's top in the look of the set's discs.
    protected string BNTop(bool white, bool shadow)
    {
        if (shadow)
            return "set:ozg_discs_top image:top_s";
        string face = "bD";
        if (white)
            face = "wD";
        string look = BWLookOf(face);
        string side = "b";
        if (white)
            side = "w";
        if (look == "stalker")
            return "set:ozg_discs_top image:top_stalker_" + side;
        return "set:ozg_discs_top image:top_classic_" + side;
    }

    // The throw: whose dice, what to do now, the throw's button, the swap's.
    protected void BNPaintThrow()
    {
        BWSetText("OZG_BThrowWhose", VariantLabel());
        string text = "";
        bool myTurn = BWPlaying() && m_BPub.Turn == m_V.Mine;
        bool canThrow = myTurn && BWButton(OZG_NardyConst.M_THROW);
        array<string> steps = new array<string>();
        BNSteps(steps);
        if (BWOver())
        {
            text = Tr("STR_OZG_BWIN_N_OVER");
        }
        else if (BWPlaying() && myTurn && canThrow)
        {
            text = Tr("STR_OZG_BWIN_N_YOUR_THROW");
        }
        else if (BWPlaying() && myTurn)
        {
            text = Tr1("STR_OZG_BWIN_N_MOVE", m_BPub.DiceLeft);
        }
        else if (BWPlaying())
        {
            int other = m_BPub.Turn;
            if (!BWLooks(other))
                text = Tr1("STR_OZG_BWIN_N_AWAY", SeatName(other));
            else
                text = Tr1("STR_OZG_BWIN_N_THEIRS", SeatName(other));
        }
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_BThrowText0"), text, 306, 2, lines);
        string t0 = "";
        string t1 = "";
        if (lines.Count() > 0)
            t0 = lines[0];
        if (lines.Count() > 1)
            t1 = lines[1];
        BWSetText("OZG_BThrowText0", t0);
        BWSetText("OZG_BThrowText1", t1);
        // the throw's button: gold when it may be pressed
        int bg = ARGB(255, 58, 44, 28);
        int fg = ARGB(255, 160, 143, 115);
        if (canThrow)
        {
            bg = ARGB(255, 201, 164, 92);
            fg = ARGB(255, 26, 18, 10);
        }
        BWPaintNamed("OZG_BBtnThrowBoxBg", bg);
        BWInk("OZG_BBtnThrowText", fg);
        // the swap: before one's own throw, while a die may still be swapped
        bool swap = canThrow && m_V.BoardSleights >= 0 && OZG_Cfg.Get().Cheating == 1;
        BWShow("OZG_BBtnSwap", swap);
        int swapBg = ARGB(255, 45, 33, 23);
        if (m_BNSwapStep > 0)
            swapBg = ARGB(255, 201, 164, 92);
        BWPaintNamed("OZG_BBtnSwapBoxBg", swapBg);
        if (!swap)
            m_BNSwapStep = 0;
        BNPaintSwap();
    }

    // The swap's chooser: the dice this player carries, then which of the dice he is
    // about to throw goes out.
    protected void BNPaintSwap()
    {
        BWShow("OZG_BSwap", m_BNSwapStep > 0);
        if (m_BNSwapStep <= 0)
            return;
        array<string> labels = new array<string>();
        m_BNSwapKeys.Clear();
        string hint = "";
        if (m_BNSwapStep == 1)
        {
            hint = Tr("STR_OZG_BWIN_SWAP_WHICH");
            array<EntityAI> dice = new array<EntityAI>();
            BNCarriedDice(dice);
            for (int i = 0; i < dice.Count() && labels.Count() < 4; i++)
            {
                m_BNSwapKeys.Insert(OZG_Keys.Of(dice[i]));
                labels.Insert(Widget.TranslateString(dice[i].GetDisplayName()));
            }
            if (labels.Count() == 0)
                hint = Tr("STR_OZG_BWIN_SWAP_NONE");
        }
        else
        {
            hint = Tr("STR_OZG_BWIN_SWAP_FOR");
            for (int d = 0; d < 2; d++)
            {
                int n = d + 1;
                m_BNSwapKeys.Insert(d.ToString());
                labels.Insert(Tr1("STR_OZG_BWIN_SWAP_DIE", n.ToString()));
            }
        }
        BWSetText("OZG_BSwapHint", hint);
        for (int k = 0; k < 4; k++)
        {
            string name = "OZG_BSwapPick" + k.ToString();
            bool shown = k < labels.Count();
            BWShow(name, shown);
            if (shown)
                BWSetText(name + "Text", labels[k]);
        }
    }

    // The dice this player carries (pockets, a can in his hands), not the place's.
    protected void BNCarriedDice(array<EntityAI> into)
    {
        into.Clear();
        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player)
            return;
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        for (int i = 0; i < items.Count(); i++)
        {
            if (items[i] && items[i].IsInherited(OZG_Die_Base))
                into.Insert(items[i]);
        }
    }

    protected void BNSwapPick(int k)
    {
        if (k < 0 || k >= m_BNSwapKeys.Count())
            return;
        if (m_BNSwapStep == 1)
        {
            m_BNSwapKey = m_BNSwapKeys[k];
            m_BNSwapStep = 2;
            BoardWinPaint();
            return;
        }
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = m_PlaceId;
        ask.CardKey = m_BNSwapKey;
        ask.SeatNo = m_BNSwapKeys[k].ToInt();
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_DSWAP, json);
        m_BNSwapStep = 0;
        m_BNSwapKey = "";
        BoardWinPaint();
    }

    // The two trays: the other side's left, this player's right (the button to bear off).
    protected void BNPaintOff()
    {
        bool meWhite = !m_BFlip;
        int mineOff = m_BNOffW;
        int theirsOff = m_BNOffB;
        string mineTitle = Tr("STR_OZG_BWIN_OFF_W");
        string theirsTitle = Tr("STR_OZG_BWIN_OFF_B");
        if (!meWhite)
        {
            mineOff = m_BNOffB;
            theirsOff = m_BNOffW;
            mineTitle = Tr("STR_OZG_BWIN_OFF_B");
            theirsTitle = Tr("STR_OZG_BWIN_OFF_W");
        }
        BWSetText("OZG_BOffBTitle", OZG_Text.Upper(theirsTitle));
        BWSetText("OZG_BOffWTitle", OZG_Text.Upper(mineTitle));
        BWSetText("OZG_BOffBCount", Tr1("STR_OZG_BWIN_OFF_COUNT", theirsOff.ToString()));
        BWSetText("OZG_BOffWCount", Tr1("STR_OZG_BWIN_OFF_COUNT", mineOff.ToString()));
        BNOffStack("OZG_BOffBStack", theirsOff, !meWhite);
        BNOffStack("OZG_BOffWStack", mineOff, meWhite);
        // my tray lit when a disc may be borne off there, or a disc put back from it
        bool lit = m_BWSl == OZG_DuelConst.S_BACK;
        array<string> steps = new array<string>();
        BNSteps(steps);
        for (int i = 0; i < steps.Count() && !lit; i++)
        {
            int from;
            int to;
            BNEnds(steps[i], from, to);
            if (from == m_BNSel && to == 0)
                lit = true;
        }
        int ring = ARGB(255, 46, 34, 23);
        if (lit)
            ring = ARGB(255, 231, 194, 122);
        BWPaintNamed("OZG_BOffWBoxEdge", ring);
    }

    // The discs borne off, a bar each, from the bottom up (the canvas: 64 x 6, 2 apart).
    protected void BNOffStack(string name, int n, bool white)
    {
        Widget stack = BW(name);
        if (!stack)
            return;
        Widget child = stack.GetChildren();
        while (child)
        {
            Widget next = child.GetSibling();
            child.Unlink();
            child = next;
        }
        float pitch = 8;
        if (n > 6)
            pitch = 52.0 / n;
        int fill = ARGB(255, 44, 38, 34);
        if (white)
            fill = ARGB(255, 232, 223, 204);
        for (int i = 0; i < n; i++)
        {
            Widget bar = MakeLayout(LAYOUT_BWTINT, stack, "OZG_BWTint");
            if (!bar)
                return;
            bar.SetColor(fill);
            bar.SetPos(0, (52 - (i + 1) * pitch) * m_K);
            bar.SetSize(64 * m_K, (pitch - 2) * m_K);
            bar.Show(true);
        }
    }

    protected void BNPaintActions()
    {
        bool sl = BWPlaying() && m_V.BoardSleights == 1;
        BWShow("OZG_BSl", sl);
        if (sl)
            BWPaintSleights();
        string left = "";
        string right = "";
        string leftKey = "STR_OZG_BWIN_DRAW";
        string rightKey = "STR_OZG_BWIN_RESIGN";
        if (BWPlaying() && m_BOwn)
        {
            if (BWButton("accept"))
            {
                left = "accept";
                leftKey = "STR_OZG_BOARD_BTN_ACCEPT";
                right = "decline";
                rightKey = "STR_OZG_BOARD_BTN_DECLINE";
            }
            else
            {
                if (BWButton("draw"))
                    left = "draw";
                if (BWButton("resign"))
                    right = "resign";
            }
        }
        BWShow("OZG_BBtnDraw", left != "");
        BWShow("OZG_BBtnResign", right != "");
        BWSetText("OZG_BBtnDrawText", Tr(leftKey));
        BWSetText("OZG_BBtnResignText", Tr(rightKey));
        m_BWDrawMove = left;
        m_BWResignMove = right;
        BWPaintHostRow();
    }

    // The board: the checkers on their points, the bar, the counts, what may be picked,
    // the dice of the throw.
    protected void BNPaintField()
    {
        ImageWidget board = ImageWidget.Cast(BW("OZG_BBoard"));
        if (board)
        {
            board.LoadImageFile(0, "OpenZone_Games/gui/boards/ozg_board_win_nardy_co.paa");
            board.SetImage(0);
        }
        if (!m_BPub)
            return;
        array<string> steps = new array<string>();
        BNSteps(steps);
        // where the picked disc may go
        array<int> targets = new array<int>();
        array<int> froms = new array<int>();
        for (int s = 0; s < steps.Count(); s++)
        {
            int f;
            int t;
            BNEnds(steps[s], f, t);
            if (froms.Find(f) < 0)
                froms.Insert(f);
            if (f == m_BNSel && targets.Find(t) < 0)
                targets.Insert(t);
        }
        for (int point = 1; point <= 24; point++)
        {
            int c = m_BNPts[point - 1];
            int n = Math.AbsInt(c);
            int col;
            bool top;
            BNWhere(point, col, top);
            float px = BNColX(col);
            bool picked = point == m_BNSel;
            bool target = targets.Find(point) >= 0;
            bool firstSl = m_BWSl != "" && m_BWSlFirst == point;
            // a lit point: picked, a target, a sleight's first
            if (picked || target || firstSl)
            {
                float my = BN_Y + 880 - BN_EDGE - 323;
                if (top)
                    my = BN_Y + BN_EDGE;
                int bg = ARGB(26, 231, 194, 122);
                if (target)
                    bg = ARGB(46, 231, 194, 122);
                BWTint(BW_FIELD, px - BN_X, my - BN_Y, 60, 323, 2, bg);
                BWRing(BW_FIELD, px - BN_X, my - BN_Y, 60, 323, 2, 3, ARGB(179, 231, 194, 122));
            }
            float step = BN_CK;
            if (n > 5)
                step = (318 - BN_CK) / (n - 1);
            float cx = px + (BN_PW - BN_CK) / 2;
            bool white = c > 0;
            for (int k = 0; k < n; k++)
            {
                float cy = BN_Y + 880 - BN_EDGE - 2 - BN_CK - k * step;
                if (top)
                    cy = BN_Y + BN_EDGE + 2 + k * step;
                BWPic(BW_FIELD, BNTop(white, true), cx - BN_X, cy - BN_Y + 3, BN_CK, BN_CK, 10 + k * 2, ARGB(255, 255, 255, 255));
                BWPic(BW_FIELD, BNTop(white, false), cx - BN_X, cy - BN_Y, BN_CK, BN_CK, 11 + k * 2, ARGB(255, 255, 255, 255));
                if (picked && k == n - 1)
                    BWPic(BW_FIELD, BW_TEX + "ozg_ring6_ca.paa", cx - BN_X - 3, cy - BN_Y - 3, BN_CK + 6, BN_CK + 6, 12 + k * 2, ARGB(255, 231, 194, 122));
            }
            if (n > 5)
            {
                float by = BN_Y + 880 - BN_EDGE - 2 - BN_CK - 4 * step + 16;
                if (top)
                    by = BN_Y + BN_EDGE + 2 + 4 * step + 16;
                BNBadge(n, cx + 12 - BN_X, by - BN_Y);
            }
            // a dot where a move or a sleight may land
            bool dot = target || BNSlCandidate(point);
            if (dot)
            {
                float dy = BN_Y + 880 - BN_EDGE - 360 + Math.Max(4, 360 - 34 - n * BN_CK);
                if (top)
                    dy = BN_Y + BN_EDGE + Math.Min(330, 10 + n * BN_CK);
                BWPic(BW_FIELD, BW_TEX + "ozg_disc_ca.paa", px + 15 - BN_X, dy - 3 - BN_Y, 30, 30, 50, ARGB(153, 20, 12, 6));
                BWPic(BW_FIELD, BW_TEX + "ozg_disc_ca.paa", px + 18 - BN_X, dy - BN_Y, 24, 24, 51, ARGB(235, 231, 194, 122));
            }
        }
        // the bar (short nardy): white from the middle down, black up
        float barX = BN_X + BN_EDGE + BN_HALF + BN_BAR / 2 - BN_CK / 2;
        int i;
        for (i = 0; i < m_BNBarW; i++)
        {
            float wy = BN_Y + 470 + i * 34;
            if (m_BFlip)
                wy = BN_Y + 356 - i * 34;
            BWPic(BW_FIELD, BNTop(true, false), barX - BN_X, wy - BN_Y, BN_CK, BN_CK, 30 + i, ARGB(255, 255, 255, 255));
        }
        for (i = 0; i < m_BNBarB; i++)
        {
            float ky = BN_Y + 356 - i * 34;
            if (m_BFlip)
                ky = BN_Y + 470 + i * 34;
            BWPic(BW_FIELD, BNTop(false, false), barX - BN_X, ky - BN_Y, BN_CK, BN_CK, 30 + i, ARGB(255, 255, 255, 255));
        }
        BNPaintDice();
    }

    protected ref array<Widget> m_BNBadges;
    protected int m_BNBadgesUsed;

    // How many discs a crowded point holds: a dark pill with the number.
    protected void BNBadge(int n, float x, float y)
    {
        if (!m_BNBadges)
            m_BNBadges = new array<Widget>();
        Widget b;
        if (m_BNBadgesUsed < m_BNBadges.Count())
        {
            b = m_BNBadges[m_BNBadgesUsed];
        }
        else
        {
            b = MakeLayout("OpenZone_Games/gui/layouts/ozg_bwbadge.layout", m_BWPoolParent[BW_FIELD], "OZG_BWBadge");
            if (!b)
                return;
            m_BNBadges.Insert(b);
        }
        m_BNBadgesUsed++;
        TextWidget t = TextWidget.Cast(b.FindAnyWidget("OZG_BWBadgeText"));
        if (t)
            t.SetText(n.ToString());
        b.SetSort(60);
        b.SetPos(x * m_K, y * m_K);
        b.Show(true);
    }

    // The dice of the throw on the board's right half; those played dim.
    protected void BNPaintDice()
    {
        array<int> thrown = new array<int>();
        array<int> left = new array<int>();
        OZG_DiceText.Parse(m_BPub.Dice, thrown);
        OZG_DiceText.Parse(m_BPub.DiceLeft, left);
        if (thrown.Count() == 0)
            return;
        string look = BWLookOf("Di");
        if (look == "classic")
            look = "white";
        int used = 4 - left.Count();
        if (thrown.Count() < 2 || thrown[0] != thrown[1])
            used = thrown.Count() - left.Count();
        for (int i = 0; i < 2 && i < thrown.Count(); i++)
        {
            int v = thrown[i];
            bool spent;
            if (thrown.Count() >= 2 && thrown[0] == thrown[1])
                spent = used >= (i + 1) * 2;
            else
                spent = left.Find(v) < 0;
            int alpha = 255;
            if (spent)
                alpha = 89;
            float x = 920 + i * 78;
            BWPic(BW_FIELD, "set:ozg_dice image:die_s", x - BN_X, 418 + 6 - BN_Y, 62, 62, 70, ARGB(alpha, 255, 255, 255));
            BWPic(BW_FIELD, "set:ozg_dice image:" + look + "_" + v.ToString(), x - BN_X, 418 - BN_Y, 62, 62, 71, ARGB(alpha, 255, 255, 255));
        }
    }

    // ------------------------------------------------------------ sleight of hand at nardy

    protected bool BNSlCandidate(int point)
    {
        if (m_BWSl == "" || point < 1 || point > 24)
            return false;
        int n = Math.AbsInt(m_BNPts[point - 1]);
        if (m_BWSl == OZG_DuelConst.S_OFF)
            return n > 0;
        if (m_BWSl == OZG_DuelConst.S_MOVE)
        {
            if (m_BWSlFirst < 0)
                return n > 0;
            return point != m_BWSlFirst;
        }
        if (m_BWSl == OZG_DuelConst.S_SWAP)
            return n == 1 && point != m_BWSlFirst;
        if (m_BWSl == OZG_DuelConst.S_BACK)
            return m_BNSlWho != "";
        return false;
    }

    protected void BNSlClick(int point)
    {
        if (!BNSlCandidate(point))
            return;
        string p = point.ToString();
        if (m_BWSl == OZG_DuelConst.S_OFF)
        {
            BWSlSend(OZG_DuelConst.S_OFF + ":" + p + ":-");
            return;
        }
        if (m_BWSl == OZG_DuelConst.S_BACK)
        {
            BWSlSend(OZG_DuelConst.S_BACK + ":" + m_BNSlWho + ":" + p);
            m_BNSlWho = "";
            return;
        }
        if (m_BWSlFirst < 0)
        {
            m_BWSlFirst = point;
            if (m_BWSl == OZG_DuelConst.S_MOVE)
                BWSay(Tr("STR_OZG_BWIN_SL_NHINT_MOVE2"), 1);
            else
                BWSay(Tr("STR_OZG_BWIN_SL_NHINT_SWAP2"), 1);
            BoardWinPaint();
            return;
        }
        BWSlSend(m_BWSl + ":" + m_BWSlFirst.ToString() + ":" + p);
    }

    // ------------------------------------------------------------ clicks at nardy

    // The board's clicks and nardy's own buttons; false for what the board window takes.
    protected bool BNClick(string n)
    {
        if (n == "OZG_BHit")
        {
            float x;
            float y;
            PointerAt(x, y);
            int point = BNPointAt(x, y);
            if (m_BWSl != "")
            {
                BNSlClick(point);
                return true;
            }
            BNPointClick(point);
            return true;
        }
        if (n == "OZG_BBtnThrow")
        {
            if (BWButton(OZG_NardyConst.M_THROW))
            {
                m_BNSwapStep = 0;
                Play(OZG_NardyConst.M_THROW);
            }
            return true;
        }
        if (n == "OZG_BBtnSwap")
        {
            if (m_BNSwapStep > 0)
                m_BNSwapStep = 0;
            else
                m_BNSwapStep = 1;
            BoardWinPaint();
            return true;
        }
        if (n.IndexOf("OZG_BSwapPick") == 0)
        {
            BNSwapPick(n.Substring(13, n.Length() - 13).ToInt());
            return true;
        }
        if (n == "OZG_BOffW")
        {
            if (m_BWSl == OZG_DuelConst.S_BACK)
            {
                m_BNSlWho = "w";
                if (m_BFlip)
                    m_BNSlWho = "b";
                BWSay(Tr("STR_OZG_BWIN_SL_NHINT_BACK2"), 1);
                BoardWinPaint();
                return true;
            }
            BNPlayTo(0);
            return true;
        }
        if (n == "OZG_BBtnDice")
        {
            if (m_V.IsHost == 1 && !BWPlaying())
                NextVariant();
            return true;
        }
        return false;
    }

    // A point clicked: the move there of the disc picked, else the disc there picked.
    protected void BNPointClick(int point)
    {
        if (point < 0)
        {
            m_BNSel = -1;
            BoardWinPaint();
            return;
        }
        if (m_BNSel >= 0 && BNPlayTo(point))
            return;
        array<string> steps = new array<string>();
        BNSteps(steps);
        m_BNSel = -1;
        for (int i = 0; i < steps.Count(); i++)
        {
            int from;
            int to;
            BNEnds(steps[i], from, to);
            if (from == point)
                m_BNSel = point;
        }
        BoardWinPaint();
    }

    // The step of the disc picked to `to` (0: off), the first of the dice that make it.
    protected bool BNPlayTo(int to)
    {
        array<string> steps = new array<string>();
        BNSteps(steps);
        for (int i = 0; i < steps.Count(); i++)
        {
            int from;
            int there;
            BNEnds(steps[i], from, there);
            if (from == m_BNSel && there == to)
            {
                m_BNSel = -1;
                Play(steps[i]);
                return true;
            }
        }
        return false;
    }
}
