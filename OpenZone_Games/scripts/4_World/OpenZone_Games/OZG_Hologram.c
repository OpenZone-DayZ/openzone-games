// What the mod adds to the game's hologram, FOR ITS OWN PROJECTION ONLY.
//
// 1. The mat's box is the mod's: 0.5 by 0.35 m and 3 cm thick, whatever the
//    model is.
// 2. The water check is answered by the mod: sea level and water depth at the
//    four corners, without the surface test that refuses a dry desk (measured
//    2026-11-23 on StaticObj_Furniture_office_desk).
// 3. Two refusals of its own: another place too close, and fewer than two
//    seats.
// 4. The reason is kept, as a string table key: a client shows it while
//    aiming, the server says it when it disagrees with the placer's client.
//
// The seat count comes from the ring the server runs (OZG_Ring), on the client
// at most four times a second and only where the spot is otherwise accepted.
modded class Hologram
{
    protected ref OZG_RingResult m_OZG_Ring;
    protected vector m_OZG_RingAt;
    protected int m_OZG_RingMode;
    protected int m_OZG_RingWhen;
    protected bool m_OZG_Told;

    bool OZG_IsMine()
    {
        return m_Projection && m_Projection.IsInherited(OZG_PlacePlacing);
    }

    // The ring the last judgement ran, for whoever makes the place.
    OZG_RingResult OZG_LastRing()
    {
        return m_OZG_Ring;
    }

    override protected void GetProjectionCollisionBox(out vector min_max[2])
    {
        if (OZG_IsMine())
        {
            min_max[0] = Vector(-OZG_Const.MAT_HALF_X, 0, -OZG_Const.MAT_HALF_Z);
            min_max[1] = Vector(OZG_Const.MAT_HALF_X, OZG_Const.MAT_HEIGHT, OZG_Const.MAT_HALF_Z);
            return;
        }
        super.GetProjectionCollisionBox(min_max);
    }

    override bool IsUnderwater()
    {
        if (!OZG_IsMine())
            return super.IsUnderwater();
        if (CfgGameplayHandler.GetDisableIsUnderwaterCheck())
            return false;

        vector c0 = m_Projection.CoordToParent(GetLeftCloseProjectionVector());
        vector c1 = m_Projection.CoordToParent(GetRightCloseProjectionVector());
        vector c2 = m_Projection.CoordToParent(GetLeftFarProjectionVector());
        vector c3 = m_Projection.CoordToParent(GetRightFarProjectionVector());
        float sea = g_Game.SurfaceGetSeaLevelMax() + 0.03;
        if (c0[1] < sea || c1[1] < sea)
            return true;
        if (c2[1] < sea || c3[1] < sea)
            return true;
        if (g_Game.GetWaterDepth(c0) > 0 || g_Game.GetWaterDepth(c1) > 0)
            return true;
        if (g_Game.GetWaterDepth(c2) > 0 || g_Game.GetWaterDepth(c3) > 0)
            return true;
        return false;
    }

    // Which of the game's own checks refused, in the game's order.
    protected string OZG_GameWhy(ItemBase action_item)
    {
        if (!m_Player.CanPlaceItem(m_Projection))
            return OZG_Const.LAY_NO_PLAYER;
        if (IsFloating())
            return OZG_Const.LAY_NO_REACH;
        if (IsHidden())
            return OZG_Const.LAY_NO_PLAYER;
        if (IsCollidingBBox(action_item))
            return OZG_Const.LAY_NO_WAY;
        if (IsCollidingGeometryProxy(action_item))
            return OZG_Const.LAY_NO_WAY;
        if (IsCollidingPlayer())
            return OZG_Const.LAY_NO_WAY;
        if (IsClippingRoof())
            return OZG_Const.LAY_NO_WAY;
        if (!IsBaseViable())
            return OZG_Const.LAY_NO_BASE;
        if (IsCollidingGPlot())
            return OZG_Const.LAY_NO_WAY;
        if (IsCollidingZeroPos())
            return OZG_Const.LAY_NO_PERMIT;
        if (IsCollidingAngle())
            return OZG_Const.LAY_NO_BASE;
        if (!IsPlacementPermitted())
            return OZG_Const.LAY_NO_PERMIT;
        if (!HeightPlacementCheck())
            return OZG_Const.LAY_NO_HEIGHT;
        if (IsUnderwater())
            return OZG_Const.LAY_NO_WATER;
        if (IsInTerrain())
            return OZG_Const.LAY_NO_BASE;
        return OZG_Const.LAY_NO_PERMIT;
    }

    // The mod's own two refusals; "" when the spot is good.
    protected string OZG_OwnWhy(int mode, bool server)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        vector p = GetProjectionPosition();

        if (OZG_Place.NearOther(p, cfg.PlaceSpacing, null))
            return OZG_Const.LAY_NO_SPACING;

        int now = GetGame().GetTime();
        bool fresh = false;
        if (m_OZG_Ring && m_OZG_RingMode == mode)
        {
            if (vector.Distance(p, m_OZG_RingAt) < 0.01)
                fresh = true;
            else if (!server && now - m_OZG_RingWhen < 250)
                fresh = true;
        }
        if (!fresh)
        {
            m_OZG_Ring = OZG_Ring.Compute(p, mode, m_Player.GetPosition(), m_Projection);
            m_OZG_RingAt = p;
            m_OZG_RingMode = mode;
            m_OZG_RingWhen = now;
        }

        if (m_OZG_Ring.Count() < 2)
            return OZG_Const.LAY_NO_ROOM;
        return "";
    }

    override void EvaluateCollision(ItemBase action_item = null)
    {
        super.EvaluateCollision(action_item);

        if (!OZG_IsMine() || !m_Player)
            return;

        bool server = GetGame().IsDedicatedServer();
        int mode = OZG_Aim.s_Mode;
        if (server)
            mode = OZG_Aim.ServerMode(m_Player);

        string why = "";
        if (IsColliding())
            why = OZG_GameWhy(action_item);
        else
            why = OZG_OwnWhy(mode, server);

        if (why != "")
            SetIsColliding(true);

        if (server)
        {
            // The placer's client saw a good spot, or the deploy would not
            // have begun: the server says why it sees another, once.
            if (why != "" && !m_OZG_Told)
            {
                m_OZG_Told = true;
                PlayerIdentity id = m_Player.GetIdentity();
                if (id)
                    OZ_Rpc.Notice(id, OZG_Const.NOTICE_LAY, false, why);
                OZG_Log.Dbg("lay out refused by the server for " + OZG_Who.Uid(m_Player) + ": " + why);
            }
            return;
        }

        // A comparison is worked out in a local first, never straight into a
        // field.
        bool refused = why != "";
        bool guessed = mode == OZG_Const.MODE_AUTO;
        OZG_Aim.s_Stamp = GetGame().GetTime();
        OZG_Aim.s_Refused = refused;
        OZG_Aim.s_Why = why;
        OZG_Aim.s_Seats = 0;
        OZG_Aim.s_Layout = OZG_Const.MODE_FLOOR;
        OZG_Aim.s_Guessed = guessed;
        bool counted = why == "" || why == OZG_Const.LAY_NO_ROOM;
        if (counted && m_OZG_Ring)
        {
            OZG_Aim.s_Seats = m_OZG_Ring.Count();
            OZG_Aim.s_Layout = m_OZG_Ring.LayMode;
        }
    }
}
