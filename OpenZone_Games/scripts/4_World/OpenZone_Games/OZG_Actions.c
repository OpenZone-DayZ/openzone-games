// The actions of OpenZone Games.
//
// Laying out is the game's own deploy with one thing added: the mode of the
// seats the placer chose while aiming travels with it. Everything else is an
// interaction with the place, a networked thing, so no key of the mod's own is
// needed: the server decides what the interaction means for this player.

// What this client knows about its own seat and the marks it was shown. The
// actions word themselves from it; the server never trusts it.
class OZG_Mine
{
    static string s_PlaceId;      // where this client holds a seat, "" nowhere
    static int    s_Seat = -1;

    static string s_MarkPlace;    // whose seats are marked now
    static int    s_MarkUntil;    // ms
    static float  s_Reach = 1.0;
    static ref array<vector> s_MarkAt = new array<vector>();

    static void Forget()
    {
        s_PlaceId = "";
        s_Seat = -1;
        s_MarkPlace = "";
        s_MarkUntil = 0;
        s_MarkAt.Clear();
    }

    static bool SeatedAt(OZG_Place place)
    {
        if (!place || s_PlaceId == "" || s_Seat < 0)
            return false;
        return place.OZG_Id() == s_PlaceId;
    }

    // Does the player stand on one of the marks shown for this place.
    static bool OnMark(OZG_Place place, vector at)
    {
        if (!place || s_MarkPlace == "" || place.OZG_Id() != s_MarkPlace)
            return false;
        if (GetGame().GetTime() > s_MarkUntil)
            return false;
        for (int i = 0; i < s_MarkAt.Count(); i++)
        {
            if (vector.Distance(at, s_MarkAt[i]) <= s_Reach)
                return true;
        }
        return false;
    }
}

class OZG_LayOutReceive : PlaceObjectActionReciveData
{
    int m_OZG_Mode;
    int m_OZG_Layout;
}

class OZG_ActionLayOut : ActionDeployObject
{
    void OZG_ActionLayOut()
    {
        m_Text = "#STR_OZG_ACTION_LAY_OUT";
    }

    // The game picks a deploy's animation by the item's itemBehaviour
    // (actiondeployobject.c:306). A deck has none (m_ItemBehaviour -1), so the
    // command stayed 0, CMD_ACTIONFB_DRINK, a prone one: the placer lay down
    // and drank. A deck is laid out with the hands, the way a garden slot is
    // fertilised (actionfertilizeslot.c): the full-body interaction.
    override void SetupAnimation(ItemBase item)
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONFB_INTERACT;
    }

    // Standing for a game played standing, crouched for one sat at or played
    // on the floor. A full-body action is not hidden by its stance mask
    // (actionbase.c:931): the mask is the stance the engine takes for it
    // (animatedactionbase.c:343).
    override protected int GetStanceMaskEx(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (OZG_Aim.LaysStanding(player))
            return DayZPlayerConstants.STANCEMASK_ERECT;
        return DayZPlayerConstants.STANCEMASK_CROUCH;
    }

    override void WriteToContext(ParamsWriteContext ctx, ActionData action_data)
    {
        super.WriteToContext(ctx, action_data);
        ctx.Write(OZG_Aim.s_Mode);
        ctx.Write(OZG_Aim.s_Layout);
    }

    override bool ReadFromContext(ParamsReadContext ctx, out ActionReciveData action_recive_data)
    {
        if (!action_recive_data)
            action_recive_data = new OZG_LayOutReceive;
        if (!super.ReadFromContext(ctx, action_recive_data))
            return false;
        int mode = 0;
        if (!ctx.Read(mode))
            return false;
        int layout = 0;
        if (!ctx.Read(layout))
            return false;
        OZG_LayOutReceive mine = OZG_LayOutReceive.Cast(action_recive_data);
        if (mine)
        {
            mine.m_OZG_Mode = mode;
            mine.m_OZG_Layout = layout;
        }
        return true;
    }

    override void HandleReciveData(ActionReciveData action_recive_data, ActionData action_data)
    {
        super.HandleReciveData(action_recive_data, action_data);
        OZG_LayOutReceive mine = OZG_LayOutReceive.Cast(action_recive_data);
        if (mine && action_data && action_data.m_Player)
        {
            OZG_Aim.ServerSetMode(action_data.m_Player, mine.m_OZG_Mode);
            OZG_Aim.ServerSetLayout(action_data.m_Player, mine.m_OZG_Layout);
        }
    }

    // The game deletes a kit it has placed, here (actiondeployobject.c:230).
    // A deck is not spent by being laid out: it is on its way into the place,
    // or still in the placer's hands when the server refused the spot. For
    // the length of this one call it is no kit.
    override void OnEndServer(ActionData action_data)
    {
        OZG_Deck_Base deck;
        if (action_data)
            deck = OZG_Deck_Base.Cast(action_data.m_MainItem);
        if (deck)
            deck.OZG_Spare(true);
        super.OnEndServer(action_data);
        if (deck)
            deck.OZG_Spare(false);
    }
}

// One base for the three interactions with a place.
class OZG_ActionPlace : ActionInteractBase
{
    void OZG_ActionPlace()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINone();
        // A seat is up to 1.6 m from the mat, and the eyes are above it.
        m_ConditionTarget = new CCTCursor(3.2);
    }

    // Several players use one place at once.
    override bool IsLockTargetOnUse()
    {
        return false;
    }

    protected OZG_Place PlaceOf(ActionTarget target)
    {
        if (!target)
            return null;
        return OZG_Place.Cast(target.GetObject());
    }

    protected OZG_Place PlaceOfData(ActionData action_data)
    {
        if (!action_data || !action_data.m_Target)
            return null;
        return OZG_Place.Cast(action_data.m_Target.GetObject());
    }
}

// "Sit down to the game", then "Sit here"; "To the game" for a player who
// holds a seat at this place.
class OZG_ActionSit : OZG_ActionPlace
{
    void OZG_ActionSit()
    {
        m_Text = "#STR_OZG_ACTION_SIT";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        return PlaceOf(target) != null;
    }

    override void OnActionInfoUpdate(PlayerBase player, ActionTarget target, ItemBase item)
    {
        OZG_Place place = PlaceOf(target);
        if (!place)
            return;
        if (OZG_Mine.SeatedAt(place))
        {
            m_Text = "#STR_OZG_ACTION_TO_GAME";
            return;
        }
        if (OZG_Mine.OnMark(place, player.GetPosition()))
        {
            m_Text = "#STR_OZG_ACTION_SIT_HERE";
            return;
        }
        string text = "#STR_OZG_ACTION_SIT";
        text = text + " (" + place.OZG_Taken().ToString() + "/" + place.OZG_Total().ToString() + ")";
        m_Text = text;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        OZG_Place place = PlaceOfData(action_data);
        if (place && action_data.m_Player)
            OZG_Sessions.SitAction(place, action_data.m_Player);
    }
}

// "Watch": for anyone without a seat here.
class OZG_ActionWatch : OZG_ActionPlace
{
    void OZG_ActionWatch()
    {
        m_Text = "#STR_OZG_ACTION_WATCH";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        OZG_Place place = PlaceOf(target);
        if (!place)
            return false;
        if (GetGame().IsDedicatedServer())
            return true;
        return !OZG_Mine.SeatedAt(place);
    }

    override void OnExecuteServer(ActionData action_data)
    {
        OZG_Place place = PlaceOfData(action_data);
        if (place && action_data.m_Player)
            OZG_Sessions.WatchAction(place, action_data.m_Player);
    }
}

// "Gather the deck": anyone may fold a place where no seat is taken, kept or
// left with cards.
class OZG_ActionGather : OZG_ActionPlace
{
    void OZG_ActionGather()
    {
        m_Text = "#STR_OZG_ACTION_GATHER";
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        OZG_Place place = PlaceOf(target);
        if (!place)
            return false;
        if (GetGame().IsDedicatedServer())
            return true;
        return place.OZG_Busy() == 0;
    }

    override void OnExecuteServer(ActionData action_data)
    {
        OZG_Place place = PlaceOfData(action_data);
        if (place && action_data.m_Player)
            OZG_Sessions.GatherAction(place, action_data.m_Player);
    }
}

// A seated player faces the mat. Where a body faces is its own client's word
// (the server's SetOrientation does not turn a player), so the client turns it:
// one frame of the heading model with the wanted angle.
modded class PlayerBase
{
    protected bool m_OZG_Turn;
    protected float m_OZG_TurnYaw;

    // yaw in degrees, clockwise from north, as an orientation's first number.
    void OZG_Face(float yaw)
    {
        m_OZG_Turn = true;
        m_OZG_TurnYaw = yaw;
    }

    // A player who is hurt has to be able to answer: the game's window
    // closes, as the game's own map does on the same call.
    override void OnPlayerRecievedHit()
    {
        super.OnPlayerRecievedHit();
        OZG_PlaceWatch.Hit(this);
    }

    override bool HeadingModel(float pDt, SDayZPlayerHeadingModel pModel)
    {
        if (m_OZG_Turn)
        {
            m_OZG_Turn = false;
            // The heading counts the other way round and in radians.
            float heading = -m_OZG_TurnYaw * Math.DEG2RAD;
            pModel.m_fHeadingAngle = heading;
            pModel.m_fOrientationAngle = heading;
            return true;
        }
        return super.HeadingModel(pDt, pModel);
    }
}

// An action nobody registers is dropped without a word.
modded class ActionConstructor
{
    override void RegisterActions(TTypenameArray actions)
    {
        super.RegisterActions(actions);
        actions.Insert(OZG_ActionLayOut);
        actions.Insert(OZG_ActionSit);
        actions.Insert(OZG_ActionWatch);
        actions.Insert(OZG_ActionGather);
        actions.Insert(OZG_ActionStudy);
        actions.Insert(OZG_ActionOpenKit);
    }
}
