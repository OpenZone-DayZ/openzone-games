// OpenZone Games in the OpenZone window of VPP Admin Tools -- an OPTIONAL pbo of
// its own (2026-11-25).
//
// The module "Ігри" on the window's rail: Налаштування (which games are played,
// the table and its seats, presence, time, the log), Шулерство (marks, noticing,
// sleight of hand) and Пози (how a player sits or stands), all three a form over
// OZ_Games_Settings.json, which the games register in the core's admin console;
// Файли конфігів comes with the core's window by itself.
//
// Glue, as the rest of the series' VPP pages: it requires the games, the core,
// the core's VPP window and VPP itself, all hard. A server without VPP never
// loads it, and the games never require VPP.
//
// DZM_VPPAdminToolsScripts -- the CfgPatches class of VPP's scripts pbo. The
// code is guarded by AVPPAdminTools and OpenZone_VPP: the engine defines the
// names of CfgMods classes, not of CfgPatches ones.

class CfgPatches
{
    class OpenZone_Games_VPP
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "OpenZone_Core",
            "OpenZone_Games",
            "OpenZone_VPP",
            "DZM_VPPAdminToolsScripts"
        };
    };
};

class CfgMods
{
    class OpenZone_Games_VPP
    {
        dir = "OpenZone_Games_VPP";
        name = "OpenZone Games VPP Tab";
        author = "OpenZone";
        version = "0.1.0";
        type = "mod";

        dependencies[] = {"Mission"};

        class defs
        {
            class missionScriptModule { value = ""; files[] = {"OpenZone_Games_VPP/scripts/5_Mission"}; };
        };
    };
};
