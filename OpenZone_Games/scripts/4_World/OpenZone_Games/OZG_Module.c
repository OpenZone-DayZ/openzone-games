// The mod's start: which games there are, and the ready line.
//
// The ready line is printed LAST and in the form `key=value`: the stand's
// verdict reads its counters from it.
[CF_RegisterModule(OZG_Module)]
class OZG_Module : CF_ModuleWorld
{
    private ref Timer m_ReadyTimer;
    private static const float READY_DELAY = 1.0;

    override void OnInit()
    {
        super.OnInit();

        // super first, then the subscriptions: otherwise CF has not registered
        // the module yet and the event arrives nowhere.
        EnableMissionStart();
        EnableMissionFinish();
    }

    override void OnMissionStart(Class sender, CF_EventArgs args)
    {
        super.OnMissionStart(sender, args);

        // The games are known on both sides: the server plays them, a client
        // words their names and variants. Nothing is reset here: another mod's
        // module may have registered its game before this one ran.
        if (!OZG_Games.Sample(OZG_DurakConst.GAME_ID))
            OZG_Games.Register(new OZG_Durak());
        // -- games register above this line --

        if (!GetGame().IsServer())
            return;

        // The ready line one tick later: the order of CF modules is not
        // promised, and a line printed here may speak before a neighbour has
        // registered what it describes.
        m_ReadyTimer = new Timer(CALL_CATEGORY_SYSTEM);
        m_ReadyTimer.Run(READY_DELAY, this, "Ready", NULL, false);
    }

    // Called by the timer by name: it must not be private.
    void Ready()
    {
        OZG_Log.Info(ReadyLine());
    }

    override void OnMissionFinish(Class sender, CF_EventArgs args)
    {
        super.OnMissionFinish(sender, args);

        if (m_ReadyTimer)
            m_ReadyTimer.Stop();
        OZG_Games.Reset();
        OZG_Log.ResetWarned();
    }

    static string ReadyLine()
    {
        string line = "games loaded: games=" + OZG_Games.Count().ToString();
        line = line + " problems=" + OZG_Log.Warned().ToString();
        return line;
    }
}
