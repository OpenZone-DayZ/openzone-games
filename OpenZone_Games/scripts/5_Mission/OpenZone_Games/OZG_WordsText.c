// Words of the server said outside a view (a notice): "key|arg|..", the way
// the window words a view (OZG_GameMenu.Words) when it is open; else the key
// and plain arguments, a seat's name unknown.
class OZG_WordsText
{
    static string Of(string words)
    {
        OZG_GameMenu menu = OZG_GameMenu.Current();
        if (menu)
            return menu.WordsOf(words);
        array<string> parts = new array<string>();
        OZG_Words.Parts(words, parts);
        if (parts.Count() == 0)
            return "";
        string s = Widget.TranslateString("#" + parts[0]);
        for (int i = 1; i < parts.Count(); i++)
        {
            string arg = parts[i];
            if (arg.IndexOf("k:") == 0)
                arg = Widget.TranslateString("#" + arg.Substring(2, arg.Length() - 2));
            else if (arg.IndexOf("s:") == 0)
                arg = "?";
            s.Replace("%" + i.ToString(), arg);
        }
        return s;
    }
}
