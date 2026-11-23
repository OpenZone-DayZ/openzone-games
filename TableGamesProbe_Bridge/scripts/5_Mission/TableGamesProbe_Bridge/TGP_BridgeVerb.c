// The `tgp` verb, following the bridge's own contract for project verbs
// (DZMCP_BridgeCore.c, "HOW A PROJECT ADDS ITS OWN VERB"): the name is added to
// KnownVerbs and IsKnownVerb and routed in Dispatch. Every op answers within
// its tick; the long ones only start a job there (TGP_Verb.c).
modded class DZMCP_BridgeCore
{
    override protected string KnownVerbs()
    {
        return super.KnownVerbs() + ", tgp";
    }

    override protected bool IsKnownVerb(string verb)
    {
        if (verb == "tgp")
            return true;
        return super.IsKnownVerb(verb);
    }

    override protected void Dispatch(string verb, string raw)
    {
        if (verb != "tgp")
        {
            super.Dispatch(verb, raw);
            return;
        }

        DZMCP_CommandFull full = new DZMCP_CommandFull();
        string parseError;
        if (!m_Json.ReadFromString(full, raw, parseError))
        {
            FinishCommand(DZMCP_STATUS_FAILED, "tgp: the args block could not be parsed -- every value must be a string: " + Excerpt(parseError));
            return;
        }

        map<string, string> args = full.args;
        string op = "status";
        string given;
        if (args && args.Find("op", given))
            op = given;

        string detail;
        if (TGP_Verb.Run(op, args, detail))
            FinishCommand(DZMCP_STATUS_DONE, detail);
        else
            FinishCommand(DZMCP_STATUS_FAILED, detail);
    }
}
