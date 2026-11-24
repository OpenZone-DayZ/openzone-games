// stage: a table set out to be looked at in the window, not played -- how a
// full bout, a long hand or a long fan of backs looks (the owner's checks of
// 2026-11-25). The game's own state is read (SaveState), cards are put on
// the table and into hands, and the state is read back (LoadState): the
// game's soundness check still holds, so every card added is a further copy
// of a face (`6s.100`), written into the deal with the deck's look, and every
// card taken away goes to the discards. Fold the game after looking.
//
//   world_exec verb=oz_games args={"op":"stage","id":"<place>", ...}
//     hand=n     the seat `seat` (default 0) holds n cards
//     table=n    the bout is n cards: (n + 1) / 2 attack cards, n / 2 of them beaten
//     fan=n      the seat `fanseat` holds n cards (another seat's fan of backs)
//
// The rules' own limit on a bout (six attack cards) is not checked here: a
// bout of more cards than durak allows only shows how the window lays it.
class OZG_Stage
{
    protected static int s_Copy = 100;

    protected static const string RANKS = "6789TJQKA";
    protected static const string SUITS = "shdc";

    static bool Op(map<string, string> args, out string detail)
    {
        OZG_Session s = Find(args, detail);
        if (!s)
            return false;
        if (!s.m_Game)
        {
            detail = "stage: no game is being played there";
            return true;
        }
        OZG_DurakState st;
        string err;
        string saved = s.m_Game.SaveState();
        if (saved == "" || !JsonFileLoader<OZG_DurakState>.LoadData(saved, st, err) || !st)
        {
            detail = "stage: the game's state did not read " + err;
            return false;
        }
        int look = 0;
        if (st.Dealt && st.Dealt.Count() > 0)
            look = st.Dealt[0].Look;

        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        int hand = OZG_VerbOps.Arg(args, "hand", "-1").ToInt();
        int table = OZG_VerbOps.Arg(args, "table", "-1").ToInt();
        int fan = OZG_VerbOps.Arg(args, "fan", "-1").ToInt();
        int fanSeat = OZG_VerbOps.Arg(args, "fanseat", "-1").ToInt();

        if (table >= 0)
            SetTable(st, table, look);
        if (hand >= 0 && !SetHand(st, seat, hand, look))
        {
            detail = "stage: no seat " + seat.ToString() + " in the game";
            return true;
        }
        if (fan >= 0 && fanSeat >= 0 && !SetHand(st, fanSeat, fan, look))
        {
            detail = "stage: no seat " + fanSeat.ToString() + " in the game";
            return true;
        }
        if (st.AttackLimit < st.AttackCards.Count())
            st.AttackLimit = st.AttackCards.Count();

        string json;
        JsonFileLoader<OZG_DurakState>.MakeData(st, json, err, false);
        if (!s.m_Game.LoadState(json))
        {
            detail = "stage: the game refused the state (its soundness check)";
            return true;
        }
        s.Changed();
        detail = "staged; " + s.Describe();
        return true;
    }

    protected static OZG_Session Find(map<string, string> args, out string detail)
    {
        string id = OZG_VerbOps.Arg(args, "id", "");
        OZG_Session s;
        if (id == "" || id == "0")
            s = OZG_Sessions.At(0);
        else
            s = OZG_Sessions.ById(id);
        if (!s)
            detail = "no session '" + id + "'; " + OZG_Sessions.Describe();
        return s;
    }

    // A further copy of a face, written into the deal.
    protected static string Made(OZG_DurakState st, string face, int look)
    {
        s_Copy++;
        string name = OZG_Cards.CopyNamed(face, s_Copy);
        st.Dealt.Insert(OZG_DealCard.Make(name, look));
        return name;
    }

    // The k-th face of a plain run: ranks low to high, suits in turn.
    protected static string FaceNo(int k, int lowRanks)
    {
        int rank = (k / 4) % lowRanks;
        int suit = k % 4;
        return RANKS.Substring(rank, 1) + SUITS.Substring(suit, 1);
    }

    protected static bool SetHand(OZG_DurakState st, int seatNo, int n, int look)
    {
        OZG_DurakSeat seat = null;
        for (int i = 0; i < st.SeatList.Count(); i++)
        {
            if (st.SeatList[i].SeatNo == seatNo)
                seat = st.SeatList[i];
        }
        if (!seat)
            return false;
        int k = 0;
        while (seat.Held.Count() < n)
        {
            seat.Held.Insert(Made(st, FaceNo(k, 9), look));
            k++;
        }
        while (seat.Held.Count() > n)
        {
            int last = seat.Held.Count() - 1;
            st.DiscardCards.Insert(seat.Held[last]);
            seat.Held.RemoveOrdered(last);
        }
        return true;
    }

    // The bout: what lay there goes to the discards; then attack cards of the
    // low ranks, each but the last of an odd number beaten by a high card of
    // its suit.
    protected static void SetTable(OZG_DurakState st, int n, int look)
    {
        int i;
        for (i = 0; i < st.AttackCards.Count(); i++)
        {
            st.DiscardCards.Insert(st.AttackCards[i]);
            if (i < st.DefenceCards.Count() && st.DefenceCards[i] != "")
                st.DiscardCards.Insert(st.DefenceCards[i]);
        }
        st.AttackCards.Clear();
        st.DefenceCards.Clear();
        st.Taking = 0;
        int attacks = (n + 1) / 2;
        int beaten = n / 2;
        for (i = 0; i < attacks; i++)
        {
            string low = FaceNo(i, 3);
            st.AttackCards.Insert(Made(st, low, look));
            string high = "";
            if (i < beaten)
            {
                string suit = low.Substring(1, 1);
                int rank = 5 + (i / 4) % 4;
                high = Made(st, RANKS.Substring(rank, 1) + suit, look);
            }
            st.DefenceCards.Insert(high);
        }
    }
}
