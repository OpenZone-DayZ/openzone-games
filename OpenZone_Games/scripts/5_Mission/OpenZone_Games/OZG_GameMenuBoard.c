// The board games' window (boards and dice design §7): the design canvas's board
// "Шахи · класична дошка" (2026-11-26), built of ozg_bwin.layout by
// tools/make_board_win.py, which says how it was measured. The board takes the
// window's height; left the game, the other player and the moves; right the log,
// sleight of hand, the draw and the resignation, and the player himself.
//
// Chess, draughts, giveaway and corners stand on the field the way the seat sees
// it (black sees it turned). A piece is a picture of the item the board holds, in
// the look of its own set (§3.2): the classic set as the canvas draws it, on a soft
// shadow; the stalker figurines inscribed in their classic pieces (§15, the
// owner's word 36), seen "at the table" -- your own from behind, the other side's
// from the front; the stalker draughts with their faces, a king the disc turned
// over. A move slides, a piece taken fades.
//
// The window decides nothing: it shows what the server last said and asks for
// every move and every sleight.
modded class OZG_GameMenu
{
    protected static const string LAYOUT_BWPIC = "OpenZone_Games/gui/layouts/ozg_bwpic.layout";
    protected static const string LAYOUT_BWTINT = "OpenZone_Games/gui/layouts/ozg_bwtint.layout";
    // The field as White reads it and as Black does (models/src/make_textures.py).
    protected static const string BOARD_WHITE = "OpenZone_Games/gui/boards/ozg_board_win_white_co.paa";
    protected static const string BOARD_BLACK = "OpenZone_Games/gui/boards/ozg_board_win_black_co.paa";
    protected static const string BW_TEX = "OpenZone_Games/gui/textures/";
    // The field in the window: its top-left corner and a square, in units.
    protected static const float BW_S = 96.25;
    protected static const float BW_OX = 415.0;
    protected static const float BW_OY = 65.25;
    protected static const int BW_MOVES = 18;
    protected static const int BW_LOG = 22;
    // The pools of pictures the script lays: on the field, beside each plate.
    protected static const int BW_FIELD = 0;
    protected static const int BW_OPP = 1;
    protected static const int BW_ME = 2;
    // How long a move slides and a piece taken fades (the canvas: 0.35 s, 0.25 s).
    protected static const float BW_SLIDE = 0.35;
    protected static const float BW_FADE = 0.25;

    protected ref OZG_BPublic m_BPub;
    protected ref OZG_BPrivate m_BOwn;

    // What the script laid, kept to be laid again: per pool its pictures and the
    // picture each shows, its plain panels.
    protected ref array<ref array<Widget>> m_BWPics;
    protected ref array<ref array<string>> m_BWPicSrc;
    protected ref array<int> m_BWPicsUsed;
    protected ref array<ref array<Widget>> m_BWTints;
    protected ref array<int> m_BWTintsUsed;
    protected ref array<Widget> m_BWPoolParent;

    // The pieces as drawn: each widget, the square it stands on, where it rests (units
    // in the field), the alpha it ends at.
    protected ref array<Widget> m_BWPieceW;
    protected ref array<int> m_BWPieceSq;
    protected ref array<float> m_BWPieceX;
    protected ref array<float> m_BWPieceY;
    protected ref array<float> m_BWPieceA;
    // A move sliding: the square it ends on, where it came from (units, from its rest),
    // time gone; a piece fading: its square and whether it goes out or comes in.
    protected ref array<int> m_BWSlideSq;
    protected ref array<float> m_BWSlideDX;
    protected ref array<float> m_BWSlideDY;
    protected ref array<float> m_BWSlideT;
    protected ref array<Widget> m_BWFadeW;
    protected ref array<float> m_BWFadeT;
    protected ref array<int> m_BWFadeIn;
    protected ref array<float> m_BWFadeA;
    // The field as drawn last ("" before the first), and the game it was of.
    protected string m_BWShown;
    protected int m_BWShownRev = -1;

    // The move being picked, a square a click; the promotion asked.
    protected ref array<int> m_PickPath;
    protected int m_PromoFrom = -1;
    protected int m_PromoTo = -1;
    protected bool m_BFlip;
    // Sleight of hand (§9.1): the kind armed ("" none), its first square, the piece
    // taken picked for "put back" (its letter, and which of the two rows).
    protected string m_BWSl = "";
    protected int m_BWSlFirst = -1;
    protected string m_BWSlTaken = "";
    protected int m_BWSlTakenAt = -1;
    protected bool m_BWSlSent;
    protected string m_BWSlSentWho = "";
    // The panel's line (0 its rule, 1 told, 2 refused) and what is open over the board.
    protected string m_BWMsg = "";
    protected int m_BWMsgTone;
    protected bool m_BWMenuOpen;
    protected bool m_BWFoldAsk;
    protected int m_BWOverShut = -1;
    // A sleight this player saw (§9.1): its squares lit and its words under the board.
    protected ref array<int> m_SawSquares;
    protected int m_SawUntil;
    protected float m_BWToastLeft;
    // The seconds left until the next sleight, as last told, and when.
    protected int m_BWWait;
    protected float m_BWWaitAge;
    protected float m_BWTickAge;
    // The host's row and the moves its buttons make.
    protected ref array<string> m_BWHostMoves;
    // The games of the menu, in its rows.
    protected ref array<string> m_BWMenuGames;

    // ------------------------------------------------------------ which window

    // Which of the board's two windows: nardy plays on the board's other side.
    override protected string BoardStyle()
    {
        if (DiceWindow())
            return "dice";
        if (!BoardWindow())
            return "";
        if (m_V.GameId == OZG_NardyConst.LONG_ID || m_V.GameId == OZG_NardyConst.SHORT_ID)
            return "board_nardy";
        return "board";
    }

    // The board's window is up (either of the two).
    protected bool BWUp()
    {
        return m_Style == "board" || m_Style == "board_nardy" || m_Style == "dice";
    }

    // A board game, or a board laid out whose game is not chosen yet.
    override protected bool BoardWindow()
    {
        if (!m_V)
            return false;
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (sample)
            return sample.SetKind() == OZG_Const.SET_BOARD;
        array<OZG_Place> places = OZG_Place.All();
        for (int i = 0; i < places.Count(); i++)
        {
            OZG_Place place = places[i];
            if (!place || place.OZG_Id() != m_PlaceId)
                continue;
            OZG_Deck_Base deck = place.OZG_Deck();
            return deck && deck.IsInherited(OZG_Board_Base);
        }
        return false;
    }

    protected void BWInit()
    {
        if (m_BWPics)
            return;
        m_BWPics = new array<ref array<Widget>>();
        m_BWPicSrc = new array<ref array<string>>();
        m_BWPicsUsed = new array<int>();
        m_BWTints = new array<ref array<Widget>>();
        m_BWTintsUsed = new array<int>();
        m_BWPoolParent = new array<Widget>();
        for (int i = 0; i < 3; i++)
        {
            m_BWPics.Insert(new array<Widget>());
            m_BWPicSrc.Insert(new array<string>());
            m_BWPicsUsed.Insert(0);
            m_BWTints.Insert(new array<Widget>());
            m_BWTintsUsed.Insert(0);
            m_BWPoolParent.Insert(null);
        }
        m_BWPieceW = new array<Widget>();
        m_BWPieceSq = new array<int>();
        m_BWPieceX = new array<float>();
        m_BWPieceY = new array<float>();
        m_BWPieceA = new array<float>();
        m_BWSlideSq = new array<int>();
        m_BWSlideDX = new array<float>();
        m_BWSlideDY = new array<float>();
        m_BWSlideT = new array<float>();
        m_BWFadeW = new array<Widget>();
        m_BWFadeT = new array<float>();
        m_BWFadeIn = new array<int>();
        m_BWFadeA = new array<float>();
        m_PickPath = new array<int>();
        m_SawSquares = new array<int>();
        m_BWHostMoves = new array<string>();
        m_BWMenuGames = new array<string>();
    }

    // The window was built anew: what was laid went with the old one.
    override protected void BoardWinFind()
    {
        BWInit();
        for (int i = 0; i < 3; i++)
        {
            m_BWPics[i].Clear();
            m_BWPicSrc[i].Clear();
            m_BWPicsUsed[i] = 0;
            m_BWTints[i].Clear();
            m_BWTintsUsed[i] = 0;
        }
        m_BWPoolParent[BW_FIELD] = BW("OZG_BField");
        if (!m_BWPoolParent[BW_FIELD])
            m_BWPoolParent[BW_FIELD] = BW("OZG_DField");
        m_BWPoolParent[BW_OPP] = BW("OZG_BOppTaken");
        m_BWPoolParent[BW_ME] = BW("OZG_BMeTaken");
        m_BWPieceW.Clear();
        m_BWPieceSq.Clear();
        m_BWPieceX.Clear();
        m_BWPieceY.Clear();
        m_BWPieceA.Clear();
        BWSlidesClear();
        if (m_BNBadges)
            m_BNBadges.Clear();
        m_BWShown = "";
        m_BWShownRev = -1;
        m_BWMenuOpen = false;
        m_BWFoldAsk = false;
        PickClear();
        BWSlClear();
        // The captions the canvas writes in capitals.
        BWSetText("OZG_BMovesTitle", OZG_Text.Upper(Tr("STR_OZG_BWIN_MOVES")));
        BWSetText("OZG_BLogTitle", OZG_Text.Upper(Tr("STR_OZG_BWIN_LOG")));
        BWSetText("OZG_BMenuHead", OZG_Text.Upper(Tr("STR_OZG_BWIN_MENU_HEAD")));
    }

    // ------------------------------------------------------------ small helpers

    protected Widget BW(string name)
    {
        if (!m_Win)
            return null;
        return m_Win.FindAnyWidget(name);
    }

    protected TextWidget BWText(string name)
    {
        return TextWidget.Cast(BW(name));
    }

    protected void BWShow(string name, bool show)
    {
        Widget w = BW(name);
        if (w)
            w.Show(show);
    }

    protected void BWSetText(string name, string text)
    {
        TextWidget t = BWText(name);
        if (t)
            t.SetText(text);
    }

    protected void BWInk(string name, int colour)
    {
        Widget w = BW(name);
        if (w)
            w.SetColor(colour);
    }

    // A shape of the layout recoloured: every part of it, or the panel it is.
    protected void BWPaint(Widget shape, int colour)
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

    protected void BWPaintNamed(string name, int colour)
    {
        BWPaint(BW(name), colour);
    }

    // A shape's seven parts laid out again for another size, in units.
    protected void BWFit(Widget shape, float w, float h, float r)
    {
        if (!shape)
            return;
        string n = shape.GetName();
        shape.SetSize(w * m_K, h * m_K);
        BWPart(shape, n + "TL", 0, 0, r, r);
        BWPart(shape, n + "TR", w - r, 0, r, r);
        BWPart(shape, n + "BL", 0, h - r, r, r);
        BWPart(shape, n + "BR", w - r, h - r, r, r);
        BWPart(shape, n + "M", 0, r, w, h - 2 * r);
        BWPart(shape, n + "T", r, 0, w - 2 * r, r);
        BWPart(shape, n + "B", r, h - r, w - 2 * r, r);
    }

    protected void BWPart(Widget shape, string name, float x, float y, float w, float h)
    {
        Widget p = shape.FindAnyWidget(name);
        if (!p)
            return;
        p.SetPos(x * m_K, y * m_K);
        p.SetSize(w * m_K, h * m_K);
    }

    // A widget moved, in units of its parent.
    protected void BWMove(Widget w, float x, float y)
    {
        if (w)
            w.SetPos(x * m_K, y * m_K);
    }

    // A text's width in units as it would be drawn now.
    protected float BWTextW(TextWidget t)
    {
        if (!t || m_K <= 0)
            return 0;
        int w;
        int h;
        // asked straight after SetText: laid out first (as the core's OZ_VppFlex)
        t.Update();
        t.GetTextSize(w, h);
        return w / m_K;
    }

    // Words broken into lines no wider than `w` units, measured on `probe` (which is
    // left holding the last line tried); at most `most` lines, the last cut short.
    protected void BWWrap(TextWidget probe, string text, float w, int most, array<string> into)
    {
        into.Clear();
        if (!probe || text == "")
            return;
        array<string> words = new array<string>();
        text.Split(" ", words);
        string line = "";
        for (int i = 0; i < words.Count(); i++)
        {
            string tried = words[i];
            if (line != "")
                tried = line + " " + words[i];
            probe.SetText(tried);
            if (line != "" && BWTextW(probe) > w)
            {
                into.Insert(line);
                line = words[i];
                if (into.Count() >= most)
                {
                    into[most - 1] = into[most - 1] + "…";
                    return;
                }
            }
            else
            {
                line = tried;
            }
        }
        if (line != "")
            into.Insert(line);
    }

    // A text the server sent, in this client's language.
    protected string BWWords(string text)
    {
        if (text.IndexOf("#") == 0)
            return Widget.TranslateString(text);
        return text;
    }

    // ------------------------------------------------------------ the pools

    protected void BWPoolsBegin()
    {
        for (int i = 0; i < 3; i++)
        {
            m_BWPicsUsed[i] = 0;
            m_BWTintsUsed[i] = 0;
        }
        m_BWPieceW.Clear();
        m_BWPieceSq.Clear();
        m_BWPieceX.Clear();
        m_BWPieceY.Clear();
        m_BWPieceA.Clear();
    }

    protected void BWPoolsEnd()
    {
        for (int i = 0; i < 3; i++)
        {
            int k;
            for (k = m_BWPicsUsed[i]; k < m_BWPics[i].Count(); k++)
            {
                if (m_BWPics[i][k])
                    m_BWPics[i][k].Show(false);
            }
            for (k = m_BWTintsUsed[i]; k < m_BWTints[i].Count(); k++)
            {
                if (m_BWTints[i][k])
                    m_BWTints[i][k].Show(false);
            }
        }
    }

    // A picture of a pool, placed in units of its parent; `colour` tints it (white:
    // as it is). "set:.. image:.." or a texture's path.
    protected ImageWidget BWPic(int pool, string picture, float x, float y, float w, float h, int prio, int colour)
    {
        Widget parent = m_BWPoolParent[pool];
        if (!parent)
            return null;
        int at = m_BWPicsUsed[pool];
        ImageWidget pic;
        if (at < m_BWPics[pool].Count())
        {
            pic = ImageWidget.Cast(m_BWPics[pool][at]);
        }
        else
        {
            pic = ImageWidget.Cast(MakeLayout(LAYOUT_BWPIC, parent, "OZG_BWPic"));
            if (!pic)
                return null;
            m_BWPics[pool].Insert(pic);
            m_BWPicSrc[pool].Insert("");
        }
        m_BWPicsUsed[pool] = at + 1;
        if (m_BWPicSrc[pool][at] != picture)
        {
            pic.LoadImageFile(0, picture);
            pic.SetImage(0);
            m_BWPicSrc[pool][at] = picture;
        }
        pic.SetColor(colour);
        pic.SetSort(prio);
        PutAt(pic, x, y, w, h);
        return pic;
    }

    // A plain panel of a pool, in units of its parent.
    protected Widget BWTint(int pool, float x, float y, float w, float h, int prio, int colour)
    {
        Widget parent = m_BWPoolParent[pool];
        if (!parent)
            return null;
        int at = m_BWTintsUsed[pool];
        Widget tint;
        if (at < m_BWTints[pool].Count())
        {
            tint = m_BWTints[pool][at];
        }
        else
        {
            tint = MakeLayout(LAYOUT_BWTINT, parent, "OZG_BWTint");
            if (!tint)
                return null;
            m_BWTints[pool].Insert(tint);
        }
        m_BWTintsUsed[pool] = at + 1;
        tint.SetColor(colour);
        tint.SetSort(prio);
        PutAt(tint, x, y, w, h);
        return tint;
    }

    // A ring inside a box: four strips `t` units wide (a CSS inset shadow).
    protected void BWRing(int pool, float x, float y, float w, float h, float t, int prio, int colour)
    {
        BWTint(pool, x, y, w, t, prio, colour);
        BWTint(pool, x, y + h - t, w, t, prio, colour);
        BWTint(pool, x, y + t, t, h - 2 * t, prio, colour);
        BWTint(pool, x + w - t, y + t, t, h - 2 * t, prio, colour);
    }

    // ------------------------------------------------------------ the game read

    protected void BoardRead()
    {
        m_BPub = null;
        m_BOwn = null;
        if (!BoardWindow() || m_V.St == OZG_Const.ST_LOBBY)
            return;
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
        string err;
        if (view.Shared != "")
            JsonFileLoader<OZG_BPublic>.LoadData(view.Shared, m_BPub, err);
        if (view.Own != "")
            JsonFileLoader<OZG_BPrivate>.LoadData(view.Own, m_BOwn, err);
        // nardy's position is its points (OZG_GameMenuNardy.c), the rest the 64 squares
        if (m_BPub && m_BPub.Board.Length() != 64 && m_BPub.Points == "")
            m_BPub = null;
    }

    protected bool BWOver()
    {
        return m_BPub && m_BPub.Final != "";
    }

    protected bool BWPlaying()
    {
        return m_BPub && m_V.St == OZG_Const.ST_PLAYING && !BWOver();
    }

    // Draughts, giveaway and corners lay discs; chess its pieces.
    protected bool BoardDiscs()
    {
        return m_V && m_V.GameId != OZG_ChessConst.GAME_ID;
    }

    // The seat at the bottom: the player's own; a watcher looks from White's side.
    protected int BWBottomSeat()
    {
        if (m_BPub && m_BPub.White >= 0)
        {
            if (m_V.Mine == m_BPub.Black)
                return m_BPub.Black;
            return m_BPub.White;
        }
        if (m_V.Mine >= 0)
            return m_V.Mine;
        return 0;
    }

    protected int BWTopSeat()
    {
        int bottom = BWBottomSeat();
        if (m_BPub && m_BPub.White >= 0)
        {
            if (bottom == m_BPub.White)
                return m_BPub.Black;
            return m_BPub.White;
        }
        for (int i = 0; i < m_V.Seats.Count(); i++)
        {
            if (m_V.Seats[i].No != bottom)
                return m_V.Seats[i].No;
        }
        return -1;
    }

    protected bool BWTaken(int seat)
    {
        if (seat < 0 || seat >= m_V.Seats.Count())
            return false;
        int st = m_V.Seats[seat].St;
        return st == OZG_Const.SEAT_TAKEN || st == OZG_Const.SEAT_KEPT;
    }

    // Whether the player of a seat has the window open and is near.
    protected bool BWLooks(int seat)
    {
        return m_V.Looking.Find(seat) >= 0;
    }

    // ------------------------------------------------------------ the pieces' pictures

    // The face of a letter of the field: "wK" .. "bP" for chess, "wD" / "bD" for discs.
    protected string BWFace(string letter)
    {
        if (BoardDiscs())
        {
            if (letter == "w" || letter == "W")
                return "wD";
            return "bD";
        }
        string up = letter;
        up.ToUpper();
        if (up == letter)
            return "w" + up;
        return "b" + up;
    }

    // A disc crowned: "W" and "B" on the field.
    protected bool BWKing(string letter)
    {
        return BoardDiscs() && (letter == "W" || letter == "B");
    }

    protected bool BWWhite(string letter)
    {
        if (BoardDiscs())
            return letter == "w" || letter == "W";
        string up = letter;
        up.ToUpper();
        return up == letter;
    }

    // The look of a face on this board: the view's list, else the set's own.
    protected string BWLookOf(string face)
    {
        if (m_V && m_V.PieceLooks != "")
        {
            string key = face + "=";
            int at = m_V.PieceLooks.IndexOf(key);
            if (at >= 0)
            {
                int from = at + key.Length();
                int end = m_V.PieceLooks.IndexOfFrom(from, ";");
                if (end < 0)
                    end = m_V.PieceLooks.Length();
                return m_V.PieceLooks.Substring(from, end - from);
            }
        }
        if (m_V && m_V.SetLook != "")
            return m_V.SetLook;
        return "classic";
    }

    // "At the table": your own figure shows its back, the other side's its face
    // (black sees it the other way round); a watcher sits on White's side.
    protected string BWView(string face)
    {
        bool white = face.IndexOf("w") == 0;
        bool mine = white != m_BFlip;
        if (mine)
            return "rear";
        return "front";
    }

    // A stalker disc's picture: a man shows his face, a king the disc turned over.
    protected string BWDiscPicture(string letter, bool shadow)
    {
        string role = "black_piece";
        if (letter == "w")
            role = "white_piece";
        else if (letter == "W")
            role = "white_king_cs";
        else if (letter == "B")
            role = "black_king";
        if (shadow)
            role = role + "_s";
        return "set:ozg_discs_stalker image:" + role;
    }

    // Where a stalker figure stands inscribed in its classic piece, in units from
    // its square's top-left corner (the research tools' shadow_fit.py: as wide as the
    // classic piece's foot allows, no higher than 80% of it, on its foot).
    protected void BWInscribed(string face, string view, out float x, out float y, out float w, out float h)
    {
        string table = OZG_StalkerFit.Table();
        string key = face + ":" + view + ":";
        int at = table.IndexOf(key);
        x = 0;
        y = 0;
        w = BW_S;
        h = BW_S;
        if (at < 0)
            return;
        int from = at + key.Length();
        int end = table.IndexOfFrom(from, ";");
        if (end < 0)
            end = table.Length();
        array<string> n = new array<string>();
        string numbers = table.Substring(from, end - from);
        numbers.Split(",", n);
        if (n.Count() != 4)
            return;
        x = n[0].ToFloat();
        y = n[1].ToFloat();
        w = n[2].ToFloat();
        h = n[3].ToFloat();
    }

    // A piece drawn on a square (units in the field: the square's top-left corner),
    // with whatever lies under it; every widget kept for the slides and fades.
    protected void BWPieceAt(string letter, int sq, float sx, float sy, int prio)
    {
        string face = BWFace(letter);
        string look = BWLookOf(face);
        if (BoardDiscs())
        {
            // A disc: 92 wide, standing on the square's bottom less 0 (the canvas:
            // a box of 92 x 108, the disc at its foot, a king's second disc 14 higher).
            float dx = sx + (BW_S - 92) / 2;
            float dy = sy + BW_S - 92;
            if (look == "stalker")
            {
                BWKeep(BWPic(BW_FIELD, BWDiscPicture(letter, true), dx, dy + 4, 92, 92, prio, ARGB(255, 255, 255, 255)), sq, dx, dy + 4, 1);
                BWKeep(BWPic(BW_FIELD, BWDiscPicture(letter, false), dx, dy, 92, 92, prio + 1, ARGB(255, 255, 255, 255)), sq, dx, dy, 1);
                return;
            }
            string disc = "set:ozg_pieces_classic image:" + face;
            BWKeep(BWPic(BW_FIELD, disc + "_s", dx, dy + 4, 92, 92, prio, ARGB(255, 255, 255, 255)), sq, dx, dy + 4, 1);
            BWKeep(BWPic(BW_FIELD, disc, dx, dy, 92, 92, prio + 1, ARGB(255, 255, 255, 255)), sq, dx, dy, 1);
            if (BWKing(letter))
            {
                BWKeep(BWPic(BW_FIELD, disc + "_s", dx, dy - 10, 92, 92, prio + 1, ARGB(255, 255, 255, 255)), sq, dx, dy - 10, 1);
                BWKeep(BWPic(BW_FIELD, disc, dx, dy - 14, 92, 92, prio + 1, ARGB(255, 255, 255, 255)), sq, dx, dy - 14, 1);
            }
            return;
        }
        // A chess piece of the classic set: 106 x 106, its foot 6 above the square's
        // bottom; the same picture dark at 36% is the classic shape a stalker figure is
        // inscribed in.
        float cx = sx + (BW_S - 106) / 2;
        float cy = sy + BW_S - 112;
        string classic = "set:ozg_pieces_classic image:" + face;
        if (look == "stalker")
        {
            BWKeep(BWPic(BW_FIELD, classic, cx, cy, 106, 106, prio, ARGB(92, 0, 0, 0)), sq, cx, cy, 0.36);
            string view = BWView(face);
            float fx;
            float fy;
            float fw;
            float fh;
            BWInscribed(face, view, fx, fy, fw, fh);
            string figure = "set:ozg_pieces_stalker image:" + face + "_" + view;
            BWKeep(BWPic(BW_FIELD, figure + "_s", sx + fx, sy + fy + 4, fw, fh, prio + 1, ARGB(255, 255, 255, 255)), sq, sx + fx, sy + fy + 4, 1);
            BWKeep(BWPic(BW_FIELD, figure, sx + fx, sy + fy, fw, fh, prio + 1, ARGB(255, 255, 255, 255)), sq, sx + fx, sy + fy, 1);
            return;
        }
        BWKeep(BWPic(BW_FIELD, classic + "_s", cx, cy + 5, 106, 106, prio, ARGB(255, 255, 255, 255)), sq, cx, cy + 5, 1);
        BWKeep(BWPic(BW_FIELD, classic, cx, cy, 106, 106, prio + 1, ARGB(255, 255, 255, 255)), sq, cx, cy, 1);
    }

    protected void BWKeep(Widget w, int sq, float x, float y, float alpha)
    {
        if (!w)
            return;
        m_BWPieceW.Insert(w);
        m_BWPieceSq.Insert(sq);
        m_BWPieceX.Insert(x);
        m_BWPieceY.Insert(y);
        m_BWPieceA.Insert(alpha);
    }

    // A small picture of a piece beside a plate (taken, the avatar, the promotion's).
    protected string BWSmallPicture(string letter)
    {
        string face = BWFace(letter);
        string look = BWLookOf(face);
        if (look == "stalker" && BoardDiscs())
            return BWDiscPicture(letter, false);
        if (look == "stalker")
            return "set:ozg_pieces_stalker image:" + face + "_front";
        return "set:ozg_pieces_classic image:" + face;
    }

    // ------------------------------------------------------------ the field's squares

    // The square drawn in a row (from the top) and a column (from the left).
    protected int SquareOfCell(int row, int col)
    {
        if (m_BFlip)
            return row * 8 + (7 - col);
        return (7 - row) * 8 + col;
    }

    // A square's top-left corner in units of the field.
    protected void SquareCorner(int sq, out float x, out float y)
    {
        int file = sq % 8;
        int rank = sq / 8;
        if (m_BFlip)
        {
            x = (7 - file) * BW_S;
            y = rank * BW_S;
            return;
        }
        x = file * BW_S;
        y = (7 - rank) * BW_S;
    }

    // The square under a point of the window (units), -1 for none.
    protected int SquareAt(float px, float py)
    {
        float fx = (px - BW_OX) / BW_S;
        float fy = (py - BW_OY) / BW_S;
        if (fx < 0 || fy < 0 || fx >= 8 || fy >= 8)
            return -1;
        int col = fx;
        int row = fy;
        return SquareOfCell(row, col);
    }

    // ------------------------------------------------------------ painting

    override protected void BoardWinPaint()
    {
        BWInit();
        BoardRead();
        // Through a local: a compound boolean written straight into a field corrupts the
        // native heap (dayz-modding SKILL.md).
        bool flip = false;
        if (m_BPub && m_V.Mine >= 0 && m_V.Mine == m_BPub.Black)
            flip = true;
        m_BFlip = flip;
        if (!BWPlaying())
        {
            PickClear();
            BWSlClear();
        }
        // A sleight this player asked for and the server did: told in the panel.
        if (m_BWSlSent)
            BWSlAnswered();
        if (m_Style == "board_nardy")
        {
            BWNardyPaint();
            return;
        }
        if (m_Style == "dice")
        {
            DWPaint();
            return;
        }
        BWPoolsBegin();
        BWPaintTitle();
        BWPaintPlate("OZG_BOpp", BWTopSeat(), false);
        BWPaintPlate("OZG_BMe", BWBottomSeat(), true);
        BWPaintMoves();
        BWPaintLog();
        BWPaintActions();
        BWPaintField();
        BWPoolsEnd();
        BWPaintDialogs();
    }

    // The game, the set and the two buttons under them.
    protected void BWPaintTitle()
    {
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        string game = "";
        if (sample)
            game = Tr(sample.LabelKey());
        BWSetText("OZG_BTitle", game);
        string sub = Tr("STR_OZG_BWIN_SET_CLASSIC");
        if (m_V.SetLook == "stalker")
            sub = Tr("STR_OZG_BWIN_SET_STALKER");
        string variant = VariantLabel();
        if (variant != "")
            sub = sub + " · " + variant;
        BWSetText("OZG_BSub", sub);

        // "Game: Chess" with the chevron after its words; the menu is the host's,
        // and only between games.
        TextWidget label = BWText("OZG_BBtnGameText");
        if (label)
        {
            label.SetText(Tr1("STR_OZG_BWIN_GAME", game));
            float tw = BWTextW(label);
            Widget chev = BW("OZG_BBtnGameChev");
            bool menu = BWMenuAllowed();
            if (chev)
            {
                chev.Show(menu);
                // the label's middle is the button's less half the chevron and its gap
                BWMove(chev, 30 + 65.5 + tw / 2 + 6, 102);
            }
        }
        // The second: the host folds the game, a seated player stands up.
        Widget fold = BW("OZG_BBtnFold");
        if (fold)
        {
            bool shown = m_V.IsHost == 1 || m_V.Mine >= 0;
            fold.Show(shown);
            if (m_V.IsHost == 1)
                BWSetText("OZG_BBtnFoldText", Tr("STR_OZG_BWIN_FOLD"));
            else
                BWSetText("OZG_BBtnFoldText", Tr("STR_OZG_BWIN_STAND"));
        }
    }

    protected bool BWMenuAllowed()
    {
        bool playing = BWPlaying();
        if (m_Style == "dice")
            playing = DWPlaying();
        return m_V.IsHost == 1 && !playing && m_V.GameIds.Count() > 1;
    }

    // A player's plate: the pieces his side took, his king, his name and side, the chip.
    protected void BWPaintPlate(string p, int seat, bool bottom)
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
            BWMove(BW(p + "You"), 68 + BWTextW(nameW) + 4, 54.5);

        // White or black, and the host's.
        bool white = true;
        if (m_BPub && m_BPub.White >= 0)
            white = seat == m_BPub.White;
        else
            white = bottom;
        string side = "";
        if (m_BPub && m_BPub.White >= 0)
        {
            if (white)
                side = Tr("STR_OZG_BWIN_SIDE_WHITE");
            else
                side = Tr("STR_OZG_BWIN_SIDE_BLACK");
        }
        if (taken && seat == m_V.HostSeat)
        {
            if (side != "")
                side = side + " · ";
            side = side + Tr("STR_OZG_BWIN_HOST");
        }
        BWSetText(p + "Side", side);

        // The avatar: the side's king (a disc at draughts) in the look of the set.
        string king = "K";
        if (!white)
            king = "k";
        if (BoardDiscs())
        {
            king = "w";
            if (!white)
                king = "b";
        }
        ImageWidget av = ImageWidget.Cast(BW(p + "AvPic"));
        if (av)
        {
            string pic = BWSmallPicture(king);
            av.LoadImageFile(0, pic);
            av.SetImage(0);
            // a figure is drawn to cover the circle, its head in it (the canvas:
            // object-fit cover, 14% from the top)
            if (BWLookOf(BWFace(king)) == "stalker" && !BoardDiscs())
            {
                av.SetPos(0, -4.8 * m_K);
                av.SetSize(46 * m_K, 80.5 * m_K);
            }
            else
            {
                av.SetPos(0, 2 * m_K);
                av.SetSize(46 * m_K, 46 * m_K);
            }
            av.Show(taken);
        }

        // The chip and the ring: what the player is doing.
        bool turn = BWPlaying() && m_BPub.Turn == seat;
        string chip;
        int chipBg;
        int chipFg;
        int ring = ARGB(255, 46, 34, 23);
        if (!taken)
        {
            chip = Tr("STR_OZG_BWIN_WAITING");
            chipBg = ARGB(255, 45, 36, 27);
            chipFg = ARGB(255, 184, 165, 133);
        }
        else if (mine)
        {
            if (BWOver())
                chip = Tr("STR_OZG_BWIN_OVER");
            else if (!BWPlaying())
                chip = Tr("STR_OZG_BWIN_WAITING");
            else if (turn)
                chip = Tr("STR_OZG_BWIN_YOUR_MOVE");
            else
                chip = Tr("STR_OZG_BWIN_THEIR_MOVE");
            chipBg = ARGB(255, 45, 36, 27);
            chipFg = ARGB(255, 184, 165, 133);
            if (turn)
            {
                chipBg = ARGB(255, 61, 48, 29);
                chipFg = ARGB(255, 231, 194, 122);
                ring = ARGB(255, 201, 164, 92);
            }
        }
        else
        {
            bool away = !BWLooks(seat);
            if (away)
            {
                chip = Tr("STR_OZG_BWIN_AWAY");
                chipBg = ARGB(255, 57, 42, 21);
                chipFg = ARGB(255, 240, 196, 106);
            }
            else if (turn)
            {
                chip = Tr("STR_OZG_BWIN_THINKING");
                chipBg = ARGB(255, 40, 43, 28);
                chipFg = ARGB(255, 166, 217, 160);
                ring = ARGB(255, 138, 106, 62);
            }
            else
            {
                chip = Tr("STR_OZG_BWIN_WATCHING");
                chipBg = ARGB(255, 40, 43, 28);
                chipFg = ARGB(255, 166, 217, 160);
            }
        }
        BWPaintNamed(p + "PlateEdge", ring);
        TextWidget chipText = BWText(p + "ChipText");
        Widget chipBox = BW(p + "Chip");
        if (chipText && chipBox)
        {
            chipText.SetText(chip);
            chipText.SetColor(chipFg);
            float tw = BWTextW(chipText);
            float lead = 10;
            if (!bottom)
                lead = 23;
            float cw = lead + tw + 10;
            chipText.SetSize((tw + 2) * m_K, 15 * m_K);
            float cy;
            if (bottom)
                cy = 61;
            else
                cy = 21;
            chipBox.SetPos((318 - cw) * m_K, cy * m_K);
            chipBox.SetSize(cw * m_K, 24 * m_K);
            Widget pill = BW(p + "ChipBg");
            BWFit(pill, cw, 24, 12);
            BWPaint(pill, chipBg);
            BWInk(p + "ChipDot", chipFg);
        }

        // The pieces this side took: the other colour's, in the order they fell.
        int pool = BW_OPP;
        if (bottom)
            pool = BW_ME;
        BWPaintTaken(pool, p, white);
    }

    // The taken pieces of the other colour, 26 units apart (closer when more than
    // the row holds); for "put back", each in a box, the one picked lit.
    protected void BWPaintTaken(int pool, string p, bool takerWhite)
    {
        array<string> pieces = new array<string>();
        if (m_BPub)
        {
            for (int i = 0; i < m_BPub.Taken.Length(); i++)
            {
                string letter = m_BPub.Taken.Get(i);
                if (BWWhite(letter) != takerWhite)
                    pieces.Insert(letter);
            }
        }
        BWShow(p + "None", pieces.Count() == 0);
        float step = 26;
        if (pieces.Count() > 11)
            step = 280.0 / (pieces.Count() - 1);
        bool backing = m_BWSl == OZG_DuelConst.S_BACK;
        for (int k = 0; k < pieces.Count(); k++)
        {
            float x = k * step;
            if (backing)
            {
                bool chosen = m_BWSlTaken == pieces[k] && m_BWSlTakenAt == k + 100 * pool;
                if (chosen)
                {
                    BWTint(pool, x, 0, 26, 34, 1, ARGB(89, 231, 194, 122));
                    BWRing(pool, x, 0, 26, 34, 2, 2, ARGB(255, 231, 194, 122));
                }
                else
                {
                    BWTint(pool, x, 0, 26, 34, 1, ARGB(26, 231, 194, 122));
                }
            }
            BWPic(pool, BWSmallPicture(pieces[k]), x - 4, 0, 34, 34, 3 + k, ARGB(255, 255, 255, 255));
        }
    }

    // The moves: a row a full move, the last half-move bold and gold.
    protected void BWPaintMoves()
    {
        array<string> half = new array<string>();
        if (m_BPub)
        {
            for (int h = 0; h < m_BPub.History.Count(); h++)
                half.Insert(BWNotation(m_BPub.History[h]));
        }
        int rows = (half.Count() + 1) / 2;
        int first = rows - BW_MOVES;
        if (first < 0)
            first = 0;
        for (int i = 0; i < BW_MOVES; i++)
        {
            int row = first + i;
            string si = i.ToString();
            bool shown = row < rows;
            BWShow("OZG_BMoveN" + si, shown);
            BWShow("OZG_BMoveW" + si, false);
            BWShow("OZG_BMoveWB" + si, false);
            BWShow("OZG_BMoveB" + si, false);
            BWShow("OZG_BMoveBB" + si, false);
            if (!shown)
                continue;
            int n = row + 1;
            BWSetText("OZG_BMoveN" + si, n.ToString() + ".");
            int w = row * 2;
            int b = w + 1;
            string wName = "OZG_BMoveW" + si;
            if (w == half.Count() - 1)
                wName = "OZG_BMoveWB" + si;
            BWSetText(wName, half[w]);
            BWShow(wName, true);
            if (b < half.Count())
            {
                string bName = "OZG_BMoveB" + si;
                if (b == half.Count() - 1)
                    bName = "OZG_BMoveBB" + si;
                BWSetText(bName, half[b]);
                BWShow(bName, true);
            }
        }
    }

    // A half-move as the canvas writes it: chess short, with the board's own letters
    // ("Кf3", "exd5", "e8Ф", "O-O", "+" and "#" kept); draughts as on the wire.
    protected string BWNotation(string text)
    {
        if (BoardDiscs() || text.Length() < 2)
            return text;
        string t = text;
        string sign = "";
        string tail = t.Substring(t.Length() - 1, 1);
        if (tail == "+" || tail == "#")
        {
            sign = tail;
            t = t.Substring(0, t.Length() - 1);
        }
        int ep = t.IndexOf(" ");
        if (ep > 0)
            t = t.Substring(0, ep);
        if (t.IndexOf("O-O") == 0)
            return t + sign;
        string piece = "";
        string first = t.Substring(0, 1);
        if (first == "K" || first == "Q" || first == "R" || first == "B" || first == "N")
        {
            piece = first;
            t = t.Substring(1, t.Length() - 1);
        }
        if (t.Length() < 5)
            return text;
        string from = t.Substring(0, 2);
        string how = t.Substring(2, 1);
        string to = t.Substring(3, 2);
        string promo = "";
        int eq = t.IndexOf("=");
        if (eq > 0 && eq + 1 < t.Length())
            promo = BWLetter(t.Substring(eq + 1, 1));
        string s;
        if (piece == "")
        {
            if (how == "x")
                s = from.Substring(0, 1) + "x" + to;
            else
                s = to;
        }
        else
        {
            s = BWLetter(piece);
            if (how == "x")
                s = s + "x";
            s = s + to;
        }
        return s + promo + sign;
    }

    // A piece's letter in this client's language (the canvas: Кр, Ф, Т, С, К).
    protected string BWLetter(string english)
    {
        array<string> ours = new array<string>();
        string letters = Tr("STR_OZG_BWIN_LETTERS");
        letters.Split(",", ours);
        string all = "KQRBN";
        int at = all.IndexOf(english);
        if (at < 0 || at >= ours.Count())
            return english;
        return ours[at];
    }

    // The log: the latest lines that fit, each broken to the panel's width.
    protected void BWPaintLog()
    {
        TextWidget probe = BWText("OZG_BLog0");
        if (probe)
            probe.Show(true);
        array<string> one = new array<string>();
        // From the newest back, while they fit: 18 a line, 5 between lines of the log.
        float room = 472 - 30 - 10;
        float used = 0;
        int count = m_V.Log.Count();
        array<ref array<string>> picked = new array<ref array<string>>();
        for (int k = count - 1; k >= 0; k--)
        {
            string worded = LogWord(m_V.Log[k]);
            BWWrap(probe, worded, 306, 3, one);
            float need = one.Count() * 18;
            if (picked.Count() > 0)
                need = need + 5;
            if (used + need > room)
                break;
            used = used + need;
            array<string> copy = new array<string>();
            copy.Copy(one);
            picked.Insert(copy);
        }
        float y = 30;
        int at = 0;
        for (int e = picked.Count() - 1; e >= 0; e--)
        {
            array<string> entry = picked[e];
            for (int l = 0; l < entry.Count() && at < BW_LOG; l++)
            {
                TextWidget line = BWText("OZG_BLog" + at.ToString());
                if (line)
                {
                    line.SetText(entry[l]);
                    BWMove(line, 12, y);
                    line.Show(true);
                }
                y = y + 18;
                at++;
            }
            y = y + 5;
        }
        while (at < BW_LOG)
        {
            BWShow("OZG_BLog" + at.ToString(), false);
            at++;
        }
    }

    // Sleight of hand, the draw and the resignation, the host's row.
    protected void BWPaintActions()
    {
        // --- the panel: shown while this player may cheat at this board
        bool sl = BWPlaying() && m_V.BoardSleights == 1;
        BWShow("OZG_BSl", sl);
        if (sl)
            BWPaintSleights();

        // --- the two big buttons: a draw offered to me, or my own two
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

        // --- the host's row where the canvas had its demo buttons
        BWPaintHostRow();
    }

    protected string m_BWDrawMove = "";
    protected string m_BWResignMove = "";

    protected bool BWButton(string move)
    {
        if (!m_BOwn || !m_BOwn.Buttons)
            return false;
        for (int i = 0; i < m_BOwn.Buttons.Count(); i++)
        {
            if (m_BOwn.Buttons[i] && m_BOwn.Buttons[i].Move == move)
                return true;
        }
        return false;
    }

    protected void BWPaintSleights()
    {
        int left = BWWaitLeft();
        TextWidget cool = BWText("OZG_BSlCool");
        if (cool)
        {
            if (left > 0)
            {
                cool.SetText(Tr1("STR_OZG_BWIN_COOL_WAIT", BWClock(left)));
                cool.SetColor(ARGB(255, 240, 196, 106));
            }
            else
            {
                cool.SetText(Tr("STR_OZG_BWIN_COOL_OK"));
                cool.SetColor(ARGB(255, 166, 217, 160));
            }
        }
        array<string> k = new array<string>();
        k.Insert(OZG_DuelConst.S_MOVE);
        k.Insert(OZG_DuelConst.S_SWAP);
        k.Insert(OZG_DuelConst.S_BACK);
        k.Insert(OZG_DuelConst.S_OFF);
        array<string> names = new array<string>();
        names.Insert("OZG_BSlMove");
        names.Insert("OZG_BSlSwap");
        names.Insert("OZG_BSlBack");
        names.Insert("OZG_BSlOff");
        for (int i = 0; i < 4; i++)
        {
            bool on = m_BWSl == k[i];
            int bg = ARGB(255, 51, 37, 26);
            int fg = ARGB(255, 239, 228, 204);
            int edge = ARGB(255, 107, 82, 51);
            if (on)
            {
                bg = ARGB(255, 201, 164, 92);
                fg = ARGB(255, 26, 18, 10);
                edge = ARGB(255, 231, 194, 122);
            }
            else if (left > 0)
            {
                fg = ARGB(255, 160, 143, 115);
            }
            BWPaintNamed(names[i] + "BoxBg", bg);
            BWPaintNamed(names[i] + "BoxEdge", edge);
            BWInk(names[i] + "Text", fg);
        }
        // Its line: what to do next, what was done, why not; else its rule.
        string msg = m_BWMsg;
        int tone = m_BWMsgTone;
        if (msg == "")
        {
            msg = Tr1("STR_OZG_BWIN_SL_RULE", OZG_Cfg.Get().BoardSleightMinutes.ToString());
            tone = 0;
        }
        int ink = ARGB(255, 184, 165, 133);
        if (tone == 1)
            ink = ARGB(255, 231, 194, 122);
        else if (tone == 2)
            ink = ARGB(255, 240, 154, 134);
        TextWidget m0 = BWText("OZG_BSlMsg0");
        TextWidget m1 = BWText("OZG_BSlMsg1");
        array<string> lines = new array<string>();
        BWWrap(m0, msg, 306, 2, lines);
        if (m0)
        {
            string l0 = "";
            if (lines.Count() > 0)
                l0 = lines[0];
            m0.SetText(l0);
            m0.SetColor(ink);
        }
        if (m1)
        {
            string l1 = "";
            if (lines.Count() > 1)
                l1 = lines[1];
            m1.SetText(l1);
            m1.SetColor(ink);
        }
    }

    // Seconds until the next sleight, counted down from what the view said.
    protected int BWWaitLeft()
    {
        int left = m_V.BoardWait - m_BWWaitAge;
        if (left < 0)
            left = 0;
        return left;
    }

    protected string BWClock(int seconds)
    {
        int m = seconds / 60;
        int s = seconds % 60;
        string ss = s.ToString();
        if (s < 10)
            ss = "0" + ss;
        return m.ToString() + ":" + ss;
    }

    // The table's own buttons where the canvas had its demo ones: begin, the variant,
    // the host's role taken, a player removed or his seat freed, the pose.
    protected void BWPaintHostRow()
    {
        m_BWHostMoves.Clear();
        array<string> labels = new array<string>();
        bool host = m_V.IsHost == 1;
        bool seated = m_V.Mine >= 0;
        bool playing = BWPlaying();
        if (host && !playing)
        {
            m_BWHostMoves.Insert("#deal");
            labels.Insert(Tr("STR_OZG_BWIN_BEGIN"));
            string variant = VariantLabel();
            if (variant != "")
            {
                m_BWHostMoves.Insert("#variant");
                labels.Insert(Tr1("STR_OZG_WIN_VARIANT", variant));
            }
        }
        if (!host && seated && m_V.HostFree == 1)
        {
            m_BWHostMoves.Insert("#host");
            labels.Insert(Tr("STR_OZG_WIN_TAKE_HOST"));
        }
        int other = BWTopSeat();
        if (host && other >= 0 && other < m_V.Seats.Count() && other != m_V.Mine)
        {
            int st = m_V.Seats[other].St;
            if (st == OZG_Const.SEAT_LEFT)
            {
                m_BWHostMoves.Insert("#drop");
                labels.Insert(Tr("STR_OZG_WIN_DROP_LEFT"));
            }
            else if (st == OZG_Const.SEAT_TAKEN || st == OZG_Const.SEAT_KEPT)
            {
                m_BWHostMoves.Insert("#kick");
                labels.Insert(Tr("STR_OZG_BWIN_KICK"));
            }
        }
        if (seated)
        {
            string pose = OZG_PoseDriver.LabelNow(m_V);
            if (pose != "")
            {
                m_BWHostMoves.Insert("#pose");
                labels.Insert(Tr1("STR_OZG_WIN_POSE", pose));
            }
        }
        for (int i = 0; i < 4; i++)
        {
            string n = "OZG_BHost" + i.ToString();
            bool shown = i < m_BWHostMoves.Count();
            BWShow(n, shown);
            if (shown)
                BWSetText(n + "Text", labels[i]);
        }
    }

    // ------------------------------------------------------------ the field

    protected void BWPaintField()
    {
        ImageWidget board = ImageWidget.Cast(BW("OZG_BBoard"));
        if (board)
        {
            string picture = BOARD_WHITE;
            if (m_BFlip)
                picture = BOARD_BLACK;
            board.LoadImageFile(0, picture);
            board.SetImage(0);
        }
        if (!m_BPub)
        {
            BWSlidesClear();
            m_BWShown = "";
            return;
        }
        // What moved since the field drawn last slides, what went fades.
        bool newer = m_BWShown != "" && m_BWShown != m_BPub.Board && m_V.Rev > m_BWShownRev;
        string was = m_BWShown;
        if (newer)
            BWPlanSlides(was, m_BPub.Board);
        else if (m_BWShown != m_BPub.Board)
            BWSlidesClear();
        BWPaintLights();
        BWPaintPieces();
        if (newer)
            BWPlanFades(was, m_BPub.Board);
        BWPaintDots();
        m_BWShown = m_BPub.Board;
        m_BWShownRev = m_V.Rev;
        BWTickPieces(0);
    }

    // The last move, a check, the squares picked, a sleight's first square and the
    // squares of a sleight seen: the canvas's tints.
    protected void BWPaintLights()
    {
        array<int> last = new array<int>();
        string tail;
        BoardPath(m_BPub.Last, last, tail);
        float x;
        float y;
        int i;
        if (last.Count() >= 2)
        {
            for (i = 0; i < last.Count(); i++)
            {
                if (last[i] < 0)
                    continue;
                SquareCorner(last[i], x, y);
                BWTint(BW_FIELD, x, y, BW_S, BW_S, 2, ARGB(92, 214, 170, 90));
            }
        }
        array<int> picked = Picked();
        if (picked.Count() > 0)
        {
            SquareCorner(picked[0], x, y);
            BWTint(BW_FIELD, x, y, BW_S, BW_S, 3, ARGB(158, 214, 170, 90));
            for (i = 1; i < picked.Count(); i++)
            {
                SquareCorner(picked[i], x, y);
                BWTint(BW_FIELD, x, y, BW_S, BW_S, 3, ARGB(110, 214, 170, 90));
            }
        }
        if (m_BWSlFirst >= 0)
        {
            SquareCorner(m_BWSlFirst, x, y);
            BWTint(BW_FIELD, x, y, BW_S, BW_S, 3, ARGB(77, 231, 194, 122));
            BWRing(BW_FIELD, x, y, BW_S, BW_S, 4, 4, ARGB(255, 231, 194, 122));
        }
        if (m_BPub.Check >= 0)
        {
            SquareCorner(m_BPub.Check, x, y);
            BWPic(BW_FIELD, BW_TEX + "ozg_check_ca.paa", x, y, BW_S, BW_S, 4, ARGB(255, 225, 45, 30));
        }
        if (m_SawSquares && GetGame().GetTime() < m_SawUntil)
        {
            for (i = 0; i < m_SawSquares.Count(); i++)
            {
                SquareCorner(m_SawSquares[i], x, y);
                BWTint(BW_FIELD, x, y, BW_S, BW_S, 3, ARGB(71, 255, 138, 61));
                BWRing(BW_FIELD, x, y, BW_S, BW_S, 5, 4, ARGB(255, 255, 138, 61));
            }
        }
    }

    // The pieces, the far rows first: a piece stands over the square behind it.
    protected void BWPaintPieces()
    {
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                int sq = SquareOfCell(row, col);
                string letter = m_BPub.Board.Get(sq);
                if (letter == ".")
                    continue;
                float x;
                float y;
                SquareCorner(sq, x, y);
                BWPieceAt(letter, sq, x, y, 10 + row * 3);
            }
        }
    }

    // Where the piece picked may go: a ring round a piece it would take, a dot on an
    // empty square; for a sleight armed, the squares it may pick, in gold.
    protected void BWPaintDots()
    {
        float x;
        float y;
        int i;
        if (m_BWSl != "")
        {
            for (int sq = 0; sq < 64; sq++)
            {
                if (!BWSlCandidate(sq))
                    continue;
                SquareCorner(sq, x, y);
                if (m_BPub.Board.Get(sq) != ".")
                    BWPic(BW_FIELD, BW_TEX + "ozg_ring4_ca.paa", x + 5.125, y + 5.125, 86, 86, 60, ARGB(217, 231, 194, 122));
                else
                    BWPic(BW_FIELD, BW_TEX + "ozg_disc_ca.paa", x + 40.125, y + 40.125, 16, 16, 60, ARGB(179, 231, 194, 122));
            }
            return;
        }
        array<int> next = new array<int>();
        PickNext(next);
        for (i = 0; i < next.Count(); i++)
        {
            SquareCorner(next[i], x, y);
            if (m_BPub.Board.Get(next[i]) != ".")
                BWPic(BW_FIELD, BW_TEX + "ozg_ring6_ca.paa", x + 5.125, y + 5.125, 86, 86, 60, ARGB(140, 34, 20, 8));
            else
                BWPic(BW_FIELD, BW_TEX + "ozg_disc_ca.paa", x + 35.125, y + 35.125, 26, 26, 60, ARGB(115, 34, 20, 8));
        }
    }

    // ------------------------------------------------------------ slides and fades

    protected void BWSlidesClear()
    {
        if (!m_BWSlideSq)
            return;
        m_BWSlideSq.Clear();
        m_BWSlideDX.Clear();
        m_BWSlideDY.Clear();
        m_BWSlideT.Clear();
        for (int i = 0; i < m_BWFadeW.Count(); i++)
        {
            if (m_BWFadeW[i] && m_BWFadeIn[i] == 0)
                m_BWFadeW[i].Show(false);
        }
        m_BWFadeW.Clear();
        m_BWFadeT.Clear();
        m_BWFadeIn.Clear();
        m_BWFadeA.Clear();
    }

    // The same kind of piece leaving one square and arriving on another moved there
    // (castling moves two); a man crowned on arriving is still that man.
    protected void BWPlanSlides(string was, string now)
    {
        BWSlidesClear();
        array<int> gone = new array<int>();
        array<int> came = new array<int>();
        BWDiff(was, now, gone, came);
        array<int> used = new array<int>();
        for (int c = 0; c < came.Count(); c++)
        {
            string arrived = BWKind(now.Get(came[c]));
            int best = -1;
            int bestD = 99;
            for (int g = 0; g < gone.Count(); g++)
            {
                if (used.Find(g) >= 0 || BWKind(was.Get(gone[g])) != arrived)
                    continue;
                int d = BWDistance(gone[g], came[c]);
                if (d < bestD)
                {
                    bestD = d;
                    best = g;
                }
            }
            if (best < 0)
                continue;
            used.Insert(best);
            float fx;
            float fy;
            float tx;
            float ty;
            SquareCorner(gone[best], fx, fy);
            SquareCorner(came[c], tx, ty);
            m_BWSlideSq.Insert(came[c]);
            m_BWSlideDX.Insert(fx - tx);
            m_BWSlideDY.Insert(fy - ty);
            m_BWSlideT.Insert(0);
        }
    }

    // What left the field and stayed off fades out where it stood; what arrived from
    // nowhere (a piece put back, a pawn become a queen) fades in.
    protected void BWPlanFades(string was, string now)
    {
        array<int> gone = new array<int>();
        array<int> came = new array<int>();
        BWDiff(was, now, gone, came);
        for (int i = 0; i < m_BWPieceW.Count(); i++)
        {
            int sq = m_BWPieceSq[i];
            if (came.Find(sq) < 0 || m_BWSlideSq.Find(sq) >= 0)
                continue;
            m_BWFadeW.Insert(m_BWPieceW[i]);
            m_BWFadeT.Insert(0);
            m_BWFadeIn.Insert(1);
            m_BWFadeA.Insert(m_BWPieceA[i]);
        }
        // the pieces gone and not slid: laid again where they stood, fading out
        for (int g = 0; g < gone.Count(); g++)
        {
            string letter = was.Get(gone[g]);
            bool slid = false;
            for (int s = 0; s < m_BWSlideSq.Count(); s++)
            {
                float fx;
                float fy;
                SquareCorner(m_BWSlideSq[s], fx, fy);
                float gx;
                float gy;
                SquareCorner(gone[g], gx, gy);
                if (Math.AbsFloat(fx + m_BWSlideDX[s] - gx) < 1 && Math.AbsFloat(fy + m_BWSlideDY[s] - gy) < 1)
                    slid = true;
            }
            if (slid)
                continue;
            int before = m_BWPieceW.Count();
            float x;
            float y;
            SquareCorner(gone[g], x, y);
            BWPieceAt(letter, -1, x, y, 9);
            for (int k = before; k < m_BWPieceW.Count(); k++)
            {
                m_BWFadeW.Insert(m_BWPieceW[k]);
                m_BWFadeT.Insert(0);
                m_BWFadeIn.Insert(0);
                m_BWFadeA.Insert(m_BWPieceA[k]);
            }
        }
    }

    protected void BWDiff(string was, string now, array<int> gone, array<int> came)
    {
        gone.Clear();
        came.Clear();
        if (was.Length() != 64 || now.Length() != 64)
            return;
        for (int sq = 0; sq < 64; sq++)
        {
            string a = was.Get(sq);
            string b = now.Get(sq);
            if (a == b)
                continue;
            if (a != ".")
                gone.Insert(sq);
            if (b != ".")
                came.Insert(sq);
        }
    }

    // A man and a king of draughts are one piece moving (crowned as it lands).
    protected string BWKind(string letter)
    {
        if (!BoardDiscs())
            return letter;
        string low = letter;
        low.ToLower();
        return low;
    }

    protected int BWDistance(int a, int b)
    {
        int dx = Math.AbsInt((a % 8) - (b % 8));
        int dy = Math.AbsInt((a / 8) - (b / 8));
        return dx + dy;
    }

    // Every piece where it is now in its slide, every fade where it is in its fade.
    protected void BWTickPieces(float dt)
    {
        int i;
        for (i = m_BWSlideT.Count() - 1; i >= 0; i--)
        {
            m_BWSlideT[i] = m_BWSlideT[i] + dt;
            if (m_BWSlideT[i] >= BW_SLIDE)
            {
                m_BWSlideSq.Remove(i);
                m_BWSlideDX.Remove(i);
                m_BWSlideDY.Remove(i);
                m_BWSlideT.Remove(i);
            }
        }
        for (i = 0; i < m_BWPieceW.Count(); i++)
        {
            Widget w = m_BWPieceW[i];
            if (!w)
                continue;
            float ox = 0;
            float oy = 0;
            int s = m_BWSlideSq.Find(m_BWPieceSq[i]);
            if (s >= 0 && m_BWPieceSq[i] >= 0)
            {
                float t = m_BWSlideT[s] / BW_SLIDE;
                float e = 1 - t * t * (3 - 2 * t);
                ox = m_BWSlideDX[s] * e;
                oy = m_BWSlideDY[s] * e;
            }
            w.SetPos((m_BWPieceX[i] + ox) * m_K, (m_BWPieceY[i] + oy) * m_K);
        }
        for (i = m_BWFadeW.Count() - 1; i >= 0; i--)
        {
            m_BWFadeT[i] = m_BWFadeT[i] + dt;
            float f = m_BWFadeT[i] / BW_FADE;
            if (f > 1)
                f = 1;
            float a = f;
            if (m_BWFadeIn[i] == 0)
                a = 1 - f;
            Widget fw = m_BWFadeW[i];
            if (fw)
            {
                fw.SetAlpha(a * m_BWFadeA[i]);
                if (m_BWFadeIn[i] == 0 && f >= 1)
                    fw.Show(false);
            }
            if (f >= 1)
            {
                m_BWFadeW.Remove(i);
                m_BWFadeT.Remove(i);
                m_BWFadeIn.Remove(i);
                m_BWFadeA.Remove(i);
            }
        }
    }

    // ------------------------------------------------------------ the dialogs

    protected void BWPaintDialogs()
    {
        // The menu: the games of this board, the one played lit.
        BWShow("OZG_BMenu", m_BWMenuOpen && BWMenuAllowed());
        if (m_BWMenuOpen && BWMenuAllowed())
        {
            m_BWMenuGames.Clear();
            for (int g = 0; g < m_V.GameIds.Count() && m_BWMenuGames.Count() < 6; g++)
                m_BWMenuGames.Insert(m_V.GameIds[g]);
            for (int i = 0; i < 6; i++)
            {
                string n = "OZG_BMenuItem" + i.ToString();
                bool shown = i < m_BWMenuGames.Count();
                BWShow(n, shown);
                if (!shown)
                    continue;
                OZG_Game sample = OZG_Games.Sample(m_BWMenuGames[i]);
                string label = m_BWMenuGames[i];
                if (sample)
                    label = Tr(sample.LabelKey());
                BWSetText(n + "Text", label);
                BWShow(n + "Bg", m_BWMenuGames[i] == m_V.GameId);
            }
            array<string> foot = new array<string>();
            BWWrap(BWText("OZG_BMenuFoot0"), Tr("STR_OZG_BWIN_MENU_FOOT"), 200, 2, foot);
            string f0 = "";
            string f1 = "";
            if (foot.Count() > 0)
                f0 = foot[0];
            if (foot.Count() > 1)
                f1 = foot[1];
            BWSetText("OZG_BMenuFoot0", f0);
            BWSetText("OZG_BMenuFoot1", f1);
        }

        // The pawn's promotion: the four pieces of its set.
        bool promo = m_PromoTo >= 0 && m_BPub;
        BWShow("OZG_BPromo", promo);
        if (promo)
        {
            bool white = m_V.Mine == m_BPub.White;
            string kinds = "QRBN";
            for (int k = 0; k < 4; k++)
            {
                string letter = kinds.Get(k);
                if (!white)
                    letter.ToLower();
                ImageWidget pic = ImageWidget.Cast(BW("OZG_BPromo" + kinds.Get(k) + "Pic"));
                if (pic)
                {
                    pic.LoadImageFile(0, BWSmallPicture(letter));
                    pic.SetImage(0);
                }
            }
            array<string> say = new array<string>();
            BWWrap(BWText("OZG_BPromoText0"), Tr("STR_OZG_BWIN_PROMO_TEXT"), 456, 2, say);
            string p0 = "";
            string p1 = "";
            if (say.Count() > 0)
                p0 = say[0];
            if (say.Count() > 1)
                p1 = say[1];
            BWSetText("OZG_BPromoText0", p0);
            BWSetText("OZG_BPromoText1", p1);
        }

        // The game over: why, and a new game for the host; shut, it stays shut until the
        // next game ends.
        if (!BWOver())
            m_BWOverShut = -1;
        bool over = BWOver() && m_BWOverShut < 0;
        BWShow("OZG_BOver", over);
        if (over)
            BWPaintOver();

        // Folding asked.
        BWShow("OZG_BFold", m_BWFoldAsk);
        if (m_BWFoldAsk)
        {
            array<string> fold = new array<string>();
            BWWrap(BWText("OZG_BFoldText0"), Tr("STR_OZG_BWIN_FOLD_TEXT"), 412, 2, fold);
            string q0 = "";
            string q1 = "";
            if (fold.Count() > 0)
                q0 = fold[0];
            if (fold.Count() > 1)
                q1 = fold[1];
            BWSetText("OZG_BFoldText0", q0);
            BWSetText("OZG_BFoldText1", q1);
        }

        // A note of the server or a sleight seen: under the board a while.
        BWShow("OZG_BSeen", m_BWToastLeft > 0);
    }

    protected void BWPaintOver()
    {
        string text = "";
        if (m_BPub.Reason != "")
            text = Tr(m_BPub.Reason);
        string result = BWResultWords();
        if (result != "")
        {
            if (text != "")
                text = text + ". ";
            text = text + result;
        }
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_BOverText0"), text, 412, 2, lines);
        string o0 = "";
        string o1 = "";
        if (lines.Count() > 0)
            o0 = lines[0];
        if (lines.Count() > 1)
            o1 = lines[1];
        BWSetText("OZG_BOverText0", o0);
        BWSetText("OZG_BOverText1", o1);
        // one line: the box as the canvas has it; two: 21 taller
        float extra = 0;
        if (lines.Count() > 1)
            extra = 21;
        Widget box = BW("OZG_BOverBox");
        if (box)
        {
            box.SetPos(570 * m_K, (367.5 - extra / 2) * m_K);
            box.SetSize(460 * m_K, (165 + extra) * m_K);
            Widget edge = BW("OZG_BOverBoxEdge");
            Widget bg = BW("OZG_BOverBoxBg");
            BWFit(edge, 462, 167 + extra, 9);
            BWFit(bg, 460, 165 + extra, 8);
        }
        BWMove(BW("OZG_BOverTitle"), 594, 389.5 - extra / 2);
        BWMove(BW("OZG_BOverText0"), 594, 435.5 - extra / 2);
        BWMove(BW("OZG_BOverText1"), 594, 456.5 - extra / 2);
        BWMove(BW("OZG_BOverBtnNew"), 594, 470.5 + extra / 2);
        BWMove(BW("OZG_BOverBtnLook"), 804, 470.5 + extra / 2);
        BWShow("OZG_BOverBtnNew", m_V.IsHost == 1);
    }

    // "You won", "Тарас won (black)", "A draw": the result from this window's side.
    protected string BWResultWords()
    {
        if (!m_BPub || m_BPub.Final == "")
            return "";
        if (m_BPub.Final == "1/2")
            return Tr("STR_OZG_BWIN_RESULT_DRAW");
        int winner = m_BPub.White;
        if (m_BPub.Final == "0-1")
            winner = m_BPub.Black;
        if (winner == m_V.Mine)
            return Tr("STR_OZG_BWIN_RESULT_YOU");
        string side = Tr("STR_OZG_BWIN_SIDE_WHITE");
        if (winner == m_BPub.Black)
            side = Tr("STR_OZG_BWIN_SIDE_BLACK");
        return Tr2("STR_OZG_BWIN_RESULT_WON", SeatName(winner), side);
    }

    // A line under the board (the canvas's "seen" box): a sleight seen, a refusal.
    protected void BWToast(string text, bool hot)
    {
        TextWidget t = BWText("OZG_BSeenText");
        if (!t)
            return;
        BWShow("OZG_BSeen", true);
        array<string> lines = new array<string>();
        BWWrap(t, text, 708, 1, lines);
        string first = "";
        if (lines.Count() > 0)
            first = lines[0];
        t.SetText(first);
        int ring = ARGB(255, 107, 82, 51);
        if (hot)
            ring = ARGB(255, 255, 138, 61);
        BWPaintNamed("OZG_BSeenBoxEdge", ring);
        m_BWToastLeft = 8.0;
        BWShow("OZG_BSeen", true);
    }

    // ------------------------------------------------------------ sleight of hand

    protected void BWSlClear()
    {
        m_BWSl = "";
        m_BWSlFirst = -1;
        m_BWSlTaken = "";
        m_BWSlTakenAt = -1;
    }

    protected void BWSay(string text, int tone)
    {
        m_BWMsg = text;
        m_BWMsgTone = tone;
    }

    // A sleight's button: armed, or disarmed if it was; refused while waiting.
    protected void BWArm(string kind)
    {
        if (!BWPlaying() || m_V.BoardSleights != 1)
            return;
        int left = BWWaitLeft();
        if (left > 0)
        {
            BWSlClear();
            BWSay(Tr1("STR_OZG_BWIN_SL_WAIT", BWClock(left)), 2);
            BoardWinPaint();
            return;
        }
        if (m_BWSl == kind)
        {
            BWSlClear();
            BWSay("", 0);
            BoardWinPaint();
            return;
        }
        PickClear();
        BWSlClear();
        m_BWSl = kind;
        string hint = "STR_OZG_BWIN_SL_HINT_";
        if (m_Style == "board_nardy")
            hint = "STR_OZG_BWIN_SL_NHINT_";
        else if (BoardDiscs())
            hint = "STR_OZG_BWIN_SL_DHINT_";
        BWSay(Tr(hint + BWUpper(kind)), 1);
        BoardWinPaint();
    }

    protected string BWUpper(string s)
    {
        string up = s;
        up.ToUpper();
        return up;
    }

    // Whether a square may be picked for the sleight armed (the canvas's rules; the
    // server checks them again).
    protected bool BWSlCandidate(int sq)
    {
        string here = m_BPub.Board.Get(sq);
        bool occupied = here != ".";
        bool king = occupied && !BoardDiscs() && (here == "K" || here == "k");
        if (m_BWSl == OZG_DuelConst.S_OFF)
            return occupied && !king;
        if (m_BWSl == OZG_DuelConst.S_MOVE)
        {
            if (m_BWSlFirst < 0)
                return occupied;
            return !occupied;
        }
        if (m_BWSl == OZG_DuelConst.S_SWAP)
            return occupied && sq != m_BWSlFirst;
        if (m_BWSl == OZG_DuelConst.S_BACK)
            return m_BWSlTaken != "" && !king;
        return false;
    }

    protected void BWSlClick(int sq)
    {
        if (sq < 0 || !BWSlCandidate(sq))
            return;
        string name = OZG_ChessBoard.SqName(sq);
        if (m_BWSl == OZG_DuelConst.S_OFF)
        {
            BWSlSend(OZG_DuelConst.S_OFF + ":" + name + ":-");
            return;
        }
        if (m_BWSl == OZG_DuelConst.S_BACK)
        {
            BWSlSend(OZG_DuelConst.S_BACK + ":" + m_BWSlTaken + ":" + name);
            return;
        }
        if (m_BWSlFirst < 0)
        {
            m_BWSlFirst = sq;
            if (m_BWSl == OZG_DuelConst.S_MOVE)
                BWSay(Tr("STR_OZG_BWIN_SL_HINT_MOVE2"), 1);
            else
                BWSay(Tr("STR_OZG_BWIN_SL_HINT_SWAP2"), 1);
            BoardWinPaint();
            return;
        }
        BWSlSend(m_BWSl + ":" + OZG_ChessBoard.SqName(m_BWSlFirst) + ":" + name);
    }

    // Asked of the server; whether the other player saw it is said when it is done.
    protected void BWSlSend(string move)
    {
        int other = BWTopSeat();
        m_BWSlSent = true;
        m_BWSlSentWho = "";
        if (other >= 0 && BWLooks(other))
            m_BWSlSentWho = SeatName(other);
        m_BWSlSentAway = "";
        if (other >= 0 && !BWLooks(other))
            m_BWSlSentAway = SeatName(other);
        m_BWSlSentRev = m_ShownRev;
        Send(OZG_Const.OP_BSLEIGHT, move, -1, "", "", "", 0);
        BWSlClear();
        BoardWinPaint();
    }

    protected string m_BWSlSentAway = "";
    protected int m_BWSlSentRev = -1;

    // The view after a sleight asked: it was done (a refusal comes through Said).
    protected void BWSlAnswered()
    {
        if (m_ShownRev <= m_BWSlSentRev)
            return;
        m_BWSlSent = false;
        if (m_BWSlSentWho != "")
            BWSay(Tr1("STR_OZG_BWIN_SL_DONE_SEEN", m_BWSlSentWho), 1);
        else if (m_BWSlSentAway != "")
            BWSay(Tr1("STR_OZG_BWIN_SL_DONE_AWAY", m_BWSlSentAway), 1);
        else
            BWSay(Tr("STR_OZG_BWIN_SL_DONE"), 1);
    }

    // A taken piece picked for "put back", in the row clicked.
    protected void BWTakenClick(bool bottom)
    {
        if (m_BWSl != OZG_DuelConst.S_BACK || !m_BPub)
            return;
        float x;
        float y;
        PointerAt(x, y);
        float left = 16 + 12;
        if (bottom)
            left = 1254 + 12;
        bool takerWhite = true;
        int seat = BWTopSeat();
        if (bottom)
            seat = BWBottomSeat();
        if (m_BPub.White >= 0)
            takerWhite = seat == m_BPub.White;
        array<string> pieces = new array<string>();
        for (int i = 0; i < m_BPub.Taken.Length(); i++)
        {
            string letter = m_BPub.Taken.Get(i);
            if (BWWhite(letter) != takerWhite)
                pieces.Insert(letter);
        }
        if (pieces.Count() == 0)
            return;
        float step = 26;
        if (pieces.Count() > 11)
            step = 280.0 / (pieces.Count() - 1);
        int k = (x - left) / step;
        if (k < 0 || k >= pieces.Count())
            return;
        int pool = BW_OPP;
        if (bottom)
            pool = BW_ME;
        m_BWSlTaken = pieces[k];
        m_BWSlTakenAt = k + 100 * pool;
        BWSay(Tr("STR_OZG_BWIN_SL_HINT_BACK2"), 1);
        BoardWinPaint();
    }

    // A sleight seen on the board (§9.1): said under it, its squares lit a while.
    void BoardSaw(OZG_WBoardSaw saw)
    {
        BWInit();
        m_SawSquares.Clear();
        int a = OZG_ChessBoard.SqOf(saw.A);
        int b = OZG_ChessBoard.SqOf(saw.B);
        if (a >= 0)
            m_SawSquares.Insert(a);
        if (b >= 0)
            m_SawSquares.Insert(b);
        m_SawUntil = GetGame().GetTime() + 8000;
        string key = "STR_OZG_BSAW_" + saw.Kind;
        key.ToUpper();
        string line = Tr(key);
        line.Replace("%1", SeatName(saw.SeatNo));
        line.Replace("%2", saw.A);
        line.Replace("%3", saw.B);
        if (BWUp())
        {
            BWToast(line, true);
            BoardWinPaint();
            return;
        }
        Said(line);
    }

    // Words with their arguments in this client's language, for what the server
    // says outside a view (a notice).
    string WordsOf(string words)
    {
        return Words(words);
    }

    // A refusal or a note: in the panel when a sleight was asked, else under the board.
    override void Said(string text)
    {
        if (!BWUp())
        {
            super.Said(text);
            return;
        }
        if (text == "")
            return;
        string words = BWWords(text);
        if (m_BWSlSent)
        {
            m_BWSlSent = false;
            BWSay(words, 2);
            BoardWinPaint();
            return;
        }
        BWToast(words, false);
    }

    // ------------------------------------------------------------ moves

    protected array<int> Picked()
    {
        if (!m_PickPath)
            m_PickPath = new array<int>();
        return m_PickPath;
    }

    protected void PickClear()
    {
        Picked().Clear();
        m_PromoFrom = -1;
        m_PromoTo = -1;
    }

    // The squares of a move as it goes on the wire: "e2e4" and "e7e8q" (the letter
    // left over in `tail`), "c3-d4", "c3:e5:c7".
    protected void BoardPath(string move, array<int> into, out string tail)
    {
        into.Clear();
        tail = "";
        if (move.IndexOf("-") >= 0 || move.IndexOf(":") >= 0)
        {
            OZG_DraughtsBoard.PathOf(move, into);
            return;
        }
        if (move.Length() < 4)
            return;
        into.Insert(OZG_ChessBoard.SqOf(move.Substring(0, 2)));
        into.Insert(OZG_ChessBoard.SqOf(move.Substring(2, 2)));
        if (move.Length() > 4)
            tail = move.Substring(4, move.Length() - 4);
    }

    protected bool PathBegins(string move, array<int> prefix, out int count)
    {
        array<int> path = new array<int>();
        string tail;
        BoardPath(move, path, tail);
        count = path.Count();
        if (path.Count() < prefix.Count() || prefix.Count() == 0)
            return false;
        for (int i = 0; i < prefix.Count(); i++)
        {
            if (path[i] != prefix[i])
                return false;
        }
        return true;
    }

    // This seat's moves whose path is exactly `path` (more than one for a pawn that
    // becomes a piece).
    protected void MovesOfPath(array<int> path, array<string> into)
    {
        into.Clear();
        if (!m_BOwn || !m_BOwn.CanPlay)
            return;
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (PathBegins(m_BOwn.CanPlay[i], path, count) && count == path.Count())
                into.Insert(m_BOwn.CanPlay[i]);
        }
    }

    protected array<string> PickExact()
    {
        array<string> exact = new array<string>();
        if (Picked().Count() >= 2)
            MovesOfPath(Picked(), exact);
        return exact;
    }

    protected void PickNext(array<int> into)
    {
        into.Clear();
        if (!m_BOwn || !m_BOwn.CanPlay || Picked().Count() == 0)
            return;
        int at = Picked().Count();
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (!PathBegins(m_BOwn.CanPlay[i], Picked(), count) || count <= at)
                continue;
            array<int> path = new array<int>();
            string tail;
            BoardPath(m_BOwn.CanPlay[i], path, tail);
            if (into.Find(path[at]) < 0)
                into.Insert(path[at]);
        }
    }

    protected bool MovesFrom(int sq)
    {
        if (!m_BOwn || !m_BOwn.CanPlay)
            return false;
        array<int> start = new array<int>();
        start.Insert(sq);
        for (int i = 0; i < m_BOwn.CanPlay.Count(); i++)
        {
            int count;
            if (PathBegins(m_BOwn.CanPlay[i], start, count) && count >= 2)
                return true;
        }
        return false;
    }

    // A click on the field: a sleight's square, or a move picked a square a click,
    // played when only one move is left that ends there.
    protected void BWFieldClick()
    {
        if (!m_BPub)
            return;
        float px;
        float py;
        PointerAt(px, py);
        int sq = SquareAt(px, py);
        if (m_BWSl != "")
        {
            BWSlClick(sq);
            return;
        }
        if (sq < 0)
        {
            PickClear();
            BoardWinPaint();
            return;
        }
        array<int> path = Picked();
        int picked = path.Count();
        if (picked >= 2 && sq == path[picked - 1])
        {
            array<string> here = PickExact();
            if (here.Count() == 1)
            {
                PickClear();
                Play(here[0]);
                return;
            }
        }
        if (picked > 0)
        {
            array<int> next = new array<int>();
            PickNext(next);
            if (next.Find(sq) >= 0)
            {
                Picked().Insert(sq);
                array<int> further = new array<int>();
                PickNext(further);
                array<string> exact = PickExact();
                if (further.Count() == 0 && exact.Count() == 1)
                {
                    PickClear();
                    Play(exact[0]);
                    return;
                }
                if (further.Count() == 0 && exact.Count() > 1)
                {
                    m_PromoFrom = path[0];
                    m_PromoTo = sq;
                }
                BoardWinPaint();
                return;
            }
        }
        Picked().Clear();
        if (MovesFrom(sq))
            Picked().Insert(sq);
        BoardWinPaint();
    }

    protected void BWPromote(string kind)
    {
        int from = m_PromoFrom;
        int to = m_PromoTo;
        PickClear();
        if (from < 0 || to < 0)
            return;
        string letter = kind;
        letter.ToLower();
        Play(OZG_ChessBoard.SqName(from) + OZG_ChessBoard.SqName(to) + letter);
    }

    // ------------------------------------------------------------ clicks and time

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (!BWUp())
            return super.OnClick(w, x, y, button);
        if (!w || button != MouseState.LEFT || !m_V)
            return false;
        string n = w.GetName();
        if (n == "OZG_BBtnClose")
        {
            Shut();
            return true;
        }
        // An open dialog takes the clicks meant for it, and no others.
        if (m_BWFoldAsk)
        {
            if (n == "OZG_BFoldBtnYes")
                Send(OZG_Const.OP_FOLD, "", -1, "", "", "", 0);
            if (n == "OZG_BFoldBtnYes" || n == "OZG_BFoldBtnNo")
            {
                m_BWFoldAsk = false;
                BoardWinPaint();
            }
            return true;
        }
        if (BW("OZG_BOver") && BW("OZG_BOver").IsVisible())
        {
            if (n == "OZG_BOverBtnNew")
                Send(OZG_Const.OP_DEAL, "", -1, "", "", "", 0);
            if (n == "OZG_BOverBtnLook" || n == "OZG_BOverBtnNew")
            {
                m_BWOverShut = 1;
                BoardWinPaint();
            }
            return true;
        }
        if (m_PromoTo >= 0)
        {
            if (n.IndexOf("OZG_BPromo") == 0 && n.Length() == 11)
            {
                BWPromote(n.Substring(10, 1));
                return true;
            }
            PickClear();
            BoardWinPaint();
            return true;
        }
        if (m_BWMenuOpen)
        {
            m_BWMenuOpen = false;
            if (n.IndexOf("OZG_BMenuItem") == 0)
            {
                int i = n.Substring(13, n.Length() - 13).ToInt();
                if (i >= 0 && i < m_BWMenuGames.Count() && m_BWMenuGames[i] != m_V.GameId)
                    Send(OZG_Const.OP_GAME, "", -1, "", "", m_BWMenuGames[i], 0);
            }
            BoardWinPaint();
            return true;
        }
        // the dice games' window (OZG_GameMenuDice.c)
        if (m_Style == "dice" && DWClick(n))
            return true;
        // nardy's board and its own buttons (OZG_GameMenuNardy.c)
        if (m_Style == "board_nardy" && BNClick(n))
            return true;
        if (n == "OZG_BHit")
        {
            BWFieldClick();
            return true;
        }
        if (n == "OZG_BBtnGame")
        {
            if (BWMenuAllowed())
            {
                m_BWMenuOpen = true;
                BoardWinPaint();
            }
            return true;
        }
        if (n == "OZG_BBtnFold")
        {
            if (m_V.IsHost == 1)
                m_BWFoldAsk = true;
            else if (m_V.Mine >= 0)
                Send(OZG_Const.OP_STAND, "", -1, OZG_Const.HOW_LEAVE, "", "", 0);
            BoardWinPaint();
            return true;
        }
        if (n == "OZG_BBtnDraw")
        {
            if (m_BWDrawMove != "")
                Play(m_BWDrawMove);
            return true;
        }
        if (n == "OZG_BBtnResign")
        {
            if (m_BWResignMove != "")
                Play(m_BWResignMove);
            return true;
        }
        if (n == "OZG_BSlMove")
        {
            BWArm(OZG_DuelConst.S_MOVE);
            return true;
        }
        if (n == "OZG_BSlSwap")
        {
            BWArm(OZG_DuelConst.S_SWAP);
            return true;
        }
        if (n == "OZG_BSlBack")
        {
            BWArm(OZG_DuelConst.S_BACK);
            return true;
        }
        if (n == "OZG_BSlOff")
        {
            BWArm(OZG_DuelConst.S_OFF);
            return true;
        }
        if (n == "OZG_BOppTakenHit")
        {
            BWTakenClick(false);
            return true;
        }
        if (n == "OZG_BMeTakenHit")
        {
            BWTakenClick(true);
            return true;
        }
        if (n.IndexOf("OZG_BHost") == 0 && n.Length() == 10)
        {
            int k = n.Substring(9, 1).ToInt();
            if (k >= 0 && k < m_BWHostMoves.Count())
                BWHostClick(m_BWHostMoves[k]);
            return true;
        }
        return false;
    }

    protected void BWHostClick(string move)
    {
        int other = BWTopSeat();
        if (move == "#deal")
            Send(OZG_Const.OP_DEAL, "", -1, "", "", "", 0);
        else if (move == "#variant")
            NextVariant();
        else if (move == "#host")
            Send(OZG_Const.OP_HOST, "", -1, "", "", "", 0);
        else if (move == "#drop")
            Send(OZG_Const.OP_DROP, "", other, "", "", "", 0);
        else if (move == "#kick")
            Send(OZG_Const.OP_KICK, "", other, OZG_Const.HOW_LEAVE, "", "", 0);
        else if (move == "#pose")
            Send(OZG_Const.OP_POSE, "", -1, "", "", "", OZG_PoseDriver.Next(m_V));
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!BWUp() || !m_Win || m_Closing || !m_V)
            return;
        BWInit();
        if (m_BWSlideT.Count() > 0 || m_BWFadeW.Count() > 0)
            BWTickPieces(timeslice);
        // The sleight's timer, a second at a time; the line under the board, its time.
        m_BWWaitAge = m_BWWaitAge + timeslice;
        m_BWTickAge = m_BWTickAge + timeslice;
        if (m_BWTickAge >= 1.0)
        {
            m_BWTickAge = 0;
            if (BWPlaying() && m_V.BoardSleights == 1)
                BWPaintSleights();
        }
        if (m_BWToastLeft > 0)
        {
            m_BWToastLeft = m_BWToastLeft - timeslice;
            if (m_BWToastLeft <= 0)
                BWShow("OZG_BSeen", false);
        }
        // The squares of a sleight seen go dark after their time.
        if (m_SawSquares && m_SawSquares.Count() > 0 && GetGame().GetTime() >= m_SawUntil)
        {
            m_SawSquares.Clear();
            BoardWinPaint();
        }
    }

    // Every new view restarts the count of the sleight's timer.
    override void Redraw()
    {
        int was = m_ShownRev;
        super.Redraw();
        if (m_ShownRev != was)
            m_BWWaitAge = 0;
    }
}
