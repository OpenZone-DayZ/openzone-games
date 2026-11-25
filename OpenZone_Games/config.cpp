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
        units[] = {"OZG_Deck_Classic", "OZG_Deck_Stalker", "OZG_Deck_Classic_Sharp", "OZG_Deck_Stalker_Sharp", "OZG_MarkTool", "OZG_Board_Classic", "OZG_Die"};
        weapons[] = {};
        requiredVersion = 0.1;
        // Hard dependencies: a blocking window before the game loads rather
        // than a silent skip.
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "JM_CF_Scripts",
            "OpenZone_Core",
            // The game's sounds config: the table's sound shaders and sets
            // inherit its baseCharacter_SoundShader and baseCharacter_SoundSet,
            // and without it the base classes are not there when the config
            // is read (OpenZone Radio's config says the same). It is the
            // game's own pbo, on every server.
            "DZ_Sounds_Effects"
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
            // suits; one more for the marks a reader sees (tools/make_marks.py).
            // A set that is not listed here loads as blank pictures, with no
            // error at all.
            class imageSets
            {
                files[] =
                {
                    "OpenZone_Games/gui/imagesets/ozg_cards_classic.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_cards_stalker.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_marks.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_pieces_classic.imageset"
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
    class Slot_OZG_Card63 { name = "OZG_Card63"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Card64 { name = "OZG_Card64"; displayName = "$STR_OZG_CARD_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
// <<< generated: slots
    // The "3 in 1" board's insert: a slot for every piece (tools/make_board_items.py).
// >>> generated: board slots
    class Slot_OZG_Bd_wK { name = "OZG_Bd_wK"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wQ { name = "OZG_Bd_wQ"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wR1 { name = "OZG_Bd_wR1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wR2 { name = "OZG_Bd_wR2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wB1 { name = "OZG_Bd_wB1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wB2 { name = "OZG_Bd_wB2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wN1 { name = "OZG_Bd_wN1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wN2 { name = "OZG_Bd_wN2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP1 { name = "OZG_Bd_wP1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP2 { name = "OZG_Bd_wP2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP3 { name = "OZG_Bd_wP3"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP4 { name = "OZG_Bd_wP4"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP5 { name = "OZG_Bd_wP5"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP6 { name = "OZG_Bd_wP6"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP7 { name = "OZG_Bd_wP7"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wP8 { name = "OZG_Bd_wP8"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bK { name = "OZG_Bd_bK"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bQ { name = "OZG_Bd_bQ"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bR1 { name = "OZG_Bd_bR1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bR2 { name = "OZG_Bd_bR2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bB1 { name = "OZG_Bd_bB1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bB2 { name = "OZG_Bd_bB2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bN1 { name = "OZG_Bd_bN1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bN2 { name = "OZG_Bd_bN2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP1 { name = "OZG_Bd_bP1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP2 { name = "OZG_Bd_bP2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP3 { name = "OZG_Bd_bP3"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP4 { name = "OZG_Bd_bP4"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP5 { name = "OZG_Bd_bP5"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP6 { name = "OZG_Bd_bP6"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP7 { name = "OZG_Bd_bP7"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bP8 { name = "OZG_Bd_bP8"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD01 { name = "OZG_Bd_wD01"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD02 { name = "OZG_Bd_wD02"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD03 { name = "OZG_Bd_wD03"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD04 { name = "OZG_Bd_wD04"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD05 { name = "OZG_Bd_wD05"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD06 { name = "OZG_Bd_wD06"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD07 { name = "OZG_Bd_wD07"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD08 { name = "OZG_Bd_wD08"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD09 { name = "OZG_Bd_wD09"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD10 { name = "OZG_Bd_wD10"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD11 { name = "OZG_Bd_wD11"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD12 { name = "OZG_Bd_wD12"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD13 { name = "OZG_Bd_wD13"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD14 { name = "OZG_Bd_wD14"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_wD15 { name = "OZG_Bd_wD15"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD01 { name = "OZG_Bd_bD01"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD02 { name = "OZG_Bd_bD02"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD03 { name = "OZG_Bd_bD03"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD04 { name = "OZG_Bd_bD04"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD05 { name = "OZG_Bd_bD05"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD06 { name = "OZG_Bd_bD06"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD07 { name = "OZG_Bd_bD07"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD08 { name = "OZG_Bd_bD08"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD09 { name = "OZG_Bd_bD09"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD10 { name = "OZG_Bd_bD10"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD11 { name = "OZG_Bd_bD11"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD12 { name = "OZG_Bd_bD12"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD13 { name = "OZG_Bd_bD13"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD14 { name = "OZG_Bd_bD14"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_bD15 { name = "OZG_Bd_bD15"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_Die1 { name = "OZG_Bd_Die1"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
    class Slot_OZG_Bd_Die2 { name = "OZG_Bd_Die2"; displayName = "$STR_OZG_PIECE_SLOT"; ghostIcon = "set:dayz_inventory image:missing"; };
// <<< generated: board slots
};

class CfgVehicles
{
    class Inventory_Base;

    // ------------------------------------------------------------ decks
    //
    // A deck is a config class and its art; no script is needed to add one.
    // Another mod adds a deck with a class that inherits OZG_Deck_Base, an
    // imageset of its own, ozg_cards_<ozgStyle>, a sheet for the 3D things,
    // ozgSheet (the back on its left half, a blank face and the edges of a
    // stack on its right), and the single cards' pictures, ozgCards: the path
    // of their files up to the face, which the script ends with <face>_co.paa
    // and back_co.paa (256 x 512, tools/make_cards.py). The file names must
    // be no other deck's: the game took two textures of one file name in two
    // folders for one texture (measured 2026-11-24), hence ozg_<style>_ in
    // each name and not only a folder of the deck's own. A deck holds cards,
    // which are things, in slots that take any card: a new deck fills itself
    // with the fifty-two of its own look, and with the two jokers when its
    // class says ozgJokers = 1 (then it needs two slots more), and what it
    // holds after that is its owner's business.
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
        // The style the cards are drawn in, the sheet the 3D things wear and
        // where the pictures of its single cards lie.
        ozgStyle = "classic";
        ozgSheet = "OpenZone_Games\data\cards\ozg_deck_classic_co.paa";
        ozgCards = "OpenZone_Games\data\cards\classic\ozg_classic_";
        // 1: a new deck gets the two jokers, Xr and Xb, besides the fifty-two.
        ozgJokers = 0;
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

    // The stalkers' deck: a worn one whose aces and kings carry stalkers and
    // whose queens, jacks and tens carry mutants. The pictures on those twenty
    // cards are GSC Game World's, from the S.T.A.L.K.E.R. games (see NOTICE).
    // It has the two jokers: fifty-four cards.
    class OZG_Deck_Stalker: OZG_Deck_Base
    {
        scope = 2;
        displayName = "$STR_OZG_DECK_STALKER";
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\ozg_deck_stalker_co.paa"};
        ozgStyle = "stalker";
        ozgSheet = "OpenZone_Games\data\cards\ozg_deck_stalker_co.paa";
        ozgCards = "OpenZone_Games\data\cards\stalker\ozg_stalker_";
        ozgJokers = 1;
// >>> generated: deck slots, stalker
        attachments[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54"
        };
// <<< generated: deck slots, stalker
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

    class OZG_Deck_Stalker_Sharp: OZG_Deck_Stalker
    {
// >>> generated: sharp slots, stalker
        attachments[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54", "OZG_Card55", "OZG_Card56", "OZG_Card57", "OZG_Card58", "OZG_Card59", "OZG_Card60",
            "OZG_Card61", "OZG_Card62", "OZG_Card63", "OZG_Card64"
        };
// <<< generated: sharp slots, stalker
    };

    // ------------------------------------------------------------ cards
    //
    // One class a face, written by tools/make_card_items.py. A card fits every
    // card slot. Its look is the deck it was made for, which the script keeps
    // and tells the clients. Every card is one model, ozg_card.p3d, with two
    // sides: `face` wears that card's picture in its look and `back` the look's
    // back. The config names the classic deck's pictures; the script puts on
    // those of the card's own look (OZG_Card_Base.OZG_Paint).
// >>> generated: cards
    class OZG_Card_Base: Inventory_Base
    {
        scope = 0;
        descriptionShort = "$STR_OZG_CARD_DESC";
        model = "\OpenZone_Games\models\card\ozg_card.p3d";
        itemSize[] = {1, 1};
        weight = 2;
        rotationFlags = 16;
        hiddenSelections[] = {"face", "back"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_As_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\data\cards\ozg_card.rvmat", "OpenZone_Games\data\cards\ozg_card.rvmat"};
        ozgFace = "";
        inventorySlot[] =
        {
            "OZG_Card01", "OZG_Card02", "OZG_Card03", "OZG_Card04", "OZG_Card05", "OZG_Card06", "OZG_Card07", "OZG_Card08", "OZG_Card09", "OZG_Card10",
            "OZG_Card11", "OZG_Card12", "OZG_Card13", "OZG_Card14", "OZG_Card15", "OZG_Card16", "OZG_Card17", "OZG_Card18", "OZG_Card19", "OZG_Card20",
            "OZG_Card21", "OZG_Card22", "OZG_Card23", "OZG_Card24", "OZG_Card25", "OZG_Card26", "OZG_Card27", "OZG_Card28", "OZG_Card29", "OZG_Card30",
            "OZG_Card31", "OZG_Card32", "OZG_Card33", "OZG_Card34", "OZG_Card35", "OZG_Card36", "OZG_Card37", "OZG_Card38", "OZG_Card39", "OZG_Card40",
            "OZG_Card41", "OZG_Card42", "OZG_Card43", "OZG_Card44", "OZG_Card45", "OZG_Card46", "OZG_Card47", "OZG_Card48", "OZG_Card49", "OZG_Card50",
            "OZG_Card51", "OZG_Card52", "OZG_Card53", "OZG_Card54", "OZG_Card55", "OZG_Card56", "OZG_Card57", "OZG_Card58", "OZG_Card59", "OZG_Card60",
            "OZG_Card61", "OZG_Card62", "OZG_Card63", "OZG_Card64"
        };
    };
    class OZG_Card_2s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2S"; ozgFace = "2s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_2s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_3s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3S"; ozgFace = "3s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_3s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_4s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4S"; ozgFace = "4s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_4s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_5s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5S"; ozgFace = "5s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_5s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_6s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6S"; ozgFace = "6s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_6s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_7s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7S"; ozgFace = "7s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_7s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_8s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8S"; ozgFace = "8s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_8s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_9s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9S"; ozgFace = "9s"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_9s_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Ts: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TS"; ozgFace = "Ts"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Ts_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Js: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JS"; ozgFace = "Js"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Js_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Qs: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QS"; ozgFace = "Qs"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Qs_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Ks: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KS"; ozgFace = "Ks"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Ks_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_As: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AS"; ozgFace = "As"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_As_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_2h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2H"; ozgFace = "2h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_2h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_3h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3H"; ozgFace = "3h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_3h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_4h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4H"; ozgFace = "4h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_4h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_5h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5H"; ozgFace = "5h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_5h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_6h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6H"; ozgFace = "6h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_6h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_7h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7H"; ozgFace = "7h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_7h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_8h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8H"; ozgFace = "8h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_8h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_9h: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9H"; ozgFace = "9h"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_9h_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Th: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TH"; ozgFace = "Th"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Th_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Jh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JH"; ozgFace = "Jh"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Jh_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Qh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QH"; ozgFace = "Qh"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Qh_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Kh: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KH"; ozgFace = "Kh"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Kh_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Ah: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AH"; ozgFace = "Ah"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Ah_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_2d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2D"; ozgFace = "2d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_2d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_3d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3D"; ozgFace = "3d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_3d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_4d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4D"; ozgFace = "4d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_4d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_5d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5D"; ozgFace = "5d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_5d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_6d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6D"; ozgFace = "6d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_6d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_7d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7D"; ozgFace = "7d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_7d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_8d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8D"; ozgFace = "8d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_8d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_9d: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9D"; ozgFace = "9d"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_9d_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Td: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TD"; ozgFace = "Td"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Td_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Jd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JD"; ozgFace = "Jd"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Jd_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Qd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QD"; ozgFace = "Qd"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Qd_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Kd: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KD"; ozgFace = "Kd"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Kd_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Ad: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AD"; ozgFace = "Ad"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Ad_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_2c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_2C"; ozgFace = "2c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_2c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_3c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_3C"; ozgFace = "3c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_3c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_4c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_4C"; ozgFace = "4c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_4c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_5c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_5C"; ozgFace = "5c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_5c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_6c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_6C"; ozgFace = "6c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_6c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_7c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_7C"; ozgFace = "7c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_7c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_8c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_8C"; ozgFace = "8c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_8c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_9c: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_9C"; ozgFace = "9c"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_9c_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Tc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_TC"; ozgFace = "Tc"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Tc_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Jc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_JC"; ozgFace = "Jc"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Jc_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Qc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_QC"; ozgFace = "Qc"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Qc_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Kc: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_KC"; ozgFace = "Kc"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Kc_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Ac: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_AC"; ozgFace = "Ac"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\classic\ozg_classic_Ac_co.paa", "OpenZone_Games\data\cards\classic\ozg_classic_back_co.paa"}; };
    class OZG_Card_Xr: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_XR"; ozgFace = "Xr"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\stalker\ozg_stalker_Xr_co.paa", "OpenZone_Games\data\cards\stalker\ozg_stalker_back_co.paa"}; };
    class OZG_Card_Xb: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_XB"; ozgFace = "Xb"; hiddenSelectionsTextures[] = {"OpenZone_Games\data\cards\stalker\ozg_stalker_Xb_co.paa", "OpenZone_Games\data\cards\stalker\ozg_stalker_back_co.paa"}; };
// <<< generated: cards

    // ------------------------------------------------------------ the card sharp's kit
    //
    // It works through a recipe of the game's own crafting (OZG_Recipes.c):
    // the kit in the hands and a card the player carries. Until 2026-11-25
    // it was a pen, the game's own model as a stand-in; now it is an open tin
    // of handmade things, a model of its own (models/README.md), which stands
    // in its model as the game's TunaCan stands and borrows its pose in the
    // hand (OZG_Hand.c). The class keeps its name: kits already in a world
    // and in the servers' spawn lists stay what they were. The loupe is gone
    // (2026-11-25): a card held in the hands is studied closely by an action
    // of its own (OZG_Study.c).
    class OZG_MarkTool: Inventory_Base
    {
        scope = 2;
        displayName = "$STR_OZG_MARKTOOL_NAME";
        descriptionShort = "$STR_OZG_MARKTOOL_DESC";
        model = "\OpenZone_Games\models\sharpkit\ozg_sharpkit.p3d";
        animClass = "Knife";
        itemSize[] = {2, 1};
        // 0: it stands on the ground as it stands in its model, open side up,
        // as the game's Pot does, which says nothing. 32, the TunaCan's,
        // turned it upside down (seen 2026-11-25).
        rotationFlags = 0;
        weight = 200;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\models\sharpkit\data\ozg_sharpkit_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\sharpkit\data\ozg_sharpkit.rvmat"};
        // Its uses as the sewing kit counts them, a percentage: a use is
        // 100 / MarkUses of it, and an empty tool is gone (OZG_Recipes.Spend).
        stackedUnit = "percentage";
        quantityBar = 1;
        varQuantityInit = 100;
        varQuantityMin = 0;
        varQuantityMax = 100;
    };

    // ------------------------------------------------------------ the board
    //
    // The "3 in 1" board (boards and dice design §3): a set as a deck is one,
    // and in script a deck (OZG_Board_Base extends OZG_Deck_Base), so laying it
    // out, its place and its way back are the deck's. It holds its pieces in
    // slots of their own and is laid out as a place of its own class.
    class OZG_Board_Base: Inventory_Base
    {
        scope = 0;
        displayName = "$STR_OZG_BOARD_NAME";
        descriptionShort = "$STR_OZG_BOARD_DESC";
        model = "\OpenZone_Games\models\board\ozg_board.p3d";
        itemSize[] = {5, 3};
        weight = 1500;
        rotationFlags = 0;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\models\board\data\ozg_board_co.paa"};
        projectionTypename = "OZG_BoardPlacePlacing";
        ozgStyle = "classic";
        ozgSetKind = "board";
        ozgPlace = "OZG_BoardPlace";
        // A laid-out game keeps its board in this slot of the place.
        inventorySlot[] = {"OZG_DeckHold"};
// >>> generated: board attachments
        attachments[] =
        {
            "OZG_Bd_wK", "OZG_Bd_wQ", "OZG_Bd_wR1", "OZG_Bd_wR2", "OZG_Bd_wB1", "OZG_Bd_wB2", "OZG_Bd_wN1", "OZG_Bd_wN2",
            "OZG_Bd_wP1", "OZG_Bd_wP2", "OZG_Bd_wP3", "OZG_Bd_wP4", "OZG_Bd_wP5", "OZG_Bd_wP6", "OZG_Bd_wP7", "OZG_Bd_wP8",
            "OZG_Bd_bK", "OZG_Bd_bQ", "OZG_Bd_bR1", "OZG_Bd_bR2", "OZG_Bd_bB1", "OZG_Bd_bB2", "OZG_Bd_bN1", "OZG_Bd_bN2",
            "OZG_Bd_bP1", "OZG_Bd_bP2", "OZG_Bd_bP3", "OZG_Bd_bP4", "OZG_Bd_bP5", "OZG_Bd_bP6", "OZG_Bd_bP7", "OZG_Bd_bP8",
            "OZG_Bd_wD01", "OZG_Bd_wD02", "OZG_Bd_wD03", "OZG_Bd_wD04", "OZG_Bd_wD05", "OZG_Bd_wD06", "OZG_Bd_wD07", "OZG_Bd_wD08",
            "OZG_Bd_wD09", "OZG_Bd_wD10", "OZG_Bd_wD11", "OZG_Bd_wD12", "OZG_Bd_wD13", "OZG_Bd_wD14", "OZG_Bd_wD15", "OZG_Bd_bD01",
            "OZG_Bd_bD02", "OZG_Bd_bD03", "OZG_Bd_bD04", "OZG_Bd_bD05", "OZG_Bd_bD06", "OZG_Bd_bD07", "OZG_Bd_bD08", "OZG_Bd_bD09",
            "OZG_Bd_bD10", "OZG_Bd_bD11", "OZG_Bd_bD12", "OZG_Bd_bD13", "OZG_Bd_bD14", "OZG_Bd_bD15", "OZG_Bd_Die1", "OZG_Bd_Die2"
        };
// <<< generated: board attachments
    };
    class OZG_Board_Classic: OZG_Board_Base
    {
        scope = 2;
    };

    // The pieces: one model a kind, the colour put on through the hidden
    // selection; every class names the board's slots it goes into.
    class OZG_Piece_Base: Inventory_Base
    {
        scope = 0;
        itemSize[] = {1, 1};
        weight = 20;
        rotationFlags = 0;
        hiddenSelections[] = {"camo"};
    };
    class OZG_Chess_Base: OZG_Piece_Base
    {
        scope = 0;
        descriptionShort = "$STR_OZG_PIECE_DESC";
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\chess\data\ozg_chess.rvmat"};
    };
    class OZG_Chess_K: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_k.p3d"; weight = 35; };
    class OZG_Chess_Q: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_q.p3d"; weight = 30; };
    class OZG_Chess_R: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_r.p3d"; weight = 25; };
    class OZG_Chess_B: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_b.p3d"; weight = 22; };
    class OZG_Chess_N: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_n.p3d"; weight = 22; };
    class OZG_Chess_P: OZG_Chess_Base { scope = 0; model = "\OpenZone_Games\models\chess\ozg_chess_p.p3d"; weight = 15; };
    class OZG_Disc_Base: OZG_Piece_Base
    {
        scope = 0;
        descriptionShort = "$STR_OZG_DISC_DESC";
        model = "\OpenZone_Games\models\disc\ozg_disc.p3d";
        weight = 10;
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\disc\data\ozg_disc.rvmat"};
    };
    class OZG_Die_Base: OZG_Piece_Base
    {
        scope = 0;
        descriptionShort = "$STR_OZG_DIE_DESC";
        model = "\OpenZone_Games\models\die\ozg_die.p3d";
        weight = 5;
        hiddenSelectionsTextures[] = {"OpenZone_Games\models\die\data\ozg_die_co.paa"};
    };
// >>> generated: board pieces
    class OZG_Chess_Classic_wK: OZG_Chess_K { scope = 2; displayName = "$STR_OZG_PIECE_WK"; ozgFace = "wK"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wK"}; };
    class OZG_Chess_Classic_wQ: OZG_Chess_Q { scope = 2; displayName = "$STR_OZG_PIECE_WQ"; ozgFace = "wQ"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wQ"}; };
    class OZG_Chess_Classic_wR: OZG_Chess_R { scope = 2; displayName = "$STR_OZG_PIECE_WR"; ozgFace = "wR"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wR1", "OZG_Bd_wR2"}; };
    class OZG_Chess_Classic_wB: OZG_Chess_B { scope = 2; displayName = "$STR_OZG_PIECE_WB"; ozgFace = "wB"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wB1", "OZG_Bd_wB2"}; };
    class OZG_Chess_Classic_wN: OZG_Chess_N { scope = 2; displayName = "$STR_OZG_PIECE_WN"; ozgFace = "wN"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wN1", "OZG_Bd_wN2"}; };
    class OZG_Chess_Classic_wP: OZG_Chess_P { scope = 2; displayName = "$STR_OZG_PIECE_WP"; ozgFace = "wP"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wP1", "OZG_Bd_wP2", "OZG_Bd_wP3", "OZG_Bd_wP4", "OZG_Bd_wP5", "OZG_Bd_wP6", "OZG_Bd_wP7", "OZG_Bd_wP8"}; };
    class OZG_Disc_Classic_W: OZG_Disc_Base { scope = 2; displayName = "$STR_OZG_PIECE_WD"; ozgFace = "wD"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\disc\data\ozg_disc_white_co.paa"}; inventorySlot[] = {"OZG_Bd_wD01", "OZG_Bd_wD02", "OZG_Bd_wD03", "OZG_Bd_wD04", "OZG_Bd_wD05", "OZG_Bd_wD06", "OZG_Bd_wD07", "OZG_Bd_wD08", "OZG_Bd_wD09", "OZG_Bd_wD10", "OZG_Bd_wD11", "OZG_Bd_wD12", "OZG_Bd_wD13", "OZG_Bd_wD14", "OZG_Bd_wD15"}; };
    class OZG_Chess_Classic_bK: OZG_Chess_K { scope = 2; displayName = "$STR_OZG_PIECE_BK"; ozgFace = "bK"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bK"}; };
    class OZG_Chess_Classic_bQ: OZG_Chess_Q { scope = 2; displayName = "$STR_OZG_PIECE_BQ"; ozgFace = "bQ"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bQ"}; };
    class OZG_Chess_Classic_bR: OZG_Chess_R { scope = 2; displayName = "$STR_OZG_PIECE_BR"; ozgFace = "bR"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bR1", "OZG_Bd_bR2"}; };
    class OZG_Chess_Classic_bB: OZG_Chess_B { scope = 2; displayName = "$STR_OZG_PIECE_BB"; ozgFace = "bB"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bB1", "OZG_Bd_bB2"}; };
    class OZG_Chess_Classic_bN: OZG_Chess_N { scope = 2; displayName = "$STR_OZG_PIECE_BN"; ozgFace = "bN"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bN1", "OZG_Bd_bN2"}; };
    class OZG_Chess_Classic_bP: OZG_Chess_P { scope = 2; displayName = "$STR_OZG_PIECE_BP"; ozgFace = "bP"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\chess\data\ozg_chess_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bP1", "OZG_Bd_bP2", "OZG_Bd_bP3", "OZG_Bd_bP4", "OZG_Bd_bP5", "OZG_Bd_bP6", "OZG_Bd_bP7", "OZG_Bd_bP8"}; };
    class OZG_Disc_Classic_B: OZG_Disc_Base { scope = 2; displayName = "$STR_OZG_PIECE_BD"; ozgFace = "bD"; hiddenSelectionsTextures[] = {"OpenZone_Games\models\disc\data\ozg_disc_black_co.paa"}; inventorySlot[] = {"OZG_Bd_bD01", "OZG_Bd_bD02", "OZG_Bd_bD03", "OZG_Bd_bD04", "OZG_Bd_bD05", "OZG_Bd_bD06", "OZG_Bd_bD07", "OZG_Bd_bD08", "OZG_Bd_bD09", "OZG_Bd_bD10", "OZG_Bd_bD11", "OZG_Bd_bD12", "OZG_Bd_bD13", "OZG_Bd_bD14", "OZG_Bd_bD15"}; };
    class OZG_Die: OZG_Die_Base { scope = 2; displayName = "$STR_OZG_PIECE_DI"; ozgFace = "Di"; inventorySlot[] = {"OZG_Bd_Die1", "OZG_Bd_Die2"}; };
// <<< generated: board pieces

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

    // The board laid out (boards and dice design §4): the unfolded board on
    // the table, the chess side up. A place in everything (OZG_Place's script,
    // slot and cargo); only the model is its own.
    class OZG_BoardPlace: OZG_Place
    {
        scope = 2;
        displayName = "$STR_OZG_BOARD_PLACE_NAME";
        model = "\OpenZone_Games\models\board\ozg_board_open.p3d";
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"OpenZone_Games\models\board\data\ozg_board_co.paa"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\board\data\ozg_board.rvmat"};
        // The field up: chess, draughts and corners, or nardy (OZG_BoardPlace).
        ozgFieldChess = "OpenZone_Games\models\board\data\ozg_board_co.paa";
        ozgFieldNardy = "OpenZone_Games\models\board\data\ozg_board_nardy_co.paa";
    };
    class OZG_BoardPlacePlacing: OZG_PlacePlacing
    {
        scope = 1;
        model = "\OpenZone_Games\models\board\ozg_board_open.p3d";
        hologramMaterial = "ozg_board";
        hologramMaterialPath = "OpenZone_Games\models\board\data";
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

// ------------------------------------------------------------ sounds
//
// The sounds of the table (design §17), every one made from numbers by
// tools/make_sounds.py (art/sounds/SOURCES.md), a mono .ogg in
// OpenZone_Games/sounds. A shader names its file without the extension, as
// the game's own do, and carries 15 m (the design's number); a set is what a
// client plays on the place (OZG_Sounds). The text between the markers is the
// script's: edit the script, then run it.
// >>> generated: sounds
class CfgSoundShaders
{
    class baseCharacter_SoundShader;
    class OZG_Shuffle_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_shuffle", 1}};
        volume = 0.45;
        range = 15;
    };
    class OZG_Deal_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_deal", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Trump_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_trump", 1}};
        volume = 0.35;
        range = 15;
    };
    class OZG_Lay_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_lay", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Beat_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_beat", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Take_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_take", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Sweep_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_sweep", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Draw_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_draw", 1}};
        volume = 0.35;
        range = 15;
    };
    class OZG_PutDown_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_putdown", 1}};
        volume = 0.45;
        range = 15;
    };
    class OZG_PickUp_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_pickup", 1}};
        volume = 0.40;
        range = 15;
    };
    class OZG_Slip_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_slip", 1}};
        volume = 0.30;
        range = 15;
    };
    class OZG_Swap_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_swap", 1}};
        volume = 0.30;
        range = 15;
    };
    class OZG_Toss_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_toss", 1}};
        volume = 0.30;
        range = 15;
    };
    class OZG_DeckIn_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_deckin", 1}};
        volume = 0.30;
        range = 15;
    };
    class OZG_DeckOut_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_deckout", 1}};
        volume = 0.30;
        range = 15;
    };
    class OZG_Pick_SoundShader: baseCharacter_SoundShader
    {
        samples[] = {{"OpenZone_Games\sounds\ozg_pick", 1}};
        volume = 0.30;
        range = 15;
    };
};
class CfgSoundSets
{
    class baseCharacter_SoundSet;
    class OZG_Shuffle_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Shuffle_SoundShader"};
    };
    class OZG_Deal_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Deal_SoundShader"};
    };
    class OZG_Trump_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Trump_SoundShader"};
    };
    class OZG_Lay_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Lay_SoundShader"};
    };
    class OZG_Beat_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Beat_SoundShader"};
    };
    class OZG_Take_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Take_SoundShader"};
    };
    class OZG_Sweep_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Sweep_SoundShader"};
    };
    class OZG_Draw_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Draw_SoundShader"};
    };
    class OZG_PutDown_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_PutDown_SoundShader"};
    };
    class OZG_PickUp_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_PickUp_SoundShader"};
    };
    class OZG_Slip_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Slip_SoundShader"};
    };
    class OZG_Swap_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Swap_SoundShader"};
    };
    class OZG_Toss_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Toss_SoundShader"};
    };
    class OZG_DeckIn_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_DeckIn_SoundShader"};
    };
    class OZG_DeckOut_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_DeckOut_SoundShader"};
    };
    class OZG_Pick_SoundSet: baseCharacter_SoundSet
    {
        soundShaders[] = {"OZG_Pick_SoundShader"};
    };
};
// <<< generated: sounds
