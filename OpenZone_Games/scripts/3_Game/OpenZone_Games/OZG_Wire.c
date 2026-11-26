// What travels between the server and a client, through the core's service
// pair (service "games"). Every body is JSON.
//
// A VIEW IS THREE TEXTS IN ONE BODY, joined by a line feed: the session's head
// (OZG_WView), the game's public half and the game's own half for that client.
// The halves are the game's JSON as it made it; nesting them inside the head
// as strings would escape every quote. Compact JSON holds no raw line feed, so
// the cut is safe (OZG_Text.Cut3).

// What a client asks for: one shape for every op, the fields an op does not
// use stay empty.
class OZG_WAsk
{
    string PlaceId;
    int    SeatNo;
    string Move;
    string How;
    string OptId;
    string OptValue;
    int    PoseNo;
    // A sleight (card sharp's design §9): the card of the player's pockets it
    // takes, by its item's network id ("<low>.<high>", OZG_Keys); the server
    // checks the item is his.
    string CardKey;
    // The dealer's three, in the deal itself: "in=<key>,..;out=<key>|*,..;
    // pick=<key>,.." -- cards of his pockets into the deck, cards out of it
    // (a key he reads, or * for any), cards he reads dealt to him first.
    string Stack;

    void OZG_WAsk()
    {
        SeatNo = -1;
    }
}

// The parts of the table a shake moves (card sharp's design §6.1): a seat's
// hand, the discards, the stock (the design's "deck"). The server names them
// in a `shake`, the window draws its cards under them.
class OZG_Regions
{
    static const string STOCK = "stock";
    static const string DISCARDS = "discards";

    static string OfSeat(int seat)
    {
        return "seat" + seat.ToString();
    }
}

// An item's network id as a text both sides read the same: the key a client
// names a card by in a sleight, and the server finds it by.
// A sleight on the board seen (boards and dice design §9.1): told at once to
// whoever has the window open on the place.
class OZG_WBoardSaw
{
    string PlaceId;
    int    SeatNo;
    string Kind;
    string A;
    string B;
}

class OZG_Keys
{
    static string Of(Object thing)
    {
        if (!thing)
            return "";
        int low;
        int high;
        thing.GetNetworkID(low, high);
        return low.ToString() + "." + high.ToString();
    }

    static Object Find(string key)
    {
        int dot = key.IndexOf(".");
        if (dot <= 0)
            return null;
        string low = key.Substring(0, dot);
        string high = key.Substring(dot + 1, key.Length() - dot - 1);
        if (low == "" || high == "")
            return null;
        return GetGame().GetObjectByNetworkId(low.ToInt(), high.ToInt());
    }
}

// A card of the deck the dealer reads, told to him before a deal (card
// sharp's design §6.5): its key, to be named in the stack, its face, the look
// of its back and its sign.
class OZG_WDeckCard
{
    string Key;
    string Face;
    int    Look;
    int    Sign;

    static OZG_WDeckCard Make(string key, string face, int look, int sign)
    {
        OZG_WDeckCard c = new OZG_WDeckCard();
        c.Key = key;
        c.Face = face;
        c.Look = look;
        c.Sign = sign;
        return c;
    }
}

// What the one who did a sleight is told (card sharp's design §6.1, §9): the
// kind, the card that came into his hand and the cards that went out of it,
// by their names in the game, and the sound to play for him. Never sent to
// anybody else.
class OZG_WSleight
{
    string PlaceId;
    int    Rev;
    string Kind;
    string CameIn;
    string WentOut;
    string SoundName;
}

// One seat as a client sees it.
class OZG_WSeat
{
    int    No;
    int    Kind;
    int    St;
    string Name;
    float  X;
    float  Y;
    float  Z;
    float  Yaw;
    int    PoseNo;
    // A game played for stakes (stakes-and-games design §3): the chips of
    // the player on this seat now, and what he brought in; -1 otherwise.
    int    Chips;
    int    BroughtIn;

    void OZG_WSeat()
    {
        Chips = -1;
        BroughtIn = -1;
    }
}

// A card this client's player reads where it lies back up for that player
// (design §4.4): in the hand of the seat `Seat`, or on top of the stock
// (`Seat` -1). The look says which back of that hand it is drawn on; the face
// is what the player knows; the sign is the fault they see in the back.
class OZG_WRead
{
    int    Seat;
    int    Look;
    string Face;
    int    Sign;

    static OZG_WRead Make(int seat, int look, string face, int sign)
    {
        OZG_WRead r = new OZG_WRead();
        r.Seat = seat;
        r.Look = look;
        r.Face = face;
        r.Sign = sign;
        return r;
    }
}

// A card this client's player notices (card sharp's design §4.5): the sign
// he sees on it, and where. On a back -- `CardName` empty -- in the hand of
// the seat `Seat`, or on top of the stock (`Seat` -1), on a back of the look
// `Look`; nothing says which card it is. On a face -- `CardName` the card's
// name, which the player sees anyway, `Seat` -2 -- that card wherever it
// lies face up for him: in his own hand, on the table, the trump under the
// stock; `Look` gives the style of its face.
class OZG_WNotice
{
    int    Seat;
    int    Look;
    string CardName;
    int    Sign;

    static OZG_WNotice MakeBack(int seat, int look, int sign)
    {
        OZG_WNotice n = new OZG_WNotice();
        n.Seat = seat;
        n.Look = look;
        n.CardName = "";
        n.Sign = sign;
        return n;
    }

    static OZG_WNotice MakeFace(string card, int look, int sign)
    {
        OZG_WNotice n = new OZG_WNotice();
        n.Seat = -2;
        n.Look = look;
        n.CardName = card;
        n.Sign = sign;
        return n;
    }
}

// The head of a view.
class OZG_WView
{
    string PlaceId;
    int    Rev;
    int    St;
    string Style;
    string GameId;
    ref array<string> OptIds;
    ref array<string> OptVals;
    int    Mine;        // this client's seat here, -1 for a watcher
    int    HostSeat;    // -1 when the host holds no seat
    string HostName;
    int    IsHost;      // 1 when this client is the host
    int    HostFree;    // 1 when the role may be taken
    int    Mode;
    int    Kind;
    ref array<ref OZG_WSeat> Seats;
    ref array<int> Dealt;
    ref array<string> Log;
    string Note;        // a key said once: why the lobby is empty
    string Hint;        // what the game that ended said (OZG_Game.Outcome)
    int    MarkSeconds;
    float  Reach;
    // What this client's player reads (design §4.4); empty for a watcher and
    // for a seat that reads nothing. Made apart for every player.
    ref array<ref OZG_WRead> Reads;
    // What this client's player notices (design §4.5); empty as the reads.
    ref array<ref OZG_WNotice> Notices;
    // Sleights (card sharp's design §6): how many this player did in this
    // deal, and how many he may; SleightsMost 0 when he may do none now (a
    // watcher, a seat not dealt in, Cheating 0). For the dealer between deals:
    // how many cards he may choose for his hand, and the cards of the deck he
    // reads (§6.5).
    int    SleightsUsed;
    int    SleightsMost;
    int    PicksMost;
    ref array<ref OZG_WDeckCard> DeckReads;
    // Stakes (stakes-and-games design §3): 1 when the game plays for them;
    // this player's own chips and what he brought in; 1 when he asked for
    // chips and waits; for the host alone, the requests waiting: who, his
    // name, the seat he asked for (-1: chips for the seat he holds).
    int    Stakes;
    int    MyChips;
    int    MyIn;
    int    AskedChips;
    ref array<string> ReqUids;
    ref array<string> ReqNames;
    ref array<int> ReqSeats;
    // The games this server offers, for the host to pick from.
    ref array<string> GameIds;

    void OZG_WView()
    {
        OptIds = new array<string>();
        OptVals = new array<string>();
        Seats = new array<ref OZG_WSeat>();
        Dealt = new array<int>();
        Log = new array<string>();
        Reads = new array<ref OZG_WRead>();
        Notices = new array<ref OZG_WNotice>();
        DeckReads = new array<ref OZG_WDeckCard>();
        ReqUids = new array<string>();
        ReqNames = new array<string>();
        ReqSeats = new array<int>();
        GameIds = new array<string>();
        Mine = -1;
        HostSeat = -1;
    }
}

// Why a place closed for a client.
class OZG_WClosed
{
    string PlaceId;
    string Why;
}

// Where this client holds a seat, if anywhere.
class OZG_WMine
{
    string PlaceId;
    int    SeatNo;

    void OZG_WMine()
    {
        SeatNo = -1;
    }
}

// What a client is told when its player saw a sleight (card sharp's design
// §6.1, §9): where the cards changed, as a region of the window
// (OZG_GameMenu.RegionOfSeat(n), "discards", "stock"), and the sound to play
// for him alone (a name of OZG_SoundIds, "" for none).
class OZG_WShake
{
    string PlaceId;
    string RegionName;
    string SoundName;
}

class OZG_Text
{
    static const string CUT = "\n";

    static string Join3(string a, string b, string c)
    {
        string s = a + CUT;
        s = s + b + CUT;
        s = s + c;
        return s;
    }

    // Three texts out of one body. string.Split would drop an empty half, and
    // a watcher's own half is empty.
    static bool Cut3(string body, out string a, out string b, out string c)
    {
        a = "";
        b = "";
        c = "";
        int first = body.IndexOf(CUT);
        if (first < 0)
        {
            a = body;
            return false;
        }
        a = body.Substring(0, first);
        string rest = body.Substring(first + 1, body.Length() - first - 1);
        int second = rest.IndexOf(CUT);
        if (second < 0)
        {
            b = rest;
            return false;
        }
        b = rest.Substring(0, second);
        c = rest.Substring(second + 1, rest.Length() - second - 1);
        return true;
    }

    // A text cut into strings of at most `bytes` bytes, never inside a
    // character: the session file's wall is the loader's 1023 bytes a string.
    static void Parts(string text, int bytes, array<string> into)
    {
        into.Clear();
        int at = 0;
        int total = text.Length();
        while (at < total)
        {
            int take = bytes;
            if (at + take > total)
                take = total - at;
            // A UTF-8 continuation byte is 10xxxxxx: step back to a lead byte.
            while (take > 1 && at + take < total)
            {
                string edge = text.Substring(at + take, 1);
                int code = edge.ToAscii();
                if (code < 0)
                    code = code + 256;
                bool inside = code >= 128 && code < 192;
                if (!inside)
                    break;
                take--;
            }
            into.Insert(text.Substring(at, take));
            at = at + take;
        }
    }

    // A text split on `sep`, empty parts kept. string.Split hands back no part
    // longer than 8191 bytes, as Substring does (measured 2026-11-26: two
    // parts of 9000 bytes came back as 8191 each; 3000 parts of 10 bytes out
    // of 33 KB came back whole), and drops empty ones. Here each piece is
    // taken with Substring from an offset: a long text of short pieces is
    // safe; a piece of its own over 8191 bytes is not, so a long list inside
    // a record is read in place (SplitSpan, FieldSpan).
    static void SplitLong(string text, string sep, array<string> into)
    {
        SplitSpan(text, 0, text.Length(), sep, into);
    }

    // The pieces of `text` from `from` up to `to` (not included), split on
    // `sep`.
    static void SplitSpan(string text, int from, int to, string sep, array<string> into)
    {
        into.Clear();
        int step = sep.Length();
        int at = from;
        while (at <= to)
        {
            int next = text.IndexOfFrom(at, sep);
            if (next < 0 || next > to)
            {
                into.Insert(text.Substring(at, to - at));
                return;
            }
            into.Insert(text.Substring(at, next - at));
            at = next + step;
        }
    }

    // Where the field `n` (from 0) of a record of fields joined by `sep`
    // begins and ends, without taking it out.
    static bool FieldSpan(string text, string sep, int n, out int from, out int to)
    {
        from = 0;
        to = 0;
        int at = 0;
        for (int i = 0; i < n; i++)
        {
            int next = text.IndexOfFrom(at, sep);
            if (next < 0)
                return false;
            at = next + sep.Length();
        }
        int end = text.IndexOfFrom(at, sep);
        if (end < 0)
            end = text.Length();
        from = at;
        to = end;
        return true;
    }

    // The pieces of the field `n` of a record (fields joined by `fieldSep`),
    // split on `sep`; nothing for a field written "-".
    static void FieldPieces(string text, string fieldSep, int n, string sep, array<string> into)
    {
        into.Clear();
        int from;
        int to;
        if (!FieldSpan(text, fieldSep, n, from, to))
            return;
        if (to - from == 1 && text.Get(from) == "-")
            return;
        if (to <= from)
            return;
        SplitSpan(text, from, to, sep, into);
    }

    static string JoinIds(array<string> ids)
    {
        string s = "";
        for (int i = 0; i < ids.Count(); i++)
        {
            if (i > 0)
                s = s + ",";
            s = s + ids[i];
        }
        return s;
    }

    static string Glue(array<string> parts)
    {
        string s = "";
        if (!parts)
            return s;
        for (int i = 0; i < parts.Count(); i++)
            s = s + parts[i];
        return s;
    }

    // A copy in capitals, Latin and Ukrainian: the stalkers' table writes its
    // title, names and buttons so, as the mock-up's CSS did. The engine's
    // ToUpper is left to the Latin letters (what it does with Cyrillic is not
    // measured); each Ukrainian letter is replaced whole -- two bytes for two,
    // and a lead byte never stands where a continuation byte does, so no
    // replacement can land inside another letter (OpenZone Research's
    // OZL_Text.Upper does the same). The small i becomes the Latin I, as the
    // series' strings write the capital (vanilla fonts lack the Cyrillic one).
    protected static ref array<string> s_Lower;
    protected static ref array<string> s_Upper;

    protected static void Pair(string lower, string upper)
    {
        s_Lower.Insert(lower);
        s_Upper.Insert(upper);
    }

    static string Upper(string s)
    {
        if (!s_Lower)
        {
            s_Lower = new array<string>();
            s_Upper = new array<string>();
            Pair("а", "А"); Pair("б", "Б"); Pair("в", "В"); Pair("г", "Г"); Pair("ґ", "Ґ");
            Pair("д", "Д"); Pair("е", "Е"); Pair("є", "Є"); Pair("ж", "Ж"); Pair("з", "З");
            Pair("и", "И"); Pair("і", "I"); Pair("ї", "Ї"); Pair("й", "Й"); Pair("к", "К");
            Pair("л", "Л"); Pair("м", "М"); Pair("н", "Н"); Pair("о", "О"); Pair("п", "П");
            Pair("р", "Р"); Pair("с", "С"); Pair("т", "Т"); Pair("у", "У"); Pair("ф", "Ф");
            Pair("х", "Х"); Pair("ц", "Ц"); Pair("ч", "Ч"); Pair("ш", "Ш"); Pair("щ", "Щ");
            Pair("ь", "Ь"); Pair("ю", "Ю"); Pair("я", "Я"); Pair("І", "I");
        }
        string t = s;
        t.ToUpper();
        for (int i = 0; i < s_Lower.Count(); i++)
            t.Replace(s_Lower[i], s_Upper[i]);
        return t;
    }
}
