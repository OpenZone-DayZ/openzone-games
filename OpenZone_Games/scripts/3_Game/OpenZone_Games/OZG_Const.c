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
}
