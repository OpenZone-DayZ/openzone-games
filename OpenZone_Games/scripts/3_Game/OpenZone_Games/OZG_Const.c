// Constants of OpenZone Games: everything the mod names in the log or on the
// wire, so that a word typed in two files cannot drift apart. What belongs to
// one game alone stands with that game (OZG_DurakConst).
class OZG_Const
{
    static const string LOG_PREFIX = "[OpenZone/Games] ";

    // What a session carries from one game to the next deal (OZG_Game.HintInt):
    // the host's seat, and whatever the game that ended adds itself.
    static const string HINT_HOST = "host";

    // Why a move or a deal was refused: KEYS of the string table, never ready
    // text. The client knows the player's language, the server does not.
    static const string NO_GAME    = "STR_OZG_NO_GAME";
    static const string NO_PLAYERS = "STR_OZG_NO_PLAYERS";
    static const string NO_OVER    = "STR_OZG_NO_OVER";
    static const string NO_SEAT    = "STR_OZG_NO_SEAT";
    static const string NO_MOVE    = "STR_OZG_NO_MOVE";
    static const string NO_CARD    = "STR_OZG_NO_CARD";
    static const string NO_CARDS   = "STR_OZG_NO_CARDS";

    // ---------------------------------------------------------------- laying out

    // How the seats of a place are chosen: the mod's guess, or the placer's
    // word. Chosen while aiming and at no other time.
    //
    // Three layouts (the owner, 2026-11-24): STANDING and SITTING put the
    // seats on the floor BELOW the mat, around the raised thing it lies on,
    // and differ in what the players do there; FLOOR puts them on the level
    // of the mat itself. The numbers are written to the world's storage and to
    // the session files: 2 was "on the ground" until that day and meant what
    // FLOOR means, so nothing saved changes its meaning.
    static const int MODE_AUTO     = 0;
    static const int MODE_STANDING = 1;
    static const int MODE_FLOOR    = 2;
    static const int MODE_SITTING  = 3;
    static const int MODE_LAST     = 3;

    // What a player does at a seat: the family of poses. 1 was "ground" until
    // 2026-11-24; the poses are the same ones.
    static const int KIND_STANDING = 0;
    static const int KIND_SITTING  = 1;

    // The mat's collision box, the one the measurements of 2026-11-23 used.
    static const float MAT_HALF_X = 0.25;
    static const float MAT_HALF_Z = 0.175;
    static const float MAT_HEIGHT = 0.03;

    // Why a spot is refused while aiming, and by the server's second look.
    static const string LAY_NO_PLAYER  = "STR_OZG_LAY_NO_PLAYER";
    static const string LAY_NO_REACH   = "STR_OZG_LAY_NO_REACH";
    static const string LAY_NO_WAY     = "STR_OZG_LAY_NO_WAY";
    static const string LAY_NO_BASE    = "STR_OZG_LAY_NO_BASE";
    static const string LAY_NO_HEIGHT  = "STR_OZG_LAY_NO_HEIGHT";
    static const string LAY_NO_WATER   = "STR_OZG_LAY_NO_WATER";
    static const string LAY_NO_SPACING = "STR_OZG_LAY_NO_SPACING";
    static const string LAY_NO_ROOM    = "STR_OZG_LAY_NO_ROOM";
    static const string LAY_NO_PERMIT  = "STR_OZG_LAY_NO_PERMIT";

    // ---------------------------------------------------------------- the session

    static const int SEAT_FREE  = 0;
    static const int SEAT_TAKEN = 1;
    static const int SEAT_KEPT  = 2;
    static const int SEAT_LEFT  = 3;

    static const int ST_LOBBY    = 0;
    static const int ST_PLAYING  = 1;
    static const int ST_FINISHED = 2;

    // What happens to the cards of a seat its player leaves in mid-game.
    static const string HOW_DISCARD = "discard";
    static const string HOW_LEAVE   = "leave";

    // The session file: its version, and the longest string in it (the JSON
    // loader's wall is 1023 bytes a string).
    static const int    FILE_VERSION = 1;
    static const int    PART_BYTES   = 900;
    static const string DIR_GAMES    = "$profile:OpenZone\\Games";

    // The place's own block in the world's storage.
    static const int SAVE_VERSION = 1;

    // Long enough never to matter beside the idle rule: the engine's ceiling.
    // A card's class is this and its face: OZG_Card_As.
    static const string CARD_PREFIX = "OZG_Card_";
    // The place's slot for the deck of its game.
    static const string SLOT_HOLD   = "OZG_DeckHold";
    // The block of a card in the world's storage: 1 its style; 2 its mark too.
    static const int CARD_SAVE = 2;
    // How long a place stays open to the session's own hand, ms.
    static const int OPEN_MS = 5000;
    // How long a place laid out by hand waits for its deck, seconds of the
    // mod's clock. Shorter than OPEN_MS.
    static const int DECK_WAIT = 4;
    static const float PLACE_LIFETIME = 3888000.0;

    // Lines of the log a view carries.
    static const int LOG_LINES = 14;

    // ---------------------------------------------------------------- the wire

    static const string SERVICE = "games";

    // From a client.
    static const string OP_OPEN    = "open";
    static const string OP_WATCH   = "watch";
    static const string OP_UNWATCH = "unwatch";
    static const string OP_STAND   = "stand";
    static const string OP_ACT     = "act";
    static const string OP_POSE    = "pose";
    static const string OP_OPT     = "opt";
    static const string OP_DEAL    = "deal";
    static const string OP_KICK    = "kick";
    static const string OP_DROP    = "drop";
    static const string OP_FOLD    = "fold";
    static const string OP_HOST    = "host";
    static const string OP_MINE    = "mine";

    // To a client.
    static const string OP_VIEW   = "view";    // the table as it stands
    static const string OP_SEATS  = "seats";   // the seats to mark in the world
    static const string OP_CLOSED = "closed";  // this place is no longer yours to look at

    // The server's word to open the window (OZ_Rpc.Show), followed by the
    // place's id.
    static const string SHOW_OPEN = "games.open:";

    // The notices of the mod (OZ_Rpc.Notice): every one starts with this.
    static const string NOTICE      = "games.";
    static const string NOTICE_LAY  = "games.lay";
    static const string NOTICE_SIT  = "games.sit";
    static const string NOTICE_SAY  = "games.say";
    static const string NOTICE_LOUPE = "games.loupe";

    // The window. The id MUST BE SMALL: with a large number EnterScriptedMenu
    // returns null and the mission is never asked (measured by the series'
    // other menus). Taken in the series: 131 and 132 the PDA, 133 the link
    // window, 134 the radio's keypad, 135 the research tree, 136 the trade
    // screen.
    static const int MENU_GAME = 137;

    // The key that steps through the modes while aiming (data/inputs.xml).
    static const string INPUT_MODE = "UAOZGMode";

    // What the server says to players, as keys.
    static const string SAY_OTHER_SEAT  = "STR_OZG_SAY_OTHER_SEAT";
    static const string SAY_NOT_NOW     = "STR_OZG_SAY_NOT_NOW";
    static const string SAY_SEAT_TAKEN  = "STR_OZG_SAY_SEAT_TAKEN";
    static const string SAY_TOO_FAR     = "STR_OZG_SAY_TOO_FAR";
    static const string SAY_NOT_HOST    = "STR_OZG_SAY_NOT_HOST";
    static const string SAY_NO_PLACE    = "STR_OZG_SAY_NO_PLACE";
    static const string SAY_NOT_SEATED  = "STR_OZG_SAY_NOT_SEATED";
    static const string SAY_PLAYING     = "STR_OZG_SAY_PLAYING";
    static const string SAY_BAD_OPTION  = "STR_OZG_SAY_BAD_OPTION";
    // What the loupe tells the one who studies a card (design §4.3).
    static const string SAY_LOUPE_MARKED  = "STR_OZG_LOUPE_MARKED";
    static const string SAY_LOUPE_NOTHING = "STR_OZG_LOUPE_NOTHING";
    static const string SAY_BUSY        = "STR_OZG_SAY_BUSY";
    static const string SAY_HOST_HERE   = "STR_OZG_SAY_HOST_HERE";
    static const string SAY_HOW         = "STR_OZG_SAY_HOW";
    static const string SAY_CANNOT      = "STR_OZG_SAY_CANNOT";
    static const string SAY_FOLDED      = "STR_OZG_SAY_FOLDED";
    static const string SAY_IDLE        = "STR_OZG_SAY_IDLE";
    static const string SAY_GONE        = "STR_OZG_SAY_GONE";
    static const string SAY_STOOD       = "STR_OZG_SAY_STOOD";
    static const string SAY_KICKED      = "STR_OZG_SAY_KICKED";
    static const string SAY_BROKEN      = "STR_OZG_SAY_BROKEN";
    static const string SAY_SLOW        = "STR_OZG_SAY_SLOW";
    static const string SAY_NO_SEATS    = "STR_OZG_SAY_NO_SEATS";
    static const string SAY_SAT         = "STR_OZG_SAY_SAT";
    static const string SAY_LAID        = "STR_OZG_SAY_LAID";
}
