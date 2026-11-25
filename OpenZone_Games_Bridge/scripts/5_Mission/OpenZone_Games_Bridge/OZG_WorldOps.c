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

    // ring at="x y z" [mode=0|1|2|3] [from="x y z"] [repeat=n]
    // The modes are OZG_Const.MODE_*: 0 the mod's guess, 1 standing, 2 on the
    // level of the mat, 3 sitting below it. A seat is said as
    // heading@distance/height over the mat.
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
        detail = detail + " lay=" + ring.LayMode.ToString();
        detail = detail + " raised=" + B(ring.Raised) + " floor=" + F2(ring.FloorY);
        detail = detail + " seats=" + ring.Count().ToString() + " ms=" + F2(ms) + " :";
        vector flat;
        for (int k = 0; k < ring.Seats.Count(); k++)
        {
            OZG_SeatSpot s = ring.Seats[k];
            flat = Vector(s.Xyz[0], at[1], s.Xyz[2]);
            detail = detail + " " + s.Heading.ToString() + "@" + F2(vector.Distance(flat, at));
            detail = detail + "/" + F2(s.Xyz[1] - at[1]);
        }
        return true;
    }

    // level at="x y z" to="x y z" [n=8] [up=0.3]: what the ring's floor reader
    // says along a line, n points from `at` towards `to`, each read from `up`
    // over at's height. A point is said as distance:road/ray/level, where road
    // is the game's road surface under the start, ray the first thing a ray
    // down meets (its height and class, "-" for nothing) and level what
    // OZG_Ring.LevelUnder answers.
    static bool OpLevel(map<string, string> args, out string detail)
    {
        vector at = At(args, "at");
        vector to = At(args, "to");
        if (at == vector.Zero || to == vector.Zero)
        {
            detail = "level: give at=\"x y z\" and to=\"x y z\"";
            return false;
        }
        int n = OZG_VerbOps.Arg(args, "n", "8").ToInt();
        if (n < 1)
            n = 1;
        if (n > 24)
            n = 24;
        float up = OZG_VerbOps.Arg(args, "up", "0.3").ToFloat();
        float top = at[1] + up;

        set<Object> seen = new set<Object>();
        vector cpos;
        vector cdir;
        int comp;
        vector from;
        vector down;
        float x;
        float z;
        float f;
        float road;
        float lvl;
        string ray;
        Object hit;
        detail = "level from " + F2(top) + ":";
        for (int i = 0; i < n; i++)
        {
            f = i;
            f = f / n;
            x = at[0] + (to[0] - at[0]) * f;
            z = at[2] + (to[2] - at[2]) * f;
            road = GetGame().SurfaceRoadY3D(x, top, z, RoadSurfaceDetection.UNDER);
            lvl = OZG_Ring.LevelUnder(x, top, z, null);

            from = Vector(x, top, z);
            down = Vector(x, top - 3.0, z);
            seen.Clear();
            ray = "-";
            if (DayZPhysics.RaycastRV(from, down, cpos, cdir, comp, seen, null, null, false, false, ObjIntersectGeom))
            {
                ray = F2(cpos[1]);
                if (seen.Count() > 0)
                {
                    hit = seen.Get(0);
                    if (hit)
                        ray = ray + "=" + hit.GetType();
                }
            }
            detail = detail + " " + F2(vector.Distance(Vector(x, at[1], z), at)) + ":" + F2(road);
            detail = detail + "/" + ray + "/" + F2(lvl);
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
        OZG_Session s = OZG_Sessions.Make(deck, ring.LayMode, at, Vector(yaw, 0, 0), ring);
        if (!s)
        {
            detail = "lay: no place was made";
            return false;
        }
        // No hands lay this one out: its deck is made in it.
        s.m_Place.OZG_MakeDeck(deck);
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
        body.SetAllowDamage(false);
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

    // The deck a player holds in his hands, or the first deck in his
    // inventory; null when he has none.
    static OZG_Deck_Base DeckOf(PlayerBase p)
    {
        if (!p)
            return null;
        OZG_Deck_Base deck = OZG_Deck_Base.Cast(p.GetHumanInventory().GetEntityInHands());
        if (deck)
            return deck;
        array<EntityAI> items = new array<EntityAI>();
        p.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        for (int i = 0; i < items.Count(); i++)
        {
            deck = OZG_Deck_Base.Cast(items[i]);
            if (deck)
                return deck;
        }
        return null;
    }

    // `deck <class> slots= cards= free= faces=<different faces> looks=<look>:<how many>,..`
    static string DeckLine(OZG_Deck_Base deck)
    {
        int held = deck.GetInventory().AttachmentCount();
        string s = "deck " + deck.GetType() + " slots=" + deck.OZG_Slots().ToString();
        s = s + " cards=" + held.ToString() + " free=" + deck.OZG_Free().ToString();
        array<string> faces = new array<string>();
        array<int> looks = new array<int>();
        array<int> counts = new array<int>();
        for (int i = 0; i < held; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(deck.GetInventory().GetAttachmentFromIndex(i));
            if (!card)
                continue;
            if (faces.Find(card.OZG_Face()) < 0)
                faces.Insert(card.OZG_Face());
            int at = looks.Find(card.OZG_Look());
            if (at < 0)
            {
                looks.Insert(card.OZG_Look());
                counts.Insert(1);
            }
            else
            {
                counts[at] = counts[at] + 1;
            }
        }
        s = s + " faces=" + faces.Count().ToString() + " looks=";
        for (int k = 0; k < looks.Count(); k++)
        {
            if (k > 0)
                s = s + ",";
            s = s + looks[k].ToString() + ":" + counts[k].ToString();
        }
        return s;
    }

    // A card out of a deck for each face named, and gone; a face named twice
    // takes two cards. Returns how many were found.
    static int PullFrom(OZG_Deck_Base deck, string faces)
    {
        array<string> wanted = new array<string>();
        OZG_Cards.Parse(faces, wanted);
        int gone = 0;
        int held = deck.GetInventory().AttachmentCount();
        for (int i = 0; i < held; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(deck.GetInventory().GetAttachmentFromIndex(i));
            if (!card)
                continue;
            int at = wanted.Find(card.OZG_Face());
            if (at < 0)
                continue;
            wanted.Remove(at);
            GetGame().ObjectDelete(card);
            gone++;
        }
        return gone;
    }

    // cards who=: the deck a player carries, counted.
    static bool OpCards(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        OZG_Deck_Base deck = DeckOf(p);
        if (!deck)
        {
            detail = "cards: '" + uid + "' carries no deck";
            return true;
        }
        detail = DeckLine(deck);
        return true;
    }

    // pull who= face="<face> [<face> ..]", or pull id= face=..: a card of each
    // face out of the deck a player carries, or out of a laid-out game's deck,
    // and gone.
    static bool OpPull(map<string, string> args, out string detail)
    {
        string faces = OZG_VerbOps.Arg(args, "face", "As");
        OZG_Deck_Base deck;
        if (OZG_VerbOps.Arg(args, "id", "") != "")
        {
            OZG_Session s = Sess(args, detail);
            if (!s)
                return false;
            deck = s.m_Place.OZG_Deck();
        }
        else
        {
            string uid;
            PlayerBase p = Who(args, uid);
            deck = DeckOf(p);
        }
        if (!deck)
        {
            detail = "pull: no deck there";
            return true;
        }
        detail = "pulled " + PullFrom(deck, faces).ToString() + " of '" + faces + "'; a tick later: cards or held";
        return true;
    }

    // stuff id= face=<face> [style=classic] [n=1]: cards of a face put into a
    // laid-out game: into the deck's free slots, then beside it.
    static bool OpStuff(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string face = OZG_VerbOps.Arg(args, "face", "As");
        string style = OZG_VerbOps.Arg(args, "style", "classic");
        int n = OZG_VerbOps.Arg(args, "n", "1").ToInt();
        OZG_Deck_Base deck = s.m_Place.OZG_Deck();
        int inDeck = 0;
        int beside = 0;
        for (int i = 0; i < n; i++)
        {
            OZG_Card_Base card;
            if (deck && deck.OZG_Free() > 0)
                card = OZG_Card_Base.Cast(deck.GetInventory().CreateAttachment(OZG_Const.CARD_PREFIX + face));
            if (card)
            {
                inDeck++;
            }
            else
            {
                s.m_Place.OZG_Open();
                card = OZG_Card_Base.Cast(s.m_Place.GetInventory().CreateEntityInCargo(OZG_Const.CARD_PREFIX + face));
                if (card)
                    beside++;
            }
            if (card)
                card.OZG_SetStyle(style);
        }
        detail = "stuffed " + face + ": deck=" + inDeck.ToString() + " beside=" + beside.ToString();
        return true;
    }

    // undeck who=: every deck a player carries is deleted, with its cards.
    static bool OpUndeck(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "undeck: nobody is '" + uid + "'";
            return false;
        }
        array<EntityAI> items = new array<EntityAI>();
        p.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        EntityAI held = p.GetHumanInventory().GetEntityInHands();
        if (held && items.Find(held) < 0)
            items.Insert(held);
        int gone = 0;
        for (int i = 0; i < items.Count(); i++)
        {
            if (items[i] && items[i].IsInherited(OZG_Deck_Base))
            {
                GetGame().ObjectDelete(items[i]);
                gone++;
            }
        }
        detail = "undeck: " + gone.ToString() + " deck(s) of " + uid + " deleted";
        return true;
    }

    // held id=: what the place holds: its deck, and the cards beside it.
    static bool OpHeld(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int beside = 0;
        CargoBase cargo = s.m_Place.GetInventory().GetCargo();
        if (cargo)
            beside = cargo.GetItemCount();
        OZG_Deck_Base deck = s.m_Place.OZG_Deck();
        if (!deck)
        {
            detail = "held: no deck; beside=" + beside.ToString();
            return true;
        }
        detail = "held: " + DeckLine(deck) + " beside=" + beside.ToString();
        return true;
    }

    // loose at="x y z" [r=2] [sweep=1]: the cards that lie on the ground near
    // a point, each as face/look; with sweep they are deleted.
    static bool OpLoose(map<string, string> args, out string detail)
    {
        vector at = At(args, "at");
        float r = OZG_VerbOps.Arg(args, "r", "2").ToFloat();
        bool sweep = OZG_VerbOps.Arg(args, "sweep", "0") == "1";
        array<Object> found = new array<Object>();
        GetGame().GetObjectsAtPosition(at, r, found, null);
        int n = 0;
        string faces = "";
        for (int i = 0; i < found.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(found[i]);
            if (!card || card.GetHierarchyParent())
                continue;
            n++;
            faces = faces + " " + card.OZG_Face() + "/" + card.OZG_Look().ToString();
            if (sweep)
                GetGame().ObjectDelete(card);
        }
        detail = "loose: " + n.ToString() + faces;
        return true;
    }

    // give who= [what=class | deck=class]: a thing into a player's hands,
    // else into the inventory.
    static bool OpGive(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "give: nobody is '" + uid + "'";
            return false;
        }
        string deck = OZG_VerbOps.Arg(args, "what", "");
        if (deck == "")
            deck = OZG_VerbOps.Arg(args, "deck", "OZG_Deck_Classic");
        EntityAI made = p.GetHumanInventory().CreateInHands(deck);
        if (!made)
            made = p.GetInventory().CreateInInventory(deck);
        bool given = made != null;
        detail = "give " + deck + " to " + uid + ": " + B(given);
        return true;
    }

    // The cards `mark` and `markof` look at: in=me those the first player
    // carries, at any depth; otherwise those of the laid-out game id=.
    protected static bool CardsIn(map<string, string> args, array<EntityAI> into, out string detail)
    {
        into.Clear();
        if (OZG_VerbOps.Arg(args, "in", "place") == "me")
        {
            PlayerBase me = FirstPlayer();
            if (!me)
            {
                detail = "nobody is connected";
                return false;
            }
            me.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, into);
            return true;
        }
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        OZG_DeckRead.ThingsOf(s.m_Place, into);
        return true;
    }

    // mark [id=<place> | in=me] face=<face>|all [who=me|<id>] [n=1] [sign=0]:
    // n cards of that face (all: every card) marked by `who`, as the marking
    // tool would; sign 0 draws a sign by chance. Only for the stand: a player
    // cannot mark a card that lies in a place (design §4.2).
    static bool OpMark(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        if (uid == "")
        {
            detail = "mark: nobody to mark it";
            return true;
        }
        string face = OZG_VerbOps.Arg(args, "face", "As");
        int n = OZG_VerbOps.Arg(args, "n", "1").ToInt();
        if (face == "all")
            n = things.Count();
        int sign = OZG_VerbOps.Arg(args, "sign", "0").ToInt();
        OZG_Random dice = new OZG_Random();
        int done = 0;
        for (int i = 0; i < things.Count() && done < n; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            int drawn = sign;
            if (drawn <= 0)
                drawn = 1 + dice.Below(99);
            card.OZG_MarkBy(uid, drawn);
            done++;
        }
        detail = "marked " + done.ToString() + " of " + face;
        return true;
    }

    // markof [id=<place> | in=me] face=<face>|all [who=me|<id>] [only=notices]:
    // each card of that face (all: each marked card) as its face, its sign,
    // whether `who` marked it, reads it and notices it, and how many notice
    // it. only=notices keeps the cards `who` notices, and counts them first.
    static bool OpMarkOf(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        string face = OZG_VerbOps.Arg(args, "face", "As");
        bool onlyNoticed = OZG_VerbOps.Arg(args, "only", "") == "notices";
        string list = "";
        int noticed = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            if (face == "all" && !card.OZG_Marked())
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            if (onlyNoticed && !card.OZG_Notices(uid))
                continue;
            if (card.OZG_Notices(uid))
                noticed++;
            string one = " " + card.OZG_Face() + " sign=" + card.OZG_Sign().ToString();
            one = one + " by_who=" + B(card.OZG_MarkedBy() == uid);
            one = one + " reads=" + B(card.OZG_Reads(uid));
            one = one + " notices=" + B(card.OZG_Notices(uid));
            one = one + " noticers=" + card.OZG_NoticerCount().ToString();
            list = list + one + ";";
        }
        detail = face + ":";
        if (onlyNoticed)
            detail = detail + " " + noticed.ToString() + " noticed:";
        detail = detail + list;
        return true;
    }

    // reads id=<place> seat=<n>: what the view of that seat's player carries
    // as reads: "<seat>:<face>/<sign>", -1 for the top of the stock.
    static bool OpReads(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        if (seat < 0 || seat >= s.m_R.Seats.Count() || !s.m_Game)
        {
            detail = "reads: no seat " + seat.ToString() + " in a game";
            return true;
        }
        array<ref OZG_WRead> reads = new array<ref OZG_WRead>();
        s.m_Game.ReadsFor(seat, s.m_R.Seats[seat].Uid, reads);
        detail = "reads seat " + seat.ToString() + ":";
        if (reads.Count() == 0)
            detail = detail + " none";
        for (int i = 0; i < reads.Count(); i++)
        {
            string one = " " + reads[i].Seat.ToString() + ":" + reads[i].Face;
            detail = detail + one + "/" + reads[i].Sign.ToString();
        }
        return true;
    }

    // craft what=mark|loupe [face=As] [in=me | in=place id=<place>]: the server's
    // half of a finished gesture, as the game's crafting action ends one
    // (PerformRecipeServer): the thing in the first player's hands on the
    // first card of that face the player carries (in=place: in the laid-out
    // game's deck, which the recipe must refuse). The recipe's own check
    // comes first, so a refusal is an answer and no script error.
    static bool OpCraft(map<string, string> args, out string detail)
    {
        PlayerBase me = FirstPlayer();
        if (!me)
        {
            detail = "craft: nobody is connected";
            return false;
        }
        string uid = OZG_Who.Uid(me);
        string what = OZG_VerbOps.Arg(args, "what", "mark");
        RecipeBase recipe = OZG_Recipes.MarkRecipe();
        string recipeName = "OZG_RecipeMark";
        if (what == "loupe")
        {
            recipe = OZG_Recipes.LoupeRecipe();
            recipeName = "OZG_RecipeLoupe";
        }
        if (OZG_VerbOps.Arg(args, "in", "") == "")
            args.Set("in", "me");
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string face = OZG_VerbOps.Arg(args, "face", "As");
        OZG_Card_Base card = null;
        for (int i = 0; i < things.Count() && !card; i++)
        {
            OZG_Card_Base one = OZG_Card_Base.Cast(things[i]);
            if (one && one.OZG_Face() == face)
                card = one;
        }
        ItemBase hands = me.GetItemInHands();
        if (!recipe || !hands || !card)
        {
            detail = "craft " + what + ": hands=" + B(hands != null) + " card=" + B(card != null);
            return true;
        }
        // The thing in the hands must be this recipe's tool: the recipe's own
        // check does not ask what the two things are.
        if (!recipe.CheckIngredientMatch(hands, card) || !recipe.CheckRecipe(hands, card, me))
        {
            // Which check said no: the recipe's, one at a time.
            ItemBase both[2];
            both[0] = hands;
            both[1] = card;
            bool match = recipe.CheckIngredientMatch(hands, card);
            bool can = recipe.CanDo(both, me);
            bool fits = recipe.CheckConditions(both);
            detail = "craft " + what + ": refused (hands " + hands.GetType();
            detail = detail + " q=" + F2(hands.GetQuantity()) + " hl=" + hands.GetHealthLevel().ToString();
            detail = detail + " match=" + B(match) + " cando=" + B(can) + " cond=" + B(fits);
            detail = detail + " tool=" + B(OZG_Recipes.Carried(hands, me)) + " card=" + B(OZG_Recipes.Carried(card, me));
            EntityAI up = card.GetHierarchyParent();
            string parentType = "none";
            if (up)
                parentType = up.GetType();
            Man root = card.GetHierarchyRootPlayer();
            detail = detail + " in=" + parentType + " root=" + B(root != null);
            detail = detail + " att=" + card.GetInventory().AttachmentCount().ToString() + ")";
            return true;
        }
        PluginRecipesManager plugin;
        Class.CastTo(plugin, GetPlugin(PluginRecipesManager));
        int id = PluginRecipesManager.RecipeIDFromClassname(recipeName);
        plugin.PerformRecipeServer(id, hands, card, me);
        detail = "craft " + what + " on " + card.OZG_Face() + ": sign=" + card.OZG_Sign().ToString();
        detail = detail + " reads=" + B(card.OZG_Reads(uid));
        if (hands.HasQuantity())
            detail = detail + " left=" + F2(hands.GetQuantity());
        return true;
    }

    // noticecfg [grace=] [step=] [max=]: the settings of noticing (design
    // §7) set on the running server, through the settings' own bounds, for a
    // check: the file is not written, and a restart reads it again. Answers
    // the three as they stand and how many the bounds put right.
    static bool OpNoticeCfg(map<string, string> args, out string detail)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        string grace = OZG_VerbOps.Arg(args, "grace", "");
        string rise = OZG_VerbOps.Arg(args, "step", "");
        string most = OZG_VerbOps.Arg(args, "max", "");
        if (grace != "")
            cfg.NoticeGrace = grace.ToInt();
        if (rise != "")
            cfg.NoticeStep = rise.ToInt();
        if (most != "")
            cfg.NoticeMax = most.ToInt();
        int putRight;
        cfg.Validate(putRight);
        detail = "notice grace=" + cfg.NoticeGrace.ToString() + " step=" + cfg.NoticeStep.ToString();
        detail = detail + " max=" + cfg.NoticeMax.ToString() + " putright=" + putRight.ToString();
        return true;
    }

    // noticer [id=<place> | in=me] face=<face>|all [who=me|<id>] [many=0]:
    // `who` made a noticer of each marked card of that face (all: of every
    // marked card), as a won roll would (design §5); with many=k the ids
    // <who>1 .. <who>k are made noticers in turn. Only for the stand.
    static bool OpNoticer(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        if (uid == "")
        {
            detail = "noticer: nobody to notice it";
            return true;
        }
        string face = OZG_VerbOps.Arg(args, "face", "As");
        int many = OZG_VerbOps.Arg(args, "many", "0").ToInt();
        array<string> ids = new array<string>();
        if (many <= 0)
            ids.Insert(uid);
        for (int m = 1; m <= many; m++)
            ids.Insert(uid + m.ToString());
        int done = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card || !card.OZG_Marked())
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            for (int k = 0; k < ids.Count(); k++)
                card.OZG_AddNoticer(ids[k]);
            done++;
        }
        detail = "noticer: " + ids.Count().ToString() + " id(s)";
        detail = detail + " on " + done.ToString() + " card(s) of " + face;
        return true;
    }

    // whole id=<place>: every hidden card of the game in play, in one line:
    // each hand as <seat>[cards], the table, the stock from the card drawn
    // next to the trump at its end (the game's Everything; never sent to a
    // client).
    static bool OpWhole(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        if (!s.m_Game)
        {
            detail = "whole: no game";
            return true;
        }
        detail = s.m_Game.Everything();
        return true;
    }

    // How many cards of a place the id `uid` notices.
    protected static int NoticedIn(OZG_Place place, string uid)
    {
        array<EntityAI> things = new array<EntityAI>();
        OZG_DeckRead.ThingsOf(place, things);
        int n = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (card && card.OZG_Notices(uid))
                n++;
        }
        return n;
    }

    // redeal id=<place> [n=10]: n deals in a row, as fast as the server runs
    // them. The game in play is ended where it stands (the stand's shortcut:
    // no move is played, nothing is said) and the host deals the next, as
    // "Deal" in the window does; the last deal is left in play. For every
    // seated player a deal that counted towards noticing is a trial at the
    // chance his count gives, and one more card he notices is a hit (design
    // §5). Answers the trials and hits at no chance (the grace), between, and
    // at the most, with the hits to expect (the sum of the chances) and their
    // spread (the root of the sum of p(1 - p)), overall and for the two.
    static bool OpRedeal(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        PlayerBase host = OZG_Sessions.PlayerOf(s.m_R.HostUid);
        if (!host)
        {
            detail = "redeal: the host is not here";
            return true;
        }
        int n = OZG_VerbOps.Arg(args, "n", "10").ToInt();
        OZG_Settings cfg = OZG_Cfg.Get();
        int top = cfg.NoticeMax;
        int graceTrials = 0;
        int graceHits = 0;
        int rampTrials = 0;
        int rampHits = 0;
        int topTrials = 0;
        int topHits = 0;
        float rampExpect = 0;
        float topExpect = 0;
        float spread = 0;
        array<string> uids = new array<string>();
        array<int> counts = new array<int>();
        array<int> seen = new array<int>();
        int k;
        for (int d = 0; d < n; d++)
        {
            uids.Clear();
            counts.Clear();
            seen.Clear();
            for (k = 0; k < s.m_R.Seats.Count(); k++)
            {
                OZG_SeatRec seat = s.m_R.Seats[k];
                if (seat.St != OZG_Const.SEAT_TAKEN || seat.Uid == "")
                    continue;
                uids.Insert(seat.Uid);
                counts.Insert(s.NoticeDeals(seat.Uid));
                seen.Insert(NoticedIn(s.m_Place, seat.Uid));
            }
            if (s.m_R.St == OZG_Const.ST_PLAYING)
                s.m_R.St = OZG_Const.ST_FINISHED;
            OZG_WAsk ask = new OZG_WAsk();
            ask.PlaceId = s.Id();
            string why = s.Ask(host, OZG_Const.OP_DEAL, ask);
            if (why != "")
            {
                detail = "redeal: deal " + d.ToString() + " refused " + why;
                return true;
            }
            for (k = 0; k < uids.Count(); k++)
            {
                int now = s.NoticeDeals(uids[k]);
                if (now == counts[k])
                    continue;
                int chance = cfg.NoticeChance(now);
                bool hit = NoticedIn(s.m_Place, uids[k]) > seen[k];
                float p = chance / 100.0;
                spread = spread + p * (1 - p);
                if (chance <= 0)
                {
                    graceTrials++;
                    if (hit)
                        graceHits++;
                }
                else if (chance >= top)
                {
                    topTrials++;
                    topExpect = topExpect + p;
                    if (hit)
                        topHits++;
                }
                else
                {
                    rampTrials++;
                    rampExpect = rampExpect + p;
                    if (hit)
                        rampHits++;
                }
            }
        }
        int trials = graceTrials + rampTrials + topTrials;
        int hits = graceHits + rampHits + topHits;
        float expect = rampExpect + topExpect;
        detail = "redeal " + n.ToString() + ": trials=" + trials.ToString();
        detail = detail + " hits=" + hits.ToString() + " expect=" + F2(expect);
        detail = detail + " sd=" + F2(Math.Sqrt(spread));
        detail = detail + " | grace " + graceTrials.ToString() + "/" + graceHits.ToString();
        detail = detail + " | ramp " + rampTrials.ToString() + "/" + rampHits.ToString();
        detail = detail + " of " + F2(rampExpect);
        detail = detail + " | top " + topTrials.ToString() + "/" + topHits.ToString();
        detail = detail + " of " + F2(topExpect);
        return true;
    }

    // notices id=<place> [seat=<n> | who=me|<id>]: the notices in the head of
    // the view that player would be sent now (the session's own head, design
    // §4.6): a back as "b<seat>:<look>/<sign>" (-1 the top of the stock), a
    // face as "f<card>/<sign>"; with his seat (mine=-1 for one who holds
    // none), the deals that counted towards noticing for him, how many reads
    // the head carries and its length in bytes.
    static bool OpNotices(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        string seatArg = OZG_VerbOps.Arg(args, "seat", "");
        if (seatArg != "")
        {
            int no = seatArg.ToInt();
            if (no < 0 || no >= s.m_R.Seats.Count())
            {
                detail = "notices: no seat " + seatArg;
                return true;
            }
            uid = s.m_R.Seats[no].Uid;
        }
        else
        {
            Who(args, uid);
        }
        string head = s.HeadFor(uid);
        OZG_WView v;
        string err;
        if (!JsonFileLoader<OZG_WView>.LoadData(head, v, err) || !v)
        {
            detail = "notices: the head did not read: " + err;
            return true;
        }
        int reads = 0;
        if (v.Reads)
            reads = v.Reads.Count();
        detail = "notices of " + uid + " mine=" + v.Mine.ToString();
        detail = detail + " deals=" + s.NoticeDeals(uid).ToString();
        detail = detail + " reads=" + reads.ToString() + " head=" + head.Length().ToString() + "B:";
        if (!v.Notices || v.Notices.Count() == 0)
        {
            detail = detail + " none";
            return true;
        }
        for (int i = 0; i < v.Notices.Count(); i++)
        {
            OZG_WNotice n = v.Notices[i];
            string one = " f" + n.CardName;
            if (n.CardName == "")
                one = " b" + n.Seat.ToString() + ":" + n.Look.ToString();
            detail = detail + one + "/" + n.Sign.ToString();
        }
        return true;
    }

    // shake [id=<place>] [region=seat<n>|discards|stock] [sound=<name>]
    // [who=me|<id>]: the card sharp's message to a player who saw a sleight
    // (card sharp's design §9), sent as the fourth phase will send it: his
    // client plays the sound for him alone and, when his window is open on
    // that place, shakes the region.
    static bool OpShake(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p || !p.GetIdentity())
        {
            detail = "shake: nobody is '" + uid + "'";
            return false;
        }
        OZG_WShake w = new OZG_WShake();
        w.PlaceId = s.Id();
        w.RegionName = OZG_VerbOps.Arg(args, "region", "");
        w.SoundName = OZG_VerbOps.Arg(args, "sound", "");
        string body;
        string err;
        JsonFileLoader<OZG_WShake>.MakeData(w, body, err, false);
        OZ_Rpc.ServiceRespond(p.GetIdentity(), OZG_Const.SERVICE, OZG_Const.OP_SHAKE, true, body, "");
        detail = "shake sent to " + uid + ": region=" + w.RegionName;
        detail = detail + " sound=" + w.SoundName;
        return true;
    }
}
