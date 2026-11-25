# Marks (the card sharp's design, phase 2) Implementation Plan

Carried out on 2026-11-25 in the session that wrote it; where the run went otherwise than written, `docs/2026-11-25-marks-record.md` says so.

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A marking tool and a loupe as items; a card keeps its mark on the server (its sign, its marker, its readers and noticers) and through restarts; in a game a reader sees, on every back they read in the other players' fans and on top of the stock, the sign drawn into the back's pattern and the face pencilled faintly in a corner, and nobody else is told anything.

**Architecture:** The mark is a few fields of the card item (`OZG_Card_Base`), written to the world's storage after the card's style (`CARD_SAVE` 2) and never synchronised to a client. Marking and studying are two recipes of the game's own crafting (`RecipeBase`): marking tool and card, the card marked and one use of the tool spent; loupe and card, "marked" or "nothing found" said to the player alone and a marked card's reader added. A deal reads every card's mark into its record (`OZG_DealCard` gains `Sign`, `Readers`, `Noticers`; durak's state `VERSION` 3); the game says which cards a seat reads where they lie back up (`OZG_Game.ReadsFor`), and the session puts them into the head of that seat's own view (`OZG_WView.Reads`); a watcher and the other seats get nothing. The window draws a read back with two more pictures inside the card's turning frame: the sign, a sprite of `ozg_marks` placed by the sign's number in the back's own lattice, and the face in pencil.

**Tech Stack:** Enforce Script (DayZ 1.30, diag build), `config.cpp`, Python 3 with Pillow (`tools/make_marks.py`), the `dayz` MCP server (`mod_lint`, `mod_build`, `server_start`, `log_verdict`, `world_exec`, `client_*`), the stand's verb `oz_games`.

**The design:** `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`: §4.1 to §4.4 and §4.6, §7 (`Cheating`, `MarkUses`, `MarkSeconds2`, `LoupeSeconds`, `MarkKnowers`), §8, §9, §10, §11.2. What a noticer sees (§4.5) and noticing (§5) are phase 3; the sleights (§6) phase 4. The table the reads are drawn on is `docs/2026-11-25-table-window-record.md`.

## Global Constraints

- **English and Ukrainian only** in the repository: code, comments, commit messages, docs. No Russian anywhere.
- **Work in `main`**, no branches. **Do NOT push; the owner alone gives the push word.** No remote is added, nothing goes to the Workshop.
- Commit author is the repository's own config (`Alexandru Covalschi`). Every commit message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- **No DayZ API is invented.** Every engine call of this plan is in "What the game's own scripts say" below, read in `.dayz-mcp/corpus/scripts`. A call that does not compile is looked up with `knowledge_find`, never guessed at.
- **Enforce Script rules that bite here**: no ternary; a compound boolean goes into a local before it goes into a field; a local is never named `out`, `owned`, `local` or like a type; a field or a method is never named like a class of the game or of another mod: before the first build of a task, `knowledge_find(kind="class", name=<the new name>)` for every new name that is one plain word; one expression holds about ten parts at most; `Length()` and `Substring()` count bytes.
- **A card's name has no fixed width** (`As`, `As.1`, `As.12`): `OZG_Cards.FaceOf`, `RankOf`, `SuitOf` take it apart, nothing else.
- **The JSON loader runs no initialiser for a nested object, and a member the file did not carry is raw memory** (the core's rule): a nested record read from JSON is made again by script (`Copy()`), and a record of an older file that lacks a member is made from the members it has.
- **Marks never reach a client** except as a read in the head of the reader's own view (spec §4.6); a player's id never leaves the server.
- **Files are LF** (`git ls-files --eol` says `i/lf w/lf`; `config.cpp` is `w/crlf` in the working tree, edit it with the Edit tool). A Python script that carries a backslash is written with the Write tool, not a Bash heredoc.
- **The stand.** A stand cycle is `client_stop` (when a client runs), `server_stop`, `mod_build`, `job_wait`, `server_start(timeout=600)`, `job_wait`, `log_verdict(since=...)` which must say `pass`, `world_ready`. A client, when a step needs one: `client_start(language="English")` or `language="russian"` (the Ukrainian texts), `job_wait`. End every working session with `client_stop`. After any edit of a `.toml`, `project_open` again.
- **The stand may be in the owner's hands.** Ask before the first cycle of a working session which stand is free; never restart a stand the owner is playing on without the owner's word.
- **The checks are written for the NHChernobyl stand** (world 84, the active MCP profile since 2026-11-24). Its open ground: a place for seven seats at `6097.4 107.65 6028.9`, one for four at `6097.4 107.65 6040` (`lay` with `mode=2`, `from` two metres west). A server holds the map only near players: the client is connected and its player stands within a few metres (`world_teleport "6095.4 107.65 6028.9"`).
- **Stand calls** are written `oz_games {...}`: the MCP tool `world_exec` with `verb="oz_games"` and those `args`. Every value of `args` is a string. What is compared is the answer's `detail` (the bridge cuts it at 400 characters).
- **Screenshots**: every stand run that has something on screen ends with its key shots sent to the owner (`SendUserFile`).
- **The spec's numbers are copied, not re-decided**: `MarkUses` 20, `MarkSeconds2` 8, `LoupeSeconds` 5, `MarkKnowers` 16, `Cheating` 1; a sign is a number 1..99 whose number picks the kind and the place of the fault; "a card marked again by another player gets him as a second reader and keeps its sign"; "the oldest noticer gives way, the marker never does".

## What the game's own scripts say

Read for this plan in `.dayz-mcp/corpus/scripts` (the 1.30 scripts the `dayz` MCP indexes; the paths below are relative to it) and in the game's configs under `P:\DZ` (extracted 2026-08-22, the values held against the 1.30 PBOs). None of it is measured yet; Task 3 measures what the design asks of it (§4.2).

**A recipe** (`4_world/classes/recipes/recipebase.c`):
- The constructor fills the ingredient lists, the spawn distance, the count of results and the name, then calls `void Init()` (:88). Every other field of an ingredient starts at 0, and 0 is not "off": `m_MaxDamageIngredient` 0 refuses all but a pristine item (:468), `m_MaxQuantityIngredient` 0 any item with quantity (:441), `m_IngredientSetHealth` 0 sets the health to 0. So `Init` sets every field, as vanilla's `PurifyWater` does (`classes/recipes/recipes/purifywater.c:3-68`); -1 is "no check" or "do nothing".
- `float GetLengthInSecs()` (:95-98) answers `m_AnimationLength * CRAFTING_TIME_UNIT_SIZE` (`3_game/constants.c:675`: 4.0). The crafting gesture asks the recipe for it on both sides (`CAContinuousCraft.Setup`, `classes/useractionscomponent/actioncomponents/cacontinuouscraft.c:19-22`) and counts frame time up to it; a recipe may override it.
- `bool CanDo(ItemBase ingredients[], PlayerBase player)` (:551-561) refuses an ingredient that has attachments; `void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)` (:563) is the recipe's own change; `void InsertIngredient(int index, string ingredient, ...)` (:159).
- `bool CheckRecipe(ItemBase item1, ItemBase item2, PlayerBase player)` (:479-502): one of the two items is the one in the player's hands (`ItemBase PlayerBase.GetItemInHands()`, `4_world/entities/manbase/playerbase.c:6733`); then `CanDo` and the conditions (:429-476). It does not ask what the two items are: `bool CheckIngredientMatch(ItemBase item1, ItemBase item2)` (:115-157) does, in either order.
- `PerformRecipe` (:521-545): the results are spawned (a loop over `m_NumberOfResults`, none when `AddResult` was never called), the ingredients' changes applied, then `Do` runs. Vanilla's `PurifyWater` makes nothing: the tablets lose quantity and `Do` changes the bottle (purifywater.c:31, :58, :75-79).
- An ingredient's name takes every class of `scope = 2` that inherits it (`4_world/plugins/pluginbase/pluginrecipesmanager.c:180-217, :263-294`): `InsertIngredient(1, "OZG_Card_Base")` takes every card.

**Registering**: `void PluginRecipesManagerBase.RegisterRecipies()` (`4_world/classes/recipes/recipes/pluginrecipesmanagerbase.c:14`, the game's own spelling) calls `RegisterRecipe(RecipeBase recipe)` (:10) once a recipe; the plugin is made on the client and on the server alike and registers in its constructor (`pluginrecipesmanager.c:52-56, :660-663`; `4_world/plugins/pluginmanager.c:53`). `RegisterRecipe` gives a recipe its index for an id and keeps it by class name (`pluginrecipesmanager.c:443-455`); `static int PluginRecipesManager.RecipeIDFromClassname(string classname)` (:470-475) finds it again. The client sends the id to the server (`classes/useractionscomponent/actions/continuous/actionworldcraft.c:196-221`), so both sides register the same recipes in the same order, which one set of mods on both gives.

**Where the two items may be**: one in the hands. The other, by the code, anywhere the player reaches: the valid recipes refuse an attachment only when its parent's `DisassembleOnLastDetach()` is true (`pluginrecipesmanager.c:101-105`; false by default, `3_game/entities/entityai.c:552-555`, and the deck does not override it), and the sanity check asks only that no other live player owns it and that it lies within 5 m (`pluginrecipesmanager.c:407-441`). In the inventory the other item is dragged onto the item in the hands (`handspreview.c:77-89`, `icon.c:700-725`). Task 3 measures whether a card in a slot of a deck the player carries can be dragged there at all.

**The server's half of a finished gesture**: `void PluginRecipesManager.PerformRecipeServer(int id, ItemBase item_a, ItemBase item_b, PlayerBase player)` (`pluginrecipesmanager.c:317-346`) orders the two items, checks the recipe and the sanity check, calls `Error(...)` when either fails, and performs the recipe. The plugin is `GetPlugin(PluginRecipesManager)` (cacontinuouscraft.c:19). The MCP has no tool that crafts, and `world_action` cannot choose a recipe (actionworldcraft.c:132-133 sets the id on a client only), so the stand's `craft` (Task 3) asks the recipe's own `CheckRecipe` first and calls `PerformRecipeServer` only when it agrees: a refusal is an answer, never a script error.

**Quantity** (`4_world/entities/itembase.c`): `bool SetQuantity(float value, bool destroy_config = true, ...)` (:3319-3356) works on the server, clamps to the config's minimum and maximum, and at the minimum deletes an item whose config says `varQuantityDestroyOnMin` (and answers true); `Inventory_Base` says it (`P:\DZ\data\config.cpp:2495, :2558`). The sewing kit counts its uses so (`P:\DZ\gear\tools\config.cpp:603-621`): `stackedUnit = "percentage"; quantityBar = 1; varQuantityInit = 100; varQuantityMin = 0; varQuantityMax = 100;`.

**Who carries a thing**: `proto native Man EntityAI.GetHierarchyRootPlayer()` (`3_game/entities/entityai.c:882`): the player whose inventory holds it at any depth; a card in the deck of a place has none. `proto native bool GameInventory.EnumerateInventory(InventoryTraversalType tt, out array<EntityAI> items)` (`3_game/systems/inventory/inventory.c:127`).

**Stand-in models.** The design leaves the tool's and the loupe's models to art of later (§12), and the game has no loupe (`knowledge_find` for Loupe, Magnif, Lens: none). The tool wears the pen's model, `\dz\gear\tools\Pen.p3d` (`P:\DZ\gear\tools\config.cpp:231-239`: `animClass = "Knife"`, `itemSize[] = {1,1}`, `rotationFlags = 17`, no hidden selections); the loupe the pistol scope's, `\DZ\weapons\attachments\optics\optic_pistol.p3d` (`P:\DZ\weapons\attachments\optics\config.cpp:865-885`: `animClass = "Binoculars"`, `hiddenSelections[] = {"camo"}` with `dz\weapons\attachments\optics\data\pistol_scope_co.paa` and `dz\weapons\attachments\data\pistol_scope.rvmat`).

**The window** (the `dayz-modding` skill, `references/gui-layouts.md`): `SetPos` and `SetSize` take screen pixels on a widget of exact flags (§31); the children of a turned `FrameWidget` turn with it, whole (§38). An imageset that `config.cpp` does not list draws blank pictures, with no error (the comment over `imageSets` in `OpenZone_Games/config.cpp`).

**The real mouse on the stand**: the skill's `references/toolchain.md`, "Dragging in the inventory with the real mouse (measured 2026-11-24)": `user32.dll` from PowerShell, `SetCursorPos` and `mouse_event` (2 down, 1 a move, 4 up). A held button is the same down, a wait, an up. Before any real click the client window moves clear of the taskbar (the `openzone` skill, `references/machine.md`), and the client window is the active one.

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c` | `CARD_SAVE` 2; the loupe's notice and its two words | modify |
| `.../3_Game/.../OZG_Settings.c` | five settings, version 2, what a client is told | modify |
| `.../3_Game/.../OZG_Cards.c` | `OZG_DealCard` keeps the mark; `Copy`, `ReadBy` | modify |
| `.../3_Game/.../OZG_Game.c` | `TakeCards` keeps the mark; `ReadsFor` | modify |
| `.../3_Game/.../OZG_Durak.c`, `OZG_DurakState.c` | the dealt records keep the mark; `VERSION` 3; `ReadsFor` | modify |
| `.../3_Game/.../OZG_Wire.c` | `OZG_WRead`, `OZG_WView.Reads` | modify |
| `.../4_World/.../OZG_CardThings.c` | the mark on the card item; reading it into a deal | modify |
| `.../4_World/.../OZG_Recipes.c` | the two recipes, what they share, their registration | create |
| `.../4_World/.../OZG_Session.c` | a seated player's reads into that player's view | modify |
| `OpenZone_Games/config.cpp` | the marking tool, the loupe, the marks' imageset | modify |
| `OpenZone_Games/strings.csv`, `stringtable.csv` | the two items, their actions, the loupe's words | modify |
| `docs/examples/types-openzone-games.xml` | the two items for a server's economy | modify |
| `tools/make_marks.py` | the signs' and the pencil's pictures, their imageset, where they sit on a back | create |
| `art/marks/SOURCES.md` | where the marks' pictures come from | create |
| `art/marks/*.png`, `OpenZone_Games/gui/imagesets/ozg_marks.imageset`, `ozg_marks_ca.paa` | written by `make_marks.py` | create |
| `.../5_Mission/.../OZG_MarkPlaces.c` | written by `make_marks.py`: where a sign and the pencil sit on a back | create |
| `tools/make_layouts.py`, `gui/layouts/ozg_card.layout` | a card frame gains its sign and its pencil | modify |
| `.../5_Mission/.../OZG_GameMenu.c`, `OZG_Table.c` | the reads drawn on the fans and on the stock | modify |
| `OpenZone_Games_Bridge/.../OZG_WorldOps.c`, `OZG_Verb.c` | stand ops `mark`, `markof`, `reads`, `craft`; `give` of any thing | modify |
| `docs/test-commands.md`, `docs/2026-11-25-marks-record.md`, `README.md` | what a person tries, and what was measured | modify, create |

---

### Task 1: The mark on the card

A card keeps its mark itself (spec §4.1): the sign, the marker, who reads it, who notices it. Server only, saved with the item, never synchronised. The stand gets two ops to mark a card and to read a card's mark, so that the later tasks can be checked without crafting.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c` (the line `static const int CARD_SAVE = 1;`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c` (class `OZG_Card_Base`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (two ops at the end of the class)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c` (dispatch and `KnownOps`)

**Interfaces:**
- Consumes: `OZG_Cfg.Get().MarkKnowers` (Task 2; until Task 2 lands, the literal 16 of the spec stands in its place, Step 2 says where).
- Produces, on `OZG_Card_Base` (server): `bool OZG_Marked()`, `int OZG_Sign()`, `string OZG_MarkedBy()`, `bool OZG_Reads(string uid)`, `void OZG_MarkBy(string uid, int sign)`, `void OZG_AddReader(string uid)`. Stand ops `mark` and `markof`, on the deck of a laid-out game (`id=`) or on the cards the first player carries (`in=me`), and the helper `OZG_WorldOps.CardsIn` they share.

- [x] **Step 1: The card's store gains a version**

In `OZG_Const.c` replace

```c
    static const int CARD_SAVE = 1;
```

with

```c
    // What a card writes to the world's storage: 1 its style; 2 its mark too.
    static const int CARD_SAVE = 2;
```

- [x] **Step 2: The mark's fields, its server methods, its store**

In `OZG_CardThings.c`, in `class OZG_Card_Base`, after the line `protected int m_OZG_Painted;` add:

```c

    // The mark (design §4.1): kept on the server with the item and written
    // to the world's storage, never synchronised to a client. Sign 0 is no
    // mark; 1..99 is the fault drawn into the back, its number picking the
    // kind and the place. The marker is the first reader and never gives way.
    protected int m_OZG_Sign;
    protected string m_OZG_By;
    protected ref array<string> m_OZG_Readers;
    protected ref array<string> m_OZG_Noticers;
```

In the constructor `void OZG_Card_Base()`, before `RegisterNetSyncVariableInt(...)`, add:

```c
        m_OZG_Readers = new array<string>();
        m_OZG_Noticers = new array<string>();
```

After the method `OZG_SetStyle` add:

```c
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
```

(Until Task 2 adds `MarkKnowers`, write `int most = 16;` with the comment `// MarkKnowers of the design; Task 2 makes it a setting.` and change it in Task 2, Step 3.)

Replace the whole `OnStoreSave` and `OnStoreLoad` with:

```c
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
```

- [x] **Step 3: The stand marks a card and reads a card's mark**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before its closing brace), add:

```c
    // The cards `mark` and `markof` look at: in=me those the first player
    // carries, at any depth; otherwise those of the laid-out game id=.
    protected static bool CardsIn(map<string, string> args, array<EntityAI> into, out string detail)
    {
        into.Clear();
        if (OZG_VerbOps.Arg(args, "in", "place") == "me")
        {
            PlayerBase me = FirstPlayer();
            if (!me)
            {
                detail = "nobody is connected";
                return false;
            }
            me.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, into);
            return true;
        }
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        OZG_DeckRead.ThingsOf(s.m_Place, into);
        return true;
    }

    // mark [id=<place> | in=me] face=<face>|all [who=me|<id>] [n=1] [sign=0]:
    // n cards of that face (all: every card) marked by `who`, as the marking
    // tool would; sign 0 draws a sign by chance. Only for the stand: a player
    // cannot mark a card that lies in a place (design §4.2).
    static bool OpMark(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        if (uid == "")
        {
            detail = "mark: nobody to mark it";
            return true;
        }
        string face = OZG_VerbOps.Arg(args, "face", "As");
        int n = OZG_VerbOps.Arg(args, "n", "1").ToInt();
        if (face == "all")
            n = things.Count();
        int sign = OZG_VerbOps.Arg(args, "sign", "0").ToInt();
        OZG_Random dice = new OZG_Random();
        int done = 0;
        for (int i = 0; i < things.Count() && done < n; i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            int drawn = sign;
            if (drawn <= 0)
                drawn = 1 + dice.Below(99);
            card.OZG_MarkBy(uid, drawn);
            done++;
        }
        detail = "marked " + done.ToString() + " of " + face;
        return true;
    }

    // markof [id=<place> | in=me] face=<face>|all [who=me|<id>]: each card of
    // that face (all: each marked card) as its face, its sign, and whether
    // `who` marked it and reads it.
    static bool OpMarkOf(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        string face = OZG_VerbOps.Arg(args, "face", "As");
        detail = face + ":";
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            if (face == "all" && !card.OZG_Marked())
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            string one = " " + card.OZG_Face() + " sign=" + card.OZG_Sign().ToString();
            one = one + " by_who=" + B(card.OZG_MarkedBy() == uid);
            one = one + " reads=" + B(card.OZG_Reads(uid));
            detail = detail + one + ";";
        }
        return true;
    }
```

In `OZG_Verb.c`, after the two lines of `op == "stuff"`, add:

```c
        if (op == "mark")
            return OZG_WorldOps.OpMark(args, detail);
        if (op == "markof")
            return OZG_WorldOps.OpMarkOf(args, detail);
```

and in `KnownOps()` append `, mark, markof` to the second line of the list.

- [x] **Step 4: Lint, build, and the first check**

Run `mod_lint`: no refusals. Run a stand cycle (Global Constraints), then `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
```

Expected: `laid <id> seats=7 kind=1`. With that id:

```
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"mark","id":"<id>","face":"As","sign":"12"}
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"mark","id":"<id>","face":"As","who":"puppet_not_there"}
oz_games {"op":"markof","id":"<id>","face":"As","who":"puppet_not_there"}
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"markof","id":"<id>","face":"all"}
```

Expected, in order: `As: As sign=0 by_who=0 reads=0;`; `marked 1 of As`; `As: As sign=12 by_who=1 reads=1;`; `marked 1 of As` (an id that is no player still marks); `As: As sign=12 by_who=0 reads=1;` (it became a second reader, and the card kept its sign); `As: As sign=12 by_who=1 reads=1;` (the marker stays); `all: As sign=12 by_who=1 reads=1;` (the one marked card of the deck).

- [x] **Step 5: The mark survives a restart**

`server_stop` (it saves), `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, `client_start`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, then:

```
oz_games {"op":"markof","id":"<id>","face":"As"}
```

Expected: `As: As sign=12 by_who=1 reads=1;` (the place, its deck and the card came back from the world's storage; spec §8). Gather the game: `oz_games {"op":"gather","id":"<id>"}` (a place the stand lays has no host to fold it; the deck then goes into free hands, and `oz_games {"op":"undeck"}` deletes the decks the player carries).

- [x] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Marks: a card keeps its mark, saved with it

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 2: The settings of marking

Five settings of the spec (§7), a settings file of version 2, and what a client is told: whether cheating is on, and how long the two crafts take.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Settings.c`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c` (the literal of Task 1)

**Interfaces:**
- Produces: `OZG_Settings.Cheating`, `MarkUses`, `MarkSeconds2`, `LoupeSeconds`, `MarkKnowers` (ints), on the server from the file and on a client from the sync packet (`Cheating`, `MarkSeconds2`, `LoupeSeconds` only).

- [x] **Step 1: The fields and their defaults**

In `class OZG_Settings`, after `int   DebugLog;` add:

```c
    // Marks and the loupe (card sharp's design §7). Cheating 0: nothing can
    // be marked, studied or read.
    int   Cheating;
    int   MarkUses;
    int   MarkSeconds2;
    int   LoupeSeconds;
    int   MarkKnowers;
```

Replace `return 1;` in `LatestVersion()` with `return 2;`. In `LoadDefaults()`, after `DebugLog = 0;` add:

```c
        Cheating = 1;
        MarkUses = 20;
        MarkSeconds2 = 8;
        LoupeSeconds = 5;
        MarkKnowers = 16;
```

Replace `Migrate`:

```c
    override bool Migrate(int from)
    {
        // Version 1 had no marks: their defaults.
        if (from < 2)
        {
            Cheating = 1;
            MarkUses = 20;
            MarkSeconds2 = 8;
            LoupeSeconds = 5;
            MarkKnowers = 16;
        }
        Version = LatestVersion();
        return true;
    }
```

In `Validate`, before `if (DebugLog != 0)`, add:

```c
        if (Cheating != 0)
            Cheating = 1;
        if (MarkUses < 1)
        {
            MarkUses = 20;
            warnings++;
        }
        if (MarkSeconds2 < 1)
        {
            MarkSeconds2 = 8;
            warnings++;
        }
        if (LoupeSeconds < 1)
        {
            LoupeSeconds = 5;
            warnings++;
        }
        if (MarkKnowers < 2)
        {
            MarkKnowers = 16;
            warnings++;
        }
```

- [x] **Step 2: What a client is told**

In `class OZG_ClientCfg`, after `int   PoseCount;` add:

```c
    int   Cheating;
    int   MarkSeconds2;
    int   LoupeSeconds;
```

In `OZG_Cfg.Fill`, after `c.PoseCount = now.PoseList.Count();` add:

```c
        c.Cheating = now.Cheating;
        c.MarkSeconds2 = now.MarkSeconds2;
        c.LoupeSeconds = now.LoupeSeconds;
```

In `OZG_Cfg.TakeSync`, after `fresh.MarkSeconds = c.MarkSeconds;` add:

```c
        fresh.Cheating = c.Cheating;
        fresh.MarkSeconds2 = c.MarkSeconds2;
        fresh.LoupeSeconds = c.LoupeSeconds;
```

- [x] **Step 3: The card's cap comes from the settings**

In `OZG_CardThings.c`, in `OZG_AddReader`, replace the line `int most = 16;` and its comment with `int most = OZG_Cfg.Get().MarkKnowers;`.

- [x] **Step 4: Check**

`mod_lint`, a stand cycle. Then read the server's settings file: `$profile:OpenZone\OZ_Games_Settings.json` in the stand's `profiles` folder (`E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json`). Expected: `"Version": 2`, `"Cheating": 1`, `"MarkUses": 20`, `"MarkSeconds2": 8`, `"LoupeSeconds": 5`, `"MarkKnowers": 16`, and every value it held before kept. `log_verdict` says `pass` with no settings warning.

- [x] **Step 5: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Settings.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c
git commit -m "Marks: the five settings of marking and the loupe

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 3: The marking tool

An item that marks a card through a recipe of the game's own crafting (spec §4.2), its uses counted as the sewing kit counts them, and every marking written to the server's log with the marker's id (§6.1). Its check is the spec's measurement: whether the recipe system takes "tool and card, nothing made, the card changed", and whether the card may stay in a slot of a deck the player holds.

**Files:**
- Modify: `OpenZone_Games/config.cpp` (`CfgPatches` units; the item after the cards)
- Create: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Recipes.c`
- Modify: `OpenZone_Games/strings.csv`; `OpenZone_Games/stringtable.csv` (written by `tools/stringtable.py`)
- Modify: `docs/examples/types-openzone-games.xml`
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (`OpGive`, `OpCraft`), `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_Card_Base.OZG_MarkBy`, `OZG_Sign`, `OZG_Reads`, `OZG_Face` and `OZG_WorldOps.CardsIn` (Task 1); `OZG_Cfg.Get().Cheating`, `MarkUses`, `MarkSeconds2` (Task 2).
- Produces: the config class `OZG_MarkTool`; in `OZG_Recipes.c`: `class OZG_Recipes` with `static RecipeBase MarkRecipe()`, `static RecipeBase MakeMark()`, `static bool Carried(EntityAI item, PlayerBase player)`, `static void Spend(ItemBase tool)`; `class OZG_RecipeMark extends RecipeBase`; the registration (`modded class PluginRecipesManagerBase`). Stand op `craft what=mark [face=<face>] [in=me | in=place id=<place>]`; `give` takes `what=<class>`.

- [x] **Step 1: The tool as an item**

In `OpenZone_Games/config.cpp` (CRLF in the working tree: edit it with the Edit tool), in `CfgPatches`, replace

```cpp
        units[] = {"OZG_Deck_Classic", "OZG_Deck_Stalker", "OZG_Deck_Classic_Sharp", "OZG_Deck_Stalker_Sharp"};
```

with

```cpp
        units[] = {"OZG_Deck_Classic", "OZG_Deck_Stalker", "OZG_Deck_Classic_Sharp", "OZG_Deck_Stalker_Sharp", "OZG_MarkTool"};
```

and after the line `// <<< generated: cards` add:

```cpp

    // ------------------------------------------------------------ the card sharp's tools
    //
    // Each works through a recipe of the game's own crafting (OZG_Recipes.c):
    // the tool in the hands and a card the player carries. Their models are
    // the game's own, stand-ins until art of their own (card sharp's design
    // §12): the pen and the pistol scope.
    class OZG_MarkTool: Inventory_Base
    {
        scope = 2;
        displayName = "$STR_OZG_MARKTOOL_NAME";
        descriptionShort = "$STR_OZG_MARKTOOL_DESC";
        model = "\dz\gear\tools\Pen.p3d";
        animClass = "Knife";
        itemSize[] = {1, 1};
        rotationFlags = 17;
        weight = 12;
        // Its uses as the sewing kit counts them, a percentage: a use is
        // 100 / MarkUses of it, and an empty tool is gone (OZG_Recipes.Spend).
        stackedUnit = "percentage";
        quantityBar = 1;
        varQuantityInit = 100;
        varQuantityMin = 0;
        varQuantityMax = 100;
    };
```

- [x] **Step 2: Its words**

At the end of `OpenZone_Games/strings.csv` add:

```
# -- the card sharp's tools
STR_OZG_MARKTOOL_NAME,Ручка шулера,Card sharp's pen
STR_OZG_MARKTOOL_DESC,"Мітить карту так, що ви впізнаєте її лице з сорочки. Візьміть ручку в руки й перетягніть на неї карту, яку несете.","Marks a card so that you know its face by its back. Take the pen in your hands and drag a card you carry onto it."
STR_OZG_MARKTOOL_ACT,Позначити карту,Mark the card
```

Run `python -I tools/stringtable.py OpenZone_Games`. Expected: `stringtable.csv: <n> strings`, three more than before the change.

- [x] **Step 3: The recipe**

Create `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Recipes.c`:

```c
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
```

Before the build, `knowledge_find(kind="class", name=...)` for `Spend` and `Carried`: neither may be a class of the game or of a loaded mod (Global Constraints).

- [x] **Step 4: The economy's line**

In `docs/examples/types-openzone-games.xml`, before `    <!-- A card out of its deck is a thing of its own; one entry a face. -->`, add:

```xml
    <!-- The card sharp's tools, small things of a house. -->
    <type name="OZG_MarkTool">
        <nominal>0</nominal>
        <lifetime>14400</lifetime>
        <restock>1800</restock>
        <min>0</min>
        <quantmin>-1</quantmin>
        <quantmax>-1</quantmax>
        <cost>100</cost>
        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" crafted="0" deloot="0"/>
        <category name="tools"/>
        <usage name="Town"/>
        <usage name="Village"/>
    </type>
```

- [x] **Step 5: The stand gives any thing and crafts as a gesture ends**

In `OZG_WorldOps.c`, replace

```c
    // give who= [deck=class]: a deck into a player's hands.
    static bool OpGive(map<string, string> args, out string detail)
```

with

```c
    // give who= [what=class | deck=class]: a thing into a player's hands,
    // else into the inventory.
    static bool OpGive(map<string, string> args, out string detail)
```

and, in `OpGive`, the line `string deck = OZG_VerbOps.Arg(args, "deck", "OZG_Deck_Classic");` with

```c
        string deck = OZG_VerbOps.Arg(args, "what", "");
        if (deck == "")
            deck = OZG_VerbOps.Arg(args, "deck", "OZG_Deck_Classic");
```

After `OpMarkOf`, add:

```c
    // craft what=mark [face=As] [in=me | in=place id=<place>]: the server's
    // half of a finished gesture, as the game's crafting action ends one
    // (PerformRecipeServer): the thing in the first player's hands on the
    // first card of that face the player carries (in=place: in the laid-out
    // game's deck, which the recipe must refuse). The recipe's own check
    // comes first, so a refusal is an answer and no script error.
    static bool OpCraft(map<string, string> args, out string detail)
    {
        PlayerBase me = FirstPlayer();
        if (!me)
        {
            detail = "craft: nobody is connected";
            return false;
        }
        string uid = OZG_Who.Uid(me);
        string what = OZG_VerbOps.Arg(args, "what", "mark");
        RecipeBase recipe = OZG_Recipes.MarkRecipe();
        string recipeName = "OZG_RecipeMark";
        if (OZG_VerbOps.Arg(args, "in", "") == "")
            args.Set("in", "me");
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string face = OZG_VerbOps.Arg(args, "face", "As");
        OZG_Card_Base card = null;
        for (int i = 0; i < things.Count() && !card; i++)
        {
            OZG_Card_Base one = OZG_Card_Base.Cast(things[i]);
            if (one && one.OZG_Face() == face)
                card = one;
        }
        ItemBase hands = me.GetItemInHands();
        if (!recipe || !hands || !card)
        {
            detail = "craft " + what + ": hands=" + B(hands != null) + " card=" + B(card != null);
            return true;
        }
        // The thing in the hands must be this recipe's tool: the recipe's own
        // check does not ask what the two things are.
        if (!recipe.CheckIngredientMatch(hands, card) || !recipe.CheckRecipe(hands, card, me))
        {
            // Which check said no: the recipe's, one at a time.
            ItemBase both[2];
            both[0] = hands;
            both[1] = card;
            bool match = recipe.CheckIngredientMatch(hands, card);
            bool can = recipe.CanDo(both, me);
            bool fits = recipe.CheckConditions(both);
            detail = "craft " + what + ": refused (hands " + hands.GetType();
            detail = detail + " q=" + F2(hands.GetQuantity()) + " hl=" + hands.GetHealthLevel().ToString();
            detail = detail + " match=" + B(match) + " cando=" + B(can) + " cond=" + B(fits);
            detail = detail + " tool=" + B(OZG_Recipes.Carried(hands, me)) + " card=" + B(OZG_Recipes.Carried(card, me));
            EntityAI up = card.GetHierarchyParent();
            string parentType = "none";
            if (up)
                parentType = up.GetType();
            Man root = card.GetHierarchyRootPlayer();
            detail = detail + " in=" + parentType + " root=" + B(root != null);
            detail = detail + " att=" + card.GetInventory().AttachmentCount().ToString() + ")";
            return true;
        }
        PluginRecipesManager plugin;
        Class.CastTo(plugin, GetPlugin(PluginRecipesManager));
        int id = PluginRecipesManager.RecipeIDFromClassname(recipeName);
        plugin.PerformRecipeServer(id, hands, card, me);
        detail = "craft " + what + " on " + card.OZG_Face() + ": sign=" + card.OZG_Sign().ToString();
        detail = detail + " reads=" + B(card.OZG_Reads(uid));
        if (hands.HasQuantity())
            detail = detail + " left=" + F2(hands.GetQuantity());
        return true;
    }
```

In `OZG_Verb.c`, after the two lines of `op == "markof"`, add:

```c
        if (op == "craft")
            return OZG_WorldOps.OpCraft(args, detail);
```

and append `, craft` to the second line of `KnownOps()`.

- [x] **Step 6: Lint, build, the server's half**

`mod_lint`: no refusals. A stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then, in order:

```
oz_games {"op":"give","what":"OZG_MarkTool"}
oz_games {"op":"give","what":"OZG_Card_As"}
oz_games {"op":"markof","in":"me","face":"As"}
oz_games {"op":"craft","what":"mark","face":"As"}
oz_games {"op":"markof","in":"me","face":"As"}
```

Expected: `give OZG_MarkTool to <uid>: 1` (into the hands); `give OZG_Card_As to <uid>: 1` (the hands are full, so into the inventory; `: 0` means the character has no room: `give` `what=Hoodie_Blue` first, then the card again); `As: As sign=0 by_who=0 reads=0;`; `craft mark on As: sign=<s> reads=1 left=95`, `<s>` in 1..99; `As: As sign=<s> by_who=1 reads=1;`. `log_tail(pattern="mark: ", n=600)` holds `mark: <uid> marked As, sign <s>`.

The uses (`MarkUses` 20): `oz_games {"op":"craft","what":"mark","face":"As"}` nineteen times more. Expected: `left=` falls by 5 each time down to `left=0` on the twentieth, and the sign stays `<s>` every time (a card marked again keeps its sign); one more answers `craft mark: hands=0 card=1` (the empty tool is gone).

The refusals:
1. A card in a place: `oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}` (`laid <id> ...`); `give what=OZG_MarkTool`; `oz_games {"op":"craft","what":"mark","face":"Ks","in":"place","id":"<id>"}` → `craft mark: refused (... card=0 in=OZG_Deck_Classic root=0 ...)`. Gather the game: `oz_games {"op":"gather","id":"<id>"}` (anyone gathers a game nobody sits at; `ask fold` is the host's, and a place the stand lays has no host).
2. `Cheating` 0: `server_stop`; in `E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json` set `"Cheating": 0`; `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client again (`give what=OZG_MarkTool` if its hands came back empty); `give what=OZG_Card_Kh`; `craft what=mark face=Kh` → `craft mark: refused`. Set `"Cheating": 1` back and restart.

- [x] **Step 7: The real gesture (the design's measurement, §4.2)**

The client in a window, moved clear of the taskbar (Global Constraints, "The real mouse on the stand"):
1. `give what=OZG_MarkTool` (into the hands), `give what=OZG_Card_Kh`, `give what=OZG_Deck_Classic` (both into the inventory).
2. Open the inventory (`client_key name="tab"`), `client_shot`. `ui_find(class_name="ItemPreviewWidget", root="screen")` gives the pictures' rectangles; the card is the one showing the king of hearts, the tool the one in the hands' panel.
3. Drag the card onto the tool with the real mouse (the skill's toolchain.md section), `client_shot`. Expected: the inventory shuts and "Mark the card" is offered. If the inventory stays and nothing is offered, the drag did not combine: write down what the screen showed and stop the task there. It is a finding for the owner, not a thing to work round.
4. Hold the left button 5 s and let go; `oz_games {"op":"markof","in":"me","face":"Kh"}` → `Kh: Kh sign=0 by_who=0 reads=0;` (5 s is under the 8 of `MarkSeconds2`: the gesture broke off).
5. Drag again and hold 11 s; `markof` → `Kh: Kh sign=<s> by_who=1 reads=1;`. So the gesture takes more than 5 s and at most 11 (the progress counts the 8 s, cacontinuouscraft.c:41-44; what the crouch before it adds is not in the code).
6. The deck: with the deck in a pocket, `client_shot`. Are its slots shown (a grid of cards beside or under the deck's picture)? If they are, drag a card from one slot onto the tool, hold 11 s, `oz_games {"op":"markof","in":"me","face":"all"}`: the answer holds `Kh` and that slot's card, both marked, and the card is still in its slot (`client_shot` of the deck). If they are not shown, a card leaves its deck to be marked: write it down; nothing in the code changes for it.
7. The same once with `client_start(language="russian")`: the tool's name and "Позначити карту" in Ukrainian (`client_shot`).

The answers of 4 to 6 go into the record (Task 8). Send the key shots to the owner (`SendUserFile`).

- [x] **Step 8: Commit**

```bash
git add OpenZone_Games/config.cpp OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Recipes.c OpenZone_Games/strings.csv OpenZone_Games/stringtable.csv docs/examples/types-openzone-games.xml OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Marks: the card sharp's pen marks a card through the game's crafting

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 4: The loupe

An item, not used up, that tells the one who studies a card with it, and nobody else, "the card is marked" or "nothing found", and makes that player a reader of a marked card (spec §4.3).

**Files:**
- Modify: `OpenZone_Games/config.cpp` (`CfgPatches` units; the item after the marking tool)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Recipes.c`
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c` (the notice and its two words)
- Modify: `OpenZone_Games/strings.csv`, `stringtable.csv`
- Modify: `docs/examples/types-openzone-games.xml`
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (`OpCraft`)

**Interfaces:**
- Consumes: `OZG_Card_Base.OZG_Marked`, `OZG_AddReader` (Task 1); `OZG_Cfg.Get().LoupeSeconds`, `Cheating` (Task 2); `OZG_Recipes.Carried` (Task 3); the core's `OZ_Rpc.Notice(PlayerIdentity to, string op, bool ok, string why)` (`openzone-core/OpenZone_Core/scripts/3_Game/OpenZone/OZ_Rpc.c:391`), which a client shows in the game's window when it is open and as a notification otherwise (`OZG_MissionGameplay.OZG_Notice`).
- Produces: the config class `OZG_Loupe`; `class OZG_RecipeLoupe extends RecipeBase`; `static RecipeBase OZG_Recipes.LoupeRecipe()`, `MakeLoupe()`; `OZG_Const.NOTICE_LOUPE`, `SAY_LOUPE_MARKED`, `SAY_LOUPE_NOTHING`; `craft what=loupe`.

- [x] **Step 1: The loupe as an item**

In `config.cpp`, in `units[]`, replace `"OZG_MarkTool"}` with `"OZG_MarkTool", "OZG_Loupe"}`, and after the class `OZG_MarkTool` add:

```cpp

    class OZG_Loupe: Inventory_Base
    {
        scope = 2;
        displayName = "$STR_OZG_LOUPE_NAME";
        descriptionShort = "$STR_OZG_LOUPE_DESC";
        model = "\DZ\weapons\attachments\optics\optic_pistol.p3d";
        animClass = "Binoculars";
        itemSize[] = {2, 1};
        rotationFlags = 12;
        weight = 120;
        hiddenSelections[] = {"camo"};
        hiddenSelectionsTextures[] = {"dz\weapons\attachments\optics\data\pistol_scope_co.paa"};
        hiddenSelectionsMaterials[] = {"dz\weapons\attachments\data\pistol_scope.rvmat"};
    };
```

- [x] **Step 2: Its words**

After the marking tool's lines in `strings.csv` add:

```
STR_OZG_LOUPE_NAME,Лупа,Loupe
STR_OZG_LOUPE_DESC,"Покаже, чи мічена карта; мічену ви відтоді впізнаватимете з сорочки. Візьміть лупу в руки й перетягніть на неї карту, яку несете.","Shows whether a card is marked; a marked one you will know by its back from then on. Take the loupe in your hands and drag a card you carry onto it."
STR_OZG_LOUPE_ACT,Роздивитися карту,Study the card
STR_OZG_LOUPE_MARKED,Карта мічена,The card is marked
STR_OZG_LOUPE_NOTHING,Нічого не знайдено,Nothing found
```

Run `python -I tools/stringtable.py OpenZone_Games` (five strings more).

In `OZG_Const.c`, after `static const string NOTICE_SAY  = "games.say";` add

```c
    static const string NOTICE_LOUPE = "games.loupe";
```

and after `static const string SAY_BAD_OPTION  = "STR_OZG_SAY_BAD_OPTION";` add

```c
    // What the loupe tells the one who studies a card (design §4.3).
    static const string SAY_LOUPE_MARKED  = "STR_OZG_LOUPE_MARKED";
    static const string SAY_LOUPE_NOTHING = "STR_OZG_LOUPE_NOTHING";
```

- [x] **Step 3: The recipe**

In `OZG_Recipes.c`, in `class OZG_Recipes`, after `protected static ref RecipeBase s_Mark;` add `protected static ref RecipeBase s_Loupe;`, and after `MakeMark()` add:

```c

    static RecipeBase LoupeRecipe()
    {
        return s_Loupe;
    }

    static RecipeBase MakeLoupe()
    {
        s_Loupe = new OZG_RecipeLoupe();
        return s_Loupe;
    }
```

Before `modded class PluginRecipesManagerBase` add:

```c
// The loupe and a card: the player is told, alone, "the card is marked" or
// "nothing found", and becomes a reader of a marked card (design §4.3).
// Nothing is used up. A line in the server's log, as for a marking.
class OZG_RecipeLoupe extends RecipeBase
{
    override void Init()
    {
        m_Name = "#STR_OZG_LOUPE_ACT";
        m_IsInstaRecipe = false;
        // Not read: GetLengthInSecs below gives the setting's length.
        m_AnimationLength = 1.25;
        m_Specialty = 0;

        m_MinDamageIngredient[0] = -1;
        m_MaxDamageIngredient[0] = 3;
        m_MinQuantityIngredient[0] = -1;
        m_MaxQuantityIngredient[0] = -1;
        m_MinDamageIngredient[1] = -1;
        m_MaxDamageIngredient[1] = -1;
        m_MinQuantityIngredient[1] = -1;
        m_MaxQuantityIngredient[1] = -1;

        InsertIngredient(0, "OZG_Loupe");
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
    }

    override float GetLengthInSecs()
    {
        return OZG_Cfg.Get().LoupeSeconds;
    }

    override bool CanDo(ItemBase ingredients[], PlayerBase player)
    {
        // The two items into locals and no super.CanDo (OZG_Recipes.NoAttachments).
        ItemBase loupe = ingredients[0];
        ItemBase card = ingredients[1];
        if (!OZG_Recipes.NoAttachments(loupe) || !OZG_Recipes.NoAttachments(card))
            return false;
        if (OZG_Cfg.Get().Cheating != 1)
            return false;
        if (!OZG_Recipes.Carried(loupe, player))
            return false;
        return OZG_Recipes.Carried(card, player);
    }

    override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
    {
        if (!GetGame().IsServer())
            return;
        OZG_Card_Base card = OZG_Card_Base.Cast(ingredients[1]);
        string uid = OZG_Who.Uid(player);
        if (!card || uid == "")
            return;
        string word = OZG_Const.SAY_LOUPE_NOTHING;
        if (card.OZG_Marked())
        {
            card.OZG_AddReader(uid);
            word = OZG_Const.SAY_LOUPE_MARKED;
        }
        PlayerIdentity id = player.GetIdentity();
        if (id)
            OZ_Rpc.Notice(id, OZG_Const.NOTICE_LOUPE, true, word);
        OZG_Log.Info("loupe: " + uid + " studied " + card.OZG_Face() + ", sign " + card.OZG_Sign().ToString());
    }
}
```

In `RegisterRecipies`, after `RegisterRecipe(OZG_Recipes.MakeMark());` add `RegisterRecipe(OZG_Recipes.MakeLoupe());`.

- [x] **Step 4: The economy's line**

In `docs/examples/types-openzone-games.xml`, after the `OZG_MarkTool` entry, add:

```xml
    <type name="OZG_Loupe">
        <nominal>0</nominal>
        <lifetime>14400</lifetime>
        <restock>1800</restock>
        <min>0</min>
        <quantmin>-1</quantmin>
        <quantmax>-1</quantmax>
        <cost>100</cost>
        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" count_in_player="0" crafted="0" deloot="0"/>
        <category name="tools"/>
        <usage name="Town"/>
        <usage name="Village"/>
    </type>
```

- [x] **Step 5: The stand studies as a gesture ends**

In `OZG_WorldOps.c`, in `OpCraft`, replace its first comment line `// craft what=mark [face=As] [in=me | in=place id=<place>]: the server's` with `// craft what=mark|loupe [face=As] [in=me | in=place id=<place>]: the server's`, and after `string recipeName = "OZG_RecipeMark";` add:

```c
        if (what == "loupe")
        {
            recipe = OZG_Recipes.LoupeRecipe();
            recipeName = "OZG_RecipeLoupe";
        }
```

- [x] **Step 6: Lint, build, check (spec §11.2: "the loupe says 'marked' and makes a reader; marks survive a restart")**

`mod_lint`, a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then, emptying the hands first if they hold something (`give` puts into the hands only when they are free):

```
oz_games {"op":"give","what":"OZG_Loupe"}
oz_games {"op":"give","what":"OZG_Card_Qs"}
oz_games {"op":"craft","what":"loupe","face":"Qs"}
```

Expected: `: 1` twice; `craft loupe on Qs: sign=0 reads=0`; on the client, with the game's window shut, the game's notification "Nothing found" (`client_shot`). Then:

```
oz_games {"op":"mark","in":"me","face":"Qs","who":"puppet_marker","sign":"33"}
oz_games {"op":"markof","in":"me","face":"Qs"}
oz_games {"op":"craft","what":"loupe","face":"Qs"}
oz_games {"op":"markof","in":"me","face":"Qs"}
```

Expected: `marked 1 of Qs`; `Qs: Qs sign=33 by_who=0 reads=0;`; `craft loupe on Qs: sign=33 reads=1` and "The card is marked" on the client (`client_shot`); `Qs: Qs sign=33 by_who=0 reads=1;` (the loupe made a reader). `log_tail(pattern="loupe: ", n=600)` holds both studies.

The restart: `client_stop`, `server_stop`, `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client again, `world_ready`; `markof in=me face=Qs` → `Qs: Qs sign=33 by_who=0 reads=1;` (the card came back in the character's inventory with its readers).

The real gesture once, the client in a window moved clear of the taskbar (Global Constraints, "The real mouse on the stand"): open the inventory (`client_key name="tab"`), find the card's picture and the loupe's in the hands' panel (`ui_find(class_name="ItemPreviewWidget", root="screen")`, `client_shot`), drag `Qs` onto the loupe with the real mouse (the skill's toolchain.md section), hold the left button 3 s → no notice; drag again, hold 8 s → "The card is marked" (`LoupeSeconds` 5). Then with `client_start(language="russian")`: the loupe's name, "Роздивитися карту" and "Карта мічена" (`client_shot`).

`Cheating` 0: `server_stop`; in `E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json` set `"Cheating": 0`; `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client again; `craft what=loupe face=Qs` → `craft loupe: refused`. Set `"Cheating": 1` back and restart.

Send the key shots to the owner (`SendUserFile`).

- [x] **Step 7: Commit**

```bash
git add OpenZone_Games/config.cpp OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Recipes.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c OpenZone_Games/strings.csv OpenZone_Games/stringtable.csv docs/examples/types-openzone-games.xml OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c
git commit -m "Marks: the loupe tells its player whether a card is marked

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 5: A deal reads the marks

At every deal the session reads each card's mark into the card's record, and the game keeps it in its state (spec §3.4: "marks are read from the items again at every deal"; §8: the session file keeps the mark as it was at the deal). A record of an older state carries no mark: durak's state goes to `VERSION` 3 and a state of version 2 is read with clean records.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c` (class `OZG_DealCard`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c` (`TakeCards`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c` (`Begin`, `LoadState`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_DurakState.c` (`VERSION`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c` (`OZG_MarkInto`, `OZG_DeckRead.CardsOf`)

**Interfaces:**
- Consumes: `OZG_Card_Base.OZG_Sign()`, its readers and noticers (Task 1).
- Produces: `OZG_DealCard.Sign` (int), `Readers`, `Noticers` (`ref array<string>`), `OZG_DealCard Copy()`, `bool ReadBy(string uid)`; `OZG_DurakConst.VERSION` 3.

- [x] **Step 1: The record keeps the mark**

In `OZG_Cards.c` replace the whole `class OZG_DealCard` with:

```c
// One card of a deal, as a game is handed it. The name tells it apart from
// every other card of the deal; the rest is what the card is like: the look
// of its back and its mark as it was when the deal began (design §3.4, §4.1).
// Server only: no record goes into a view.
class OZG_DealCard
{
    // A face, or for a further card of that face the face, a dot and its
    // number: `As`, `As.1`, `As.2` (OZG_Cards.CopyNamed).
    string Name;
    // The look of its back (OZG_Decks.StyleAt).
    int Look;
    // 0: a clean card; 1..99 the sign of its mark.
    int Sign;
    // The ids that read it and that notice it.
    ref array<string> Readers;
    ref array<string> Noticers;

    static OZG_DealCard Make(string name, int look)
    {
        OZG_DealCard c = new OZG_DealCard();
        c.Name = name;
        c.Look = look;
        c.Sign = 0;
        c.Readers = new array<string>();
        c.Noticers = new array<string>();
        return c;
    }

    // A record made by script from one the JSON loader filled, or from one
    // a game was handed: every member of a record written since VERSION 3.
    OZG_DealCard Copy()
    {
        OZG_DealCard c = Make(Name, Look);
        c.Sign = Sign;
        if (Readers)
            c.Readers.Copy(Readers);
        if (Noticers)
            c.Noticers.Copy(Noticers);
        return c;
    }

    bool ReadBy(string uid)
    {
        if (Sign <= 0 || uid == "" || !Readers)
            return false;
        return Readers.Find(uid) >= 0;
    }
}
```

- [x] **Step 2: The game keeps what it was handed, mark and all**

In `OZG_Game.c`, in `TakeCards`, replace `m_Given.Insert(OZG_DealCard.Make(c.Name, c.Look));` with `m_Given.Insert(c.Copy());`.

In `OZG_Durak.c`, in `Begin`, replace `s.Dealt.Insert(OZG_DealCard.Make(m_Given[f].Name, m_Given[f].Look));` with `s.Dealt.Insert(m_Given[f].Copy());`.

In `OZG_DurakState.c`, replace `static const int VERSION = 2;` with `static const int VERSION = 3;` and extend the comment above `class OZG_DurakState` (the one that ends in "a change of VERSION") with the line `// 3: a dealt card keeps its mark (Sign, Readers, Noticers).`

In `OZG_Durak.c`, in `LoadState`, after the block `if (s.Version == 1) { ... }` add:

```c
        // A game saved before marks: its records carry the name and the look
        // and nothing more, and a member the file did not carry is raw memory.
        if (s.Version == 2 && s.Dealt)
        {
            array<ref OZG_DealCard> plain = new array<ref OZG_DealCard>();
            for (int was = 0; was < s.Dealt.Count(); was++)
            {
                if (s.Dealt[was])
                    plain.Insert(OZG_DealCard.Make(s.Dealt[was].Name, s.Dealt[was].Look));
            }
            s.Dealt = plain;
            s.Version = OZG_DurakConst.VERSION;
        }
```

and after the check `if (!s.Dealt) return false;` add:

```c
        // Every record made anew by script (the core's rule for what the JSON
        // loader filled).
        array<ref OZG_DealCard> made = new array<ref OZG_DealCard>();
        for (int rec = 0; rec < s.Dealt.Count(); rec++)
        {
            if (s.Dealt[rec])
                made.Insert(s.Dealt[rec].Copy());
        }
        s.Dealt = made;
```

The block for version 1 sets `s.Version = OZG_DurakConst.VERSION`, which is now 3: a game of version 1 gets records made by `Make`, clean. Nothing else changes there.

- [x] **Step 3: The deal reads the marks off the items**

In `OZG_CardThings.c`, in `class OZG_Card_Base`, after `OZG_AddReader` add:

```c
    // What a deal keeps of the mark.
    void OZG_MarkInto(OZG_DealCard rec)
    {
        if (!rec)
            return;
        rec.Sign = m_OZG_Sign;
        rec.Readers.Copy(m_OZG_Readers);
        rec.Noticers.Copy(m_OZG_Noticers);
    }
```

In `OZG_DeckRead.CardsOf`, replace

```c
            into.Insert(OZG_DealCard.Make(named, card.OZG_Look()));
```

with

```c
            OZG_DealCard rec = OZG_DealCard.Make(named, card.OZG_Look());
            card.OZG_MarkInto(rec);
            into.Insert(rec);
```

and in the comment above `CardsOf` replace "the next phase reads the card's mark here too" with "and the card's mark".

- [x] **Step 4: Lint, build, check**

Before the build, leave a game of version 2 on the stand: lay a place, `sit`, `bots` (`n` 1, `on` 0), `ask` `deal` (Task 1's calls, with the build before this task). Then `mod_lint`, a stand cycle with this task's build, and:

1. The old game loads: `log_verdict` says `pass`, and `oz_games {"op":"sess","id":"<old id>"}` answers `st=1` with the same hands as before the restart (`hands=0:6,3:6`).
2. A new deal carries the mark: fold the old game; lay a new one; `mark` `As` with `sign` `12`; `sit`, `bots`, `deal`; `server_stop` (it saves the session file); read `E:\dayzmod\testserver\games-nh\profiles\OpenZone\Games\<id>.json`. Expected: the game's state in it has `Version` 3 and the record of `As` has `Sign` 12. The state sits in that file as JSON strings, its quotes escaped (`\"Sign\":12`), cut into parts of 900 bytes: search for `Sign` and read the parts on both sides of an edge together. Exactly one record has a sign that is not 0.
3. `server_start`: the session of version 3 loads, `log_verdict` = `pass`.

- [x] **Step 5: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_DurakState.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c
git commit -m "Marks: a deal reads every card's mark into its record

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 6: What a reader is told

The game says which cards a seat reads where they lie back up for that seat's player: in the other seats' hands and on top of the stock (spec §4.4; the dealer's choice is phase 4). The session puts them into the head of that seat's own view; the head is made for each player apart, so nobody else gets them, and a watcher gets none (spec §4.6). `Cheating` 0 sends nothing.

The design names the seat's private half of the view for this (§4.6, §9). The head serves instead: the game makes its private half by seat and knows no players, while a read belongs to the player (a seat taken over by another player reads what that player reads), and the session, which knows them, makes the head for each player apart. The record says so (Task 8).

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Wire.c` (`OZG_WRead`, `OZG_WView.Reads`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c` (`ReadsFor`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c` (`ReadsFor`, `DealtCard`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`ViewBody`)
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Table.c` (`Take`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c`, `OZG_Verb.c` (op `reads`)

**Interfaces:**
- Consumes: `OZG_DealCard.ReadBy`, `Look`, `Sign` (Task 5).
- Produces: `class OZG_WRead { int Seat; int Look; string Face; int Sign; static OZG_WRead Make(int seat, int look, string face, int sign); }`; `OZG_WView.Reads` (`ref array<ref OZG_WRead>`, never null on a client after `OZG_Table.Take`); `void OZG_Game.ReadsFor(int seat, string uid, array<ref OZG_WRead> into)`. `Seat` is the place's seat number of the hand the card is in, or -1 for the top of the stock.

- [x] **Step 1: A read on the wire**

In `OZG_Wire.c`, before `// The head of a view.`, add:

```c
// A card this client's player reads where it lies back up for that player
// (design §4.4): in the hand of the seat `Seat`, or on top of the stock
// (`Seat` -1). The look says which back of that hand it is drawn on; the face
// is what the player knows; the sign is the fault they see in the back.
class OZG_WRead
{
    int    Seat;
    int    Look;
    string Face;
    int    Sign;

    static OZG_WRead Make(int seat, int look, string face, int sign)
    {
        OZG_WRead r = new OZG_WRead();
        r.Seat = seat;
        r.Look = look;
        r.Face = face;
        r.Sign = sign;
        return r;
    }
}
```

In `class OZG_WView`, after `float  Reach;` add:

```c
    // What this client's player reads (design §4.4); empty for a watcher and
    // for a seat that reads nothing. Made apart for every player.
    ref array<ref OZG_WRead> Reads;
```

and in its constructor, after `Log = new array<string>();`, add `Reads = new array<ref OZG_WRead>();`.

- [x] **Step 2: The game says what a seat reads**

In `OZG_Game.c`, after `SeatView(int seat)`, add:

```c
    // The cards the seat `seat`, held by the player `uid`, reads where they
    // lie back up for that player: in the other seats' hands and on top of
    // the stock (card sharp's design §4.4). A game without hidden cards says
    // nothing.
    void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
    }
```

In `OZG_Durak.c`, after `SeatView`, add:

```c
    override void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
        if (!m_S || uid == "")
            return;
        for (int i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat other = m_S.SeatList[i];
            if (other.SeatNo == seat)
                continue;
            for (int h = 0; h < other.Held.Count(); h++)
                ReadOne(other.SeatNo, other.Held[h], uid, into);
        }
        // The top of the stock; the last card of the stock is the trump, face
        // up already.
        if (m_S.StockCards.Count() > 1)
            ReadOne(-1, m_S.StockCards[0], uid, into);
    }

    protected void ReadOne(int where, string card, string uid, array<ref OZG_WRead> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.ReadBy(uid))
            return;
        into.Insert(OZG_WRead.Make(where, rec.Look, OZG_Cards.FaceOf(card), rec.Sign));
    }

    protected OZG_DealCard DealtCard(string card)
    {
        if (!m_S || card == "")
            return null;
        for (int i = 0; i < m_S.Dealt.Count(); i++)
        {
            if (m_S.Dealt[i] && m_S.Dealt[i].Name == card)
                return m_S.Dealt[i];
        }
        return null;
    }
```

- [x] **Step 3: The session puts them into the player's own view**

In `OZG_Session.c`, in `ViewBody`, after `v.Note = m_Note;` add:

```c
        // What this player reads: only a player dealt into the game, and only
        // into the head made for that player (design §4.6).
        v.Reads.Clear();
        bool dealt = seat >= 0 && m_R.Dealt.Find(seat) >= 0;
        bool reading = OZG_Cfg.Get().Cheating == 1;
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY && dealt && reading)
            m_Game.ReadsFor(seat, uid, v.Reads);
```

- [x] **Step 4: The client keeps the list whatever came**

In `OZG_Table.c`, in `Take`, after `if (!v.Log) v.Log = new array<string>();` (two lines), add:

```c
        if (!v.Reads)
            v.Reads = new array<ref OZG_WRead>();
```

- [x] **Step 5: The stand says what a seat reads**

In `OZG_WorldOps.c`, after `OpMarkOf`, add:

```c
    // reads id=<place> seat=<n>: what the view of that seat's player carries
    // as reads: "<seat>:<face>/<sign>", -1 for the top of the stock.
    static bool OpReads(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        int seat = OZG_VerbOps.Arg(args, "seat", "0").ToInt();
        if (seat < 0 || seat >= s.m_R.Seats.Count() || !s.m_Game)
        {
            detail = "reads: no seat " + seat.ToString() + " in a game";
            return true;
        }
        array<ref OZG_WRead> reads = new array<ref OZG_WRead>();
        s.m_Game.ReadsFor(seat, s.m_R.Seats[seat].Uid, reads);
        detail = "reads seat " + seat.ToString() + ":";
        if (reads.Count() == 0)
            detail = detail + " none";
        for (int i = 0; i < reads.Count(); i++)
        {
            string one = " " + reads[i].Seat.ToString() + ":" + reads[i].Face;
            detail = detail + one + "/" + reads[i].Sign.ToString();
        }
        return true;
    }
```

In `OZG_Verb.c`, after the `markof` lines, add:

```c
        if (op == "reads")
            return OZG_WorldOps.OpReads(args, detail);
```

and append `, reads` to `KnownOps()`.

- [x] **Step 6: Check the reads (spec §11.2: "a marker reads his card in a bot's hand and on the stock and no other seat's view holds it")**

`mod_lint`, a stand cycle, a client, `world_teleport "6095.4 107.65 6028.9"`. Then, `<id>` the place laid:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"reads","id":"<id>","seat":"0"}
```

Expected: `marked 52 of all`, and with every card marked the reads are every card that lies back up for seat 0: the three bots' hands of six each and the top of the stock, nineteen in all, none with seat 0's number. Compare with each bot's hand, `oz_games {"op":"hand","id":"<id>","seat":"<n>"}` for each of the bots' seats: every face it holds is in the reads with its seat's number (a copy suffix aside), and no other face is.

Then the other seats read nothing: `oz_games {"op":"reads","id":"<id>","seat":"<n>"}` for a bot's seat answers `reads seat <n>: none`.

Then a restart in mid-game keeps the reads (spec §8): `server_stop`, `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client, `world_ready`; `reads seat 0` answers the same nineteen.

Then one marked card: fold the game; lay a new one; `mark` with `face` `As`; `sit`, `bots`, `deal`; `reads seat 0`. Expected: `As` with a bot's seat when a bot holds it (`hand`), `-1:As` when it lies on top of the stock, `none` when it is in seat 0's hand, deeper in the stock or the trump.

Then `Cheating` 0 sends nothing: `server_stop`; in `E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json` set `"Cheating": 0`; `server_start`, the client, `world_teleport`. The stand's `reads` asks the game and not the view, so it still lists them; the view is seen in the window (Task 7, Step 7). Set `"Cheating": 1` back and restart.

- [x] **Step 7: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Wire.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Table.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Marks: a seated player is told the cards they read, and nobody else

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 7: What a reader sees

The window draws, on every back a reader reads, the sign as a fault in the back's own pattern and the face pencilled faintly in the top corner (spec §4.4, §4.5, §10), as the table mock-up drew them after the owner's word on its first marks (spec §0.12; memory: project-table-games; the mock-up's `make_marks.py` lies in the table mock-up's `canvas` folder, outside the repository, and writes its sheet `out\_marks_view.png` beside itself when run). The pictures are made by a script, as the cards' are; the window lays them inside the card's turning frame, so they turn with it.

**Files:**
- Create: `tools/make_marks.py`, `art/marks/SOURCES.md`
- Create (written by the script): `art/marks/_view.png`, `art/marks/classic_signs.png`, `art/marks/stalker_signs.png`, `OpenZone_Games/gui/imagesets/ozg_marks.imageset`, `OpenZone_Games/gui/imagesets/ozg_marks_ca.paa`, `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c`
- Modify: `OpenZone_Games/config.cpp` (`imageSets`)
- Modify: `tools/make_layouts.py` (`card()`); `OpenZone_Games/gui/layouts/ozg_card.layout` (written by it)
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c`

**Interfaces:**
- Consumes: `OZG_WView.Reads`, `OZG_WRead.Seat`, `Look`, `Face`, `Sign` (Task 6); `OZG_Decks.StyleAt(int look)`; the back pictures `art/cards/<style>/back.png` (200 x 280, `tools/make_cards.py`).
- Produces: `class OZG_MarkPlaces` with `static bool SignBox(string style, int sign, out int x, out int y, out int w, out int h)`, `static bool PencilBox(out int x, out int y, out int w, out int h)`, `static string SignImage(string style, int sign)`, `static string PencilImage(string face)`, `CARD_W` 200, `CARD_H` 280; the widgets `OZG_CardSign` and `OZG_CardPencil` in every card's frame; in the menu `ReadAt` and `MarkBack`.

- [x] **Step 1: The script that draws the marks**

Create `tools/make_marks.py` with the Write tool (it holds backslashes):

```python
# Draws the marks of the game window (card sharp's design §4.4, §4.5). A sign
# is a fault drawn INTO a back's own pattern, to show to whoever looks closely
# and not at a glance; the face pencilled small and faint into the top corner
# of the back is what its reader sees beside it. The kinds, their sizes and
# inks and the pencil are those of the table mock-up the owner kept on
# 2026-11-24 (its make_marks.py, outside the repository; the shapes here
# GROW larger all round, so that they cover what the mock-up's covered); the
# shade and the nail nick are the design's two more kinds.
#
#   dot    a speck of dark ink inside one cell of the lattice
#   block  block-out: a crossing of the lattice, the next one down its line
#          and the line between them painted over in the ground's colour
#   cut    cut-out: a pale fleck scraped out of the ground, as with a razor
#   shade  a cell, or two cells along a line of the lattice, a little darker
#   nick   a nail nick: a small dent in the card's left or right edge
#
# A sign is a number 1..99: (sign - 1) % 5 picks the kind and (sign - 1) // 5
# its place among twenty of that kind, so ninety-nine signs are ninety-nine
# faults. The places are drawn by lot from the back's lattice, away from its
# emblem and from the pencil's corner, with fixed seeds: the same every run.
#
# Every fault and every pencilled face is a picture of one ink on a clear
# ground, laid over the back where it was drawn, as the mock-up laid it.
#
# Usage: python -I tools/make_marks.py [--paa <ImageToPAA.exe>] [--no-paa]
#
# Reads art/cards/<style>/back.png (tools/make_cards.py). Writes
# art/marks/<style>_signs.png (the ninety-nine on that back, to look at),
# art/marks/_view.png (each kind and a read back, as the mock-up's sheet),
# build/marks/ozg_marks_ca.png, OpenZone_Games/gui/imagesets/ozg_marks.imageset
# and, given ImageToPAA, ozg_marks_ca.paa beside it, and
# OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c.
import io
import math
import os
import random
import subprocess
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
CARDS = os.path.join(REPO, "art", "cards")
ART = os.path.join(REPO, "art", "marks")
BUILD = os.path.join(REPO, "build", "marks")
SETS = os.path.join(REPO, "OpenZone_Games", "gui", "imagesets")
CODE = os.path.join(REPO, "OpenZone_Games", "scripts", "5_Mission", "OpenZone_Games", "OZG_MarkPlaces.c")
IMAGE_TO_PAA = r"E:\Programs\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"
# The mock-up's pencil: a hand and the symbols of Windows (art/marks/SOURCES.md).
HAND = r"C:\Windows\Fonts\segoepr.ttf"
SYMBOL = r"C:\Windows\Fonts\seguisym.ttf"

SET = "ozg_marks"
TEXTURE = "OpenZone_Games/gui/imagesets/ozg_marks_ca.paa"
LANCZOS = Image.Resampling.LANCZOS

W, H = 200, 280                     # a card, px (tools/make_cards.py)
CX, CY = W / 2, H / 2
S = 4                               # drawn S times larger, then reduced
STYLES = ("classic", "stalker")
KINDS = ("dot", "block", "cut", "shade", "nick")
PER_KIND = 20
SIGNS = 99
RANKS = "23456789TJQKA"
SUITS = "shdc"
FACES = [r + u for u in SUITS for r in RANKS]
SUIT_SIGN = {"s": "\u2660", "h": "\u2665", "d": "\u2666", "c": "\u2663"}

# The lattices of the backs (tools/make_cards.py, back_classic and
# back_stalker): the step between parallel lines and their slope. A crossing
# lies at (CX + i * step / 2, CY + j * step / 2 * slope) with i + j even, the
# middle of a cell where i + j is odd.
LATTICE = {"classic": (14.0, math.tan(math.radians(55))), "stalker": (9.0, math.tan(math.radians(52)))}
# The box the lattice is clipped to, and the emblem in its middle that no
# fault touches: its middle and a radius with room round it.
FIELD = {"classic": (16, 16, 184, 264), "stalker": (14, 14, 186, 266)}
EMBLEM = {"classic": (94.5, 145.5, 50.0), "stalker": (100.0, 140.0, 50.0)}
# The inks: the mock-up's dot, scrape and pencil; a shade a little darker
# than the ground; a nick in a darker edge colour.
DOT = (18, 8, 8, 235)
PALE = {"classic": (236, 226, 204, 230), "stalker": (224, 208, 172, 220)}
SHADE = (0, 0, 0, 64)
NICK = {"classic": (70, 62, 50, 235), "stalker": (54, 44, 28, 235)}
PENCIL = (22, 12, 12, 125)
ROOM = 6                            # kept clear round the emblem and the pencil
GAP = {"dot": 22, "block": 22, "cut": 22, "shade": 22, "nick": 12}
# The mock-up drew at a card's size, pixel by pixel, and Pillow counts a shape's last
# pixels in whole; drawn larger and reduced, a shape covers as much when it is this
# much larger all round, in card pixels (held against the mock-up's sheet, 2026-11-25).
GROW = 0.5


def at(style, i, j):
    step, slope = LATTICE[style]
    return CX + i * step / 2, CY + j * step / 2 * slope


def s(*v):
    """Card pixels on the S sheet."""
    return [a * S for a in v]


def sheet(ink):
    """A clear sheet a card large, S times, of one ink."""
    return Image.new("RGBA", (W * S, H * S), ink[:3] + (0,))


def reduce(big, ink):
    """The S sheet brought to a card's size: its alpha resampled, its colour the ink
    everywhere, so that neither a resampling nor a texture filter that reaches past an
    edge finds black there."""
    out = Image.new("RGBA", (W, H), ink[:3] + (0,))
    out.putalpha(big.getchannel("A").resize((W, H), LANCZOS))
    return out


def ground(back, x, y):
    """The ground's colour round a cell's middle, where no line runs (the mock-up's)."""
    px = [back.getpixel((int(x + dx), int(y + dy)))[:3] for dx in (-1, 0, 1) for dy in (-1, 0, 1)]
    return tuple(sum(c[k] for c in px) // len(px) for k in range(3))


# ---------------------------------------------------------------- the pencil

def pencil(face):
    """The face pencilled into the top corner of a back, as the mock-up wrote it: the rank
    in the hand, the suit's sign beside it a little lower."""
    rank, suit = face[0], face[1]
    text = "10" if rank == "T" else rank
    big = sheet(PENCIL)
    d = ImageDraw.Draw(big)
    hand = ImageFont.truetype(HAND, 30 * S)
    sym = ImageFont.truetype(SYMBOL, 24 * S)
    d.text((19 * S, 12 * S), text, font=hand, fill=PENCIL)
    d.text((19 * S + d.textlength(text, font=hand) + S, 20 * S), SUIT_SIGN[suit], font=sym, fill=PENCIL)
    return reduce(big, PENCIL)


def union_box(images):
    """The box all of them cover, a pixel round it: (x0, y0, x1, y1), x1 and y1 past it."""
    boxes = [im.getchannel("A").getbbox() for im in images]
    x0 = max(0, min(b[0] for b in boxes) - 1)
    y0 = max(0, min(b[1] for b in boxes) - 1)
    x1 = min(W, max(b[2] for b in boxes) + 1)
    y1 = min(H, max(b[3] for b in boxes) + 1)
    return x0, y0, x1, y1


# ---------------------------------------------------------------- places

def free(style, x, y, keep):
    x0, y0, x1, y1 = FIELD[style]
    if not (x0 + 8 <= x <= x1 - 8 and y0 + 8 <= y <= y1 - 8):
        return False
    ex, ey, er = EMBLEM[style]
    if math.hypot(x - ex, y - ey) < er:
        return False
    k0, k1, k2, k3 = keep
    return not (k0 - ROOM <= x <= k2 + ROOM and k1 - ROOM <= y <= k3 + ROOM)


def candidates(style, kind, keep):
    """Every place a fault of this kind can take: (place, the point it is judged by)."""
    out = []
    if kind == "nick":
        for side in (-1, 1):
            for y in range(60, 231, 2):
                out.append(((side, y), (CX + side * CX, float(y))))
        return out
    for j in range(-40, 41):
        for i in range(-40, 41):
            crossing = (i + j) % 2 == 0
            if kind == "block":
                if not crossing:
                    continue
                points = [at(style, i, j), at(style, i + 1, j + 1)]
            else:
                if crossing:
                    continue
                points = [at(style, i, j)]
                if kind == "shade":
                    points.append(at(style, i + 1, j + 1))
            if all(free(style, x, y, keep) for x, y in points):
                out.append(((i, j), points[0]))
    return out


def places(style, kind, keep):
    """Twenty places of a kind on a back, drawn by lot, GAP apart."""
    rng = random.Random("%s:%s" % (style, kind))
    pool = candidates(style, kind, keep)
    rng.shuffle(pool)
    chosen = []
    for place, (x, y) in pool:
        if all(math.hypot(x - cx, y - cy) >= GAP[kind] for _, (cx, cy) in chosen):
            chosen.append((place, (x, y)))
            if len(chosen) == PER_KIND:
                return [p for p, _ in chosen]
    raise SystemExit("error: %s: only %d places for a %s" % (style, len(chosen), kind))


# ---------------------------------------------------------------- the faults

def cell(style, i, j, shrink=0.66):
    """A cell of the lattice drawn smaller about its middle, clear of the lines round it."""
    x, y = at(style, i, j)
    step, slope = LATTICE[style]
    hx, hy = step / 2 * shrink, step / 2 * slope * shrink
    return s(x, y - hy, x + hx, y, x, y + hy, x - hx, y)


def fault(style, back, sign, spots):
    """The fault of a sign on a clear sheet a card large, and its kind."""
    kind = KINDS[(sign - 1) % len(KINDS)]
    slot = (sign - 1) // len(KINDS)
    place = spots[kind][slot]
    step, slope = LATTICE[style]
    if kind == "dot":
        ink = DOT
    elif kind == "block":
        ink = ground(back, *at(style, place[0] + 1, place[1])) + (255,)
    elif kind == "cut":
        ink = PALE[style]
    elif kind == "shade":
        ink = SHADE
    else:
        ink = NICK[style]
    big = sheet(ink)
    d = ImageDraw.Draw(big)
    g = GROW
    if kind == "dot":
        x, y = at(style, *place)
        d.ellipse(s(x - 3.4 - g, y - 3.0 - g, x + 3.2 + g, y + 3.4 + g), fill=ink)
    elif kind == "block":
        x, y = at(style, *place)
        dx, dy = step / 2, step / 2 * slope
        for k in (0, 1):                                # the crossing and the next one down the line
            cx, cy = x + k * dx, y + k * dy
            d.ellipse(s(cx - 3.6 - g, cy - 3.6 - g, cx + 3.6 + g, cy + 3.6 + g), fill=ink)
        d.line(s(x, y, x + dx, y + dy), fill=ink, width=round((5 + 2 * g) * S))
    elif kind == "cut":
        x, y = at(style, *place)
        d.line(s(x - 5, y + 3, x + 5, y - 3), fill=ink, width=round((3 + 2 * g) * S))
        d.ellipse(s(x + 5 - g, y - g, x + 7 + g, y + 2 + g), fill=ink)
        d.ellipse(s(x - 4 - g, y - 5 - g, x - 2 + g, y - 3 + g), fill=ink)
    elif kind == "shade":
        i, j = place
        d.polygon(cell(style, i, j), fill=ink)
        if slot % 2 == 1:                               # and the next cell along its strip
            d.polygon(cell(style, i + 1, j + 1), fill=ink)
    else:
        side, y = place
        edge = 0.0 if side < 0 else float(W)
        d.polygon(s(edge, y - 2.6, edge - side * 3.4, y, edge, y + 2.6), fill=ink)
    return reduce(big, ink), kind


def cut_out(img):
    """A picture cut to what it covers, a pixel round it, and that box (x, y, w, h)."""
    b = img.getchannel("A").getbbox()
    if not b:
        raise SystemExit("error: a mark drew nothing")
    x0, y0 = max(0, b[0] - 1), max(0, b[1] - 1)
    x1, y1 = min(W, b[2] + 1), min(H, b[3] + 1)
    return img.crop((x0, y0, x1, y1)), (x0, y0, x1 - x0, y1 - y0)


# ---------------------------------------------------------------- the sheets

def pack(pieces, size):
    """Shelves, the tallest pieces first, 2 px apart; None when they do not fit."""
    order = sorted(pieces, key=lambda n: (-pieces[n].height, n))
    x, y, row, spots = 2, 2, 0, {}
    for name in order:
        w, h = pieces[name].size
        if x + w + 2 > size[0]:
            x, y, row = 2, y + row + 2, 0
        if y + h + 2 > size[1]:
            return None
        spots[name] = (x, y)
        x, row = x + w + 2, max(row, h)
    return spots


def atlas(pieces):
    """The pieces on the smallest sheet of the three that takes them, each with a pixel of
    its own ink round it, clear."""
    for size in ((512, 512), (1024, 512), (1024, 1024)):
        spots = pack(pieces, size)
        if spots:
            break
    else:
        raise SystemExit("error: the marks do not fit a sheet of 1024 x 1024")
    out = Image.new("RGBA", size, (0, 0, 0, 0))
    d = ImageDraw.Draw(out)
    for name, (x, y) in spots.items():
        im = pieces[name]
        d.rectangle((x - 1, y - 1, x + im.width, y + im.height), fill=im.getpixel((0, 0))[:3] + (0,))
        out.paste(im, (x, y))
    return out, spots


def signs_sheet(faulted):
    """The ninety-nine at half size, ten a row, sign 1 at the top left."""
    cols, w, h, gap = 10, W // 2, H // 2, 6
    rows = (len(faulted) + cols - 1) // cols
    out = Image.new("RGB", (cols * (w + gap) + gap, rows * (h + gap) + gap), (45, 45, 45))
    for k, im in enumerate(faulted):
        small = im.resize((w, h), LANCZOS)
        out.paste(small, (gap + (k % cols) * (w + gap), gap + (k // cols) * (h + gap)), small)
    return out


def view_sheet(rows):
    """A row a style: the back, a fault of each kind, a read back; at their size and as
    small as a fan shows them, as the mock-up's sheet."""
    n = len(rows[0])
    out = Image.new("RGB", (n * 210 + n * 80, len(rows) * 300 - 10), (45, 45, 45))
    for r, shown in enumerate(rows):
        for k, im in enumerate(shown):
            out.paste(im, (k * 210, 5 + r * 300), im)
            small = im.resize((70, 98), LANCZOS)
            out.paste(small, (n * 210 + k * 80, 5 + r * 300), small)
    return out


def imageset(size, spots, pieces):
    lines = ["// GENERATED by tools/make_marks.py -- edit the script, not this file.",
             "ImageSetClass {", ' Name "%s"' % SET, " RefSize %d %d" % size, " Textures {",
             "  ImageSetTextureClass {", "   mpix 0", '   path "%s"' % TEXTURE, "  }", " }", " Images {"]
    for name in sorted(spots):
        x, y = spots[name]
        w, h = pieces[name].size
        lines += ["  ImageSetDefClass %s {" % name, '   Name "%s"' % name, "   Pos %d %d" % (x, y),
                  "   Size %d %d" % (w, h), "   Flags 0", "  }"]
    lines += [" }", " Groups {", " }", "}"]
    return "\n".join(lines) + "\n"


CODE_TOP = """// GENERATED by tools/make_marks.py -- edit the script, not this file.
//
// Where the game window draws a mark on a back (card sharp's design §4.4,
// §4.5): the box of each sign's picture in set:ozg_marks, by the style of the
// back, and the box of the pencilled face, in pixels of a 200 x 280 card. A
// style with no signs here (another mod's deck) shows the pencil alone.
class OZG_MarkPlaces
{
    static const string IMAGESET = "ozg_marks";
    static const float CARD_W = 200;
    static const float CARD_H = 280;

    protected static ref map<string, ref array<int>> s_Boxes;

    // The box of sign `sign` on a back of `style`; false: none is drawn.
    static bool SignBox(string style, int sign, out int x, out int y, out int w, out int h)
    {
        return BoxOf(style + "_s" + sign.ToString(), x, y, w, h);
    }

    // The box the pencil writes a face in, the same for every face.
    static bool PencilBox(out int x, out int y, out int w, out int h)
    {
        return BoxOf("pencil", x, y, w, h);
    }

    static string SignImage(string style, int sign)
    {
        return "set:" + IMAGESET + " image:" + style + "_s" + sign.ToString();
    }

    static string PencilImage(string face)
    {
        return "set:" + IMAGESET + " image:p_" + face;
    }

    protected static bool BoxOf(string key, out int x, out int y, out int w, out int h)
    {
        if (!s_Boxes)
            FillBoxes();
        array<int> b;
        if (!s_Boxes.Find(key, b) || !b)
            return false;
        x = b[0];
        y = b[1];
        w = b[2];
        h = b[3];
        return true;
    }

    protected static void PutBox(string key, int x, int y, int w, int h)
    {
        array<int> b = new array<int>();
        b.Insert(x);
        b.Insert(y);
        b.Insert(w);
        b.Insert(h);
        s_Boxes.Set(key, b);
    }

    protected static void FillBoxes()
    {
        s_Boxes = new map<string, ref array<int>>();
"""

CODE_END = """    }
}
"""


def code(boxes):
    keys = ["pencil"] + ["%s_s%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
    body = ['        PutBox("%s", %d, %d, %d, %d);' % ((k,) + tuple(boxes[k])) for k in keys]
    return CODE_TOP + "\n".join(body) + "\n" + CODE_END


# ---------------------------------------------------------------- the run

def main():
    paa = IMAGE_TO_PAA
    if "--paa" in sys.argv:
        paa = sys.argv[sys.argv.index("--paa") + 1]
    if "--no-paa" in sys.argv or not os.path.isfile(paa):
        paa = None
    for font in (HAND, SYMBOL):
        if not os.path.isfile(font):
            raise SystemExit("error: the pencil's font is missing: " + font)
    for folder in (ART, BUILD, SETS):
        os.makedirs(folder, exist_ok=True)

    pieces, boxes = {}, {}
    pencils = {face: pencil(face) for face in FACES}
    keep = union_box(list(pencils.values()))
    for face, img in pencils.items():
        pieces["p_" + face] = img.crop(keep)
    boxes["pencil"] = (keep[0], keep[1], keep[2] - keep[0], keep[3] - keep[1])

    rows = []
    for style in STYLES:
        back = Image.open(os.path.join(CARDS, style, "back.png")).convert("RGBA")
        if back.size != (W, H):
            raise SystemExit("error: the %s back is %s, not %d x %d" % (style, back.size, W, H))
        spots = {kind: places(style, kind, keep) for kind in KINDS}
        faulted, firsts = [], {}
        for sign in range(1, SIGNS + 1):
            full, kind = fault(style, back, sign, spots)
            name = "%s_s%d" % (style, sign)
            pieces[name], boxes[name] = cut_out(full)
            on = back.copy()
            on.alpha_composite(full)
            faulted.append(on)
            firsts.setdefault(kind, on)
        signs_sheet(faulted).save(os.path.join(ART, "%s_signs.png" % style))
        read = firsts["cut"].copy()
        read.alpha_composite(pencils["Kh"])
        rows.append([back] + [firsts[k] for k in KINDS] + [read])
        print("%s: %d signs, %d places of each kind" % (style, SIGNS, PER_KIND))
    view_sheet(rows).save(os.path.join(ART, "_view.png"))

    sheet_img, spots = atlas(pieces)
    png = os.path.join(BUILD, SET + "_ca.png")
    sheet_img.save(png)
    with io.open(os.path.join(SETS, SET + ".imageset"), "w", encoding="utf-8", newline="\n") as f:
        f.write(imageset(sheet_img.size, spots, pieces))
    with io.open(CODE, "w", encoding="utf-8", newline="\n") as f:
        f.write(code(boxes))
    print("%s: %d pictures on %d x %d; OZG_MarkPlaces.c: %d boxes"
          % (SET, len(pieces), sheet_img.size[0], sheet_img.size[1], len(boxes)))
    if paa is None:
        print("no .paa made")
        return
    target = os.path.join(SETS, SET + "_ca.paa")
    if os.path.exists(target):
        os.remove(target)
    done = subprocess.run([paa, png, target], capture_output=True, text=True, stdin=subprocess.DEVNULL)
    if done.returncode != 0 or not os.path.isfile(target):
        raise SystemExit("error: ImageToPAA failed: " + done.stdout + done.stderr)
    print("%s_ca.paa: %d KB" % (SET, os.path.getsize(target) // 1024))


if __name__ == "__main__":
    main()
```

- [x] **Step 2: Where the pictures come from**

Create `art/marks/SOURCES.md`:

```markdown
# Where the marks come from

`tools/make_marks.py` draws every picture in this folder and in
`OpenZone_Games/gui/imagesets/ozg_marks_ca.paa`: the faults from numbers, on the
backs `art/cards/<style>/back.png` that `tools/make_cards.py` draws, and the
pencil with two fonts of Windows, Segoe Print (the ranks) and Segoe UI Symbol
(the suits), as the table mock-up of 2026-11-24 wrote it. The fonts are not in
this repository; the script reads them from `C:\Windows\Fonts`.
```

- [x] **Step 3: Draw them, and hold them against the mock-up**

Run `python -I tools/make_marks.py`. Expected output (the plan's script was run so, its outputs pointed at a scratch folder, when the plan was written):

```
classic: 99 signs, 20 places of each kind
stalker: 99 signs, 20 places of each kind
ozg_marks: 250 pictures on 512 x 512; OZG_MarkPlaces.c: 199 boxes
ozg_marks_ca.paa: <n> KB
```

An `error: ... only <k> places for a <kind>` means the lattice left too little room: lower that kind's `GAP` by 2 and run again (say so in the record). Then run the mock-up's own `make_marks.py` in its `canvas` folder (`python -I make_marks.py`; it reads this repository's backs and writes only its own `out` folder) and look at `art/marks/_view.png` beside its `out\_marks_view.png`: the dot, the block-out, the cut-out and the read back (a cut-out and `K♥` pencilled) must look alike in both, at full size and small; the shade and the nick show only to an eye that looks for them. Look through `art/marks/classic_signs.png` and `stalker_signs.png`: no fault on the emblem, none across the border, none under the pencil's corner.

- [x] **Step 4: The imageset is the mod's**

In `OpenZone_Games/config.cpp` (with the Edit tool), replace

```cpp
            // One imageset a deck style: 52 faces, the back and the four
            // suits. A set that is not listed here loads as blank pictures,
            // with no error at all.
            class imageSets
            {
                files[] =
                {
                    "OpenZone_Games/gui/imagesets/ozg_cards_classic.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_cards_stalker.imageset"
                };
            };
```

with

```cpp
            // One imageset a deck style: 52 faces, the back and the four
            // suits; one more for the marks a reader sees (tools/make_marks.py).
            // A set that is not listed here loads as blank pictures, with no
            // error at all.
            class imageSets
            {
                files[] =
                {
                    "OpenZone_Games/gui/imagesets/ozg_cards_classic.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_cards_stalker.imageset",
                    "OpenZone_Games/gui/imagesets/ozg_marks.imageset"
                };
            };
```

- [x] **Step 5: A card's frame gains its sign and its pencil**

In `tools/make_layouts.py`, in `card()`, replace

```python
        image("OZG_CardFace", f(fx, fy, fw, fh), 2),
    ]
```

with

```python
        image("OZG_CardFace", f(fx, fy, fw, fh), 2),
        # A read back's mark (tools/make_marks.py): the sign and the pencilled
        # face, placed by the script in pixels inside the frame, so that they
        # turn with the card.
        image("OZG_CardSign", exact(0, 0, 1, 1), 3, None, "1 1 1 1", [("visible", 0)]),
        image("OZG_CardPencil", exact(0, 0, 1, 1), 4, None, "1 1 1 1", [("visible", 0)]),
    ]
```

Run `python -I tools/make_layouts.py`; `git status --short OpenZone_Games/gui/layouts` lists `ozg_card.layout` alone (the other layouts are written again the same).

- [x] **Step 6: The window draws the reads**

In `OZG_GameMenu.c`, after the method `OnCard`, add:

```c
    // ------------------------------------------------------------ marks

    // Which of this player's reads a back of the seat `seat` (-1: the top of
    // the stock) in the look `look` shows: the first that lies there in that
    // look and that no back drawn before took (`used`); -1: none. A fan shows
    // its backs by look and not in the hand's order, so a read lies on the
    // first free back of its look.
    protected int ReadAt(int seat, int look, array<int> used)
    {
        if (!m_V || !m_V.Reads)
            return -1;
        for (int i = 0; i < m_V.Reads.Count(); i++)
        {
            OZG_WRead r = m_V.Reads[i];
            if (r.Seat != seat || r.Look != look)
                continue;
            if (used.Find(i) >= 0)
                continue;
            used.Insert(i);
            return i;
        }
        return -1;
    }

    // A read on a back of w x h units: the sign where its fault lies and the
    // face in pencil in the top corner (design §4.4), inside the card's frame
    // so that they turn with it. The places are pixels of a 200 x 280 card
    // (OZG_MarkPlaces, tools/make_marks.py); the face lies in the middle of
    // the frame, 5 : 7, as tools/make_layouts.py lays it.
    protected void MarkBack(Widget card, float w, float h, OZG_WRead r)
    {
        ImageWidget sign = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        ImageWidget pencil = ImageWidget.Cast(card.FindAnyWidget("OZG_CardPencil"));
        if (!sign || !pencil)
            return;
        float side = BoxOf(w, h) * m_K;
        float d = CARD_BOX * Math.Sqrt(74);
        float faceW = side * 5 / d;
        float faceH = side * 7 / d;
        float left = (side - faceW) * 0.5;
        float top = (side - faceH) * 0.5;
        float kx = faceW / OZG_MarkPlaces.CARD_W;
        float ky = faceH / OZG_MarkPlaces.CARD_H;
        int x;
        int y;
        int bw;
        int bh;
        string style = OZG_Decks.StyleAt(r.Look);
        if (OZG_MarkPlaces.SignBox(style, r.Sign, x, y, bw, bh))
        {
            sign.LoadImageFile(0, OZG_MarkPlaces.SignImage(style, r.Sign));
            sign.SetImage(0);
            sign.SetPos(left + x * kx, top + y * ky);
            sign.SetSize(bw * kx, bh * ky);
            sign.Show(true);
        }
        if (OZG_MarkPlaces.PencilBox(x, y, bw, bh))
        {
            pencil.LoadImageFile(0, OZG_MarkPlaces.PencilImage(r.Face));
            pencil.SetImage(0);
            pencil.SetPos(left + x * kx, top + y * ky);
            pencil.SetSize(bw * kx, bh * ky);
            pencil.Show(true);
        }
    }
```

Replace

```c
    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks)
    {
```

with

```c
    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first), the hand of the seat `seat`: a
    // back this player reads there carries its mark (design §4.4).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks, int seat)
    {
        array<int> used = new array<int>();
```

and in its loop replace

```c
            string back = PictureOf("back", LookAt(looks, i));
            Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true);
```

with

```c
            int look = LookAt(looks, i);
            string back = PictureOf("back", look);
            Widget card = Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true);
            int readNo = ReadAt(seat, look, used);
            if (card && readNo >= 0)
                MarkBack(card, w, h, m_V.Reads[readNo]);
```

In `PaintSeat`, replace `PaintFan(spot.X, spot.Y, spot.Turn, spot.Scale, g.CardCount, g.Backs);` with `PaintFan(spot.X, spot.Y, spot.Turn, spot.Scale, g.CardCount, g.Backs, no);`. In `PaintBottom`, replace `PaintFan(m_Look.HandX, m_Look.HandY + 20, 0, 1, g.CardCount, g.Backs);` with `PaintFan(m_Look.HandX, m_Look.HandY + 20, 0, 1, g.CardCount, g.Backs, no);` (a watcher's view carries no reads; the seat is passed all the same).

In `PaintTable`, before `for (k = 0; k < layers; k++)` add `array<int> stockUsed = new array<int>();`, and in that loop replace

```c
            Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true);
```

with

```c
            Widget layer = Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true);
            // The top card: a back this player reads carries its mark (design §4.4).
            int topRead = -1;
            if (k == layers - 1)
                topRead = ReadAt(-1, look, stockUsed);
            if (layer && topRead >= 0)
                MarkBack(layer, pw, ph, m_V.Reads[topRead]);
```

Before the build, `knowledge_find(kind="class", name=...)` for `ReadAt`, `MarkBack`, `BoxOf`, `PutBox`, `FillBoxes`, `SignBox`, `PencilBox`: none may be a class of the game or of a loaded mod.

- [x] **Step 7: Lint, build, look (spec §11.2: "a marker reads his card in a bot's hand and on the stock")**

`mod_lint`, a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
```

Open the game's window (the client presses `j`: `client_type` with `j`) and `client_shot`. Crop the shot round each bot's fan and round the stock (Pillow in the scratchpad) and go over them beside `art/marks/_view.png` and the mock-up's `_marks_view.png`, element by element, before anything is shown to the owner (memory: feedback-game-ui-match-mock):
1. Every back of every bot's fan, and the top of the stock, carries one fault and a pencilled face; the faces are those the bots hold (`oz_games {"op":"hand","id":"<id>","seat":"<n>"}`), one pencil a card, in the look order of the fan.
2. A fault lies in the lattice as the sheet drew it: a speck in a cell, a block-out over a crossing, a scrape, a shaded cell, a nick at the card's edge; none on the emblem, none cut off at a card's edge, none moved off the lattice by a pixel or more.
3. The pencil sits in the card's own top corner, inside its border, faint; at the top seats (turned 180) it is upside down with its card, at the side seats (-90, 90) turned with it.
4. The whole shot, not cropped: at a glance the table is a table of plain backs.

Then:
- Two marked cards: fold the game, lay a new one, `mark` `As` and `Kh`, `sit`, `bots`, `deal`, the window: at most the marked ones carry a mark (`client_shot`).
- The stalkers' deck on a crowded table: fold; lay with `"deck":"OZG_Deck_Stalker"`; `mark` `all`; `sit`; `bots` with `n` 5 (six seats: the side fans are smaller); `deal`; the window, `client_shot`, points 1 to 3 again on the stalkers' lattice.
- `Cheating` 0 (Task 6, Step 6 leaves it to here): set it, restart, the same game comes back with its marked cards; the window shows no mark at all (`client_shot`). Set it back to 1 and restart.
- The owner's screen, if `client_start` gives a 3840 x 1600 window (`docs/2026-11-25-table-window-record.md` says how the table was shot at that size): one shot of a fan and the stock.

Send the key shots to the owner (`SendUserFile`): a whole table, the crops, the crowded stalkers' table.

- [x] **Step 8: Commit**

```bash
git add tools/make_marks.py art/marks OpenZone_Games/gui/imagesets/ozg_marks.imageset OpenZone_Games/gui/imagesets/ozg_marks_ca.paa OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c OpenZone_Games/config.cpp tools/make_layouts.py OpenZone_Games/gui/layouts/ozg_card.layout OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c
git commit -m "Marks: a reader sees the sign and the pencilled face on the backs read

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 8: The record

What was built and measured goes where the next reader looks for it: the record, the commands to try it, the README, the design's status, the two skills, the memory.

**Files:**
- Create: `docs/2026-11-25-marks-record.md`
- Modify: `docs/test-commands.md`, `README.md`, this plan (its ticks)
- Outside the repository: `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`; the skills `E:\dayzmod\dayz-agentic-modding-mcp\skills\dayz-modding` and `E:\dayzmod\dayz-modding-skill\openzone`; the memory `project-table-games.md`

- [x] **Step 1: The record**

Create `docs/2026-11-25-marks-record.md` in the shape of `docs/2026-11-25-table-window-record.md`: a heading `# Marks (the card sharp's phase 2): what was built and measured`, the date, and these sections, each filled from the steps' answers and never from this plan's expectations:
- **Built**: a line a task, with its commit.
- **Measured**: the answers of Task 3, Step 7 (whether the drag combines; the two holds and what they bracket; whether a deck's slots show while the deck is in a pocket, and whether a card marked there stays in its slot); the uses of Task 3, Step 6; the loupe's holds (Task 4, Step 6); the reads and the restart (Task 6, Step 6); the size of the marks' sheet and of its `.paa` (Task 7, Step 3); every `GAP` that had to be lowered.
- **Where the build differs from the design**: the stand-in models (the pen, the pistol scope), with the sizes and the weights this plan chose; the reads in the head of a player's view, not in the seat's private half (§4.6, §9; Task 6 says why); the loupe's studies written to the server's log beside the markings (§6.1 asks it of the markings only); a read drawn on the first back of its look in a fan, the fan being in look order; what else came up.
- **Not done**: noticing (phase 3), the sleights and the dealer's choice (phase 4), the tool's and the loupe's own models (§12).

- [x] **Step 2: What a person tries**

In `docs/test-commands.md`, before `## The verb for scripts`, add:

```markdown
## Marks and the loupe

The pen marks a card, the loupe tells whether a card is marked; both through the
game's crafting: the tool in your hands, a card you carry dragged onto it, then hold
the left button (the pen 8 s, the loupe 5 s; `MarkSeconds2`, `LoupeSeconds`).

    oz_games {"op":"give","what":"OZG_MarkTool"}        the pen, into the hands
    oz_games {"op":"give","what":"OZG_Loupe"}           the loupe, into the hands
    oz_games {"op":"give","what":"OZG_Card_As"}         a card, into the inventory when the hands are full

Without the gesture, the server's half of it:

    oz_games {"op":"craft","what":"mark","face":"As"}   the thing in the hands on the ace of spades you carry
    oz_games {"op":"craft","what":"loupe","face":"As"}
    oz_games {"op":"markof","in":"me","face":"all"}     every marked card you carry: sign, by you, read by you

At the table, with the window open, every back you read in another seat's fan and on
the stock carries its fault and your pencil:

    oz_games {"op":"mark","id":"<id>","face":"all"}     every card of a laid-out game marked by you
    oz_games {"op":"reads","id":"<id>","seat":"0"}      what seat 0's player reads now
```

- [x] **Step 3: The README**

In `README.md`, in the status list, after the bullet `- **A card is a thing too**: ...` (the whole bullet), add:

```markdown
- **Marks**: a card sharp's pen marks a card through the game's crafting, a loupe tells
  whether a card is marked; whoever marked a card, or studied it, sees its face pencilled
  on its back wherever it lies back up in a game, with the mark drawn into the back's
  pattern, and nobody else is told anything. The pen and the loupe wear the game's own
  models for now.
```

and in `## Українською`, after the line on the cards as things (the same place in that list), add:

```markdown
- **Крап**: ручка шулера мітить карту через звичайне крафтування гри, лупа показує, чи
  карта мічена; хто карту позначив або роздивився, бачить її лице олівцем на сорочці,
  де б вона не лежала сорочкою догори, і саму мітку в узорі сорочки; більше нікому
  нічого не кажуть. Ручка й лупа поки що мають моделі самої гри.
```

- [x] **Step 4: The design's status**

In `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (not a repository): the status line says phase 2 is built and checked on the stand, with the date and the record's path; §4.2's two "to be measured" get their answers from the record; §12's line on the models says the stand-ins.

- [x] **Step 5: The skills**

Each fact measured here about the engine goes into the public skill with neutral names (`MyMod_`, "a stand mod"): into `references/items-and-processing.md`, by the recipes, what Task 3 measured (a recipe that makes nothing and changes an ingredient in `Do`; `GetLengthInSecs` overridden for a length from settings, and the gesture it gave; whether an ingredient in an attachment slot of a container in a pocket can be dragged onto the item in the hands); into `references/gui-layouts.md`, by §38, that children of exact flags placed by `SetPos`/`SetSize` in pixels turn with their frame too (Task 7). The private `openzone` skill gets the pointers to our code (`OZG_Recipes.c`, `tools/make_marks.py`, `OZG_GameMenu.MarkBack`), in `references/ui.md` and `references/networking-and-items.md`. Then `python -I openzone/scripts/check-generic.py` from `E:\dayzmod\dayz-modding-skill` prints nothing. The skill repositories hold other people's uncommitted changes: stage only this task's hunks, commit each repository once, do NOT push.

- [x] **Step 6: The memory**

`C:\Users\56869\.claude\projects\E--openzone\memory\project-table-games.md` gains a paragraph: phase 2 built (the date, the commits), what was measured, what is open; its `description` line says so. `MEMORY.md` stays under 17.1 KB.

- [x] **Step 7: Commit**

```bash
git add docs/2026-11-25-marks-record.md docs/test-commands.md README.md docs/2026-11-25-marks-plan.md
git commit -m "Marks: the record, the commands to try them, the README

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

Do NOT push: the owner alone gives the push word.
