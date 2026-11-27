// What the dice games share (boards and dice design §6, §8): the seats in their
// order, whose dice each throws, every die as it fell, and the game kept as
// the moves and the dice: loading begins the game again and hands the dice
// out as they fell instead of throwing them.
class OZG_DiceGameConst
{
    static const string M_THROW = "throw";
    // A seat gone, kept among the moves so a loaded game has it too.
    static const string M_DROP = "~drop";
    static const string SHORT_DICE = "STR_OZG_DICE_SHORT";
}

class OZG_DiceGame extends OZG_Game
{
    protected ref array<int> m_Seats;
    protected ref OZG_DiceCups m_Cups;
    protected ref OZG_Random m_Chance;
    protected string m_Whose;
    protected string m_Opts;
    protected string m_Hint;
    // "<seat> <move>" as acted, and every die thrown, in order.
    protected ref array<string> m_Moves;
    protected ref array<int> m_Rolled;
    // While loading: the dice to hand out instead of throwing.
    protected ref array<int> m_Replay;
    // The stacks the game began with (a game for chips).
    protected ref array<int> m_StartSeats;
    protected ref array<int> m_StartChips;
    protected ref map<string, int> m_Have;
    protected bool m_Over;
    // Words: how it ended.
    protected string m_End;
    protected ref array<string> m_Hist;
    // The seats out of the game (left), by index; whose turn it is, and who
    // began.
    protected ref array<int> m_Gone;
    protected int m_Turn;
    protected int m_First;
    // The dice thrown since the session last asked (TakeThrows): the seat,
    // the die's place among its dice, the face.
    protected ref array<int> m_ThrewSeat;
    protected ref array<int> m_ThrewDie;
    protected ref array<int> m_ThrewFace;
    // The dice each seat threw last (by seat), for its plate (the canvas's "останній
    // кидок"): begun anew with a seat's first die of a move.
    protected ref map<int, ref array<int>> m_LastOf;
    // Beside m_LastOf: the place of each of those dice among the dice the seat throws.
    protected ref map<int, ref array<int>> m_LastAt;
    protected ref array<int> m_RolledThisMove;
    // The throws the game has seen (a seat's dice of one move are one throw) and the seat
    // that threw last: the windows play a throw when the number grows (OZG_GPublic.Throws).
    protected int m_ThrowNo;
    protected int m_ThrowSeat = -1;

    void OZG_DiceGame()
    {
        m_LastOf = new map<int, ref array<int>>();
        m_LastAt = new map<int, ref array<int>>();
        m_RolledThisMove = new array<int>();
        m_Seats = new array<int>();
        m_Cups = new OZG_DiceCups();
        m_Moves = new array<string>();
        m_Rolled = new array<int>();
        m_Replay = new array<int>();
        m_StartSeats = new array<int>();
        m_StartChips = new array<int>();
        m_Hist = new array<string>();
        m_Gone = new array<int>();
        m_ThrewSeat = new array<int>();
        m_ThrewDie = new array<int>();
        m_ThrewFace = new array<int>();
        m_Whose = OZG_DiceConst.WHOSE_HOST;
    }

    override string SetKind()
    {
        return OZG_Const.SET_DICE;
    }

    override int MinPlayers()
    {
        return 2;
    }

    override int MaxPlayers()
    {
        return 6;
    }

    override int DiceNeeded()
    {
        return 5;
    }

    // Perudo is played with own dice only.
    protected bool OwnOnly()
    {
        return false;
    }

    override string DiceMode(map<string, string> options)
    {
        if (OwnOnly())
            return OZG_DiceConst.WHOSE_OWN;
        return super.DiceMode(options);
    }

    override void CardsUsed(array<string> into)
    {
        into.Clear();
        into.Insert(OZG_DiceConst.DIE_FACE);
    }

    override void TakePieces(array<string> faces)
    {
        m_Have = new map<string, int>();
        for (int i = 0; i < faces.Count(); i++)
        {
            int had = 0;
            m_Have.Find(faces[i], had);
            m_Have.Set(faces[i], had + 1);
        }
    }

    override void TakeDice(int seat, array<int> faces, array<int> weights)
    {
        m_Cups.Set(seat, faces, weights);
    }

    override void OptionList(array<ref OZG_Option> into)
    {
        into.Insert(OZG_DiceOptions.Whose(OwnOnly()));
        DgOptions(into);
    }

    override void TakeStacks(array<int> seats, array<int> amounts)
    {
        m_StartSeats.Copy(seats);
        m_StartChips.Copy(amounts);
    }

    // ---- what a kind of game gives -----------------------------------------------

    protected void DgOptions(array<ref OZG_Option> into)
    {
    }

    protected void DgRead(map<string, string> options)
    {
    }

    // The game set out: the seats are in m_Seats, the stacks in m_Start*.
    protected bool DgBegin(out string refusal)
    {
        refusal = "";
        return true;
    }

    // A move checked and made; false with a refusal.
    protected bool DgAct(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        return false;
    }

    // ---- a game ------------------------------------------------------------------

    override bool Begin(array<int> seats, map<string, string> options, OZG_Random chance, string hint, out string refusal)
    {
        refusal = "";
        if (seats.Count() < MinPlayers() || seats.Count() > MaxPlayers())
        {
            refusal = OZG_Const.NO_PLAYERS;
            return false;
        }
        m_Chance = chance;
        m_Seats.Copy(seats);
        m_Hint = hint;
        m_Opts = OZG_BoardDuel.OptsText(options);
        string fallback = OZG_DiceConst.WHOSE_HOST;
        if (OwnOnly())
            fallback = OZG_DiceConst.WHOSE_OWN;
        m_Whose = OptionOf(options, OZG_DiceConst.OPT_WHOSE, fallback);
        if (OwnOnly())
            m_Whose = OZG_DiceConst.WHOSE_OWN;
        DgRead(options);
        // The host's dice: the can laid out must hold enough of them.
        if (m_Have && m_Whose != OZG_DiceConst.WHOSE_OWN)
        {
            int dice = 0;
            m_Have.Find(OZG_DiceConst.DIE_FACE, dice);
            if (dice < DiceNeeded())
            {
                refusal = OZG_DiceGameConst.SHORT_DICE;
                return false;
            }
        }
        m_Moves.Clear();
        m_Rolled.Clear();
        m_Hist.Clear();
        m_Gone.Clear();
        m_Over = false;
        m_End = "";
        // The seat after the one that began the last game begins this one.
        m_First = 0;
        int was = HintInt(hint, "first", -1);
        int at = m_Seats.Find(was);
        if (at >= 0)
            m_First = (at + 1) % m_Seats.Count();
        m_Turn = m_First;
        return DgBegin(refusal);
    }

    protected OZG_Random Chance()
    {
        if (!m_Chance)
            m_Chance = new OZG_Random();
        return m_Chance;
    }

    // The die `index` of the dice `seat` throws: as it falls, or as it fell
    // when the game is loaded.
    protected int DgRoll(int seat, int index)
    {
        int v;
        if (m_Replay.Count() > 0)
        {
            v = m_Replay[0];
            m_Replay.RemoveOrdered(0);
        }
        else
        {
            v = m_Cups.Of(seat, DiceNeeded()).Throw(index, Chance());
            // the dice rattle (boards and dice design §15): heard once a throw
            HearSound(OZG_SoundIds.ROLL);
            m_ThrewSeat.Insert(seat);
            m_ThrewDie.Insert(index);
            m_ThrewFace.Insert(v);
        }
        m_Rolled.Insert(v);
        if (m_RolledThisMove.Find(seat) < 0)
        {
            m_RolledThisMove.Insert(seat);
            m_ThrowNo++;
            m_ThrowSeat = seat;
            array<int> fresh = new array<int>();
            m_LastOf.Set(seat, fresh);
            array<int> freshAt = new array<int>();
            m_LastAt.Set(seat, freshAt);
        }
        array<int> last;
        if (m_LastOf.Find(seat, last) && last)
            last.Insert(v);
        array<int> lastAt;
        if (m_LastAt.Find(seat, lastAt) && lastAt)
            lastAt.Insert(index);
        return v;
    }

    protected void DgRollInto(int seat, int n, array<int> into)
    {
        into.Clear();
        for (int i = 0; i < n; i++)
            into.Insert(DgRoll(seat, i));
    }

    // The dice of `slots` (their places among the seat's dice) thrown, in that
    // order: a game that throws again only the dice left on the table.
    protected void DgRollSlots(int seat, array<int> slots, array<int> into)
    {
        into.Clear();
        for (int i = 0; i < slots.Count(); i++)
            into.Insert(DgRoll(seat, slots[i]));
    }

    override bool Act(int seat, string move, out string refusal)
    {
        refusal = OZG_Const.NO_MOVE;
        if (m_Over || m_Seats.Find(seat) < 0)
            return false;
        m_RolledThisMove.Clear();
        if (!DgAct(seat, move, refusal))
            return false;
        m_Moves.Insert(seat.ToString() + " " + move);
        refusal = "";
        return true;
    }

    protected int DgIndex(int seat)
    {
        return m_Seats.Find(seat);
    }

    override bool IsOver()
    {
        return m_Over;
    }

    override void TakeThrows(array<int> seats, array<int> dice, array<int> faces)
    {
        for (int i = 0; i < m_ThrewSeat.Count(); i++)
        {
            seats.Insert(m_ThrewSeat[i]);
            dice.Insert(m_ThrewDie[i]);
            faces.Insert(m_ThrewFace[i]);
        }
        m_ThrewSeat.Clear();
        m_ThrewDie.Clear();
        m_ThrewFace.Clear();
    }

    // A throw is the seat's next move: its turn, and nothing else to do first.
    override bool ThrowsNext(int seat)
    {
        if (m_Over || seat != DgCurrent())
            return false;
        array<string> can = new array<string>();
        Moves(seat, can);
        return can.Find(OZG_DiceGameConst.M_THROW) >= 0;
    }

    override string Outcome()
    {
        if (m_Seats.Count() == 0)
            return "";
        return "first=" + m_Seats[m_First].ToString();
    }

    // The next index after `i` still in the game.
    protected int DgNextIndex(int i)
    {
        int n = m_Seats.Count();
        for (int k = 1; k <= n; k++)
        {
            int j = (i + k) % n;
            if (m_Gone.Find(j) < 0)
                return j;
        }
        return i;
    }

    protected int DgLeft()
    {
        return m_Seats.Count() - m_Gone.Count();
    }

    protected int DgCurrent()
    {
        if (m_Over || m_Turn < 0 || m_Turn >= m_Seats.Count())
            return -1;
        return m_Seats[m_Turn];
    }

    override bool Waits(int seat)
    {
        return !m_Over && seat == DgCurrent();
    }

    // A throw is never a move one may pass over (design §6.3): the table waits
    // for an absent player, or the host lets him go. So nothing is Skippable.

    // The turn passed on with nothing done (a seat gone); a kind ends what its
    // turn had going.
    protected void DgPassTurn()
    {
        m_Turn = DgNextIndex(m_Turn);
    }

    override void DropSeat(int seat)
    {
        int i = m_Seats.Find(seat);
        if (i < 0 || m_Gone.Find(i) >= 0 || m_Over)
            return;
        m_Moves.Insert(seat.ToString() + " " + OZG_DiceGameConst.M_DROP);
        bool current = i == m_Turn;
        m_Gone.Insert(i);
        DgGone(i);
        if (m_Over)
            return;
        if (DgLeft() <= 1)
        {
            int last = DgNextIndex(i);
            DgWonBy(last, "STR_OZG_DICE_END_LEFT");
            return;
        }
        if (current)
            DgPassTurn();
    }

    // A kind's own bookkeeping of a seat gone.
    protected void DgGone(int index)
    {
    }

    // The game won by the seat of index `i`.
    protected void DgWonBy(int i, string reasonKey)
    {
        DgFinish(OZG_Words.W2("STR_OZG_DICE_END_WON", OZG_Words.SeatArg(m_Seats[i]), OZG_Words.KeyArg(reasonKey)));
    }

    protected void DgFinish(string words)
    {
        m_Over = true;
        m_End = words;
        HearSound(OZG_SoundIds.OVER);
    }

    // ---- kept and loaded -----------------------------------------------------------

    static string Dash(string s)
    {
        if (s == "")
            return "-";
        return s;
    }

    static string Undash(string s)
    {
        if (s == "-")
            return "";
        return s;
    }

    static string Ints(array<int> values, string sep)
    {
        string s = "";
        for (int i = 0; i < values.Count(); i++)
        {
            if (i > 0)
                s = s + sep;
            s = s + values[i].ToString();
        }
        return s;
    }

    static void IntsOf(string text, string sep, array<int> into)
    {
        into.Clear();
        array<string> parts = new array<string>();
        OZG_Text.SplitLong(text, sep, parts);
        for (int i = 0; i < parts.Count(); i++)
        {
            if (parts[i] != "")
                into.Insert(parts[i].ToInt());
        }
    }

    override string SaveState()
    {
        string s = "g1;" + Dash(m_Opts);
        s = s + ";" + Dash(Ints(m_Seats, ","));
        // The hint's pairs are joined with ';', the save's fields too.
        string hint = m_Hint;
        hint.Replace(";", "&");
        s = s + ";" + Dash(hint);
        s = s + ";" + Dash(Ints(m_StartSeats, ","));
        s = s + ";" + Dash(Ints(m_StartChips, ","));
        s = s + ";" + Dash(Ints(m_Rolled, " "));
        string moves = "";
        for (int i = 0; i < m_Moves.Count(); i++)
        {
            if (i > 0)
                moves = moves + "|";
            moves = moves + m_Moves[i];
        }
        s = s + ";" + Dash(moves);
        return s;
    }

    override bool LoadState(string data)
    {
        OZG_DiceGame again = OZG_DiceGame.Cast(Fresh());
        if (!again || !again.DgReplay(data))
            return false;
        // It loads: the same once more on this game -- heard by nobody (its end was
        // heard when it came).
        m_Have = null;
        bool loaded = DgReplay(data);
        array<int> unheard = new array<int>();
        TakeHeard(unheard);
        // and said by nobody: its lines are in the log already
        array<string> unsaid = new array<string>();
        TakeSaid(unsaid);
        return loaded;
    }

    // The game begun again and played again from the text.
    protected bool DgReplay(string data)
    {
        array<string> parts = new array<string>();
        OZG_Text.SplitLong(data, ";", parts);
        if (parts.Count() < 8 || parts[0] != "g1")
            return false;
        map<string, string> options = OZG_BoardDuel.OptsOf(Undash(parts[1]));
        array<int> seats = new array<int>();
        IntsOf(Undash(parts[2]), ",", seats);
        string hint = Undash(parts[3]);
        hint.Replace("&", ";");
        array<int> stackSeats = new array<int>();
        array<int> stackChips = new array<int>();
        IntsOf(Undash(parts[4]), ",", stackSeats);
        IntsOf(Undash(parts[5]), ",", stackChips);
        if (stackSeats.Count() > 0)
            TakeStacks(stackSeats, stackChips);
        // The dice and the moves can each pass 8191 bytes: read in place.
        m_Replay.Clear();
        array<string> rolled = new array<string>();
        OZG_Text.FieldPieces(data, ";", 6, " ", rolled);
        for (int r = 0; r < rolled.Count(); r++)
        {
            if (rolled[r] != "")
                m_Replay.Insert(rolled[r].ToInt());
        }
        string refusal;
        if (!Begin(seats, options, m_Chance, hint, refusal))
        {
            OZG_Log.Warn(GameId() + " load: the game does not begin again: " + refusal);
            return false;
        }
        array<string> moves = new array<string>();
        OZG_Text.FieldPieces(data, ";", 7, "|", moves);
        for (int i = 0; i < moves.Count(); i++)
        {
            string line = moves[i];
            if (line == "")
                continue;
            int space = line.IndexOf(" ");
            if (space <= 0)
            {
                OZG_Log.Warn(GameId() + " load: move " + i.ToString() + " has no seat: " + line);
                return false;
            }
            int seat = line.Substring(0, space).ToInt();
            string move = line.Substring(space + 1, line.Length() - space - 1);
            if (move == OZG_DiceGameConst.M_DROP)
            {
                DropSeat(seat);
                continue;
            }
            if (!Act(seat, move, refusal))
            {
                OZG_Log.Warn(GameId() + " load: move " + i.ToString() + " (" + line + ") refused: " + refusal + "; " + Everything());
                return false;
            }
        }
        bool clean = m_Replay.Count() == 0;
        if (!clean)
            OZG_Log.Warn(GameId() + " load: " + m_Replay.Count().ToString() + " dice left over of " + moves.Count().ToString() + " moves");
        m_Replay.Clear();
        return clean;
    }

    override string Brief()
    {
        string s = GameId() + " seats=" + Ints(m_Seats, ",");
        s = s + " moves=" + m_Moves.Count().ToString();
        if (m_Over)
            s = s + " over " + m_End;
        return s;
    }

    override string Everything()
    {
        return Brief() + " rolled=" + Ints(m_Rolled, " ");
    }

    override string BotMove(int seat, OZG_Random chance)
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

    // The common words of a throw for the log: the seat's name comes first.
    override void MoveWords(string move, array<string> into)
    {
        into.Clear();
        if (move == OZG_DiceGameConst.M_THROW)
        {
            into.Insert("STR_OZG_DICE_LOG_THROW");
            return;
        }
        into.Insert("STR_OZG_DICE_LOG_MOVE");
        into.Insert(move);
    }

    // The view's text, the throws' number and who threw last put into it.
    protected string DgPublicOut(OZG_GPublic v)
    {
        v.Throws = m_ThrowNo;
        v.ThrowSeat = m_ThrowSeat;
        return DgPublicText(v);
    }

    static string DgPublicText(OZG_GPublic v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_GPublic>.MakeData(v, text, err, false);
        return text;
    }

    static string DgPrivateText(OZG_GPrivate v)
    {
        string text;
        string err;
        JsonFileLoader<OZG_GPrivate>.MakeData(v, text, err, false);
        return text;
    }

    // A seat's plate: its role words and line words, and the dice it threw last as
    // faces (not where the dice are hidden: DgShowsLast).
    protected OZG_GSeat DgPlate(int seat, string role, string line)
    {
        OZG_GSeat g = new OZG_GSeat();
        g.SeatNo = seat;
        g.Role = role;
        g.Line = line;
        array<int> last;
        if (DgShowsLast() && m_LastOf.Find(seat, last) && last)
        {
            array<int> lastAt;
            m_LastAt.Find(seat, lastAt);
            for (int i = 0; i < last.Count(); i++)
            {
                int face = last[i];
                g.Faces.Insert(face.ToString());
                if (lastAt && i < lastAt.Count())
                {
                    int at = lastAt[i];
                    g.FacePlaces.Insert(at);
                }
            }
        }
        return g;
    }

    // Does everybody see the dice a seat threw last (perudo's stay under the cups).
    protected bool DgShowsLast()
    {
        return true;
    }

    protected OZG_GZone DgZone(string id, string label, array<int> dice)
    {
        OZG_GZone zone = new OZG_GZone();
        zone.Id = id;
        zone.Label = label;
        zone.Line = OZG_Words.W1("STR_OZG_DICE_SHOWN", OZG_DiceText.Line(dice));
        return zone;
    }
}
