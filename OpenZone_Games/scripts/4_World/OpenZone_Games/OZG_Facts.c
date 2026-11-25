// The games' card on Ядро › Огляд (the core's OZ_AdminFacts, VPP rework design
// 4.3): which games are on, the places laid out, the games being played and the
// players seated at them, whether card sharping is on; the problems the last
// load found in the settings.
//
// "Reload every module" reads the settings file again, as the start does, and
// sends what a client holds of it to everyone online.
//
// Registered at the server mission start of OZG_Module, after the settings are
// read.
class OZG_Facts : OZ_AdminFactsProvider
{
    override void Fill(OZ_AdminModuleCard card)
    {
        card.Id = "games";
        card.Name = "#STR_OZG_VPP_MOD";

        // Pages in the window come with this repository's VPP glue.
        card.Pages = 0;
#ifdef OpenZone_Games_VPP
        card.Pages = 1;
#endif

        card.Problems = OZG_Cfg.Problems();

        OZG_Settings cfg = OZG_Cfg.Get();
        array<string> ids = new array<string>();
        OZG_Games.Ids(ids);
        int on = 0;
        for (int i = 0; i < ids.Count(); i++)
        {
            if (cfg.GameOn(ids[i]))
                on++;
        }
        string games = on.ToString() + " / " + ids.Count().ToString();
        card.Add("#STR_OZG_FACT_GAMES", games);

        card.Add("#STR_OZG_FACT_PLACES", OZG_Sessions.Count().ToString());

        int playing;
        int seated;
        OZG_Sessions.Counts(playing, seated);
        card.Add("#STR_OZG_FACT_PLAYING", playing.ToString());
        card.Add("#STR_OZG_FACT_SEATED", seated.ToString());

        string sharp = "#STR_OZG_VPP_OFF";
        if (cfg.Cheating == 1)
            sharp = "#STR_OZG_VPP_ON";
        card.Add("#STR_OZG_FACT_SHARP", sharp);
    }

    override bool Reload(array<string> said)
    {
        OZG_Cfg.ServerLoad();
        OZ_SyncSender.SendAll("games settings reloaded");
        return true;
    }
}
