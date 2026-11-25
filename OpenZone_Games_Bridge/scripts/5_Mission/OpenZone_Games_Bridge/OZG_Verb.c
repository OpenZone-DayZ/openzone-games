// The `oz_games` verb, by the bridge's contract for project verbs: the name is
// added to KnownVerbs and IsKnownVerb and routed in Dispatch. Every op answers
// within its tick.
//
//   world_exec verb=oz_games args={"op":"status"}
//
// Every value of args is a string. The answer is one line (the bridge cuts it
// at 400 characters). A refusal by a game is an answer and comes back DONE;
// FAILED is for a command that could not be understood.
class OZG_VerbOps
{
    static bool Run(map<string, string> args, out string detail)
    {
        string op = Arg(args, "op", "status");

        if (op == "status")
        {
            detail = OZG_Module.ReadyLine();
            return true;
        }
        if (op == "deck")
            return OZG_Bench.OpDeck(detail);
        if (op == "names")
            return OZG_Bench.OpNames(detail);
        if (op == "games")
            return OZG_Bench.OpGames(detail);
        if (op == "begin")
            return OZG_Bench.OpBegin(args, detail);
        if (op == "act")
            return OZG_Bench.OpAct(args, detail);
        if (op == "play")
            return OZG_Bench.OpPlay(args, detail);
        if (op == "outcome")
            return OZG_Bench.OpOutcome(detail);
        if (op == "brief")
            return OZG_Bench.OpBrief(detail);
        if (op == "heard")
            return OZG_Bench.OpHeard(detail);
        if (op == "all")
            return OZG_Bench.OpAll(detail);
        if (op == "can")
            return OZG_Bench.OpCan(args, detail);
        if (op == "drop")
            return OZG_Bench.OpDrop(args, detail);
        if (op == "away")
            return OZG_Bench.OpAway(args, detail);
        if (op == "view")
            return OZG_Bench.OpView(args, detail);
        if (op == "save")
            return OZG_Bench.OpSave(detail);
        if (op == "load")
            return OZG_Bench.OpLoad(args, detail);
        if (op == "fuzz")
            return OZG_Fuzz.Run(args, detail);
        if (op == "ring")
            return OZG_WorldOps.OpRing(args, detail);
        if (op == "level")
            return OZG_WorldOps.OpLevel(args, detail);
        if (op == "lay")
            return OZG_WorldOps.OpLay(args, detail);
        if (op == "places")
            return OZG_WorldOps.OpPlaces(detail);
        if (op == "sess")
            return OZG_WorldOps.OpSess(args, detail);
        if (op == "seats")
            return OZG_WorldOps.OpSeats(args, detail);
        if (op == "puppet")
            return OZG_WorldOps.OpPuppet(args, detail);
        if (op == "move")
            return OZG_WorldOps.OpMove(args, detail);
        if (op == "ask")
            return OZG_WorldOps.OpAsk(args, detail);
        if (op == "sit")
            return OZG_WorldOps.OpSit(args, detail);
        if (op == "ticks")
            return OZG_WorldOps.OpTicks(args, detail);
        if (op == "gather")
            return OZG_WorldOps.OpGather(args, detail);
        if (op == "hand")
            return OZG_WorldOps.OpHand(args, detail);
        if (op == "give")
            return OZG_WorldOps.OpGive(args, detail);
        if (op == "chat")
            return OZG_WorldOps.OpChat(args, detail);
        if (op == "bots")
            return OZG_WorldOps.OpBots(args, detail);
        if (op == "auto")
            return OZG_WorldOps.OpAuto(args, detail);
        if (op == "face")
            return OZG_WorldOps.OpFace(args, detail);
        if (op == "cards")
            return OZG_WorldOps.OpCards(args, detail);
        if (op == "pull")
            return OZG_WorldOps.OpPull(args, detail);
        if (op == "undeck")
            return OZG_WorldOps.OpUndeck(args, detail);
        if (op == "held")
            return OZG_WorldOps.OpHeld(args, detail);
        if (op == "stuff")
            return OZG_WorldOps.OpStuff(args, detail);
        if (op == "mark")
            return OZG_WorldOps.OpMark(args, detail);
        if (op == "markof")
            return OZG_WorldOps.OpMarkOf(args, detail);
        if (op == "craft")
            return OZG_WorldOps.OpCraft(args, detail);
        if (op == "reads")
            return OZG_WorldOps.OpReads(args, detail);
        if (op == "loose")
            return OZG_WorldOps.OpLoose(args, detail);
        if (op == "stage")
            return OZG_Stage.Op(args, detail);
        if (op == "noticecfg")
            return OZG_WorldOps.OpNoticeCfg(args, detail);
        if (op == "noticer")
            return OZG_WorldOps.OpNoticer(args, detail);
        if (op == "whole")
            return OZG_WorldOps.OpWhole(args, detail);
        if (op == "redeal")
            return OZG_WorldOps.OpRedeal(args, detail);
        if (op == "notices")
            return OZG_WorldOps.OpNotices(args, detail);
        if (op == "shake")
            return OZG_WorldOps.OpShake(args, detail);
        if (op == "sound")
            return OZG_WorldOps.OpSound(args, detail);
        if (op == "sleightcfg")
            return OZG_WorldOps.OpSleightCfg(args, detail);
        // -- ops of the later phases go above this line --
        detail = "unknown op '" + op + "'; known: " + KnownOps();
        return false;
    }

    static string KnownOps()
    {
        string known = "status, deck, names, games, begin, act, play, outcome, brief, all, can, drop, away, view, save, load, fuzz, heard";
        known = known + ", ring, level, cards, pull, undeck, held, stuff, loose, lay, places, sess, seats, puppet, move, ask, sit, ticks, gather, hand, give, chat, bots, auto, face, stage, mark, markof, craft, reads, noticecfg, noticer, whole, redeal, notices, shake, sound";
        known = known + ", sleightcfg";
        return known;
    }

    // ---- what the ops share ---------------------------------------------------

    static string Arg(map<string, string> args, string key, string fallback)
    {
        if (!args)
            return fallback;
        string v;
        if (args.Find(key, v))
            return v;
        return fallback;
    }

    // "0, 2,5" -> 0 2 5.
    static void Numbers(string text, array<int> into)
    {
        string plain = text;
        plain.Replace(" ", "");
        array<string> parts = new array<string>();
        plain.Split(",", parts);
        for (int i = 0; i < parts.Count(); i++)
            into.Insert(parts[i].ToInt());
    }

    static string NumbersLine(array<int> numbers)
    {
        if (numbers.Count() == 0)
            return "-";
        string text = "";
        for (int i = 0; i < numbers.Count(); i++)
        {
            if (i > 0)
                text = text + ",";
            text = text + numbers[i].ToString();
        }
        return text;
    }

    // "variant=perevodnoy;other=x" -> a map.
    static void Pairs(string text, map<string, string> into)
    {
        array<string> pairs = new array<string>();
        text.Split(";", pairs);
        for (int i = 0; i < pairs.Count(); i++)
        {
            string pair = pairs[i];
            int eq = pair.IndexOf("=");
            if (eq <= 0)
                continue;
            into.Set(pair.Substring(0, eq), pair.Substring(eq + 1, pair.Length() - eq - 1));
        }
    }
}

modded class DZMCP_BridgeCore
{
    override protected string KnownVerbs()
    {
        return super.KnownVerbs() + ", oz_games";
    }

    override protected bool IsKnownVerb(string verb)
    {
        if (verb == "oz_games")
            return true;
        return super.IsKnownVerb(verb);
    }

    override protected void Dispatch(string verb, string raw)
    {
        if (verb != "oz_games")
        {
            super.Dispatch(verb, raw);
            return;
        }

        DZMCP_CommandFull full = new DZMCP_CommandFull();
        string parseError;
        if (!m_Json.ReadFromString(full, raw, parseError))
        {
            FinishCommand(DZMCP_STATUS_FAILED, "oz_games: the args block could not be parsed -- every value must be a string: " + Excerpt(parseError));
            return;
        }

        string detail;
        if (OZG_VerbOps.Run(full.args, detail))
            FinishCommand(DZMCP_STATUS_DONE, detail);
        else
            FinishCommand(DZMCP_STATUS_FAILED, detail);
    }
}
