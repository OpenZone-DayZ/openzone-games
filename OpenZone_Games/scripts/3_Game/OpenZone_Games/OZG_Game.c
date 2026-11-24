// One thing the host chooses before a deal: a variant of the game. Ids go on
// the wire and into files; keys are of the string table.
class OZG_Option
{
    string OptionId;
    string LabelKey;
    ref array<string> ValueIds;
    // Beside ValueIds, one key for each.
    ref array<string> ValueKeys;
    string DefaultId;

    void OZG_Option()
    {
        ValueIds = new array<string>();
        ValueKeys = new array<string>();
    }
}

// A game is a module (design §7). It lives in 3_Game and sees nothing of the
// world: seats come to it as numbers, chance is handed in, and what it tells is
// text. One object is one game being played; the registry (OZG_Games) keeps a
// sample of each kind and asks it for fresh ones.
//
// The names differ a little from the design's table on purpose: a method is not
// named like a class another mod may declare (Result, Options, Cards), because
// the compiler would read a call of it as that class's constructor.
class OZG_Game
{
    // ---- what the lobby asks -------------------------------------------------

    string GameId()
    {
        return "";
    }

    // The game's name, a key of the string table.
    string LabelKey()
    {
        return "";
    }

    int MinPlayers()
    {
        return 2;
    }

    int MaxPlayers()
    {
        return 2;
    }

    // Which of the deck's 52 cards the game plays with.
    void CardsUsed(array<string> into)
    {
    }

    // The variants the host chooses from.
    void OptionList(array<ref OZG_Option> into)
    {
    }

    // A new, empty game of the same kind.
    OZG_Game Fresh()
    {
        return null;
    }

    // ---- the cards of a deal -------------------------------------------------

    // What the next deal is dealt from, when there is a deck to read: a
    // record a card, every name of the deal its own. A game nobody hands
    // cards to deals its own whole set, every back of look 0: what the
    // stand's bench and a check rely on.
    protected ref array<ref OZG_DealCard> m_Given;

    void TakeCards(array<ref OZG_DealCard> cards)
    {
        m_Given = new array<ref OZG_DealCard>();
        for (int i = 0; i < cards.Count(); i++)
        {
            OZG_DealCard c = cards[i];
            if (c && OZG_Cards.IsCard(c.Name))
                m_Given.Insert(OZG_DealCard.Make(c.Name, c.Look));
        }
    }

    // ---- a game being played -------------------------------------------------

    // A deal. `seats` are the seats dealt in, in their order around the mat;
    // `options` maps an option's id to the id of the chosen value; `chance` is
    // handed in, so a check can fix the order of the deck; `hint` is what the
    // session carries from the last game (HintInt).
    bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // One move: checked and applied, or refused with a key of the string table.
    bool Act(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // How many cards the seat holds now: what the fan in a player's hands
    // shows. Nothing of which cards they are.
    int HeldCount(int seat)
    {
        return 0;
    }

    // Does the game stand still until this seat moves?
    bool Waits(int seat)
    {
        return false;
    }

    // Is the move this seat is waited for one it need not make? The session
    // passes it over for a seat whose player is not present (design §6.3).
    bool Skippable(int seat)
    {
        return false;
    }

    // Passes that move over.
    void SkipSeat(int seat)
    {
    }

    // The seat is out and its cards are discarded.
    void DropSeat(int seat)
    {
    }

    // The moves this seat may make now, as they go on the wire.
    void Moves(int seat, array<string> into)
    {
    }

    // The two halves of a view, as JSON: what everybody may know, and what
    // this seat alone may.
    string PublicView()
    {
        return "{}";
    }

    string SeatView(int seat)
    {
        return "{}";
    }

    bool IsOver()
    {
        return true;
    }

    // What the next deal of this kind should know of the game that ended, as
    // `key=value`; "" for nothing. The session joins it to the hint.
    string Outcome()
    {
        return "";
    }

    // The whole state, hidden cards included (design §9).
    string SaveState()
    {
        return "";
    }

    // False when the text does not load or does not add up; the game is then
    // left as it was.
    bool LoadState(string data)
    {
        return false;
    }

    // Does the state add up: every card the game plays with exactly once?
    bool Sound(out string why)
    {
        why = "";
        return true;
    }

    // One line for the log and the stand: what everybody may know.
    string Brief()
    {
        return "";
    }

    // Every hidden thing in full, in one line: for the stand and for the log
    // at debug level. Never sent to a client.
    string Everything()
    {
        return "";
    }

    // ---- what every game shares ----------------------------------------------

    // The value chosen for an option, or the fallback when none was.
    static string OptionOf(map<string, string> options, string id, string fallback)
    {
        if (!options)
            return fallback;
        string chosen;
        if (options.Find(id, chosen))
            return chosen;
        return fallback;
    }

    // A whole number out of a hint: pairs `key=value` joined with `;`.
    static int HintInt(string hint, string key, int fallback)
    {
        array<string> pairs = new array<string>();
        hint.Split(";", pairs);
        for (int i = 0; i < pairs.Count(); i++)
        {
            string pair = pairs[i];
            int eq = pair.IndexOf("=");
            if (eq <= 0)
                continue;
            if (pair.Substring(0, eq) != key)
                continue;
            string number = pair.Substring(eq + 1, pair.Length() - eq - 1);
            return number.ToInt();
        }
        return fallback;
    }

    // Passes over every move that may be passed over, for the seats whose
    // players are not present. Returns how many it passed.
    int SkipAbsent(array<int> absent)
    {
        int skipped = 0;
        bool again = true;
        int rounds = 0;
        while (again && rounds < 64)
        {
            again = false;
            rounds++;
            for (int i = 0; i < absent.Count(); i++)
            {
                if (IsOver())
                    return skipped;
                if (!Skippable(absent[i]))
                    continue;
                SkipSeat(absent[i]);
                skipped++;
                again = true;
            }
        }
        return skipped;
    }
}
