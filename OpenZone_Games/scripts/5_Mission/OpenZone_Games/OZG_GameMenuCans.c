// The dice tables' cans and dice (boards and dice design §7, §9.2): the design canvas's
// boards of 2026-11-27 and their parts "Банка", "Кістка", "Вікно підміни кістки" and "Вікно
// «Банка й кості»". A can of dice stands at the seat that throws; a throw is the can shaken
// and tipped and the dice flying out of it onto the felt, seen by everybody at the table; a
// die swapped is a die seen flying into the can -- by the one who does it, and by each who
// noticed. Which can a player throws from and which of its dice is his own choice, made in
// the open (design §8): a button and a window of its own.
//
// What moves is a widget of its own, kept from one paint to the next: a die (ozg_ddie.layout:
// a frame that turns whole, the die in it with its ring, its shadow and what this player
// knows of it) and a can (ozg_dcan.layout). A paint says where each should be and what it
// shows; the clock moves it there the way the canvas's CSS does (a die 0.46 s along
// cubic-bezier(0.2, 0.8, 0.3, 1), a can's turn 0.24 s) and tumbles a die that has just
// fallen.
//
// A throw is played from the view that tells of it: the game's number of throws grew
// (OZG_GPublic.Throws). The window then keeps showing the view it had -- the dice it threw
// gathered back into the can, the can shaken, tipped -- and takes the new one when the
// dice fly out; nothing of the throw is told before it is seen. Dice the new view no longer
// has on the table (a bolt at thousand, a one at pig) lie a moment before they go.
//
// This file loads after OZG_GameMenuBoard.c and before the dice window's and nardy's own
// (the modded chain takes its files in the order of their names): what they call is here.

// A die on a dice table.
class OZG_DTDie
{
    string m_Id;
    Widget m_Root;
    ImageWidget m_RingPic;
    ImageWidget m_ShadowPic;
    ImageWidget m_FacePic;
    Widget m_LoadBox;
    ImageWidget m_LoadBg;
    ImageWidget m_LoadFacePic;
    ImageWidget m_LoadBars;
    Widget m_EyeBox;
    ImageWidget m_EyeRing;
    ImageWidget m_EyeBg;
    ImageWidget m_EyeIcon;
    bool m_Used;
    // made by this paint: it is where it is told to be, at once
    bool m_Fresh;
    // as shown: its middle (units of its parent), its side, its turn, its alpha, its lift
    float m_X;
    float m_Y;
    float m_Side;
    float m_Rot;
    float m_Alpha;
    float m_Lift;
    // where it came from and where it goes, the seconds since it was sent (below 0: there)
    float m_FromX;
    float m_FromY;
    float m_FromSide;
    float m_FromRot;
    float m_FromAlpha;
    float m_FromLift;
    float m_ToX;
    float m_ToY;
    float m_ToSide;
    float m_ToRot;
    float m_ToAlpha;
    float m_ToLift;
    float m_MoveT = -1;
    // how long its way, its turn and its fading take
    float m_MoveLen = 0.46;
    float m_TurnLen = 0.4;
    float m_FadeLen = 0.16;
    // the seconds since it began to tumble (below 0: it lies)
    float m_TumbleT = -1;
    // what it shows: its picture, the side it is drawn for at full size, the colour of its
    // ring (0: none), the load this player knows (a face, how heavy), the eye, lifted
    string m_Picture;
    float m_Nominal = 74;
    int m_RingCol;
    int m_LoadFace;
    int m_LoadStep;
    bool m_EyeOn;
    bool m_Lifted;
    // its parts were laid for this side (pixels) and these marks
    float m_LaidSide = -1;
    string m_LaidMarks;
}

// A can of dice at a seat.
class OZG_DTCan
{
    string m_Id;
    Widget m_Root;
    Widget m_TurnBox;
    ImageWidget m_Body;
    ImageWidget m_Glow;
    ImageWidget m_Pulse;
    ImageWidget m_Shadow;
    ref array<ImageWidget> m_Inside;
    bool m_Used;
    bool m_Fresh;
    // its box's corner as shown (units of its parent), where it came from, where it goes
    float m_X;
    float m_Y;
    float m_FromX;
    float m_FromY;
    float m_ToX;
    float m_ToY;
    float m_MoveT = -1;
    float m_Scale = 1;
    // full, empty, shake, tip, down, peek, up; how far it tips, degrees
    string m_State = "full";
    float m_TipDeg = 105;
    // its turn as shown: moved, turned, its alpha, its shadow's
    float m_Tx;
    float m_Ty;
    float m_Rot;
    float m_Al = 1;
    float m_Sh = 1;
    float m_FromTx;
    float m_FromTy;
    float m_FromRot;
    float m_FromAl;
    float m_FromSh;
    float m_ToTx;
    float m_ToTy;
    float m_ToRot;
    float m_ToAl = 1;
    float m_ToSh = 1;
    float m_TurnT = -1;
    float m_ShakeT = -1;
    float m_PulseT = -1;
    bool m_GlowOn;
    string m_GlowTex;
    string m_BodyTex;
    int m_InsideN;

    void OZG_DTCan()
    {
        m_Inside = new array<ImageWidget>();
    }
}

// Where things stand at a seat of a dice table (the window's units): its plate, its can and
// how big, the mouth the dice fly out of, how the can tips, the hand a slipped die comes
// from, its row of dice and its row of dice put aside (the middle of the first, the step
// to the next, how big they lie).
class OZG_DTSeat
{
    int m_Seat = -1;
    float m_PlateX;
    float m_PlateY;
    float m_PlateW;
    float m_CanX;
    float m_CanY;
    float m_CanK = 1.5;
    float m_MouthX;
    float m_MouthY;
    float m_Tip = 108;
    float m_HandX;
    float m_HandY;
    float m_RowX;
    float m_RowY;
    float m_RowStep;
    float m_RowScale = 1;
    float m_AsideX;
    float m_AsideY;
    float m_AsideStep;
    float m_AsideScale = 0.42;
}

modded class OZG_GameMenu
{
    protected static const string DT_DIE = "OpenZone_Games/gui/layouts/ozg_ddie.layout";
    protected static const string DT_CAN = "OpenZone_Games/gui/layouts/ozg_dcan.layout";
    protected static const string DT_ROW = "OpenZone_Games/gui/layouts/ozg_dsrow.layout";
    protected static const string DT_PROW = "OpenZone_Games/gui/layouts/ozg_dprow.layout";
    // a die's tumble as it falls (the canvas's ozgTumble: 0.55 s)
    protected static const float DT_TUMBLE = 0.55;
    // a can: its turn, its way to another seat, its shake, a swap seen
    protected static const float DT_CAN_TURN = 0.24;
    protected static const float DT_CAN_MOVE = 0.5;
    protected static const float DT_CAN_SHAKE = 0.62;
    protected static const float DT_CAN_PULSE = 0.9;
    // a die slipped in: the whole of it, and its flight
    protected static const float DT_SWAP = 0.64;
    protected static const float DT_SWAP_FLY = 0.5;
    // a note stays this long (the canvas: 4.2 s)
    protected static const float DT_TOAST = 4.2;
    // the dice go home before the table's can goes to the next seat
    protected static const float DT_HOME = 0.34;
    // the game over's box waits for the last throw to be seen
    protected static const float DT_OVER_WAIT = 0.6;

    protected ref array<ref OZG_DTDie> m_DTDice;
    protected ref array<ref OZG_DTCan> m_DTCans;
    // the window the dice and the cans were made in; where their parent lies in it
    protected Widget m_DTWin;
    protected float m_DTOX = 52;
    protected float m_DTOY = 86;

    // ---- a throw being played
    // "", gather, shake, tip, fly, linger, home; the seconds the step has left
    protected string m_DTFx = "";
    protected float m_DTFxLeft;
    // who throws, which of his dice (their places) and what they show
    protected int m_DTThrower = -1;
    protected ref array<int> m_DTThrowAt;
    protected ref array<int> m_DTThrowFaces;
    // the view shown, and the one that waits for the dice to fly out
    protected ref OZG_WView m_DTV;
    protected ref OZG_GPublic m_DTPub;
    protected ref OZG_GPrivate m_DTOwn;
    protected ref OZG_WView m_DTNextV;
    protected ref OZG_GPublic m_DTNextPub;
    protected ref OZG_GPrivate m_DTNextOwn;
    // the views that came while a throw was being played, in their order: each is shown
    // when the one before it has been seen (a throw is never cut short by the next move)
    protected ref array<ref OZG_WView> m_DTQueueV;
    protected ref array<ref OZG_GPublic> m_DTQueuePub;
    protected ref array<ref OZG_GPrivate> m_DTQueueOwn;
    // the seat the table's one can stands at (-1: not placed yet), and where it goes
    // once the dice are home
    protected int m_DTHostAt = -1;
    protected int m_DTHostWant = -1;
    protected float m_DTOverWait;
    // nardy and perudo count the throws themselves
    protected int m_DTThrowsSeen = -1;
    protected string m_DTDealSeen = "";

    // ---- a die slipped in
    protected float m_DTSwapT = -1;
    protected int m_DTSwapSeat = -1;
    protected string m_DTSwapIn = "white";
    protected string m_DTSwapOut = "white";
    protected bool m_DTSwapNoticed;
    protected float m_DTSwapAX;
    protected float m_DTSwapAY;
    protected float m_DTSwapBX;
    protected float m_DTSwapBY;

    // ---- the note
    protected float m_DTToastLeft;

    // ---- the swap's window: open, the die of the pocket picked (its key), the die of the
    // can picked (its place among his dice), the places it offers, its rows
    protected bool m_DTSwapOpen;
    protected string m_DTSwapPick = "";
    protected int m_DTSwapTarget = -1;
    protected ref array<int> m_DTSwapPlaces;
    protected ref array<Widget> m_DTSwapRows;
    protected ref array<ref OZG_DTDie> m_DTSwapDice;
    protected bool m_DTSwapUnder;
    // the faces the two dice of a swap fly on (a die is never drawn blank)
    protected int m_DTSwapFaceIn = 1;
    protected int m_DTSwapFaceOut = 1;
    // a number taken each time a window that lists dice opens: the faces its dice lie on
    // (OZG_Keys.FaceOf) -- the same while it is open, others the next time
    protected int m_DTFaceSalt;

    // ---- the window of the can and the dice: open, the can chosen in it (its key of the
    // view's pick lists), the dice of it chosen (their keys), the dice it shows now (their
    // keys, in their order), its rows and its dice
    protected bool m_DTPickOpen;
    // just opened: its list is scrolled to the can chosen
    protected bool m_DTPickReveal;
    protected string m_DTPickCan = "";
    protected ref array<string> m_DTPickKeys;
    protected ref array<string> m_DTPickShown;
    protected ref array<Widget> m_DTPickRows;
    protected ref array<ref OZG_DTDie> m_DTPickDice;
    // the table shows something a swap may not be made over (a showdown, the cans shaken)
    protected bool m_DTBlocked;
    // whose dice the variant chosen says, read once a paint (between games)
    protected string m_DTWhose;
    // words about a throw (a loaded face noticed) kept until that throw is seen
    protected string m_DTNoteHeld;
    protected float m_DTNoteWait;

    // ---- the table's sounds, played with what the window shows (the canvas's sfx): the
    // place the window is of, found once; a throw heard on the place with no throw played
    // yet (the seconds it waits for one); when the window last sounded a throw (mission
    // time, ms); the game over heard on the place (the seconds it may wait for its box);
    // whether the swap being played has sounded
    protected Object m_DTPlace;
    protected string m_DTPlaceOf = "";
    protected float m_DTRollWait;
    protected int m_DTSungAt;
    protected float m_DTOverSong;
    protected bool m_DTSwapSung;

    protected void DTInit()
    {
        if (m_DTDice)
            return;
        m_DTDice = new array<ref OZG_DTDie>();
        m_DTCans = new array<ref OZG_DTCan>();
        m_DTThrowAt = new array<int>();
        m_DTThrowFaces = new array<int>();
        m_DTSwapPlaces = new array<int>();
        m_DTSwapRows = new array<Widget>();
        m_DTSwapDice = new array<ref OZG_DTDie>();
        m_DTPickKeys = new array<string>();
        m_DTPickShown = new array<string>();
        m_DTPickRows = new array<Widget>();
        m_DTPickDice = new array<ref OZG_DTDie>();
        m_DTQueueV = new array<ref OZG_WView>();
        m_DTQueuePub = new array<ref OZG_GPublic>();
        m_DTQueueOwn = new array<ref OZG_GPrivate>();
    }

    // The window built anew: what was made in the old one went with it, a throw being
    // played is forgotten.
    protected void DTWindowCheck()
    {
        DTInit();
        if (m_DTWin == m_Win)
            return;
        m_DTWin = m_Win;
        m_DTDice.Clear();
        m_DTCans.Clear();
        m_DTSwapRows.Clear();
        m_DTSwapDice.Clear();
        m_DTPickRows.Clear();
        m_DTPickDice.Clear();
        m_DTPickKeys.Clear();
        m_DTPickShown.Clear();
        m_DTPickOpen = false;
        m_DTPickCan = "";
        m_DTFx = "";
        m_DTFxLeft = 0;
        m_DTThrower = -1;
        m_DTV = null;
        m_DTPub = null;
        m_DTOwn = null;
        m_DTNextV = null;
        m_DTNextPub = null;
        m_DTNextOwn = null;
        DTQueueClear();
        m_DTHostAt = -1;
        m_DTHostWant = -1;
        m_DTOverWait = 0;
        m_DTThrowsSeen = -1;
        m_DTDealSeen = "";
        m_DTSwapT = -1;
        m_DTToastLeft = 0;
        m_DTSwapOpen = false;
        m_DTSwapPick = "";
        m_DTSwapTarget = -1;
        m_DTRollWait = 0;
        m_DTOverSong = 0;
        m_DTSungAt = 0;
        // the dice window's tray lies at 52, 86 of the window; nardy's dice fly over all of it
        m_DTOX = 52;
        m_DTOY = 86;
        if (m_Style == "board_nardy")
        {
            m_DTOX = 0;
            m_DTOY = 0;
        }
    }

    protected Widget DTParent()
    {
        return BW("OZG_DTable");
    }

    // ------------------------------------------------------------ the curves

    // cubic-bezier(x1, y1, x2, y2) at x: the curve's t found by Newton's steps.
    protected float DTBezier(float x1, float y1, float x2, float y2, float x)
    {
        if (x <= 0)
            return 0;
        if (x >= 1)
            return 1;
        float t = x;
        for (int i = 0; i < 8; i++)
        {
            float u = 1 - t;
            float bx = 3 * u * u * t * x1 + 3 * u * t * t * x2 + t * t * t - x;
            float dx = 3 * u * u * x1 + 6 * u * t * (x2 - x1) + 3 * t * t * (1 - x2);
            if (dx < 0.0001 && dx > -0.0001)
                break;
            t = t - bx / dx;
            if (t < 0)
                t = 0;
            if (t > 1)
                t = 1;
        }
        float v = 1 - t;
        return 3 * v * v * t * y1 + 3 * v * t * t * y2 + t * t * t;
    }

    // The canvas's curves: a die's way and turn; a can's; ease, ease-out, ease-in-out.
    protected float DTEaseDie(float x)
    {
        return DTBezier(0.2, 0.8, 0.3, 1, x);
    }

    protected float DTEaseCan(float x)
    {
        return DTBezier(0.3, 0.7, 0.3, 1, x);
    }

    protected float DTEase(float x)
    {
        return DTBezier(0.25, 0.1, 0.25, 1, x);
    }

    protected float DTEaseOut(float x)
    {
        return DTBezier(0, 0, 0.58, 1, x);
    }

    protected float DTEaseInOut(float x)
    {
        return DTBezier(0.42, 0, 0.58, 1, x);
    }

    protected float DTMix(float a, float b, float e)
    {
        return a + (b - a) * e;
    }

    // The canvas's keyframes ozgTumble (its part "Кістка") at `p` (0 .. 1): lifted 22 and
    // turned -200 degrees at 0.9 of its size; at 55% down 4, 12 degrees, 1.04; at 78% up
    // 2, -4 degrees; then at rest. Each stretch eased as CSS eases between two keyframes.
    protected void DTTumbleAt(float p, out float ty, out float rot, out float sc)
    {
        float a = 0.78;
        float b = 1;
        float ya = -2;
        float yb = 0;
        float ra = -4;
        float rb = 0;
        float sa = 1;
        float sb = 1;
        if (p < 0.55)
        {
            a = 0;
            b = 0.55;
            ya = -22;
            yb = 4;
            ra = -200;
            rb = 12;
            sa = 0.9;
            sb = 1.04;
        }
        else if (p < 0.78)
        {
            a = 0.55;
            b = 0.78;
            ya = 4;
            yb = -2;
            ra = 12;
            rb = -4;
            sa = 1.04;
            sb = 1;
        }
        float e = DTEaseDie((p - a) / (b - a));
        ty = ya + (yb - ya) * e;
        rot = ra + (rb - ra) * e;
        sc = sa + (sb - sa) * e;
    }

    // The canvas's keyframes ozgCanShake at `p` (0 .. 1): where the can is and how it is
    // turned; it ends lifted 16, from where it is tipped.
    protected void DTShakeAt(float p, out float tx, out float ty, out float rot)
    {
        float at[9] = {0.0, 0.10, 0.22, 0.34, 0.46, 0.58, 0.70, 0.82, 1.0};
        float xs[9] = {0.0, -5.0, 7.0, -7.0, 6.0, -6.0, 6.0, -4.0, 0.0};
        float ys[9] = {0.0, -14.0, -22.0, -17.0, -24.0, -18.0, -23.0, -19.0, -16.0};
        float rs[9] = {0.0, -13.0, 12.0, -14.0, 13.0, -12.0, 12.0, -9.0, 0.0};
        int k = 0;
        for (int i = 0; i < 8; i++)
        {
            if (p >= at[i])
                k = i;
        }
        float span = at[k + 1] - at[k];
        float e = 1;
        if (span > 0)
            e = DTEaseInOut((p - at[k]) / span);
        tx = xs[k] + (xs[k + 1] - xs[k]) * e;
        ty = ys[k] + (ys[k + 1] - ys[k]) * e;
        rot = rs[k] + (rs[k + 1] - rs[k]) * e;
    }

    // ------------------------------------------------------------ dice

    // A paint begins: nothing is in use until it is asked for again.
    protected void DTBegin()
    {
        DTWindowCheck();
        m_DTWhose = "";
        int i;
        for (i = 0; i < m_DTDice.Count(); i++)
            m_DTDice[i].m_Used = false;
        for (i = 0; i < m_DTCans.Count(); i++)
            m_DTCans[i].m_Used = false;
    }

    // What the paint did not ask for goes: a die of a seat that still lies in sight flies
    // home into its can (the turn passed, a round ended: the canvas gathers the dice into
    // the can before the can goes anywhere), anything else is hidden at once. A die hidden
    // is made anew by the paint that asks for it next: it never flies in from where it was.
    protected void DTEnd()
    {
        for (int i = 0; i < m_DTDice.Count(); i++)
        {
            OZG_DTDie d = m_DTDice[i];
            if (d.m_Used)
                continue;
            bool flies = false;
            int cut = d.m_Id.IndexOf("_");
            if (d.m_Alpha > 0.01 && d.m_Id.IndexOf("s") == 0 && cut > 1)
            {
                int seat = d.m_Id.Substring(1, cut - 1).ToInt();
                float hx;
                float hy;
                if (DTHomeOf(seat, hx, hy))
                {
                    d.m_Used = true;
                    DTDieGo(d, hx, hy, d.m_Nominal * 0.3, 0, 0, 0);
                    flies = true;
                }
            }
            if (flies)
                continue;
            d.m_Alpha = 0;
            d.m_ToAlpha = 0;
            d.m_MoveT = -1;
            d.m_TumbleT = -1;
            d.m_Fresh = true;
        }
        DTApplyAll();
    }

    // The later files' part: where the mouth of the can a seat throws from is now (the
    // window's units); false when the seat has no place on the table shown.
    protected bool DTHomeOf(int seat, out float x, out float y)
    {
        x = 0;
        y = 0;
        return false;
    }

    protected ImageWidget DTPart(Widget root, string name)
    {
        return ImageWidget.Cast(root.FindAnyWidget(name));
    }

    // The die of this id, made when it is first asked for (into `parent`, or the table).
    protected OZG_DTDie DTDieOf(string id, Widget parent = null)
    {
        DTInit();
        for (int i = 0; i < m_DTDice.Count(); i++)
        {
            OZG_DTDie had = m_DTDice[i];
            if (had.m_Id == id)
            {
                had.m_Used = true;
                return had;
            }
        }
        OZG_DTDie d = DTDieMake(id, parent);
        if (d)
            m_DTDice.Insert(d);
        return d;
    }

    protected OZG_DTDie DTDieMake(string id, Widget parent)
    {
        if (!parent)
            parent = DTParent();
        if (!parent)
            return null;
        Widget root = MakeLayout(DT_DIE, parent, "OZG_DDie");
        if (!root)
            return null;
        OZG_DTDie d = new OZG_DTDie();
        d.m_Id = id;
        d.m_Root = root;
        d.m_RingPic = DTPart(root, "OZG_DDieRing");
        d.m_ShadowPic = DTPart(root, "OZG_DDieSh");
        d.m_FacePic = DTPart(root, "OZG_DDiePic");
        d.m_LoadBox = root.FindAnyWidget("OZG_DDieLoad");
        d.m_LoadBg = DTPart(root, "OZG_DDieLoadBg");
        d.m_LoadFacePic = DTPart(root, "OZG_DDieLoadFace");
        d.m_LoadBars = DTPart(root, "OZG_DDieLoadBars");
        d.m_EyeBox = root.FindAnyWidget("OZG_DDieEye");
        d.m_EyeRing = DTPart(root, "OZG_DDieEyeRing");
        d.m_EyeBg = DTPart(root, "OZG_DDieEyeBg");
        d.m_EyeIcon = DTPart(root, "OZG_DDieEyeIcon");
        d.m_Used = true;
        d.m_Fresh = true;
        if (d.m_RingPic)
        {
            d.m_RingPic.LoadImageFile(0, "set:ozg_dice image:white_0");
            d.m_RingPic.SetImage(0);
        }
        if (d.m_ShadowPic)
        {
            d.m_ShadowPic.LoadImageFile(0, "set:ozg_dice image:die_s");
            d.m_ShadowPic.SetImage(0);
        }
        root.Show(false);
        return d;
    }

    // What a die shows: its face (0: none) in its look, drawn `nominal` units at full size;
    // `red`: a one with a red pip (pig's, on the white dice).
    protected void DTDieFace(OZG_DTDie d, int face, string look, float nominal, bool red)
    {
        if (!d)
            return;
        if (look == "")
            look = "white";
        string pic = look + "_" + face.ToString();
        bool white = look == "white" || look == "stalker";
        if (red && face == 1 && white)
            pic = look + "_1r";
        pic = "set:ozg_dice image:" + pic;
        d.m_Nominal = nominal;
        if (pic == d.m_Picture || !d.m_FacePic)
            return;
        d.m_Picture = pic;
        d.m_FacePic.LoadImageFile(0, pic);
        d.m_FacePic.SetImage(0);
    }

    // What lies round it: a ring (its colour, 0 for none), the load this player knows (its
    // face and how heavy, 0 for none), the eye of a die he saw slipped in, lifted (picked).
    protected void DTDieMarks(OZG_DTDie d, int ringCol, int loadFace, int loadStep, bool eye, bool lifted)
    {
        if (!d)
            return;
        d.m_RingCol = ringCol;
        d.m_LoadFace = loadFace;
        d.m_LoadStep = loadStep;
        d.m_EyeOn = eye;
        d.m_Lifted = lifted;
    }

    // Where a die should be: its middle (units of the window), its side as it lies, its
    // turn, its alpha, how far it is lifted. A die just made is there at once; one that
    // was elsewhere goes there.
    protected void DTDieGo(OZG_DTDie d, float cx, float cy, float side, float rot, float alpha, float lift)
    {
        if (!d)
            return;
        float x = cx - m_DTOX;
        float y = cy - m_DTOY;
        DTDieGoIn(d, x, y, side, rot, alpha, lift);
    }

    // The same in the units of the die's own parent (a die in a plate's row).
    protected void DTDieGoIn(OZG_DTDie d, float x, float y, float side, float rot, float alpha, float lift)
    {
        if (!d)
            return;
        if (d.m_Fresh)
        {
            d.m_Fresh = false;
            d.m_X = x;
            d.m_Y = y;
            d.m_Side = side;
            d.m_Rot = rot;
            d.m_Alpha = alpha;
            d.m_Lift = lift;
            d.m_ToX = x;
            d.m_ToY = y;
            d.m_ToSide = side;
            d.m_ToRot = rot;
            d.m_ToAlpha = alpha;
            d.m_ToLift = lift;
            d.m_MoveT = -1;
            return;
        }
        float moved = Math.AbsFloat(d.m_ToX - x) + Math.AbsFloat(d.m_ToY - y) + Math.AbsFloat(d.m_ToSide - side);
        float turned = Math.AbsFloat(d.m_ToRot - rot) + Math.AbsFloat(d.m_ToAlpha - alpha) + Math.AbsFloat(d.m_ToLift - lift);
        if (moved + turned < 0.01)
            return;
        d.m_FromX = d.m_X;
        d.m_FromY = d.m_Y;
        d.m_FromSide = d.m_Side;
        d.m_FromRot = d.m_Rot;
        d.m_FromAlpha = d.m_Alpha;
        d.m_FromLift = d.m_Lift;
        d.m_ToX = x;
        d.m_ToY = y;
        d.m_ToSide = side;
        d.m_ToRot = rot;
        d.m_ToAlpha = alpha;
        d.m_ToLift = lift;
        d.m_MoveT = 0;
    }

    // A die put where it should be with no way to go (a row laid anew).
    protected void DTDieSnap(OZG_DTDie d)
    {
        if (!d)
            return;
        d.m_X = d.m_ToX;
        d.m_Y = d.m_ToY;
        d.m_Side = d.m_ToSide;
        d.m_Rot = d.m_ToRot;
        d.m_Alpha = d.m_ToAlpha;
        d.m_Lift = d.m_ToLift;
        d.m_MoveT = -1;
    }

    // A die that stays as the last paint had it (dice that still lie while a throw is seen).
    protected void DTDieKeep(string id)
    {
        if (!m_DTDice)
            return;
        for (int i = 0; i < m_DTDice.Count(); i++)
        {
            if (m_DTDice[i].m_Id == id)
                m_DTDice[i].m_Used = true;
        }
    }

    protected OZG_DTDie DTDieFind(string id)
    {
        if (!m_DTDice)
            return null;
        for (int i = 0; i < m_DTDice.Count(); i++)
        {
            if (m_DTDice[i].m_Id == id)
                return m_DTDice[i];
        }
        return null;
    }

    // The die's id on a table: a seat's die by its place among the seat's dice.
    protected string DTDieId(int seat, int place)
    {
        return "s" + seat.ToString() + "_" + place.ToString();
    }

    protected void DTDieTick(OZG_DTDie d, float dt)
    {
        if (d.m_TumbleT >= 0)
        {
            d.m_TumbleT = d.m_TumbleT + dt;
            if (d.m_TumbleT >= DT_TUMBLE)
                d.m_TumbleT = -1;
        }
        if (d.m_MoveT < 0)
            return;
        d.m_MoveT = d.m_MoveT + dt;
        float pm = 1;
        float pt = 1;
        float pf = 1;
        if (d.m_MoveLen > 0)
            pm = d.m_MoveT / d.m_MoveLen;
        if (d.m_TurnLen > 0)
            pt = d.m_MoveT / d.m_TurnLen;
        if (d.m_FadeLen > 0)
            pf = d.m_MoveT / d.m_FadeLen;
        float em = DTEaseDie(pm);
        float et = DTEaseDie(pt);
        float ef = DTEase(pf);
        d.m_X = DTMix(d.m_FromX, d.m_ToX, em);
        d.m_Y = DTMix(d.m_FromY, d.m_ToY, em);
        d.m_Side = DTMix(d.m_FromSide, d.m_ToSide, et);
        d.m_Rot = DTMix(d.m_FromRot, d.m_ToRot, et);
        d.m_Lift = DTMix(d.m_FromLift, d.m_ToLift, et);
        d.m_Alpha = DTMix(d.m_FromAlpha, d.m_ToAlpha, ef);
        if (pm >= 1 && pt >= 1 && pf >= 1)
            DTDieSnap(d);
    }

    // A die where the clock has it: its frame twice its side about its middle, its parts
    // laid in the frame.
    protected void DTDieApply(OZG_DTDie d)
    {
        if (!d.m_Root)
            return;
        bool seen = d.m_Used && d.m_Alpha > 0.01;
        d.m_Root.Show(seen);
        if (!seen)
            return;
        float ty = 0;
        float trot = 0;
        float tsc = 1;
        if (d.m_TumbleT >= 0)
            DTTumbleAt(d.m_TumbleT / DT_TUMBLE, ty, trot, tsc);
        float k = 1;
        if (d.m_Nominal > 0)
            k = d.m_Side / d.m_Nominal;
        float side = d.m_Side * tsc;
        float cx = d.m_X;
        float cy = d.m_Y + d.m_Lift + ty * k;
        d.m_Root.SetPos((cx - side) * m_K, (cy - side) * m_K);
        d.m_Root.SetSize(2 * side * m_K, 2 * side * m_K);
        d.m_Root.SetRotation(0, 0, d.m_Rot + trot);
        d.m_Root.SetAlpha(d.m_Alpha);
        DTDieLay(d, side);
    }

    protected void DTPartPut(Widget w, float x, float y, float width, float height)
    {
        if (!w)
            return;
        w.SetPos(x, y);
        w.SetSize(width, height);
    }

    // A die's parts in its frame, in pixels: the die in the middle, its shadow under it,
    // its ring round it; over its right corner the load this player knows (the canvas: a
    // grey pill with the loaded face as a tiny die and one to three bars -- 38 x 24 on a
    // die of 74, 30 x 18 on one under 60, the face alone on one under 36), over its left an
    // orange eye. Laid again only when the die's size or its marks changed.
    protected void DTDieLay(OZG_DTDie d, float side)
    {
        float s = side * m_K;
        string marks = d.m_RingCol.ToString() + "/" + d.m_LoadFace.ToString() + "/" + d.m_LoadStep.ToString();
        if (d.m_EyeOn)
            marks = marks + "e";
        if (d.m_Lifted)
            marks = marks + "l";
        if (Math.AbsFloat(s - d.m_LaidSide) < 0.05 && marks == d.m_LaidMarks)
            return;
        d.m_LaidSide = s;
        d.m_LaidMarks = marks;
        // a unit of the die at this size, in pixels
        float u = s / d.m_Nominal;
        float o = s / 2;
        DTPartPut(d.m_FacePic, o, o, s, s);
        float drop = 0.08 * s;
        if (d.m_Lifted)
            drop = 14 * u;
        DTPartPut(d.m_ShadowPic, o, o + drop, s, s);
        if (d.m_RingPic)
        {
            d.m_RingPic.Show(d.m_RingCol != 0);
            if (d.m_RingCol != 0)
            {
                float rw = Math.Round(d.m_Nominal * 0.055);
                if (rw < 3)
                    rw = 3;
                rw = rw * u;
                DTPartPut(d.m_RingPic, o - rw, o - rw, s + 2 * rw, s + 2 * rw);
                d.m_RingPic.SetColor(d.m_RingCol);
            }
        }
        bool small = d.m_Nominal < 36;
        bool mid = d.m_Nominal < 60;
        float bh = 24;
        float bout = 10;
        float mini = 16;
        float pad = 4;
        float gap = 3;
        float barsW = 11;
        float barsH = 14;
        if (mid)
        {
            bh = 18;
            bout = 7;
            mini = 12;
            gap = 2;
            barsW = 8;
            barsH = 10;
        }
        if (small)
        {
            bh = 14;
            bout = 5;
            mini = 10;
            pad = 2;
        }
        bool loaded = d.m_LoadFace > 0;
        if (d.m_LoadBox)
            d.m_LoadBox.Show(loaded);
        if (loaded && d.m_LoadBox)
        {
            bool bars = d.m_LoadStep > 0 && !small;
            float bw = pad + mini + pad;
            string bg = "ozg_dbadge_ln_ca.paa";
            if (mid)
                bg = "ozg_dbadge_mn_ca.paa";
            if (small)
                bg = "ozg_dbadge_s_ca.paa";
            if (bars)
            {
                bw = pad + mini + gap + barsW + pad;
                bg = "ozg_dbadge_l_ca.paa";
                if (mid)
                    bg = "ozg_dbadge_m_ca.paa";
            }
            // its right edge `bout` past the die's, its top as far above it
            float bx = o + s + (bout - bw) * u;
            float by = o - bout * u;
            DTPartPut(d.m_LoadBox, bx, by, bw * u, bh * u);
            if (d.m_LoadBg)
            {
                d.m_LoadBg.LoadImageFile(0, BW_TEX + bg);
                d.m_LoadBg.SetImage(0);
                DTPartPut(d.m_LoadBg, -2 * u, -2 * u, (bw + 4) * u, (bh + 4) * u);
            }
            if (d.m_LoadFacePic)
            {
                d.m_LoadFacePic.LoadImageFile(0, BW_TEX + "ozg_mini_" + d.m_LoadFace.ToString() + "_ca.paa");
                d.m_LoadFacePic.SetImage(0);
                DTPartPut(d.m_LoadFacePic, pad * u, (bh - mini) / 2 * u, mini * u, mini * u);
            }
            if (d.m_LoadBars)
            {
                d.m_LoadBars.Show(bars);
                if (bars)
                {
                    d.m_LoadBars.LoadImageFile(0, BW_TEX + "ozg_dbars_" + d.m_LoadStep.ToString() + "_ca.paa");
                    d.m_LoadBars.SetImage(0);
                    DTPartPut(d.m_LoadBars, (pad + mini + gap) * u, (bh - barsH) / 2 * u, barsW * u, barsH * u);
                }
            }
        }
        if (d.m_EyeBox)
            d.m_EyeBox.Show(d.m_EyeOn);
        if (d.m_EyeOn && d.m_EyeBox)
        {
            float es = 24;
            float ei = 16;
            if (mid)
            {
                es = 18;
                ei = 12;
            }
            if (small)
            {
                es = 14;
                ei = 10;
            }
            DTPartPut(d.m_EyeBox, o - bout * u, o - bout * u, es * u, es * u);
            DTPartPut(d.m_EyeRing, -2 * u, -2 * u, (es + 4) * u, (es + 4) * u);
            DTPartPut(d.m_EyeBg, 0, 0, es * u, es * u);
            DTPartPut(d.m_EyeIcon, (es - ei) / 2 * u, (es - ei) / 2 * u, ei * u, ei * u);
        }
    }

    // ------------------------------------------------------------ cans

    protected OZG_DTCan DTCanOf(string id)
    {
        DTInit();
        for (int i = 0; i < m_DTCans.Count(); i++)
        {
            OZG_DTCan had = m_DTCans[i];
            if (had.m_Id == id)
            {
                had.m_Used = true;
                return had;
            }
        }
        Widget parent = DTParent();
        if (!parent)
            return null;
        Widget root = MakeLayout(DT_CAN, parent, "OZG_DCan");
        if (!root)
            return null;
        OZG_DTCan c = new OZG_DTCan();
        c.m_Id = id;
        c.m_Root = root;
        c.m_TurnBox = root.FindAnyWidget("OZG_DCanTurn");
        c.m_Body = DTPart(root, "OZG_DCanBody");
        c.m_Glow = DTPart(root, "OZG_DCanGlow");
        c.m_Pulse = DTPart(root, "OZG_DCanPulse");
        c.m_Shadow = DTPart(root, "OZG_DCanShadow");
        for (int k = 0; k < 7; k++)
            c.m_Inside.Insert(DTPart(root, "OZG_DCanIn" + k.ToString()));
        c.m_Used = true;
        c.m_Fresh = true;
        if (c.m_Pulse)
            c.m_Pulse.Show(false);
        root.Show(false);
        m_DTCans.Insert(c);
        return c;
    }

    // The colour of a die lying in a can's mouth, by its look.
    protected int DTLookColour(string look)
    {
        if (look.IndexOf("black") >= 0)
            return ARGB(255, 35, 32, 29);
        if (look.IndexOf("red") >= 0)
            return ARGB(255, 163, 32, 27);
        return ARGB(255, 242, 234, 215);
    }

    // The turn of a can in a state: moved, turned, its alpha and its shadow's.
    protected void DTCanTurnOf(string state, float tip, float k, out float tx, out float ty, out float rot, out float al, out float sh)
    {
        tx = 0;
        ty = 0;
        rot = 0;
        al = 1;
        sh = 1;
        if (state == "tip")
        {
            tx = 16 * k;
            if (tip < 0)
                tx = -16 * k;
            ty = -20 * k;
            rot = tip;
            sh = 0.4;
        }
        else if (state == "shake")
        {
            ty = -16;
            sh = 0.5;
        }
        else if (state == "peek")
        {
            ty = -16 * k;
            rot = -16;
            sh = 0.5;
        }
        else if (state == "up")
        {
            ty = -58 * k;
            al = 0.4;
            sh = 0.15;
        }
    }

    // A can as it should be: its box's corner (units of the window), how big, its state
    // (full, empty, shake, tip, down, peek, up), how far it tips, the tin or the army's
    // (`army`), the looks of the dice lying in it, the ring of whose throw it is.
    protected void DTCanSet(OZG_DTCan c, float wx, float wy, float k, string state, float tip, bool army, array<string> looks, bool glow)
    {
        if (!c)
            return;
        float x = wx - m_DTOX;
        float y = wy - m_DTOY;
        c.m_Scale = k;
        c.m_TipDeg = tip;
        c.m_GlowOn = glow;
        if (c.m_Fresh)
        {
            c.m_X = x;
            c.m_Y = y;
            c.m_ToX = x;
            c.m_ToY = y;
            c.m_MoveT = -1;
        }
        else if (Math.AbsFloat(c.m_ToX - x) + Math.AbsFloat(c.m_ToY - y) > 0.01)
        {
            c.m_FromX = c.m_X;
            c.m_FromY = c.m_Y;
            c.m_ToX = x;
            c.m_ToY = y;
            c.m_MoveT = 0;
        }
        float tx;
        float ty;
        float rot;
        float al;
        float sh;
        DTCanTurnOf(state, tip, k, tx, ty, rot, al, sh);
        if (c.m_Fresh)
        {
            c.m_State = state;
            c.m_Tx = tx;
            c.m_Ty = ty;
            c.m_Rot = rot;
            c.m_Al = al;
            c.m_Sh = sh;
            c.m_TurnT = -1;
            c.m_ShakeT = -1;
        }
        else if (state != c.m_State || Math.AbsFloat(c.m_ToRot - rot) > 0.01)
        {
            c.m_State = state;
            c.m_FromTx = c.m_Tx;
            c.m_FromTy = c.m_Ty;
            c.m_FromRot = c.m_Rot;
            c.m_FromAl = c.m_Al;
            c.m_FromSh = c.m_Sh;
            c.m_TurnT = 0;
            c.m_ShakeT = -1;
            // the shake is its own keyframes from where the can stands
            if (state == "shake")
            {
                c.m_ShakeT = 0;
                c.m_TurnT = -1;
            }
        }
        c.m_ToTx = tx;
        c.m_ToTy = ty;
        c.m_ToRot = rot;
        c.m_ToAl = al;
        c.m_ToSh = sh;
        c.m_Fresh = false;
        // what it is: the tin or the army's can, open or turned over; the dice in it
        bool closed = state == "down" || state == "peek" || state == "up";
        string tex = "ozg_can_tin_open_ca.paa";
        if (army && closed)
            tex = "ozg_can_army_closed_ca.paa";
        else if (army)
            tex = "ozg_can_army_open_ca.paa";
        else if (closed)
            tex = "ozg_can_tin_closed_ca.paa";
        if (tex != c.m_BodyTex && c.m_Body)
        {
            c.m_BodyTex = tex;
            c.m_Body.LoadImageFile(0, BW_TEX + tex);
            c.m_Body.SetImage(0);
        }
        int n = 0;
        bool shows = state == "full" || state == "shake";
        if (shows && looks)
            n = looks.Count();
        if (n > 7)
            n = 7;
        c.m_InsideN = n;
        for (int i = 0; i < c.m_Inside.Count(); i++)
        {
            ImageWidget lying = c.m_Inside[i];
            if (!lying)
                continue;
            lying.Show(i < n);
            if (i < n)
            {
                string look = looks[i];
                lying.SetColor(DTLookColour(look));
            }
        }
    }

    // A swap seen: the can's foot pulses orange, twice.
    protected void DTCanPulse(OZG_DTCan c)
    {
        if (c)
            c.m_PulseT = 0;
    }

    protected void DTCanTick(OZG_DTCan c, float dt)
    {
        if (c.m_MoveT >= 0)
        {
            c.m_MoveT = c.m_MoveT + dt;
            float pm = c.m_MoveT / DT_CAN_MOVE;
            float em = DTEaseCan(pm);
            c.m_X = DTMix(c.m_FromX, c.m_ToX, em);
            c.m_Y = DTMix(c.m_FromY, c.m_ToY, em);
            if (pm >= 1)
            {
                c.m_X = c.m_ToX;
                c.m_Y = c.m_ToY;
                c.m_MoveT = -1;
            }
        }
        if (c.m_ShakeT >= 0)
        {
            c.m_ShakeT = c.m_ShakeT + dt;
            float ps = c.m_ShakeT / DT_CAN_SHAKE;
            if (ps > 1)
                ps = 1;
            float sx;
            float sy;
            float sr;
            DTShakeAt(ps, sx, sy, sr);
            c.m_Tx = sx;
            c.m_Ty = sy;
            c.m_Rot = sr;
            c.m_Al = 1;
            c.m_Sh = 0.5;
            if (ps >= 1)
                c.m_ShakeT = -1;
        }
        else if (c.m_TurnT >= 0)
        {
            c.m_TurnT = c.m_TurnT + dt;
            float pt = c.m_TurnT / DT_CAN_TURN;
            float et = DTEaseCan(pt);
            c.m_Tx = DTMix(c.m_FromTx, c.m_ToTx, et);
            c.m_Ty = DTMix(c.m_FromTy, c.m_ToTy, et);
            c.m_Rot = DTMix(c.m_FromRot, c.m_ToRot, et);
            c.m_Al = DTMix(c.m_FromAl, c.m_ToAl, et);
            c.m_Sh = DTMix(c.m_FromSh, c.m_ToSh, et);
            if (pt >= 1)
            {
                c.m_Tx = c.m_ToTx;
                c.m_Ty = c.m_ToTy;
                c.m_Rot = c.m_ToRot;
                c.m_Al = c.m_ToAl;
                c.m_Sh = c.m_ToSh;
                c.m_TurnT = -1;
            }
        }
        if (c.m_PulseT >= 0)
        {
            c.m_PulseT = c.m_PulseT + dt;
            if (c.m_PulseT >= 2 * DT_CAN_PULSE)
                c.m_PulseT = -1;
        }
    }

    // A can where the clock has it. Its picture is 92 units square with the can's box (76 x
    // 92) 8 in; it turns about 50%, 62% of its box (the canvas's transform-origin) in a
    // frame of 150 units whose middle is that point; the dice in its mouth are pictures of
    // the part of the box at 6, 0, 64 x 32. On the felt under it: the glow of whose throw
    // it is (its picture the box's -18, 50, 112 x 62), its shadow (-8, 64, 96 x 40), the
    // pulse of a swap seen about the middle of its foot (38, 81).
    protected void DTCanApply(OZG_DTCan c)
    {
        if (!c.m_Root)
            return;
        c.m_Root.Show(c.m_Used);
        if (!c.m_Used)
            return;
        float k = c.m_Scale * m_K;
        c.m_Root.SetPos(c.m_X * m_K, c.m_Y * m_K);
        c.m_Root.SetSize(76 * k, 92 * k);
        if (c.m_TurnBox)
        {
            float side = 150 * k;
            float px = 38 * k + c.m_Tx * m_K - side / 2;
            float py = 57.04 * k + c.m_Ty * m_K - side / 2;
            c.m_TurnBox.SetPos(px, py);
            c.m_TurnBox.SetSize(side, side);
            c.m_TurnBox.SetRotation(0, 0, c.m_Rot);
            c.m_TurnBox.SetAlpha(c.m_Al);
        }
        DTPartPut(c.m_Body, 29 * k, 17.96 * k, 92 * k, 92 * k);
        for (int i = 0; i < c.m_Inside.Count(); i++)
            DTPartPut(c.m_Inside[i], 43 * k, 17.96 * k, 64 * k, 32 * k);
        if (c.m_Glow)
        {
            c.m_Glow.Show(c.m_GlowOn);
            // the canvas's ring is two units and its glow fourteen at any size of the can:
            // a picture for each size the canvas draws a can at, one more for the smaller
            string glowTex = "ozg_can_glow_ca.paa";
            if (c.m_Scale >= 1.4)
                glowTex = "ozg_can_glow15_ca.paa";
            else if (c.m_Scale >= 1.2)
                glowTex = "ozg_can_glow125_ca.paa";
            if (c.m_GlowTex != glowTex)
            {
                c.m_GlowTex = glowTex;
                c.m_Glow.LoadImageFile(0, BW_TEX + glowTex);
            }
            DTPartPut(c.m_Glow, -18 * k, 50 * k, 112 * k, 62 * k);
        }
        if (c.m_Shadow)
        {
            DTPartPut(c.m_Shadow, -8 * k, 64 * k, 96 * k, 40 * k);
            c.m_Shadow.SetAlpha(c.m_Sh);
        }
        if (c.m_Pulse)
        {
            bool pulses = c.m_PulseT >= 0;
            c.m_Pulse.Show(pulses);
            if (pulses)
            {
                float p = c.m_PulseT / DT_CAN_PULSE;
                if (p >= 1)
                    p = p - 1;
                float e = DTEaseOut(p);
                float sc = 0.8 + 0.7 * e;
                float w = 80 * k * sc;
                float h = 30 * k * sc;
                DTPartPut(c.m_Pulse, 38 * k - w / 2, 81 * k - h / 2, w, h);
                // a light layer in linear light: the canvas's alpha to the power of 1.5
                float fade = 0.9 * (1 - e);
                c.m_Pulse.SetAlpha(Math.Pow(fade, 1.5));
            }
        }
    }

    protected void DTApplyAll()
    {
        if (!m_DTDice)
            return;
        int i;
        for (i = 0; i < m_DTDice.Count(); i++)
            DTDieApply(m_DTDice[i]);
        for (i = 0; i < m_DTCans.Count(); i++)
            DTCanApply(m_DTCans[i]);
    }

    // ------------------------------------------------------------ what the view says of dice

    // A line "<seat>=a,b,c" of the view's lists, split.
    protected void DTLineOf(array<string> lines, int seat, array<string> into)
    {
        into.Clear();
        if (!lines || seat < 0)
            return;
        string head = seat.ToString() + "=";
        int headLen = head.Length();
        for (int i = 0; i < lines.Count(); i++)
        {
            string line = lines[i];
            if (line.IndexOf(head) != 0)
                continue;
            string rest = line.Substring(headLen, line.Length() - headLen);
            OZG_Text.SplitLong(rest, ",", into);
            return;
        }
    }

    // The look of the die at `place` of a seat: as it was thrown last (`thrown`), or as it
    // lies in the can now; the set's when the view says neither.
    protected string DTLookAt(int seat, int place, bool thrown)
    {
        string look = "";
        array<string> parts = new array<string>();
        // between games this player's own dice are the ones he would throw
        if (m_V && !thrown && seat == m_V.Mine && m_V.St != OZG_Const.ST_PLAYING)
        {
            look = DTPickLookAt(place);
            if (look != "")
                return look;
        }
        if (m_V && thrown)
        {
            DTLineOf(m_V.ThrownDiceLooks, seat, parts);
            if (place >= 0 && place < parts.Count())
                look = parts[place];
        }
        if (m_V && look == "")
        {
            DTLineOf(m_V.SeatDiceLooks, seat, parts);
            if (place >= 0 && place < parts.Count())
                look = parts[place];
        }
        if (look == "")
            look = DTSetLook();
        return look;
    }

    // The look of the set's dice (the can laid out): white unless the view says another.
    protected string DTSetLook()
    {
        string look = BWLookOf("Di");
        if (look == "classic" || look == "")
            look = "white";
        return look;
    }

    // What this player knows of the die at `place` of a seat (OZG_WView.SeatDiceMarks):
    // the face a load he knows is on times ten plus how heavy, 100 more for a die he saw
    // slipped in.
    protected int DTMarkAt(int seat, int place, bool thrown)
    {
        if (!m_V)
            return 0;
        array<string> parts = new array<string>();
        if (thrown)
            DTLineOf(m_V.ThrownDiceMarks, seat, parts);
        else
            DTLineOf(m_V.SeatDiceMarks, seat, parts);
        if (place < 0 || place >= parts.Count())
            return 0;
        return parts[place].ToInt();
    }

    // A die's marks from that number.
    protected void DTDieMarked(OZG_DTDie d, int mark, int ringCol, bool lifted)
    {
        bool eye = mark >= 100;
        int load = mark % 100;
        DTDieMarks(d, ringCol, load / 10, load % 10, eye, lifted);
    }

    // Does the seat throw its own dice (the rest throw the table's)? In a game: as the deal
    // found the dice. Between games nobody's are taken yet, and the cans stand by the
    // variant chosen: everybody's own, the table's one, or -- by wish -- as the last deal
    // had them.
    protected bool DTOwnCan(int seat)
    {
        if (!m_V)
            return false;
        if (m_V.St != OZG_Const.ST_PLAYING)
        {
            if (m_DTWhose == "")
                m_DTWhose = DTWhose();
            if (m_DTWhose == OZG_DiceConst.WHOSE_OWN)
                return true;
            if (m_DTWhose == OZG_DiceConst.WHOSE_HOST)
                return false;
            // by wish this player's own seat stands as his choice has it
            if (seat == m_V.Mine)
            {
                int picks = DTPickOwnNow();
                if (picks >= 0)
                    return picks == 1;
            }
        }
        return m_V.OwnDiceSeats.Find(seat) >= 0;
    }

    // Between games: would this player throw from a can of his own, as the view's pick
    // lists have it (the can he chose, or the one a deal would take; nobody else's choice
    // is told to him)? 1 yes, 0 the one laid out, -1 the view does not say.
    protected int DTPickOwnNow()
    {
        if (!m_V || m_V.Mine < 0 || m_V.PickNeed <= 0 || m_V.PickCanNow != 1)
            return -1;
        int inPlay = m_V.PickCanOn.Find(1);
        if (inPlay < 0 || inPlay >= m_V.PickCanKeys.Count())
            return -1;
        string key = m_V.PickCanKeys[inPlay];
        if (key == OZG_DiceConst.PICK_HOST)
            return 0;
        return 1;
    }

    // Between games: the look of the die at `place` among the dice this player would throw
    // (the pick lists); "" when the view does not say.
    protected string DTPickLookAt(int place)
    {
        if (!m_V || m_V.PickCanNow != 1 || place < 0)
            return "";
        int n = 0;
        for (int i = 0; i < m_V.PickDieOn.Count() && i < m_V.PickDieLooks.Count(); i++)
        {
            if (m_V.PickDieOn[i] != 1)
                continue;
            if (n == place)
                return m_V.PickDieLooks[i];
            n++;
        }
        return "";
    }

    // Whose dice the variant chosen says: OZG_DiceConst.WHOSE_*.
    protected string DTWhose()
    {
        string chosen = OZG_DiceConst.WHOSE_HOST;
        OZG_Game sample = OZG_Games.Sample(m_V.GameId);
        if (sample)
        {
            array<ref OZG_Option> opts = new array<ref OZG_Option>();
            sample.OptionList(opts);
            for (int i = 0; i < opts.Count(); i++)
            {
                OZG_Option o = opts[i];
                if (o.OptionId == OZG_DiceConst.OPT_WHOSE)
                    chosen = o.DefaultId;
            }
        }
        int at = m_V.OptIds.Find(OZG_DiceConst.OPT_WHOSE);
        if (at >= 0 && at < m_V.OptVals.Count())
            chosen = m_V.OptVals[at];
        return chosen;
    }

    // Is the can a seat throws from the army's (the stalkers' set) or a tin?
    protected bool DTArmy(int seat)
    {
        string look = "";
        // between games this player's own can is the one he would throw from
        if (m_V && seat == m_V.Mine && m_V.St != OZG_Const.ST_PLAYING && DTPickOwnNow() == 1)
        {
            int inPlay = m_V.PickCanOn.Find(1);
            if (inPlay >= 0 && inPlay < m_V.PickCanLooks.Count())
                return m_V.PickCanLooks[inPlay] == "stalker";
        }
        if (m_V)
        {
            array<string> parts = new array<string>();
            DTLineOf(m_V.SeatCanLooks, seat, parts);
            if (parts.Count() > 0)
                look = parts[0];
            if (look == "")
                look = m_V.SetLook;
        }
        return look == "stalker";
    }

    // The can a seat throws from: its own, or the table's one.
    protected string DTCanId(int seat)
    {
        if (DTOwnCan(seat))
            return "c" + seat.ToString();
        return "host";
    }

    // ------------------------------------------------------------ a throw played

    // Is a throw being played (nothing may be asked of the game meanwhile)?
    protected bool DTBusy()
    {
        if (m_DTQueueV && m_DTQueueV.Count() > 0)
            return true;
        return m_DTFx != "" || m_DTSwapT >= 0;
    }

    // While the dice a seat is about to throw are gathered and shaken: is this die of them?
    protected bool DTHeld(int seat, int place)
    {
        if (m_DTFx != "gather" && m_DTFx != "shake" && m_DTFx != "tip")
            return false;
        return seat == m_DTThrower && m_DTThrowAt.Find(place) >= 0;
    }

    // While a throw is seen flying and lying: the face of this die of it, 0 for a die that
    // is not of it.
    protected int DTFlown(int seat, int place)
    {
        if (m_DTFx != "fly" && m_DTFx != "linger")
            return 0;
        if (seat != m_DTThrower)
            return 0;
        int at = m_DTThrowAt.Find(place);
        if (at < 0 || at >= m_DTThrowFaces.Count())
            return 0;
        return m_DTThrowFaces[at];
    }

    // The can's state at a seat while a throw is played: "" when the seat is not throwing.
    protected string DTCanFx(int seat)
    {
        if (seat != m_DTThrower)
            return "";
        if (m_DTFx == "shake")
            return "shake";
        if (m_DTFx == "tip" || m_DTFx == "fly")
            return "tip";
        return "";
    }

    // The dice a view says a seat threw last: their places among the seat's dice and their
    // faces (its plate's "last throw").
    protected void DTThrowOf(OZG_GPublic pub, int seat, array<int> places, array<int> faces)
    {
        places.Clear();
        faces.Clear();
        if (!pub)
            return;
        for (int s = 0; s < pub.SeatList.Count(); s++)
        {
            OZG_GSeat g = pub.SeatList[s];
            if (!g || g.SeatNo != seat)
                continue;
            for (int f = 0; f < g.Faces.Count(); f++)
            {
                string faceText = g.Faces[f];
                faces.Insert(faceText.ToInt());
                int place = f;
                if (g.FacePlaces && f < g.FacePlaces.Count())
                    place = g.FacePlaces[f];
                places.Insert(place);
            }
        }
    }

    // How long a throw's steps take at this game (the canvas's timers), seconds: the dice
    // gathered (when any lie), the can's way to the seat (when the table's can stands
    // elsewhere), the shake, the tip, the flight, and how long dice lie that the view has
    // no more on the table.
    protected void DTFxTimes(out float gather, out float travel, out float shake, out float tip, out float fly, out float linger)
    {
        gather = 0.32;
        travel = 0.62;
        shake = 0.66;
        tip = 0.15;
        fly = 0.54;
        linger = 1.3;
        if (!m_V)
            return;
        if (m_V.GameId == OZG_PigConst.GAME_ID)
        {
            gather = 0.28;
            shake = 0.48;
            tip = 0.13;
            fly = 0.52;
            linger = 1.1;
        }
        else if (m_V.GameId == OZG_HighRollConst.GAME_ID || m_V.GameId == OZG_DPokerConst.GAME_ID)
        {
            shake = 0.6;
            tip = 0.13;
            fly = 0.52;
            linger = 1.2;
        }
        else if (m_Style == "board_nardy")
        {
            shake = 0.62;
            tip = 0.13;
            fly = 0.54;
            linger = 1.2;
        }
    }

    // The later files' parts: does any die of the seat lie outside its can in the view
    // shown; does the view shown have the dice of the last throw on the table.
    protected bool DTAnyOut(int seat)
    {
        return false;
    }

    protected bool DTThrowLies()
    {
        return true;
    }

    // The dice window takes a view (called when it paints): the one it shows, or a new one.
    // A new one that tells of a throw of the game shown waits; the throw is played first.
    protected void DTAdopt()
    {
        DTWindowCheck();
        if (m_V == m_DTV && m_GPub == m_DTPub && m_GOwn == m_DTOwn)
            return;
        OZG_WView newV = m_V;
        OZG_GPublic newPub = m_GPub;
        OZG_GPrivate newOwn = m_GOwn;
        // the view whose throw is being played, sent again: the newer copy waits in its place
        if (m_DTNextV && newV && newV.Rev == m_DTNextV.Rev)
        {
            m_DTNextV = newV;
            m_DTNextPub = newPub;
            m_DTNextOwn = newOwn;
            m_V = m_DTV;
            m_GPub = m_DTPub;
            m_GOwn = m_DTOwn;
            return;
        }
        if (m_DTFx != "")
        {
            // a view of the same game while a throw is played, its dice fly or lie a
            // moment: it waits its turn, the window showing what it showed
            if (DTQueues(newV, newPub))
            {
                m_DTQueueV.Insert(newV);
                m_DTQueuePub.Insert(newPub);
                m_DTQueueOwn.Insert(newOwn);
                m_V = m_DTV;
                m_GPub = m_DTPub;
                m_GOwn = m_DTOwn;
                return;
            }
            // anything else (another game, another place, too many waiting): what waited
            // is dropped for the newest and the throw played is cut
            DTQueueClear();
        }
        if (m_DTNextV)
        {
            m_DTV = m_DTNextV;
            m_DTPub = m_DTNextPub;
            m_DTOwn = m_DTNextOwn;
            m_DTNextV = null;
            m_DTNextPub = null;
            m_DTNextOwn = null;
        }
        if (m_DTFx != "")
        {
            m_DTFx = "";
            m_DTFxLeft = 0;
            if (m_DTHostWant >= 0)
                m_DTHostAt = m_DTHostWant;
            m_DTHostWant = -1;
        }
        bool throws = false;
        if (m_DTV && m_DTPub && newV && newPub)
        {
            bool same = m_DTV.PlaceId == newV.PlaceId && m_DTV.GameId == newV.GameId && m_DTV.DealNo == newV.DealNo;
            bool grew = newPub.Throws > m_DTPub.Throws && newPub.ThrowSeat >= 0;
            bool played = m_DTV.St == OZG_Const.ST_PLAYING && m_DTPub.GameId == newPub.GameId;
            if (same && grew && played && DTPlaysThrows())
                throws = true;
        }
        if (!throws)
        {
            m_DTV = newV;
            m_DTPub = newPub;
            m_DTOwn = newOwn;
            return;
        }
        m_DTNextV = newV;
        m_DTNextPub = newPub;
        m_DTNextOwn = newOwn;
        m_V = m_DTV;
        m_GPub = m_DTPub;
        m_GOwn = m_DTOwn;
        m_DTThrower = newPub.ThrowSeat;
        DTThrowOf(newPub, m_DTThrower, m_DTThrowAt, m_DTThrowFaces);
        DTThrowBegin();
    }

    // May this view wait behind the throw being played: one of the same place and game,
    // while no more than three wait already (a window never falls far behind the table).
    protected bool DTQueues(OZG_WView v, OZG_GPublic pub)
    {
        if (!v || !pub || !m_DTV || !m_DTPub)
            return false;
        if (m_DTQueueV.Count() >= 3)
            return false;
        bool same = v.PlaceId == m_DTV.PlaceId && v.GameId == m_DTV.GameId;
        return same && pub.GameId == m_DTPub.GameId;
    }

    protected void DTQueueClear()
    {
        if (!m_DTQueueV)
            return;
        m_DTQueueV.Clear();
        m_DTQueuePub.Clear();
        m_DTQueueOwn.Clear();
    }

    // A throw has been seen to its end: the views that came meanwhile, each painted in its
    // turn (one that tells of a throw starts that throw, and the rest wait on).
    protected void DTQueueNext()
    {
        while (m_DTFx == "" && m_DTQueueV.Count() > 0)
        {
            OZG_WView nextV = m_DTQueueV[0];
            OZG_GPublic nextPub = m_DTQueuePub[0];
            OZG_GPrivate nextOwn = m_DTQueueOwn[0];
            m_V = nextV;
            m_GPub = nextPub;
            m_GOwn = nextOwn;
            m_DTQueueV.RemoveOrdered(0);
            m_DTQueuePub.RemoveOrdered(0);
            m_DTQueueOwn.RemoveOrdered(0);
            BoardWinPaint();
        }
    }

    // Does the game shown play its throws through the view kept back (perudo and nardy
    // play theirs their own way)?
    protected bool DTPlaysThrows()
    {
        if (!m_V)
            return false;
        return m_V.GameId != OZG_PerudoConst.GAME_ID;
    }

    // The first step of a throw: the dice that lie go back into the can, and the table's
    // can comes to the seat if it stands elsewhere.
    protected void DTThrowBegin()
    {
        float gather;
        float travel;
        float shake;
        float tip;
        float fly;
        float linger;
        DTFxTimes(gather, travel, shake, tip, fly, linger);
        bool lies = DTAnyOut(m_DTThrower);
        bool travels = false;
        if (!DTOwnCan(m_DTThrower))
        {
            if (m_DTHostAt >= 0 && m_DTHostAt != m_DTThrower)
                travels = true;
            m_DTHostAt = m_DTThrower;
            m_DTHostWant = -1;
        }
        m_DTSwapOpen = false;
        m_DTPickOpen = false;
        m_DTFx = "gather";
        m_DTFxLeft = 0;
        if (lies)
            m_DTFxLeft = gather;
        if (travels)
            m_DTFxLeft = travel;
        if (m_DTFxLeft <= 0)
        {
            m_DTFx = "shake";
            m_DTFxLeft = shake;
            DTThrowSound(OZG_SoundIds.RATTLE);
        }
    }

    // The next step of the throw played.
    protected void DTFxNext()
    {
        float gather;
        float travel;
        float shake;
        float tip;
        float fly;
        float linger;
        DTFxTimes(gather, travel, shake, tip, fly, linger);
        if (m_DTFx == "gather")
        {
            m_DTFx = "shake";
            m_DTFxLeft = shake;
            DTThrowSound(OZG_SoundIds.RATTLE);
        }
        else if (m_DTFx == "shake")
        {
            m_DTFx = "tip";
            m_DTFxLeft = tip;
        }
        else if (m_DTFx == "tip")
        {
            // the dice fly out: the view that told of the throw is shown from here on
            m_DTFx = "fly";
            m_DTFxLeft = fly;
            if (m_DTNextV)
            {
                m_DTV = m_DTNextV;
                m_DTPub = m_DTNextPub;
                m_DTOwn = m_DTNextOwn;
                m_V = m_DTV;
                m_GPub = m_DTPub;
                m_GOwn = m_DTOwn;
                m_DTNextV = null;
                m_DTNextPub = null;
                m_DTNextOwn = null;
            }
            DTFxPaint();
            DTTumbleThrown();
            DTThrowSound(OZG_SoundIds.LAND);
            return;
        }
        else if (m_DTFx == "fly")
        {
            m_DTFx = "";
            m_DTFxLeft = 0;
            m_DTOverWait = DT_OVER_WAIT;
            if (!DTThrowLies())
            {
                m_DTFx = "linger";
                m_DTFxLeft = linger;
            }
        }
        else if (m_DTFx == "home")
        {
            m_DTFx = "";
            m_DTFxLeft = 0;
            if (m_DTHostWant >= 0)
                m_DTHostAt = m_DTHostWant;
            m_DTHostWant = -1;
        }
        else
        {
            m_DTFx = "";
            m_DTFxLeft = 0;
        }
        DTFxPaint();
        DTQueueNext();
    }

    // The window painted again for the step the throw is at.
    protected void DTFxPaint()
    {
        BoardWinPaint();
    }

    // The dice that have just flown out tumble as they fall.
    protected void DTTumbleThrown()
    {
        for (int i = 0; i < m_DTThrowAt.Count(); i++)
        {
            int place = m_DTThrowAt[i];
            OZG_DTDie d = DTDieFind(DTDieId(m_DTThrower, place));
            if (d)
                d.m_TumbleT = 0;
        }
    }

    // Where the table's one can stands: the seat a paint wants it at, but not before the
    // dice that lie are home (the canvas: the dice go back into the can, then the can goes
    // to the next seat), and at the thrower's while a throw is played.
    protected int DTHostSeat(int want, bool anyOut)
    {
        if (m_DTFx == "gather" || m_DTFx == "shake" || m_DTFx == "tip" || m_DTFx == "fly" || m_DTFx == "linger")
        {
            if (m_DTHostAt < 0)
                m_DTHostAt = want;
            return m_DTHostAt;
        }
        if (m_DTHostAt < 0 || want < 0)
        {
            m_DTHostAt = want;
            return m_DTHostAt;
        }
        if (want == m_DTHostAt)
            return m_DTHostAt;
        if (m_DTFx == "home")
            return m_DTHostAt;
        if (!anyOut)
        {
            m_DTHostAt = want;
            return m_DTHostAt;
        }
        m_DTHostWant = want;
        m_DTFx = "home";
        m_DTFxLeft = DT_HOME;
        return m_DTHostAt;
    }

    // The clock: the throw's steps, the dice, the cans, a die slipped in, the note.
    protected void DTTick(float dt)
    {
        if (!m_DTDice || m_DTWin != m_Win)
            return;
        DTSoundsTick(dt);
        if (m_DTOverWait > 0)
        {
            m_DTOverWait = m_DTOverWait - dt;
            if (m_DTOverWait <= 0)
            {
                m_DTOverWait = 0;
                if (m_DTFx == "")
                    BoardWinPaint();
            }
        }
        if (m_DTFx != "")
        {
            m_DTFxLeft = m_DTFxLeft - dt;
            if (m_DTFxLeft <= 0)
                DTFxNext();
        }
        int i;
        for (i = 0; i < m_DTDice.Count(); i++)
            DTDieTick(m_DTDice[i], dt);
        for (i = 0; i < m_DTCans.Count(); i++)
            DTCanTick(m_DTCans[i], dt);
        if (m_DTSwapT >= 0)
            DTSwapTick(dt);
        if (m_DTSwapOpen)
            DTSwapRowsCull();
        if (m_DTPickOpen && m_V)
            DTRowsCull("OZG_DPkRows", m_DTPickRows, m_V.PickCanKeys.Count(), 3, 72, 66);
        DTApplyAll();
        if (m_DTToastLeft > 0)
        {
            m_DTToastLeft = m_DTToastLeft - dt;
            if (m_DTToastLeft <= 0)
                BWShow("OZG_DToast", false);
        }
        if (m_DTNoteHeld != "")
        {
            if (m_DTNoteWait > 0)
                m_DTNoteWait = m_DTNoteWait - dt;
            bool coming = m_DTFx == "home" || m_DTFx == "gather" || m_DTFx == "shake" || m_DTFx == "tip";
            if (m_DTNoteWait <= 0 && !coming)
            {
                string held = m_DTNoteHeld;
                m_DTNoteHeld = "";
                Said(held);
            }
        }
    }

    // A loaded face noticed is said with the throw that showed it, and the window shows
    // that throw a moment later (the dice gathered, the can shaken and tipped): the words
    // wait for the dice to fly. The notice and the view come in either order, so it waits
    // a third of a second for the view first.
    override void Noticed(string kind, string text)
    {
        bool table = m_Style == "dice" || m_Style == "board_nardy";
        if (kind != OZG_Const.NOTICE_LOADED || !table || !BWUp())
        {
            super.Noticed(kind, text);
            return;
        }
        if (m_DTNoteHeld != "")
            Said(m_DTNoteHeld);
        m_DTNoteHeld = text;
        m_DTNoteWait = 0.35;
    }

    // ------------------------------------------------------------ sounds

    // The place the window is of.
    protected Object DTPlaceThing()
    {
        if (!m_DTPlace || m_DTPlaceOf != m_PlaceId)
        {
            m_DTPlace = OZG_Place.OZG_ById(m_PlaceId);
            m_DTPlaceOf = m_PlaceId;
        }
        return m_DTPlace;
    }

    // A sound of the table played on its place, on this client alone.
    protected void DTSound(int id)
    {
        OZG_Sounds.PlayIdLocal(id, DTPlaceThing(), "window");
    }

    // A step of a throw sounded (the can rattles, the dice land): the place's own sound of
    // that throw is not played on this client, whenever it comes.
    protected void DTThrowSound(int id)
    {
        m_DTSungAt = GetGame().GetTime();
        m_DTRollWait = 0;
        DTSound(id);
    }

    // Is a throw being played or about to be, or was one sounded a moment ago (its steps
    // sound it)? The later files add their own (perudo's cans, its showdown).
    protected bool DTThrowSounding()
    {
        if (m_DTQueueV && m_DTQueueV.Count() > 0)
            return true;
        if (m_DTFx == "gather" || m_DTFx == "shake" || m_DTFx == "tip" || m_DTFx == "fly")
            return true;
        if (m_DTSungAt <= 0)
            return false;
        int since = GetGame().GetTime() - m_DTSungAt;
        return since >= 0 && since < 3000;
    }

    // The window says each frame that it plays this place's throws itself, and takes what
    // the place's lots left it (OZG_Sounds.TakeOn). A throw is sounded by the steps of its
    // play; one no play follows (the window opened in the middle of it) sounds as the
    // place would have sounded it, half a second late. The game over sounds when its box
    // shows: once its last throw has been seen.
    protected void DTSoundsTick(float dt)
    {
        Object place = DTPlaceThing();
        if (!place)
            return;
        OZG_Sounds.TakeOn(place, m_Style == "dice");
        array<int> taken = OZG_Sounds.s_Taken;
        for (int i = 0; i < taken.Count(); i++)
        {
            int id = taken[i];
            if (id == OZG_SoundIds.ROLL && !DTThrowSounding())
                m_DTRollWait = 0.5;
            if (id == OZG_SoundIds.OVER)
                m_DTOverSong = 8.0;
        }
        taken.Clear();
        if (m_DTRollWait > 0)
        {
            if (DTThrowSounding())
            {
                m_DTRollWait = 0;
            }
            else
            {
                m_DTRollWait = m_DTRollWait - dt;
                if (m_DTRollWait <= 0)
                {
                    m_DTRollWait = 0;
                    DTSound(OZG_SoundIds.ROLL);
                }
            }
        }
        if (m_DTOverSong > 0)
        {
            m_DTOverSong = m_DTOverSong - dt;
            if (BWOver())
            {
                m_DTOverSong = 0;
                DTSound(OZG_SoundIds.OVER);
            }
        }
    }

    // ------------------------------------------------------------ seats

    // The plates' row along the felt's top: the corner and the width of plate `i` of `n`.
    // Two (the canvas's tables of three) lie 280 wide at the ends; three (its tables of
    // four) 260 wide; more share the row.
    protected void DTTopPlate(int i, int n, bool wide, out float x, out float w)
    {
        if (n <= 2 && wide)
        {
            w = 280;
            x = 130;
            if (i > 0)
                x = 790;
            return;
        }
        if (n <= 3)
        {
            w = 260;
            x = 120 + i * 350;
            if (n == 2 && i == 1)
                x = 820;
            return;
        }
        float gap = 14;
        if (n == 5)
            gap = 12;
        if (n >= 6)
            gap = 10;
        w = (960 - (n - 1) * gap) / n;
        x = 120 + i * (w + gap);
    }

    // ------------------------------------------------------------ the note

    // A note over the table (the canvas: 356 wide, its ring orange for a swap seen): a
    // caption, the words, smaller words under them; DT_TOAST seconds.
    protected void DTToast(string text, string cap, string sub, bool hot)
    {
        Widget box = BW("OZG_DToast");
        if (!box)
            return;
        float y = 10;
        TextWidget capW = BWText("OZG_DToastCap");
        if (capW)
        {
            capW.Show(cap != "");
            capW.SetText(OZG_Text.Upper(cap));
            capW.SetPos(14 * m_K, y * m_K);
        }
        if (cap != "")
            y = y + 15 + 4;
        int i;
        array<string> lines = new array<string>();
        BWWrap(BWText("OZG_DToastText0"), text, 328, 3, lines);
        for (i = 0; i < 3; i++)
        {
            string tn = "OZG_DToastText" + i.ToString();
            string line = "";
            if (i < lines.Count())
                line = lines[i];
            BWSetText(tn, line);
            BWShow(tn, i < lines.Count());
            BWMove(BW(tn), 14, y + i * 20);
        }
        y = y + lines.Count() * 20;
        array<string> subs = new array<string>();
        if (sub != "")
            BWWrap(BWText("OZG_DToastSub0"), sub, 328, 4, subs);
        if (subs.Count() > 0)
            y = y + 4;
        for (i = 0; i < 4; i++)
        {
            string sn = "OZG_DToastSub" + i.ToString();
            string subLine = "";
            if (i < subs.Count())
                subLine = subs[i];
            BWSetText(sn, subLine);
            BWShow(sn, i < subs.Count());
            BWMove(BW(sn), 14, y + i * 16);
        }
        y = y + subs.Count() * 16 + 10;
        box.SetSize(356 * m_K, y * m_K);
        Widget edge = BW("OZG_DToastEdge");
        if (edge)
            edge.SetPos(-2 * m_K, -2 * m_K);
        BWFit(edge, 360, y + 4, 8);
        BWFit(BW("OZG_DToastBg"), 356, y, 6);
        int ring = ARGB(255, 107, 82, 51);
        if (hot)
            ring = ARGB(255, 255, 138, 61);
        BWPaint(edge, ring);
        box.Show(true);
        m_DTToastLeft = DT_TOAST;
    }

    // Where the note lies over the table (units of its parent).
    protected void DTToastAt(float x, float y)
    {
        BWMove(BW("OZG_DToast"), x, y);
    }

    // ------------------------------------------------------------ a die slipped in

    // A die swapped, as the one who does it sees it and as each who noticed sees it: a die
    // of 34 flies from the hand at the seat into the can and the one taken out flies back
    // (the canvas's playSwap: 0.5 s, hopping); `noticed`: the die that goes in is ringed
    // orange and the can's foot pulses. `ax`, `ay`: the hand; `bx`, `by`: the can's mouth
    // (the window's units, a die's corner).
    protected void DTSwapPlay(int seat, string inLook, string outLook, bool noticed, float ax, float ay, float bx, float by, OZG_DTCan can)
    {
        m_DTSwapT = 0;
        m_DTSwapSeat = seat;
        m_DTSwapIn = inLook;
        m_DTSwapOut = outLook;
        m_DTSwapNoticed = noticed;
        m_DTSwapAX = ax;
        m_DTSwapAY = ay;
        m_DTSwapBX = bx;
        m_DTSwapBY = by;
        // a face each, taken by chance for this flight: a die is never drawn blank
        m_DTSwapFaceIn = Math.RandomIntInclusive(1, 6);
        m_DTSwapFaceOut = Math.RandomIntInclusive(1, 6);
        if (noticed)
            DTCanPulse(can);
        m_DTSwapSung = false;
        DTSwapLay();
        DTApplyAll();
    }

    // The canvas's keyframes ozgHop at `p`: up 44 at the half, a whole turn.
    protected void DTHopAt(float p, out float ty, out float rot)
    {
        if (p < 0.5)
        {
            float e = DTEaseOut(p / 0.5);
            ty = -44 * e;
            rot = 200 * e;
            return;
        }
        float f = DTEaseOut((p - 0.5) / 0.5);
        ty = -44 + 44 * f;
        rot = 200 + 160 * f;
    }

    // The two dice of the swap where its clock has them.
    protected void DTSwapLay()
    {
        float p = m_DTSwapT / DT_SWAP_FLY;
        if (p > 1)
            p = 1;
        if (p < 0)
            p = 0;
        float ty;
        float rot;
        DTHopAt(p, ty, rot);
        OZG_DTDie goes = DTDieOf("#swapin");
        OZG_DTDie comes = DTDieOf("#swapout");
        int ring = 0;
        if (m_DTSwapNoticed)
            ring = ARGB(255, 255, 138, 61);
        // the die going in: seen until it is almost there (the canvas: its opacity goes
        // out over 0.12 s from 0.42 s on)
        float inAlpha = 1;
        if (m_DTSwapT > 0.42)
            inAlpha = 1 - DTEase((m_DTSwapT - 0.42) / 0.12);
        if (inAlpha < 0)
            inAlpha = 0;
        float outAlpha = DTEase(m_DTSwapT / 0.12);
        if (outAlpha > 1)
            outAlpha = 1;
        if (goes)
        {
            goes.m_Used = true;
            goes.m_Fresh = true;
            DTDieFace(goes, m_DTSwapFaceIn, m_DTSwapIn, 34, false);
            DTDieMarks(goes, ring, 0, 0, false, false);
            float ix = DTMix(m_DTSwapAX, m_DTSwapBX, p) + 17;
            float iy = DTMix(m_DTSwapAY, m_DTSwapBY, p) + 17 + ty;
            DTDieGo(goes, ix, iy, 34, rot, inAlpha, 0);
            if (goes.m_Root)
                goes.m_Root.SetSort(41);
        }
        if (comes)
        {
            comes.m_Used = true;
            comes.m_Fresh = true;
            DTDieFace(comes, m_DTSwapFaceOut, m_DTSwapOut, 34, false);
            DTDieMarks(comes, 0, 0, 0, false, false);
            float ox = DTMix(m_DTSwapBX, m_DTSwapAX, p) + 17;
            float oy = DTMix(m_DTSwapBY, m_DTSwapAY, p) + 17 + ty;
            DTDieGo(comes, ox, oy, 34, rot, outAlpha, 0);
            if (comes.m_Root)
                comes.m_Root.SetSort(40);
        }
    }

    protected void DTSwapTick(float dt)
    {
        m_DTSwapT = m_DTSwapT + dt;
        if (m_DTSwapT >= DT_SWAP)
        {
            m_DTSwapT = -1;
            OZG_DTDie goes = DTDieFind("#swapin");
            OZG_DTDie comes = DTDieFind("#swapout");
            if (goes)
                goes.m_Used = false;
            if (comes)
                comes.m_Used = false;
            BoardWinPaint();
            return;
        }
        // the die clacks as it gets to the can (the canvas: 0.38 s after it left the hand)
        if (!m_DTSwapSung && m_DTSwapT >= 0.38)
        {
            m_DTSwapSung = true;
            DTSound(OZG_SoundIds.DIESLIP);
        }
        DTSwapLay();
    }

    // The two dice of a swap stay in use through a paint made while they fly.
    protected void DTSwapKeep()
    {
        if (m_DTSwapT < 0)
            return;
        DTDieKeep("#swapin");
        DTDieKeep("#swapout");
    }

    // A die swapped, told by the server (OZG_WDiceSaw): played by the window that shows the
    // place. The later files say where the seat's hand and can are.
    void DiceSaw(OZG_WDiceSaw saw)
    {
        if (!saw || !m_V || !BWUp())
            return;
        DTSawPlay(saw);
    }

    protected void DTSawPlay(OZG_WDiceSaw saw)
    {
    }

    // ------------------------------------------------------------ the swap's button

    // "Підмінити кістку" at x, y of the column (or of nardy's throw panel), `w` wide: shown
    // while the server lets this player swap at all in this game; its ring bright when he
    // may swap now, gold while its window is open; how many swaps are left. False when it
    // is not shown.
    protected bool DTSwapButton(float x, float y, float w, bool live)
    {
        Widget b = BW("OZG_DSwapBtn");
        if (!b)
            return false;
        bool shown = live && m_V && m_V.DiceSwapsMost > 0 && m_V.Mine >= 0;
        b.Show(shown);
        if (!shown)
        {
            m_DTSwapOpen = false;
            return false;
        }
        b.SetPos(x * m_K, y * m_K);
        bool can = DTSwapNow();
        if (!can)
            m_DTSwapOpen = false;
        int fill = ARGB(255, 45, 33, 23);
        int ring = ARGB(255, 61, 45, 29);
        int ink = ARGB(255, 160, 143, 115);
        int note = ARGB(255, 184, 165, 133);
        if (can)
        {
            ring = ARGB(255, 107, 82, 51);
            ink = ARGB(255, 239, 228, 204);
        }
        if (m_DTSwapOpen)
        {
            fill = ARGB(255, 201, 164, 92);
            ink = ARGB(255, 26, 18, 10);
            note = ARGB(255, 26, 18, 10);
        }
        BWPaintNamed("OZG_DSwapBtnBoxBg", fill);
        BWPaintNamed("OZG_DSwapBtnBoxEdge", ring);
        BWInk("OZG_DSwapBtnText", ink);
        BWInk("OZG_DSwapBtnNote", note);
        string left = m_V.DiceSwapsLeft.ToString();
        string most = m_V.DiceSwapsMost.ToString();
        BWSetText("OZG_DSwapBtnNote", Tr2("STR_OZG_DWIN_SWAP_LEFT", left, most));
        return true;
    }

    // May this player swap a die now: the server says so, and no throw is being played.
    protected bool DTSwapNow()
    {
        return m_V && m_V.DiceSwapNow == 1 && !DTBusy() && !m_DTBlocked;
    }

    // The button pressed: its window opened or shut; when he may not swap now, why.
    protected void DTSwapToggle()
    {
        if (m_DTSwapOpen)
        {
            m_DTSwapOpen = false;
            BoardWinPaint();
            return;
        }
        if (DTSwapNow())
        {
            m_DTSwapOpen = true;
            m_DTPickOpen = false;
            m_DTFaceSalt = Math.RandomInt(0, 100000);
            m_DTSwapPick = "";
            m_DTSwapTarget = -1;
            OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
            BoardWinPaint();
            return;
        }
        DTToast(DTSwapWhyNot(), "", "", false);
    }

    // Why he may not swap now: none left, one a throw, only before his own throw.
    protected string DTSwapWhyNot()
    {
        if (m_V.DiceSwapsLeft <= 0)
            return Tr("STR_OZG_DWIN_SWAP_NONE_LEFT");
        if (DTMyThrowNext())
            return Tr("STR_OZG_SAY_DSWAP_ONE") + ".";
        return Tr(DTSwapWhenKey()) + ".";
    }

    // The later files' parts: is a throw this player's next move; the words for when a die
    // may be swapped at the game shown.
    protected bool DTMyThrowNext()
    {
        return false;
    }

    protected string DTSwapWhenKey()
    {
        return "STR_OZG_SAY_DSWAP_NOT_NOW";
    }

    // The rows of the swap's window that lie out of the scroll's sight are hidden. The
    // scroll clips what is in it, but for one thing the stand saw: with fourteen dice
    // carried, the ninth row, two heights of the scroll below its edge, drew one line of
    // itself over the felt (at both window sizes, gone once the list was scrolled).
    protected void DTSwapRowsCull()
    {
        if (!m_V || !m_DTSwapRows)
            return;
        DTRowsCull("OZG_DSwRows", m_DTSwapRows, m_V.PocketDice.Count(), 4, 64, 58);
    }

    // The same for any list of this file's windows: `count` rows a `step` apart, each
    // `rowH` high, `inSight` of them in the scroll's sight.
    protected void DTRowsCull(string scrollName, array<Widget> rows, int count, int inSight, float step, float rowH)
    {
        ScrollWidget scroll = ScrollWidget.Cast(BW(scrollName));
        if (!scroll || !rows)
            return;
        int rowsSeen = count;
        if (rowsSeen > inSight)
            rowsSeen = inSight;
        float gap = step - rowH;
        float sightH = (rowsSeen * step - gap) * m_K;
        float most = (count * step - gap) * m_K - sightH;
        if (most < 0)
            most = 0;
        float at = scroll.GetVScrollPos01() * most;
        for (int i = 0; i < rows.Count() && i < count; i++)
        {
            Widget row = rows[i];
            if (!row)
                continue;
            float top = i * step * m_K;
            float bottom = top + rowH * m_K;
            bool seen = bottom > at && top < at + sightH;
            if (row.IsVisible() != seen)
                row.Show(seen);
        }
    }

    // ------------------------------------------------------------ the swap's window

    // A die's name by its look ("Червона кістка").
    protected string DTDieName(string look)
    {
        if (look.IndexOf("black") >= 0)
            return Tr("STR_OZG_DWIN_DIE_BLACK");
        if (look.IndexOf("red") >= 0)
            return Tr("STR_OZG_DWIN_DIE_RED");
        return Tr("STR_OZG_DWIN_DIE_WHITE");
    }

    // The same as what is slipped in ("червону").
    protected string DTDieNameAcc(string look)
    {
        if (look.IndexOf("black") >= 0)
            return Tr("STR_OZG_DWIN_DIE_BLACK_ACC");
        if (look.IndexOf("red") >= 0)
            return Tr("STR_OZG_DWIN_DIE_RED_ACC");
        return Tr("STR_OZG_DWIN_DIE_WHITE_ACC");
    }

    // The swap's window (the canvas's "Вікно підміни кістки"): the dice he carries, a row
    // each -- its picture with the loaded face up, its name, what he knows of it in words,
    // how heavy as bars and a word --, the dice he is about to throw with what he knows of
    // each, one of each picked by him; what the bars mean; what will be done; the buttons.
    // He may carry any number of dice: the rows scroll, four in sight.
    protected void DTPaintSwapDlg()
    {
        Widget dlg = BW("OZG_DSwapDlg");
        if (!dlg)
            return;
        if (m_DTSwapOpen && !DTSwapNow())
            m_DTSwapOpen = false;
        dlg.Show(m_DTSwapOpen);
        if (!m_DTSwapOpen)
            return;
        int i;
        string leftText = m_V.DiceSwapsLeft.ToString();
        string mostText = m_V.DiceSwapsMost.ToString();
        string count = Tr("STR_OZG_DWIN_SWAP_ONE_A");
        if (m_DTSwapUnder)
            count = Tr("STR_OZG_DWIN_SWAP_ONE_ROUND");
        count = count + " · " + Tr2("STR_OZG_DWIN_SWAP_LEFT", leftText, mostText);
        TextWidget title = BWText("OZG_DSwTitle");
        float tw = 0;
        if (title)
            tw = BWTextW(title);
        BWSetText("OZG_DSwCount", count);
        BWMove(BW("OZG_DSwCount"), 18 + tw + 12, 23);

        // ---- step 1: the dice of the pocket
        int pockets = m_V.PocketDice.Count();
        int pick = -1;
        for (i = 0; i < pockets; i++)
        {
            if (m_V.PocketDice[i] == m_DTSwapPick)
                pick = i;
        }
        if (pick < 0)
            m_DTSwapPick = "";
        Widget inside = BW("OZG_DSwRowsIn");
        while (inside && m_DTSwapRows.Count() < pockets)
        {
            Widget made = MakeLayout(DT_ROW, inside, "OZG_DSRow");
            if (!made)
                break;
            int madeNo = m_DTSwapRows.Count();
            made.SetName("OZG_DSRow" + madeNo.ToString());
            m_DTSwapRows.Insert(made);
        }
        OZG_Settings cfg = OZG_Cfg.Get();
        for (i = 0; i < m_DTSwapRows.Count(); i++)
        {
            Widget row = m_DTSwapRows[i];
            if (!row)
                continue;
            bool there = i < pockets;
            row.Show(there);
            if (!there)
                continue;
            row.SetPos(0, i * 64 * m_K);
            string look = "white";
            if (i < m_V.PocketLooks.Count() && m_V.PocketLooks[i] != "")
                look = m_V.PocketLooks[i];
            int loadFace = 0;
            int loadStep = 0;
            if (i < m_V.PocketLoads.Count())
                loadFace = m_V.PocketLoads[i];
            if (i < m_V.PocketSteps.Count())
                loadStep = m_V.PocketSteps[i];
            bool on = i == pick;
            int ring = ARGB(255, 74, 55, 38);
            int fill = ARGB(255, 42, 29, 18);
            if (on)
            {
                ring = ARGB(255, 231, 194, 122);
                fill = ARGB(255, 58, 44, 28);
            }
            BWPaint(row.FindAnyWidget("OZG_DSRowBoxEdge"), ring);
            BWPaint(row.FindAnyWidget("OZG_DSRowBoxBg"), fill);
            // the loaded face up when he knows it; else a face of its own (never blank)
            int faceUp = loadFace;
            if (faceUp <= 0)
            {
                string pocketKey = m_V.PocketDice[i];
                faceUp = OZG_Keys.FaceOf(pocketKey, m_DTFaceSalt);
            }
            ImageWidget pic = ImageWidget.Cast(row.FindAnyWidget("OZG_DSRowPic"));
            if (pic)
            {
                pic.LoadImageFile(0, "set:ozg_dice image:" + look + "_" + faceUp.ToString());
                pic.SetImage(0);
            }
            ImageWidget sh = ImageWidget.Cast(row.FindAnyWidget("OZG_DSRowSh"));
            if (sh)
            {
                sh.LoadImageFile(0, "set:ozg_dice image:die_s");
                sh.SetImage(0);
            }
            TextWidget name = TextWidget.Cast(row.FindAnyWidget("OZG_DSRowTitle"));
            if (name)
                name.SetText(DTDieName(look));
            TextWidget subW = TextWidget.Cast(row.FindAnyWidget("OZG_DSRowSub"));
            ImageWidget bars = ImageWidget.Cast(row.FindAnyWidget("OZG_DSRowBars"));
            TextWidget word = TextWidget.Cast(row.FindAnyWidget("OZG_DSRowWord"));
            bool loaded = loadFace > 0;
            string sub = Tr("STR_OZG_DWIN_SWAP_FAIR");
            int subInk = ARGB(255, 184, 165, 133);
            string stepWord = "";
            if (loaded)
            {
                string faceText = loadFace.ToString();
                stepWord = Tr(OZG_Const.LOAD_STEP + loadStep.ToString());
                int often = 0;
                if (cfg && loadStep >= 1 && loadStep <= cfg.LoadWeights.Count())
                    often = cfg.LoadWeights[loadStep - 1];
                int tens = (often + 5) / 10;
                string oftenText = tens.ToString();
                sub = Tr1("STR_OZG_DWIN_SWAP_LOADED_ON", faceText) + " · " + stepWord + " · " + Tr1("STR_OZG_DWIN_SWAP_OF_TEN", oftenText);
                subInk = ARGB(255, 201, 211, 221);
            }
            if (subW)
            {
                subW.SetText(sub);
                subW.SetColor(subInk);
            }
            if (bars)
            {
                bars.Show(loaded && loadStep > 0);
                if (loaded && loadStep > 0)
                {
                    bars.LoadImageFile(0, BW_TEX + "ozg_sbars_" + loadStep.ToString() + "_ca.paa");
                    bars.SetImage(0);
                }
            }
            if (word)
                word.SetText(stepWord);
        }
        int rowsSeen = pockets;
        if (rowsSeen > 4)
            rowsSeen = 4;
        float rowsH = rowsSeen * 64 - 6;
        if (rowsH < 0)
            rowsH = 0;
        ScrollWidget scroll = ScrollWidget.Cast(BW("OZG_DSwRows"));
        float insideH = pockets * 64 - 6;
        if (insideH < rowsH)
            insideH = rowsH;
        if (scroll)
        {
            scroll.Show(pockets > 0);
            scroll.SetSize(394 * m_K, rowsH * m_K);
        }
        if (inside)
        {
            inside.SetSize(384 * m_K, insideH * m_K);
            inside.Update();
        }
        if (scroll)
        {
            scroll.Update();
            float most = (insideH - rowsH) * m_K;
            if (most < 0)
                most = 0;
            if (scroll.GetVScrollPos() > most)
                scroll.VScrollToPos(most);
        }
        DTSwapRowsCull();
        BWShow("OZG_DSwNone", pockets == 0);
        float leftH = rowsH;
        if (pockets == 0)
        {
            array<string> none = new array<string>();
            BWWrap(BWText("OZG_DSwNone0"), Tr("STR_OZG_DWIN_SWAP_NO_POCKET"), 356, 2, none);
            string n0 = "";
            string n1 = "";
            if (none.Count() > 0)
                n0 = none[0];
            if (none.Count() > 1)
                n1 = none[1];
            BWSetText("OZG_DSwNone0", n0);
            BWSetText("OZG_DSwNone1", n1);
            leftH = 64;
        }

        // ---- step 2: the dice he is about to throw, each by its place among his dice
        m_DTSwapPlaces.Clear();
        for (i = 0; i < m_V.MyDicePlaces.Count() && i < 5; i++)
        {
            int thrownPlace = m_V.MyDicePlaces[i];
            m_DTSwapPlaces.Insert(thrownPlace);
        }
        if (m_DTSwapPlaces.Find(m_DTSwapTarget) < 0)
            m_DTSwapTarget = -1;
        int targets = m_DTSwapPlaces.Count();
        Widget diceBox = BW("OZG_DSwCanDice");
        int mineSeat = m_V.Mine;
        for (i = 0; i < 5; i++)
        {
            string ti = i.ToString();
            bool has = i < targets;
            BWShow("OZG_DSwCanNum" + ti, has);
            BWShow("OZG_DSwCanHit" + ti, has);
            OZG_DTDie cd = null;
            if (i < m_DTSwapDice.Count())
                cd = m_DTSwapDice[i];
            if (!cd && diceBox)
            {
                cd = DTDieMake("#can" + ti, diceBox);
                if (cd)
                    m_DTSwapDice.Insert(cd);
            }
            if (!cd)
                continue;
            cd.m_Used = has;
            if (!has)
                continue;
            int place = m_DTSwapPlaces[i];
            bool picked = place == m_DTSwapTarget;
            string canLook = DTLookAt(mineSeat, place, false);
            int mark = DTMarkAt(mineSeat, place, false);
            int canRing = 0;
            int numInk = ARGB(255, 230, 216, 188);
            if (picked)
            {
                canRing = ARGB(255, 231, 194, 122);
                numInk = ARGB(255, 231, 194, 122);
            }
            int restFace = OZG_Keys.FaceOf("can" + place.ToString(), m_DTFaceSalt);
            DTDieFace(cd, restFace, canLook, 42, false);
            DTDieMarked(cd, mark, canRing, false);
            cd.m_Fresh = true;
            DTDieGoIn(cd, 10 + i * 54 + 23, 12 + 21, 42, 0, 1, 0);
            int shownNo = place + 1;
            BWSetText("OZG_DSwCanNum" + ti, shownNo.ToString());
            BWInk("OZG_DSwCanNum" + ti, numInk);
        }
        for (i = 0; i < m_DTSwapDice.Count(); i++)
            DTDieApply(m_DTSwapDice[i]);
        string noteKey = "STR_OZG_DWIN_SWAP_CAN_NOTE";
        if (m_DTSwapUnder)
            noteKey = "STR_OZG_DWIN_SWAP_CAN_NOTE_UNDER";
        array<string> notes = new array<string>();
        BWWrap(BWText("OZG_DSwNote0"), Tr(noteKey), 282, 4, notes);
        for (i = 0; i < 4; i++)
        {
            string noteLine = "";
            if (i < notes.Count())
                noteLine = notes[i];
            BWSetText("OZG_DSwNote" + i.ToString(), noteLine);
        }
        float rightH = 86 + 8 + notes.Count() * 17;
        float colsH = leftH;
        if (rightH > colsH)
            colsH = rightH;

        // ---- what the bars mean
        float legendY = 90 + colsH + 14;
        BWMove(BW("OZG_DSwLegend"), 18, legendY);
        TextWidget legend = BWText("OZG_DSwLegendLabel");
        float lx = 12;
        if (legend)
            lx = 12 + BWTextW(legend) + 18;
        for (i = 0; i < 3; i++)
        {
            int step = i + 1;
            string li = i.ToString();
            int weight = 0;
            if (cfg && i < cfg.LoadWeights.Count())
                weight = cfg.LoadWeights[i];
            int tenth = (weight + 5) / 10;
            string tenthText = tenth.ToString();
            string words = Tr(OZG_Const.LOAD_STEP + step.ToString()) + ", " + Tr1("STR_OZG_DWIN_SWAP_OF_TEN_SHORT", tenthText);
            BWMove(BW("OZG_DSwLegendBars" + li), lx, 12);
            TextWidget lt = BWText("OZG_DSwLegendText" + li);
            float ltw = 0;
            if (lt)
            {
                lt.SetText(words);
                ltw = BWTextW(lt);
            }
            BWMove(BW("OZG_DSwLegendText" + li), lx + 14.25 + 6, 10.5);
            lx = lx + 14.25 + 6 + ltw + 18;
        }

        // ---- what will be done, and the buttons
        bool ready = pick >= 0 && m_DTSwapTarget >= 0 && m_V.DiceSwapsLeft > 0;
        string summary = Tr("STR_OZG_DWIN_SWAP_SUM_BOTH");
        if (m_V.DiceSwapsLeft <= 0)
            summary = Tr("STR_OZG_DWIN_SWAP_NONE_LEFT");
        else if (pick >= 0 && m_DTSwapTarget < 0)
            summary = Tr("STR_OZG_DWIN_SWAP_SUM_CAN");
        else if (pick < 0 && m_DTSwapTarget >= 0)
            summary = Tr("STR_OZG_DWIN_SWAP_SUM_POCKET");
        else if (ready)
        {
            string pickLook = "white";
            if (pick < m_V.PocketLooks.Count() && m_V.PocketLooks[pick] != "")
                pickLook = m_V.PocketLooks[pick];
            string what = DTDieNameAcc(pickLook);
            int pf = 0;
            int ps = 0;
            if (pick < m_V.PocketLoads.Count())
                pf = m_V.PocketLoads[pick];
            if (pick < m_V.PocketSteps.Count())
                ps = m_V.PocketSteps[pick];
            if (pf > 0)
            {
                string pfText = pf.ToString();
                string psWord = Tr(OZG_Const.LOAD_STEP + ps.ToString());
                what = what + " " + Tr2("STR_OZG_DWIN_SWAP_SUM_LOAD", pfText, psWord);
            }
            int targetNo = m_DTSwapTarget + 1;
            string targetText = targetNo.ToString();
            summary = Tr2("STR_OZG_DWIN_SWAP_SUM_DO", what, targetText);
        }
        float bottomY = legendY + 36 + 14;
        BWSetText("OZG_DSwGoText", Tr("STR_OZG_DWIN_SWAP_GO"));
        DTDlgFoot("OZG_DSw", bottomY, summary, ready);
        // ---- the box round it all, in the middle of what it covers
        DTDlgFit(dlg, "OZG_DSw", bottomY + 42 + 18);
    }

    // The foot of a window of this file (`p`: its widgets' prefix): what will be done -- two
    // lines at most, bright when it can be --, right its two buttons, each as wide as its
    // words: cancel plain, go gold when `ready`.
    protected void DTDlgFoot(string p, float bottomY, string summary, bool ready)
    {
        TextWidget cancelT = BWText(p + "CancelText");
        TextWidget goT = BWText(p + "GoText");
        float cancelW = 110;
        float goW = 120;
        if (cancelT)
            cancelW = BWTextW(cancelT) + 36;
        if (goT)
            goW = BWTextW(goT) + 44;
        float goX = 720 - 18 - goW;
        float cancelX = goX - 12 - cancelW;
        DTButtonAt(p + "Go", goX, bottomY, goW, 42);
        DTButtonAt(p + "Cancel", cancelX, bottomY, cancelW, 42);
        int goFill = ARGB(255, 51, 37, 26);
        int goInk = ARGB(255, 160, 143, 115);
        if (ready)
        {
            goFill = ARGB(255, 201, 164, 92);
            goInk = ARGB(255, 26, 18, 10);
        }
        BWPaintNamed(p + "GoBoxBg", goFill);
        BWInk(p + "GoText", goInk);
        array<string> sums = new array<string>();
        BWWrap(BWText(p + "Sum0"), summary, cancelX - 18 - 12, 2, sums);
        string s0 = "";
        string s1 = "";
        if (sums.Count() > 0)
            s0 = sums[0];
        if (sums.Count() > 1)
            s1 = sums[1];
        BWSetText(p + "Sum0", s0);
        BWSetText(p + "Sum1", s1);
        int sumInk = ARGB(255, 184, 165, 133);
        if (ready)
            sumInk = ARGB(255, 239, 228, 204);
        BWInk(p + "Sum0", sumInk);
        BWInk(p + "Sum1", sumInk);
        float sumY = bottomY + 11.5;
        if (sums.Count() > 1)
            sumY = bottomY + 2;
        BWMove(BW(p + "Sum0"), 18, sumY);
        BWMove(BW(p + "Sum1"), 18, sumY + 19);
    }

    // A window's box (720 wide) sized to what it holds, `boxH` high, and laid in the middle
    // of what its dim covers; its shadow with it.
    protected void DTDlgFit(Widget dlg, string p, float boxH)
    {
        Widget pane = BW(p + "Pane");
        if (pane && dlg)
        {
            float dw;
            float dh;
            dlg.GetSize(dw, dh);
            float px = (dw / m_K - 720) / 2;
            float py = (dh / m_K - boxH) / 2;
            pane.SetPos(Math.Round(px * m_K), Math.Round(py * m_K));
            pane.SetSize(720 * m_K, boxH * m_K);
        }
        Widget boxW = BW(p + "Box");
        if (boxW)
            boxW.SetSize(720 * m_K, boxH * m_K);
        BWFit(BW(p + "BoxEdge"), 722, boxH + 2, 9);
        BWFit(BW(p + "BoxBg"), 720, boxH, 8);
        // its shadow: the board's shadow picture, whose box is 880 of its 1040, 18 lower
        Widget shadow = BW(p + "Shadow");
        if (shadow)
        {
            float m = 80.0 / 880;
            float sc = 1040.0 / 880;
            shadow.SetPos(-720 * m * m_K, (18 - boxH * m) * m_K);
            shadow.SetSize(720 * sc * m_K, boxH * sc * m_K);
        }
    }

    // A button of the layout moved and sized, its box's shapes and its words with it.
    protected void DTButtonAt(string name, float x, float y, float w, float h)
    {
        Widget b = BW(name);
        if (!b)
            return;
        b.SetPos(x * m_K, y * m_K);
        b.SetSize(w * m_K, h * m_K);
        Widget boxW = BW(name + "Box");
        if (boxW)
            boxW.SetSize(w * m_K, h * m_K);
        Widget edge = BW(name + "BoxEdge");
        BWFit(edge, w, h, 4);
        Widget bg = BW(name + "BoxBg");
        if (bg)
        {
            if (edge)
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

    // The swap asked of the server: the die picked of the pocket for the die picked of the
    // can (its place among his dice).
    protected void DTSwapSend()
    {
        if (m_DTSwapPick == "" || m_DTSwapTarget < 0 || !DTSwapNow())
            return;
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = m_PlaceId;
        ask.CardKey = m_DTSwapPick;
        ask.SeatNo = m_DTSwapTarget;
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_DSWAP, json);
        m_DTSwapOpen = false;
        m_DTSwapPick = "";
        m_DTSwapTarget = -1;
        BoardWinPaint();
    }

    // ------------------------------------------------------------ the can and the dice

    // The choice of the can a player throws from and of the dice of it he throws (design
    // §8; the canvas's "Вікно «Банка й кості»"). The view lists the cans he may throw from
    // and their dice (OZG_WView.Pick*): the one laid out where the game lets him, then the
    // cans he carries. He chooses in the open -- the table sees the can at his seat and the
    // dice on the felt --, so nothing here is a sleight: no chance, nothing to notice, as
    // often as he likes. The can before a deal; the dice before a deal and, in a game,
    // before each throw of his own.

    // The dice a can of the pick lists holds.
    protected int DTPickCount(int can)
    {
        if (!m_V || can < 0)
            return 0;
        int n = 0;
        for (int i = 0; i < m_V.PickDieCan.Count(); i++)
        {
            if (m_V.PickDieCan[i] == can)
                n++;
        }
        return n;
    }

    // Has this player anything to choose at the game shown: the can he throws from holds
    // more dice than the game throws, or -- before a deal -- there is another can to take.
    protected bool DTPickAny()
    {
        if (!m_V || m_V.Mine < 0 || m_V.PickNeed <= 0)
            return false;
        int inPlay = m_V.PickCanOn.Find(1);
        if (inPlay < 0)
            return false;
        if (DTPickCount(inPlay) > m_V.PickNeed)
            return true;
        return m_V.PickCanNow == 1 && m_V.PickCanKeys.Count() > 1;
    }

    // May he choose now: the server says so -- a can before the deal, dice before his own
    // throw --, and no throw is being played.
    protected bool DTPickNow()
    {
        if (!m_V || DTBusy())
            return false;
        return m_V.PickCanNow == 1 || m_V.PickDiceNow == 1;
    }

    // "Банка й кості" at x, y of the column (or of nardy's throw panel), `w` wide: shown
    // while this player has a can or dice to choose at the game shown; its ring bright when
    // he may choose now, gold while its window is open; whose can he throws from and how
    // many of its dice. False when it is not shown.
    protected bool DTPickButton(float x, float y, float w, bool live)
    {
        Widget b = BW("OZG_DPickBtn");
        if (!b)
            return false;
        bool shown = live && DTPickAny();
        b.Show(shown);
        if (!shown)
        {
            m_DTPickOpen = false;
            return false;
        }
        b.SetPos(x * m_K, y * m_K);
        bool can = DTPickNow();
        if (!can)
            m_DTPickOpen = false;
        int fill = ARGB(255, 45, 33, 23);
        int ring = ARGB(255, 61, 45, 29);
        int ink = ARGB(255, 160, 143, 115);
        int note = ARGB(255, 184, 165, 133);
        if (can)
        {
            ring = ARGB(255, 107, 82, 51);
            ink = ARGB(255, 239, 228, 204);
        }
        if (m_DTPickOpen)
        {
            fill = ARGB(255, 201, 164, 92);
            ink = ARGB(255, 26, 18, 10);
            note = ARGB(255, 26, 18, 10);
        }
        BWPaintNamed("OZG_DPickBtnBoxBg", fill);
        BWPaintNamed("OZG_DPickBtnBoxEdge", ring);
        BWInk("OZG_DPickBtnText", ink);
        BWInk("OZG_DPickBtnNote", note);
        int inPlay = m_V.PickCanOn.Find(1);
        string noteKey = "STR_OZG_DWIN_PICK_NOTE_OWN";
        if (inPlay >= 0 && inPlay < m_V.PickCanKeys.Count())
        {
            string canKey = m_V.PickCanKeys[inPlay];
            if (canKey == OZG_DiceConst.PICK_HOST)
                noteKey = "STR_OZG_DWIN_PICK_NOTE_HOST";
        }
        string needText = m_V.PickNeed.ToString();
        string haveText = DTPickCount(inPlay).ToString();
        BWSetText("OZG_DPickBtnNote", Tr2(noteKey, needText, haveText));
        return true;
    }

    // The button pressed: its window opened or shut; when he may not choose now, why. A
    // window just opened asks the server again: a can picked up since the last view is
    // there to choose.
    protected void DTPickToggle()
    {
        if (m_DTPickOpen)
        {
            m_DTPickOpen = false;
            BoardWinPaint();
            return;
        }
        if (DTPickNow())
        {
            m_DTPickOpen = true;
            m_DTPickReveal = true;
            m_DTSwapOpen = false;
            m_DTFaceSalt = Math.RandomInt(0, 100000);
            m_DTPickCan = "";
            int inPlay = m_V.PickCanOn.Find(1);
            if (inPlay >= 0 && inPlay < m_V.PickCanKeys.Count())
                m_DTPickCan = m_V.PickCanKeys[inPlay];
            DTPickDefault();
            OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
            Send(OZG_Const.OP_OPEN, "", -1, "", "", "", 0);
            BoardWinPaint();
            return;
        }
        if (!DTBusy())
            DTToast(Tr("STR_OZG_SAY_DPICK_NOT_NOW") + ".", "", "", false);
    }

    // The dice of the can chosen in the window that are his as the server has it; when it
    // has none of that can, the first so many.
    protected void DTPickDefault()
    {
        m_DTPickKeys.Clear();
        if (!m_V)
            return;
        int can = m_V.PickCanKeys.Find(m_DTPickCan);
        if (can < 0)
            return;
        array<string> firsts = new array<string>();
        for (int i = 0; i < m_V.PickDieCan.Count() && i < m_V.PickDieKeys.Count(); i++)
        {
            if (m_V.PickDieCan[i] != can)
                continue;
            string key = m_V.PickDieKeys[i];
            if (firsts.Count() < m_V.PickNeed)
                firsts.Insert(key);
            if (i < m_V.PickDieOn.Count() && m_V.PickDieOn[i] == 1)
                m_DTPickKeys.Insert(key);
        }
        if (m_DTPickKeys.Count() != m_V.PickNeed)
            m_DTPickKeys.Copy(firsts);
    }

    // Whose can a can of the pick lists is, in words: the one laid out (at nardy the
    // board's dice), or his own by its kind.
    protected string DTPickCanTitle(int can)
    {
        if (!m_V || can < 0 || can >= m_V.PickCanKeys.Count())
            return "";
        string key = m_V.PickCanKeys[can];
        if (key == OZG_DiceConst.PICK_HOST)
        {
            if (m_Style == "board_nardy")
                return Tr("STR_OZG_DWIN_PICK_CAN_BOARD");
            return Tr("STR_OZG_DWIN_PICK_CAN_HOST");
        }
        string look = "";
        if (can < m_V.PickCanLooks.Count())
            look = m_V.PickCanLooks[can];
        if (look == "stalker")
            return Tr("STR_OZG_DWIN_PICK_CAN_ARMY");
        return Tr("STR_OZG_DWIN_PICK_CAN_TIN");
    }

    // A count and its noun in the form it takes ("1 кістка", "3 кістки", "5 кісток").
    protected string DTCounted(string key, int n)
    {
        string s = Tr(key + OZG_Text.CountForm(n));
        s.Replace("%1", n.ToString());
        return s;
    }

    // The window (the canvas's "Вікно «Банка й кості»"): the cans this player may throw
    // from, a row each; the dice of the can chosen with what he knows of each, the ones of
    // his throw ringed gold; a note under each list; what he throws; the buttons. In a game
    // the can is the one in play and only the dice are his to choose.
    protected void DTPaintPickDlg()
    {
        Widget dlg = BW("OZG_DPickDlg");
        if (!dlg)
            return;
        if (m_DTPickOpen)
        {
            bool still = DTPickAny() && DTPickNow();
            if (!still)
                m_DTPickOpen = false;
        }
        dlg.Show(m_DTPickOpen);
        if (!m_DTPickOpen)
            return;
        int i;
        int need = m_V.PickNeed;
        bool fixedCan = m_V.PickCanNow != 1;
        int cans = m_V.PickCanKeys.Count();
        // the can chosen here: still one of them, and in a game the one in play
        int inPlay = m_V.PickCanOn.Find(1);
        int at = m_V.PickCanKeys.Find(m_DTPickCan);
        bool lost = at < 0;
        if (fixedCan && at != inPlay)
            lost = true;
        if (lost)
        {
            at = inPlay;
            m_DTPickCan = "";
            if (at >= 0 && at < cans)
                m_DTPickCan = m_V.PickCanKeys[at];
            DTPickDefault();
        }
        // its dice as the lists have them; a die chosen that it no longer holds is not
        array<int> diceAt = new array<int>();
        m_DTPickShown.Clear();
        for (i = 0; i < m_V.PickDieCan.Count() && i < m_V.PickDieKeys.Count(); i++)
        {
            if (m_V.PickDieCan[i] != at)
                continue;
            string shownKey = m_V.PickDieKeys[i];
            diceAt.Insert(i);
            m_DTPickShown.Insert(shownKey);
        }
        for (i = m_DTPickKeys.Count() - 1; i >= 0; i--)
        {
            string hadKey = m_DTPickKeys[i];
            bool gone = m_DTPickShown.Find(hadKey) < 0;
            if (gone || i >= need)
                m_DTPickKeys.RemoveOrdered(i);
        }
        TextWidget title = BWText("OZG_DPkTitle");
        float tw = 0;
        if (title)
            tw = BWTextW(title);
        BWMove(BW("OZG_DPkCount"), 18 + tw + 12, 23);

        // ---- step 1: the cans, three in sight
        Widget inside = BW("OZG_DPkRowsIn");
        while (inside && m_DTPickRows.Count() < cans)
        {
            Widget made = MakeLayout(DT_PROW, inside, "OZG_DPRow");
            if (!made)
                break;
            int madeNo = m_DTPickRows.Count();
            made.SetName("OZG_DPRow" + madeNo.ToString());
            m_DTPickRows.Insert(made);
        }
        for (i = 0; i < m_DTPickRows.Count(); i++)
        {
            Widget row = m_DTPickRows[i];
            if (!row)
                continue;
            bool there = i < cans;
            row.Show(there);
            if (!there)
                continue;
            row.SetPos(0, i * 72 * m_K);
            DTPickRowPaint(row, i, i == at, fixedCan);
        }
        int rowsSeen = cans;
        if (rowsSeen > 3)
            rowsSeen = 3;
        float rowsH = rowsSeen * 72 - 6;
        if (rowsH < 0)
            rowsH = 0;
        float insideH = cans * 72 - 6;
        if (insideH < rowsH)
            insideH = rowsH;
        ScrollWidget scroll = ScrollWidget.Cast(BW("OZG_DPkRows"));
        if (scroll)
            scroll.SetSize(340 * m_K, rowsH * m_K);
        if (inside)
        {
            inside.SetSize(330 * m_K, insideH * m_K);
            inside.Update();
        }
        if (scroll)
        {
            scroll.Update();
            float most = (insideH - rowsH) * m_K;
            if (most < 0)
                most = 0;
            if (scroll.GetVScrollPos() > most)
                scroll.VScrollToPos(most);
            // just opened with more cans than are in sight: the one chosen is in sight
            if (m_DTPickReveal && at >= 0)
            {
                float sightH = rowsH * m_K;
                float rowTop = at * 72 * m_K;
                float rowEnd = rowTop + 66 * m_K;
                float scrollAt = scroll.GetVScrollPos();
                if (rowEnd > scrollAt + sightH)
                    scrollAt = rowEnd - sightH;
                if (rowTop < scrollAt)
                    scrollAt = rowTop;
                scroll.VScrollToPos(scrollAt);
            }
        }
        m_DTPickReveal = false;
        DTRowsCull("OZG_DPkRows", m_DTPickRows, cans, 3, 72, 66);
        string canNote = Tr("STR_OZG_DWIN_PICK_CAN_NOTE_ONE");
        if (fixedCan)
            canNote = Tr("STR_OZG_DWIN_PICK_CAN_NOTE_LOCK");
        else if (cans > 1)
            canNote = Tr("STR_OZG_DWIN_PICK_CAN_NOTE");
        array<string> canNotes = new array<string>();
        BWWrap(BWText("OZG_DPkCanNote0"), canNote, 330, 3, canNotes);
        for (i = 0; i < 3; i++)
        {
            string canNoteName = "OZG_DPkCanNote" + i.ToString();
            string canLine = "";
            if (i < canNotes.Count())
                canLine = canNotes[i];
            BWSetText(canNoteName, canLine);
            BWMove(BW(canNoteName), 18, 90 + rowsH + 8 + i * 17);
        }
        float leftH = rowsH + 8 + canNotes.Count() * 17;

        // ---- step 2: the dice of the can chosen, each on a face of its own
        string diceTitle = Tr("STR_OZG_DWIN_PICK_STEP2_ONE");
        if (need > 1)
            diceTitle = Tr1("STR_OZG_DWIN_PICK_STEP2", need.ToString());
        BWSetText("OZG_DPkLabel2", diceTitle);
        Widget diceBox = BW("OZG_DPkCanDice");
        int shown = diceAt.Count();
        for (i = 0; i < 5; i++)
        {
            string ti = i.ToString();
            bool has = i < shown;
            string dieKey = "";
            if (has)
                dieKey = m_DTPickShown[i];
            bool picked = has && m_DTPickKeys.Find(dieKey) >= 0;
            BWShow("OZG_DPkHit" + ti, has);
            BWShow("OZG_DPkWord" + ti, picked);
            OZG_DTDie cd = null;
            if (i < m_DTPickDice.Count())
                cd = m_DTPickDice[i];
            if (!cd && diceBox)
            {
                cd = DTDieMake("#pick" + ti, diceBox);
                if (cd)
                    m_DTPickDice.Insert(cd);
            }
            if (!cd)
                continue;
            cd.m_Used = has;
            if (!has)
                continue;
            int listAt = diceAt[i];
            string look = "white";
            if (listAt < m_V.PickDieLooks.Count() && m_V.PickDieLooks[listAt] != "")
                look = m_V.PickDieLooks[listAt];
            int mark = 0;
            if (listAt < m_V.PickDieMarks.Count())
                mark = m_V.PickDieMarks[listAt];
            int dieRing = 0;
            if (picked)
                dieRing = ARGB(255, 231, 194, 122);
            int face = OZG_Keys.FaceOf(dieKey, m_DTFaceSalt);
            DTDieFace(cd, face, look, 46, false);
            DTDieMarked(cd, mark, dieRing, false);
            cd.m_Fresh = true;
            DTDieGoIn(cd, 37 + i * 64, 37, 46, 0, 1, 0);
        }
        for (i = 0; i < m_DTPickDice.Count(); i++)
            DTDieApply(m_DTPickDice[i]);
        string diceNote = Tr("STR_OZG_DWIN_PICK_NOTE_ONE");
        if (need > 1)
            diceNote = Tr("STR_OZG_DWIN_PICK_NOTE_MANY");
        if (shown == need)
            diceNote = Tr("STR_OZG_DWIN_PICK_NOTE_EXACT");
        array<string> notes = new array<string>();
        BWWrap(BWText("OZG_DPkNote0"), diceNote, 336, 4, notes);
        for (i = 0; i < 4; i++)
        {
            string noteLine = "";
            if (i < notes.Count())
                noteLine = notes[i];
            BWSetText("OZG_DPkNote" + i.ToString(), noteLine);
        }
        float rightH = 91 + 8 + notes.Count() * 17;
        float colsH = leftH;
        if (rightH > colsH)
            colsH = rightH;

        // ---- what he throws, and the buttons
        int chosen = m_DTPickKeys.Count();
        bool ready = at >= 0 && chosen == need;
        string canName = DTPickCanTitle(at);
        string summary = Tr("STR_OZG_DWIN_PICK_SUM");
        summary.Replace("%1", canName);
        summary.Replace("%2", need.ToString());
        summary.Replace("%3", shown.ToString());
        if (shown == need)
            summary = Tr1("STR_OZG_DWIN_PICK_SUM_ALL", canName);
        if (!ready)
        {
            int more = need - chosen;
            summary = Tr("STR_OZG_DWIN_PICK_MORE_ONE");
            if (more != 1)
                summary = Tr1("STR_OZG_DWIN_PICK_MORE", DTCounted("STR_OZG_KWIN_DICE_", more));
        }
        float bottomY = 90 + colsH + 14;
        DTDlgFoot("OZG_DPk", bottomY, summary, ready);
        // ---- the box round it all, in the middle of what it covers
        DTDlgFit(dlg, "OZG_DPk", bottomY + 42 + 18);
    }

    // A row of that window, the can `can` of the pick lists: the can drawn with its dice
    // in its mouth, whose it is, how many dice it holds (too few for the game: said so,
    // and the row half seen), ringed gold when it is the one chosen (`on`); while a game is
    // on (`fixedCan`) the can in play carries the chip and the others are half seen.
    protected void DTPickRowPaint(Widget row, int can, bool on, bool fixedCan)
    {
        int need = m_V.PickNeed;
        string canKey = m_V.PickCanKeys[can];
        string canLook = "";
        if (can < m_V.PickCanLooks.Count())
            canLook = m_V.PickCanLooks[can];
        bool hosts = canKey == OZG_DiceConst.PICK_HOST;
        bool army = canLook == "stalker";
        array<string> looks = new array<string>();
        int i;
        for (i = 0; i < m_V.PickDieCan.Count() && i < m_V.PickDieLooks.Count(); i++)
        {
            if (m_V.PickDieCan[i] != can)
                continue;
            string dieLook = m_V.PickDieLooks[i];
            looks.Insert(dieLook);
        }
        int held = looks.Count();
        bool few = held < need;
        bool shut = few;
        if (fixedCan && !on)
            shut = true;
        int ring = ARGB(255, 74, 55, 38);
        int fill = ARGB(255, 42, 29, 18);
        if (on)
        {
            ring = ARGB(255, 231, 194, 122);
            fill = ARGB(255, 58, 44, 28);
        }
        BWPaint(row.FindAnyWidget("OZG_DPRowBoxEdge"), ring);
        BWPaint(row.FindAnyWidget("OZG_DPRowBoxBg"), fill);
        Widget whole = row.FindAnyWidget("OZG_DPRowAll");
        if (whole)
        {
            float seen = 1;
            if (shut)
                seen = 0.5;
            whole.SetAlpha(seen);
        }
        ImageWidget body = ImageWidget.Cast(row.FindAnyWidget("OZG_DPRowCan"));
        if (body)
        {
            string tex = "ozg_can_tin_open_ca.paa";
            if (army)
                tex = "ozg_can_army_open_ca.paa";
            body.LoadImageFile(0, BW_TEX + tex);
            body.SetImage(0);
        }
        for (i = 0; i < 7; i++)
        {
            ImageWidget lying = ImageWidget.Cast(row.FindAnyWidget("OZG_DPRowIn" + i.ToString()));
            if (!lying)
                continue;
            lying.Show(i < held);
            if (i < held)
            {
                string lyingLook = looks[i];
                lying.SetColor(DTLookColour(lyingLook));
            }
        }
        TextWidget name = TextWidget.Cast(row.FindAnyWidget("OZG_DPRowTitle"));
        if (name)
            name.SetText(DTPickCanTitle(can));
        string sub = DTCounted("STR_OZG_KWIN_DICE_", held);
        int subInk = ARGB(255, 184, 165, 133);
        if (few)
        {
            sub = Tr1("STR_OZG_DWIN_PICK_FEW", sub);
            subInk = ARGB(255, 201, 138, 106);
        }
        else if (hosts)
        {
            sub = Tr1("STR_OZG_DWIN_PICK_TABLES", sub);
        }
        TextWidget subW = TextWidget.Cast(row.FindAnyWidget("OZG_DPRowSub"));
        if (subW)
        {
            subW.SetText(sub);
            subW.SetColor(subInk);
        }
        // the chip of the can a game is on with, as wide as its words
        Widget chip = row.FindAnyWidget("OZG_DPRowChip");
        if (!chip)
            return;
        bool chipOn = fixedCan && on;
        chip.Show(chipOn);
        if (!chipOn)
            return;
        TextWidget chipText = TextWidget.Cast(row.FindAnyWidget("OZG_DPRowChipText"));
        float cw = 57;
        if (chipText)
            cw = BWTextW(chipText) + 16;
        chip.SetPos((330 - 13 - cw) * m_K, 22 * m_K);
        chip.SetSize(cw * m_K, 22 * m_K);
        BWFit(row.FindAnyWidget("OZG_DPRowChipBg"), cw, 22, 11);
        if (chipText)
            chipText.SetSize(cw * m_K, 22 * m_K);
    }

    // A can's row pressed: that can chosen, with the dice of it that are his by the
    // server's word, else its first so many. Not in a game (the can is chosen before the
    // deal), and not a can with too few dice for the game.
    protected void DTPickCanClick(int can)
    {
        if (!m_V || m_V.PickCanNow != 1 || can < 0 || can >= m_V.PickCanKeys.Count())
            return;
        string key = m_V.PickCanKeys[can];
        if (key == m_DTPickCan || DTPickCount(can) < m_V.PickNeed)
            return;
        m_DTPickCan = key;
        DTPickDefault();
        OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
        BoardWinPaint();
    }

    // A die pressed. One die a throw: that die is the one. More: a die of the throw goes
    // out of it, another comes into it while there is room.
    protected void DTPickDieClick(string key)
    {
        if (!m_V || key == "")
            return;
        int need = m_V.PickNeed;
        int had = m_DTPickKeys.Find(key);
        if (need == 1)
        {
            if (had >= 0)
                return;
            m_DTPickKeys.Clear();
            m_DTPickKeys.Insert(key);
        }
        else if (had >= 0)
        {
            m_DTPickKeys.RemoveOrdered(had);
        }
        else if (m_DTPickKeys.Count() < need)
        {
            m_DTPickKeys.Insert(key);
        }
        else
        {
            return;
        }
        OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
        BoardWinPaint();
    }

    // The choice asked of the server (OZG_Const.OP_DPICK): the can -- named before a deal
    // only, in a game it is the one in play -- and the keys of the dice of it he throws.
    protected void DTPickSend()
    {
        if (!m_V || !DTPickNow() || m_DTPickKeys.Count() != m_V.PickNeed)
            return;
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = m_PlaceId;
        ask.CardKey = "";
        if (m_V.PickCanNow == 1)
            ask.CardKey = m_DTPickCan;
        string keys = "";
        for (int i = 0; i < m_DTPickKeys.Count(); i++)
        {
            string key = m_DTPickKeys[i];
            if (i > 0)
                keys = keys + ",";
            keys = keys + key;
        }
        ask.Move = keys;
        string json;
        string err;
        JsonFileLoader<OZG_WAsk>.MakeData(ask, json, err, false);
        OZ_Rpc.ServiceRequest(OZG_Const.SERVICE, OZG_Const.OP_DPICK, json);
        m_DTPickOpen = false;
        OZG_Sounds.PlayLocal("click", OZG_Place.OZG_ById(m_PlaceId));
        BoardWinPaint();
    }

    // The clicks of that window while it is open: nothing under it is pressed.
    protected bool DTPickClick(string n)
    {
        if (n == "OZG_DPkClose" || n == "OZG_DPkCancel")
        {
            m_DTPickOpen = false;
            BoardWinPaint();
            return true;
        }
        if (n == "OZG_DPkGo")
        {
            DTPickSend();
            return true;
        }
        if (n.IndexOf("OZG_DPRow") == 0 && n.Length() > 9)
        {
            string rowText = n.Substring(9, n.Length() - 9);
            int r = rowText.ToInt();
            if (rowText == r.ToString())
                DTPickCanClick(r);
            return true;
        }
        if (n.IndexOf("OZG_DPkHit") == 0 && n.Length() == 11)
        {
            int t = n.Substring(10, 1).ToInt();
            if (t >= 0 && t < m_DTPickShown.Count())
            {
                string dieKey = m_DTPickShown[t];
                DTPickDieClick(dieKey);
            }
            return true;
        }
        return true;
    }

    override void Update(float timeslice)
    {
        super.Update(timeslice);
        if (!BWUp() || !m_Win || m_Closing || !m_V)
            return;
        if (m_Style == "dice" || m_Style == "board_nardy")
            DTTick(timeslice);
    }

    // The clicks of the two buttons -- the swap's, the can and dice's -- and of their
    // windows. While a window is open nothing under it is pressed.
    protected bool DTClick(string n)
    {
        if (n == "OZG_DSwapBtn")
        {
            DTSwapToggle();
            return true;
        }
        if (n == "OZG_DPickBtn")
        {
            DTPickToggle();
            return true;
        }
        if (m_DTPickOpen)
            return DTPickClick(n);
        if (!m_DTSwapOpen)
            return false;
        if (n == "OZG_DSwClose" || n == "OZG_DSwCancel")
        {
            m_DTSwapOpen = false;
            BoardWinPaint();
            return true;
        }
        if (n == "OZG_DSwGo")
        {
            DTSwapSend();
            return true;
        }
        if (n.IndexOf("OZG_DSRow") == 0 && n.Length() > 9)
        {
            string rowText = n.Substring(9, n.Length() - 9);
            int r = rowText.ToInt();
            if (rowText == r.ToString() && r >= 0 && r < m_V.PocketDice.Count())
            {
                string key = m_V.PocketDice[r];
                if (key == m_DTSwapPick)
                    m_DTSwapPick = "";
                else
                    m_DTSwapPick = key;
                BoardWinPaint();
            }
            return true;
        }
        if (n.IndexOf("OZG_DSwCanHit") == 0 && n.Length() == 14)
        {
            int t = n.Substring(13, 1).ToInt();
            int wanted = -1;
            if (t >= 0 && t < m_DTSwapPlaces.Count())
                wanted = m_DTSwapPlaces[t];
            if (wanted == m_DTSwapTarget)
                m_DTSwapTarget = -1;
            else
                m_DTSwapTarget = wanted;
            BoardWinPaint();
            return true;
        }
        return true;
    }
}
