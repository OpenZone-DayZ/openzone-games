// Durak's own words: its id, its option, its numbers and its refusals.
class OZG_DurakConst
{
    static const string GAME_ID = "durak";

    // The one option and its two values (design §8, rule 7).
    static const string OPT_VARIANT = "variant";
    static const string VARIANT_THROW    = "podkidnoy";
    static const string VARIANT_TRANSFER = "perevodnoy";

    // The format of a saved game.
    static const int VERSION = 3;
    // Cards dealt to a seat, and drawn up to after a bout.
    static const int HAND = 6;
    // Attack cards in one bout, and in the first bout of a game.
    static const int LIMIT       = 6;
    static const int LIMIT_FIRST = 5;
    // The lowest rank of the 36 cards durak plays with.
    static const string LOWEST = "6";

    // A seat: in play, out with no cards, or dropped with its cards discarded.
    static const int IN_PLAY      = 0;
    static const int GONE_OUT     = 1;
    static const int GONE_DROPPED = 2;

    // What the game that ended tells the next deal: the durak's seat.
    static const string HINT_DURAK = "durak";

    // Refusals: keys of the string table.
    static const string NO_TURN          = "STR_OZG_DURAK_NO_TURN";
    static const string NO_DEFENDER      = "STR_OZG_DURAK_NO_DEFENDER";
    static const string NO_RANK          = "STR_OZG_DURAK_NO_RANK";
    static const string NO_LIMIT         = "STR_OZG_DURAK_NO_LIMIT";
    static const string NO_NOT_DEFENDER  = "STR_OZG_DURAK_NO_NOT_DEFENDER";
    static const string NO_TARGET        = "STR_OZG_DURAK_NO_TARGET";
    static const string NO_BEAT          = "STR_OZG_DURAK_NO_BEAT";
    static const string NO_TAKING        = "STR_OZG_DURAK_NO_TAKING";
    static const string NO_TAKE          = "STR_OZG_DURAK_NO_TAKE";
    static const string NO_EMPTY         = "STR_OZG_DURAK_NO_EMPTY";
    static const string NO_PASS          = "STR_OZG_DURAK_NO_PASS";
    static const string NO_TRANSFER_OFF  = "STR_OZG_DURAK_NO_TRANSFER_OFF";
    static const string NO_TRANSFER_LATE = "STR_OZG_DURAK_NO_TRANSFER_LATE";
    static const string NO_TRANSFER_RANK = "STR_OZG_DURAK_NO_TRANSFER_RANK";
    static const string NO_TRANSFER_FEW  = "STR_OZG_DURAK_NO_TRANSFER_FEW";
}

// One seat's part of a game.
class OZG_DurakSeat
{
    // The session's number of the seat.
    int SeatNo;
    ref array<string> Held;
    // OZG_DurakConst.IN_PLAY, GONE_OUT or GONE_DROPPED.
    int Gone;
    // 1: said "pass" since the last card was laid.
    int Passed;

    void OZG_DurakSeat()
    {
        Held = new array<string>();
    }
}

// The whole state of a game, hidden cards included. It is what SaveState
// writes, so a field here is a field of the saved game: a change of meaning is
// a change of VERSION.
// 3: a dealt card keeps its mark (Sign, Readers, Noticers).
class OZG_DurakState
{
    int Version;
    // 1: perevodnoy.
    int Transfer;
    // The seats dealt in, in their order around the mat. Every index below is
    // an index into this list, never a seat's number.
    ref array<ref OZG_DurakSeat> SeatList;
    // [0] is drawn next. The last one is the trump card and is drawn last.
    ref array<string> StockCards;
    // Stays known after it is drawn.
    string TrumpCard;
    ref array<string> DiscardCards;
    // On the table, in the order laid.
    ref array<string> AttackCards;
    // Beside each attack card: the card that beat it, "" while it is unbeaten.
    ref array<string> DefenceCards;
    // Who laid the first card of this bout; who answers it now.
    int Opener;
    int Defender;
    // Attack cards this bout may hold.
    int AttackLimit;
    // 1: the defender said "take".
    int Taking;
    // 1 until a bout of this game has closed.
    int FirstBout;
    // 1: the game is over.
    int Ended;
    // The NUMBER of the durak's seat; -1 for none.
    int DurakSeat;
    // 1: the last cards left both hands at once.
    int Tie;
    // Every card this deal began with, in the order it was handed over: its
    // name and what it is like. Sound counts against it.
    ref array<ref OZG_DealCard> Dealt;

    void OZG_DurakState()
    {
        SeatList = new array<ref OZG_DurakSeat>();
        StockCards = new array<string>();
        DiscardCards = new array<string>();
        AttackCards = new array<string>();
        DefenceCards = new array<string>();
        Dealt = new array<ref OZG_DealCard>();
        DurakSeat = -1;
    }
}

// ---- what goes to clients -----------------------------------------------------

// A seat as everybody may see it.
class OZG_DurakSeatPublic
{
    int SeatNo;
    int CardCount;
    int Gone;
    int Passed;
    // The look of each card's back, the lowest first: what anybody at the
    // table sees of a hand.
    ref array<int> Backs;

    void OZG_DurakSeatPublic()
    {
        Backs = new array<int>();
    }
}

// The public half of a view.
class OZG_DurakPublic
{
    string GameId;
    string VariantId;
    string TrumpCard;
    int StockCount;
    int DiscardCount;
    ref array<string> AttackCards;
    ref array<string> DefenceCards;
    // Seat numbers.
    int OpenerSeat;
    int DefenderSeat;
    int AttackLimit;
    int Taking;
    ref array<ref OZG_DurakSeatPublic> SeatList;
    // The seats the game stands still for.
    ref array<int> Awaited;
    int Ended;
    int DurakSeat;
    int Tie;
    int TrumpLook;
    // The look of the card drawn next; -1 when the stock is empty.
    int StockTop;
    // Beside AttackCards and DefenceCards; -1 beside an unbeaten card.
    ref array<int> AttackLooks;
    ref array<int> DefenceLooks;

    void OZG_DurakPublic()
    {
        AttackCards = new array<string>();
        DefenceCards = new array<string>();
        SeatList = new array<ref OZG_DurakSeatPublic>();
        Awaited = new array<int>();
        AttackLooks = new array<int>();
        DefenceLooks = new array<int>();
        StockTop = -1;
        OpenerSeat = -1;
        DefenderSeat = -1;
        DurakSeat = -1;
    }
}

// A seat's own half: its cards and the moves it may make now.
class OZG_DurakPrivate
{
    ref array<string> Held;
    ref array<string> CanPlay;
    // Beside Held.
    ref array<int> HeldLooks;

    void OZG_DurakPrivate()
    {
        Held = new array<string>();
        CanPlay = new array<string>();
        HeldLooks = new array<int>();
    }
}
