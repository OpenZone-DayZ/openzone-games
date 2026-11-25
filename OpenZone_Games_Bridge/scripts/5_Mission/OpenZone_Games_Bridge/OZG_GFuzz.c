// The games after durak played by chance (stakes-and-games design §1): a
// string of deals of one game, the hint of each handed to the next, every
// move picked by the game's own bot or by chance among the moves the game
// offers, and the state checked after every move -- the cards (Sound), the
// chips (a game that uses stakes never makes nor loses any), both halves of
// the view, and a save that loads back.
//
//   oz_games {"op":"gfuzz","game":"poker","deals":"200","players":"6",
//             "chips":"1000","seed":"1","opt":"blinds=5-10"}
class OZG_GFuzz
{
    static const int MOVE_LIMIT = 4000;

    static bool Run(map<string, string> args, out string detail)
    {
        string id = OZG_VerbOps.Arg(args, "game", "ochko");
        OZG_Game sample = OZG_Games.Sample(id);
        if (!sample)
        {
            detail = "no game '" + id + "' is registered; known: " + OZG_Bench.KnownGames();
            return false;
        }
        int deals = OZG_VerbOps.Arg(args, "deals", "50").ToInt();
        if (deals < 1)
            deals = 1;
        if (deals > 2000)
            deals = 2000;
        int players = OZG_VerbOps.Arg(args, "players", "0").ToInt();
        if (players < sample.MinPlayers() || players > sample.MaxPlayers())
            players = sample.MinPlayers();
        int chips = OZG_VerbOps.Arg(args, "chips", "1000").ToInt();
        int seed = OZG_VerbOps.Arg(args, "seed", "1").ToInt();
        // Per hundred: how many moves are picked by chance rather than by
        // the game's bot.
        int wild = OZG_VerbOps.Arg(args, "wild", "40").ToInt();
        map<string, string> options = new map<string, string>();
        OZG_VerbOps.Pairs(OZG_VerbOps.Arg(args, "opt", ""), options);

        OZG_Random chance;
        if (seed == 0)
            chance = new OZG_Random();
        else
            chance = new OZG_BenchSeeded(seed);

        array<int> seats = new array<int>();
        array<int> stacks = new array<int>();
        int i;
        for (i = 0; i < players; i++)
        {
            seats.Insert(i);
            stacks.Insert(chips);
        }

        int started = TickCount(0);
        int moves = 0;
        int longest = 0;
        int ended = 0;
        int rebuys = 0;
        string hint = "";
        string last = "";
        for (int d = 0; d < deals; d++)
        {
            OZG_Game game = OZG_Games.Create(id);
            array<int> dealt = new array<int>();
            array<int> amounts = new array<int>();
            for (i = 0; i < seats.Count(); i++)
            {
                // A seat with no chips gets a rebuy, as a host would give.
                if (game.UsesStakes() && stacks[i] <= 0)
                {
                    stacks[i] = chips;
                    rebuys++;
                }
                dealt.Insert(seats[i]);
                amounts.Insert(stacks[i]);
            }
            int total = 0;
            for (i = 0; i < amounts.Count(); i++)
                total = total + amounts[i];
            if (game.UsesStakes())
                game.TakeStacks(dealt, amounts);
            string fullHint = OZG_Const.HINT_HOST + "=0";
            if (hint != "")
                fullHint = hint + ";" + fullHint;
            string refusal;
            if (!game.Begin(dealt, options, chance, fullHint, refusal))
            {
                detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " refused: " + refusal + " hint " + fullHint;
                return true;
            }
            string defect = Check(game, dealt, total);
            if (defect != "")
            {
                detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " after the deal: " + defect;
                return true;
            }
            int steps = 0;
            while (!game.IsOver())
            {
                if (steps >= MOVE_LIMIT)
                {
                    detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " never ends: " + game.Brief();
                    return true;
                }
                int actor = -1;
                for (i = 0; i < dealt.Count(); i++)
                {
                    if (game.Waits(dealt[i]))
                    {
                        actor = dealt[i];
                        break;
                    }
                }
                if (actor < 0)
                {
                    detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " waits for nobody: " + game.Brief();
                    return true;
                }
                array<string> can = new array<string>();
                game.Moves(actor, can);
                if (can.Count() == 0)
                {
                    detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " seat " + actor.ToString() + " is waited for with no move: " + game.Brief();
                    return true;
                }
                string move;
                if (chance.Below(100) < wild)
                    move = can[chance.Below(can.Count())];
                else
                    move = game.BotMove(actor, chance);
                if (move == "")
                    move = can[0];
                if (!game.Act(actor, move, refusal))
                {
                    detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " seat " + actor.ToString() + " move " + move + " offered and refused (" + refusal + "): " + game.Brief();
                    return true;
                }
                last = move;
                steps++;
                moves++;
                defect = Check(game, dealt, total);
                if (defect != "")
                {
                    detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " after " + actor.ToString() + " " + move + ": " + defect;
                    return true;
                }
                if (steps % 7 == 0)
                {
                    defect = Reload(id, game);
                    if (defect != "")
                    {
                        detail = "gfuzz " + id + ": DEFECT deal " + d.ToString() + " reload: " + defect;
                        return true;
                    }
                }
            }
            ended++;
            if (steps > longest)
                longest = steps;
            if (game.UsesStakes())
            {
                for (i = 0; i < dealt.Count(); i++)
                    stacks[i] = game.StackOf(dealt[i]);
            }
            hint = game.Outcome();
        }
        int ms = TickCount(started) / 10000;
        detail = "gfuzz " + id + ": " + ended.ToString() + " deals of " + players.ToString();
        detail = detail + ", " + moves.ToString() + " moves, longest " + longest.ToString();
        if (sample.UsesStakes())
            detail = detail + ", rebuys " + rebuys.ToString();
        detail = detail + ", 0 defects, " + ms.ToString() + " ms; last hint " + hint;
        return true;
    }

    // What is wrong with a game now, or "".
    protected static string Check(OZG_Game game, array<int> dealt, int total)
    {
        string why;
        if (!game.Sound(why))
            return "unsound: " + why + " | " + game.Brief();
        if (game.UsesStakes())
        {
            int sum = 0;
            for (int i = 0; i < dealt.Count(); i++)
            {
                int s = game.StackOf(dealt[i]);
                if (s < 0)
                    return "seat " + dealt[i].ToString() + " has no stack | " + game.Brief();
                sum = sum + s;
            }
            if (sum != total)
                return "chips " + sum.ToString() + " of " + total.ToString() + " | " + game.Brief();
        }
        string pub = game.PublicView();
        if (pub == "" || pub == "{}")
            return "no public view";
        OZG_GPublic gp = new OZG_GPublic();
        string err;
        if (!JsonFileLoader<OZG_GPublic>.LoadData(pub, gp, err))
            return "the public view does not read: " + err;
        for (int k = 0; k < dealt.Count(); k++)
        {
            string own = game.SeatView(dealt[k]);
            OZG_GPrivate gv = new OZG_GPrivate();
            if (!JsonFileLoader<OZG_GPrivate>.LoadData(own, gv, err))
                return "the view of seat " + dealt[k].ToString() + " does not read: " + err;
        }
        return "";
    }

    // The game saved and loaded into a fresh one gives the same state back.
    protected static string Reload(string id, OZG_Game game)
    {
        string saved = game.SaveState();
        OZG_Game copy = OZG_Games.Create(id);
        if (!copy.LoadState(saved))
            return "the save does not load: " + game.Brief();
        if (copy.SaveState() != saved)
            return "the save loads into another state: " + game.Brief();
        return "";
    }
}
