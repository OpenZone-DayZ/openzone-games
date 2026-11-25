// What this client was last told about each place it looks at: the head of the
// view and the game's two halves, as the server sent them.
class OZG_TableView
{
    ref OZG_WView HeadPart;
    string Shared;    // the game's public half, JSON
    string Own;       // the game's half for this client's seat, JSON; "" for a watcher
}

class OZG_Table
{
    protected static ref map<string, ref OZG_TableView> s_Views = new map<string, ref OZG_TableView>();

    // One body of the service: three texts (OZG_Text.Cut3). Null when it does
    // not read.
    static OZG_TableView Take(string body)
    {
        string head;
        string shared;
        string own;
        OZG_Text.Cut3(body, head, shared, own);

        OZG_WView v;
        string err;
        if (!JsonFileLoader<OZG_WView>.LoadData(head, v, err) || !v)
        {
            OZG_Log.Warn("a view did not read: " + err);
            return null;
        }
        if (!v.Seats)
            v.Seats = new array<ref OZG_WSeat>();
        if (!v.OptIds)
            v.OptIds = new array<string>();
        if (!v.OptVals)
            v.OptVals = new array<string>();
        if (!v.Dealt)
            v.Dealt = new array<int>();
        if (!v.Log)
            v.Log = new array<string>();
        if (!v.Reads)
            v.Reads = new array<ref OZG_WRead>();
        if (!v.Notices)
            v.Notices = new array<ref OZG_WNotice>();

        OZG_TableView view = new OZG_TableView();
        view.HeadPart = v;
        view.Shared = shared;
        view.Own = own;
        s_Views.Set(v.PlaceId, view);
        return view;
    }

    static OZG_TableView Of(string placeId)
    {
        return s_Views.Get(placeId);
    }

    static void Forget(string placeId)
    {
        s_Views.Remove(placeId);
    }

    static void Reset()
    {
        s_Views.Clear();
    }
}
