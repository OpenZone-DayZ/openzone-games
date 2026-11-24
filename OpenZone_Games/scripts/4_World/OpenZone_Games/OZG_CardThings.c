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
        OZG_Paint();
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

    // The card wears its own face and the back of its look, one picture on
    // each side. Done on the server as well as on each client: the game's own
    // food changes its pictures on the server, and that is what the inventory
    // shows; the config's pictures are those of the first look.
    protected void OZG_Paint()
    {
        if (!GetGame())
            return;
        if (m_OZG_Painted == m_OZG_Look)
            return;
        m_OZG_Painted = m_OZG_Look;
        string face = OZG_Decks.CardFaceAt(m_OZG_Look, OZG_Face());
        string back = OZG_Decks.CardBackAt(m_OZG_Look);
        int at = GetHiddenSelectionIndex("face");
        if (at >= 0 && face != "")
            SetObjectTexture(at, face);
        at = GetHiddenSelectionIndex("back");
        if (at >= 0 && back != "")
            SetObjectTexture(at, back);
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
        OZG_Paint();
    }
}

// What a place holds, read as cards: the deck's own, in the order of its
// slots, then those the place keeps beside it.
class OZG_DeckRead
{
    // Every card item of a place.
    static void ThingsOf(OZG_Place place, array<EntityAI> into)
    {
        into.Clear();
        if (!place)
            return;
        OZG_Deck_Base deck = place.OZG_Deck();
        int i;
        if (deck)
        {
            int held = deck.GetInventory().AttachmentCount();
            for (i = 0; i < held; i++)
            {
                EntityAI one = deck.GetInventory().GetAttachmentFromIndex(i);
                if (one && one.IsInherited(OZG_Card_Base))
                    into.Insert(one);
            }
        }
        CargoBase cargo = place.GetInventory().GetCargo();
        if (cargo)
        {
            for (i = 0; i < cargo.GetItemCount(); i++)
            {
                EntityAI beside = cargo.GetItem(i);
                if (beside && beside.IsInherited(OZG_Card_Base))
                    into.Insert(beside);
            }
        }
    }

    // The cards as a game is handed them: a record a card. The first card of
    // a face found is named by the face, every further one by the face and
    // its number (OZG_Cards.CopyNamed), however many there are: a deck of
    // aces alone is as many names as it has cards. Beside the name, the look
    // of the card's back; the next phase reads the card's mark here too.
    // Returns how many cards there are.
    static int CardsOf(OZG_Place place, array<ref OZG_DealCard> into)
    {
        into.Clear();
        array<EntityAI> things = new array<EntityAI>();
        ThingsOf(place, things);
        map<string, int> seen = new map<string, int>();
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            string face = card.OZG_Face();
            int had = 0;
            seen.Find(face, had);
            seen.Set(face, had + 1);
            string named = OZG_Cards.CopyNamed(face, had);
            if (named == "")
                continue;
            into.Insert(OZG_DealCard.Make(named, card.OZG_Look()));
        }
        return things.Count();
    }
}
