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

    void OZG_WView()
    {
        OptIds = new array<string>();
        OptVals = new array<string>();
        Seats = new array<ref OZG_WSeat>();
        Dealt = new array<int>();
        Log = new array<string>();
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

// What counting the deck found: for the one who counted, and nobody else.
class OZG_WCount
{
    string PlaceId;
    int    Total;      // cards the deck holds
    int    Missing;    // faces of the game's set that are not there
    int    Copies;     // cards that are a second, third or further of their face
    ref array<string> Names;   // the missing faces, when the server says them

    void OZG_WCount()
    {
        Names = new array<string>();
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
}
