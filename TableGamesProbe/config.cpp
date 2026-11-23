// Table Games Probe -- measurement tooling for the table games research.
//
// Not a gameplay mod and never published. It asks the engine the questions
// the research note leaves open (docs/specs/2026-11-23-table-games-research.md,
// section 8): which surfaces of the world a placement hologram lands on, and
// what it costs to work out the places around one.
//
// The server half is driven through the MCP bridge verb `tgp`
// (TableGamesProbe_Bridge) and writes into $profile:TableGamesProbe/.

class CfgPatches
{
    class TableGamesProbe
    {
        units[] = {"TGP_Deck", "TGP_Mat"};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            // The two borrowed models: a sheet of paper and a burlap sack.
            "DZ_Gear_Consumables",
            "DZ_Gear_Crafting"
        };
    };
};

class CfgMods
{
    class TableGamesProbe
    {
        dir = "TableGamesProbe";
        name = "Table Games Probe";
        author = "OpenZone";
        version = "0.1.0";
        type = "mod";

        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class worldScriptModule   { value = ""; files[] = {"TableGamesProbe/scripts/4_World"}; };
            class missionScriptModule { value = ""; files[] = {"TableGamesProbe/scripts/5_Mission"}; };
        };
    };
};

class CfgVehicles
{
    class Inventory_Base;

    // The deck: a kit in the game's sense. Its hologram is TGP_DeckPlacing.
    class TGP_Deck : Inventory_Base
    {
        scope = 2;
        displayName = "Probe deck";
        descriptionShort = "A stand-in deck of the table games probe: it is laid out on a surface. Not for play.";
        model = "\dz\gear\consumables\Paper.p3d";
        itemSize[] = {1, 2};
        weight = 100;
        rotationFlags = 16;
        projectionTypename = "TGP_DeckPlacing";
    };

    // What the hologram shows and what is left on the surface: a flat mat.
    class TGP_Mat : Inventory_Base
    {
        scope = 2;
        displayName = "Probe mat";
        descriptionShort = "The laid-out place of the table games probe. Not for play.";
        model = "\dz\gear\crafting\bp_burlap_sack.p3d";
        itemSize[] = {3, 2};
        weight = 100;
        // slopeTolerance and yawPitchRollLimit are left to what the game gives
        // every item: the probe measures the hologram as it ships.
    };

    class TGP_DeckPlacing : TGP_Mat
    {
        scope = 1;
        displayName = "Probe mat (hologram)";
    };
};
