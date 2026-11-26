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

    // What a place is laid out from (boards and dice design §2): a game plays
    // with one kind of set, and a set item's config names its kind
    // (ozgSetKind; a deck says nothing and is cards).
    static const string SET_CARDS = "cards";
    static const string SET_BOARD = "board";
    static const string SET_DICE  = "dice";
    // A die's stored record (OZG_Die_Base): its load and who knows of it.
    static const int DIE_SAVE = 1;

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

    // SFP's squat (its EmoteGopStyle, Workshop 2623995836): the gesture of
    // the default "squatting", where SFP runs; elsewhere the pose crouches.
    static const int GESTURE_SFP_SQUAT = 65008;

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
    // The later games (stakes-and-games design §3): the host picks the game;
    // a seated player asks for chips; the host lets a player in, refuses
    // him, or gives a seat chips.
    static const string OP_GAME   = "game";
    static const string OP_REBUY  = "rebuy";
    static const string OP_LET    = "let";
    static const string OP_REFUSE = "refuse";
    static const string OP_CHIPS  = "chips";
    // The most chips one buy-in or rebuy may bring.
    static const int STAKE_MOST = 1000000;

    // To a client.
    static const string OP_VIEW   = "view";    // the table as it stands
    static const string OP_SEATS  = "seats";   // the seats to mark in the world
    static const string OP_CLOSED = "closed";  // this place is no longer yours to look at
    // A sleight was seen (card sharp's design §6.1, §9): a sound for this
    // player alone and a small shake of the part of the table where the cards
    // changed.
    static const string OP_SHAKE  = "shake";
    // A sleight was done (to the one who did it): which cards came into his
    // hand and went out of it, for his window to fly them, and its sound.
    static const string OP_SLEIGHT = "sleight";

    // ---------------------------------------------------------------- sleights
    //
    // From a client (card sharp's design §9): a card of the pockets slipped
    // into the hand, a card of the hand swapped for one of the pockets, up to
    // two cards of the hand thrown to the discards. The dealer's three ride
    // in the deal itself (OZG_WAsk.Stack).
    static const string OP_SLIP  = "slip";
    static const string OP_SWAP  = "swap";
    static const string OP_THROW = "throw";
    // A sleight on the board, a die swapped (asked); a sleight on the board
    // seen (told to whoever has the window open).
    static const string OP_BSLEIGHT = "bsleight";
    static const string OP_DSWAP = "dswap";
    // The card sharp's kit's window (boards and dice design §10): what this player
    // carries and knows; a card marked, a die loaded from it.
    static const string OP_KIT_VIEW = "kitview";
    static const string OP_KIT_MARK = "kitmark";
    static const string OP_KIT_LOAD = "kitload";
    // A thing picked in the kit's window: another's mark or load on it opens to him.
    static const string OP_KIT_LOOK = "kitlook";
    static const string OP_BSAW = "bsaw";

    // The kinds of sleight, as the server's log, the settings (CaughtOf) and
    // the doer's message name them.
    static const string SLEIGHT_SLIP     = "slip";
    static const string SLEIGHT_SWAP     = "swap";
    static const string SLEIGHT_THROW    = "throw";
    static const string SLEIGHT_DECK_IN  = "deckin";
    static const string SLEIGHT_DECK_OUT = "deckout";
    static const string SLEIGHT_PICK     = "pick";

    // How a throw's second card says it belongs to the first (OZG_WAsk.How).
    static const string HOW_MORE = "more";
    // Cards one throw may send to the discards.
    static const int THROW_MOST = 2;
    // A card taken out of the deck by chance, in a stack's "out=" list.
    static const string ANY_CARD = "*";

    // Why a sleight was refused: keys of the string table.
    static const string SAY_SLEIGHT_OFF     = "STR_OZG_SAY_SLEIGHT_OFF";
    static const string SAY_SLEIGHT_NO_MORE = "STR_OZG_SAY_SLEIGHT_NO_MORE";
    static const string SAY_SLEIGHT_NO_CARD = "STR_OZG_SAY_SLEIGHT_NO_CARD";
    static const string SAY_SLEIGHT_NOT_GAME = "STR_OZG_SAY_SLEIGHT_NOT_GAME";
    static const string SAY_SLEIGHT_NO_ROOM = "STR_OZG_SAY_SLEIGHT_NO_ROOM";
    static const string SAY_BSLEIGHT_WAIT = "STR_OZG_SAY_BSLEIGHT_WAIT";
    static const string SAY_DSWAP_NOT_NOW = "STR_OZG_SAY_DSWAP_NOT_NOW";
    static const string SAY_DSWAP_ONE = "STR_OZG_SAY_DSWAP_ONE";
    static const string SAY_DSWAP_NO_DIE = "STR_OZG_SAY_DSWAP_NO_DIE";
    static const string SAY_DSWAP_SEEN = "STR_OZG_SAY_DSWAP_SEEN";
    static const string SAY_LOAD_NOTICED = "STR_OZG_SAY_LOAD_NOTICED";
    static const string SAY_LOAD_OPENED = "STR_OZG_SAY_LOAD_OPENED";
    static const string SAY_LOAD_DONE = "STR_OZG_SAY_LOAD_DONE";
    static const string SAY_LOAD_BAD = "STR_OZG_SAY_LOAD_BAD";
    static const string SAY_LOAD_NO_KIT = "STR_OZG_SAY_LOAD_NO_KIT";
    static const string SAY_STUDY_DIE_FAIR = "STR_OZG_SAY_STUDY_DIE_FAIR";
    static const string SAY_STUDY_DIE_LOADED = "STR_OZG_SAY_STUDY_DIE_LOADED";
    // The three weights of a load, as words: STR_OZG_LOAD_STEP_1 .. _3.
    static const string LOAD_STEP = "STR_OZG_LOAD_STEP_";

    // The server's word to open the window (OZ_Rpc.Show), followed by the
    // place's id.
    static const string SHOW_OPEN = "games.open:";

    // The notices of the mod (OZ_Rpc.Notice): every one starts with this.
    static const string NOTICE      = "games.";
    static const string NOTICE_LAY  = "games.lay";
    static const string NOTICE_SIT  = "games.sit";
    static const string NOTICE_SAY  = "games.say";
    static const string NOTICE_STUDY = "games.study";
    // Cheating at a board and with dice (boards and dice design §9, §10).
    static const string NOTICE_DSWAP = "games.dswap";
    static const string NOTICE_LOADED = "games.loaded";
    static const string NOTICE_LOAD = "games.load";

    // The window. The id MUST BE SMALL: with a large number EnterScriptedMenu
    // returns null and the mission is never asked (measured by the series'
    // other menus). Taken in the series: 131 and 132 the PDA, 133 the link
    // window, 134 the radio's keypad, 135 the research tree, 136 the trade
    // screen.
    static const int MENU_GAME = 137;
    static const int MENU_KIT = 138;

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
    // What a player who studies a card closely is told (design §4.3; the
    // loupe's work since 2026-11-25).
    static const string SAY_STUDY_MARKED  = "STR_OZG_STUDY_MARKED";
    static const string SAY_STUDY_NOTHING = "STR_OZG_STUDY_NOTHING";
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
    static const string SAY_ASKED       = "STR_OZG_SAY_ASKED";
    static const string SAY_REFUSED     = "STR_OZG_SAY_REFUSED";
    static const string SAY_NO_CHIPS    = "STR_OZG_SAY_NO_CHIPS";
    static const string SAY_BAD_AMOUNT  = "STR_OZG_SAY_BAD_AMOUNT";
    static const string SAY_ASKED_CHIPS = "STR_OZG_SAY_ASKED_CHIPS";
}
