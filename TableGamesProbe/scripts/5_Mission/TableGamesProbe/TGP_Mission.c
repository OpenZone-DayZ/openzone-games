// Table Games Probe -- the server's frame drives the long jobs.

modded class MissionServer
{
    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        TGP_Verb.Pump();
    }
}
