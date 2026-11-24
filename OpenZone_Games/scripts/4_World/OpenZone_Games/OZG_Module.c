// The mod's start: which games there are, and the ready line.
//
// The ready line is printed LAST and in the form `key=value`: the stand's
// verdict reads its counters from it.
[CF_RegisterModule(OZG_Module)]
class OZG_Module : CF_ModuleWorld
{
    private ref Timer m_ReadyTimer;
    private ref Timer m_TickTimer;
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

        // The service the windows talk to, and the sessions' clock. The places
        // that came back from the world's storage were made before this
        // moment and have waited for the games to be known: Start restores
        // them.
        OZ_ServiceRegistry.Register(OZG_Const.SERVICE, new OZG_Service());
        OZG_Sessions.Start();
        m_TickTimer = new Timer(CALL_CATEGORY_SYSTEM);
        m_TickTimer.Run(1.0, this, "Tick", NULL, true);

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

    void Tick()
    {
        OZG_Sessions.Tick();
    }

    override void OnMissionFinish(Class sender, CF_EventArgs args)
    {
        super.OnMissionFinish(sender, args);

        if (m_ReadyTimer)
            m_ReadyTimer.Stop();
        if (m_TickTimer)
            m_TickTimer.Stop();
        if (GetGame().IsServer())
            OZG_Sessions.Shutdown();
        OZG_Aim.Reset();
        OZG_Who.Reset();
        OZG_Decks.Reset();
        OZG_Mine.Forget();
        OZG_Games.Reset();
        OZG_Log.ResetWarned();
    }

    static string ReadyLine()
    {
        string line = "games loaded: games=" + OZG_Games.Count().ToString();
        line = line + " places=" + OZG_Sessions.Count().ToString();
        line = line + " problems=" + OZG_Log.Warned().ToString();
        return line;
    }
}
