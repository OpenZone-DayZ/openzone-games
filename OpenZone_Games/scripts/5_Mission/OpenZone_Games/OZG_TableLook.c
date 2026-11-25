// Where things lie in one look of the game window: the table of the design
// canvas of 2026-11-24, classic or stalker, chosen by the deck the game was
// laid out from. Layout units of the 1600 x 900 window. tools/make_layouts.py
// draws the same places into ozg_win_<style>.layout; the two must agree.

// Where one other seat sits round the table: its fan of backs and its plate.
class OZG_TableSpot
{
    float X;        // the fan's middle card
    float Y;
    float Turn;     // degrees, clockwise: 180 at the top, -90 on the left, 90 on the right
    float PlateX;   // the plate's middle, at its top edge
    float PlateY;
    float PlateW;
    float Scale;    // of the fan's cards
}

class OZG_TableLook
{
    string Style;
    bool Caps;              // capitals in the title, names, labels and buttons

    // The player's hand: a fan round a point PivotR below its middle card.
    float HandX;
    float HandY;
    float HandR;
    float HandSpread;       // degrees between two cards at most
    float HandFan;          // degrees the whole fan may take
    float CardW;
    float CardH;
    float HoverLift;
    float PickLift;

    // The other seats' fans.
    float FanR;
    float FanSpread;
    float FanFan;
    float FanW;
    float FanH;

    // The stock: the top-left of its lowest card, the trump's top-left before
    // it is turned; the discards' middle; the bout's middle and first row.
    float StockX;
    float StockY;
    float TrumpX;
    float TrumpY;
    float PileW;
    float PileH;
    float DiscardX;
    float DiscardY;
    float DiscardW;
    float DiscardH;
    float BoutX;
    float BoutY;
    float BoutW;
    float BoutH;

    // The buttons of the moment and the right column's stack.
    float MomentX;
    float MomentY;
    float MomentStep;
    float StackX;
    float StackY;
    float StackStep;

    // The seats when there are few: named places of the mock-up.
    float TopY;
    float TopPlateY;
    float TopLeftX;
    float TopX;
    float TopRightX;
    float SideY;
    float SidePlateY;
    float LeftX;
    float RightX;
    float PlateW;

    // Colours.
    int Glow;               // behind a card that can be played
    int PickGlow;           // behind the card picked up
    int TargetGlow;         // behind a card on the table the picked card can beat
    int EdgeTurn;           // a plate whose seat the game waits for
    int EdgeIdle;
    int RoleTurn;
    int RoleIdle;

    // The card sharp's phase 4 (design §10, the canvas's boards Main and
    // Stalker): tools/make_layouts.py draws the boxes, these are the places of
    // what the script lays in them and moves.
    // The pockets: the middle of the first card, the step to the next, a
    // card's size; the room the cards may take along the strip.
    float PocketX;
    float PocketY;
    float PocketStep;
    float PocketW;
    float PocketH;
    float PocketRoom;
    // The right column while the panel of sleights shows: the log's lines,
    // the host's line and the first stacked button, how many buttons fit.
    int LogShort;
    float HostLineY;
    float HostLineY2;
    float StackY2;
    int StackFits2;
    // The dealer's frame: the cards he reads (their first middle, the step,
    // per row and the rows' step), the tick under each.
    float TileX;
    float TileY;
    float TileStep;
    int TilesPerRow;
    float TileRowStep;
    float TickDown;
    // Colours of the sleights: armed (pink or rust), the glow of what an
    // armed sleight takes, brighter for the card picked; a button ready,
    // armed, spent and its label's two inks; the hint's two inks.
    int Hot;
    int SleightGlow;
    int SleightPick;
    int BtnBg;
    int BtnBgArmed;
    int BtnBgSpent;
    int BtnEdge;
    int BtnInk;
    int BtnInkSpent;
    int HintInk;
    int HintHot;
    int TickIdle;
    int TickOn;

    protected static ref OZG_TableLook s_Classic;
    protected static ref OZG_TableLook s_Stalker;

    static OZG_TableLook Of(string style)
    {
        if (style == "stalker")
        {
            if (!s_Stalker)
                s_Stalker = MakeStalker();
            return s_Stalker;
        }
        if (!s_Classic)
            s_Classic = MakeClassic();
        return s_Classic;
    }

    protected void Common()
    {
        HandR = 620;
        HandSpread = 7;
        HandFan = 44;
        CardW = 100;
        CardH = 140;
        HoverLift = 16;
        PickLift = 28;
        FanR = 200;
        FanSpread = 9;
        FanFan = 60;
        FanW = 58;
        FanH = 81;
        PileW = 84;
        PileH = 118;
        DiscardW = 76;
        DiscardH = 106;
        BoutW = 88;
        BoutH = 123;
        MomentStep = 46;
        StackStep = 46;
        PocketStep = 68;
        PocketW = 54;
        PocketH = 76;
        PocketRoom = 198;
        TileStep = 70;
        TilesPerRow = 5;
        TileRowStep = 112;
        TickDown = 56;
    }

    protected static OZG_TableLook MakeClassic()
    {
        OZG_TableLook k = new OZG_TableLook();
        k.Common();
        k.Style = "classic";
        k.Caps = false;
        k.HandX = 602;
        k.HandY = 710;
        k.StockX = 310;
        k.StockY = 315;
        k.TrumpX = 340;
        k.TrumpY = 315;
        k.DiscardX = 852;
        k.DiscardY = 374;
        k.BoutX = 610;
        k.BoutY = 352;
        k.MomentX = 900;
        k.MomentY = 770;
        k.StackX = 1180;
        k.StackY = 494;
        k.TopY = 180;
        k.TopPlateY = 236;
        k.TopLeftX = 302;
        k.TopX = 602;
        k.TopRightX = 902;
        k.SideY = 414;
        k.SidePlateY = 584;
        k.LeftX = 148;
        k.RightX = 1056;
        k.PlateW = 180;
        k.Glow = ARGB(165, 236, 196, 64);
        k.PickGlow = ARGB(255, 255, 236, 160);
        k.TargetGlow = ARGB(230, 236, 196, 64);
        k.EdgeTurn = ARGB(255, 236, 196, 64);
        k.EdgeIdle = ARGB(255, 70, 84, 76);
        k.RoleTurn = ARGB(255, 236, 196, 64);
        k.RoleIdle = ARGB(255, 152, 163, 155);
        k.PocketX = 107;
        k.PocketY = 770;
        k.LogShort = 13;
        k.HostLineY = 462;
        k.HostLineY2 = 680;
        k.StackY2 = 705;
        k.StackFits2 = 3;
        k.TileX = 482;
        k.TileY = 512;
        k.Hot = ARGB(255, 255, 120, 170);
        k.SleightGlow = ARGB(185, 255, 120, 170);
        k.SleightPick = ARGB(255, 255, 150, 195);
        k.BtnBg = ARGB(255, 44, 54, 50);
        k.BtnBgArmed = ARGB(255, 74, 42, 58);
        k.BtnBgSpent = ARGB(255, 35, 42, 39);
        k.BtnEdge = ARGB(255, 255, 120, 170);
        k.BtnInk = ARGB(255, 226, 230, 222);
        k.BtnInkSpent = ARGB(255, 111, 122, 116);
        k.HintInk = ARGB(255, 152, 163, 155);
        k.HintHot = ARGB(255, 255, 156, 196);
        k.TickIdle = ARGB(255, 152, 163, 155);
        k.TickOn = ARGB(255, 236, 196, 64);
        return k;
    }

    protected static OZG_TableLook MakeStalker()
    {
        OZG_TableLook k = new OZG_TableLook();
        k.Common();
        k.Style = "stalker";
        k.Caps = true;
        k.HandX = 595;
        k.HandY = 722;
        k.StockX = 290;
        k.StockY = 299;
        k.TrumpX = 320;
        k.TrumpY = 299;
        k.DiscardX = 858;
        k.DiscardY = 358;
        k.BoutX = 600;
        k.BoutY = 336;
        k.MomentX = 885;
        k.MomentY = 780;
        k.StackX = 1206;
        k.StackY = 432;
        k.TopY = 164;
        k.TopPlateY = 222;
        k.TopLeftX = 295;
        k.TopX = 595;
        k.TopRightX = 895;
        k.SideY = 398;
        k.SidePlateY = 566;
        k.LeftX = 128;
        k.RightX = 1062;
        k.PlateW = 190;
        k.Glow = ARGB(165, 224, 160, 48);
        k.PickGlow = ARGB(255, 255, 214, 130);
        k.TargetGlow = ARGB(230, 224, 160, 48);
        k.EdgeTurn = ARGB(255, 224, 160, 48);
        k.EdgeIdle = ARGB(255, 110, 102, 91);
        k.RoleTurn = ARGB(255, 224, 160, 48);
        k.RoleIdle = ARGB(255, 181, 171, 144);
        k.PocketX = 87;
        k.PocketY = 778;
        k.LogShort = 15;
        k.HostLineY = 402;
        k.HostLineY2 = 640;
        k.StackY2 = 665;
        k.StackFits2 = 4;
        k.TileX = 470;
        // Forty units lower than the classic frame's place: the title clear
        // of the cloth's seam.
        k.TileY = 538;
        k.Hot = ARGB(255, 214, 104, 44);
        k.SleightGlow = ARGB(200, 214, 104, 44);
        k.SleightPick = ARGB(255, 240, 140, 70);
        k.BtnBg = ARGB(255, 37, 34, 30);
        k.BtnBgArmed = ARGB(255, 59, 41, 31);
        k.BtnBgSpent = ARGB(255, 26, 24, 21);
        k.BtnEdge = ARGB(255, 77, 70, 61);
        k.BtnInk = ARGB(255, 217, 207, 176);
        k.BtnInkSpent = ARGB(255, 107, 99, 82);
        k.HintInk = ARGB(255, 163, 154, 126);
        k.HintHot = ARGB(255, 232, 148, 90);
        k.TickIdle = ARGB(255, 163, 154, 126);
        k.TickOn = ARGB(255, 224, 160, 48);
        return k;
    }

    protected OZG_TableSpot Spot(float x, float y, float turn, float px, float py, float plateW, float scale)
    {
        OZG_TableSpot s = new OZG_TableSpot();
        s.X = x;
        s.Y = y;
        s.Turn = turn;
        s.PlateX = px;
        s.PlateY = py;
        s.PlateW = plateW;
        s.Scale = scale;
        return s;
    }

    // The places of `k` other seats, in their order round the table from the
    // player's left shoulder over the far side to his right: the seat after
    // his in the ring sits on his left (the ring counts clockwise seen from
    // above, OZG_Ring), and in durak that seat defends against him.
    void Spots(int k, array<ref OZG_TableSpot> into)
    {
        into.Clear();
        if (k <= 0)
            return;
        if (k <= 5)
        {
            array<string> named = new array<string>();
            if (k >= 3)
                named.Insert("left");
            if (k == 2 || k >= 4)
                named.Insert("topLeft");
            if (k == 1 || k == 3 || k == 5)
                named.Insert("top");
            if (k == 2 || k >= 4)
                named.Insert("topRight");
            if (k >= 3)
                named.Insert("right");
            for (int i = 0; i < named.Count(); i++)
                into.Insert(Named(named[i]));
            return;
        }

        // A crowded table: two seats down each side, the rest along the far
        // side; smaller fans, the side seats' plates turned inwards.
        float small = 0.8;
        into.Insert(Spot(LeftX, SideY + 85, -90, LeftX + 150, SideY + 61, PlateW - 20, small));
        into.Insert(Spot(LeftX, SideY - 85, -90, LeftX + 150, SideY - 109, PlateW - 20, small));
        int top = k - 4;
        float a = TopLeftX - 40;
        float b = TopRightX + 40;
        float step = (b - a) / (top - 1);
        float plateW = PlateW;
        if (plateW > step - 10)
            plateW = step - 10;
        float scale = small;
        if (step < 170)
            scale = small * step / 170;
        for (int t = 0; t < top; t++)
            into.Insert(Spot(a + t * step, TopY, 180, a + t * step, TopPlateY, plateW, scale));
        into.Insert(Spot(RightX, SideY - 85, 90, RightX - 150, SideY - 109, PlateW - 20, small));
        into.Insert(Spot(RightX, SideY + 85, 90, RightX - 150, SideY + 61, PlateW - 20, small));
    }

    protected OZG_TableSpot Named(string name)
    {
        if (name == "topLeft")
            return Spot(TopLeftX, TopY, 180, TopLeftX, TopPlateY, PlateW, 1);
        if (name == "topRight")
            return Spot(TopRightX, TopY, 180, TopRightX, TopPlateY, PlateW, 1);
        if (name == "left")
            return Spot(LeftX, SideY, -90, LeftX, SidePlateY, PlateW, 1);
        if (name == "right")
            return Spot(RightX, SideY, 90, RightX, SidePlateY, PlateW, 1);
        return Spot(TopX, TopY, 180, TopX, TopPlateY, PlateW, 1);
    }

    // Where the discards' backs lie round DiscardX/Y: a careless heap, the
    // first eight as the mock-up drew them, eight more over them. (dx, dy,
    // turn); the heap grows by a back for every two cards beaten off.
    static const int HEAP = 16;

    static void HeapAt(int i, out float dx, out float dy, out float turn)
    {
        array<float> xs = {-10.0, 8.0, -4.0, 12.0, -14.0, 4.0, 10.0, -6.0, -6.0, 10.0, -12.0, 4.0, 14.0, -8.0, 2.0, -14.0};
        array<float> ys = {-6.0, 4.0, 10.0, -10.0, 6.0, -4.0, 12.0, -12.0, 8.0, -4.0, -10.0, 12.0, 6.0, 2.0, -12.0, 10.0};
        array<float> ts = {-22.0, 14.0, 31.0, -8.0, 6.0, -30.0, 24.0, -14.0, 18.0, -26.0, 9.0, -15.0, 33.0, -5.0, 22.0, -35.0};
        int k = i % HEAP;
        dx = xs[k];
        dy = ys[k];
        turn = ts[k];
    }
}
