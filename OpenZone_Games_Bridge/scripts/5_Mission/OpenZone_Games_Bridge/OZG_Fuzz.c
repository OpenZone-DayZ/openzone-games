// Whole games played by chance, with the state checked after every move. The
// game itself is asked what may be played (Moves), so this finds: a move the
// game offers and then refuses, a seat waited for with no move, a game that
// waits for nobody, cards that go missing or double, a save that does not load
// back, and a game that never ends.
class OZG_Fuzz
{
    // More moves than any game of the kind takes.
    static const int MOVE_LIMIT = 3000;

    protected static int s_Moves;
    protected static int s_Longest;
    protected static int s_Drops;
    protected static int s_Skips;
    protected static int s_WithLoser;
    protected static int s_NoLoser;

    // fuzz game=<id> games=<n> players=<n; 0: by chance> opt=<id=value;..>
    //      seed=<n> drop=<per mille of moves> away=<per mille of moves>
    static bool Run(map<string, string> args, out string detail)
    {
        string id = OZG_VerbOps.Arg(args, "game", "durak");
        if (!OZG_Games.Sample(id))
        {
            detail = "no game '" + id + "' is registered; known: " + OZG_Bench.KnownGames();
            return false;
        }
        int games = OZG_VerbOps.Arg(args, "games", "20").ToInt();
        if (games < 1)
            games = 1;
        if (games > 500)
            games = 500;
        int players = OZG_VerbOps.Arg(args, "players", "0").ToInt();
        int seed = OZG_VerbOps.Arg(args, "seed", "1").ToInt();
        int dropRate = OZG_VerbOps.Arg(args, "drop", "0").ToInt();
        int awayRate = OZG_VerbOps.Arg(args, "away", "0").ToInt();
        map<string, string> options = new map<string, string>();
        OZG_VerbOps.Pairs(OZG_VerbOps.Arg(args, "opt", ""), options);

        s_Moves = 0;
        s_Longest = 0;
        s_Drops = 0;
        s_Skips = 0;
        s_WithLoser = 0;
        s_NoLoser = 0;

        OZG_BenchSeeded chance = new OZG_BenchSeeded(seed);
        int started = TickCount(0);
        for (int g = 0; g < games; g++)
        {
            string defect = PlayOne(id, players, options, chance, dropRate, awayRate);
            if (defect != "")
            {
                detail = "fuzz: DEFECT in game " + g.ToString() + " of seed " + seed.ToString() + ": " + defect;
                return true;
            }
        }
        int ms = TickCount(started) / 10000;

        detail = "fuzz: " + games.ToString() + " games, " + s_Moves.ToString() + " moves";
        detail = detail + ", longest " + s_Longest.ToString();
        detail = detail + ", drops " + s_Drops.ToString() + ", skipped " + s_Skips.ToString();
        detail = detail + ", with a loser " + s_WithLoser.ToString() + ", without " + s_NoLoser.ToString();
        detail = detail + ", 0 defects, " + ms.ToString() + " ms";
        return true;
    }

    // One game from its deal to its end. Returns "" or what went wrong.
    protected static string PlayOne(string id, int players, map<string, string> options, OZG_BenchSeeded chance, int dropRate, int awayRate)
    {
        OZG_Game game = OZG_Games.Create(id);
        int n = players;
        if (n <= 0)
            n = game.MinPlayers() + chance.Below(game.MaxPlayers() - game.MinPlayers() + 1);
        array<int> seats = new array<int>();
        int i;
        for (i = 0; i < n; i++)
            seats.Insert(i);

        string refusal;
        if (!game.Begin(seats, options, chance, "host=0", refusal))
            return "the deal for " + n.ToString() + " was refused with " + refusal;

        array<int> absent = new array<int>();
        array<int> movers = new array<int>();
        array<string> offered = new array<string>();
        array<string> can = new array<string>();
        string why;
        int moves = 0;
        int turns = 0;
        while (!game.IsOver())
        {
            turns++;
            if (turns > MOVE_LIMIT)
                return "the game does not end: " + game.Brief();
            if (!game.Sound(why))
                return "the state does not add up (" + why + "): " + game.Brief();

            // Who is waited for, and what everybody may play.
            movers.Clear();
            offered.Clear();
            int waited = 0;
            for (i = 0; i < n; i++)
            {
                can.Clear();
                game.Moves(i, can);
                if (game.Waits(i))
                {
                    waited++;
                    if (can.Count() == 0)
                        return "seat " + i.ToString() + " is waited for and has no move: " + game.Brief();
                }
                for (int m = 0; m < can.Count(); m++)
                {
                    movers.Insert(i);
                    offered.Insert(can[m]);
                }
            }
            if (waited == 0)
                return "the game waits for nobody: " + game.Brief();

            // Now and then a seat is dropped, or a player leaves or comes back.
            if (dropRate > 0 && chance.Below(1000) < dropRate)
            {
                game.DropSeat(chance.Below(n));
                s_Drops++;
                continue;
            }
            if (awayRate > 0 && chance.Below(1000) < awayRate)
            {
                absent.Clear();
                int who = chance.Below(n + 1);
                if (who < n)
                    absent.Insert(who);
            }

            int pick = chance.Below(offered.Count());
            string before = game.Brief();
            if (!game.Act(movers[pick], offered[pick], refusal))
            {
                string refused = "seat " + movers[pick].ToString() + " was offered " + offered[pick];
                return refused + " and refused with " + refusal + ": " + before;
            }
            s_Skips = s_Skips + game.SkipAbsent(absent);
            moves++;

            // Every tenth move the game is saved and loaded into a fresh one,
            // which must read the same and is played on.
            if (moves % 10 == 0)
            {
                string saved = game.SaveState();
                OZG_Game copy = game.Fresh();
                if (!copy.LoadState(saved))
                    return "a save did not load back: " + game.Brief();
                if (copy.SaveState() != saved)
                    return "a loaded save reads differently: " + game.Brief();
                game = copy;
            }
        }

        if (!game.Sound(why))
            return "the state does not add up at the end (" + why + "): " + game.Brief();
        s_Moves = s_Moves + moves;
        if (moves > s_Longest)
            s_Longest = moves;
        // An outcome names a seat, or -1 for nobody.
        string outcome = game.Outcome();
        if (outcome.IndexOf("=-1") >= 0)
            s_NoLoser++;
        else
            s_WithLoser++;
        return "";
    }
}
