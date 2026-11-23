// Table Games Probe -- the client half: the live hologram, asked which of its
// checks refused, and the ring of places drawn in the world.
//
// The server's verb sends an op to the first player's client (TGP_Rpc.ASK) and
// the client says one line back (TGP_Rpc.TELL), which `tgp op=told` returns.

class TGP_Rpc
{
    static const int ASK = 87210011;      // server -> client: op, text
    static const int TELL = 87210012;     // client -> server: one line
}

class TGP_Holo
{
    // A virtual aim. The hologram's ray then goes from the player's head to
    // this point instead of along the camera, so a table top can be aimed at
    // exactly, by its coordinates. Everything after the ray is the game's own.
    static bool s_AimOn;
    static vector s_AimAt;

    // What the checks said on the last frame of placing.
    static string s_Last;

    // The stand-in's model is a burlap sack whose collision box stands 0.56 m
    // tall on a 0.43 by 0.10 m foot. A mat is flat: while this is on, the
    // hologram's box is the one a real mat would have, 0.5 by 0.35 m and 3 cm
    // thick, whatever the borrowed model says.
    static bool s_FlatBox = true;
}

// The stand-in deck: a kit in the game's sense. Its hologram is the config
// class named by projectionTypename, and laying it out leaves a mat.
class TGP_Deck extends ItemBase
{
    override bool IsBasebuildingKit()
    {
        return true;
    }

    override bool IsDeployable()
    {
        return true;
    }

    override void SetActions()
    {
        super.SetActions();

        AddAction(ActionTogglePlaceObject);
        AddAction(ActionDeployObject);
    }

    override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
    {
        super.OnPlacementComplete(player, position, orientation);

        if (!GetGame().IsServer())
            return;

        Object mat = GetGame().CreateObjectEx("TGP_Mat", position, ECE_PLACE_ON_SURFACE | ECE_NOLIFETIME);
        if (mat)
        {
            mat.SetPosition(position);
            mat.SetOrientation(orientation);
        }
    }
}

modded class Hologram
{
    override protected vector GetProjectionEntityPosition(PlayerBase player)
    {
        if (!TGP_Holo.s_AimOn)
            return super.GetProjectionEntityPosition(player);

        // The game's own steps with one difference: where the ray comes from.
        float minProjectionDistance = SMALL_PROJECTION_RADIUS;
        float maxProjectionDistance = SMALL_PROJECTION_RADIUS * 2;
        float projectionRadius = GetProjectionRadius();
        if (projectionRadius >= SMALL_PROJECTION_RADIUS)
        {
            minProjectionDistance = projectionRadius;
            maxProjectionDistance = Math.Clamp(projectionRadius * 2, SMALL_PROJECTION_RADIUS, LARGE_PROJECTION_DISTANCE_LIMIT);
        }

        m_ContactDir = vector.Zero;

        vector from = player.GetPosition();
        from[1] = from[1] + 1.6;
        vector dir = TGP_Holo.s_AimAt - from;
        float reach = dir.Length() + 0.5;
        dir.Normalize();
        vector to = from + dir * reach;
        vector contactPosition;
        set<Object> hitObjects = new set<Object>();

        DayZPhysics.RaycastRV(from, to, contactPosition, m_ContactDir, m_ContactComponent, hitObjects, player, m_Projection, false, false, ObjIntersectFire);

        if (!CfgGameplayHandler.GetDisableIsCollidingBBoxCheck())
        {
            if (hitObjects.Count() > 0)
            {
                if (hitObjects[0].IsInherited(InventoryItem))
                    contactPosition = hitObjects[0].GetPosition();
            }
        }

        bool isFloating = SetHologramPosition(player.GetPosition(), minProjectionDistance, maxProjectionDistance, contactPosition);
        SetIsFloating(isFloating);
        m_FromAdjusted = from;

        return contactPosition;
    }

    override protected void GetProjectionCollisionBox(out vector min_max[2])
    {
        if (TGP_Holo.s_FlatBox && m_Projection && m_Projection.IsKindOf("TGP_Mat"))
        {
            min_max[0] = Vector(-0.25, 0, -0.175);
            min_max[1] = Vector(0.25, 0.03, 0.175);
            return;
        }

        super.GetProjectionCollisionBox(min_max);
    }

    override void EvaluateCollision(ItemBase action_item = null)
    {
        super.EvaluateCollision(action_item);

        if (!m_Projection)
            return;
        if (!m_Projection.IsKindOf("TGP_Mat"))
            return;

        // Asked again one by one: retail names none of them, and what is
        // wanted here is WHICH one refused.
        vector p = GetProjectionPosition();
        string s = "at=" + TGP_Out.F2(p[0]) + " " + TGP_Out.F2(p[1]) + " " + TGP_Out.F2(p[2]);
        s = s + " refused=" + TGP_Out.B(IsColliding()) + " why:";

        if (!m_Player.CanPlaceItem(m_Projection))
            s = s + " player-cannot";
        if (IsFloating())
            s = s + " floating";
        if (IsHidden())
            s = s + " hidden";
        if (IsCollidingBBox(action_item))
            s = s + " bbox";
        if (IsCollidingGeometryProxy(action_item))
            s = s + " proxy";
        if (IsCollidingPlayer())
            s = s + " player";
        if (IsClippingRoof())
            s = s + " roof";
        if (!IsBaseViable())
            s = s + " base";
        if (IsCollidingGPlot())
            s = s + " gplot";
        if (IsCollidingZeroPos())
            s = s + " zero";
        if (IsCollidingAngle())
            s = s + " angle";
        if (!IsPlacementPermitted())
            s = s + " permit";
        if (!HeightPlacementCheck())
            s = s + " height";
        if (IsUnderwater())
            s = s + " water";
        if (IsInTerrain())
            s = s + " terrain";

        // What the mat lies on.
        vector from = p;
        from[1] = from[1] + 0.3;
        vector to = p;
        to[1] = to[1] - 0.6;
        vector cpos;
        vector cdir;
        int comp;
        set<Object> under = new set<Object>();
        string holder = "nothing";
        if (DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, under, null, m_Projection, false, false, ObjIntersectFire))
        {
            holder = "ground";
            if (under.Count() > 0)
            {
                Object o = under[0];
                holder = o.GetType() + "/b" + TGP_Out.B(o.IsBuilding()) + "p" + TGP_Out.B(o.IsPlainObject());
                holder = holder + "i" + TGP_Out.B(o.IsInherited(InventoryItem));
            }
        }

        vector ypr = GetProjectionOrientation();
        s = s + " on=" + holder + " slopeTol=" + m_SlopeTolerance.ToString();
        s = s + " pitch=" + TGP_Out.F2(ypr[1]) + " roll=" + TGP_Out.F2(ypr[2]);
        s = s + " " + TgpBaseParts() + " " + TgpBoxHits(action_item);
        TGP_Holo.s_Last = s;
    }

    // The game's base test taken apart (IsBaseViable): are the four corners
    // over one thing (i), is that thing static (s), do their heights agree
    // with the projection's within the slope tolerance (f); `drop` is the
    // worst of the four differences.
    protected string TgpBaseParts()
    {
        vector corner[4];
        corner[0] = GetLeftCloseProjectionVector();
        corner[1] = GetRightCloseProjectionVector();
        corner[2] = GetLeftFarProjectionVector();
        corner[3] = GetRightFarProjectionVector();

        vector pp = GetProjectionPosition();
        vector from;
        vector to;
        vector cpos;
        vector cdir;
        int comp;
        set<Object> res = new set<Object>();
        Object first;
        Object got;
        bool intact = true;
        float worst = 0;
        float d;

        for (int k = 0; k < 4; k++)
        {
            from = m_Projection.CoordToParent(corner[k]) + PLACEMENT_RC_START_OFFSET;
            to = from + PLACEMENT_RC_END_OFFSET;
            res.Clear();
            cpos = pp;
            DayZPhysics.RaycastRV(from, to, cpos, cdir, comp, res, null, m_Projection, false, false, ObjIntersectFire);

            got = null;
            if (res.Count() > 0)
                got = res[res.Count() - 1];

            if (k == 0)
                first = got;
            else if (got != first)
                intact = false;

            d = Math.AbsFloat(pp[1] - cpos[1]);
            if (d > worst)
                worst = d;
        }

        bool rooted = IsBaseStatic(first);
        bool flat = worst < m_SlopeTolerance;
        string parts = "base:i" + TGP_Out.B(intact) + "s" + TGP_Out.B(rooted) + "f" + TGP_Out.B(flat);
        return parts + " drop=" + TGP_Out.F2(worst);
    }

    // The game's box test again (IsCollidingBBox), to name what it touched.
    protected string TgpBoxHits(ItemBase action_item)
    {
        vector minMax[2];
        GetProjectionCollisionBox(minMax);

        vector lift = "0 0.05 0";
        lift[1] = lift[1] + (minMax[1][1] - minMax[0][1]) * 0.5;
        vector center = m_Projection.GetPosition() + lift;
        vector edge = GetCollisionBoxSize(minMax);

        array<Object> excluded = new array<Object>();
        array<Object> collided = new array<Object>();
        excluded.Insert(m_Projection);
        excluded.Insert(m_Player);
        if (action_item)
            excluded.Insert(action_item);

        g_Game.IsBoxCollidingGeometry(center, GetProjectionOrientation(), edge, ObjIntersectFire, ObjIntersectGeom, excluded, collided);

        int n = collided.Count();
        string s = "box=" + TGP_Out.F2(edge[0]) + "x" + TGP_Out.F2(edge[1]) + "x" + TGP_Out.F2(edge[2]) + " touched=" + n.ToString();
        if (n > 0)
        {
            Object t = collided.Get(0);
            if (t)
                s = s + ":" + t.GetType();
        }
        return s;
    }
}

class TGP_Client
{
    protected static ref array<Shape> s_Shapes = new array<Shape>;

    static void Tell(PlayerBase player, string text)
    {
        ScriptRPC rpc = new ScriptRPC();
        rpc.Write(text);
        rpc.Send(player, TGP_Rpc.TELL, true, null);
    }

    static void Wipe()
    {
        Shape one;
        int n = s_Shapes.Count();
        for (int i = 0; i < n; i++)
        {
            one = s_Shapes.Get(i);
            if (one)
                Debug.RemoveShape(one);
        }
        s_Shapes.Clear();
    }

    protected static bool Numbers3(string text, out vector v)
    {
        TStringArray parts = new TStringArray;
        text.Split(" ", parts);
        array<float> nums = new array<float>;
        string part;
        int n = parts.Count();
        for (int i = 0; i < n; i++)
        {
            part = parts.Get(i);
            if (part != "")
                nums.Insert(part.ToFloat());
        }

        if (nums.Count() != 3)
            return false;

        v = Vector(nums.Get(0), nums.Get(1), nums.Get(2));
        return true;
    }

    protected static void DrawRing(vector anchor)
    {
        Wipe();

        int green = 0x6600FF00;
        int yellow = 0xAAFFFF00;
        float bodyH = 1.0;
        float bodyR = 0.35;
        if (TGP_Seat.s_Standing)
        {
            bodyH = 1.7;
            bodyR = 0.25;
        }

        s_Shapes.Insert(Debug.DrawSphere(anchor, 0.08, yellow, ShapeFlags.NOOUTLINE));

        vector at;
        int n = TGP_Seat.s_Places.Count();
        for (int i = 0; i < n; i++)
        {
            at = TGP_Seat.s_Places.Get(i);
            at[1] = at[1] + bodyH * 0.5;
            s_Shapes.Insert(Debug.DrawCylinder(at, bodyR, bodyH, green, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE));
        }
    }

    // One op from the server; always answers with one line.
    static void Answer(PlayerBase player, string op, string text)
    {
        vector v;
        string said = op + ": ";

        if (op == "state")
        {
            vector pos = player.GetPosition();
            vector ori = player.GetOrientation();
            vector cam = GetGame().GetCurrentCameraPosition();
            vector look = GetGame().GetCurrentCameraDirection();
            EntityAI inHands = player.GetHumanInventory().GetEntityInHands();

            said = said + "pos=" + TGP_Out.F2(pos[0]) + " " + TGP_Out.F2(pos[1]) + " " + TGP_Out.F2(pos[2]);
            said = said + " yaw=" + TGP_Out.F2(ori[0]);
            said = said + " cam=" + TGP_Out.F2(cam[0]) + " " + TGP_Out.F2(cam[1]) + " " + TGP_Out.F2(cam[2]);
            said = said + " look=" + TGP_Out.F2(look[0]) + " " + TGP_Out.F2(look[1]) + " " + TGP_Out.F2(look[2]);
            if (inHands)
                said = said + " hands=" + inHands.GetType();
            else
                said = said + " hands=none";
            said = said + " placing=" + TGP_Out.B(player.IsPlacingLocal());
        }
        else if (op == "seat")
        {
            if (Numbers3(text, v))
            {
                said = said + TGP_Seat.SeatRun(v, "client", 20);
                DrawRing(v);
            }
            else
            {
                said = said + "text must be 'x y z'";
            }
        }
        else if (op == "aim")
        {
            if (Numbers3(text, v))
            {
                TGP_Holo.s_AimAt = v;
                TGP_Holo.s_AimOn = true;
                said = said + "on";
            }
            else
            {
                TGP_Holo.s_AimOn = false;
                said = said + "off";
            }
        }
        else if (op == "aimplace")
        {
            // Aim, and start placing if that is not going yet: what the sweep sends.
            if (!Numbers3(text, v))
            {
                said = said + "text must be 'x y z'";
            }
            else
            {
                TGP_Holo.s_AimAt = v;
                TGP_Holo.s_AimOn = true;
                TGP_Holo.s_Last = "";

                if (player.IsPlacingLocal())
                {
                    said = said + "aimed";
                }
                else
                {
                    ItemBase held = ItemBase.Cast(player.GetHumanInventory().GetEntityInHands());
                    if (held)
                    {
                        player.PlacingStartLocal(held);
                        said = said + "placing";
                    }
                    else
                    {
                        said = said + "nothing in hands";
                    }
                }
            }
        }
        else if (op == "place")
        {
            ItemBase item = ItemBase.Cast(player.GetHumanInventory().GetEntityInHands());
            if (!item)
            {
                said = said + "nothing in hands";
            }
            else if (player.IsPlacingLocal())
            {
                said = said + "already placing";
            }
            else
            {
                TGP_Holo.s_Last = "";
                player.PlacingStartLocal(item);
                said = said + "started with " + item.GetType();
            }
        }
        else if (op == "unplace")
        {
            if (player.IsPlacingLocal())
                player.PlacingCancelLocal();
            said = said + "stopped";
        }
        else if (op == "holo")
        {
            if (!player.IsPlacingLocal())
                said = said + "not placing";
            else if (TGP_Holo.s_Last == "")
                said = said + "no frame yet";
            else
                said = said + TGP_Holo.s_Last;
        }
        else if (op == "wipe")
        {
            Wipe();
            said = said + "done";
        }
        else if (op == "box")
        {
            // text "flat" or "model": which collision box the hologram uses.
            bool flatWanted = text != "model";
            TGP_Holo.s_FlatBox = flatWanted;
            said = said + "flat=" + TGP_Out.B(flatWanted);
        }
        else
        {
            said = said + "unknown client op";
        }

        Tell(player, said);
    }
}

modded class PlayerBase
{
    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        if (rpc_type == TGP_Rpc.ASK)
        {
            if (GetGame().IsDedicatedServer())
                return;

            string op;
            string text;
            if (!ctx.Read(op))
                return;
            if (!ctx.Read(text))
                return;

            TGP_Client.Answer(this, op, text);
            return;
        }

        if (rpc_type == TGP_Rpc.TELL)
        {
            if (!GetGame().IsServer())
                return;

            string line;
            if (ctx.Read(line))
                TGP_Verb.Told(line);
        }
    }
}
