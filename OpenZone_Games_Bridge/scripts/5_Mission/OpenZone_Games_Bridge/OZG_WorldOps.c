// The stand's doors into the world half of OpenZone Games: the ring at a spot,
// a place made by script, characters the server makes and the script moves
// for, and a session asked the way a client's window asks it.
//
// A stand tool, never published.
class OZG_WorldOps
{
    protected static int s_Puppets;

    protected static vector At(map<string, string> args, string key)
    {
        string text = OZG_VerbOps.Arg(args, key, "");
        if (text == "")
            return vector.Zero;
        return text.ToVector();
    }

    protected static string F2(float f)
    {
        float r = Math.Round(f * 100.0) / 100.0;
        return r.ToString();
    }

    protected static string B(bool on)
    {
        if (on)
            return "1";
        return "0";
    }

    protected static string P3(vector v)
    {
        return F2(v[0]) + " " + F2(v[1]) + " " + F2(v[2]);
    }

    // The first connected player.
    protected static PlayerBase FirstPlayer()
    {
        array<Man> men = new array<Man>();
        GetGame().GetPlayers(men);
        for (int i = 0; i < men.Count(); i++)
        {
            PlayerBase p = PlayerBase.Cast(men[i]);
            if (p && p.GetIdentity())
                return p;
        }
        return null;
    }

    // who=me is the first connected player, anything else an id.
    protected static PlayerBase Who(map<string, string> args, out string uid)
    {
        uid = OZG_VerbOps.Arg(args, "who", "me");
        if (uid == "me")
        {
            PlayerBase me = FirstPlayer();
            uid = OZG_Who.Uid(me);
            return me;
        }
        return OZG_Sessions.PlayerOf(uid);
    }

    protected static OZG_Session Sess(map<string, string> args, out string detail)
    {
        string id = OZG_VerbOps.Arg(args, "id", "");
        OZG_Session s;
        if (id == "" || id == "0")
            s = OZG_Sessions.At(0);
        else if (id.Length() <= 2)
            s = OZG_Sessions.At(id.ToInt());
        else
            s = OZG_Sessions.ById(id);
        if (!s)
            detail = "no session '" + id + "'; " + OZG_Sessions.Describe();
        return s;
    }

    // ring at="x y z" [mode=0|1|2] [from="x y z"] [repeat=n]
    static bool OpRing(map<string, string> args, out string detail)
    {
        vector at = At(args, "at");
        if (at == vector.Zero)
        {
            PlayerBase me = FirstPlayer();
            if (!me)
            {
                detail = "ring: give at=\"x y z\"";
                return false;
            }
            at = me.GetPosition();
        }
        vector from = At(args, "from");
        if (from == vector.Zero)
            from = at + "1 0 0";
        int mode = OZG_VerbOps.Arg(args, "mode", "0").ToInt();
        int repeat = OZG_VerbOps.Arg(args, "repeat", "1").ToInt();
        if (repeat < 1)
            repeat = 1;

        OZG_RingResult ring;
        int t0 = TickCount(0);
        for (int i = 0; i < repeat; i++)
            ring = OZG_Ring.Compute(at, mode, from, null);
        int dt = TickCount(t0);
        float ms = dt / 10000.0;
        ms = ms / repeat;

        detail = "ring at " + P3(at) + " mode=" + mode.ToString() + " kind=" + ring.Kind.ToString();
        detail = detail + " raised=" + B(ring.Raised) + " floor=" + F2(ring.FloorY);
        detail = detail + " seats=" + ring.Count().ToString() + " ms=" + F2(ms) + " :";
        for (int k = 0; k < ring.Seats.Count(); k++)
        {
            OZG_SeatSpot s = ring.Seats[k];
            detail = detail + " " + s.Heading.ToString() + "@" + F2(vector.Distance(s.Xyz, at));
        }
        return true;
    }

    // lay at="x y z" [mode=] [deck=class] [yaw=]: a place made by script.
    static bool OpLay(map<string, string> args, out string detail)
    {
        vector at = At(args, "at");
        if (at == vector.Zero)
        {
            detail = "lay: give at=\"x y z\"";
            return false;
        }
        int mode = OZG_VerbOps.Arg(args, "mode", "0").ToInt();
        string deck = OZG_VerbOps.Arg(args, "deck", "OZG_Deck_Classic");
        float yaw = OZG_VerbOps.Arg(args, "yaw", "0").ToFloat();
        vector from = At(args, "from");
        if (from == vector.Zero)
            from = at + "1 0 0";

        if (OZG_Place.NearOther(at, OZG_Cfg.Get().PlaceSpacing, null))
        {
            detail = "lay refused: " + OZG_Const.LAY_NO_SPACING;
            return true;
        }
        OZG_RingResult ring = OZG_Ring.Compute(at, mode, from, null);
        if (ring.Count() < 2)
        {
            detail = "lay refused: " + OZG_Const.LAY_NO_ROOM + " seats=" + ring.Count().ToString();
            return true;
        }
        OZG_Session s = OZG_Sessions.Make(deck, mode, at, Vector(yaw, 0, 0), ring);
        if (!s)
        {
            detail = "lay: no place was made";
            return false;
        }
        s.Changed();
        detail = "laid " + s.Id() + " seats=" + ring.Count().ToString() + " kind=" + ring.Kind.ToString();
        return true;
    }

    static bool OpPlaces(out string detail)
    {
        detail = OZG_Sessions.Describe();
        for (int i = 0; i < OZG_Sessions.Count(); i++)
        {
            OZG_Session s = OZG_Sessions.At(i);
            if (s)
                detail = detail + " | " + i.ToString() + "=" + s.Id() + " st=" + s.m_R.St.ToString() + " busy=" + s.Busy().ToString();
        }
        return true;
    }

    static bool OpSess(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        detail = s.Describe();
        return true;
    }

    // seats id=: where the seats are.
    static bool OpSeats(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        detail = "seats " + s.Id() + ":";
        for (int i = 0; i < s.m_R.Seats.Count(); i++)
        {
            OZG_SeatRec r = s.m_R.Seats[i];
            detail = detail + " " + i.ToString() + "=" + P3(r.Where()) + "/" + r.St.ToString();
            if (r.Uid != "")
                detail = detail + "/" + r.Uid;
        }
        return true;
    }

    // puppet id= seat=: a character made at a seat and seated on it.
    static bool OpPuppet(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int no = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        if (no < 0 || no >= s.m_R.Seats.Count())
        {
            detail = "puppet: no seat " + no.ToString();
            return false;
        }
        vector at = s.m_R.Seats[no].Where();
        string type = OZG_VerbOps.Arg(args, "type", "SurvivorM_Mirek");
        PlayerBase body = PlayerBase.Cast(GetGame().CreateObjectEx(type, at, ECE_PLACE_ON_SURFACE));
        if (!body)
        {
            detail = "puppet: " + type + " was not made";
            return false;
        }
        body.SetPosition(at);
        s_Puppets++;
        string uid = "puppet" + s_Puppets.ToString();
        string name = OZG_VerbOps.Arg(args, "name", "Puppet " + s_Puppets.ToString());
        OZG_Who.Fake(body, uid, name);

        string why;
        bool sat = s.Sit(body, no, why);
        detail = uid + " made at seat " + no.ToString() + " sat=" + B(sat);
        if (!sat)
            detail = detail + " (" + why + ")";
        return true;
    }

    // move who= at="x y z": a character is put somewhere.
    static bool OpMove(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "move: nobody is '" + uid + "'";
            return false;
        }
        vector at = At(args, "at");
        p.SetPosition(at);
        detail = uid + " at " + P3(p.GetPosition());
        return true;
    }

    // ask id= who= ask=<op> [seat=] [move=] [how=] [opt=] [val=] [pose=]
    static bool OpAsk(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "ask: nobody is '" + uid + "'";
            return false;
        }
        string what = OZG_VerbOps.Arg(args, "ask", "");
        OZG_WAsk ask = new OZG_WAsk();
        ask.PlaceId = s.Id();
        ask.SeatNo = OZG_VerbOps.Arg(args, "seat", "-1").ToInt();
        ask.Move = OZG_VerbOps.Arg(args, "move", "");
        ask.How = OZG_VerbOps.Arg(args, "how", "");
        ask.OptId = OZG_VerbOps.Arg(args, "opt", "");
        ask.OptValue = OZG_VerbOps.Arg(args, "val", "");
        ask.PoseNo = OZG_VerbOps.Arg(args, "pose", "0").ToInt();

        if (what == OZG_Const.OP_FOLD)
        {
            if (!s.IsHost(uid))
            {
                detail = "refused " + OZG_Const.SAY_NOT_HOST;
                return true;
            }
            OZG_Sessions.Fold(s, p, OZG_Const.SAY_FOLDED);
            detail = "folded; " + OZG_Sessions.Describe();
            return true;
        }

        string why = s.Ask(p, what, ask);
        if (why != "")
        {
            detail = "refused " + why;
            return true;
        }
        detail = "ok; " + s.Describe();
        return true;
    }

    // sit id= who= [seat=n]: straight onto a seat, or the action on the place.
    static bool OpSit(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "sit: nobody is '" + uid + "'";
            return false;
        }
        int no = OZG_VerbOps.Arg(args, "seat", "-1").ToInt();
        if (no < 0)
        {
            s.SitAction(p);
            detail = "sit action for " + uid + "; " + s.Describe();
            return true;
        }
        if (OZG_VerbOps.Arg(args, "put", "1") == "1")
            p.SetPosition(s.m_R.Seats[no].Where());
        string why;
        bool sat = s.Sit(p, no, why);
        detail = "sat=" + B(sat);
        if (!sat)
            detail = detail + " (" + why + ")";
        detail = detail + "; " + s.Describe();
        return true;
    }

    // ticks n=: the mod's clock run forward.
    static bool OpTicks(map<string, string> args, out string detail)
    {
        int n = OZG_VerbOps.Arg(args, "n", "1").ToInt();
        for (int i = 0; i < n; i++)
            OZG_Sessions.Tick();
        detail = "ticked " + n.ToString() + "; " + OZG_Sessions.Describe();
        return true;
    }

    // gather id= who=: the action "Gather the deck".
    static bool OpGather(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        PlayerBase p = Who(args, uid);
        OZG_Sessions.GatherAction(s.m_Place, p);
        detail = "gather by " + uid + "; " + OZG_Sessions.Describe();
        return true;
    }

    // hand id= seat=: a seat's own half of the view, and how long the texts are.
    static bool OpHand(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        if (!s.m_Game)
        {
            detail = "hand: no game";
            return true;
        }
        int no = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        string own = s.m_Game.SeatView(no);
        string shared = s.m_Game.PublicView();
        detail = "pub=" + shared.Length().ToString() + "B lf=" + shared.IndexOf("\n").ToString();
        detail = detail + " own=" + own.Length().ToString() + "B: " + own;
        return true;
    }

    // chat text="!ozg ...": a chat command as the first connected player
    // would have typed it.
    static bool OpChat(map<string, string> args, out string detail)
    {
        PlayerBase me = FirstPlayer();
        if (!me)
        {
            detail = "chat: nobody is connected";
            return false;
        }
        string text = OZG_VerbOps.Arg(args, "text", "!ozg");
        OZG_Bots.OnChat(OZG_Who.Name(me), text);
        detail = "said '" + text + "' as " + OZG_Who.Name(me) + "; " + OZG_Sessions.Describe();
        return true;
    }

    // bots id= n= [on=0|1]: bots on a session's free seats, playing or waiting.
    static bool OpBots(map<string, string> args, out string detail)
    {
        string on = OZG_VerbOps.Arg(args, "on", "1");
        bool playing = on == "1";
        OZG_Bots.s_On = playing;
        int n = OZG_VerbOps.Arg(args, "n", "0").ToInt();
        string names = "";
        if (n > 0)
        {
            OZG_Session s = Sess(args, detail);
            if (!s)
                return false;
            for (int i = 0; i < n; i++)
            {
                string name = OZG_Bots.Seat(s);
                if (name == "")
                    break;
                names = names + " " + name;
            }
        }
        detail = "bots on=" + on + " seated:" + names;
        return true;
    }

    // auto id= n=: up to n moves played for whoever the game waits for, people
    // and bots alike -- a game played out to its end in one call.
    static bool OpAuto(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int n = OZG_VerbOps.Arg(args, "n", "20").ToInt();
        int made = 0;
        for (int i = 0; i < n; i++)
        {
            if (!s.m_Game || s.m_R.St != OZG_Const.ST_PLAYING)
                break;
            bool moved = false;
            for (int k = 0; k < s.m_R.Seats.Count(); k++)
            {
                OZG_SeatRec seat = s.m_R.Seats[k];
                if (seat.Uid == "" || !s.m_Game.Waits(k))
                    continue;
                string move = OZG_Bots.Choose(s.m_Game, k);
                PlayerBase body = OZG_Sessions.PlayerOf(seat.Uid);
                if (move == "" || !body)
                    continue;
                OZG_WAsk ask = new OZG_WAsk();
                ask.PlaceId = s.Id();
                ask.Move = move;
                if (s.Ask(body, OZG_Const.OP_ACT, ask) == "")
                {
                    moved = true;
                    made++;
                    break;
                }
            }
            if (!moved)
                break;
        }
        detail = "played " + made.ToString() + "; " + s.Describe();
        return true;
    }

    // face who= id=: where a player faces, and the facing of his seat.
    static bool OpFace(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "face: nobody is '" + uid + "'";
            return false;
        }
        detail = uid + " faces " + F2(p.GetOrientation()[0]) + " at " + P3(p.GetPosition());
        OZG_Session s = OZG_Sessions.SeatedIn(uid);
        if (s)
        {
            int no = s.SeatOf(uid);
            detail = detail + "; seat " + no.ToString() + " faces " + F2(s.m_R.Seats[no].Yaw);
        }
        return true;
    }

    // give who= [deck=class]: a deck into a player's hands.
    static bool OpGive(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "give: nobody is '" + uid + "'";
            return false;
        }
        string deck = OZG_VerbOps.Arg(args, "deck", "OZG_Deck_Classic");
        EntityAI made = p.GetHumanInventory().CreateInHands(deck);
        if (!made)
            made = p.GetInventory().CreateInInventory(deck);
        bool given = made != null;
        detail = "give " + deck + " to " + uid + ": " + B(given);
        return true;
    }
}
