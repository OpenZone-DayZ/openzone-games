// Studying a card closely (card sharp's design §4.3): the loupe's work since
// 2026-11-25, when the owner took the loupe away. A card held in the hands is
// looked at for StudySeconds; the player is told, alone, "the card is marked"
// or "nothing found", and becomes a reader of a marked card (MarkKnowers
// keep it: the oldest reader who is not the marker gives way). Nothing is
// used up. A line in the server's log with the player's id, as for a marking.
// Cheating 0 takes the action away.
class OZG_Study
{
    // The outcome, on the server: what the action ends with, and the stand's
    // `craft what=study`.
    static void Done(PlayerBase player, OZG_Card_Base card)
    {
        if (!GetGame().IsServer() || !player || !card)
            return;
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return;
        string word = OZG_Const.SAY_STUDY_NOTHING;
        if (card.OZG_Marked())
        {
            card.OZG_AddReader(uid);
            word = OZG_Const.SAY_STUDY_MARKED;
        }
        PlayerIdentity id = player.GetIdentity();
        if (id)
            OZ_Rpc.Notice(id, OZG_Const.NOTICE_STUDY, true, word);
        string line = "study: " + uid + " studied " + card.OZG_Face();
        line = line + ", sign " + card.OZG_Sign().ToString();
        OZG_Log.Info(line);
    }
}

// Studying a die closely (boards and dice design §10): whether it is loaded,
// and to what; the player knows its load from then on.
class OZG_StudyDie
{
    static void Done(PlayerBase player, OZG_Die_Base die)
    {
        if (!GetGame().IsServer() || !player || !die)
            return;
        string uid = OZG_Who.Uid(player);
        if (uid == "")
            return;
        string word = OZG_Const.SAY_STUDY_DIE_FAIR;
        if (die.OZG_Loaded())
        {
            die.OZG_AddKnower(uid);
            int step = OZG_Cfg.Get().LoadStep(die.OZG_LoadWeight());
            word = OZG_Words.W2(OZG_Const.SAY_STUDY_DIE_LOADED, die.OZG_LoadFace().ToString(), OZG_Words.KeyArg(OZG_Const.LOAD_STEP + step.ToString()));
        }
        PlayerIdentity id = player.GetIdentity();
        if (id)
            OZ_Rpc.Notice(id, OZG_Const.NOTICE_STUDY, true, word);
        OZG_Log.Info("study: " + uid + " studied a die " + OZG_Keys.Of(die) + ", load " + die.OZG_LoadFace().ToString() + " at " + die.OZG_LoadWeight().ToString() + "%");
    }
}

// The time it takes: the setting, a client's from the sync.
class OZG_ActionStudyCB : ActionContinuousBaseCB
{
    override void CreateActionComponent()
    {
        m_ActionData.m_ActionComponent = new CAContinuousTime(OZG_Cfg.Get().StudySeconds);
    }
}

// The card held up to the eyes: the game's "view compass" motion, a looping
// one of an item in the hands; lying down, its full-body form.
class OZG_ActionStudy : ActionContinuousBase
{
    void OZG_ActionStudy()
    {
        m_CallbackClass = OZG_ActionStudyCB;
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_VIEWCOMPASS;
        m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_VIEWCOMPASS;
        m_Text = "#STR_OZG_STUDY_ACT";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINonRuined;
        m_ConditionTarget = new CCTNone;
    }

    override bool HasTarget()
    {
        return false;
    }

    override bool HasProneException()
    {
        return true;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!OZG_Card_Base.Cast(item) && !OZG_Die_Base.Cast(item))
            return false;
        return OZG_Cfg.Get().Cheating == 1;
    }

    override void OnFinishProgressServer(ActionData action_data)
    {
        OZG_Die_Base die = OZG_Die_Base.Cast(action_data.m_MainItem);
        if (die)
        {
            OZG_StudyDie.Done(action_data.m_Player, die);
            return;
        }
        OZG_Study.Done(action_data.m_Player, OZG_Card_Base.Cast(action_data.m_MainItem));
    }
}
