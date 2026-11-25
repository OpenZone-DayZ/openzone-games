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

    // The mark (design §4.1): kept on the server with the item and written
    // to the world's storage, never synchronised to a client. Sign 0 is no
    // mark; 1..99 is the fault drawn into the back, its number picking the
    // kind and the place. The marker is the first reader and never gives way.
    protected int m_OZG_Sign;
    protected string m_OZG_By;
    protected ref array<string> m_OZG_Readers;
    protected ref array<string> m_OZG_Noticers;

    void OZG_Card_Base()
    {
        m_OZG_Style = "classic";
        m_OZG_Look = 0;
        m_OZG_Painted = -1;
        m_OZG_Readers = new array<string>();
        m_OZG_Noticers = new array<string>();
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

    bool OZG_Marked()
    {
        return m_OZG_Sign > 0;
    }

    int OZG_Sign()
    {
        return m_OZG_Sign;
    }

    string OZG_MarkedBy()
    {
        return m_OZG_By;
    }

    bool OZG_Reads(string uid)
    {
        if (uid == "" || m_OZG_Sign <= 0)
            return false;
        return m_OZG_Readers.Find(uid) >= 0;
    }

    // Server: the ids that read it, copied into `into`.
    void OZG_ReadersInto(array<string> into)
    {
        into.Clear();
        into.Copy(m_OZG_Readers);
    }

    // Server: `uid` marks the card. A clean card takes the sign, and `uid`
    // as its marker and first reader; a marked one keeps its sign and takes
    // `uid` as one more reader (design §4.1).
    void OZG_MarkBy(string uid, int sign)
    {
        if (uid == "" || sign <= 0)
            return;
        if (m_OZG_Sign <= 0)
        {
            m_OZG_Sign = sign;
            m_OZG_By = uid;
            m_OZG_Readers.Clear();
            m_OZG_Noticers.Clear();
        }
        OZG_AddReader(uid);
    }

    // Server: one more reader of a marked card. The list keeps at most
    // MarkKnowers ids; the oldest reader who is not the marker gives way.
    void OZG_AddReader(string uid)
    {
        if (uid == "" || m_OZG_Sign <= 0)
            return;
        if (m_OZG_Readers.Find(uid) >= 0)
            return;
        m_OZG_Readers.Insert(uid);
        int most = OZG_Cfg.Get().MarkKnowers;
        while (m_OZG_Readers.Count() > most)
        {
            int gone = 0;
            if (m_OZG_Readers[0] == m_OZG_By)
                gone = 1;
            if (gone >= m_OZG_Readers.Count())
                break;
            m_OZG_Readers.RemoveOrdered(gone);
        }
    }

    bool OZG_Notices(string uid)
    {
        if (uid == "" || m_OZG_Sign <= 0)
            return false;
        return m_OZG_Noticers.Find(uid) >= 0;
    }

    int OZG_NoticerCount()
    {
        return m_OZG_Noticers.Count();
    }

    // Server: one more noticer of a marked card (design §5). The list keeps
    // at most MarkKnowers ids; the oldest gives way (§4.1). A reader is not
    // made one: he sees more already.
    void OZG_AddNoticer(string uid)
    {
        if (uid == "" || m_OZG_Sign <= 0)
            return;
        if (m_OZG_Readers.Find(uid) >= 0)
            return;
        if (m_OZG_Noticers.Find(uid) >= 0)
            return;
        m_OZG_Noticers.Insert(uid);
        int most = OZG_Cfg.Get().MarkKnowers;
        while (m_OZG_Noticers.Count() > most)
            m_OZG_Noticers.RemoveOrdered(0);
    }

    // What a deal keeps of the mark.
    void OZG_MarkInto(OZG_DealCard rec)
    {
        if (!rec)
            return;
        rec.Sign = m_OZG_Sign;
        rec.Readers.Copy(m_OZG_Readers);
        rec.Noticers.Copy(m_OZG_Noticers);
    }

    protected void OZG_WriteIds(ParamsWriteContext ctx, array<string> ids)
    {
        int n = ids.Count();
        ctx.Write(n);
        for (int i = 0; i < n; i++)
            ctx.Write(ids[i]);
    }

    protected bool OZG_ReadIds(ParamsReadContext ctx, array<string> into)
    {
        into.Clear();
        int n;
        if (!ctx.Read(n))
            return false;
        if (n < 0 || n > 1000)
            return false;
        for (int i = 0; i < n; i++)
        {
            string one;
            if (!ctx.Read(one))
                return false;
            into.Insert(one);
        }
        return true;
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
        ctx.Write(m_OZG_Sign);
        ctx.Write(m_OZG_By);
        OZG_WriteIds(ctx, m_OZG_Readers);
        OZG_WriteIds(ctx, m_OZG_Noticers);
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
        // A card stored before marks: clean.
        if (v < 2)
            return true;
        int sign;
        if (!ctx.Read(sign))
            return false;
        string by;
        if (!ctx.Read(by))
            return false;
        m_OZG_Sign = sign;
        m_OZG_By = by;
        if (!OZG_ReadIds(ctx, m_OZG_Readers))
            return false;
        return OZG_ReadIds(ctx, m_OZG_Noticers);
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
    // of the card's back, and the card's mark.
    // Returns how many cards there are.
    static int CardsOf(OZG_Place place, array<ref OZG_DealCard> into)
    {
        array<EntityAI> things = new array<EntityAI>();
        ThingsOf(place, things);
        array<string> keys = new array<string>();
        RecordsOf(things, into, keys);
        return things.Count();
    }

    // Records of these card items, named as CardsOf names them, and beside
    // each record its item's key (OZG_Keys): what the dealer's stack names a
    // card by.
    static void RecordsOf(array<EntityAI> things, array<ref OZG_DealCard> into, array<string> keys)
    {
        into.Clear();
        keys.Clear();
        map<string, int> seen = new map<string, int>();
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            string face = card.OZG_Face();
            int had = 0;
            seen.Find(face, had);
            seen.Set(face, had + 1);
            string named = OZG_Cards.CopyNamed(face, had);
            if (named == "")
                continue;
            OZG_DealCard rec = OZG_DealCard.Make(named, card.OZG_Look());
            card.OZG_MarkInto(rec);
            into.Insert(rec);
            keys.Insert(OZG_Keys.Of(card));
        }
    }

    // ------------------------------------------------------------ the cards of a game in play
    //
    // A card of the game in play is found again by what the deal's record
    // keeps of it (card sharp's design §8): its face, the look of its back,
    // its mark and the mark's readers. Nothing in a place changes those while
    // it lies laid out (the place is shut; marking and the loupe need the card
    // out of any deck), and two items alike in all of them are the same card
    // to the game. The names a deal gave by the walk of the place shift when
    // a sleight takes a card out, so they are never used to find an item
    // after the deal's own frame.

    protected static bool Alike(OZG_Card_Base card, OZG_DealCard rec, bool readers)
    {
        if (!card || !rec)
            return false;
        if (card.OZG_Face() != OZG_Cards.FaceOf(rec.Name))
            return false;
        if (card.OZG_Look() != rec.Look || card.OZG_Sign() != rec.Sign)
            return false;
        if (!readers || rec.Sign <= 0)
            return true;
        array<string> theirs = new array<string>();
        card.OZG_ReadersInto(theirs);
        if (theirs.Count() != rec.Readers.Count())
            return false;
        for (int i = 0; i < theirs.Count(); i++)
        {
            if (rec.Readers.Find(theirs[i]) < 0)
                return false;
        }
        return true;
    }

    // Every record of the deal matched with an item of the place, each item
    // once: an item alike in everything first, then one alike but for its
    // readers. `things` comes back beside `deal`, null where no item matched.
    // Returns how many records found none.
    static int MatchDeal(OZG_Place place, array<ref OZG_DealCard> deal, array<OZG_Card_Base> things)
    {
        things.Clear();
        array<EntityAI> all = new array<EntityAI>();
        ThingsOf(place, all);
        array<OZG_Card_Base> free = new array<OZG_Card_Base>();
        int i;
        for (i = 0; i < all.Count(); i++)
        {
            OZG_Card_Base one = OZG_Card_Base.Cast(all[i]);
            if (one)
                free.Insert(one);
        }
        int missing = 0;
        for (i = 0; i < deal.Count(); i++)
        {
            OZG_Card_Base found = null;
            for (int pass = 0; pass < 2 && !found; pass++)
            {
                for (int k = 0; k < free.Count(); k++)
                {
                    if (Alike(free[k], deal[i], pass == 0))
                    {
                        found = free[k];
                        free.Remove(k);
                        break;
                    }
                }
            }
            if (!found)
                missing++;
            things.Insert(found);
        }
        return missing;
    }

    // The item a card of the game in play stands for; null when the place
    // holds none that matches.
    static OZG_Card_Base ThingFor(OZG_Place place, OZG_Game game, string name)
    {
        if (!place || !game || name == "")
            return null;
        array<ref OZG_DealCard> deal = new array<ref OZG_DealCard>();
        game.DealtRecords(deal);
        array<OZG_Card_Base> things = new array<OZG_Card_Base>();
        MatchDeal(place, deal, things);
        for (int i = 0; i < deal.Count(); i++)
        {
            if (deal[i].Name == name)
                return things[i];
        }
        return null;
    }

    // ------------------------------------------------------------ a player's pockets
    //
    // A card a player carries (card sharp's design §9): anywhere in his
    // inventory, his hands too, but not in a deck -- a deck is a box of its
    // own, and a deck in a backpack is not fifty-two cards in the pockets.

    static bool Pocketed(OZG_Card_Base card, PlayerBase player)
    {
        if (!card || !player)
            return false;
        if (card.GetHierarchyRootPlayer() != player)
            return false;
        EntityAI up = card.GetHierarchyParent();
        while (up && up != player)
        {
            if (up.IsInherited(OZG_Deck_Base) || up.IsInherited(OZG_Place))
                return false;
            up = up.GetHierarchyParent();
        }
        return true;
    }

    // The cards a player carries, in the order of his inventory.
    static void PocketCards(PlayerBase player, array<OZG_Card_Base> into)
    {
        into.Clear();
        if (!player)
            return;
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        EntityAI held = player.GetHumanInventory().GetEntityInHands();
        if (held && items.Find(held) < 0)
            items.Insert(held);
        for (int i = 0; i < items.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(items[i]);
            if (card && Pocketed(card, player))
                into.Insert(card);
        }
    }

    // The card of a player's pockets a key names; null when the key names no
    // card, or one he does not carry.
    static OZG_Card_Base PocketCard(PlayerBase player, string key)
    {
        OZG_Card_Base card = OZG_Card_Base.Cast(OZG_Keys.Find(key));
        if (!Pocketed(card, player))
            return null;
        return card;
    }

    // ------------------------------------------------------------ moving a card
    //
    // The synchronous server move (dayz-modding, items-and-processing.md):
    // the destination asked first, the item moved inside the call, and the
    // clients told. A player's own TakeToDst would be a juncture, late and
    // gate-bound; this moves at once, and the place is opened for it.

    protected static bool MoveTo(EntityAI item, InventoryLocation dst)
    {
        InventoryLocation src = new InventoryLocation;
        if (!item.GetInventory().GetCurrentInventoryLocation(src))
            return false;
        if (!GameInventory.LocationCanAddEntity(dst))
            return false;
        if (!GameInventory.LocationSyncMoveEntity(src, dst))
            return false;
        InventoryInputUserData.SendServerMove(null, InventoryCommandType.SYNC_MOVE, src, dst);
        return true;
    }

    // A card into the place (card sharp's design §6.6): a free slot of its
    // deck, else the place's own holding beside it. False when neither has
    // room.
    static bool IntoPlace(OZG_Place place, OZG_Card_Base card)
    {
        if (!place || !card)
            return false;
        place.OZG_Open();
        OZG_Deck_Base deck = place.OZG_Deck();
        InventoryLocation dst = new InventoryLocation;
        if (deck && deck.GetInventory().FindFreeLocationFor(card, FindInventoryLocationType.ATTACHMENT, dst))
        {
            if (MoveTo(card, dst))
                return true;
        }
        dst = new InventoryLocation;
        if (place.GetInventory().FindFreeLocationFor(card, FindInventoryLocationType.CARGO, dst))
        {
            if (MoveTo(card, dst))
                return true;
        }
        return false;
    }

    // A card to a player (§6.6): into his inventory, else onto the ground at
    // his feet. Returns "pocket", "ground" or "" when it did not move.
    static string ToPlayer(OZG_Card_Base card, PlayerBase player)
    {
        if (!card || !player)
            return "";
        EntityAI holder = card.GetHierarchyParent();
        OZG_Place place = OZG_Place.Cast(card.GetHierarchyRoot());
        if (place)
            place.OZG_Open();
        InventoryLocation dst = new InventoryLocation;
        if (player.GetInventory().FindFreeLocationFor(card, FindInventoryLocationType.ANY_CARGO, dst))
        {
            if (MoveTo(card, dst))
                return "pocket";
        }
        if (!holder)
            return "";
        vector m4[4];
        player.GetTransform(m4);
        if (holder.GetInventory().DropEntityWithTransform(InventoryMode.SERVER, holder, card, m4))
            return "ground";
        return "";
    }

    // The card item a deal's name stands for: the walk and the naming of
    // CardsOf, so a name read off this place finds its card again while the
    // place holds the same things (the session asks it in the frame of the
    // deal). Null when the place holds no such card.
    static OZG_Card_Base CardNamed(OZG_Place place, string name)
    {
        if (name == "")
            return null;
        array<EntityAI> things = new array<EntityAI>();
        ThingsOf(place, things);
        map<string, int> seen = new map<string, int>();
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            string face = card.OZG_Face();
            int had = 0;
            seen.Find(face, had);
            seen.Set(face, had + 1);
            if (OZG_Cards.CopyNamed(face, had) == name)
                return card;
        }
        return null;
    }
}
