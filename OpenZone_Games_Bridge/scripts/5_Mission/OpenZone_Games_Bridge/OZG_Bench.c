// Chance that does what it was told: the deck comes out in the order named.
class OZG_BenchFixed : OZG_Random
{
    protected ref array<string> m_Order;

    void OZG_BenchFixed(array<string> order)
    {
        m_Order = new array<string>();
        m_Order.Copy(order);
    }

    // The cards named come first, in the order named; the rest follow in the
    // order they had.
    override void Shuffle(array<string> cards)
    {
        array<string> rest = new array<string>();
        rest.Copy(cards);
        cards.Clear();
        int i;
        for (i = 0; i < m_Order.Count(); i++)
        {
            int at = rest.Find(m_Order[i]);
            if (at < 0)
                continue;
            cards.Insert(m_Order[i]);
            rest.RemoveOrdered(at);
        }
        for (i = 0; i < rest.Count(); i++)
            cards.Insert(rest[i]);
    }
}

// Chance that repeats itself: the same seed gives the same games.
class OZG_BenchSeeded : OZG_Random
{
    protected int m_Seed;

    void OZG_BenchSeeded(int seed)
    {
        m_Seed = seed % 65536;
        if (m_Seed < 0)
            m_Seed = -m_Seed;
    }

    // x' = 75 * (x + 1) - 1 mod 65537: every value of 0..65535 once in 65536
    // steps, and 75 * 65536 stays inside an int.
    override int Below(int n)
    {
        m_Seed = (m_Seed * 75 + 74) % 65537;
        if (n <= 1)
            return 0;
        return m_Seed % n;
    }
}

// The stand's table: one game, played through the contract a session will use.
// Nothing of it is in the world; seats are numbers.
class OZG_Bench
{
    static const string VIEW_FILE = "$profile:OpenZone\\games_view.txt";
    static const string SAVE_FILE = "$profile:OpenZone\\games_save.json";

    static ref OZG_Game s_Game;
    // Seats whose players are "not present": what they need not do is passed
    // over after every change, as a session does (design §6.3).
    static ref array<int> s_Absent = new array<int>();
    // What `save` kept, for `load`.
    static string s_Saved;

    static bool HasGame(out string detail)
    {
        if (s_Game)
            return true;
        detail = "no game on the bench: begin one first";
        return false;
    }

    // deck: the card helpers and the three kinds of chance, each asked what it
    // must answer.
    static bool OpDeck(out string detail)
    {
        array<string> whole = new array<string>();
        OZG_Cards.Whole(whole);
        array<string> six = new array<string>();
        OZG_Cards.FromRank("6", six);
        array<string> parsed = new array<string>();
        OZG_Cards.Parse("6s, 7h zz Qd 10s", parsed);

        int refused = 0;
        if (!OZG_Cards.IsCard(""))
            refused++;
        if (!OZG_Cards.IsCard("6"))
            refused++;
        if (!OZG_Cards.IsCard("6x"))
            refused++;
        if (!OZG_Cards.IsCard("1s"))
            refused++;
        if (!OZG_Cards.IsCard("6ss"))
            refused++;

        detail = "deck: whole=" + whole.Count().ToString() + " " + Ends(whole);
        detail = detail + " from6=" + six.Count().ToString() + " " + Ends(six);
        detail = detail + " rank(Th)=" + OZG_Cards.RankOf("Th").ToString();
        detail = detail + " suit(Th)=" + OZG_Cards.SuitOf("Th");
        detail = detail + " notcards=" + refused.ToString() + "/5";
        detail = detail + " parsed=" + OZG_Cards.Line(parsed);

        // A seeded shuffle keeps every card and moves them; a fixed one puts
        // the named cards first; the real one keeps every card.
        array<string> seeded = new array<string>();
        seeded.Copy(whole);
        OZG_BenchSeeded seven = new OZG_BenchSeeded(7);
        seven.Shuffle(seeded);
        string moved = "same";
        if (OZG_Cards.Line(seeded) != OZG_Cards.Line(whole))
            moved = "moved";
        detail = detail + " seeded=" + Kept(seeded, whole) + "/" + moved;

        array<string> order = new array<string>();
        order.Insert("Ac");
        order.Insert("2s");
        array<string> named = new array<string>();
        named.Copy(whole);
        OZG_BenchFixed told = new OZG_BenchFixed(order);
        told.Shuffle(named);
        detail = detail + " fixed=" + named[0] + " " + named[1] + " " + named[2];

        array<string> real = new array<string>();
        real.Copy(whole);
        OZG_Random dice = new OZG_Random();
        dice.Shuffle(real);
        detail = detail + " real=" + Kept(real, whole);
        return true;
    }

    // names: what a card's name says, the names of further cards of one face
    // with it.
    static bool OpNames(out string detail)
    {
        int wrong = 0;
        if (OZG_Cards.IsCard("As.0"))
            wrong++;
        if (OZG_Cards.IsCard("As.01"))
            wrong++;
        if (OZG_Cards.IsCard("As1"))
            wrong++;
        if (OZG_Cards.IsCard("As.x"))
            wrong++;
        if (OZG_Cards.IsCard("As."))
            wrong++;

        detail = "names: copy(As)=" + OZG_Cards.CopyOf("As").ToString();
        detail = detail + " copy(As.7)=" + OZG_Cards.CopyOf("As.7").ToString();
        detail = detail + " face(As.61)=" + OZG_Cards.FaceOf("As.61");
        detail = detail + " rank(As.61)=" + OZG_Cards.RankOf("As.61").ToString();
        detail = detail + " suit(As.61)=" + OZG_Cards.SuitOf("As.61");
        detail = detail + " named(As,0)=" + OZG_Cards.CopyNamed("As", 0);
        detail = detail + " named(As,3)=" + OZG_Cards.CopyNamed("As", 3);
        detail = detail + " move(a:As)=" + OZG_Cards.OfMove("a:As");
        detail = detail + " move(d:As.3:2)=" + OZG_Cards.OfMove("d:As.3:2");
        detail = detail + " move(take)=[" + OZG_Cards.OfMove("take") + "]";
        detail = detail + " wrong=" + wrong.ToString() + "/5";
        return true;
    }

    // games: every registered game with its numbers and its options; the
    // default value of an option is starred.
    static bool OpGames(out string detail)
    {
        array<string> ids = new array<string>();
        OZG_Games.Ids(ids);
        if (ids.Count() == 0)
        {
            detail = "games: none";
            return true;
        }
        detail = "games:";
        for (int i = 0; i < ids.Count(); i++)
        {
            OZG_Game sample = OZG_Games.Sample(ids[i]);
            array<string> cards = new array<string>();
            sample.CardsUsed(cards);
            detail = detail + " " + ids[i] + " " + sample.MinPlayers().ToString() + "-" + sample.MaxPlayers().ToString();
            detail = detail + " cards=" + cards.Count().ToString();

            array<ref OZG_Option> options = new array<ref OZG_Option>();
            sample.OptionList(options);
            for (int o = 0; o < options.Count(); o++)
            {
                OZG_Option option = options[o];
                detail = detail + " " + option.OptionId + "=";
                for (int v = 0; v < option.ValueIds.Count(); v++)
                {
                    if (v > 0)
                        detail = detail + "|";
                    detail = detail + option.ValueIds[v];
                    if (option.ValueIds[v] == option.DefaultId)
                        detail = detail + "*";
                }
            }
        }
        return true;
    }

    // begin game=<id> seats=<n,n,..> opt=<id=value;..> hint=<key=value;..>
    //       deck=<cards>  the order of the deck, fixed (the cards named first)
    //       seed=<n>      or a shuffle that repeats itself (the default, 1);
    //                     seed=0 is the real chance
    //       cards=<cards> looks=<n,..>  what the deal is dealt from, and their backs
    static bool OpBegin(map<string, string> args, out string detail)
    {
        string id = OZG_VerbOps.Arg(args, "game", "durak");
        OZG_Game game = OZG_Games.Create(id);
        if (!game)
        {
            detail = "no game '" + id + "' is registered; known: " + KnownGames();
            return false;
        }

        array<int> seats = new array<int>();
        OZG_VerbOps.Numbers(OZG_VerbOps.Arg(args, "seats", "0,1"), seats);
        map<string, string> options = new map<string, string>();
        OZG_VerbOps.Pairs(OZG_VerbOps.Arg(args, "opt", ""), options);

        // cards=<cards> looks=<n,n,..>: what the deal is dealt from, as a
        // session hands it over after reading a deck.
        string given = OZG_VerbOps.Arg(args, "cards", "");
        if (given != "")
        {
            array<string> givenNames = new array<string>();
            OZG_Cards.Parse(given, givenNames);
            array<int> givenBacks = new array<int>();
            string lookText = OZG_VerbOps.Arg(args, "looks", "");
            if (lookText != "")
                OZG_VerbOps.Numbers(lookText, givenBacks);
            array<ref OZG_DealCard> handed = new array<ref OZG_DealCard>();
            for (int h = 0; h < givenNames.Count(); h++)
            {
                int back = 0;
                if (h < givenBacks.Count())
                    back = givenBacks[h];
                handed.Insert(OZG_DealCard.Make(givenNames[h], back));
            }
            game.TakeCards(handed);
        }

        OZG_Random chance;
        string deck = OZG_VerbOps.Arg(args, "deck", "");
        string seed = OZG_VerbOps.Arg(args, "seed", "1");
        if (deck != "")
        {
            array<string> order = new array<string>();
            OZG_Cards.Parse(deck, order);
            chance = new OZG_BenchFixed(order);
        }
        else if (seed == "0")
        {
            chance = new OZG_Random();
        }
        else
        {
            chance = new OZG_BenchSeeded(seed.ToInt());
        }

        // first=<seat>:<card> <card>..: the dealer's choice (card sharp's
        // design §6.5), dealt to that seat first.
        string first = OZG_VerbOps.Arg(args, "first", "");
        int colon = first.IndexOf(":");
        if (colon > 0)
        {
            array<string> chosen = new array<string>();
            OZG_Cards.Parse(first.Substring(colon + 1, first.Length() - colon - 1), chosen);
            game.DealFirst(first.Substring(0, colon).ToInt(), chosen);
        }

        // A refusal of the game is an answer, and the game on the bench stays.
        string refusal;
        if (!game.Begin(seats, options, chance, OZG_VerbOps.Arg(args, "hint", ""), refusal))
        {
            detail = "no " + refusal;
            return true;
        }
        s_Game = game;
        s_Absent.Clear();
        s_Saved = "";
        detail = game.Brief();
        return true;
    }

    // act seat=<n> move=<move>
    static bool OpAct(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        string move = OZG_VerbOps.Arg(args, "move", "");
        string refusal;
        if (!s_Game.Act(seat, move, refusal))
        {
            detail = "no " + refusal + " | " + s_Game.Brief();
            return true;
        }
        detail = "ok" + Skipped() + " | " + s_Game.Brief();
        return true;
    }

    // play moves=<seat> <move>; <seat> <move>; ..: several moves in one call.
    // The answer marks each `+`, or `-` and its refusal, then says the brief
    // after the last one.
    static bool OpPlay(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        string moves = OZG_VerbOps.Arg(args, "moves", "");
        array<string> steps = new array<string>();
        moves.Split(";", steps);
        string marks = "";
        int skipped = 0;
        for (int i = 0; i < steps.Count(); i++)
        {
            string step = steps[i].Trim();
            int gap = step.IndexOf(" ");
            if (gap <= 0)
            {
                detail = "play: '" + step + "' is not <seat> <move>";
                return false;
            }
            int seat = step.Substring(0, gap).ToInt();
            string move = step.Substring(gap + 1, step.Length() - gap - 1).Trim();
            string refusal;
            if (i > 0)
                marks = marks + " ";
            if (s_Game.Act(seat, move, refusal))
            {
                marks = marks + "+";
                skipped = skipped + s_Game.SkipAbsent(s_Absent);
            }
            else
            {
                marks = marks + "-" + refusal;
            }
        }
        if (skipped > 0)
            marks = marks + " skipped=" + skipped.ToString();
        detail = marks + " | " + s_Game.Brief();
        return true;
    }

    // cheat seat=<n> kind=in|out|swap|discard [card=<name>] [face=<face>]
    // [look=<n>]: a sleight's change told to the game on the bench as a
    // session tells it (card sharp's design §6.7): `in` a card of `face`
    // joins the hand under a name the deal has not used, `out` the card
    // `card` leaves the hand and the deal, `swap` both at once, `discard` the
    // card goes to the discards. The answer says the names, whether the state
    // adds up, and the brief.
    static bool OpCheat(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        string kind = OZG_VerbOps.Arg(args, "kind", "in");
        string card = OZG_VerbOps.Arg(args, "card", "");
        string face = OZG_VerbOps.Arg(args, "face", "As");
        int look = OZG_VerbOps.Arg(args, "look", "0").ToInt();
        string refusal;
        bool done = false;
        string named = "";
        if (kind == "in" || kind == "swap")
            named = s_Game.SleightName(face);
        OZG_DealCard rec = OZG_DealCard.Make(named, look);
        if (kind == "in")
            done = s_Game.SleightIn(seat, rec, refusal);
        else if (kind == "out")
            done = s_Game.SleightOut(seat, card, refusal);
        else if (kind == "swap")
            done = s_Game.SleightSwap(seat, card, rec, refusal);
        else if (kind == "discard")
            done = s_Game.SleightDiscard(seat, card, refusal);
        else
            refusal = "kind is in, out, swap or discard";
        string why;
        string sound = "yes";
        if (!s_Game.Sound(why))
            sound = "no (" + why + ")";
        detail = kind + " " + card;
        if (named != "")
            detail = detail + " -> " + named;
        if (done)
            detail = detail + ": ok";
        else
            detail = detail + ": no " + refusal;
        detail = detail + " sound=" + sound + " | " + s_Game.Brief();
        return true;
    }

    // outcome: what the game that ended tells the next deal.
    static bool OpOutcome(out string detail)
    {
        if (!HasGame(detail))
            return false;
        string outcome = s_Game.Outcome();
        if (outcome == "")
            outcome = "-";
        detail = "outcome: " + outcome;
        return true;
    }

    static bool OpBrief(out string detail)
    {
        if (!HasGame(detail))
            return false;
        detail = s_Game.Brief();
        return true;
    }

    // heard: what the game on the bench made heard since the last ask (the
    // sounds of design §17, by name), and the list emptied.
    static bool OpHeard(out string detail)
    {
        if (!HasGame(detail))
            return false;
        array<int> ids = new array<int>();
        s_Game.TakeHeard(ids);
        detail = "heard " + OZG_Sounds.NamesOf(ids);
        return true;
    }

    // all: every hand, the table and the stock.
    static bool OpAll(out string detail)
    {
        if (!HasGame(detail))
            return false;
        detail = s_Game.Everything();
        return true;
    }

    // can seat=<n>: is the seat waited for, may it be passed over, and what may
    // it play now.
    static bool OpCan(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        array<string> can = new array<string>();
        s_Game.Moves(seat, can);
        detail = "seat " + seat.ToString() + " waits=" + YesNo(s_Game.Waits(seat));
        detail = detail + " skippable=" + YesNo(s_Game.Skippable(seat));
        detail = detail + " can:";
        for (int i = 0; i < can.Count(); i++)
            detail = detail + " " + can[i];
        if (can.Count() == 0)
            detail = detail + " -";
        return true;
    }

    // drop seat=<n>: the seat is out and its cards are discarded.
    static bool OpDrop(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        s_Game.DropSeat(seat);
        detail = "dropped " + seat.ToString() + Skipped() + " | " + s_Game.Brief();
        return true;
    }

    // away seats=<n,n,..>: the players of these seats are not present; with no
    // seats, everybody is back.
    static bool OpAway(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        s_Absent.Clear();
        OZG_VerbOps.Numbers(OZG_VerbOps.Arg(args, "seats", ""), s_Absent);
        detail = "away: " + OZG_VerbOps.NumbersLine(s_Absent) + Skipped() + " | " + s_Game.Brief();
        return true;
    }

    // view seat=<n>: the two halves as a client at that seat gets them, written
    // to VIEW_FILE one a line; the answer says how big they are.
    static bool OpView(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        string shared = s_Game.PublicView();
        string own = s_Game.SeatView(seat);

        OZ_Json.EnsureTree();
        FileHandle fh = OpenFile(VIEW_FILE, FileMode.WRITE);
        if (fh)
        {
            FPrintln(fh, shared);
            FPrintln(fh, own);
            CloseFile(fh);
        }
        detail = "view: public " + shared.Length().ToString() + " B, seat " + seat.ToString();
        detail = detail + " " + own.Length().ToString() + " B -> games_view.txt";
        return true;
    }

    // save: the whole state is kept for `load` and written to SAVE_FILE.
    static bool OpSave(out string detail)
    {
        if (!HasGame(detail))
            return false;
        s_Saved = s_Game.SaveState();

        OZ_Json.EnsureTree();
        FileHandle fh = OpenFile(SAVE_FILE, FileMode.WRITE);
        if (fh)
        {
            FPrintln(fh, s_Saved);
            CloseFile(fh);
        }
        string why;
        string sound = "yes";
        if (!s_Game.Sound(why))
            sound = "no (" + why + ")";
        detail = "saved " + s_Saved.Length().ToString() + " B sound=" + sound + " -> games_save.json";
        return true;
    }

    // load [swap=<text>><text>]: a fresh game of the same kind loads what
    // `save` kept. With swap=, one piece of that text is replaced first, to
    // see a save that does not add up refused.
    static bool OpLoad(map<string, string> args, out string detail)
    {
        if (!HasGame(detail))
            return false;
        if (s_Saved == "")
        {
            detail = "nothing is saved: save first";
            return false;
        }
        string text = s_Saved;
        string swap = OZG_VerbOps.Arg(args, "swap", "");
        if (swap != "")
        {
            int mark = swap.IndexOf(">");
            if (mark <= 0)
            {
                detail = "swap= is <text>><text>";
                return false;
            }
            string piece = swap.Substring(0, mark);
            string instead = swap.Substring(mark + 1, swap.Length() - mark - 1);
            text.Replace(piece, instead);
        }

        OZG_Game game = s_Game.Fresh();
        if (!game.LoadState(text))
        {
            detail = "not loaded, the game on the bench stays | " + s_Game.Brief();
            return true;
        }
        s_Game = game;
        detail = "loaded | " + s_Game.Brief();
        return true;
    }

    // ---- what the ops share ---------------------------------------------------

    // What the absent seats need not do is passed over, as a session does
    // after every change.
    static string Skipped()
    {
        int skipped = s_Game.SkipAbsent(s_Absent);
        if (skipped == 0)
            return "";
        return " skipped=" + skipped.ToString();
    }

    static string KnownGames()
    {
        array<string> ids = new array<string>();
        OZG_Games.Ids(ids);
        if (ids.Count() == 0)
            return "none";
        string text = "";
        for (int i = 0; i < ids.Count(); i++)
        {
            if (i > 0)
                text = text + ", ";
            text = text + ids[i];
        }
        return text;
    }

    static string YesNo(bool yes)
    {
        if (yes)
            return "yes";
        return "no";
    }

    // The first and the last card of a list.
    static string Ends(array<string> cards)
    {
        if (cards.Count() == 0)
            return "-";
        return cards[0] + ".." + cards[cards.Count() - 1];
    }

    // How many cards a list holds, when it holds exactly the cards of the
    // other; "lost" when it does not.
    static string Kept(array<string> cards, array<string> whole)
    {
        if (cards.Count() != whole.Count())
            return "lost";
        array<string> rest = new array<string>();
        rest.Copy(whole);
        for (int i = 0; i < cards.Count(); i++)
        {
            int at = rest.Find(cards[i]);
            if (at < 0)
                return "lost";
            rest.RemoveOrdered(at);
        }
        return cards.Count().ToString();
    }
}
