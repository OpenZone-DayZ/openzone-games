// The card sharp's crafts (design §4.2, §4.3), recipes of the game's own
// crafting: the marking tool on a card, and (Task 4) the loupe on a card.
// Nothing is made; the card changes on the server. One of the two items is
// the one in the player's hands (recipebase.c:479-502) and the other
// anywhere the player carries it; neither may lie in a place, on the ground
// or in another player's things.
class OZG_Recipes
{
    protected static ref RecipeBase s_Mark;

    // The recipe as registered, for the stand's `craft`.
    static RecipeBase MarkRecipe()
    {
        return s_Mark;
    }

    static RecipeBase MakeMark()
    {
        s_Mark = new OZG_RecipeMark();
        return s_Mark;
    }

    // An item the base recipe takes: none with attachments (recipebase.c:
    // 551-561). Checked here, not through super.CanDo: an override that
    // handed its `ingredients` on to super found the array empty afterwards,
    // "Index out of range (0 >= 0)", and the virtual machine's exception
    // left its answer to chance (measured 2026-11-25). The game's own
    // recipes that override CanDo do not call super either.
    static bool NoAttachments(ItemBase item)
    {
        if (!item)
            return false;
        if (item.GetInventory() && item.GetInventory().AttachmentCount() > 0)
            return false;
        return true;
    }

    // Does `player` carry `item`, at any depth of the inventory?
    static bool Carried(EntityAI item, PlayerBase player)
    {
        if (!item || !player)
            return false;
        return item.GetHierarchyRootPlayer() == player;
    }

    // One use of the marking tool: 100 / MarkUses of its quantity. What is
    // left under half a use counts as none, so a tool lasts MarkUses uses
    // whatever their number, and the empty tool is deleted (its config's
    // varQuantityDestroyOnMin, inherited from Inventory_Base).
    static void Spend(ItemBase tool)
    {
        if (!tool)
            return;
        float use = 100.0 / OZG_Cfg.Get().MarkUses;
        float left = tool.GetQuantity() - use;
        if (left < use * 0.5)
            left = 0;
        tool.SetQuantity(left);
    }
}

// The marking tool and a card: the card marked by the player (its sign, if
// it had none, drawn by chance), one use of the tool spent, a line in the
// server's log (design §6.1: every marking, with the player's id).
class OZG_RecipeMark extends RecipeBase
{
    override void Init()
    {
        m_Name = "#STR_OZG_MARKTOOL_ACT";
        m_IsInstaRecipe = false;
        // Not read: GetLengthInSecs below gives the setting's length.
        m_AnimationLength = 2;
        m_Specialty = 0;

        // -1 is no check (recipebase.c:429-476): a tool that is not ruined,
        // any card.
        m_MinDamageIngredient[0] = -1;
        m_MaxDamageIngredient[0] = 3;
        m_MinQuantityIngredient[0] = -1;
        m_MaxQuantityIngredient[0] = -1;
        m_MinDamageIngredient[1] = -1;
        m_MaxDamageIngredient[1] = -1;
        m_MinQuantityIngredient[1] = -1;
        m_MaxQuantityIngredient[1] = -1;

        InsertIngredient(0, "OZG_MarkTool");
        m_IngredientAddHealth[0] = 0;
        m_IngredientSetHealth[0] = -1;
        m_IngredientAddQuantity[0] = 0;
        m_IngredientDestroy[0] = false;
        m_IngredientUseSoftSkills[0] = false;

        InsertIngredient(1, "OZG_Card_Base");
        m_IngredientAddHealth[1] = 0;
        m_IngredientSetHealth[1] = -1;
        m_IngredientAddQuantity[1] = 0;
        m_IngredientDestroy[1] = false;
        m_IngredientUseSoftSkills[1] = false;
        // Nothing is made: no AddResult.
    }

    // The gesture's length: the setting, a client's from the sync.
    override float GetLengthInSecs()
    {
        return OZG_Cfg.Get().MarkSeconds2;
    }

    override bool CanDo(ItemBase ingredients[], PlayerBase player)
    {
        ItemBase tool = ingredients[0];
        ItemBase card = ingredients[1];
        if (!OZG_Recipes.NoAttachments(tool) || !OZG_Recipes.NoAttachments(card))
            return false;
        if (OZG_Cfg.Get().Cheating != 1)
            return false;
        if (!OZG_Recipes.Carried(tool, player))
            return false;
        if (!OZG_Recipes.Carried(card, player))
            return false;
        return tool.GetQuantity() > 0;
    }

    override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
    {
        if (!GetGame().IsServer())
            return;
        OZG_Card_Base card = OZG_Card_Base.Cast(ingredients[1]);
        string uid = OZG_Who.Uid(player);
        if (!card || uid == "")
            return;
        int sign = card.OZG_Sign();
        if (sign <= 0)
        {
            OZG_Random dice = new OZG_Random();
            sign = 1 + dice.Below(99);
        }
        card.OZG_MarkBy(uid, sign);
        OZG_Recipes.Spend(ingredients[0]);
        OZG_Log.Info("mark: " + uid + " marked " + card.OZG_Face() + ", sign " + sign.ToString());
    }
}

modded class PluginRecipesManagerBase
{
    override void RegisterRecipies()
    {
        super.RegisterRecipies();
        RegisterRecipe(OZG_Recipes.MakeMark());
    }
}
