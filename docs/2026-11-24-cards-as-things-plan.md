# Cards as things (the card sharp's design, phase 1) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A deck becomes a container of fifty-two card items, a laid-out game keeps the deck inside its place, a deal is dealt from the cards the deck really holds (fewer, or with copies), and a player can count the deck.

**Architecture:** A deck may hold any number of cards of one face (the owner: "even a deck of aces alone"), so in a deal the first card of a face is named by the face (`As`) and every further one by the face and its number (`As.1`, `As.2`, with no end to it), and nothing cuts a card's name by position. A card of a deal is a record (`OZG_DealCard`): the name that tells it apart, the look of its back, and from the next phase its mark and who sees it. The session reads the deck's card items at each deal and hands the game those records (`OZG_Game.TakeCards`); the game keeps what it was dealt from in its own state. Card items live in sixty-two universal slots (fifty-two on an ordinary deck, all sixty-two on a sharp's deck). The place holds the deck in one slot and overflow cards in a small cargo; the deck leaves the placer's hands the way the game's own indoor fireplace takes its first stick, and comes back by the ground.

**Tech Stack:** Enforce Script (DayZ 1.30.164289, diag build), `config.cpp` written in part by a Python tool, the `dayz` MCP server (`mod_lint`, `mod_build`, `server_start`, `log_verdict`, `world_exec`, `client_*`), the stand's own verbs `oz_games` and `tgp`.

**The design:** `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`, §3 and §11.1. Marks, noticing and sleights (§4 to §6) are later phases; nothing of them is built here.

## Global Constraints

- **English and Ukrainian only** in the repository: code, comments, commit messages, docs. No Russian anywhere.
- **Work in `main`**, no branches. **Do NOT push; the owner alone gives the push word.** No remote is added, nothing goes to the Workshop.
- Commit author is the repository's own config (`Alexandru Covalschi`). Every commit message ends with `Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>`.
- **No DayZ API is invented.** Every engine call in this plan was read in the game's scripts (`.dayz-mcp/corpus/scripts`, section "What the game's own scripts say" below). A call that does not compile is looked up with `knowledge_find`, never guessed at.
- **Enforce Script rules that bite here**: no ternary; a compound boolean is worked out in a local before it goes into a field; a local is never named `out`, `owned`, `local` or like a type (`set`, `map`, `array`); a field or a method is never named like a class of the game or of another mod (`Count`, `Step`, `Slot`, `Hud`, `Debug` are taken): before the first build of a task, ask `knowledge_find(kind="class", name=<the new name>)` for every new field and method name that is one plain word; one expression holds about ten parts at most; `Length()` and `Substring()` count bytes.
- **A card's name has no fixed width** (`As`, `As.1`, `As.12`): nothing takes a card apart by position. `OZG_Cards.RankOf`, `SuitOf`, `FaceOf` and `OfMove` do.
- **Files are LF** (`git ls-files --eol` says `i/lf w/lf`). Edit with the Edit tool or Python; a Python script that carries a backslash is written with the Write tool, not a Bash heredoc (the shell eats `\\`).
- **The stand.** A stand cycle is `client_stop` (when a client runs), `server_stop`, `mod_build`, `job_wait`, `server_start(timeout=600)`, `job_wait`, `log_verdict(since=...)` which must say `pass`, `world_ready`. A client, when a step needs one: `client_start(language="English")`, `job_wait`. End every working session with `client_stop` (it unplugs the virtual pad). After any edit of a `.toml`, `project_open` again.
- **The stand may be in the owner's hands.** Ask before the first cycle of a working session which stand is free; never restart a stand he is playing on without his word.
- **The world checks of this plan are written for the Chernarus stand** (world 83): copy `dayz-mcp.local.chernarus.toml` over `dayz-mcp.local.toml`, then `project_open`. Its flat spot is the runway: `world_teleport "4699.56 339.02 10192.5"` after the client is in. World 83 is always booted with CF and the core (the profile does). On another stand, replace the coordinates of every check with a flat spot there, the points as far apart as here.
- **A server holds the map only near players**: every check that reads the world needs the client connected and standing there.
- **Stand calls** are written `oz_games {...}` and `tgp {...}`: the MCP tool `world_exec` with `verb="oz_games"` (or `"tgp"`) and those `args`. Every value of `args` is a string. What is compared is the answer's `detail`; the bridge cuts it at 400 characters. Several calls sent in one message run in the order sent.
- **Screenshots**: every stand run that has something on screen ends with its key shots sent to the owner (`SendUserFile`).
- The spec's numbers are copied here, not re-decided: 52 slots on an ordinary deck, 62 on a sharp's (the owner's numbers, 2026-11-24), `CountSeconds` 6, `CountNamesCards` 0.

## What the game's own scripts say

Read on 2026-11-24 in `.dayz-mcp/corpus/scripts` (1.30.164289). The plan stands on these; Task 1 measures the first six on the stand before the mod is touched.

1. **A placed kit is deleted by the action that placed it.** `ActionDeployObject.OnEndServer` (`4_world/classes/useractionscomponent/actions/continuous/deployactions/actiondeployobject.c:205-238`): when the item was placed and `IsBasebuildingKit()`, `m_MainItem.Delete()`. A kit is not dropped while it is being placed (`actiondeploybase.c:185-204`, `DropDuringPlacing` returns for a kit) and is not moved to the spot (`:206-209`): at `OnPlacementComplete` it is still in the placer's hands.
2. **The game itself takes a thing out of a player's hands into something the server has just made**: `ActionCreateIndoorFireplace.OnExecuteServer` (`.../actions/singleuse/actioncreateindoorfireplace.c:63-97`) creates the fireplace and calls `m_Player.ServerTakeEntityToTargetAttachment(fp_indoor, m_MainItem)`.
3. **That move is not done inside the call.** `Man.ServerTakeEntityToTargetAttachmentEx` (`3_game/entities/man.c:677`) goes through `GameInventory.TakeEntityAsTargetAttachmentEx` (`3_game/systems/inventory/inventory.c:1100`) to `HumanInventory.TakeToDst` (`humaninventory.c:243`), which turns a move out of the hands into a hand event; `DayZPlayerInventory.HandEvent` (`4_world/systems/inventory/dayzplayerinventory.c:4435`) only stores it, and `HandleHandEvent` (`:4587-4615`) sends a server's event through a juncture. `RedirectToHandEvent` answers `true` whether or not the event was taken (`humaninventory.c:228-229`), so the call's answer says nothing.
4. **Both sides ask the script whether the move is allowed.** `HandGuardCanMove` (`3_game/systems/inventory/hand_guards.c:290-313`) calls `GameInventory.LocationCanMoveEntity` unless the event is another player's: the placer's own client asks the place's `CanReceiveAttachment` too. A place that says no on a client would leave that client's hands believing they still hold the deck.
5. **A move between two things that are no player's hands is done at once**: `GameInventory.TakeToDst(InventoryMode.SERVER, src, dst)` (`inventory.c:1057-1074`) is `LocationSyncMoveEntity` and a message to the clients. `DropEntityWithTransform(InventoryMode.SERVER, owner, item, transform)` (`inventory.c:1250`) goes the same way.
6. **A server asks a thing on the ground into a player's hands** with `Man.ServerTakeEntityToHands(item)` (`man.c:197`); the game does so in `ActionRepackTent` (`.../actions/interact/actionrepacktent.c:144`). It is a hand event too: done a moment later, or not at all.
7. `CargoBase.GetItem(int)` and `GetItemCount()` (`cargo.c:28-32`), `GameInventory.FindFreeLocationFor(item, FindInventoryLocationType, out loc)` (`inventory.c:276`), `CreateAttachment(typeName)` (`:209`), `CreateEntityInCargo(typeName)` (`:143`), `GetAttachmentFromIndex(int)` (`:220`), `FindAttachmentByName(slotName)` (`:228`), `EnumerateInventory(InventoryTraversalType, out array<EntityAI>)` (`:127`).

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `TableGamesProbe/config.cpp`, `.../TGP_Client.c`, `.../TGP_Slots.c`, `.../TGP_Verb.c` | Task 1's measurement: a kit that goes into what it placed | modify |
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c` | a card's rank, suit, face and copy | modify |
| `.../3_Game/.../OZG_Game.c` | the game contract: `TakeCards` | modify |
| `.../3_Game/.../OZG_Durak.c`, `OZG_DurakState.c` | durak dealt from given cards; looks in its views | modify |
| `.../3_Game/.../OZG_Const.c`, `OZG_Settings.c`, `OZG_Wire.c` | names, two settings, the count's answer | modify |
| `tools/make_card_items.py` | writes the slots, the card classes, the cards' names and their lines for the economy | create |
| `OpenZone_Games/config.cpp`, `strings.csv`, `stringtable.csv`, `docs/examples/types-openzone-games.xml` | slots, cards, two kinds of deck, the place's slot and cargo | modify |
| `.../4_World/.../OZG_CardThings.c` | the card item; reading a place's cards | create |
| `.../4_World/.../OZG_Items.c` | looks of decks; the deck fills itself; the place holds a deck | modify |
| `.../4_World/.../OZG_Actions.c` | laying out never deletes the deck | modify |
| `.../4_World/.../OZG_Sessions.c`, `OZG_Session.c` | laying out keeps the deck; a deal reads it; counting; gathering | modify |
| `.../5_Mission/.../OZG_GameMenu.c`, `OZG_MissionGameplay.c`, `tools/make_layouts.py` | backs and faces by look, the count button and its answer | modify |
| `OpenZone_Games_Bridge/.../OZG_Bench.c`, `OZG_WorldOps.c`, `OZG_Verb.c`, `OZG_Bots.c` | checks: cards handed to a game, a deck's contents, pull, stuff, loose cards | modify |
| `docs/test-commands.md`, `docs/2026-11-24-cards-as-things-record.md`, `README.md` | what a person tries, and what was measured | modify, create |

---

### Task 1: Measure that a kit goes into what it placed, and comes back

The design keeps the deck item (spec §3.3). Facts 1 to 6 above say how; none of them was seen working on the stand in this shape: a kit, spared at the end of its own placing, asked out of the hands into the thing it has just placed. This task proves it on the probe's own kit before the mod is touched. **If the go line of Step 5 does not come, stop and report to the owner**: the way out is in "Risks" at the end, and it changes Tasks 4, 5 and 8.

**Files:**
- Modify: `TableGamesProbe/config.cpp`
- Modify: `TableGamesProbe/scripts/4_World/TableGamesProbe/TGP_Client.c:31-67` (class `TGP_Deck`)
- Modify: `TableGamesProbe/scripts/4_World/TableGamesProbe/TGP_Slots.c`
- Modify: `TableGamesProbe/scripts/4_World/TableGamesProbe/TGP_Verb.c:334-357`

**Interfaces:**
- Consumes: the probe's slots `TGP_C00`..`TGP_C51` and cards `TGP_SlotCard_00`..`51` (commit 6aa95d7).
- Produces: a measured answer, written into the record (Task 9) and into the `dayz-modding` skill (Step 7).

- [x] **Step 1: The config: one more slot, a mat that holds a deck, a deck that holds cards**

In `TableGamesProbe/config.cpp`, inside `class CfgSlots`, after the line of `Slot_TGP_C51`, add:

```cpp
    class Slot_TGP_Hold { name = "TGP_Hold"; displayName = "Probe deck"; ghostIcon = "set:dayz_inventory image:missing"; };
```

In `class TGP_Deck`, after `projectionTypename = "TGP_DeckPlacing";`, add:

```cpp
        inventorySlot[] = {"TGP_Hold"};
        attachments[] =
        {
            "TGP_C00", "TGP_C01", "TGP_C02", "TGP_C03", "TGP_C04", "TGP_C05", "TGP_C06", "TGP_C07", "TGP_C08", "TGP_C09", "TGP_C10", "TGP_C11", "TGP_C12",
            "TGP_C13", "TGP_C14", "TGP_C15", "TGP_C16", "TGP_C17", "TGP_C18", "TGP_C19", "TGP_C20", "TGP_C21", "TGP_C22", "TGP_C23", "TGP_C24", "TGP_C25",
            "TGP_C26", "TGP_C27", "TGP_C28", "TGP_C29", "TGP_C30", "TGP_C31", "TGP_C32", "TGP_C33", "TGP_C34", "TGP_C35", "TGP_C36", "TGP_C37", "TGP_C38",
            "TGP_C39", "TGP_C40", "TGP_C41", "TGP_C42", "TGP_C43", "TGP_C44", "TGP_C45", "TGP_C46", "TGP_C47", "TGP_C48", "TGP_C49", "TGP_C50", "TGP_C51"
        };
```

In `class TGP_Mat`, after its `weight = 100;`, add:

```cpp
        attachments[] = {"TGP_Hold"};
```

- [x] **Step 2: The probe's deck is spared and asked into the mat**

In `TGP_Client.c` replace the whole `class TGP_Deck` (from the comment `// The stand-in deck:` to the class's closing brace) with:

```c
// The stand-in deck: a kit in the game's sense. Its hologram is the config
// class named by projectionTypename, and laying it out leaves a mat that takes
// the deck itself, with what the deck holds.
class TGP_Deck extends ItemBase
{
    // Server: true only while the game's own action ends this deck's placing.
    protected bool m_TGP_Spared;

    override bool IsBasebuildingKit()
    {
        return !m_TGP_Spared;
    }

    void TGP_Spare(bool on)
    {
        m_TGP_Spared = on;
    }

    override bool IsDeployable()
    {
        return true;
    }

    override void SetActions()
    {
        super.SetActions();

        AddAction(ActionTogglePlaceObject);
        AddAction(ActionDeployObject);
    }

    override void OnPlacementComplete(Man player, vector position = "0 0 0", vector orientation = "0 0 0")
    {
        super.OnPlacementComplete(player, position, orientation);

        if (!GetGame().IsServer())
            return;

        EntityAI mat = EntityAI.Cast(GetGame().CreateObjectEx("TGP_Mat", position, ECE_PLACE_ON_SURFACE | ECE_NOLIFETIME));
        if (!mat)
            return;
        mat.SetPosition(position);
        mat.SetOrientation(orientation);

        // The deck is asked out of the placer's hands into the mat, as the
        // game's indoor fireplace takes its first stick
        // (actioncreateindoorfireplace.c:95). The hands answer later.
        TGP_Slots.s_AskedAt = GetGame().GetTime();
        TGP_Slots.s_ArrivedMs = -1;
        bool asked = false;
        if (player)
            asked = player.ServerTakeEntityToTargetAttachmentEx(mat, this, InventorySlots.GetSlotIdFromString("TGP_Hold"));
        string parent = "none";
        if (GetHierarchyParent())
            parent = GetHierarchyParent().GetType();
        TGP_Slots.s_Moved = "asked=" + TGP_Out.B(asked) + " parentAtOnce=" + parent;
    }
}

// The mat: when the deck reaches it is written down.
class TGP_Mat extends ItemBase
{
    override void EEItemAttached(EntityAI item, string slot_name)
    {
        super.EEItemAttached(item, slot_name);
        if (GetGame().IsServer() && TGP_Slots.s_ArrivedMs < 0)
            TGP_Slots.s_ArrivedMs = GetGame().GetTime() - TGP_Slots.s_AskedAt;
    }
}

// The game's action deletes a kit it has placed (actiondeployobject.c:230).
// The probe's deck is spared: for the length of that one call it is no kit.
modded class ActionDeployObject
{
    override void OnEndServer(ActionData action_data)
    {
        TGP_Deck deck;
        if (action_data)
            deck = TGP_Deck.Cast(action_data.m_MainItem);
        if (deck)
            deck.TGP_Spare(true);
        super.OnEndServer(action_data);
        if (deck)
            deck.TGP_Spare(false);
    }
}
```

- [x] **Step 3: Three ops: a full deck into hands, what the mat holds, the deck back**

In `TGP_Slots.c`, inside `class TGP_Slots`, add after `static const int CARDS = 52;`:

```c
    // What the probe's deck said when it was placed, when it asked the hands,
    // and how long after that the mat had it (-1: not yet).
    static string s_Moved;
    static int s_AskedAt;
    static int s_ArrivedMs = -1;

    protected static PlayerBase FirstPlayer()
    {
        array<Man> men = new array<Man>;
        GetGame().GetPlayers(men);
        for (int i = 0; i < men.Count(); i++)
        {
            PlayerBase p = PlayerBase.Cast(men.Get(i));
            if (p && p.GetIdentity())
                return p;
        }
        return null;
    }

    protected static EntityAI MatNear(vector at)
    {
        array<Object> found = new array<Object>;
        GetGame().GetObjectsAtPosition(at, 4.0, found, null);
        for (int i = 0; i < found.Count(); i++)
        {
            Object o = found.Get(i);
            if (o && o.GetType() == "TGP_Mat")
                return EntityAI.Cast(o);
        }
        return null;
    }

    // A probe deck with its fifty-two cards into the first player's hands.
    static string DeckFill()
    {
        PlayerBase p = FirstPlayer();
        if (!p)
            return "deckfill: nobody is connected";
        // What the hands hold goes: the probe wants them free, and the stand's
        // own delete does not reach a thing in hands. They are free a tick
        // later, so the deck is asked for again.
        EntityAI busy = p.GetHumanInventory().GetEntityInHands();
        if (busy)
        {
            string was = busy.GetType();
            GetGame().ObjectDelete(busy);
            return "deckfill: the hands held " + was + ", which is deleted; ask again";
        }
        EntityAI deck = p.GetHumanInventory().CreateInHands("TGP_Deck");
        if (!deck)
            return "deckfill: the hands did not take a deck";
        int made = 0;
        for (int i = 0; i < CARDS; i++)
        {
            int id = InventorySlots.GetSlotIdFromString("TGP_C" + Two(i));
            if (deck.GetInventory().CreateAttachmentEx("TGP_SlotCard_" + Two(i), id))
                made++;
        }
        return "deckfill: cards=" + made.ToString() + " held=" + deck.GetInventory().AttachmentCount().ToString();
    }

    // What the mat near a point holds, and what the first player's hands hold.
    static string Hold(vector at)
    {
        string s = "hold:";
        EntityAI mat = MatNear(at);
        if (!mat)
        {
            s = s + " no mat within 4 m";
        }
        else
        {
            s = s + " mat atts=" + mat.GetInventory().AttachmentCount().ToString();
            EntityAI deck = mat.GetInventory().FindAttachmentByName("TGP_Hold");
            if (deck)
                s = s + " deck=" + deck.GetType() + " cards=" + deck.GetInventory().AttachmentCount().ToString();
            else
                s = s + " deck=none";
        }
        string hands = "none";
        PlayerBase p = FirstPlayer();
        EntityAI held;
        if (p)
            held = p.GetHumanInventory().GetEntityInHands();
        if (held)
            hands = held.GetType() + "/" + held.GetInventory().AttachmentCount().ToString();
        s = s + " hands=" + hands + " ms=" + s_ArrivedMs.ToString() + "; " + s_Moved;
        return s;
    }

    // The deck out of the mat onto the ground where the mat lies, at once; the
    // mat is removed; free hands are asked to take the deck up.
    static string HoldBack(vector at)
    {
        EntityAI mat = MatNear(at);
        PlayerBase p = FirstPlayer();
        if (!mat || !p)
            return "holdback: no mat, or nobody connected";
        EntityAI deck = mat.GetInventory().FindAttachmentByName("TGP_Hold");
        if (!deck)
            return "holdback: the mat holds no deck";
        vector m4[4];
        mat.GetTransform(m4);
        bool dropped = mat.GetInventory().DropEntityWithTransform(InventoryMode.SERVER, mat, deck, m4);
        string parent = "none";
        if (deck.GetHierarchyParent())
            parent = deck.GetHierarchyParent().GetType();
        GetGame().ObjectDelete(mat);
        if (dropped && !p.GetHumanInventory().GetEntityInHands())
            p.ServerTakeEntityToHands(deck);
        string s = "holdback: dropped=" + TGP_Out.B(dropped) + " parentAfterDrop=" + parent;
        s = s + " cards=" + deck.GetInventory().AttachmentCount().ToString();
        return s;
    }
```

In `TGP_Verb.c`, before the line `if (op == "slotfill" || op == "slotclear")`, add:

```c
        // The hold probe: deckfill (a full probe deck into the hands), hold
        // pos="x y z" (what the mat there holds), holdback pos="x y z" (the
        // deck out of it again).
        if (op == "deckfill")
        {
            detail = TGP_Slots.DeckFill();
            return true;
        }

        if (op == "hold" || op == "holdback")
        {
            if (!Numbers(Arg(args, "pos", ""), nums) || nums.Count() != 3)
            {
                detail = op + ": pos must be 'x y z'";
                return false;
            }
            vector holdAt = Vector(nums.Get(0), nums.Get(1), nums.Get(2));
            if (op == "hold")
                detail = TGP_Slots.Hold(holdAt);
            else
                detail = TGP_Slots.HoldBack(holdAt);
            return true;
        }

```

and add `deckfill, hold, holdback` to the list of ops in the last `detail = "tgp knows the ops: ..."` line.

- [x] **Step 4: Lint, then a stand cycle with a client**

Run `mod_lint`. Expected: nothing refused.
Run the stand cycle, start the client, `world_teleport "4699.56 339.02 10192.5"`.

- [x] **Step 5: Place the probe's deck by hand of the client**

```
tgp {"op":"deckfill"}
```
Expected: `deckfill: cards=52 held=52`. (When it says the hands held something, that thing is deleted: ask again. `world_delete` does not reach a thing in a player's hands: measured 2026-11-24, 0 deleted.)

```
tgp {"op":"ask","do":"aimplace","text":"4698.1 339.02 10192.5"}
```
then `client_move(0, 0, 1)` (it attaches the pad; a hold measured from the attaching call is cut short), `client_trigger(which="right", seconds=14)` (the probe's deck asks for the game's default deploy time: 7 s was too short, 14 s enough), then `tgp {"op":"ask","do":"aim","text":""}`. Two seconds later:

```
tgp {"op":"hold","pos":"4698.1 339.02 10192.5"}
```
**Go**: `hold: mat atts=1 deck=TGP_Deck cards=52 hands=none ms=<n>; asked=1 parentAtOnce=<class>`. Write down `ms` (how long the hands took) and `parentAtOnce` (the player's class is what fact 3 predicts; `TGP_Mat` would mean the move is done inside the call after all).
**No go**: `deck=none`; or `cards` under 52; or `no mat within 4 m` with `hands=none` (the deck was deleted). Stop here and report, with the line.

`client_shot`: the character's hands are empty and the mat lies there. The client's own belief matters as much: `tgp {"op":"ask","do":"state","text":""}` then `tgp {"op":"told"}` must name no item in the hands. A client that still believes it holds the deck is a no go.

- [x] **Step 6: It survives a restart, and the deck comes back whole**

Stand cycle (no build), client, `world_teleport "4699.56 339.02 10192.5"`, then:

```
tgp {"op":"hold","pos":"4698.1 339.02 10192.5"}
```
Expected: `hold: mat atts=1 deck=TGP_Deck cards=52 hands=none ...`.

```
tgp {"op":"holdback","pos":"4698.1 339.02 10192.5"}
```
Expected: `holdback: dropped=1 parentAfterDrop=none cards=52`. Two seconds later:

```
tgp {"op":"hold","pos":"4698.1 339.02 10192.5"}
```
Expected: `hold: no mat within 4 m hands=TGP_Deck/52 ...`. `client_shot`: the deck is in the hands. When the hands did not take it (`hands=none`), `world_entities` for `TGP_Deck` near the spot must show it on the ground: that is still a go for the plan (a deck that lies where the game lay is the design's own second choice), and the record says so.

- [x] **Step 7: Write the facts down, and commit**

The measured facts go into the `dayz-modding` skill now, not at the end of the plan (a plan that stops here must not lose them): in `E:\dayzmod\dayz-agentic-modding-mcp\skills\dayz-modding\references\items-and-processing.md`, a section "A kit that goes into what it placed" with neutral names (`MyMod_Deck`, `MyMod_Mat`): the action's override, the server's ask, how long the hands took, what the client's guard asks, the way back by the ground, what a restart kept. `python -I openzone/scripts/check-generic.py` from `E:\dayzmod\dayz-modding-skill` must print nothing. Stage only this section's lines (the repository holds other sessions' changes: stage HEAD's blob plus the new lines with `git hash-object -w` and `git update-index --cacheinfo`), commit there, **do not push**.

```bash
cd /e/openzone/openzone-games
git add TableGamesProbe
git commit -m "The stand's probe: a kit that goes into what it placed

Measured before a deck is made to stay a thing when it is laid out: <one line
with what Step 5 and Step 6 said: ms, parentAtOnce, the restart, the way back>.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

**Done 2026-11-24, and it says GO.** Step 5: `hold: mat atts=1 deck=TGP_Deck cards=52 hands=none ms=73; asked=1 parentAtOnce=SurvivorM_Seth`, and the client's own state said `hands=none`. Step 6, after a restart: `hold: mat atts=1 deck=TGP_Deck cards=52 hands=none`; `holdback: dropped=1 parentAfterDrop=none cards=52`; three seconds later `hold: no mat within 4 m hands=TGP_Deck/52`, the client's state `hands=TGP_Deck`, and the client offers "Choose placement" for it: it is a kit again. So the move out of the hands is not done inside the call (73 ms on one machine), the way back by the ground works, and `EEItemAttached` fires for the deck when the mat is loaded from storage (`ms=30400` after the restart). One thing changed on the way: `deckfill` deletes what the hands hold, because `world_delete` does not reach it.

---

### Task 2: Any number of cards of one face; a game is dealt from the cards it is handed

Rules only, on the stand's bench: nothing of the world is touched, and no client is needed.

The owner's words this task answers (2026-11-24): "There may be as many copies as you like: you can put together any deck, even of aces alone", and "a card must have the property of the mark: whether it is seen by particular players, how it is seen". So a card's name in a game has no limit to its copies and no fixed width, and a card of a deal is a record: its name, the look of its back, and from the next phase its mark.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c` (lines 1-37 replaced; a class appended)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c:63-68`
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c` (`Begin`, `PublicView`, `SeatView`, `LoadState`, `Sound`), `OZG_DurakState.c`
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c:19`
- Modify: `OpenZone_Games/strings.csv` (then `python -I tools/stringtable.py OpenZone_Games`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Bench.c` (`OpBegin`, new `OpNames`), `OZG_Verb.c`, `OZG_Bots.c:32-38,54-55`

**Interfaces:**
- Produces: `OZG_Cards.RankOf(card) : int`, `.SuitOf(card) : string`, `.IsCard(card) : bool` for a name of any length; `OZG_Cards.FaceOf(card) : string`, `.CopyOf(card) : int`, `.CopyNamed(face, copy) : string`, `.OfMove(move) : string`; `class OZG_DealCard { string Name; int Look; static OZG_DealCard Make(string name, int look); }`; `OZG_Game.TakeCards(array<ref OZG_DealCard> cards)`; `OZG_Const.NO_CARDS`; `OZG_DurakState.Dealt`; in durak's views `OZG_DurakSeatPublic.Backs`, `OZG_DurakPublic.TrumpLook`, `.StockTop`, `.AttackLooks`, `.DefenceLooks`, `OZG_DurakPrivate.HeldLooks`.

- [x] **Step 1: The checks on the bench, written first**

In `OZG_Bench.c`, after `OpDeck` (before the comment of the next op), add:

```c
    // names: what a card's name says, the names of further cards of one face
    // with it.
    static bool OpNames(out string detail)
    {
        int wrong = 0;
        if (OZG_Cards.IsCard("As.0"))
            wrong++;
        if (OZG_Cards.IsCard("As.01"))
            wrong++;
        if (OZG_Cards.IsCard("As1"))
            wrong++;
        if (OZG_Cards.IsCard("As.x"))
            wrong++;
        if (OZG_Cards.IsCard("As."))
            wrong++;

        detail = "names: copy(As)=" + OZG_Cards.CopyOf("As").ToString();
        detail = detail + " copy(As.7)=" + OZG_Cards.CopyOf("As.7").ToString();
        detail = detail + " face(As.61)=" + OZG_Cards.FaceOf("As.61");
        detail = detail + " rank(As.61)=" + OZG_Cards.RankOf("As.61").ToString();
        detail = detail + " suit(As.61)=" + OZG_Cards.SuitOf("As.61");
        detail = detail + " named(As,0)=" + OZG_Cards.CopyNamed("As", 0);
        detail = detail + " named(As,3)=" + OZG_Cards.CopyNamed("As", 3);
        detail = detail + " move(a:As)=" + OZG_Cards.OfMove("a:As");
        detail = detail + " move(d:As.3:2)=" + OZG_Cards.OfMove("d:As.3:2");
        detail = detail + " move(take)=[" + OZG_Cards.OfMove("take") + "]";
        detail = detail + " wrong=" + wrong.ToString() + "/5";
        return true;
    }

```

In `OZG_Verb.c`, after the two lines of `if (op == "deck")`, add:

```c
        if (op == "names")
            return OZG_Bench.OpNames(detail);
```

and add `names` to `KnownOps()` after `deck`.

In `OpBegin`, after `OZG_VerbOps.Pairs(OZG_VerbOps.Arg(args, "opt", ""), options);`:

```c
        // cards=<cards> looks=<n,n,..>: what the deal is dealt from, as a
        // session hands it over after reading a deck.
        string given = OZG_VerbOps.Arg(args, "cards", "");
        if (given != "")
        {
            array<string> givenNames = new array<string>();
            OZG_Cards.Parse(given, givenNames);
            array<int> givenBacks = new array<int>();
            string lookText = OZG_VerbOps.Arg(args, "looks", "");
            if (lookText != "")
                OZG_VerbOps.Numbers(lookText, givenBacks);
            array<ref OZG_DealCard> handed = new array<ref OZG_DealCard>();
            for (int h = 0; h < givenNames.Count(); h++)
            {
                int back = 0;
                if (h < givenBacks.Count())
                    back = givenBacks[h];
                handed.Insert(OZG_DealCard.Make(givenNames[h], back));
            }
            game.TakeCards(handed);
        }
```

and add the line `//       cards=<cards> looks=<n,..>  what the deal is dealt from, and their backs` to the comment above `OpBegin` that lists its arguments.

Nothing is run yet: the build would stop on `CopyOf`.

- [x] **Step 2: A card's name, of any length**

Replace `OZG_Cards.c` from its first line down to the end of `IsCard` (everything above the comment of `Whole`) with:

```c
// Cards. A card is a rank of RANKS and a suit of SUITS: `6s` is the six of
// spades, `Th` the ten of hearts (design §8).
//
// A DECK MAY HOLD ANY NUMBER OF ONE FACE. Cards are things, and a deck can be
// put together of aces alone. In a deal the first card of a face is named by
// the face, `As`, and every further one by the face, a dot and its number:
// `As.1`, `As.2`, with no end to it. So A CARD'S NAME HAS NO FIXED WIDTH:
// nothing may cut it by position. The rules ask RankOf and SuitOf; whoever
// needs the face for a picture or a word asks FaceOf; whoever takes a card
// out of a move asks OfMove.
//
// THE NAME ONLY TELLS CARDS APART. What a card is besides its face (the look
// of its back, and from the next phase its mark and who sees it) is kept
// beside the name, a record a card (OZG_DealCard below): two aces of spades
// may wear one back or two, be marked or clean, and both ways they are two
// cards.
class OZG_Cards
{
    static const string RANKS = "23456789TJQKA";
    static const string SUITS = "shdc";
    // Between a face and the number of a further card of that face.
    static const string COPY_MARK = ".";

    // Whether a text is a card's name, and which card of its face it names:
    // 0 for the first, 1 and up for the others.
    protected static bool WellMade(string card, out int copy)
    {
        copy = 0;
        int n = card.Length();
        if (n < 2)
            return false;
        string ranks = RANKS;
        if (ranks.IndexOf(card.Substring(0, 1)) < 0)
            return false;
        string suits = SUITS;
        if (suits.IndexOf(card.Substring(1, 1)) < 0)
            return false;
        if (n == 2)
            return true;
        // A further card: the mark and a number from 1 up, and nothing else.
        if (n < 4 || card.Substring(2, 1) != COPY_MARK)
            return false;
        string digits = card.Substring(3, n - 3);
        int number = digits.ToInt();
        if (number < 1 || number.ToString() != digits)
            return false;
        copy = number;
        return true;
    }

    // 0 for a two .. 12 for an ace; -1 for what is not a card.
    static int RankOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return -1;
        string ranks = RANKS;
        return ranks.IndexOf(card.Substring(0, 1));
    }

    // "s", "h", "d" or "c"; "" for what is not a card.
    static string SuitOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return "";
        return card.Substring(1, 1);
    }

    // 0 for the first card of a face, 1 and up for the others; -1 for what is
    // not a card.
    static int CopyOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return -1;
        return copy;
    }

    // The face: the name of the first card of its kind. `As.3` is `As`.
    static string FaceOf(string card)
    {
        int copy;
        if (!WellMade(card, copy))
            return "";
        return card.Substring(0, 2);
    }

    // The name of the n-th card of a face; the face itself for 0.
    static string CopyNamed(string face, int copy)
    {
        string plain = FaceOf(face);
        if (plain == "" || copy < 0)
            return "";
        if (copy == 0)
            return plain;
        return plain + COPY_MARK + copy.ToString();
    }

    static bool IsCard(string card)
    {
        int copy;
        return WellMade(card, copy);
    }

    // The card a move names: `a:<card>`, `d:<card>:<n>`, `t:<card>`. "" for a
    // move that names none (`take`, `pass`).
    static string OfMove(string move)
    {
        int first = move.IndexOf(":");
        if (first < 0)
            return "";
        string named = move.Substring(first + 1, move.Length() - first - 1);
        int second = named.IndexOf(":");
        if (second >= 0)
            named = named.Substring(0, second);
        if (!IsCard(named))
            return "";
        return named;
    }

```

Everything from `Whole` down stays as it is (`Whole`, `FromRank`, `Line`, `Parse`). At the end of the file, after the closing brace of `OZG_Cards`, add:

```c

// One card of a deal, as a game is handed it. The name tells it apart from
// every other card of the deal; the rest is what the card is like. The next
// phases add what else a card carries into a game: the sign of its mark, and
// which seats read it and which notice it (design §4).
class OZG_DealCard
{
    // A face, or for a further card of that face the face, a dot and its
    // number: `As`, `As.1`, `As.2` (OZG_Cards.CopyNamed).
    string Name;
    // The look of its back (OZG_Decks.StyleAt).
    int Look;

    static OZG_DealCard Make(string name, int look)
    {
        OZG_DealCard c = new OZG_DealCard();
        c.Name = name;
        c.Look = look;
        return c;
    }
}
```

- [x] **Step 3: `TakeCards` in the contract**

In `OZG_Game.c`, after the method `Fresh()` and before the comment `// ---- a game being played`, add:

```c
    // ---- the cards of a deal -------------------------------------------------

    // What the next deal is dealt from, when there is a deck to read: a
    // record a card, every name of the deal its own. A game nobody hands
    // cards to deals its own whole set, every back of look 0: what the
    // stand's bench and a check rely on.
    protected ref array<ref OZG_DealCard> m_Given;

    void TakeCards(array<ref OZG_DealCard> cards)
    {
        m_Given = new array<ref OZG_DealCard>();
        for (int i = 0; i < cards.Count(); i++)
        {
            OZG_DealCard c = cards[i];
            if (c && OZG_Cards.IsCard(c.Name))
                m_Given.Insert(OZG_DealCard.Make(c.Name, c.Look));
        }
    }

```

- [x] **Step 4: The refusal's key and word**

In `OZG_Const.c`, after `static const string NO_CARD    = "STR_OZG_NO_CARD";`:

```c
    static const string NO_CARDS   = "STR_OZG_NO_CARDS";
```

In `strings.csv`, after the line of `STR_OZG_NO_CARD`:

```
STR_OZG_NO_CARDS,У колоді замало карт для цієї гри,There are not enough cards in the deck for this game
```

Run `python -I tools/stringtable.py OpenZone_Games`. Expected: `stringtable.csv: 158 strings` (one more than before).

- [x] **Step 5: The state keeps what the deal began with**

In `OZG_DurakState.c`:

1. `static const int VERSION = 1;` becomes `static const int VERSION = 2;`.
2. In `class OZG_DurakState`, after `int Tie;`:

```c
    // Every card this deal began with, in the order it was handed over: its
    // name and what it is like. Sound counts against it.
    ref array<ref OZG_DealCard> Dealt;
```

   and in its constructor, after `DefenceCards = new array<string>();`:

```c
        Dealt = new array<ref OZG_DealCard>();
```

3. Replace `class OZG_DurakSeatPublic` with:

```c
// A seat as everybody may see it.
class OZG_DurakSeatPublic
{
    int SeatNo;
    int CardCount;
    int Gone;
    int Passed;
    // The look of each card's back, the lowest first: what anybody at the
    // table sees of a hand.
    ref array<int> Backs;

    void OZG_DurakSeatPublic()
    {
        Backs = new array<int>();
    }
}
```

4. In `OZG_DurakPublic`, after `int Tie;`:

```c
    int TrumpLook;
    // The look of the card drawn next; -1 when the stock is empty.
    int StockTop;
    // Beside AttackCards and DefenceCards; -1 beside an unbeaten card.
    ref array<int> AttackLooks;
    ref array<int> DefenceLooks;
```

   and in its constructor, after `Awaited = new array<int>();`:

```c
        AttackLooks = new array<int>();
        DefenceLooks = new array<int>();
        StockTop = -1;
```

5. In `OZG_DurakPrivate`, after `ref array<string> CanPlay;`:

```c
    // Beside Held.
    ref array<int> HeldLooks;
```

   and in its constructor `HeldLooks = new array<int>();`.

- [x] **Step 6: Durak deals from what it is handed**

In `OZG_Durak.c`, `Begin`, replace

```c
        array<string> deck = new array<string>();
        CardsUsed(deck);
```

with

```c
        // The cards of this deal: what the session read out of the deck, or
        // the game's whole set when nobody handed any. The state keeps its
        // own record of them, in the order they were handed over.
        array<string> deck = new array<string>();
        int f;
        if (m_Given)
        {
            for (f = 0; f < m_Given.Count(); f++)
            {
                s.Dealt.Insert(OZG_DealCard.Make(m_Given[f].Name, m_Given[f].Look));
                deck.Insert(m_Given[f].Name);
            }
        }
        else
        {
            CardsUsed(deck);
            for (f = 0; f < deck.Count(); f++)
                s.Dealt.Insert(OZG_DealCard.Make(deck[f], 0));
        }
        // Six to each seat at the least; the trump is then the last card dealt.
        if (deck.Count() < n * OZG_DurakConst.HAND)
        {
            refusal = OZG_Const.NO_CARDS;
            return false;
        }
```

Before `protected int SeatNoAt(int i)` add:

```c
    // The look of a card's back, by what the deal began with; 0 for a card the
    // deal does not know.
    protected int LookOf(string card)
    {
        if (!m_S || card == "")
            return 0;
        for (int i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (m_S.Dealt[i] && m_S.Dealt[i].Name == card)
                return m_S.Dealt[i].Look;
        }
        return 0;
    }

```

In `PublicView`, after `v.Tie = m_S.Tie;`:

```c
        v.TrumpLook = LookOf(m_S.TrumpCard);
        if (m_S.StockCards.Count() > 0)
            v.StockTop = LookOf(m_S.StockCards[0]);
        for (int t = 0; t < m_S.AttackCards.Count(); t++)
        {
            v.AttackLooks.Insert(LookOf(m_S.AttackCards[t]));
            if (m_S.DefenceCards[t] != "")
                v.DefenceLooks.Insert(LookOf(m_S.DefenceCards[t]));
            else
                v.DefenceLooks.Insert(-1);
        }
```

and inside its loop over the seats, after `shown.CardCount = seat.Held.Count();`:

```c
            for (int b = 0; b < seat.Held.Count(); b++)
                shown.Backs.Insert(LookOf(seat.Held[b]));
            shown.Backs.Sort();
```

In `SeatView`, after `v.Held.Copy(m_S.SeatList[i].Held);`:

```c
            for (int h = 0; h < v.Held.Count(); h++)
                v.HeldLooks.Insert(LookOf(v.Held[h]));
```

In `LoadState`, replace

```c
        if (!s || s.Version != OZG_DurakConst.VERSION)
            return false;
```

with

```c
        if (!s)
            return false;
        // A game saved before cards were things: it was dealt from the whole
        // set, every back the deck's own.
        if (s.Version == 1)
        {
            s.Dealt = new array<ref OZG_DealCard>();
            array<string> whole = new array<string>();
            CardsUsed(whole);
            for (int old = 0; old < whole.Count(); old++)
                s.Dealt.Insert(OZG_DealCard.Make(whole[old], 0));
            s.Version = OZG_DurakConst.VERSION;
        }
        if (s.Version != OZG_DurakConst.VERSION)
            return false;
        if (!s.Dealt)
            return false;
```

In `Sound`, replace the two lines of its comment with

```c
    // Every card the deal began with exactly once, between the hands, the
    // stock, the table and the discards; and the few numbers in their ranges.
```

and replace

```c
        array<string> want = new array<string>();
        CardsUsed(want);
```

with

```c
        array<string> want = new array<string>();
        for (i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (!m_S.Dealt[i])
            {
                why = "a card of the deal has no record";
                return false;
            }
            want.Insert(m_S.Dealt[i].Name);
        }
```

Then read `OZG_Durak.c` once for anything else that takes a card apart by position (Grep for `Substring(` and `Length()` in the file): moves are already split at the colons (`move.Split(":", parts)` in `Act`), and nothing else was found when this plan was written.

- [x] **Step 7: The bots take a move's card and the trump's suit by the helpers**

`OZG_Bots.c`, `CardOf`: replace its body

```c
        if (move.Length() < 4)
            return "";
        return move.Substring(2, 2);
```

with

```c
        return OZG_Cards.OfMove(move);
```

and in `Choose` replace

```c
            if (pub.TrumpCard.Length() == 2)
                trumpSuit = pub.TrumpCard.Substring(1, 1);
```

with

```c
            trumpSuit = OZG_Cards.SuitOf(pub.TrumpCard);
```

- [x] **Step 8: Lint, a stand cycle without a client, the bench**

Run `mod_lint`, then the stand cycle.

```
oz_games {"op":"names"}
```
Expected: `names: copy(As)=0 copy(As.7)=7 face(As.61)=As rank(As.61)=12 suit(As.61)=s named(As,0)=As named(As,3)=As.3 move(a:As)=As move(d:As.3:2)=As.3 move(take)=[] wrong=0/5`.

```
oz_games {"op":"deck"}
```
Expected: what it said before this task, to the letter (`notcards=5/5` among it).

```
oz_games {"op":"begin","seats":"0,1"}
```
Expected: a line beginning `durak podkidnoy trump=` with `stock=24` (the whole set, as before).

A second ace of spades, with another back:

```
oz_games {"op":"begin","seats":"0,1","cards":"6s 7s 8s 9s Ts Js Qs Ks As 6h 7h 8h As.1","looks":"0,0,0,0,0,0,0,0,0,0,0,0,1"}
oz_games {"op":"save"}
oz_games {"op":"load"}
```
Expected: `stock=1`; `saved <n> B sound=yes -> games_save.json`; `loaded | durak ...`.

Aces alone, fourteen of them:

```
oz_games {"op":"begin","seats":"0,1","cards":"As As.1 As.2 As.3 As.4 As.5 As.6 As.7 As.8 As.9 As.10 As.11 As.12 As.13"}
```
Expected: `durak podkidnoy trump=As... stock=2 ...`; the line names the opener (`op=`) and the defender (`def=`).

```
oz_games {"op":"can","seat":"<the opener>"}
```
Expected: six moves, `a:` and a different ace each. Play the first of them:

```
oz_games {"op":"act","seat":"<the opener>","move":"<that move>"}
oz_games {"op":"can","seat":"<the defender>"}
```
Expected: `ok ...`; then the defender can `take` and nothing else (an ace does not beat an ace). `oz_games {"op":"save"}`: `sound=yes`.

```
oz_games {"op":"begin","seats":"0,1","cards":"6s 7s 8s"}
```
Expected: `no STR_OZG_NO_CARDS`.

```
oz_games {"op":"fuzz","games":"20"}
```
Expected: what it said before this task (the header of `OZG_Fuzz.c` says what the line looks like): every game ends, no state unsound.

- [x] **Step 9: Commit**

```bash
git add OpenZone_Games OpenZone_Games_Bridge
git commit -m "Any number of cards of one face, and durak dealt from what it is handed

A further card of a face is named by the face and its number, with no end to
it, so a card's name has no fixed width and nothing cuts it by position. A
card of a deal is a record: its name and the look of its back; the mark will
join them. A game takes its cards from the session (TakeCards) and keeps what
the deal began with; it refuses a deal with too few. Its views say the look of
every back that can be seen.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

**Done 2026-11-24.** The bench said, to the letter: `names: copy(As)=0 copy(As.7)=7 face(As.61)=As rank(As.61)=12 suit(As.61)=s named(As,0)=As named(As,3)=As.3 move(a:As)=As move(d:As.3:2)=As.3 move(take)=[] wrong=0/5`; `deck` as before; the whole set `stock=24`; a second ace of another back `stock=1`, `saved 684 B sound=yes`, `loaded`; fourteen aces `trump=As.9 stock=2`, the opener `can: a:As.12 a:As a:As.13 a:As.11 a:As.1 a:As.2`, the defender after `a:As.12` `can: take`, `saved 771 B sound=yes`; three cards `no STR_OZG_NO_CARDS`; `fuzz: 20 games, 3032 moves, longest 242, ... 0 defects, 465 ms`.

---

### Task 3: Card items, sixty-two slots, two kinds of deck that fill themselves

**Files:**
- Create: `tools/make_card_items.py`
- Modify: `OpenZone_Games/config.cpp` (five marked regions, the deck classes, `CfgPatches`)
- Modify: `OpenZone_Games/strings.csv` (a marked region), then the string table
- Modify: `docs/examples/types-openzone-games.xml` (a marked region, two decks)
- Create: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c` (`OZG_Decks`, `OZG_Deck_Base`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c`
- Modify: `OpenZone_Games_Bridge/.../OZG_WorldOps.c`, `OZG_Verb.c` (ops `cards`, `pull`, `undeck`), `OZG_Bots.c:404-408,504`

**Interfaces:**
- Consumes: `OZG_Cards.Whole`, `OZG_Cards.Parse` (exist).
- Produces: slots `OZG_Card01`..`OZG_Card62` and `OZG_DeckHold`; config classes `OZG_Card_Base`, `OZG_Card_<face>` (`OZG_Card_As`), `OZG_Deck_Classic_Sharp`, `OZG_Deck_Zone_Sharp`; script `OZG_Card_Base.OZG_Face() : string`, `.OZG_Look() : int`, `.OZG_SetStyle(string style)`; `OZG_Decks.LookOfDeck(string deckClass) : int`, `.LookOfStyle(string style) : int`, `.StyleAt(int look) : string`, `.SheetAt(int look) : string`; `OZG_Deck_Base.OZG_FillNow()`, `.OZG_Spare(bool on)`, `.OZG_Slots() : int`, `.OZG_Free() : int`; `OZG_Const.CARD_PREFIX = "OZG_Card_"`, `.SLOT_HOLD = "OZG_DeckHold"`; in the bridge `OZG_WorldOps.DeckOf(PlayerBase) : OZG_Deck_Base`, `.DeckLine(OZG_Deck_Base) : string`, `.PullFrom(OZG_Deck_Base, string faces) : int`.

- [ ] **Step 1: The generator**

Write `tools/make_card_items.py` with the Write tool:

```python
# Writes what there is one of for every card: sixty-two slots, fifty-two card
# classes, the slot lists of the decks, the cards' names, and the cards' lines
# for a server's economy.
#
#   python -I tools/make_card_items.py
#
# It rewrites the text between pairs of marker lines in
# OpenZone_Games/config.cpp, OpenZone_Games/strings.csv and
# docs/examples/types-openzone-games.xml; everything outside the markers is
# left as it is. Run tools/stringtable.py after it.
import io
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MOD = os.path.join(ROOT, "OpenZone_Games")
TYPES = os.path.join(ROOT, "docs", "examples", "types-openzone-games.xml")

RANKS = "23456789TJQKA"
SUITS = "shdc"
SLOTS = 62
PLAIN = 52

RANK_UA = ["Двійка", "Трійка", "Четвірка", "П'ятірка", "Шістка", "Сімка", "Вісімка", "Дев'ятка",
           "Десятка", "Валет", "Дама", "Король", "Туз"]
RANK_EN = ["Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen",
           "King", "Ace"]
SUIT_UA = {"s": "пік", "h": "чирв", "d": "бубн", "c": "треф"}
SUIT_EN = {"s": "of spades", "h": "of hearts", "d": "of diamonds", "c": "of clubs"}


def faces():
    # The order of OZG_Cards.Whole: a suit at a time, the two first.
    out = []
    for s in SUITS:
        for r in RANKS:
            out.append(r + s)
    return out


def read(path):
    raw = io.open(path, "rb").read()
    bom = raw.startswith(b"\xef\xbb\xbf")
    text = raw.decode("utf-8-sig")
    crlf = "\r\n" in text
    return text.replace("\r\n", "\n"), bom, crlf


def write(path, text, bom, crlf):
    if crlf:
        text = text.replace("\n", "\r\n")
    data = text.encode("utf-8")
    if bom:
        data = b"\xef\xbb\xbf" + data
    io.open(path, "wb").write(data)


def between(text, opener, closer, lines):
    i = text.index(opener)
    j = text.index(closer)
    i = text.index("\n", i) + 1
    j = text.rindex("\n", 0, j) + 1
    return text[:i] + "\n".join(lines) + "\n" + text[j:]


def rows(names, indent):
    out = []
    for k in range(0, len(names), 10):
        part = ", ".join('"%s"' % n for n in names[k:k + 10])
        if k + 10 < len(names):
            part += ","
        out.append(indent + part)
    return out


def slot_list(names):
    return ["        attachments[] =", "        {"] + rows(names, "            ") + ["        };"]


def config():
    names = ["OZG_Card%02d" % i for i in range(1, SLOTS + 1)]

    slots = []
    for n in names:
        slots.append('    class Slot_%s { name = "%s"; displayName = "$STR_OZG_CARD_SLOT"; '
                     'ghostIcon = "set:dayz_inventory image:missing"; };' % (n, n))

    cards = ["    class OZG_Card_Base: Inventory_Base",
             "    {",
             "        scope = 0;",
             '        descriptionShort = "$STR_OZG_CARD_DESC";',
             '        model = "\\OpenZone_Games\\models\\deck\\ozg_deck.p3d";',
             "        itemSize[] = {1, 1};",
             "        weight = 2;",
             "        rotationFlags = 16;",
             '        hiddenSelections[] = {"camo"};',
             '        hiddenSelectionsTextures[] = {"OpenZone_Games\\data\\cards\\ozg_deck_classic_co.paa"};',
             '        hiddenSelectionsMaterials[] = {"OpenZone_Games\\data\\cards\\ozg_deck.rvmat"};',
             '        ozgFace = "";',
             "        inventorySlot[] =",
             "        {"]
    cards += rows(names, "            ")
    cards += ["        };", "    };"]
    for f in faces():
        cards.append('    class OZG_Card_%s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_%s"; '
                     'ozgFace = "%s"; };' % (f, f.upper(), f))

    path = os.path.join(MOD, "config.cpp")
    text, bom, crlf = read(path)
    for name, lines in (("slots", slots),
                        ("cards", cards),
                        ("deck slots", slot_list(names[:PLAIN])),
                        ("sharp slots, classic", slot_list(names)),
                        ("sharp slots, zone", slot_list(names))):
        text = between(text, "// >>> generated: " + name, "// <<< generated: " + name, lines)
    write(path, text, bom, crlf)
    return len(names)


def strings():
    lines = []
    for f in faces():
        r = RANKS.index(f[0])
        lines.append("STR_OZG_CARD_%s,%s %s,%s %s" % (f.upper(), RANK_UA[r], SUIT_UA[f[1]],
                                                      RANK_EN[r], SUIT_EN[f[1]]))
    path = os.path.join(MOD, "strings.csv")
    text, bom, crlf = read(path)
    text = between(text, "# >>> generated: cards", "# <<< generated: cards", lines)
    write(path, text, bom, crlf)
    return len(lines)


def types():
    lines = []
    for f in faces():
        lines += ['    <type name="OZG_Card_%s">' % f,
                  "        <nominal>0</nominal>",
                  "        <lifetime>14400</lifetime>",
                  "        <restock>0</restock>",
                  "        <min>0</min>",
                  "        <quantmin>-1</quantmin>",
                  "        <quantmax>-1</quantmax>",
                  "        <cost>100</cost>",
                  '        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" '
                  'count_in_player="0" crafted="0" deloot="0"/>',
                  '        <category name="tools"/>',
                  "    </type>"]
    text, bom, crlf = read(TYPES)
    text = between(text, "<!-- >>> generated: cards -->", "<!-- <<< generated: cards -->", lines)
    write(TYPES, text, bom, crlf)
    return len(faces())


print("slots %d, names %d, types %d" % (config(), strings(), types()))
```

- [ ] **Step 2: The markers and the hand-written part of the config**

In `OpenZone_Games/config.cpp`:

1. In `CfgPatches`, `units[]` becomes `{"OZG_Deck_Classic", "OZG_Deck_Zone", "OZG_Deck_Classic_Sharp", "OZG_Deck_Zone_Sharp"};`.

2. Between the closing `};` of `class CfgMods` and `class CfgVehicles`, add:

```cpp
// A slot for a card, any card: sixty-two of them, and every card names them
// all. An ordinary deck has the first fifty-two, a sharp's deck all sixty-two.
// One more slot is the place's, for the deck it holds. The sixty-two are
// written by
// tools/make_card_items.py.
class CfgSlots
{
    class Slot_OZG_DeckHold { name = "OZG_DeckHold"; displayName = "$STR_OZG_DECK_NAME"; ghostIcon = "set:dayz_inventory image:missing"; };
// >>> generated: slots
// <<< generated: slots
};

```

3. Replace the comment above `class OZG_Deck_Base` (the block that begins `// A deck is a config class and its art`) with:

```cpp
    // A deck is a config class and its art; no script is needed to add one.
    // Another mod adds a deck with a class that inherits OZG_Deck_Base, an
    // imageset of its own, ozg_cards_<ozgStyle>, and a sheet for the 3D
    // things, ozgSheet (the back on its left half, a blank face and the edges
    // of a stack on its right). A deck holds cards, which are things, in slots
    // that take any card: a new deck fills itself with the fifty-two of its
    // own look, and what it holds after that is its owner's business.
```

4. In `class OZG_Deck_Base`, after `ozgSheet = "OpenZone_Games\data\cards\ozg_deck_classic_co.paa";`, add:

```cpp
        // A laid-out game keeps its deck in this slot of the place.
        inventorySlot[] = {"OZG_DeckHold"};
// >>> generated: deck slots
// <<< generated: deck slots
```

5. After the closing `};` of `class OZG_Deck_Zone`, add:

```cpp

    // A card sharp's deck: the same deck outside, ten empty slots more
    // inside, for the cards mixed in.
    class OZG_Deck_Classic_Sharp: OZG_Deck_Classic
    {
// >>> generated: sharp slots, classic
// <<< generated: sharp slots, classic
    };

    class OZG_Deck_Zone_Sharp: OZG_Deck_Zone
    {
// >>> generated: sharp slots, zone
// <<< generated: sharp slots, zone
    };

    // ------------------------------------------------------------ cards
    //
    // One class a face, written by tools/make_card_items.py. A card fits every
    // card slot. Its back is the look of the deck it was made for, which the
    // script keeps and tells the clients; the model and the picture are the
    // deck's own until a card has art of its own.
// >>> generated: cards
// <<< generated: cards
```

6. In `strings.csv`, at the end of the file, add:

```
# -- cards as things
STR_OZG_CARD_SLOT,Карта,Card
STR_OZG_CARD_DESC,Гральна карта. Її місце в колоді.,A playing card. It belongs in a deck.
# >>> generated: cards
# <<< generated: cards
```

7. In `docs/examples/types-openzone-games.xml`: the heading comment's first line becomes `OpenZone Games: the decks and the cards for a server's economy.`; after the `</type>` of `OZG_Deck_Zone` add the two sharp decks (copies of the two entries above them, named `OZG_Deck_Classic_Sharp` and `OZG_Deck_Zone_Sharp`) and then:

```xml
    <!-- A card out of its deck is a thing of its own; one entry a face. -->
    <!-- >>> generated: cards -->
    <!-- <<< generated: cards -->
```

Run `python -I tools/make_card_items.py`. Expected: `slots 62, names 52, types 52`.
Run `python -I tools/stringtable.py OpenZone_Games`. Expected: `stringtable.csv: 212 strings`.
Run `grep -c "class OZG_Card_" OpenZone_Games/config.cpp`. Expected: `53`.
Run the tool a second time, then `git diff --stat` before and after: the second run changes nothing.

- [ ] **Step 3: The looks of decks**

In `OZG_Items.c`, `class OZG_Decks`: add beside `s_Classes`:

```c
    // The looks there are: every style a deck names, in the config's order,
    // and the sheet of the first deck of each. A look's number is what a card
    // tells the clients; both sides read the same config in the same order.
    protected static ref array<string> s_Styles;
    protected static ref array<string> s_Sheets;
```

In `Load()`, after `s_Classes = new array<string>();` add

```c
        s_Styles = new array<string>();
        s_Sheets = new array<string>();
```

and after `s_Classes.Insert(name);`:

```c
            string style = StyleOf(name);
            if (s_Styles.Find(style) < 0)
            {
                s_Styles.Insert(style);
                s_Sheets.Insert(GetGame().ConfigGetTextOut("CfgVehicles " + name + " ozgSheet"));
            }
```

After `SheetOf`, add:

```c
    // The number of a look; 0 for a style no deck names.
    static int LookOfStyle(string style)
    {
        if (!s_Classes)
            Load();
        int at = s_Styles.Find(style);
        if (at < 0)
            return 0;
        return at;
    }

    // The look of a deck's own cards.
    static int LookOfDeck(string deckClass)
    {
        return LookOfStyle(StyleOf(deckClass));
    }

    static string StyleAt(int look)
    {
        if (!s_Classes)
            Load();
        if (look < 0 || look >= s_Styles.Count())
            return "classic";
        return s_Styles[look];
    }

    static string SheetAt(int look)
    {
        if (!s_Classes)
            Load();
        if (look < 0 || look >= s_Sheets.Count())
            return "";
        return s_Sheets[look];
    }
```

and in `Reset()` add `s_Styles = null;` and `s_Sheets = null;`.

- [ ] **Step 4: The card item**

In `OZG_Const.c`, before the line `static const float PLACE_LIFETIME = 3888000.0;`, add:

```c
    // A card's class is this and its face: OZG_Card_As.
    static const string CARD_PREFIX = "OZG_Card_";
    // The place's slot for the deck of its game.
    static const string SLOT_HOLD   = "OZG_DeckHold";
    // The block of a card in the world's storage.
    static const int CARD_SAVE = 1;
```

Create `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c`:

```c
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
```

- [ ] **Step 5: The deck fills itself, takes cards only, and can be spared**

In `OZG_Items.c`, replace the comment above `class OZG_Deck_Base`, the class's opening and its `IsBasebuildingKit` (down to that method's closing brace) with:

```c
// The deck. A kit in the game's sense: "choose placement", the hologram,
// "deploy" held. A container of its cards as well: slots that take any card.
class OZG_Deck_Base extends ItemBase
{
    // Server: true only while the game's own action ends this deck's placing.
    // A kit that was placed is deleted there (actiondeployobject.c:230); a
    // deck is not spent by being laid out (OZG_ActionLayOut.OnEndServer).
    protected bool m_OZG_Spared;
    // Came back from the world's storage, or from a character's: what it held
    // then is what it holds, and it is never filled again. Nothing of the
    // deck's own is stored; that it WAS stored is what tells an old deck from
    // a new one.
    protected bool m_OZG_Stored;
    protected bool m_OZG_Filled;

    override bool IsBasebuildingKit()
    {
        return !m_OZG_Spared;
    }

    void OZG_Spare(bool on)
    {
        m_OZG_Spared = on;
    }
```

and, in the same class after `GetDeployTime`, add:

```c
    override void EEInit()
    {
        super.EEInit();
        // Not at once: a deck that comes back from storage learns only after
        // this that it was stored.
        if (GetGame() && GetGame().IsServer())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_FillIfNew, 200, false);
    }

    override bool OnStoreLoad(ParamsReadContext ctx, int version)
    {
        m_OZG_Stored = true;
        return super.OnStoreLoad(ctx, version);
    }

    void OZG_FillIfNew()
    {
        if (!m_OZG_Stored)
            OZG_FillNow();
    }

    // Server: the fifty-two cards of this deck's own look, once.
    void OZG_FillNow()
    {
        if (m_OZG_Filled)
            return;
        m_OZG_Filled = true;
        if (!GetGame().IsServer())
            return;
        if (GetInventory().AttachmentCount() > 0)
            return;
        string style = OZG_Decks.StyleOf(GetType());
        array<string> faces = new array<string>();
        OZG_Cards.Whole(faces);
        int made = 0;
        for (int i = 0; i < faces.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(GetInventory().CreateAttachment(OZG_Const.CARD_PREFIX + faces[i]));
            if (card)
            {
                card.OZG_SetStyle(style);
                made++;
            }
        }
        if (made != faces.Count())
            OZG_Log.Warn("a deck " + GetType() + " took " + made.ToString() + " of its cards");
    }

    // How many cards it has room for: the slots its class names.
    int OZG_Slots()
    {
        TStringArray names = new TStringArray;
        GetGame().ConfigGetTextArray("CfgVehicles " + GetType() + " attachments", names);
        return names.Count();
    }

    int OZG_Free()
    {
        return OZG_Slots() - GetInventory().AttachmentCount();
    }

    // A deck takes cards and nothing else.
    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (!attachment || !attachment.IsInherited(OZG_Card_Base))
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }
```

A deck made by a build before this one has no cards and was stored: it stays empty. That is accepted (nothing is published); the record says so, and `!ozg deck` gives a new one.

- [ ] **Step 6: Ops to look at a deck and to take cards out of it; the chat's words for a sharp's deck**

In `OZG_WorldOps.c`, before `// give who= [deck=class]`, add:

```c
    // The deck a player holds in his hands, or the first deck in his
    // inventory; null when he has none.
    static OZG_Deck_Base DeckOf(PlayerBase p)
    {
        if (!p)
            return null;
        OZG_Deck_Base deck = OZG_Deck_Base.Cast(p.GetHumanInventory().GetEntityInHands());
        if (deck)
            return deck;
        array<EntityAI> items = new array<EntityAI>();
        p.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        for (int i = 0; i < items.Count(); i++)
        {
            deck = OZG_Deck_Base.Cast(items[i]);
            if (deck)
                return deck;
        }
        return null;
    }

    // `deck <class> slots= cards= free= faces=<different faces> looks=<look>:<how many>,..`
    static string DeckLine(OZG_Deck_Base deck)
    {
        int held = deck.GetInventory().AttachmentCount();
        string s = "deck " + deck.GetType() + " slots=" + deck.OZG_Slots().ToString();
        s = s + " cards=" + held.ToString() + " free=" + deck.OZG_Free().ToString();
        array<string> faces = new array<string>();
        array<int> looks = new array<int>();
        array<int> counts = new array<int>();
        for (int i = 0; i < held; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(deck.GetInventory().GetAttachmentFromIndex(i));
            if (!card)
                continue;
            if (faces.Find(card.OZG_Face()) < 0)
                faces.Insert(card.OZG_Face());
            int at = looks.Find(card.OZG_Look());
            if (at < 0)
            {
                looks.Insert(card.OZG_Look());
                counts.Insert(1);
            }
            else
            {
                counts[at] = counts[at] + 1;
            }
        }
        s = s + " faces=" + faces.Count().ToString() + " looks=";
        for (int k = 0; k < looks.Count(); k++)
        {
            if (k > 0)
                s = s + ",";
            s = s + looks[k].ToString() + ":" + counts[k].ToString();
        }
        return s;
    }

    // A card out of a deck for each face named, and gone; a face named twice
    // takes two cards. Returns how many were found.
    static int PullFrom(OZG_Deck_Base deck, string faces)
    {
        array<string> wanted = new array<string>();
        OZG_Cards.Parse(faces, wanted);
        int gone = 0;
        int held = deck.GetInventory().AttachmentCount();
        for (int i = 0; i < held; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(deck.GetInventory().GetAttachmentFromIndex(i));
            if (!card)
                continue;
            int at = wanted.Find(card.OZG_Face());
            if (at < 0)
                continue;
            wanted.Remove(at);
            GetGame().ObjectDelete(card);
            gone++;
        }
        return gone;
    }

    // cards who=: the deck a player carries, counted.
    static bool OpCards(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        OZG_Deck_Base deck = DeckOf(p);
        if (!deck)
        {
            detail = "cards: '" + uid + "' carries no deck";
            return true;
        }
        detail = DeckLine(deck);
        return true;
    }

    // pull who= face="<face> [<face> ..]": a card of each face out of the deck
    // a player carries, and gone.
    static bool OpPull(map<string, string> args, out string detail)
    {
        string faces = OZG_VerbOps.Arg(args, "face", "As");
        string uid;
        PlayerBase p = Who(args, uid);
        OZG_Deck_Base deck = DeckOf(p);
        if (!deck)
        {
            detail = "pull: '" + uid + "' carries no deck";
            return true;
        }
        detail = "pulled " + PullFrom(deck, faces).ToString() + " of '" + faces + "'; a tick later: cards";
        return true;
    }

    // undeck who=: every deck a player carries is deleted, with its cards.
    static bool OpUndeck(map<string, string> args, out string detail)
    {
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p)
        {
            detail = "undeck: nobody is '" + uid + "'";
            return false;
        }
        array<EntityAI> items = new array<EntityAI>();
        p.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
        EntityAI held = p.GetHumanInventory().GetEntityInHands();
        if (held && items.Find(held) < 0)
            items.Insert(held);
        int gone = 0;
        for (int i = 0; i < items.Count(); i++)
        {
            if (items[i] && items[i].IsInherited(OZG_Deck_Base))
            {
                GetGame().ObjectDelete(items[i]);
                gone++;
            }
        }
        detail = "undeck: " + gone.ToString() + " deck(s) of " + uid + " deleted";
        return true;
    }

```

In `OZG_Verb.c`, before `// -- ops of the later phases go above this line --`, add:

```c
        if (op == "cards")
            return OZG_WorldOps.OpCards(args, detail);
        if (op == "pull")
            return OZG_WorldOps.OpPull(args, detail);
        if (op == "undeck")
            return OZG_WorldOps.OpUndeck(args, detail);
```

and add `level, cards, pull, undeck` to `KnownOps()`.

In `OZG_Bots.c`, after the two lines `if (arg == "zone")` / `deck = "OZG_Deck_Zone";`, add:

```c
            if (arg == "sharp")
                deck = "OZG_Deck_Classic_Sharp";
            if (arg == "zonesharp")
                deck = "OZG_Deck_Zone_Sharp";
```

and in the help line `"!ozg deck [zone] | table | ..."` make it `"!ozg deck [zone|sharp|zonesharp] | table | ..."`.

- [ ] **Step 7: Lint, a stand cycle with a client, the checks**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`, `oz_games {"op":"undeck","who":"me"}` (a deck of an older build is empty and would mislead every check).

```
oz_games {"op":"give","who":"me","deck":"OZG_Deck_Classic"}
```
then, a second later (the deck fills itself a moment after it is made):
```
oz_games {"op":"cards","who":"me"}
```
Expected: `deck OZG_Deck_Classic slots=52 cards=52 free=0 faces=52 looks=0:52`. Fifty-two cards went into slots that every card names: the "first free slot" of the design is measured here.

```
oz_games {"op":"undeck","who":"me"}
oz_games {"op":"give","who":"me","deck":"OZG_Deck_Zone_Sharp"}
```
a second later:
```
oz_games {"op":"cards","who":"me"}
```
Expected: `deck OZG_Deck_Zone_Sharp slots=62 cards=52 free=10 faces=52 looks=1:52`.

```
oz_games {"op":"pull","who":"me","face":"As"}
oz_games {"op":"cards","who":"me"}
```
Expected: `pulled 1 of 'As'; ...`, then `... cards=51 free=11 faces=51 looks=1:51`.

Open the inventory on the client (`client_key`, the Tab key) and `client_shot`: what a deck in the hands shows of its cards is written into the record as it is seen (the slots as icons, or nothing until the deck is looked at). This is the owner's to judge, not a pass or a fail of this step.

Stand cycle (no build), client:

```
oz_games {"op":"cards","who":"me"}
```
Expected: `... cards=51 free=11 faces=51 looks=1:51` (kept through the restart, the looks with it, and not filled again).

- [ ] **Step 8: Commit**

```bash
git add tools OpenZone_Games OpenZone_Games_Bridge docs/examples
git commit -m "Cards are things: fifty-two classes, sixty-two slots, two kinds of deck

A deck holds its cards in slots that take any card; a sharp's deck has ten
more, empty. A new deck fills itself once with the cards of its own look; one
that comes back from storage is left as it is. The slots, the card classes,
the cards' names and their lines for the economy are written by
tools/make_card_items.py.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 4: The place keeps the deck

**Files:**
- Modify: `OpenZone_Games/config.cpp` (`OZG_Place`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Actions.c:57-92` (`OZG_ActionLayOut`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c` (`OZG_Deck_Base.OnPlacementComplete`, `OZG_Place`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Sessions.c` (`LayOut`, `GatherAction`, `Fold`, new `ReturnDeck`, `Unlay`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`AwaitDeck`, `Tick`, `Deal`)
- Modify: `OpenZone_Games_Bridge/.../OZG_WorldOps.c` (`OpLay`, new `OpHeld`), `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_Deck_Base.OZG_Spare(bool)`, `.OZG_FillNow()` (Task 3); `OZG_WorldOps.DeckLine` (Task 3); what Task 1 measured.
- Produces: `OZG_Place.OZG_Deck() : OZG_Deck_Base`, `.OZG_Open()`, `.OZG_TakeDeck(PlayerBase from, OZG_Deck_Base deck)`, `.OZG_MakeDeck(string deckClass) : OZG_Deck_Base`, `.OZG_Release() : OZG_Deck_Base`; `OZG_Sessions.LayOut(PlayerBase player, OZG_Deck_Base deck, vector position, vector orientation)`, `OZG_Sessions.Unlay(OZG_Session s, PlayerBase placer)`; `OZG_Session.AwaitDeck(PlayerBase by)`; `OZG_Const.OPEN_MS`, `.DECK_WAIT`.

- [ ] **Step 1: The place's slot and its holding; two numbers**

In `config.cpp`, `class OZG_Place`, after `weight = 400;`:

```cpp
        // The deck of the laid-out game, and room for cards the deck has no
        // slot for. Neither is a player's to open (OZG_Place in the script).
        attachments[] = {"OZG_DeckHold"};
        itemsCargoSize[] = {10, 3};
```

In `OZG_Const.c`, after the three constants Task 3 added:

```c
    // How long a place stays open to the session's own hand, ms.
    static const int OPEN_MS = 5000;
    // How long a place laid out by hand waits for its deck, seconds of the
    // mod's clock. Shorter than OPEN_MS.
    static const int DECK_WAIT = 4;
```

- [ ] **Step 2: The action never deletes the deck**

In `OZG_Actions.c`, `class OZG_ActionLayOut`, after `HandleReciveData`, add:

```c

    // The game deletes a kit it has placed, here (actiondeployobject.c:230).
    // A deck is not spent by being laid out: it is on its way into the place,
    // or still in the placer's hands when the server refused the spot. For
    // the length of this one call it is no kit.
    override void OnEndServer(ActionData action_data)
    {
        OZG_Deck_Base deck;
        if (action_data)
            deck = OZG_Deck_Base.Cast(action_data.m_MainItem);
        if (deck)
            deck.OZG_Spare(true);
        super.OnEndServer(action_data);
        if (deck)
            deck.OZG_Spare(false);
    }
```

- [ ] **Step 3: The place takes, makes and releases a deck**

In `OZG_Items.c`, `class OZG_Place`: beside `protected bool m_OZG_Loaded;` add

```c
    protected int m_OZG_OpenUntil;
```

and after `CanPutIntoHands` (the end of the section "not an item") add:

```c
    // ------------------------------------------------------------ the deck it holds

    // Server: for a moment the session's own hand may put things in and take
    // them out.
    void OZG_Open()
    {
        m_OZG_OpenUntil = GetGame().GetTime() + OZG_Const.OPEN_MS;
    }

    // Whether a SERVER says no. A client never does: the deck comes out of
    // the placer's hands by an event his own client has to agree with
    // (hand_guards.c:302 asks LocationCanMoveEntity on both sides), and a
    // client that refused would go on believing the deck is in its hands.
    // What keeps players out on a client is that the place shows no
    // inventory at all (IsInventoryVisible, DisableVicinityIcon above); what
    // keeps a dishonest client out is this.
    protected bool OZG_Shut()
    {
        if (!GetGame() || !GetGame().IsServer())
            return false;
        return GetGame().GetTime() >= m_OZG_OpenUntil;
    }

    override bool CanReceiveAttachment(EntityAI attachment, int slotId)
    {
        if (OZG_Shut())
            return false;
        if (!attachment || !attachment.IsInherited(OZG_Deck_Base))
            return false;
        return super.CanReceiveAttachment(attachment, slotId);
    }

    override bool CanReleaseAttachment(EntityAI attachment)
    {
        if (OZG_Shut())
            return false;
        return super.CanReleaseAttachment(attachment);
    }

    override bool CanReceiveItemIntoCargo(EntityAI item)
    {
        if (OZG_Shut())
            return false;
        if (!item || !item.IsInherited(OZG_Card_Base))
            return false;
        return super.CanReceiveItemIntoCargo(item);
    }

    override bool CanReleaseCargo(EntityAI cargo)
    {
        if (OZG_Shut())
            return false;
        return super.CanReleaseCargo(cargo);
    }

    OZG_Deck_Base OZG_Deck()
    {
        return OZG_Deck_Base.Cast(GetInventory().FindAttachmentByName(OZG_Const.SLOT_HOLD));
    }

    // Server: the deck is asked out of the placer's hands into the place, as
    // the game's indoor fireplace takes its first stick
    // (actioncreateindoorfireplace.c:95). The hands answer a tick or more
    // later, and the call's own answer says nothing (humaninventory.c:228):
    // whoever asks looks again (OZG_Session.AwaitDeck).
    void OZG_TakeDeck(PlayerBase from, OZG_Deck_Base deck)
    {
        if (!from || !deck)
            return;
        OZG_Open();
        from.ServerTakeEntityToTargetAttachmentEx(this, deck, InventorySlots.GetSlotIdFromString(OZG_Const.SLOT_HOLD));
    }

    // Server: a full deck of a class made inside the place: for a place made
    // by script, and for one that lost its deck.
    OZG_Deck_Base OZG_MakeDeck(string deckClass)
    {
        OZG_Deck_Base deck = OZG_Deck();
        if (deck)
            return deck;
        if (deckClass == "")
            deckClass = OZG_Decks.FirstClass();
        OZG_Open();
        deck = OZG_Deck_Base.Cast(GetInventory().CreateAttachment(deckClass));
        if (deck)
            deck.OZG_FillNow();
        else
            OZG_Log.Error("place " + OZG_Id() + " could not be given a deck " + deckClass);
        return deck;
    }

    // Server: the deck out of the place onto the spot where the place lies,
    // at once (inventory.c:1250, a move between two things that are no
    // player's hands). Null when the place holds none.
    OZG_Deck_Base OZG_Release()
    {
        OZG_Deck_Base deck = OZG_Deck();
        if (!deck)
            return null;
        OZG_Open();
        vector m4[4];
        GetTransform(m4);
        if (!GetInventory().DropEntityWithTransform(InventoryMode.SERVER, this, deck, m4))
        {
            OZG_Log.Error("the deck of place " + OZG_Id() + " did not leave it");
            return null;
        }
        return deck;
    }
```

- [ ] **Step 4: Laying out keeps the deck**

In `OZG_Items.c`, `OZG_Deck_Base.OnPlacementComplete`, replace the call

```c
        OZG_Sessions.LayOut(PlayerBase.Cast(player), GetType(), position, orientation);
```

with

```c
        OZG_Sessions.LayOut(PlayerBase.Cast(player), this, position, orientation);
```

In `OZG_Session.c`: beside `protected string m_Note;` add

```c
    // Laid out by hand: the second of the mod's clock by which the deck must
    // have come out of the placer's hands (0: nothing is waited for), and who
    // placed it.
    protected int m_DeckDue;
    protected string m_DeckUid;
```

after `SetHost` add

```c
    // Server: the deck was asked out of the placer's hands.
    void AwaitDeck(PlayerBase by)
    {
        m_DeckDue = OZG_Sessions.Now() + OZG_Const.DECK_WAIT;
        m_DeckUid = OZG_Who.Uid(by);
    }

```

in `Tick`, after its first two lines (`if (!m_Place)` / `return;`):

```c
        // A place the deck has not reached in time is no game.
        if (m_DeckDue > 0)
        {
            if (m_Place.OZG_Deck())
            {
                m_DeckDue = 0;
            }
            else if (OZG_Sessions.Now() >= m_DeckDue)
            {
                OZG_Sessions.Unlay(this, OZG_Sessions.PlayerOf(m_DeckUid));
                return;
            }
        }
```

and in `Deal`, after its first check (`if (m_R.St == OZG_Const.ST_PLAYING)` / `return OZG_Const.SAY_PLAYING;`):

```c
        if (m_DeckDue > 0)
            return OZG_Const.SAY_NOT_NOW;
```

In `OZG_Sessions.c`:

1. The signature and the first lines of `LayOut` become:

```c
    // The deploy completed: the server decides again and makes the place.
    static void LayOut(PlayerBase player, OZG_Deck_Base deck, vector position, vector orientation)
    {
        if (!player || !deck)
            return;
        string deckClass = deck.GetType();
```

2. Both refusals stop giving a new deck. Replace

```c
            // The action deletes the kit it placed: the player gets another.
            GiveDeck(null, deckClass, player.GetPosition());
            Tell(player, OZG_Const.NOTICE_LAY, why);
            return;
```

   with

```c
            // The deck is still in his hands: the action that placed it does
            // not delete a deck (OZG_ActionLayOut.OnEndServer).
            Tell(player, OZG_Const.NOTICE_LAY, why);
            return;
```

   and in the `if (!s)` branch below delete the line `GiveDeck(null, deckClass, player.GetPosition());`.

3. After that `if (!s)` branch and before the comment `// The placer is the host`, add:

```c
        // The place asks for the deck, and what follows does not wait for it.
        s.m_Place.OZG_TakeDeck(player, deck);
        s.AwaitDeck(player);

```

4. After `Make` (before the section comment `// ---- the actions on a place`), add:

```c
    // A place its deck never reached is no game: it goes, and no deck is
    // given, because the deck never left its owner.
    static void Unlay(OZG_Session s, PlayerBase placer)
    {
        if (!s)
            return;
        string id = s.Id();
        OZG_Place place = s.m_Place;
        s.CloseAll(OZG_Const.SAY_CANNOT);
        s.DropFans();
        DeleteFile(OZG_Session.PathOf(id));
        // Out of the table first: deleting the place looks for its session.
        s.m_Place = null;
        s_ById.Remove(id);
        if (place)
            GetGame().ObjectDelete(place);
        Tell(placer, OZG_Const.NOTICE_LAY, OZG_Const.SAY_CANNOT);
        OZG_Log.Warn("place " + id + " was unlaid: its deck did not reach it");
    }

    // The deck goes back. It leaves the place for the ground where the place
    // lies, at once, so that the place can be deleted; then free hands are
    // asked to take it up (man.c:197; the hands answer a tick later, and a
    // deck they do not take lies where the place lay). A place that holds no
    // deck gives a new one of its class.
    protected static void ReturnDeck(OZG_Place place, PlayerBase to, string deckClass, vector at)
    {
        OZG_Deck_Base deck;
        if (place)
            deck = place.OZG_Release();
        if (!deck)
        {
            GiveDeck(to, deckClass, at);
            return;
        }
        if (to && to.IsAlive() && !to.GetItemInHands())
            to.ServerTakeEntityToHands(deck);
    }

```

5. In `GatherAction`, the branch for a place with no session becomes:

```c
            // A place with no session is a mat and a deck: it folds.
            if (place)
            {
                string deck = place.OZG_DeckClass();
                vector at = place.GetPosition();
                // The deck is out before the place goes: a thing deleted
                // takes what it holds with it.
                ReturnDeck(place, player, deck, at);
                GetGame().ObjectDelete(place);
            }
            return;
```

6. In `Fold`, replace

```c
        if (place)
            GetGame().ObjectDelete(place);
        GiveDeck(by, deck, at);
```

   with

```c
        // The deck is out before the place goes: a thing deleted takes what
        // it holds with it.
        ReturnDeck(place, by, deck, at);
        if (place)
            GetGame().ObjectDelete(place);
```

   and change the comment above `GiveDeck` to `// A new deck of a class: into free hands, else onto the spot.`

- [ ] **Step 5: A place made by script gets its deck; an op to look into a place**

In `OZG_WorldOps.c`, `OpLay`, after the `if (!s)` branch and before `s.Changed();`:

```c
        // No hands lay this one out: its deck is made in it.
        s.m_Place.OZG_MakeDeck(deck);
```

Before `// give who= [deck=class]` add (and route `held` in `OZG_Verb.c` like `cards`, and add it to `KnownOps()`):

```c
    // held id=: what the place holds: its deck, and the cards beside it.
    static bool OpHeld(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int beside = 0;
        CargoBase cargo = s.m_Place.GetInventory().GetCargo();
        if (cargo)
            beside = cargo.GetItemCount();
        OZG_Deck_Base deck = s.m_Place.OZG_Deck();
        if (!deck)
        {
            detail = "held: no deck; beside=" + beside.ToString();
            return true;
        }
        detail = "held: " + DeckLine(deck) + " beside=" + beside.ToString();
        return true;
    }

```

- [ ] **Step 6: Lint, a stand cycle with a client, the checks**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`.

First the world's old places. `oz_games {"op":"places"}`; a place saved before this task holds no deck. For each: `oz_games {"op":"gather","id":"0","who":"me"}` (when it is busy: `oz_games {"op":"sess","id":"0"}` names the host's seat, and its player asks `fold`; a place whose players are gone folds by `ask who=me ask=host` after `sit who=me seat=<a free one>`, then `ask who=me ask=fold`). Expected at the end: `sessions=0 places=0`; each fold gave a new deck of the place's class (the second half of `ReturnDeck`): `oz_games {"op":"undeck","who":"me"}`, and `world_delete` the ones on the ground.

By script:

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2"}
oz_games {"op":"held","id":"0"}
```
Expected: `laid <id> seats=...`, then `held: deck OZG_Deck_Classic slots=52 cards=52 free=0 faces=52 looks=0:52 beside=0`.

```
oz_games {"op":"gather","id":"0","who":"me"}
```
Expected: `gather by <id>; sessions=0 places=0 ...`. Two seconds later `oz_games {"op":"cards","who":"me"}`. Expected: `deck OZG_Deck_Classic slots=52 cards=52 ...` (the deck came off the ground into the hands; when it says the player carries no deck, `world_entities` for `OZG_Deck_Classic` near the spot shows where it lies, and the record says which of the two happened).

By hand of the client, with that deck in the hands: `oz_games {"op":"pull","who":"me","face":"As"}` (so that this deck can be told from a new one), then `world_teleport "4699.9 339.02 10196.0"`, `tgp {"op":"ask","do":"aimplace","text":"4701.5 339.0 10196.0"}`, `client_move(0, 0, 1)`, `client_trigger(which="right", seconds=6)`, `tgp {"op":"ask","do":"aim","text":""}`. Two seconds later:

```
oz_games {"op":"held","id":"0"}
```
Expected: `held: deck OZG_Deck_Classic slots=52 cards=51 free=1 faces=51 looks=0:51 beside=0`: the placer's own deck, short of its ace. `client_shot`: the mat lies there and the hands are empty. `log_tail(pattern="unlaid", n=600)`: nothing.

Do not run `ticks` in the four seconds after a hand's laying out: the op runs the mod's clock forward, and the wait for the deck is counted on that clock.

Stand cycle (no build), client: `oz_games {"op":"held","id":"0"}`. Expected: the same line (a place that is shut took its deck back out of the world's storage). Then `oz_games {"op":"gather","id":"0","who":"me"}` and, two seconds later, `oz_games {"op":"cards","who":"me"}`. Expected: `cards=51`.

The spot refused by the server (another place within two metres, or no room for two seats) cannot be reached through the client, which refuses first; it is checked by reading: `LayOut`'s two refusals make no deck, and `OZG_ActionLayOut.OnEndServer` spares the one in the hands.

- [ ] **Step 7: Commit**

```bash
git add OpenZone_Games OpenZone_Games_Bridge
git commit -m "A laid-out game keeps its deck

The deck is no longer deleted and remembered as a class: the place asks it out
of the placer's hands, holds it with its cards, and gives it back when the
game is gathered or folded. A place is shut to players on the server and shows
no inventory on a client. A place made by script gets a deck of its class; a
place its deck never reached is unlaid.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 5: A deal is dealt from what the deck holds

**Files:**
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c` (add `OZG_DeckRead`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`Deal`)
- Modify: `OpenZone_Games_Bridge/.../OZG_WorldOps.c`, `OZG_Verb.c` (op `stuff`; `pull` for a place)

**Interfaces:**
- Consumes: `OZG_Place.OZG_Deck()`, `.OZG_MakeDeck`, `.OZG_Open` (Task 4); `OZG_Game.TakeCards`, `OZG_Cards.CopyNamed`, `.FaceOf`, `OZG_DealCard` (Task 2); `OZG_Card_Base.OZG_Face()`, `.OZG_Look()`, `.OZG_SetStyle` (Task 3); `OZG_WorldOps.DeckOf`, `.PullFrom` (Task 3).
- Produces: `OZG_DeckRead.ThingsOf(OZG_Place place, array<EntityAI> into)` (every card item a place holds: the deck's in the order of its slots, then those beside it); `OZG_DeckRead.CardsOf(OZG_Place place, array<ref OZG_DealCard> into) : int` (the same cards as the records a game is handed: a name each, however many of a face there are, and the look of the back; the answer is how many cards there are).

- [ ] **Step 1: Reading a place's cards**

Append to `OZG_CardThings.c`:

```c
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
```

- [ ] **Step 2: The deal reads the deck**

In `OZG_Session.c`, `Deal`, before `string refusal;`:

```c
        // The cards of this deal are the cards the deck holds that are of the
        // game's set, however many of a face there are. A place that lost its
        // deck gets one of its class.
        if (m_Place && !m_Place.OZG_Deck())
            m_Place.OZG_MakeDeck(m_R.DeckClass);
        array<ref OZG_DealCard> all = new array<ref OZG_DealCard>();
        OZG_DeckRead.CardsOf(m_Place, all);
        array<string> ofGame = new array<string>();
        game.CardsUsed(ofGame);
        array<ref OZG_DealCard> cards = new array<ref OZG_DealCard>();
        for (i = 0; i < all.Count(); i++)
        {
            if (ofGame.Find(OZG_Cards.FaceOf(all[i].Name)) >= 0)
                cards.Insert(all[i]);
        }
        game.TakeCards(cards);

```

- [ ] **Step 3: Ops to change a laid-out deck**

In `OZG_WorldOps.c`, replace `OpPull` with a version that takes a session's deck when `id=` is given:

```c
    // pull who= face="<face> [<face> ..]", or pull id= face=..: a card of each
    // face out of the deck a player carries, or out of a laid-out game's deck,
    // and gone.
    static bool OpPull(map<string, string> args, out string detail)
    {
        string faces = OZG_VerbOps.Arg(args, "face", "As");
        OZG_Deck_Base deck;
        if (OZG_VerbOps.Arg(args, "id", "") != "")
        {
            OZG_Session s = Sess(args, detail);
            if (!s)
                return false;
            deck = s.m_Place.OZG_Deck();
        }
        else
        {
            string uid;
            PlayerBase p = Who(args, uid);
            deck = DeckOf(p);
        }
        if (!deck)
        {
            detail = "pull: no deck there";
            return true;
        }
        detail = "pulled " + PullFrom(deck, faces).ToString() + " of '" + faces + "'; a tick later: cards or held";
        return true;
    }

    // stuff id= face=<face> [style=classic] [n=1]: cards of a face put into a
    // laid-out game: into the deck's free slots, then beside it.
    static bool OpStuff(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string face = OZG_VerbOps.Arg(args, "face", "As");
        string style = OZG_VerbOps.Arg(args, "style", "classic");
        int n = OZG_VerbOps.Arg(args, "n", "1").ToInt();
        OZG_Deck_Base deck = s.m_Place.OZG_Deck();
        int inDeck = 0;
        int beside = 0;
        for (int i = 0; i < n; i++)
        {
            OZG_Card_Base card;
            if (deck && deck.OZG_Free() > 0)
                card = OZG_Card_Base.Cast(deck.GetInventory().CreateAttachment(OZG_Const.CARD_PREFIX + face));
            if (card)
            {
                inDeck++;
            }
            else
            {
                s.m_Place.OZG_Open();
                card = OZG_Card_Base.Cast(s.m_Place.GetInventory().CreateEntityInCargo(OZG_Const.CARD_PREFIX + face));
                if (card)
                    beside++;
            }
            if (card)
                card.OZG_SetStyle(style);
        }
        detail = "stuffed " + face + ": deck=" + inDeck.ToString() + " beside=" + beside.ToString();
        return true;
    }

```

Route `stuff` in `OZG_Verb.c` and add it to `KnownOps()`.

- [ ] **Step 4: Lint, a stand cycle with a client, the checks**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`; gather what the world holds (`places`, `gather`), `undeck`.

**A whole deck plays as before:**

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2"}
oz_games {"op":"puppet","id":"0","seat":"0"}
oz_games {"op":"puppet","id":"0","seat":"1"}
```
The two answers name the puppets (`puppet1 made at seat 0 sat=1`): use the first one's name where this plan says `puppet1`.
```
oz_games {"op":"ask","id":"0","who":"puppet1","ask":"host"}
oz_games {"op":"ask","id":"0","who":"puppet1","ask":"deal"}
```
Expected of the last: `ok; session <id> st=1 ... | durak podkidnoy trump=.. stock=24 ...`.
```
oz_games {"op":"auto","id":"0","n":"400"}
oz_games {"op":"sess","id":"0"}
```
Expected: `st=2` (the game was played to its end); `log_tail(pattern="ERROR", n=600)`: nothing of this mod.

**A deck short of an ace deals and plays:**

```
oz_games {"op":"pull","id":"0","face":"As"}
oz_games {"op":"ask","id":"0","who":"puppet1","ask":"deal"}
```
Expected: `pulled 1 of 'As'; ...`, then `stock=23`. Then `auto` and `sess` as above: `st=2`.

**A sharp's deck with a second ace deals it.** `oz_games {"op":"ask","id":"0","who":"puppet1","ask":"fold"}` (the deck lies where the place lay: `world_delete` it), then:

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2","deck":"OZG_Deck_Classic_Sharp"}
oz_games {"op":"stuff","id":"0","face":"As","style":"zone"}
oz_games {"op":"held","id":"0"}
```
Expected: `stuffed As: deck=1 beside=0`, then `held: deck OZG_Deck_Classic_Sharp slots=62 cards=53 free=9 faces=52 looks=0:52,1:1 beside=0`. Two new puppets, host, deal. Expected: `stock=25` (thirty-seven cards). `auto`, `sess`: `st=2`, no error: a copy plays.

**A deck of aces alone** (the owner's own example). After that game: every card of the game's set leaves the deck but the aces of spades, and aces of spades take their place:

```
oz_games {"op":"pull","id":"0","face":"6s 7s 8s 9s Ts Js Qs Ks 6h 7h 8h 9h Th Jh Qh Kh Ah 6d 7d 8d 9d Td Jd Qd Kd Ad 6c 7c 8c 9c Tc Jc Qc Kc Ac"}
```
a tick later
```
oz_games {"op":"stuff","id":"0","face":"As","n":"35"}
oz_games {"op":"held","id":"0"}
```
Expected: `pulled 35 of ...`; `stuffed As: deck=35 beside=0`; `held: deck OZG_Deck_Classic_Sharp slots=62 cards=53 free=9 faces=17 looks=0:52,1:1 beside=0` (the sixteen low cards and thirty-seven aces of spades). Deal. Expected: `trump=As` or `trump=As.<n>`, and `stock=25`. `auto`, `sess`: `st=2`: thirty-seven aces of spades are a game of durak, and it ends. `oz_games {"op":"hand","id":"0","seat":"0"}` during it shows names like `As.4`, `As.17`: every one its own.

**Too few.** After that game twenty-five of the aces go (a face named twice takes two cards):

```
oz_games {"op":"pull","id":"0","face":"As As As As As As As As As As As As As As As As As As As As As As As As As"}
oz_games {"op":"held","id":"0"}
```
Expected: `pulled 25 of ...`, then `cards=28` (the sixteen low cards and twelve aces). Deal. Expected: `stock=0`. `auto` to the end, then `oz_games {"op":"pull","id":"0","face":"As"}` and deal. Expected: `refused STR_OZG_NO_CARDS` (eleven cards for two hands of six).

**A restart in mid-game.** Fold, `world_delete` the deck, lay an ordinary deck, two puppets, host, deal; write down the `trump=`, `stock=` and `hands=` of `sess`. Stand cycle (no build), client. `oz_games {"op":"sess","id":"0"}`. Expected: `st=1` and the same three. (The puppets are gone after a restart and their seats are kept: that is the session's rule, not this task's.)

- [ ] **Step 5: Commit**

```bash
git add OpenZone_Games OpenZone_Games_Bridge
git commit -m "A deal is dealt from the cards the deck holds

The session reads the place's deck at every deal and hands the game the cards
of its set that are there: a deck short of a card deals without it, a second
card of a face is dealt as its copy. Nobody is told.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 6: The window draws every card as it is

**Files:**
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c` (`SetPicture` at 425, `MovesOf` at 496, `PaintSeats` at 606, `MakeCard` at 712, `PaintTable` at 723, `PaintHand` at 812, `LogWord` at 945; the numbers are of the file before this task)

**Interfaces:**
- Consumes: `OZG_DurakSeatPublic.Backs`, `OZG_DurakPublic.TrumpLook`, `.StockTop`, `.AttackLooks`, `.DefenceLooks`, `OZG_DurakPrivate.HeldLooks` (Task 2); `OZG_Decks.StyleAt` (Task 3); `OZG_Cards.FaceOf`, `.SuitOf`, `.OfMove` (Task 2).

- [ ] **Step 1: A picture by look**

After `SetPicture`, add:

```c
    // A picture in the art of a look: a card brought from another deck is
    // drawn as it is. A look of -1 (none said) is the deck's own.
    protected string PictureOf(string name, int look)
    {
        if (look < 0)
            return Picture(name);
        return "set:ozg_cards_" + OZG_Decks.StyleAt(look) + " image:" + name;
    }

    protected void SetPictureOf(ImageWidget img, string name, int look)
    {
        if (!img)
            return;
        img.LoadImageFile(0, PictureOf(name, look));
        img.SetImage(0);
    }

    protected int LookAt(array<int> looks, int at)
    {
        if (!looks || at < 0 || at >= looks.Count())
            return -1;
        return looks[at];
    }
```

- [ ] **Step 2: `MakeCard` takes the look**

`MakeCard(Widget parent, string picture, float x, float y, float w, float h)` becomes `MakeCard(Widget parent, string picture, int look, float x, float y, float w, float h)`, and its line

```c
        SetPicture(ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace")), picture);
```

becomes

```c
        SetPictureOf(ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace")), picture, look);
```

- [ ] **Step 3: Every caller says the face and the look; nothing cuts a name by position**

| Now | Becomes |
|---|---|
| `SetPicture(back, "back");` (in `PaintSeats`) | `SetPictureOf(back, "back", LookAt(g.Backs, 0));` |
| `SetPicture(m_Trump, "c_" + m_Pub.TrumpCard);` | `SetPictureOf(m_Trump, "c_" + OZG_Cards.FaceOf(m_Pub.TrumpCard), m_Pub.TrumpLook);` |
| `SetPicture(m_StockBack, "back");` | `SetPictureOf(m_StockBack, "back", m_Pub.StockTop);` |
| `MakeCard(m_Bout, "c_" + m_Pub.AttackCards[i], i * pitch, 16, TABLE_W, TABLE_H)` | `MakeCard(m_Bout, "c_" + OZG_Cards.FaceOf(m_Pub.AttackCards[i]), LookAt(m_Pub.AttackLooks, i), i * pitch, 16, TABLE_W, TABLE_H)` |
| `MakeCard(m_Bout, "c_" + beaten, i * pitch + 30, 70, TABLE_W, TABLE_H)` | `MakeCard(m_Bout, "c_" + OZG_Cards.FaceOf(beaten), LookAt(m_Pub.DefenceLooks, i), i * pitch + 30, 70, TABLE_W, TABLE_H)` |
| `MakeCard(m_HandBox, "c_" + card, x0 + i * step, y, HAND_W, HAND_H)` | `MakeCard(m_HandBox, "c_" + OZG_Cards.FaceOf(card), LookAt(m_Own.HeldLooks, m_Own.Held.Find(card)), x0 + i * step, y, HAND_W, HAND_H)` |

The hand is drawn sorted, so a card's look is found by the card, not by its place in the row: that is what `m_Own.Held.Find(card)` is for.

In `PaintSeats`, after `cards = g.CardCount.ToString();`:

```c
                // Backs of more than one look in a hand are there for all to
                // see: schematic for now, a number beside the count.
                int strange = 0;
                for (int bk = 1; bk < g.Backs.Count(); bk++)
                {
                    if (g.Backs[bk] != g.Backs[0])
                        strange++;
                }
                if (strange > 0)
                    cards = cards + " (" + strange.ToString() + "*)";
```

A card's name has no fixed width now (`As`, `As.1`, `As.12`). Three places took it for two characters:

`MovesOf` found a card's moves by looking for its name inside a move, and `As` is inside `a:As.1`. Replace

```c
            if (move.IndexOf(":" + card) >= 0)
                into.Insert(move);
```

with

```c
            if (OZG_Cards.OfMove(move) == card)
                into.Insert(move);
```

`PaintHand` took the trump's suit only from a name of two characters. Replace

```c
        if (m_Pub && m_Pub.TrumpCard.Length() == 2)
            trumpSuit = m_Pub.TrumpCard.Substring(1, 1);
```

with

```c
        if (m_Pub)
            trumpSuit = OZG_Cards.SuitOf(m_Pub.TrumpCard);
```

`LogWord` cut the card out of a move by position. Replace

```c
        if (rest.Length() >= 4)
        {
            string kind = rest.Substring(0, 1);
            string card = rest.Substring(2, 2);
```

with

```c
        string card = OZG_Cards.OfMove(rest);
        if (card != "")
        {
            string kind = rest.Substring(0, 1);
```

`CardWord`, `SortKey` and the trump's suit picture in `PaintTable` take a name's first two characters, which is right for a name of any length: they stay as they are. The log words a card by its face alone, so nobody reads from the log which of several aces was played.

Run Grep in `OpenZone_Games/scripts/5_Mission`: for `"c_" +` (every hit is followed by `OZG_Cards.FaceOf(`), for `Length() == 2` (no hit), for `IndexOf(":" +` (no hit), for `Substring(2, 2)` (no hit).

- [ ] **Step 4: Lint, a stand cycle with a client, the picture**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`.

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2","deck":"OZG_Deck_Classic_Sharp"}
oz_games {"op":"stuff","id":"0","face":"As","style":"zone","n":"3"}
oz_games {"op":"sit","id":"0","who":"me","seat":"0"}
oz_games {"op":"sit","id":"0","who":"me"}
oz_games {"op":"bots","id":"0","n":"1","on":"0"}
oz_games {"op":"ask","id":"0","who":"me","ask":"host"}
oz_games {"op":"ask","id":"0","who":"me","ask":"deal"}
```
(The second `sit`, with no seat, is the action on the place for somebody already seated: it opens the window.)

`client_shot`: the window as it was for a clean deck, the trump and the stock drawn. `oz_games {"op":"hand","id":"0","seat":"0"}` says the hand: when it holds one of `As.1`, `As.2`, `As.3` (the aces of the worn look; `As` itself is the deck's own), the shot shows one ace in the worn art among the classic ones. When it does not, end the game (`auto`) and deal again: three strange aces among thirty-nine cards come into a hand of six within a few deals. Send the shot with the strange ace, and one where the bot's seat says `(1*)`. When the hand holds two aces of spades at once, click one of them: that one alone rises, and the move goes through with it (`sess` names the card on the table, `As` or `As.<n>`).

- [ ] **Step 5: Commit**

```bash
git add OpenZone_Games
git commit -m "The window draws a card with its own back and its own face

A card brought from a deck of another look is drawn in that look's art, and a
hand that holds backs of two looks says how many are strange. A copy of a face
is drawn as the face.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 7: Counting the deck

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c:94-111`, `OZG_Wire.c:87`, `OZG_Settings.c:49-96`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`Ask`, `Tick`, `Deal`, new `CountDone`)
- Modify: `tools/make_layouts.py:159-172`, then the layout it writes
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c`, `OZG_MissionGameplay.c:137`
- Modify: `OpenZone_Games/strings.csv`, then the string table

**Interfaces:**
- Consumes: `OZG_DeckRead.CardsOf` (Task 5); `OZG_Session.m_DeckDue` (Task 4).
- Produces: `OZG_Const.OP_COUNT = "count"`, `OZG_Const.OP_COUNTED = "counted"`; `class OZG_WCount { string PlaceId; int Total; int Missing; int Copies; ref array<string> Names; }`; settings `CountSeconds` (6), `CountNamesCards` (0); `OZG_GameMenu.Counted(OZG_WCount c)`.

- [ ] **Step 1: Names, the answer's shape, the settings**

`OZG_Const.c`: after `static const string OP_MINE    = "mine";` add

```c
    static const string OP_COUNT   = "count";
```

and after `static const string OP_CLOSED = "closed";  // this place is no longer yours to look at`:

```c
    static const string OP_COUNTED = "counted"; // what counting the deck found, to the one who counted
```

`OZG_Wire.c`, after `class OZG_WMine`:

```c

// What counting the deck found: for the one who counted, and nobody else.
class OZG_WCount
{
    string PlaceId;
    int    Total;      // cards the deck holds
    int    Missing;    // faces of the game's set that are not there
    int    Copies;     // cards that are a second, third or further of their face
    ref array<string> Names;   // the missing faces, when the server says them

    void OZG_WCount()
    {
        Names = new array<string>();
    }
}
```

`OZG_Settings.c`: after `int   MaxSeats;` add

```c
    int   CountSeconds;     // counting the deck
    int   CountNamesCards;  // 1: a count also says which faces are missing
```

`LatestVersion()` returns `2`; in `LoadDefaults`, after `MaxSeats = 8;`:

```c
        CountSeconds = 6;
        CountNamesCards = 0;
```

`Migrate` becomes:

```c
    override bool Migrate(int from)
    {
        // 2: counting the deck.
        if (from < 2)
        {
            CountSeconds = 6;
            CountNamesCards = 0;
        }
        Version = LatestVersion();
        return true;
    }
```

and in `Validate`, after the block that puts `IdleMinutes` right:

```c
        if (CountSeconds < 1)
        {
            CountSeconds = 6;
            warnings++;
        }
        if (CountNamesCards != 0 && CountNamesCards != 1)
        {
            CountNamesCards = 0;
            warnings++;
        }
```

- [ ] **Step 2: The session counts**

`OZG_Session.c`: beside `m_DeckDue` add

```c
    // Who counts the deck, and the second of the mod's clock he is done.
    protected string m_CountUid;
    protected int m_CountAt;
```

In `Ask`, before the comment `// The rest is the host's.`:

```c
        // Counting the deck: any seated player, between deals. Everybody's log
        // says who counts; what he finds is his alone.
        if (op == OZG_Const.OP_COUNT)
        {
            if (mine < 0 || !Present(player))
                return OZG_Const.SAY_NOT_SEATED;
            if (m_R.St == OZG_Const.ST_PLAYING)
                return OZG_Const.SAY_PLAYING;
            if (m_CountUid != "" || m_DeckDue > 0)
                return OZG_Const.SAY_NOT_NOW;
            m_CountUid = uid;
            m_CountAt = OZG_Sessions.Now() + OZG_Cfg.Get().CountSeconds;
            AddLog("count " + mine.ToString());
            Changed();
            return "";
        }

```

In `Deal`, after the `m_DeckDue` check Task 4 added: `m_CountUid = "";` (a deal ends a count unfinished).

In `Tick`, after the `m_DeckDue` block Task 4 added:

```c
        if (m_CountUid != "" && OZG_Sessions.Now() >= m_CountAt)
            CountDone();
```

and, before the comment `// ---- the clock` above `Tick`, the method:

```c
    // The count is done: its answer goes to the one who counted.
    protected void CountDone()
    {
        string uid = m_CountUid;
        m_CountUid = "";
        PlayerBase p = OZG_Sessions.PlayerOf(uid);
        if (!p || SeatOf(uid) < 0)
            return;

        OZG_WCount c = new OZG_WCount();
        c.PlaceId = m_R.PlaceId;
        array<ref OZG_DealCard> cards = new array<ref OZG_DealCard>();
        c.Total = OZG_DeckRead.CardsOf(m_Place, cards);

        // The faces that are there, each once.
        array<string> faces = new array<string>();
        int i;
        for (i = 0; i < cards.Count(); i++)
        {
            string face = OZG_Cards.FaceOf(cards[i].Name);
            if (faces.Find(face) < 0)
                faces.Insert(face);
        }
        c.Copies = c.Total - faces.Count();

        array<string> ofGame = new array<string>();
        OZG_Game sample = OZG_Games.Sample(m_R.GameId);
        if (sample)
            sample.CardsUsed(ofGame);
        bool named = OZG_Cfg.Get().CountNamesCards == 1;
        for (i = 0; i < ofGame.Count(); i++)
        {
            if (faces.Find(ofGame[i]) >= 0)
                continue;
            c.Missing = c.Missing + 1;
            if (named && c.Names.Count() < 40)
                c.Names.Insert(ofGame[i]);
        }

        string body;
        string err;
        JsonFileLoader<OZG_WCount>.MakeData(c, body, err, false);
        SendTo(p, OZG_Const.OP_COUNTED, body);
    }

```

- [ ] **Step 3: The button, the log's line, the answer**

`tools/make_layouts.py`: in the list `stacked`, after `("OZG_BtnDeal", "#STR_OZG_WIN_DEAL"),` add `("OZG_BtnCount", "#STR_OZG_WIN_COUNT"),`. Run `python -I tools/make_layouts.py`, then Grep for `OZG_BtnCount` under `OpenZone_Games`: the button and its children are in `ozg_game.layout`.

`strings.csv`: after the line of `STR_OZG_WIN_DEAL` add

```
STR_OZG_WIN_COUNT,Перерахувати колоду,Count the deck
```

after the line of `STR_OZG_LOG_HOST` add

```
STR_OZG_LOG_COUNT,%1 перераховує колоду,%1 counts the deck
```

and at the end of the section `# -- what the server says`:

```
STR_OZG_COUNTED,"Карт у колоді: %1. Для цієї гри бракує: %2. Повторів: %3.","Cards in the deck: %1. Missing for this game: %2. Copies: %3."
STR_OZG_COUNTED_NAMES,Бракує: %1,Missing: %1
```

Run `python -I tools/stringtable.py OpenZone_Games`. Expected: `stringtable.csv: 216 strings`.

`OZG_GameMenu.c`:
- beside `protected Widget m_BtnDeal;` add `protected Widget m_BtnCount;`; beside `m_BtnDeal = Find("OZG_BtnDeal");` add `m_BtnCount = Find("OZG_BtnCount");`;
- in `PaintColumn`, after the block `if (host && !playing) { ... }`:

```c
        if (seated && !playing)
            shown.Insert(m_BtnCount);
```

  and after `all.Insert(m_BtnDeal);` add `all.Insert(m_BtnCount);`;
- in `OnClick`, after the branch of `m_BtnDeal`:

```c
        if (w == m_BtnCount)
        {
            Send(OZG_Const.OP_COUNT, "", -1, "", "", "", 0);
            return true;
        }
```

- in `LogWord`, after the branch of `host`:

```c
        if (head == "count")
            return Tr1("STR_OZG_LOG_COUNT", SeatName(rest.ToInt()));
```

- after `Said`, a door for the answer:

```c
    // What a count of the deck found, for this client alone.
    void Counted(OZG_WCount c)
    {
        if (!c)
            return;
        string line = Tr("STR_OZG_COUNTED");
        line.Replace("%1", c.Total.ToString());
        line.Replace("%2", c.Missing.ToString());
        line.Replace("%3", c.Copies.ToString());
        if (c.Names && c.Names.Count() > 0)
        {
            string faces = "";
            for (int i = 0; i < c.Names.Count(); i++)
            {
                if (i > 0)
                    faces = faces + ", ";
                faces = faces + CardWord(c.Names[i]);
            }
            line = line + " " + Tr1("STR_OZG_COUNTED_NAMES", faces);
        }
        Said(line);
    }
```

`OZG_MissionGameplay.c`, `OZG_SvcRes`, before `if (op == OZG_Const.OP_CLOSED)`:

```c
        if (op == OZG_Const.OP_COUNTED)
        {
            OZG_WCount tally;
            string tallyErr;
            if (!JsonFileLoader<OZG_WCount>.LoadData(json, tally, tallyErr) || !tally)
                return;
            if (menu && menu.PlaceId() == tally.PlaceId)
                menu.Counted(tally);
            return;
        }

```

- [ ] **Step 4: Lint, a stand cycle with a client, the checks**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`.

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2","deck":"OZG_Deck_Classic_Sharp"}
oz_games {"op":"pull","id":"0","face":"As"}
oz_games {"op":"stuff","id":"0","face":"Ks","style":"zone"}
oz_games {"op":"sit","id":"0","who":"me","seat":"0"}
oz_games {"op":"sit","id":"0","who":"me"}
oz_games {"op":"ask","id":"0","who":"me","ask":"count"}
```
Expected of the last: `ok; ...`. Asked again at once: `refused STR_OZG_SAY_NOT_NOW`.

```
oz_games {"op":"ticks","n":"7"}
```
`client_shot`: the window's line says `Cards in the deck: 52. Missing for this game: 1. Copies: 1.`, and the log holds `... counts the deck`. The button "Count the deck" stands in the right column and nothing overlaps (with the question of how to stand up open the column is at its longest: press "Stand up" once, `client_shot`, then close the window and open it again).

Press the button itself once (`ui_find` for `OZG_BtnCount`, `ui_click` by the path it gives), `ticks n=7`, `client_shot`: the same line again.

`oz_games {"op":"bots","id":"0","n":"1","on":"0"}`, host, deal, then `ask who=me ask=count`. Expected: `refused STR_OZG_SAY_PLAYING`.

With `CountNamesCards` 1: stop the server, set `"CountNamesCards": 1` in the stand's `profiles\OpenZone\OZ_Games_Settings.json` (the file gains the two settings at the first boot with this build; `log_tail(pattern="settings", n=600)` after that boot shows no warning about them), start, repeat the first block. The shot says `... Missing: Ace of spades` after the three numbers. Set it back to 0.

- [ ] **Step 5: Commit**

```bash
git add tools OpenZone_Games
git commit -m "Counting the deck

Between deals a seated player counts the deck: everybody's log says so, and
after CountSeconds he alone is told how many cards there are, how many of the
game's set are missing and how many are copies; which, only when the server's
CountNamesCards says so.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 8: More cards than slots: what gathering leaves behind

**Files:**
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c` (`OZG_Place.OZG_Tidy`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Sessions.c` (`ReturnDeck`)
- Modify: `OpenZone_Games_Bridge/.../OZG_WorldOps.c`, `OZG_Verb.c` (op `loose`)

**Interfaces:**
- Consumes: `OZG_Place.OZG_Open`, `.OZG_Deck` (Task 4); `OZG_Deck_Base.OZG_Free` (Task 3); the op `stuff` (Task 5).
- Produces: `OZG_Place.OZG_Tidy() : int` (the cards beside the deck go into it; as many as do not fit are left on the ground where the place lies, chosen by chance among ALL the cards; the answer is how many were left).

- [ ] **Step 1: Tidying**

In `class OZG_Place`, after `OZG_Release`:

```c
    // Server: before the deck leaves. Cards the place kept beside the deck go
    // back into it. When there are more cards than slots, as many as do not
    // fit are left lying where the place lies, and WHICH is chance among all
    // of them, the deck's own included: the owner's word is "random extra
    // cards", not "the ones that were brought in". Returns how many were
    // left.
    int OZG_Tidy()
    {
        array<EntityAI> beside = new array<EntityAI>();
        CargoBase cargo = GetInventory().GetCargo();
        int i;
        if (cargo)
        {
            for (i = 0; i < cargo.GetItemCount(); i++)
                beside.Insert(cargo.GetItem(i));
        }
        if (beside.Count() == 0)
            return 0;

        OZG_Deck_Base deck = OZG_Deck();
        int room = 0;
        array<EntityAI> all = new array<EntityAI>();
        if (deck)
        {
            room = deck.OZG_Free();
            int held = deck.GetInventory().AttachmentCount();
            for (i = 0; i < held; i++)
                all.Insert(deck.GetInventory().GetAttachmentFromIndex(i));
        }
        for (i = 0; i < beside.Count(); i++)
            all.Insert(beside[i]);

        int over = beside.Count() - room;
        if (over < 0)
            over = 0;

        OZG_Open();
        vector m4[4];

        // Chance picks the ones that stay behind.
        for (i = 0; i < over; i++)
        {
            int pick = Math.RandomInt(0, all.Count());
            EntityAI stray = all[pick];
            all.Remove(pick);
            beside.RemoveItem(stray);
            if (!stray)
                continue;
            EntityAI holder = stray.GetHierarchyParent();
            if (!holder)
                continue;
            GetTransform(m4);
            float dx = Math.RandomFloat(-0.25, 0.25);
            float dz = Math.RandomFloat(-0.25, 0.25);
            m4[3] = m4[3] + Vector(dx, 0, dz);
            holder.GetInventory().DropEntityWithTransform(InventoryMode.SERVER, holder, stray, m4);
        }

        // Each of the others beside the deck now has a slot.
        for (i = 0; i < beside.Count(); i++)
        {
            EntityAI card = beside[i];
            if (!card || !deck)
                continue;
            InventoryLocation src = new InventoryLocation;
            InventoryLocation dst = new InventoryLocation;
            if (!card.GetInventory().GetCurrentInventoryLocation(src))
                continue;
            if (deck.GetInventory().FindFreeLocationFor(card, FindInventoryLocationType.ATTACHMENT, dst))
                GetInventory().TakeToDst(InventoryMode.SERVER, src, dst);
        }
        return over;
    }
```

In `OZG_Sessions.ReturnDeck`, replace

```c
        if (place)
            deck = place.OZG_Release();
```

with

```c
        if (place)
        {
            int left = place.OZG_Tidy();
            if (left > 0)
                OZG_Log.Info("place " + place.OZG_Id() + ": " + left.ToString() + " card(s) found no slot and stay where it lay");
            deck = place.OZG_Release();
        }
```

In `OZG_WorldOps.c`, before `// give who= [deck=class]` (and route `loose` in `OZG_Verb.c`, and add it to `KnownOps()`):

```c
    // loose at="x y z" [r=2] [sweep=1]: the cards that lie on the ground near
    // a point, each as face/look; with sweep they are deleted.
    static bool OpLoose(map<string, string> args, out string detail)
    {
        vector at = At(args, "at");
        float r = OZG_VerbOps.Arg(args, "r", "2").ToFloat();
        bool sweep = OZG_VerbOps.Arg(args, "sweep", "0") == "1";
        array<Object> found = new array<Object>();
        GetGame().GetObjectsAtPosition(at, r, found, null);
        int n = 0;
        string faces = "";
        for (int i = 0; i < found.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(found[i]);
            if (!card || card.GetHierarchyParent())
                continue;
            n++;
            faces = faces + " " + card.OZG_Face() + "/" + card.OZG_Look().ToString();
            if (sweep)
                GetGame().ObjectDelete(card);
        }
        detail = "loose: " + n.ToString() + faces;
        return true;
    }

```

- [ ] **Step 2: Lint, a stand cycle with a client, the checks**

`mod_lint`, the stand cycle, the client, `world_teleport "4699.56 339.02 10192.5"`, `undeck`.

**An ordinary deck is full, so everything stuffed lands beside it:**

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2"}
oz_games {"op":"stuff","id":"0","face":"As","style":"zone","n":"3"}
oz_games {"op":"held","id":"0"}
```
Expected: `stuffed As: deck=0 beside=3`, then `... cards=52 free=0 faces=52 looks=0:52 beside=3`.

```
oz_games {"op":"gather","id":"0","who":"me"}
oz_games {"op":"loose","at":"4701.5 339.0 10196.0"}
```
Expected of the second: `loose: 3 <face>/<look> <face>/<look> <face>/<look>`. Write the three down: with fifty-five cards and three to leave, they are nearly always not the three worn aces that were stuffed, and that is the point. Two seconds later `oz_games {"op":"cards","who":"me"}`. Expected: `cards=52 free=0`, and `looks=` says how many of the stuffed aces went into the deck (`0:<n>,1:<m>` with `n + m = 52`).

`client_shot` of the cards on the ground. `oz_games {"op":"loose","at":"4701.5 339.0 10196.0","sweep":"1"}`, `undeck`.

Do it twice more: the three that stay differ from run to run.

**A sharp's deck takes ten of twelve:**

```
oz_games {"op":"lay","at":"4701.5 339.0 10196.0","mode":"2","deck":"OZG_Deck_Classic_Sharp"}
oz_games {"op":"stuff","id":"0","face":"Kh","style":"zone","n":"12"}
```
Expected: `stuffed Kh: deck=10 beside=2`. `gather`, then `loose`: `loose: 2 ...`; two seconds later `cards`: `cards=62 free=0`. Sweep, `undeck`.

**Room for all leaves nothing:** lay an ordinary deck, `pull id=0 face="2s 3s"`, a tick later `stuff id=0 face=Qd n=2`: `stuffed Qd: deck=2 beside=0`; `gather`; `loose`: `loose: 0`.

**A restart keeps what lies beside the deck:** lay, `stuff ... n=3` (`beside=3`), stand cycle (no build), client, `held`: `beside=3`.

- [ ] **Step 3: Commit**

```bash
git add OpenZone_Games OpenZone_Games_Bridge
git commit -m "Cards that find no slot stay where the game lay

When a game is gathered or folded the cards the place kept beside the deck go
back into it; when there are more cards than slots, as many as do not fit are
left on the ground there, and which ones is chance among all of them.

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

### Task 9: The live pass, the record, the skills

**Files:**
- Create: `docs/2026-11-24-cards-as-things-record.md`
- Modify: `docs/test-commands.md`, `README.md`
- Modify: `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (status of phase 1; anything the build decided otherwise)
- Modify: the `dayz-modding` skill and the `openzone` skill (own lines only)

- [ ] **Step 1: One game by hand of the client, start to end**

On a fresh boot with the client, at the runway: `oz_games {"op":"chat","text":"!ozg deck"}`, `oz_games {"op":"chat","text":"!ozg table"}`, lay the deck out on the table by the hologram (Task 4's way, the aim on the table's top), `oz_games {"op":"bots","id":"0","n":"2"}`, press "Count the deck" (`ui_find` for `OZG_BtnCount`, `ui_click` by the path it gives), wait seven seconds, deal, play three moves by clicks, fold by the button. Shots: the count's line, a bout, the deck back in the hands after the fold. Send them to the owner.

Then the same once on the other stand's terrain when that stand is free (NHChernobyl, the wall in Rostok: `docs/test-commands.md`, "The stand on the NHChernobyl terrain"): the deck must reach a place that lies on a step, not only on a runway.

- [ ] **Step 2: The slot table of a heavy pack**

Sixty-three slots more against the slot table of a server with many mods (spec §12). **Ask the owner first**: which stand carries such a pack and whether it may be booted with this mod added (`E:\dayzmod\testserver\mirage1114` held the Mirage pack on 2026-11-14; that stand, its world and its profile are not this project's, and what was added is taken out again afterwards). On it: `oz_games {"op":"give","who":"me","deck":"OZG_Deck_Classic_Sharp"}`, a second later `{"op":"cards","who":"me"}`. Expected: `slots=62 cards=52 free=10 faces=52`. If the cards do not all go in, stop and report: the spec says a failure here comes back to the owner.

When the owner says "not now", the record says "not measured" in plain words and phase 1 is not called done.

- [ ] **Step 3: The record**

`docs/2026-11-24-cards-as-things-record.md`, in the shape of `docs/2026-11-24-phases-2-6-record.md`:

- what exists now, in ten lines;
- the decisions made on the way and why: a further card of a face named by the face and its number, with no limit to them; a card of a deal as a record that the mark will join; the deck spared by the action, not by a clock; the deck asked out of the hands and waited for; the place shut on the server and silent on a client; the deck back by the ground; a card's look stored as a style's name; a deck told new from old by having been stored;
- where the build differs from the spec, each with its reason;
- every number measured: Task 1's `ms`, the fill, the boots, the lines of the checks;
- the engine facts found (they are in the skill; here a line each and the pointer);
- what is not done: marks, noticing, sleights (phases 2 to 4); a card's own model and picture; the table from a card in a game to its item, which phase 2 needs for marks (a game keeps names and looks; the items are read anew at each deal); traders taking a deck with its cards; how long a loose card the economy has no line for lies on the ground; `OZG_Session.Blank`'s ring worked out with nobody near (an older defect); and whatever Step 2 left.

- [ ] **Step 4: What a person tries**

`docs/test-commands.md`: rows for `!ozg deck sharp` and `!ozg deck zonesharp`; a short part "The deck is a box of cards" (take a card out, lay the deck out short of it, count, gather; put more cards in than a deck has room for); in "The verb for scripts" the ops `cards`, `pull`, `undeck`, `stuff`, `held`, `loose`, and `begin`'s `cards=` and `looks=`; a line that a deck from before this build is empty and `!ozg deck` gives a new one. `README.md`: the status paragraph says that a deck is a container of its cards and that a deal is dealt from what it holds; the line about `docs/examples` says "the decks and the cards".

- [ ] **Step 5: The spec, the skills, the commit**

The spec: its status line says phase 1 is built and names the record; every place where the build decided otherwise is amended (none is expected beyond what was amended when this plan was written: §3.1, §3.2, §3.3, §3.4, §9).

Every engine fact measured since Task 1 goes into `dayz-modding` with neutral names (a card that names sixty-two slots goes into the first free one; what a storage load asks of `CanReceiveAttachment`; what a deck in the hands shows of its attachments), the pointers to this repository's code into `openzone` (`references/networking-and-items.md`). `python -I openzone/scripts/check-generic.py` from `E:\dayzmod\dayz-modding-skill` must print nothing. Stage only this work's lines in both; commit; **do not push**.

```bash
cd /e/openzone/openzone-games
git add docs README.md
git commit -m "Cards as things: the record, and how one person tries it

Co-Authored-By: Claude Fable 5.2 <noreply@anthropic.com>"
```

---

## What the spec asks of phase 1, and where it is

| Spec | Task |
|---|---|
| §3.1 a card: a class a face, 1 by 1, fits every slot, a back of its deck's look | 3 |
| §3.2 two kinds of deck, 52 and 62 slots; fills itself once; takes cards only | 3 |
| §3.2 to be measured: sixty-two names on a card, the first free slot; a heavy pack | 3 (Step 7), 9 (Step 2) |
| §3.3 the place holds the deck; nobody opens it; the deck and its cards survive the move and a restart | 1, 4 |
| §3.3 more cards than slots: random ones stay where the place lay | 8 |
| §3.4 a deal from what the deck holds; any number of cards of a face; "not enough cards"; `Sound` | 2, 5 |
| §3.5 counting | 7 |
| §3.6, §6.8 the look of every back, a copy drawn as it is, a hand's length | 2, 6 |
| §7 `CountSeconds`, `CountNamesCards` | 7 |
| §8 a restart: cards and looks with the items, a game in mid-play | 3, 5, 8 |
| §9 `count`, and its answer `counted` | 7 |
| §11.1 "a whole deck plays as it does today" | 5 |
| §11.1 "a deck short of the ace of trumps deals and plays" (an ace: the trump is not known before the deal) | 5 |
| §11.1 "a sharp's deck with a second ace deals it" | 5 |
| §11.1 "a count says so" | 7 |
| §11.1 "gather gives back the very deck with its cards" | 4 |
| §11.1 "with more cards than slots leaves the rest on the ground" | 8 |
| §11.1 "a restart in mid-game" | 5 |

## Risks the plan knows of

- **Task 1 says no**: the deck cannot be kept through its own placing. Then the place gets the sixty-two card slots itself; the kit is deleted as it is today; at laying out its cards are moved one by one into the place (`GetInventory().TakeToDst(InventoryMode.SERVER, src, dst)`: they are in no hands, so each move is done at once), and at gathering into a new, empty deck of the remembered class. Tasks 4, 5 and 8 are rewritten against "the place's own slots"; Tasks 2, 3, 6 and 7 stand. The owner is told before that is built.
- **A storage load that asks the script.** If a place that is shut could not take its deck back at a restart, Task 4's restart check shows `held: no deck`. The way round: the place is open for the first seconds after `EEInit` on the server.
- **`CreateAttachment` and sixty-two names**: Task 3's first check (`cards=52`) is where a card that will not go into "the first free slot" shows. The way round is `CreateAttachmentEx(type, slotId)` with the slot's own id, counted from `OZG_Card01`.
- **A slow client.** The hands answer after a round trip; a placer whose answer takes longer than `DECK_WAIT` loses the place and keeps the deck. Four seconds is a guess until Task 1 says how long the hands take.
- **The window's seat strip shows one back a seat**: a hand's strange backs are a number beside it, not pictures. The spec calls the traces schematic for now.
