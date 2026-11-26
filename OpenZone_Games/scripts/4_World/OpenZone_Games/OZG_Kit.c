// The card sharp's kit's window (boards and dice design §10, the canvas's board
// "Шулерський набір"): "Open the kit" on the kit in the hands; the server tells the player
// what he carries -- his cards and dice, in his pockets, in the deck in his hands, in a
// can -- and what he knows of each (his own marks, the marks he reads, the loads he
// knows); a card picked is marked (its sign falls by chance), a die picked is loaded on a
// face and a weight, one use of the kit each; another's mark or load, picked, opens to
// him and spends nothing.

// The kit, so that it has its action.
class OZG_MarkTool extends Inventory_Base
{
    override void SetActions()
    {
        super.SetActions();
        AddAction(OZG_ActionOpenKit);
    }
}

// "Open the kit": the server sends the window its things; the client opens it on them.
class OZG_ActionOpenKit : ActionSingleUseBase
{
    void OZG_ActionOpenKit()
    {
        m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_ITEM_ON;
        m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
        m_Text = "#STR_OZG_KIT_OPEN";
    }

    override void CreateConditionComponents()
    {
        m_ConditionItem = new CCINonRuined();
        m_ConditionTarget = new CCTNone();
    }

    override bool HasTarget()
    {
        return false;
    }

    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        return item && item.IsKindOf("OZG_MarkTool");
    }

    override void OnExecuteServer(ActionData action_data)
    {
        if (action_data && action_data.m_Player)
            OZG_KitService.Push(action_data.m_Player, "");
    }
}

class OZG_KitService
{
    // The service's kit ops (OZG_Service.Handle): a thing looked at, a card marked, a die
    // loaded. ask.CardKey: the thing; ask.SeatNo: the face; ask.PoseNo: the weight (1 .. 3).
    // The answer is the window's things anew, with what was done in words.
    static string Handle(string op, string json, PlayerBase player, out bool ok, out string error)
    {
        ok = false;
        error = OZG_Const.SAY_CANNOT;
        if (!player)
            return "";
        OZG_WAsk ask = new OZG_WAsk();
        if (json != "")
        {
            string bad;
            if (!JsonFileLoader<OZG_WAsk>.LoadData(json, ask, bad) || !ask)
                return "";
        }
        string said = "";
        string why = "";
        if (op == OZG_Const.OP_KIT_LOOK)
            why = Look(player, ask.CardKey, said);
        else if (op == OZG_Const.OP_KIT_MARK)
            why = Mark(player, ask.CardKey, said);
        else if (op == OZG_Const.OP_KIT_LOAD)
            why = Load(player, ask.CardKey, ask.SeatNo, ask.PoseNo, said);
        if (why != "")
        {
            error = why;
            return "";
        }
        ok = true;
        error = "";
        return View(player, said);
    }

    // The window's things, sent unasked (the action), with `said` the words of what was
    // just done.
    static void Push(PlayerBase player, string said)
    {
        if (!player)
            return;
        PlayerIdentity id = player.GetIdentity();
        if (!id)
            return;
        OZ_Rpc.ServiceRespond(id, OZG_Const.SERVICE, OZG_Const.OP_KIT_VIEW, true, View(player, said), "");
    }

    static string View(PlayerBase player, string said)
    {
        OZG_WKit kit = new OZG_WKit();
        kit.Said = said;
        string uid = OZG_Who.Uid(player);
        kit.Uses = UsesLeft(player);
        array<EntityAI> items = Carried(player);
        OZG_Settings cfg = OZG_Cfg.Get();
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI thing = items[i];
            if (!thing)
                continue;
            OZG_Card_Base card = OZG_Card_Base.Cast(thing);
            OZG_Die_Base die = OZG_Die_Base.Cast(thing);
            if (!card && !die)
                continue;
            OZG_WKitThing t = new OZG_WKitThing();
            t.Key = OZG_Keys.Of(thing);
            t.Where = WhereOf(thing);
            if (card)
            {
                t.Kind = "card";
                t.Face = OZG_Cards.FaceOf(card.OZG_Face());
                t.Look = card.OZG_StyleName();
                if (card.OZG_Marked() && card.OZG_MarkedBy() == uid)
                    t.Mark = 1;
                else if (card.OZG_Marked() && card.OZG_Reads(uid))
                    t.Mark = 2;
                if (t.Mark > 0)
                    t.Sign = card.OZG_Sign();
            }
            else
            {
                t.Kind = "die";
                t.Look = OZG_BoardRead.DieLookOf(die.GetType());
                if (die.OZG_Loaded() && die.OZG_Knows(uid))
                {
                    t.LoadFace = die.OZG_LoadFace();
                    t.LoadStep = cfg.LoadStep(die.OZG_LoadWeight());
                    // 1: by his hand, 2: another's he knows
                    t.Mark = 2;
                    if (die.OZG_LoadBy() == uid)
                        t.Mark = 1;
                }
            }
            kit.Things.Insert(t);
        }
        string text;
        string err;
        JsonFileLoader<OZG_WKit>.MakeData(kit, text, err, false);
        return text;
    }

    // Everything in the hands and the pockets, the hands' thing too.
    static array<EntityAI> Carried(PlayerBase player)
    {
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        EntityAI held = player.GetHumanInventory().GetEntityInHands();
        if (held && items.Find(held) < 0)
            items.Insert(held);
        return items;
    }

    // The uses left in the kit he carries (its quantity counts percent, OZG_Recipes.Spend):
    // -1 when he carries none.
    static int UsesLeft(PlayerBase player)
    {
        ItemBase tool = OZG_DiceCheat.Kit(player);
        if (!tool)
        {
            if (KitHeld(player))
                return 0;
            return -1;
        }
        int most = OZG_Cfg.Get().MarkUses;
        if (most < 1)
            most = 1;
        float per = 100.0 / most;
        int left = Math.Round(tool.GetQuantity() / per);
        if (left < 1)
            left = 1;
        return left;
    }

    // A kit in the hands or the pockets, used up or not.
    static bool KitHeld(PlayerBase player)
    {
        array<EntityAI> items = Carried(player);
        for (int i = 0; i < items.Count(); i++)
        {
            if (items[i] && items[i].IsKindOf("OZG_MarkTool"))
                return true;
        }
        return false;
    }

    // Where a thing is carried: in a deck, in a can of dice, else in the pockets.
    static string WhereOf(EntityAI thing)
    {
        EntityAI parent = thing.GetHierarchyParent();
        while (parent)
        {
            if (parent.IsInherited(OZG_DiceCan_Base))
                return "can";
            if (parent.IsInherited(OZG_Deck_Base))
                return "deck";
            parent = parent.GetHierarchyParent();
        }
        return "pocket";
    }

    // A card's name in the stringtable (STR_OZG_CARD_QH), as a word of the said.
    static string CardNameArg(OZG_Card_Base card)
    {
        string face = OZG_Cards.FaceOf(card.OZG_Face());
        face.ToUpper();
        return OZG_Words.KeyArg("STR_OZG_CARD_" + face);
    }

    // The same, written small (the window lowers its first letter: "l:" in the said).
    static string CardNameLowArg(OZG_Card_Base card)
    {
        string face = OZG_Cards.FaceOf(card.OZG_Face());
        face.ToUpper();
        return "l:STR_OZG_CARD_" + face;
    }

    // A thing he carries, looked at in the window: another's mark or load on it opens to
    // him and spends nothing (design §10); his own or a clean one says nothing.
    static string Look(PlayerBase player, string key, out string said)
    {
        said = "";
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        string uid = OZG_Who.Uid(player);
        EntityAI thing = EntityAI.Cast(OZG_Keys.Find(key));
        if (!thing || uid == "" || thing.GetHierarchyRootPlayer() != player)
            return OZG_Const.SAY_CANNOT;
        OZG_Card_Base card = OZG_Card_Base.Cast(thing);
        if (card)
        {
            bool known = card.OZG_MarkedBy() == uid || card.OZG_Reads(uid);
            if (card.OZG_Marked() && !known)
            {
                card.OZG_AddReader(uid);
                said = OZG_Words.W1("STR_OZG_KWIN_SAID_CARD_OPENED", CardNameArg(card));
                OZG_Log.Info("mark-open: " + uid + " opened " + card.OZG_Face() + " in the kit's window, sign " + card.OZG_Sign().ToString());
            }
            return "";
        }
        OZG_Die_Base die = OZG_Die_Base.Cast(thing);
        if (die && die.OZG_Loaded() && !die.OZG_Knows(uid))
        {
            die.OZG_AddKnower(uid);
            said = DieOpenedWords(die);
            OZG_Log.Info("load-open: " + uid + " opened " + OZG_Keys.Of(die) + " in the kit's window (" + die.OZG_LoadFace().ToString() + " at " + die.OZG_LoadWeight().ToString() + "%)");
        }
        return "";
    }

    static string DieOpenedWords(OZG_Die_Base die)
    {
        int step = OZG_Cfg.Get().LoadStep(die.OZG_LoadWeight());
        string stepArg = OZG_Words.KeyArg(OZG_Const.LOAD_STEP + step.ToString());
        return OZG_Words.W2("STR_OZG_KWIN_SAID_DIE_OPENED", die.OZG_LoadFace().ToString(), stepArg);
    }

    // A card he carries marked from the window: a clean one takes a sign by chance and
    // spends a use; another's mark opens to him and spends nothing (design §10).
    static string Mark(PlayerBase player, string key, out string said)
    {
        said = "";
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1)
            return OZG_Const.SAY_SLEIGHT_OFF;
        string uid = OZG_Who.Uid(player);
        OZG_Card_Base card = OZG_Card_Base.Cast(OZG_Keys.Find(key));
        if (!card || uid == "" || card.GetHierarchyRootPlayer() != player)
            return OZG_Const.SAY_CANNOT;
        bool known = card.OZG_MarkedBy() == uid || card.OZG_Reads(uid);
        if (card.OZG_Marked() && known)
        {
            said = OZG_Words.W0("STR_OZG_KWIN_SAID_KNOWN");
            return "";
        }
        if (card.OZG_Marked())
        {
            card.OZG_AddReader(uid);
            said = OZG_Words.W1("STR_OZG_KWIN_SAID_CARD_OPENED", CardNameArg(card));
            OZG_Log.Info("mark-open: " + uid + " opened " + card.OZG_Face() + ", sign " + card.OZG_Sign().ToString());
            return "";
        }
        ItemBase tool = OZG_DiceCheat.Kit(player);
        if (!tool)
            return OZG_Const.SAY_LOAD_NO_KIT;
        OZG_Random dice = new OZG_Random();
        int sign = 1 + dice.Below(99);
        card.OZG_MarkBy(uid, sign);
        OZG_Recipes.Spend(tool);
        if (UsesLeft(player) <= 0)
            said = OZG_Words.W0("STR_OZG_KWIN_SAID_MARKED_LAST");
        else
            said = OZG_Words.W1("STR_OZG_KWIN_SAID_MARKED", CardNameLowArg(card));
        OZG_Log.Info("mark: " + uid + " marked " + card.OZG_Face() + " from the kit's window, sign " + sign.ToString());
        return "";
    }

    // A die he carries loaded from the window (OZG_DiceCheat.Load: another's load on it
    // opens instead), its words as the window says them.
    static string Load(PlayerBase player, string key, int face, int step, out string said)
    {
        said = "";
        OZG_Die_Base die = OZG_Die_Base.Cast(OZG_Keys.Find(key));
        string done;
        string refused = OZG_DiceCheat.Load(player, die, face, step, done);
        if (refused != "")
            return refused;
        if (done.IndexOf(OZG_Const.SAY_LOAD_OPENED) == 0)
        {
            said = DieOpenedWords(die);
            return "";
        }
        if (UsesLeft(player) <= 0)
        {
            said = OZG_Words.W0("STR_OZG_KWIN_SAID_LOADED_LAST");
            return "";
        }
        string stepArg = OZG_Words.KeyArg(OZG_Const.LOAD_STEP + step.ToString());
        said = OZG_Words.W2("STR_OZG_KWIN_SAID_LOADED", face.ToString(), stepArg);
        return "";
    }
}
