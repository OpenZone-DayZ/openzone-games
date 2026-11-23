// The games this server can play. A mod registers a sample of its game when the
// mission starts; a session asks for a fresh one by id.
class OZG_Games
{
    private static ref map<string, ref OZG_Game> s_Samples = new map<string, ref OZG_Game>();
    // The ids in the order they were registered: a map keeps none.
    private static ref array<string> s_Order = new array<string>();

    static void Register(OZG_Game sample)
    {
        if (!sample)
            return;
        string id = sample.GameId();
        if (id == "")
        {
            OZG_Log.Warn("a game with no id was not registered");
            return;
        }
        if (s_Samples.Contains(id))
        {
            OZG_Log.Warn("the game '" + id + "' is registered twice; the first one stays");
            return;
        }
        s_Samples.Set(id, sample);
        s_Order.Insert(id);
    }

    // A new game of this kind, or null when nobody registered one.
    static OZG_Game Create(string id)
    {
        OZG_Game sample = Sample(id);
        if (!sample)
            return null;
        return sample.Fresh();
    }

    // The registered sample itself: for what the lobby asks.
    static OZG_Game Sample(string id)
    {
        OZG_Game sample;
        if (!s_Samples.Find(id, sample))
            return null;
        return sample;
    }

    static void Ids(array<string> into)
    {
        for (int i = 0; i < s_Order.Count(); i++)
            into.Insert(s_Order[i]);
    }

    static int Count()
    {
        return s_Order.Count();
    }

    static void Reset()
    {
        s_Samples.Clear();
        s_Order.Clear();
    }
}
