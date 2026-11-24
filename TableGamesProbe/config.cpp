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
        units[] = {"TGP_Deck", "TGP_Mat", "TGP_SlotBox", "TGP_GridBox"};
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

// The slot probe (2026-11-24): does the engine take fifty-two more inventory slots,
// and what does a thing with fifty-two attachments cost. One slot a card.
class CfgSlots
{
    class Slot_TGP_C00 { name = "TGP_C00"; displayName = "Probe card 00"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C01 { name = "TGP_C01"; displayName = "Probe card 01"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C02 { name = "TGP_C02"; displayName = "Probe card 02"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C03 { name = "TGP_C03"; displayName = "Probe card 03"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C04 { name = "TGP_C04"; displayName = "Probe card 04"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C05 { name = "TGP_C05"; displayName = "Probe card 05"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C06 { name = "TGP_C06"; displayName = "Probe card 06"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C07 { name = "TGP_C07"; displayName = "Probe card 07"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C08 { name = "TGP_C08"; displayName = "Probe card 08"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C09 { name = "TGP_C09"; displayName = "Probe card 09"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C10 { name = "TGP_C10"; displayName = "Probe card 10"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C11 { name = "TGP_C11"; displayName = "Probe card 11"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C12 { name = "TGP_C12"; displayName = "Probe card 12"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C13 { name = "TGP_C13"; displayName = "Probe card 13"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C14 { name = "TGP_C14"; displayName = "Probe card 14"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C15 { name = "TGP_C15"; displayName = "Probe card 15"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C16 { name = "TGP_C16"; displayName = "Probe card 16"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C17 { name = "TGP_C17"; displayName = "Probe card 17"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C18 { name = "TGP_C18"; displayName = "Probe card 18"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C19 { name = "TGP_C19"; displayName = "Probe card 19"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C20 { name = "TGP_C20"; displayName = "Probe card 20"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C21 { name = "TGP_C21"; displayName = "Probe card 21"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C22 { name = "TGP_C22"; displayName = "Probe card 22"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C23 { name = "TGP_C23"; displayName = "Probe card 23"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C24 { name = "TGP_C24"; displayName = "Probe card 24"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C25 { name = "TGP_C25"; displayName = "Probe card 25"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C26 { name = "TGP_C26"; displayName = "Probe card 26"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C27 { name = "TGP_C27"; displayName = "Probe card 27"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C28 { name = "TGP_C28"; displayName = "Probe card 28"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C29 { name = "TGP_C29"; displayName = "Probe card 29"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C30 { name = "TGP_C30"; displayName = "Probe card 30"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C31 { name = "TGP_C31"; displayName = "Probe card 31"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C32 { name = "TGP_C32"; displayName = "Probe card 32"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C33 { name = "TGP_C33"; displayName = "Probe card 33"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C34 { name = "TGP_C34"; displayName = "Probe card 34"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C35 { name = "TGP_C35"; displayName = "Probe card 35"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C36 { name = "TGP_C36"; displayName = "Probe card 36"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C37 { name = "TGP_C37"; displayName = "Probe card 37"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C38 { name = "TGP_C38"; displayName = "Probe card 38"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C39 { name = "TGP_C39"; displayName = "Probe card 39"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C40 { name = "TGP_C40"; displayName = "Probe card 40"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C41 { name = "TGP_C41"; displayName = "Probe card 41"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C42 { name = "TGP_C42"; displayName = "Probe card 42"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C43 { name = "TGP_C43"; displayName = "Probe card 43"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C44 { name = "TGP_C44"; displayName = "Probe card 44"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C45 { name = "TGP_C45"; displayName = "Probe card 45"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C46 { name = "TGP_C46"; displayName = "Probe card 46"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C47 { name = "TGP_C47"; displayName = "Probe card 47"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C48 { name = "TGP_C48"; displayName = "Probe card 48"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C49 { name = "TGP_C49"; displayName = "Probe card 49"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C50 { name = "TGP_C50"; displayName = "Probe card 50"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_C51 { name = "TGP_C51"; displayName = "Probe card 51"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_TGP_Hold { name = "TGP_Hold"; displayName = "Probe deck"; ghostIcon = "set:dayz_inventory image:missing"; };
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
        // The hold probe: the deck goes into the mat it has placed, with the
        // fifty-two probe cards it holds.
        inventorySlot[] = {"TGP_Hold"};
        attachments[] =
        {
            "TGP_C00", "TGP_C01", "TGP_C02", "TGP_C03", "TGP_C04", "TGP_C05", "TGP_C06", "TGP_C07", "TGP_C08", "TGP_C09", "TGP_C10", "TGP_C11", "TGP_C12",
            "TGP_C13", "TGP_C14", "TGP_C15", "TGP_C16", "TGP_C17", "TGP_C18", "TGP_C19", "TGP_C20", "TGP_C21", "TGP_C22", "TGP_C23", "TGP_C24", "TGP_C25",
            "TGP_C26", "TGP_C27", "TGP_C28", "TGP_C29", "TGP_C30", "TGP_C31", "TGP_C32", "TGP_C33", "TGP_C34", "TGP_C35", "TGP_C36", "TGP_C37", "TGP_C38",
            "TGP_C39", "TGP_C40", "TGP_C41", "TGP_C42", "TGP_C43", "TGP_C44", "TGP_C45", "TGP_C46", "TGP_C47", "TGP_C48", "TGP_C49", "TGP_C50", "TGP_C51"
        };
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
        attachments[] = {"TGP_Hold"};
        // slopeTolerance and yawPitchRollLimit are left to what the game gives
        // every item: the probe measures the hologram as it ships.
    };

    class TGP_DeckPlacing : TGP_Mat
    {
        scope = 1;
        displayName = "Probe mat (hologram)";
    };
    // The slot probe: a box with a slot for each of fifty-two cards, the same
    // cards in a box with a grid of 13 by 4, and the cards themselves.
    class TGP_SlotBox : Inventory_Base
    {
        scope = 2;
        displayName = "Probe slot box";
        descriptionShort = "Fifty-two slots, one a card. Not for play.";
        model = "\dz\gear\crafting\bp_burlap_sack.p3d";
        itemSize[] = {3, 2};
        weight = 100;
        attachments[] =
        {
            "TGP_C00", "TGP_C01", "TGP_C02", "TGP_C03", "TGP_C04", "TGP_C05", "TGP_C06", "TGP_C07", "TGP_C08", "TGP_C09", "TGP_C10", "TGP_C11", "TGP_C12",
            "TGP_C13", "TGP_C14", "TGP_C15", "TGP_C16", "TGP_C17", "TGP_C18", "TGP_C19", "TGP_C20", "TGP_C21", "TGP_C22", "TGP_C23", "TGP_C24", "TGP_C25",
            "TGP_C26", "TGP_C27", "TGP_C28", "TGP_C29", "TGP_C30", "TGP_C31", "TGP_C32", "TGP_C33", "TGP_C34", "TGP_C35", "TGP_C36", "TGP_C37", "TGP_C38",
            "TGP_C39", "TGP_C40", "TGP_C41", "TGP_C42", "TGP_C43", "TGP_C44", "TGP_C45", "TGP_C46", "TGP_C47", "TGP_C48", "TGP_C49", "TGP_C50", "TGP_C51"
        };
    };

    class TGP_GridBox : Inventory_Base
    {
        scope = 2;
        displayName = "Probe grid box";
        descriptionShort = "A grid of 13 by 4 for fifty-two cards. Not for play.";
        model = "\dz\gear\crafting\bp_burlap_sack.p3d";
        itemSize[] = {3, 2};
        weight = 100;
        itemsCargoSize[] = {13, 4};
    };

    class TGP_SlotCard_Base : Inventory_Base
    {
        scope = 0;
        model = "\dz\gear\consumables\Paper.p3d";
        itemSize[] = {1, 1};
        weight = 2;
    };
    class TGP_SlotCard_00 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 00"; inventorySlot[] = {"TGP_C00"}; };
    class TGP_SlotCard_01 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 01"; inventorySlot[] = {"TGP_C01"}; };
    class TGP_SlotCard_02 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 02"; inventorySlot[] = {"TGP_C02"}; };
    class TGP_SlotCard_03 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 03"; inventorySlot[] = {"TGP_C03"}; };
    class TGP_SlotCard_04 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 04"; inventorySlot[] = {"TGP_C04"}; };
    class TGP_SlotCard_05 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 05"; inventorySlot[] = {"TGP_C05"}; };
    class TGP_SlotCard_06 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 06"; inventorySlot[] = {"TGP_C06"}; };
    class TGP_SlotCard_07 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 07"; inventorySlot[] = {"TGP_C07"}; };
    class TGP_SlotCard_08 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 08"; inventorySlot[] = {"TGP_C08"}; };
    class TGP_SlotCard_09 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 09"; inventorySlot[] = {"TGP_C09"}; };
    class TGP_SlotCard_10 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 10"; inventorySlot[] = {"TGP_C10"}; };
    class TGP_SlotCard_11 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 11"; inventorySlot[] = {"TGP_C11"}; };
    class TGP_SlotCard_12 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 12"; inventorySlot[] = {"TGP_C12"}; };
    class TGP_SlotCard_13 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 13"; inventorySlot[] = {"TGP_C13"}; };
    class TGP_SlotCard_14 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 14"; inventorySlot[] = {"TGP_C14"}; };
    class TGP_SlotCard_15 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 15"; inventorySlot[] = {"TGP_C15"}; };
    class TGP_SlotCard_16 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 16"; inventorySlot[] = {"TGP_C16"}; };
    class TGP_SlotCard_17 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 17"; inventorySlot[] = {"TGP_C17"}; };
    class TGP_SlotCard_18 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 18"; inventorySlot[] = {"TGP_C18"}; };
    class TGP_SlotCard_19 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 19"; inventorySlot[] = {"TGP_C19"}; };
    class TGP_SlotCard_20 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 20"; inventorySlot[] = {"TGP_C20"}; };
    class TGP_SlotCard_21 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 21"; inventorySlot[] = {"TGP_C21"}; };
    class TGP_SlotCard_22 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 22"; inventorySlot[] = {"TGP_C22"}; };
    class TGP_SlotCard_23 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 23"; inventorySlot[] = {"TGP_C23"}; };
    class TGP_SlotCard_24 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 24"; inventorySlot[] = {"TGP_C24"}; };
    class TGP_SlotCard_25 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 25"; inventorySlot[] = {"TGP_C25"}; };
    class TGP_SlotCard_26 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 26"; inventorySlot[] = {"TGP_C26"}; };
    class TGP_SlotCard_27 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 27"; inventorySlot[] = {"TGP_C27"}; };
    class TGP_SlotCard_28 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 28"; inventorySlot[] = {"TGP_C28"}; };
    class TGP_SlotCard_29 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 29"; inventorySlot[] = {"TGP_C29"}; };
    class TGP_SlotCard_30 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 30"; inventorySlot[] = {"TGP_C30"}; };
    class TGP_SlotCard_31 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 31"; inventorySlot[] = {"TGP_C31"}; };
    class TGP_SlotCard_32 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 32"; inventorySlot[] = {"TGP_C32"}; };
    class TGP_SlotCard_33 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 33"; inventorySlot[] = {"TGP_C33"}; };
    class TGP_SlotCard_34 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 34"; inventorySlot[] = {"TGP_C34"}; };
    class TGP_SlotCard_35 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 35"; inventorySlot[] = {"TGP_C35"}; };
    class TGP_SlotCard_36 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 36"; inventorySlot[] = {"TGP_C36"}; };
    class TGP_SlotCard_37 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 37"; inventorySlot[] = {"TGP_C37"}; };
    class TGP_SlotCard_38 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 38"; inventorySlot[] = {"TGP_C38"}; };
    class TGP_SlotCard_39 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 39"; inventorySlot[] = {"TGP_C39"}; };
    class TGP_SlotCard_40 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 40"; inventorySlot[] = {"TGP_C40"}; };
    class TGP_SlotCard_41 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 41"; inventorySlot[] = {"TGP_C41"}; };
    class TGP_SlotCard_42 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 42"; inventorySlot[] = {"TGP_C42"}; };
    class TGP_SlotCard_43 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 43"; inventorySlot[] = {"TGP_C43"}; };
    class TGP_SlotCard_44 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 44"; inventorySlot[] = {"TGP_C44"}; };
    class TGP_SlotCard_45 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 45"; inventorySlot[] = {"TGP_C45"}; };
    class TGP_SlotCard_46 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 46"; inventorySlot[] = {"TGP_C46"}; };
    class TGP_SlotCard_47 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 47"; inventorySlot[] = {"TGP_C47"}; };
    class TGP_SlotCard_48 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 48"; inventorySlot[] = {"TGP_C48"}; };
    class TGP_SlotCard_49 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 49"; inventorySlot[] = {"TGP_C49"}; };
    class TGP_SlotCard_50 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 50"; inventorySlot[] = {"TGP_C50"}; };
    class TGP_SlotCard_51 : TGP_SlotCard_Base { scope = 2; displayName = "Probe card 51"; inventorySlot[] = {"TGP_C51"}; };
};
