// Chance, handed to a game from outside (design §7). The server hands in this
// one; a stand check hands in one that knows its answers, so that the order of
// the deck can be fixed.
class OZG_Random
{
    // A whole number from 0 up to, not including, n.
    int Below(int n)
    {
        if (n <= 1)
            return 0;
        return Math.RandomInt(0, n);
    }

    // Fisher-Yates over Below().
    void Shuffle(array<string> cards)
    {
        for (int i = cards.Count() - 1; i > 0; i--)
        {
            int j = Below(i + 1);
            cards.SwapItems(i, j);
        }
    }
}
