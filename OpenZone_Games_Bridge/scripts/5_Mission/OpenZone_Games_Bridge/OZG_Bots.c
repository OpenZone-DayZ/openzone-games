// The stand's players: characters the server makes and the script moves for,
// so that one person can see a whole game. And the chat commands that set a
// table for them.
//
// A stand tool, never published: no player of a real server gets a bot or a
// command from here.

class OZG_Bots
{
    static bool s_On;
    protected static float s_Wait;
    protected static int s_Made;
    protected static ref array<Object> s_Tables = new array<Object>();

    static const float STEP_SECONDS = 2.5;
    static const string PREFIX = "bot";

    static bool IsBot(string uid)
    {
        return uid.IndexOf(PREFIX) == 0;
    }

    // What a card costs to part with: its rank, and a trump much more.
    protected static int Cost(string card, string trumpSuit)
    {
        int cost = OZG_Cards.RankOf(card);
        if (OZG_Cards.SuitOf(card) == trumpSuit)
            cost = cost + 20;
        return cost;
    }

    // The card a move plays: "a:6s" and "t:6s" and "d:6s:1" all name it second.
    protected static string CardOf(string move)
    {
        return OZG_Cards.OfMove(move);
    }

    // One move for a seat the game waits for; "" when it has nothing to say.
    static string Choose(OZG_Game game, int seat)
    {
        array<string> moves = new array<string>();
        game.Moves(seat, moves);
        if (moves.Count() == 0)
            return "";

        string trumpSuit = "";
        bool opening = true;
        OZG_DurakPublic pub;
        string err;
        if (JsonFileLoader<OZG_DurakPublic>.LoadData(game.PublicView(), pub, err) && pub)
        {
            trumpSuit = OZG_Cards.SuitOf(pub.TrumpCard);
            if (pub.AttackCards && pub.AttackCards.Count() > 0)
                opening = false;
        }

        string beat = "";
        int beatCost = 1000;
        string lead = "";
        int leadCost = 1000;
        bool mayPass = false;
        bool mayTake = false;
        for (int i = 0; i < moves.Count(); i++)
        {
            string move = moves[i];
            if (move == "pass")
            {
                mayPass = true;
                continue;
            }
            if (move == "take")
            {
                mayTake = true;
                continue;
            }
            string kind = move.Substring(0, 1);
            int cost = Cost(CardOf(move), trumpSuit);
            if (kind == "d" && cost < beatCost)
            {
                beatCost = cost;
                beat = move;
            }
            if (kind == "a" && cost < leadCost)
            {
                leadCost = cost;
                lead = move;
            }
        }

        // Beat with the cheapest card; a dear trump is kept and the cards
        // taken, now and then.
        if (beat != "")
        {
            if (mayTake && beatCost > 30 && Math.RandomInt(0, 3) == 0)
                return "take";
            return beat;
        }
        if (lead != "")
        {
            // Opening the bout: the cheapest card. Throwing in: cheap cards
            // only, a trump never.
            if (opening || !mayPass)
                return lead;
            if (leadCost <= 10)
                return lead;
        }
        if (mayPass)
            return "pass";
        if (mayTake)
            return "take";
        return moves[0];
    }

    // Every few seconds: one move in each session where a bot is waited for.
    static void Tick(float timeslice)
    {
        if (!s_On)
            return;
        s_Wait = s_Wait + timeslice;
        if (s_Wait < STEP_SECONDS)
            return;
        s_Wait = 0;

        for (int i = 0; i < OZG_Sessions.Count(); i++)
        {
            OZG_Session s = OZG_Sessions.At(i);
            if (!s || !s.m_Game || s.m_R.St != OZG_Const.ST_PLAYING)
                continue;
            for (int k = 0; k < s.m_R.Seats.Count(); k++)
            {
                OZG_SeatRec seat = s.m_R.Seats[k];
                if (!IsBot(seat.Uid) || seat.St != OZG_Const.SEAT_TAKEN)
                    continue;
                if (!s.m_Game.Waits(k))
                    continue;
                string move = Choose(s.m_Game, k);
                if (move == "")
                    continue;
                PlayerBase body = OZG_Sessions.PlayerOf(seat.Uid);
                if (!body)
                    continue;
                OZG_WAsk ask = new OZG_WAsk();
                ask.PlaceId = s.Id();
                ask.Move = move;
                s.Ask(body, OZG_Const.OP_ACT, ask);
                // One move a session a step: a person follows the game.
                break;
            }
        }
    }

    // A bot on a free seat of a session. Returns its name, "" when there is
    // no seat for one.
    static string Seat(OZG_Session s)
    {
        if (!s)
            return "";
        array<string> names = {"Ostap", "Taras", "Bohdan", "Marko", "Petro", "Danylo", "Ivan"};
        // The free seat furthest from every taken one first: bots spread
        // round the mat as people do.
        array<int> order = new array<int>();
        array<float> rooms = new array<float>();
        int n = s.m_R.Seats.Count();
        int a;
        int b;
        for (a = 0; a < n; a++)
        {
            if (s.m_R.Seats[a].St != OZG_Const.SEAT_FREE)
                continue;
            float room = 1000;
            for (b = 0; b < n; b++)
            {
                if (s.m_R.Seats[b].St == OZG_Const.SEAT_FREE)
                    continue;
                float d = vector.Distance(s.m_R.Seats[a].Where(), s.m_R.Seats[b].Where());
                if (d < room)
                    room = d;
            }
            int at0 = 0;
            while (at0 < order.Count() && rooms[at0] >= room)
                at0++;
            order.InsertAt(a, at0);
            rooms.InsertAt(room, at0);
        }
        for (int k = 0; k < order.Count(); k++)
        {
            int i = order[k];
            OZG_SeatRec r = s.m_R.Seats[i];
            vector at = r.Where();
            PlayerBase body = PlayerBase.Cast(GetGame().CreateObjectEx("SurvivorM_Mirek", at, ECE_PLACE_ON_SURFACE));
            if (!body)
                return "";
            body.SetPosition(at);
            // A bot stands at a table for hours: hunger and cold are not for it.
            body.SetAllowDamage(false);
            s_Made++;
            string uid = PREFIX + s_Made.ToString();
            string name = names[(s_Made - 1) % names.Count()];
            OZG_Who.Fake(body, uid, name);
            string why;
            if (!s.Sit(body, i, why))
            {
                OZG_Who.Unfake(uid);
                GetGame().ObjectDelete(body);
                continue;
            }
            return name;
        }
        return "";
    }

    // Every bot of every session stands up and goes.
    static int Clear()
    {
        int gone = 0;
        for (int i = 0; i < OZG_Sessions.Count(); i++)
        {
            OZG_Session s = OZG_Sessions.At(i);
            if (!s)
                continue;
            for (int k = 0; k < s.m_R.Seats.Count(); k++)
            {
                string uid = s.m_R.Seats[k].Uid;
                if (!IsBot(uid))
                    continue;
                PlayerBase body = OZG_Sessions.PlayerOf(uid);
                OZG_WAsk ask = new OZG_WAsk();
                ask.PlaceId = s.Id();
                ask.How = OZG_Const.HOW_DISCARD;
                if (body)
                {
                    s.Ask(body, OZG_Const.OP_STAND, ask);
                    GetGame().ObjectDelete(body);
                }
                OZG_Who.Unfake(uid);
                gone++;
            }
        }
        return gone;
    }

    // The session of the place nearest to a player, within reach of a walk.
    static OZG_Session Nearest(PlayerBase player)
    {
        OZG_Session best;
        float bestFar = 30;
        for (int i = 0; i < OZG_Sessions.Count(); i++)
        {
            OZG_Session s = OZG_Sessions.At(i);
            if (!s || !s.m_Place)
                continue;
            float dist = vector.Distance(player.GetPosition(), s.m_Place.GetPosition());
            if (dist < bestFar)
            {
                bestFar = dist;
                best = s;
            }
        }
        return best;
    }

    // A kitchen table two metres in front of a player.
    static bool Table(PlayerBase player)
    {
        vector dir = player.GetDirection();
        dir[1] = 0;
        dir.Normalize();
        vector spot = player.GetPosition() + dir * 2.0;
        float ground = GetGame().SurfaceRoadY3D(spot[0], spot[1] + 1.5, spot[2], RoadSurfaceDetection.UNDER);
        vector pos = Vector(spot[0], ground + 0.5, spot[2]);
        Object table = GetGame().CreateObjectEx("StaticObj_Furniture_kitchen_table_a", pos, ECE_PLACE_ON_SURFACE);
        if (!table)
            return false;
        vector mm[2];
        table.ClippingInfo(mm);
        pos = table.GetPosition();
        float off = ground - (pos[1] + mm[0][1]);
        if (off > 0.02 || off < -0.02)
        {
            pos[1] = pos[1] + off;
            table.SetPosition(pos);
        }
        s_Tables.Insert(table);
        return true;
    }

    // Fed, watered and whole again. A stand's character is logged in for
    // hours and starves while its owner looks at a window.
    static void Feed(PlayerBase player)
    {
        if (!player)
            return;
        // The diag build's own reset: agents, modifiers, bleeding. A retail
        // build compiles it to nothing.
        player.ResetPlayer(true);
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
        player.SetHealth("", "", player.GetMaxHealth("", ""));
        player.SetHealth("", "Blood", player.GetMaxHealth("", "Blood"));
        player.SetHealth("", "Shock", player.GetMaxHealth("", "Shock"));
    }

    // Noon, whatever the hour.
    static void SetNoon()
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);
        GetGame().GetWorld().SetDate(year, month, day, 12, 0);
    }

    // Daylight kept, once asked for: a stand's clock runs twelve times faster
    // than a watch, and a tester who comes back an hour later finds the table
    // in the dark.
    static bool s_KeepDay;
    protected static float s_DayWait;

    static void DayTick(float timeslice)
    {
        if (!s_KeepDay)
            return;
        s_DayWait = s_DayWait + timeslice;
        if (s_DayWait < 30)
            return;
        s_DayWait = 0;
        int year;
        int month;
        int day;
        int hour;
        int minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);
        if (hour < 9 || hour >= 16)
            GetGame().GetWorld().SetDate(year, month, day, 12, 0);
    }

    static int ClearTables()
    {
        int n = 0;
        for (int i = 0; i < s_Tables.Count(); i++)
        {
            if (s_Tables[i])
            {
                GetGame().ObjectDelete(s_Tables[i]);
                n++;
            }
        }
        s_Tables.Clear();
        return n;
    }

    // ------------------------------------------------------------ the chat

    protected static void Reply(PlayerBase player, string text)
    {
        player.MessageStatus("[OZG] " + text);
    }

    protected static PlayerBase ByName(string name)
    {
        PlayerBase found;
        int matches = 0;
        array<Man> men = new array<Man>();
        GetGame().GetPlayers(men);
        for (int i = 0; i < men.Count(); i++)
        {
            PlayerBase p = PlayerBase.Cast(men[i]);
            if (p && p.GetIdentity() && p.GetIdentity().GetName() == name)
            {
                matches++;
                found = p;
            }
        }
        if (matches != 1)
            return null;
        return found;
    }

    // "!ozg ..." in the chat. The server is told only the sender's name: the
    // command runs when exactly one connected player bears it.
    static void OnChat(string senderName, string text)
    {
        if (text.IndexOf("!ozg") != 0)
            return;
        PlayerBase player = ByName(senderName);
        if (!player)
            return;

        TStringArray words = new TStringArray();
        text.Split(" ", words);
        string cmd = "help";
        if (words.Count() > 1)
            cmd = words[1];
        cmd.ToLower();
        string arg = "";
        if (words.Count() > 2)
            arg = words[2];

        if (cmd == "deck")
        {
            string deck = "OZG_Deck_Classic";
            if (arg == "zone")
                deck = "OZG_Deck_Zone";
            if (arg == "sharp")
                deck = "OZG_Deck_Classic_Sharp";
            if (arg == "zonesharp")
                deck = "OZG_Deck_Zone_Sharp";
            EntityAI made = player.GetHumanInventory().CreateInHands(deck);
            if (!made)
                made = player.GetInventory().CreateInInventory(deck);
            if (!made)
                made = EntityAI.Cast(GetGame().CreateObjectEx(deck, player.GetPosition(), ECE_PLACE_ON_SURFACE));
            Reply(player, "a deck: " + deck);
            return;
        }

        if (cmd == "table")
        {
            if (Table(player))
                Reply(player, "a table stands in front of you");
            else
                Reply(player, "no table could be made here");
            return;
        }

        if (cmd == "bots")
        {
            OZG_Session s = Nearest(player);
            if (!s)
            {
                Reply(player, "no laid-out game within 30 m");
                return;
            }
            int want = 2;
            if (arg != "")
                want = arg.ToInt();
            if (want < 1)
                want = 1;
            int sat = 0;
            string names = "";
            for (int i = 0; i < want; i++)
            {
                string name = Seat(s);
                if (name == "")
                    break;
                sat++;
                if (names != "")
                    names = names + ", ";
                names = names + name;
            }
            s_On = true;
            Reply(player, sat.ToString() + " bot(s) sat down: " + names);
            return;
        }

        if (cmd == "clear")
        {
            int gone = Clear();
            int tables = ClearTables();
            Reply(player, gone.ToString() + " bot(s) gone, " + tables.ToString() + " table(s) removed");
            return;
        }

        if (cmd == "pause")
        {
            s_On = !s_On;
            if (s_On)
                Reply(player, "the bots play");
            else
                Reply(player, "the bots wait");
            return;
        }

        if (cmd == "noon")
        {
            SetNoon();
            Reply(player, "it is noon");
            return;
        }

        if (cmd == "day")
        {
            s_KeepDay = !s_KeepDay;
            if (s_KeepDay)
            {
                SetNoon();
                Reply(player, "it is noon, and daylight is kept until the server stops");
            }
            else
            {
                Reply(player, "the night may come");
            }
            return;
        }

        if (cmd == "feed")
        {
            Feed(player);
            Reply(player, "fed, watered and whole");
            return;
        }

        Reply(player, "!ozg deck [zone|sharp|zonesharp] | table | bots [n] | pause | clear | noon | day | feed");
    }
}

modded class MissionServer
{
    override void OnEvent(EventType eventTypeId, Param params)
    {
        super.OnEvent(eventTypeId, params);
        if (eventTypeId == ChatMessageEventTypeID)
        {
            ChatMessageEventParams chat = ChatMessageEventParams.Cast(params);
            if (chat)
                OZG_Bots.OnChat(chat.param2, chat.param3);
        }
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);
        OZG_Bots.Tick(timeslice);
        OZG_Bots.DayTick(timeslice);
    }
}
