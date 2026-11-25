// What lies over a card where the window lays it, and a mark's box moved to
// the part of the card that is seen in play (the owner's word, 2026-11-25: a
// reader's pencil and a noticer's speck are never under another card). Boxes
// are in pixels of a 200 x 280 card (OZG_MarkPlaces); the sign in a back's
// pattern is not moved.
//
// The window's fans all spread the same way in a card's own frame: the next
// card lies to its right, turned on by the fan's step about a pivot below the
// cards, so the left side of a card is seen, wider at its top. The trump lies
// turned under the stock with its top end out. An attack card has the card
// that beat it laid over it, a little down, to the right and turned.
class OZG_Cover
{
    static const int NONE = 0;
    static const int FAN  = 1;
    static const int TOP  = 2;
    static const int BOUT = 3;

    // A few pixels clear of the other card's edge.
    static const float PAD = 3;

    int   CoverKind;
    // The card's size, units.
    float CardWide;
    float CardHigh;
    // FAN: the pivot's distance below the card's middle, units; the angle to
    // the next card, radians.
    float PivotR;
    float StepAngle;
    // TOP: the part of the height seen, from the top.
    float SeenPart;
    // BOUT: the middle of the card over it, units from this card's middle,
    // and its turn, degrees clockwise.
    float OverX;
    float OverY;
    float OverTurn;

    static OZG_Cover MakeFan(float r, float stepDegrees, float w, float h)
    {
        OZG_Cover c = new OZG_Cover();
        c.CoverKind = FAN;
        c.PivotR = r;
        c.StepAngle = stepDegrees * Math.DEG2RAD;
        c.CardWide = w;
        c.CardHigh = h;
        return c;
    }

    static OZG_Cover MakeTop(float seen)
    {
        OZG_Cover c = new OZG_Cover();
        c.CoverKind = TOP;
        c.SeenPart = seen;
        return c;
    }

    static OZG_Cover MakeBout(float ox, float oy, float turn, float w, float h)
    {
        OZG_Cover c = new OZG_Cover();
        c.CoverKind = BOUT;
        c.OverX = ox;
        c.OverY = oy;
        c.OverTurn = turn;
        c.CardWide = w;
        c.CardHigh = h;
        return c;
    }

    // FAN: how much of the card's width, in pixels from its left edge, is
    // seen at the height `py` (pixels from its top).
    protected float SeenWidth(float py)
    {
        float y = (py / OZG_MarkPlaces.CARD_H - 0.5) * CardHigh;
        float units = (PivotR - y) * StepAngle;
        return units / CardWide * OZG_MarkPlaces.CARD_W;
    }

    // BOUT: is the point (pixels of this card) under the card over it?
    protected bool Under(float px, float py)
    {
        float x = (px / OZG_MarkPlaces.CARD_W - 0.5) * CardWide - OverX;
        float y = (py / OZG_MarkPlaces.CARD_H - 0.5) * CardHigh - OverY;
        float t = -OverTurn * Math.DEG2RAD;
        float c = Math.Cos(t);
        float s = Math.Sin(t);
        float qx = x * c - y * s;
        float qy = x * s + y * c;
        return Math.AbsFloat(qx) <= CardWide * 0.5 + PAD && Math.AbsFloat(qy) <= CardHigh * 0.5 + PAD;
    }

    // Is the box seen whole?
    bool Shows(float x, float y, float w, float h)
    {
        if (CoverKind == FAN)
            return x + w <= SeenWidth(y + h) - PAD;
        if (CoverKind == TOP)
            return y + h <= SeenPart * OZG_MarkPlaces.CARD_H - PAD;
        if (CoverKind == BOUT)
        {
            if (Under(x, y) || Under(x + w, y) || Under(x, y + h) || Under(x + w, y + h))
                return false;
            return !Under(x + w * 0.5, y + h * 0.5);
        }
        return true;
    }

    // The box where it is seen: as it is, else mirrored across the card, up
    // and down, or both -- the sign's place kept as far as it can be -- else
    // as near the seen part as it goes.
    void PutSeen(inout int x, inout int y, int w, int h)
    {
        if (CoverKind == NONE || Shows(x, y, w, h))
            return;
        int mx = OZG_MarkPlaces.CARD_W - x - w;
        int my = OZG_MarkPlaces.CARD_H - y - h;
        if (Shows(mx, y, w, h))
        {
            x = mx;
            return;
        }
        if (Shows(x, my, w, h))
        {
            y = my;
            return;
        }
        if (Shows(mx, my, w, h))
        {
            x = mx;
            y = my;
            return;
        }
        if (CoverKind == FAN)
        {
            float room = SeenWidth(y + h) - PAD - w;
            if (room < PAD)
                room = PAD;
            if (room < x)
                x = room;
            return;
        }
        if (CoverKind == TOP)
        {
            float top = SeenPart * OZG_MarkPlaces.CARD_H - PAD - h;
            if (top < PAD)
                top = PAD;
            if (top < y)
                y = top;
            return;
        }
        // BOUT: the top margin over the card's left, which the card over it
        // leaves.
        y = 0;
        if (!Shows(x, y, w, h))
            x = 0;
    }
}
