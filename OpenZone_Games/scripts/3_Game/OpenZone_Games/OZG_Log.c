// The mod's log lines.
//
// A prefix of its own: in a server log with thirty mods a line must say whose
// it is, and the stand's verdict looks for this prefix. WARNING and ERROR are
// written in full: the verdict counts the first as a whole word and matches
// `[OpenZone/Games] ERROR` (error_regex in dayz-mcp.toml).
//
// The debug switch is the core's: the core is always there (requiredAddons),
// and a second switch would be a second place for the two to disagree.
class OZG_Log
{
    // The warnings written since ResetWarned(): the ready line counts them as
    // `problems=`.
    private static int s_Warned;

    static void Info(string msg)
    {
        Print(OZG_Const.LOG_PREFIX + msg);
    }

    static void Warn(string msg)
    {
        s_Warned++;
        Print(OZG_Const.LOG_PREFIX + "WARNING: " + msg);
    }

    static void ResetWarned()
    {
        s_Warned = 0;
    }

    static int Warned()
    {
        return s_Warned;
    }

    static void Error(string msg)
    {
        Print(OZG_Const.LOG_PREFIX + "ERROR: " + msg);
    }

    static void Dbg(string msg)
    {
        if (!OZ_Log.IsDebug())
            return;
        Print(OZG_Const.LOG_PREFIX + "dbg: " + msg);
    }
}
