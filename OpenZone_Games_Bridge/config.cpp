// Server-side glue between OpenZone Games and the MCP bridge: the `oz_games`
// verb for world_exec -- a game played by script, through the same contract a
// session uses. A stand tool, never published. Requires @DZMCP_Bridge; this pbo
// is listed in mods.server_only of dayz-mcp.local.toml and never reaches a
// client.

class CfgPatches
{
    class OpenZone_Games_Bridge
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "DZMCP_Bridge",
            "OpenZone_Games"
        };
    };
};

class CfgMods
{
    class OpenZone_Games_Bridge
    {
        dir = "OpenZone_Games_Bridge";
        name = "OpenZone Games Bridge";
        author = "OpenZone";
        version = "0.1.0";
        type = "mod";

        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class missionScriptModule { value = ""; files[] = {"OpenZone_Games_Bridge/scripts/5_Mission"}; };
        };
    };
};
