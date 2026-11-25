// The paper of a game played for stakes (stakes-and-games design §3): when the
// game is folded or the deck gathered and somebody brought chips in, how much
// each player brought in all told and how much he had at the end. The game's
// own Paper item, its note's text set by the server: it is saved with the
// item, and reading it sends the text to the reader (ActionReadPaperCB).
//
// THE NOTE KEEPS 512 BYTES (MiscGameplayFunctions.SanitizeString cuts by bytes,
// so a cut can land inside a Ukrainian letter): the text is kept under 500.
class OZG_StakePaper
{
    static const int MOST_BYTES = 500;

    // The server writes the paper once for whoever reads it: its few words
    // stand in both languages.
    static string TitleOf(string gameId)
    {
        if (gameId == OZG_PokerConst.GAME_ID)
            return "Texas hold'em / Техаський холдем";
        if (gameId == OZG_OchkoConst.GAME_ID)
            return "Ochko / Очко";
        // The book of a place outlives a change of game: a paper made after
        // hold'em gave way to preferans was headed with preferans' bare id
        // (seen 2026-11-25).
        if (gameId == OZG_PrefConst.GAME_ID)
            return "Preferans / Преферанс";
        if (gameId == OZG_BridgeConst.GAME_ID)
            return "Bridge / Бридж";
        if (gameId == OZG_DurakConst.GAME_ID)
            return "Durak / Дурень";
        return gameId;
    }

    protected static string Two(int n)
    {
        if (n < 10)
            return "0" + n.ToString();
        return n.ToString();
    }

    // The text; names cut to `nameChars` characters when above 0.
    static string Text(OZG_SessionRec r, int nameChars, bool legend)
    {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        GetGame().GetWorld().GetDate(year, month, day, hour, minute);
        string text = "OpenZone Games: " + TitleOf(r.GameId) + "\n";
        string when = Two(day) + "." + Two(month) + "." + year.ToString();
        when = when + " " + Two(hour) + ":" + Two(minute);
        text = text + when + "\n";
        if (legend)
            text = text + "in -> end / занесено -> наприкінці\n";
        for (int i = 0; i < r.StakeUids.Count(); i++)
        {
            int brought = r.StakeIn[i];
            int chips = r.StakeChips[i];
            if (brought <= 0 && chips <= 0)
                continue;
            string name = r.StakeNames[i];
            if (name == "")
                name = "?";
            if (nameChars > 0 && name.LengthUtf8() > nameChars)
                name = name.SubstringUtf8(0, nameChars);
            int diff = chips - brought;
            string sign = "";
            if (diff > 0)
                sign = "+";
            string line = name + ": " + brought.ToString() + " -> " + chips.ToString();
            line = line + " (" + sign + diff.ToString() + ")";
            text = text + line + "\n";
        }
        return text;
    }

    // Into the hands or the inventory of whoever folded or gathered, else
    // onto the ground at the place. Nothing when nobody brought chips in.
    static void Make(OZG_SessionRec r, PlayerBase by, vector at)
    {
        if (!r || !r.StakeUids)
            return;
        bool any = false;
        for (int i = 0; i < r.StakeIn.Count(); i++)
        {
            if (r.StakeIn[i] > 0)
                any = true;
        }
        if (!any)
            return;
        string text = Text(r, 0, true);
        if (text.Length() > MOST_BYTES)
            text = Text(r, 12, true);
        if (text.Length() > MOST_BYTES)
            text = Text(r, 8, false);

        EntityAI made = null;
        if (by)
            made = by.GetInventory().CreateInInventory("Paper");
        if (!made)
            made = EntityAI.Cast(GetGame().CreateObjectEx("Paper", at, ECE_PLACE_ON_SURFACE));
        ItemBase paper = ItemBase.Cast(made);
        if (!paper)
        {
            OZG_Log.Warn("place " + r.PlaceId + ": the paper of the game was not made");
            return;
        }
        WrittenNoteData note = paper.GetWrittenNoteData();
        if (note)
            note.SetNoteText(text);
        string flat = text;
        flat.Replace("\n", " | ");
        OZG_Log.Info("place " + r.PlaceId + " paper: " + flat);
    }
}
