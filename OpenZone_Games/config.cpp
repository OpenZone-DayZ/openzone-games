// OpenZone Games -- table games for DayZ on the OpenZone core.
//
// WHAT THIS PBO IS. Games played on a surface the player chooses: a deck laid
// out, seats worked out around it, a game the server holds and every client
// shows. The first game is durak.
//
// WHAT IT STANDS ON. OpenZone_Core, hard: the log, and later the service pair
// and the config loader. CF through the core. Nothing else: with the core alone
// the mod runs whole.
//
// WHAT IT IS NOT YET. Phase 1 is the rules alone: cards, what a game is, durak.
// Nothing is in the world yet (docs/2026-11-24-phase-1-plan.md).

class CfgPatches
{
    class OpenZone_Games
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        // Hard dependencies: a blocking window before the game loads rather
        // than a silent skip.
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "JM_CF_Scripts",
            "OpenZone_Core"
        };
    };
};

class CfgMods
{
    class OpenZone_Games
    {
        dir        = "OpenZone_Games";
        name       = "OpenZone Games";
        credits    = "OpenZone";
        author     = "OpenZone";
        version    = "0.1.0";
        type       = "mod";

        // No mission module yet: the window brings it in its own phase.
        dependencies[] = {"Game", "World"};

        class defs
        {
            class gameScriptModule  { value = ""; files[] = {"OpenZone_Games/scripts/3_Game"}; };
            class worldScriptModule { value = ""; files[] = {"OpenZone_Games/scripts/4_World"}; };
        };
    };
};
