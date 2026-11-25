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

    void OZG_WAsk()
    {
        SeatNo = -1;
    }
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

    void OZG_WView()
    {
        OptIds = new array<string>();
        OptVals = new array<string>();
        Seats = new array<ref OZG_WSeat>();
        Dealt = new array<int>();
        Log = new array<string>();
        Reads = new array<ref OZG_WRead>();
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
