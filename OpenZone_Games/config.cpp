// OpenZone Games -- table games for DayZ on the OpenZone core.
//
// WHAT THIS PBO IS. Games played on a surface the player chooses: a deck laid
// out, seats worked out around it, a game the server holds and every client
// shows. The first game is durak.
//
// WHAT IT STANDS ON. OpenZone_Core, hard: the log, the service pair, the
// notices. CF through the core. Nothing else: with the core alone the mod runs
// whole.

class CfgPatches
{
    class OpenZone_Games
    {
        units[] = {"OZG_Deck_Classic", "OZG_Deck_Zone"};
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
        version    = "0.2.0";
        type       = "mod";

        // The key that steps through the seat modes while aiming. A mod gets
        // exactly one inputs file.
        inputs = "OpenZone_Games/data/inputs.xml";

        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class gameScriptModule    { value = ""; files[] = {"OpenZone_Games/scripts/3_Game"}; };
            class worldScriptModule   { value = ""; files[] = {"OpenZone_Games/scripts/4_World"}; };
            class missionScriptModule { value = ""; files[] = {"OpenZone_Games/scripts/5_Mission"}; };

            // One imageset a deck style: 52 faces, the back and the four
            // suits. A set that is not listed here loads as blank pictures,
            // with no error at all.
            class imageSets
            {
                files[] =
                {
                    "OpenZone_Games/gui/imagesets/ozg_cards_classic.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_cards_zone.imageset"
                };
            };
        };
    };
};

class CfgVehicles
{
    class Inventory_Base;

    // ------------------------------------------------------------ decks
    //
    // A deck is a config class and its art; no script is needed to add one.
    // Another mod adds a deck with a class that inherits OZG_Deck_Base, an
    // imageset of its own, ozg_cards_<ozgStyle>, and a sheet for the 3D
    // things, ozgSheet (the back on its left half, a blank face and the edges
    // of a stack on its right). A deck is always 52 cards: decks differ in how
    // they look, not in what they hold.
    class OZG_Deck_Base: Inventory_Base
    {
        scope = 0;
        displayName = "$STR_OZG_DECK_NAME";
        descriptionShort = "$STR_OZG_DECK_DESC";
        model = "\OpenZone_Games\models\deck\ozg_deck.p3d";
        itemSize[] = {1, 2};
        weight = 100;
        rotationFlags = 16;
        isMeleeWeapon = 1;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_classic_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\data\cards\ozg_deck.rvmat"};
        // What the hologram shows while the deck is being laid out.
        projectionTypename = "OZG_PlacePlacing";
        // The style the cards are drawn in, and the sheet the 3D things wear.
        ozgStyle = "classic";
        ozgSheet = "OpenZone_Games\data\cards\ozg_deck_classic_co.paa";
    };

    class OZG_Deck_Classic: OZG_Deck_Base
    {
        scope = 2;
        displayName = "$STR_OZG_DECK_CLASSIC";
    };

    class OZG_Deck_Zone: OZG_Deck_Base
    {
        scope = 2;
        displayName = "$STR_OZG_DECK_ZONE";
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_zone_co.paa"};
        ozgStyle = "zone";
        ozgSheet = "OpenZone_Games\data\cards\ozg_deck_zone_co.paa";
    };

    // ------------------------------------------------------------ the place
    //
    // scope 2, and not for the spawn lists' sake: a class of scope 1 is never
    // written to the world's storage -- its OnStoreSave is not even called
    // (measured 2026-11-24: of three things made by one call on one spot, the
    // two of scope 1 were gone after a restart and the one of scope 2 stood).
    // What the hologram shows, the fan in hands and the seats' marks stay at
    // scope 1 for the same reason turned round: they must never be saved.
    class OZG_Place: Inventory_Base
    {
        scope = 2;
        displayName = "$STR_OZG_PLACE_NAME";
        descriptionShort = "$STR_OZG_PLACE_DESC";
        model = "\OpenZone_Games\models\place\ozg_place.p3d";
        itemSize[] = {10, 10};
        weight = 400;
        // The deck on the mat wears the sheet of the deck the place was laid
        // out from (OZG_Place.OZG_Look).
        hiddenSelections[] = {"deck"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_classic_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\data\cards\ozg_deck.rvmat"};
    };

    // The same model with one selection over all of it: the hologram repaints
    // it with <hologramMaterialPath>\<hologramMaterial>_deployable.rvmat or
    // _undeployable.rvmat.
    class OZG_PlacePlacing: Inventory_Base
    {
        scope = 1;
        displayName = "$STR_OZG_PLACE_NAME";
        descriptionShort = "$STR_OZG_PLACE_DESC";
        model = "\OpenZone_Games\models\place\ozg_place.p3d";
        itemSize[] = {10, 10};
        weight = 400;
        hiddenSelections[] = {"camo"};
        hologramMaterial = "ozg_place";
        hologramMaterialPath = "OpenZone_Games\models\place\data";
    };

    // ------------------------------------------------------------ the fan in hands
    //
    // Eight cards, each a selection of its own: the cards a player does not
    // hold are blanked (OZG_Hand.OZG_Show).
    class OZG_Hand: Inventory_Base
    {
        scope = 1;
        displayName = "$STR_OZG_HAND_NAME";
        descriptionShort = "$STR_OZG_HAND_DESC";
        model = "\OpenZone_Games\models\hand\ozg_hand.p3d";
        itemSize[] = {2, 3};
        weight = 20;
        rotationFlags = 16;
        hiddenSelections[] = {"card1", "card2", "card3", "card4", "card5", "card6", "card7", "card8"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa", "OpenZone_Games\data\cards\ozg_deck_classic_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat", "OpenZone_Games\data\cards\ozg_deck.rvmat"};
    };

    // ------------------------------------------------------------ a seat's mark
    class OZG_SeatMark: Inventory_Base
    {
        scope = 1;
        displayName = "$STR_OZG_MARK_NAME";
        descriptionShort = "$STR_OZG_MARK_NAME";
        model = "\OpenZone_Games\models\mark\ozg_mark.p3d";
        itemSize[] = {10, 10};
        weight = 10;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(0.25,0.9,0.35,1.0,co)"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\mark\data\ozg_mark.rvmat"};
    };

    // A free seat.
    class OZG_SeatMark_Free: OZG_SeatMark
    {
    };

    // A seat left with cards.
    class OZG_SeatMark_Left: OZG_SeatMark
    {
        hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(1.0,0.72,0.15,1.0,co)"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\mark\data\ozg_mark_amber.rvmat"};
    };
};
