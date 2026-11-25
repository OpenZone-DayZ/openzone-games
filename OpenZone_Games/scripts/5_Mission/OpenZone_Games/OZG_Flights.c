// Cards in the air over the game window, and small shakes of a part of it
// (design §11; card sharp's design §6.1, §10). The window draws what the
// server said at once; these only move pictures over it, so nothing waits for
// them: a click goes through them to the table, and a newer view lands every
// card still in the air before it is drawn (OZG_GameMenu.LandAll).

// Where a card lies in the window: its middle, its size and its turn
// (degrees, clockwise), in layout units of the 1600 x 900 window -- the
// numbers OZG_TableLook gives.
class OZG_CardSpot
{
    float X;
    float Y;
    float W;
    float H;
    float Turn;

    static OZG_CardSpot Make(float x, float y, float w, float h, float turn)
    {
        OZG_CardSpot s = new OZG_CardSpot();
        s.X = x;
        s.Y = y;
        s.W = w;
        s.H = h;
        s.Turn = turn;
        return s;
    }
}

// One card on its way: a frame of ozg_card.layout in the window's layer of
// flights (OZG_Air).
class OZG_Flight
{
    Widget CardFrame;
    ImageWidget FacePic;
    // What the face shows from halfway on; "" keeps the first picture.
    string LandPicture;
    ref OZG_CardSpot FromSpot;
    ref OZG_CardSpot ToSpot;
    float WaitSecs;
    float FlySecs;
    float AgeSecs;
    // The window's key shown when the card lands; "" for none.
    string RevealKey;
    bool Turned;
}

class OZG_Flights
{
    // The side of a card's turning frame, in diagonals of the card
    // (tools/make_layouts.py, card()).
    static const float BOX = 1.1;
    // How much larger a card is halfway, as if lifted off the table.
    static const float LIFT = 0.08;

    protected ref array<ref OZG_Flight> m_Air;

    void OZG_Flights()
    {
        m_Air = new array<ref OZG_Flight>();
    }

    int Count()
    {
        return m_Air.Count();
    }

    // A card frame already made in the layer of flights sets off from `from`
    // after `delay` seconds (hidden until then) and lands on `to` after
    // `seconds` more, eased in and out, a little lifted on its way.
    // `pictureLand`, when not "", is put on its face halfway: a back that
    // turns face up, or a face that goes down. `reveal` comes back from Tick
    // or Finish when it lands.
    OZG_Flight Launch(Widget card, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        OZG_Flight f = new OZG_Flight();
        f.CardFrame = card;
        f.FacePic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace"));
        f.LandPicture = pictureLand;
        f.FromSpot = from;
        f.ToSpot = to;
        f.WaitSecs = delay;
        f.FlySecs = seconds;
        if (f.FlySecs < 0.05)
            f.FlySecs = 0.05;
        f.RevealKey = reveal;
        m_Air.Insert(f);
        card.Show(false);
        return f;
    }

    // `t` of the way, eased: slow away, quick in the middle, slow to land.
    static float EaseOf(float t)
    {
        if (t <= 0)
            return 0;
        if (t >= 1)
            return 1;
        if (t < 0.5)
            return 4 * t * t * t;
        float u = 2 - 2 * t;
        return 1 - u * u * u * 0.5;
    }

    // From one turn to another the short way round, degrees.
    static float TurnBy(float from, float to)
    {
        float d = to - from;
        while (d > 180)
            d = d - 360;
        while (d < -180)
            d = d + 360;
        return d;
    }

    // One frame: every card moved; the ones that landed taken away and their
    // keys put into `landed`. `k` is real pixels in one layout unit.
    void Tick(float dt, float k, array<string> landed)
    {
        for (int i = m_Air.Count() - 1; i >= 0; i--)
        {
            OZG_Flight f = m_Air[i];
            f.AgeSecs = f.AgeSecs + dt;
            if (!f.CardFrame)
            {
                LandAt(i, landed);
                continue;
            }
            float t = (f.AgeSecs - f.WaitSecs) / f.FlySecs;
            if (t < 0)
                continue;
            if (t >= 1)
            {
                LandAt(i, landed);
                continue;
            }
            PutAt(f, t, k);
        }
    }

    // Every card at once where it lands.
    void Finish(array<string> landed)
    {
        for (int i = m_Air.Count() - 1; i >= 0; i--)
            LandAt(i, landed);
    }

    protected void LandAt(int i, array<string> landed)
    {
        OZG_Flight f = m_Air[i];
        if (f.CardFrame)
            f.CardFrame.Unlink();
        if (f.RevealKey != "")
            landed.Insert(f.RevealKey);
        m_Air.RemoveOrdered(i);
    }

    // The card `t` of its way.
    protected void PutAt(OZG_Flight f, float t, float k)
    {
        float e = EaseOf(t);
        OZG_CardSpot a = f.FromSpot;
        OZG_CardSpot b = f.ToSpot;
        float x = a.X + (b.X - a.X) * e;
        float y = a.Y + (b.Y - a.Y) * e;
        float w = a.W + (b.W - a.W) * e;
        float h = a.H + (b.H - a.H) * e;
        float turn = a.Turn + TurnBy(a.Turn, b.Turn) * e;
        float rise = 1 + LIFT * Math.Sin(t * Math.PI);
        float side = BOX * Math.Sqrt(w * w + h * h) * rise;
        float half = side * 0.5;
        f.CardFrame.SetSize(side * k, side * k);
        f.CardFrame.SetPos((x - half) * k, (y - half) * k);
        f.CardFrame.SetRotation(0, 0, turn);
        if (!f.CardFrame.IsVisible())
            f.CardFrame.Show(true);
        if (t < 0.5 || f.Turned || f.LandPicture == "" || !f.FacePic)
            return;
        f.FacePic.LoadImageFile(0, f.LandPicture);
        f.FacePic.SetImage(0);
        f.Turned = true;
    }
}

// Small shakes of parts of the window: a seat's fan, the discards or the
// stock, where a sleight was seen (card sharp's design §6.1). A region is a
// name; the window moves every card it drew in that region by the offset
// asked here, and puts it back when the shake ends.
class OZG_Shakes
{
    static const float SECONDS = 0.45;
    // How far a card goes at the start, layout units, and how fast it goes to
    // and fro.
    static const float SIZE = 4.0;
    static const float HZ = 17.0;

    protected ref array<string> m_Region;
    protected ref array<float> m_Age;

    void OZG_Shakes()
    {
        m_Region = new array<string>();
        m_Age = new array<float>();
    }

    // The region starts shaking, or starts anew when it shakes already.
    void Start(string region)
    {
        int at = m_Region.Find(region);
        if (at >= 0)
        {
            m_Age[at] = 0;
            return;
        }
        m_Region.Insert(region);
        m_Age.Insert(0);
    }

    bool Busy()
    {
        return m_Region.Count() > 0;
    }

    // One frame; the regions whose shake ended go into `ended`.
    void Tick(float dt, array<string> ended)
    {
        for (int i = m_Region.Count() - 1; i >= 0; i--)
        {
            m_Age[i] = m_Age[i] + dt;
            if (m_Age[i] < SECONDS)
                continue;
            ended.Insert(m_Region[i]);
            m_Region.RemoveOrdered(i);
            m_Age.RemoveOrdered(i);
        }
    }

    // Where the region stands now, units off its rest; false when it is not
    // shaking.
    bool Offset(string region, out float dx, out float dy)
    {
        dx = 0;
        dy = 0;
        int at = m_Region.Find(region);
        if (at < 0)
            return false;
        float t = m_Age[at];
        float fade = 1 - t / SECONDS;
        float size = SIZE * fade * fade;
        float phase = t * HZ * 2 * Math.PI;
        dx = size * Math.Sin(phase);
        dy = size * 0.5 * Math.Sin(phase * 1.3 + 1);
        return true;
    }

    void Clear()
    {
        m_Region.Clear();
        m_Age.Clear();
    }
}
