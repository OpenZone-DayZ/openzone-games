// A card as a thing. Fifty-two config classes, one a face (the config names
// the face in ozgFace); the script is one. A card knows its face and the look
// of its back and nothing of any game.
class OZG_Card_Base extends ItemBase
{
    // The look of its back. The style's NAME is what the world's storage
    // keeps: a number would shift when a server adds a mod with a deck. The
    // number is what clients are told.
    protected string m_OZG_Style;
    protected int m_OZG_Look;
    protected int m_OZG_Painted;

    void OZG_Card_Base()
    {
        m_OZG_Style = "classic";
        m_OZG_Look = 0;
        m_OZG_Painted = -1;
        RegisterNetSyncVariableInt("m_OZG_Look", 0, 30);
    }

    string OZG_Face()
    {
        return ConfigGetString("ozgFace");
    }

    int OZG_Look()
    {
        return m_OZG_Look;
    }

    // Server: said once, by whoever makes the card.
    void OZG_SetStyle(string style)
    {
        m_OZG_Style = style;
        m_OZG_Look = OZG_Decks.LookOfStyle(style);
        SetSynchDirty();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Paint();
    }

    override void EEInit()
    {
        super.EEInit();
        OZG_Paint();
    }

    // A client: the thing wears the sheet of its look.
    protected void OZG_Paint()
    {
        if (!GetGame() || GetGame().IsDedicatedServer())
            return;
        if (m_OZG_Painted == m_OZG_Look)
            return;
        m_OZG_Painted = m_OZG_Look;
        string sheet = OZG_Decks.SheetAt(m_OZG_Look);
        if (sheet == "")
            return;
        int at = GetHiddenSelectionIndex("camo");
        if (at >= 0)
            SetObjectTexture(at, sheet);
    }

    override void OnStoreSave(ParamsWriteContext ctx)
    {
        super.OnStoreSave(ctx);
        ctx.Write(OZG_Const.CARD_SAVE);
        ctx.Write(m_OZG_Style);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        if (!super.OnStoreLoad(ctx, version))
            return false;
        int v;
        if (!ctx.Read(v))
            return false;
        string style;
        if (!ctx.Read(style))
            return false;
        m_OZG_Style = style;
        return true;
    }

    override void EEOnAfterLoad()
    {
        super.EEOnAfterLoad();
        if (!GetGame() || !GetGame().IsServer())
            return;
        m_OZG_Look = OZG_Decks.LookOfStyle(m_OZG_Style);
        SetSynchDirty();
    }
}
