// Dice (boards and dice design §3.2, §8, §10): the dice a seat throws and how a
// throw comes out. The session reads which dice each seat throws -- the
// host's, the seat's own can, by wish -- and hands the game their loads
// (TakeDice); the game throws with chance handed in. A die with no load comes
// up any face alike; a loaded one shows its face as often as it was loaded
// for, the other five alike otherwise.
class OZG_DiceConst
{
    static const string DIE_FACE = "Di";
    // Whose dice (§8): an option of every dice game and of nardy.
    static const string OPT_WHOSE = "dice";
    static const string WHOSE_HOST = "host";
    static const string WHOSE_OWN = "own";
    static const string WHOSE_WISH = "wish";
    static const string NO_DICE = "STR_OZG_DICE_NONE";
}

class OZG_DiceCup
{
    // Beside each die: its loaded face (0: none) and how often it comes up, in
    // percent.
    ref array<int> LoadFace;
    ref array<int> LoadWeight;

    void OZG_DiceCup()
    {
        LoadFace = new array<int>();
        LoadWeight = new array<int>();
    }

    static OZG_DiceCup Plain(int count)
    {
        OZG_DiceCup cup = new OZG_DiceCup();
        for (int i = 0; i < count; i++)
        {
            cup.LoadFace.Insert(0);
            cup.LoadWeight.Insert(0);
        }
        return cup;
    }

    int Count()
    {
        return LoadFace.Count();
    }

    // The die `i` thrown; a cup short of dice throws plain ones.
    int Throw(int i, OZG_Random chance)
    {
        int face = 0;
        int weight = 0;
        if (i >= 0 && i < LoadFace.Count())
        {
            face = LoadFace[i];
            weight = LoadWeight[i];
        }
        if (face < 1 || face > 6 || weight <= 0)
            return chance.Below(6) + 1;
        if (chance.Below(100) < weight)
            return face;
        // One of the other five.
        int other = chance.Below(5) + 1;
        if (other >= face)
            other++;
        return other;
    }

    // `n` dice thrown, the first `n` of the cup.
    void ThrowInto(int n, OZG_Random chance, array<int> into)
    {
        into.Clear();
        for (int i = 0; i < n; i++)
            into.Insert(Throw(i, chance));
    }
}

// The cups of a game's seats, as the session hands them.
class OZG_DiceCups
{
    protected ref map<int, ref OZG_DiceCup> m_Cups;

    void OZG_DiceCups()
    {
        m_Cups = new map<int, ref OZG_DiceCup>();
    }

    void Set(int seat, array<int> faces, array<int> weights)
    {
        OZG_DiceCup cup = new OZG_DiceCup();
        for (int i = 0; i < faces.Count(); i++)
        {
            cup.LoadFace.Insert(faces[i]);
            int w = 0;
            if (i < weights.Count())
                w = weights[i];
            cup.LoadWeight.Insert(w);
        }
        m_Cups.Set(seat, cup);
    }

    OZG_DiceCup Of(int seat, int count)
    {
        OZG_DiceCup cup;
        if (m_Cups.Find(seat, cup) && cup)
            return cup;
        return OZG_DiceCup.Plain(count);
    }
}

// Dice as text: "5 3", and the words of a throw.
class OZG_DiceText
{
    static string Line(array<int> dice)
    {
        string s = "";
        for (int i = 0; i < dice.Count(); i++)
        {
            if (i > 0)
                s = s + " ";
            s = s + dice[i].ToString();
        }
        return s;
    }

    static void Parse(string text, array<int> into)
    {
        into.Clear();
        array<string> parts = new array<string>();
        text.Split(" ", parts);
        for (int i = 0; i < parts.Count(); i++)
        {
            int v = parts[i].ToInt();
            if (v >= 1 && v <= 6)
                into.Insert(v);
        }
    }
}
