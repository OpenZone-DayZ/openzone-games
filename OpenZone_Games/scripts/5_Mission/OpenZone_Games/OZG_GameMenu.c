// The window of a game. Phase 2 has only its doors: the table itself is drawn
// in its own phase.
class OZG_GameMenu : UIScriptedMenu
{
    protected static OZG_GameMenu s_Current;
    protected static string s_ForId;

    protected string m_PlaceId;

    static OZG_GameMenu Current()
    {
        return s_Current;
    }

    // The place the next window opens on.
    static void For(string placeId)
    {
        s_ForId = placeId;
    }

    string PlaceId()
    {
        return m_PlaceId;
    }

    void Redraw()
    {
    }

    void Said(string text)
    {
    }

    void CloseBecause(string why)
    {
        Close();
    }

    void PulseOf(OZG_Place place)
    {
    }

    void KeyPressed(int key)
    {
    }

    override Widget Init()
    {
        s_Current = this;
        m_PlaceId = s_ForId;
        layoutRoot = GetGame().GetWorkspace().CreateWidgets("OpenZone_Games/gui/layouts/ozg_aim.layout");
        return layoutRoot;
    }

    void ~OZG_GameMenu()
    {
        if (s_Current == this)
            s_Current = null;
    }
}
