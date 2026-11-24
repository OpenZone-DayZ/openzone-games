// The fan of cards in a player's hands, which everybody near sees.
//
// The server puts it into the hands of a present seated player who has cards
// and whose hands are empty (OZG_Session.Fans). It shows up to eight cards, as
// many as he holds, backs outwards, in the back of the deck the game was laid
// out from.
//
// It is never in the way: it can be dropped or swapped like any item, and
// whenever it leaves those hands, for any reason, it deletes itself a moment
// later. It comes back when the hands are empty again. Its class is of scope
// 1, so the world's storage never keeps one.
class OZG_Hand extends ItemBase
{
    static const int CARDS = 8;

    protected int m_OZG_Cards;
    protected int m_OZG_DeckNo;

    // Client: what is drawn now.
    protected int m_OZG_ShownCards;
    protected int m_OZG_ShownDeck;

    void OZG_Hand()
    {
        m_OZG_DeckNo = -1;
        m_OZG_ShownCards = -1;
        m_OZG_ShownDeck = -2;
        RegisterNetSyncVariableInt("m_OZG_Cards", 0, CARDS);
        RegisterNetSyncVariableInt("m_OZG_DeckNo", -1, 126);
    }

    // Server: how many cards the fan shows, and whose deck's back.
    void OZG_Set(int cards, int deckNo)
    {
        if (cards < 0)
            cards = 0;
        if (cards > CARDS)
            cards = CARDS;
        if (cards == m_OZG_Cards && deckNo == m_OZG_DeckNo)
            return;
        m_OZG_Cards = cards;
        m_OZG_DeckNo = deckNo;
        SetSynchDirty();
    }

    override void EEInit()
    {
        super.EEInit();
        OZG_Show();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Show();
    }

    // The cards a player does not hold are blanked: a run in the middle of
    // the fan stays, so three cards are the three middle ones.
    protected void OZG_Show()
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (m_OZG_ShownCards == m_OZG_Cards && m_OZG_ShownDeck == m_OZG_DeckNo)
            return;
        // Until the server has said how many, the fan is as its config made
        // it.
        if (m_OZG_Cards <= 0)
            return;
        m_OZG_ShownCards = m_OZG_Cards;
        m_OZG_ShownDeck = m_OZG_DeckNo;

        string sheet = OZG_Decks.SheetOf(OZG_Decks.ClassOf(m_OZG_DeckNo));
        TStringArray own = GetHiddenSelectionsTextures();
        TStringArray mats = GetHiddenSelectionsMaterials();
        int first = (CARDS - m_OZG_Cards) / 2;
        int shown = 0;
        for (int i = 0; i < CARDS; i++)
        {
            int number = i + 1;
            int at = GetHiddenSelectionIndex("card" + number.ToString());
            if (at < 0)
                continue;
            bool on = i >= first && i < first + m_OZG_Cards;
            if (!on)
            {
                SetObjectTexture(at, "");
                continue;
            }
            string texture = sheet;
            if (texture == "" && own && at < own.Count())
                texture = own[at];
            SetObjectTexture(at, texture);
            if (mats && at < mats.Count())
                SetObjectMaterial(at, mats[at]);
            shown++;
        }
        OZG_Log.Dbg("fan: " + shown.ToString() + " of " + m_OZG_Cards.ToString() + " cards shown, sheet '" + sheet + "'");
    }

    // Out of the hands, for any reason: gone a moment later.
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        super.EEItemLocationChanged(oldLoc, newLoc);
        if (!GetGame() || !GetGame().IsServer())
            return;
        if (newLoc.GetType() != InventoryLocationType.HANDS)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_Vanish, 100, false);
    }

    void OZG_Vanish()
    {
        InventoryLocation now = new InventoryLocation();
        if (GetInventory() && GetInventory().GetCurrentInventoryLocation(now))
        {
            if (now.GetType() == InventoryLocationType.HANDS)
                return;
        }
        GetGame().ObjectDelete(this);
    }

    // Nothing an item's own actions should offer on it.
    override void SetActions()
    {
    }

    override bool CanPutInCargo(EntityAI parent)
    {
        return false;
    }

    override bool CanPutAsAttachment(EntityAI parent)
    {
        return false;
    }

    override bool IsInventoryVisible()
    {
        return false;
    }
}

// How the mod's hand-held things are held: with the game's own poses, no
// animation is added. The fan as a sheet of paper is held; the deck as the
// matchbox, which stands in its model as the deck does; one card as the
// punched card, held by its end (models/README.md).
modded class ModItemRegisterCallbacks
{
    override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
    {
        super.RegisterOneHanded(pType, pBehavior);
        pType.AddItemInHandsProfileIK("OZG_Hand", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/paper.anm");
        pType.AddItemInHandsProfileIK("OZG_Deck_Base", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/Matchbox.anm");
        pType.AddItemInHandsProfileIK("OZG_Card_Base", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/gear/PunchedCard.anm");
    }
}
