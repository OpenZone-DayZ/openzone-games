// Server-side glue between the table games probe and the MCP bridge: adds the
// `tgp` verb to DZMCP_BridgeCore so world_exec can drive the probe. Requires
// @DZMCP_Bridge; this pbo is listed in mods.server_only of dayz-mcp.local.toml
// and never reaches a client.

class CfgPatches
{
    class TableGamesProbe_Bridge
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "DZMCP_Bridge",
            "TableGamesProbe"
        };
    };
};

class CfgMods
{
    class TableGamesProbe_Bridge
    {
        dir = "TableGamesProbe_Bridge";
        name = "Table Games Probe Bridge";
        author = "OpenZone";
        version = "0.1.0";
        type = "mod";

        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class missionScriptModule { value = ""; files[] = {"TableGamesProbe_Bridge/scripts/5_Mission"}; };
        };
    };
};
