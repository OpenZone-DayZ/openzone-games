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
        units[] = {"OZG_Deck_Classic", "OZG_Deck_Zone", "OZG_Deck_Classic_Sharp", "OZG_Deck_Zone_Sharp"};
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

// A slot for a card, any card: sixty-two of them, and every card names them
// all. An ordinary deck has the first fifty-two, a sharp's deck all sixty-two.
// One more slot is the place's, for the deck it holds. The sixty-two are
// written by tools/make_card_items.py.
class CfgSlots
{
    class Slot_OZG_DeckHold { name = "OZG_DeckHold"; displayName = "$STR_OZG_DECK_NAME"; ghostIcon = "set:dayz_inventory image:missing"; };
// >>> generated: slots
    class Slot_OZG_Card01 { name = "OZG_Card01"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card02 { name = "OZG_Card02"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card03 { name = "OZG_Card03"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card04 { name = "OZG_Card04"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card05 { name = "OZG_Card05"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card06 { name = "OZG_Card06"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card07 { name = "OZG_Card07"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card08 { name = "OZG_Card08"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card09 { name = "OZG_Card09"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card10 { name = "OZG_Card10"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card11 { name = "OZG_Card11"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card12 { name = "OZG_Card12"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card13 { name = "OZG_Card13"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card14 { name = "OZG_Card14"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card15 { name = "OZG_Card15"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card16 { name = "OZG_Card16"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card17 { name = "OZG_Card17"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card18 { name = "OZG_Card18"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card19 { name = "OZG_Card19"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card20 { name = "OZG_Card20"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card21 { name = "OZG_Card21"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card22 { name = "OZG_Card22"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card23 { name = "OZG_Card23"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card24 { name = "OZG_Card24"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card25 { name = "OZG_Card25"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card26 { name = "OZG_Card26"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card27 { name = "OZG_Card27"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card28 { name = "OZG_Card28"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card29 { name = "OZG_Card29"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card30 { name = "OZG_Card30"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card31 { name = "OZG_Card31"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card32 { name = "OZG_Card32"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card33 { name = "OZG_Card33"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card34 { name = "OZG_Card34"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card35 { name = "OZG_Card35"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card36 { name = "OZG_Card36"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card37 { name = "OZG_Card37"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card38 { name = "OZG_Card38"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card39 { name = "OZG_Card39"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card40 { name = "OZG_Card40"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card41 { name = "OZG_Card41"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card42 { name = "OZG_Card42"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card43 { name = "OZG_Card43"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card44 { name = "OZG_Card44"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card45 { name = "OZG_Card45"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card46 { name = "OZG_Card46"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card47 { name = "OZG_Card47"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card48 { name = "OZG_Card48"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card49 { name = "OZG_Card49"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card50 { name = "OZG_Card50"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card51 { name = "OZG_Card51"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card52 { name = "OZG_Card52"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card53 { name = "OZG_Card53"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card54 { name = "OZG_Card54"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card55 { name = "OZG_Card55"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card56 { name = "OZG_Card56"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card57 { name = "OZG_Card57"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card58 { name = "OZG_Card58"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card59 { name = "OZG_Card59"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card60 { name = "OZG_Card60"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card61 { name = "OZG_Card61"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card62 { name = "OZG_Card62"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
// <<< generated: slots
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
    // of a stack on its right). A deck holds cards, which are things, in slots
    // that take any card: a new deck fills itself with the fifty-two of its
    // own look, and what it holds after that is its owner's business.
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
        // A laid-out game keeps its deck in this slot of the place.
        inventorySlot[] = {"OZG_DeckHold"};
// >>> generated: deck slots
        attachments[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52"
        };
// <<< generated: deck slots
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

    // A card sharp's deck: the same deck outside, ten empty slots more
    // inside, for the cards mixed in.
    class OZG_Deck_Classic_Sharp: OZG_Deck_Classic
    {
// >>> generated: sharp slots, classic
        attachments[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54", "OZG_Card55", "OZG_Card56", "OZG_Card57", "OZG_Card58", "OZG_Card59", "OZG_Card60",
            "OZG_Card61", "OZG_Card62"
        };
// <<< generated: sharp slots, classic
    };

    class OZG_Deck_Zone_Sharp: OZG_Deck_Zone
    {
// >>> generated: sharp slots, zone
        attachments[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54", "OZG_Card55", "OZG_Card56", "OZG_Card57", "OZG_Card58", "OZG_Card59", "OZG_Card60",
            "OZG_Card61", "OZG_Card62"
        };
// <<< generated: sharp slots, zone
    };

    // ------------------------------------------------------------ cards
    //
    // One class a face, written by tools/make_card_items.py. A card fits every
    // card slot. Its back is the look of the deck it was made for, which the
    // script keeps and tells the clients; the model and the picture are the
    // deck's own until a card has art of its own.
// >>> generated: cards
    class OZG_Card_Base: Inventory_Base
    {
        scope = 0;
        descriptionShort = "$STR_OZG_CARD_DESC";
        model = "\OpenZone_Games\models\deck\ozg_deck.p3d";
        itemSize[] = {1, 1};
        weight = 2;
        rotationFlags = 16;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_classic_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\data\cards\ozg_deck.rvmat"};
        ozgFace = "";
        inventorySlot[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54", "OZG_Card55", "OZG_Card56", "OZG_Card57", "OZG_Card58", "OZG_Card59", "OZG_Card60",
            "OZG_Card61", "OZG_Card62"
        };
    };
    class OZG_Card_2s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2S"; ozgFace = "2s"; };
    class OZG_Card_3s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3S"; ozgFace = "3s"; };
    class OZG_Card_4s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4S"; ozgFace = "4s"; };
    class OZG_Card_5s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5S"; ozgFace = "5s"; };
    class OZG_Card_6s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6S"; ozgFace = "6s"; };
    class OZG_Card_7s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7S"; ozgFace = "7s"; };
    class OZG_Card_8s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8S"; ozgFace = "8s"; };
    class OZG_Card_9s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9S"; ozgFace = "9s"; };
    class OZG_Card_Ts: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TS"; ozgFace = "Ts"; };
    class OZG_Card_Js: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JS"; ozgFace = "Js"; };
    class OZG_Card_Qs: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QS"; ozgFace = "Qs"; };
    class OZG_Card_Ks: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KS"; ozgFace = "Ks"; };
    class OZG_Card_As: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AS"; ozgFace = "As"; };
    class OZG_Card_2h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2H"; ozgFace = "2h"; };
    class OZG_Card_3h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3H"; ozgFace = "3h"; };
    class OZG_Card_4h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4H"; ozgFace = "4h"; };
    class OZG_Card_5h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5H"; ozgFace = "5h"; };
    class OZG_Card_6h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6H"; ozgFace = "6h"; };
    class OZG_Card_7h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7H"; ozgFace = "7h"; };
    class OZG_Card_8h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8H"; ozgFace = "8h"; };
    class OZG_Card_9h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9H"; ozgFace = "9h"; };
    class OZG_Card_Th: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TH"; ozgFace = "Th"; };
    class OZG_Card_Jh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JH"; ozgFace = "Jh"; };
    class OZG_Card_Qh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QH"; ozgFace = "Qh"; };
    class OZG_Card_Kh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KH"; ozgFace = "Kh"; };
    class OZG_Card_Ah: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AH"; ozgFace = "Ah"; };
    class OZG_Card_2d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2D"; ozgFace = "2d"; };
    class OZG_Card_3d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3D"; ozgFace = "3d"; };
    class OZG_Card_4d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4D"; ozgFace = "4d"; };
    class OZG_Card_5d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5D"; ozgFace = "5d"; };
    class OZG_Card_6d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6D"; ozgFace = "6d"; };
    class OZG_Card_7d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7D"; ozgFace = "7d"; };
    class OZG_Card_8d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8D"; ozgFace = "8d"; };
    class OZG_Card_9d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9D"; ozgFace = "9d"; };
    class OZG_Card_Td: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TD"; ozgFace = "Td"; };
    class OZG_Card_Jd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JD"; ozgFace = "Jd"; };
    class OZG_Card_Qd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QD"; ozgFace = "Qd"; };
    class OZG_Card_Kd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KD"; ozgFace = "Kd"; };
    class OZG_Card_Ad: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AD"; ozgFace = "Ad"; };
    class OZG_Card_2c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2C"; ozgFace = "2c"; };
    class OZG_Card_3c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3C"; ozgFace = "3c"; };
    class OZG_Card_4c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4C"; ozgFace = "4c"; };
    class OZG_Card_5c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5C"; ozgFace = "5c"; };
    class OZG_Card_6c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6C"; ozgFace = "6c"; };
    class OZG_Card_7c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7C"; ozgFace = "7c"; };
    class OZG_Card_8c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8C"; ozgFace = "8c"; };
    class OZG_Card_9c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9C"; ozgFace = "9c"; };
    class OZG_Card_Tc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TC"; ozgFace = "Tc"; };
    class OZG_Card_Jc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JC"; ozgFace = "Jc"; };
    class OZG_Card_Qc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QC"; ozgFace = "Qc"; };
    class OZG_Card_Kc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KC"; ozgFace = "Kc"; };
    class OZG_Card_Ac: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AC"; ozgFace = "Ac"; };
// <<< generated: cards

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
        // The deck of the laid-out game, and room for cards the deck has no
        // slot for. Neither is a player's to open (OZG_Place in the script).
        attachments[] = {"OZG_DeckHold"};
        itemsCargoSize[] = {10, 3};
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
