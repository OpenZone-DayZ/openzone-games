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

    // What the game is played with (boards and dice design §2): a deck, a
    // board or a can of dice (OZG_Const.SET_*).
    string SetKind()
    {
        return OZG_Const.SET_CARDS;
    }

    // Which of the deck's 52 cards the game plays with; for a board game, the
    // faces of the pieces it plays with.
    void CardsUsed(array<string> into)
    {
    }

    // Before Begin, for a game of a board: the faces of the pieces the board
    // holds that the game plays with, one entry a piece. A game nobody hands
    // pieces to plays with a whole set (the stand's checks).
    void TakePieces(array<string> faces)
    {
    }

    // The variants the host chooses from.
    void OptionList(array<ref OZG_Option> into)
    {
    }

    // How many dice a seat throws (boards and dice design §8): 0 for a game of
    // no dice. The session picks each seat's dice by the option of whose dice.
    int DiceNeeded()
    {
        return 0;
    }

    // Whose dice the seats throw, the options read (OZG_DiceConst.WHOSE_*).
    string DiceMode(map<string, string> options)
    {
        return OptionOf(options, OZG_DiceConst.OPT_WHOSE, OZG_DiceConst.WHOSE_HOST);
    }

    // Before Begin and whenever a seat's dice change: the loads of the dice
    // `seat` throws, a loaded face (0: none) and its weight in percent each.
    void TakeDice(int seat, array<int> faces, array<int> weights)
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
                m_Given.Insert(c.Copy());
        }
    }

    // The dealer's choice (card sharp's design §6.5): before Begin, the
    // cards named (names of the cards handed with TakeCards) are dealt to
    // the seat `seat` first, and the rest of the deal is by chance.
    protected int m_FirstSeat = -1;
    protected ref array<string> m_FirstCards;

    void DealFirst(int seat, array<string> names)
    {
        m_FirstSeat = seat;
        m_FirstCards = new array<string>();
        if (names)
            m_FirstCards.Copy(names);
    }

    // ---- sleights (card sharp's design §6.7) ---------------------------------
    //
    // A sleight moves a card the rules do not move: into a hand, out of it,
    // onto the discards. The session moves the items and tells the game of
    // every card it brings in or takes out; the game keeps its own record of
    // the deal true (every card of it somewhere exactly once) and settles
    // whatever the change leaves to settle. A game with no hands refuses all.

    // May a sleight bring a card of this face into a hand of this game?
    bool SleightTakes(string face)
    {
        return false;
    }

    // A name for one more card of `face` that this deal has not used, its
    // cards gone included.
    string SleightName(string face)
    {
        return "";
    }

    // Does the hand of `seat` hold the card `card`?
    bool SleightHolds(int seat, string card)
    {
        return false;
    }

    // The card `card` (a record the session made from the item) joins the
    // hand of `seat`.
    bool SleightIn(int seat, OZG_DealCard card, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // The card `card` leaves the hand of `seat`, and the deal.
    bool SleightOut(int seat, string card, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // The card `card` leaves the hand of `seat` and the deal, and `instead`
    // takes its place: the count stays.
    bool SleightSwap(int seat, string card, OZG_DealCard instead, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // The card `card` goes from the hand of `seat` onto the discards; it
    // stays a card of the deal.
    bool SleightDiscard(int seat, string card, out string refusal)
    {
        refusal = OZG_Const.NO_GAME;
        return false;
    }

    // Every card of the deal as it stands now, copies: what finds a card's
    // item again (OZG_DeckRead.MatchDeal).
    void DealtRecords(array<ref OZG_DealCard> into)
    {
        into.Clear();
    }

    // ---- what is heard -------------------------------------------------------

    // The sounds the last change made, in the order it made them: ids of
    // OZG_SoundIds (design §17). The session takes them after every change it
    // accepts and puts them on the place, where every client near it plays
    // them. A game nobody asks (the stand's bench) keeps the last eight.
    protected ref array<int> m_Heard;

    protected void HearSound(int sound)
    {
        if (!m_Heard)
            m_Heard = new array<int>();
        // The same sound twice in a row is heard once: two cards laid in one
        // change sound as one.
        int n = m_Heard.Count();
        if (n > 0 && m_Heard[n - 1] == sound)
            return;
        if (n >= 8)
            m_Heard.RemoveOrdered(0);
        m_Heard.Insert(sound);
    }

    // What was heard since the last ask, added to `into`; the list emptied.
    void TakeHeard(array<int> into)
    {
        if (!m_Heard)
            return;
        for (int i = 0; i < m_Heard.Count(); i++)
            into.Insert(m_Heard[i]);
        m_Heard.Clear();
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

    // The cards the seat `seat`, held by the player `uid`, reads where they
    // lie back up for that player: in the other seats' hands and on top of
    // the stock (card sharp's design §4.4). A game without hidden cards says
    // nothing.
    void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
    }

    // From now on the player `uid` notices the card `card` of this deal: the
    // session rolled it (card sharp's design §5). False when the deal has no
    // such marked card.
    bool AddNoticer(string card, string uid)
    {
        return false;
    }

    // The cards the seat `seat`, held by the player `uid`, notices, where
    // they lie for that player (card sharp's design §4.5): on a back where a
    // read would lie, a card he notices and does not read (a reader sees
    // more); on a face -- his own hand, the table, the trump -- every card he
    // notices. A game without hidden cards says nothing.
    void NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)
    {
        into.Clear();
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

    // ---- stakes and the rest the later games ask (stakes-and-games design §2) --

    // Does this game play for stacks the host hands out (design §3)?
    bool UsesStakes()
    {
        return false;
    }

    // Before Begin: the chips of each seat dealt in, beside `seats`.
    void TakeStacks(array<int> seats, array<int> amounts)
    {
    }

    // The chips of a seat now; -1 when the game keeps none.
    int StackOf(int seat)
    {
        return -1;
    }

    // May sleights and the dealer's three touch this game (card sharp's
    // design §6)? Durak's answer is yes; the later games' no.
    bool HasSleights()
    {
        return false;
    }

    // How the log words a move of this game: a key of the string table and
    // its arguments (`c:<card>` a card, `s:<seat>` a seat's name, anything
    // else as it is). Empty for a move the window words itself (durak's).
    void MoveWords(string move, array<string> into)
    {
        into.Clear();
    }

    // A legal move for the stand's bots: the first the game offers, unless a
    // game knows better. "" when the seat has none.
    string BotMove(int seat, OZG_Random chance)
    {
        array<string> can = new array<string>();
        Moves(seat, can);
        if (can.Count() == 0)
            return "";
        OZG_Random dice = chance;
        if (!dice)
            dice = new OZG_Random();
        return can[dice.Below(can.Count())];
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
