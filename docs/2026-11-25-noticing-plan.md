# Noticing (the card sharp's design, phase 3) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A player who is dealt, deal after deal, marked cards he neither reads nor notices begins after a grace of deals to notice them, one card a success, by a chance that grows with the deals; nothing is said to him; from then on the card's back shows him its sign drawn into the pattern and its face a small dark speck in a margin, only in his own view; who noticed what stays on the card through restarts, the count of deals does not.

**Architecture:** Three settings (`NoticeGrace`, `NoticeStep`, `NoticeMax`, file version 4) give the chance `min(NoticeMax, max(0, deals - NoticeGrace) * NoticeStep)`. The session keeps, in memory, a count of counted deals per player id; right after each deal, before the first view goes out, it counts and rolls for every player dealt in, and on a success makes him a noticer of one such card picked at random: on the card item (`OZG_Card_Base.OZG_AddNoticer`, saved since phase 2's `CARD_SAVE` 2, capped by `MarkKnowers`) and on the deal's record in the game (`OZG_Game.AddNoticer`), with a line in the server's log. The game says what a player notices where it lies for him (`OZG_Game.NoticesFor`: backs where a read would lie, faces in his hand, on the table and the trump), the session puts it into the head made for that player alone (`OZG_WView.Notices`, beside phase 2's `Reads`), and the window draws a noticed back's sign with phase 2's art and a noticed face's speck, a new picture of `ozg_marks` placed by `tools/make_marks.py` at the sign's own place in the face's margin.

**Tech Stack:** Enforce Script (DayZ 1.30, diag build), Python 3 with Pillow (`tools/make_marks.py`), the `dayz` MCP server (`mod_lint`, `mod_build`, `server_start`, `log_verdict`, `log_tail`, `world_exec`, `client_*`), the stand's verb `oz_games`.

**The design:** `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`: §5 (noticing), §4.1 (what a card keeps), §4.5 (what a noticer sees), §4.6 (nothing leaks), §7 (`NoticeGrace` 5, `NoticeStep` 4, `NoticeMax` 40), §8 (who noticed what survives on the cards, the count of deals does not), §9 (a seat's view gains the cards it notices, with their signs), §10 (on a back the sign in the pattern, on a face a speck in a margin), §11.3 (checked by script with the grace at 0 and the chance at 100, then with the defaults over many deals). Phase 2, which this builds on, is `docs/2026-11-25-marks-plan.md` and `docs/2026-11-25-marks-record.md`; the sleights (§6) are phase 4.

## Global Constraints

- **English and Ukrainian only** in the repository: code, comments, commit messages, docs. No Russian anywhere.
- **Work in `main`**, no branches. **Do NOT push; the owner alone gives the push word.** No remote is added, nothing goes to the Workshop.
- Commit author is the repository's own config (`Alexandru Covalschi`). Every commit message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- **No DayZ API is invented.** Every engine call of this plan is in "What the game's own scripts say" below, read in `.dayz-mcp/corpus/scripts`. A call that does not compile is looked up with `knowledge_find`, never guessed at.
- **Enforce Script rules that bite here**: no ternary; a compound boolean goes into a local before it goes into a field; a local is never named `out`, `owned` or `local`; a field or a method is never named like a class of the game or of another mod (the wire's card name is `CardName`, not `Card`: a card game mod may declare `class Card`): before the first build of a task, `knowledge_find(kind="class", name=<the new name>)` for every new name of the task; one expression holds about ten parts at most, so long strings are built in steps; `Length()` and `Substring()` count bytes.
- **A card's name has no fixed width** (`As`, `As.1`, `As.12`): `OZG_Cards.FaceOf`, `RankOf`, `SuitOf` take it apart, nothing else.
- **The JSON loader runs no initialiser for a nested object, and a member the file did not carry is raw memory** (the core's rule): every record read from JSON is made again by script, as phase 2 does (`OZG_DealCard.Copy`); every wire entry is written with all its members.
- **Nothing of noticing reaches anybody but the noticer** (design §4.6): a notice travels only in the head of the view made for that player; a notice on a back never carries the card's name or face; the player is told nothing (no notice, no line in the game's log `m_R.Log`); the server's log names him, for the admin. A player's id never leaves the server.
- **Files.** `core.autocrlf` is true and many sources are CRLF in the working tree (`git ls-files --eol` says `w/crlf` for `OZG_Settings.c`, `OZG_Cards.c`, `OZG_Game.c`, `OZG_Durak.c`, `OZG_Wire.c`, `OZG_Session.c`, `OZG_Verb.c`; git keeps them LF). Edit every source with the Edit tool, never with `sed -i` (it strips the CR); a Python script that carries a backslash is written with the Write tool, not a Bash heredoc.
- **The stand.** A stand cycle is `client_stop` (when a client runs), `server_stop`, `mod_build`, `job_wait`, `server_start(timeout=600)`, `job_wait`, `log_verdict(since=...)` which must say `pass`, `world_ready`. A client, when a step needs one: `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. End every working session with `client_stop`. After any edit of a `.toml`, `project_open` again.
- **The stand may be in the owner's hands.** Ask before the first cycle of a working session which stand is free; never restart a stand the owner is playing on without the owner's word.
- **The checks are written for the NHChernobyl stand** (world 84, the active MCP profile since 2026-11-24): a place for seven seats at `6097.4 107.65 6028.9` (`lay` with `mode=2`, `from` two metres west). A server holds the map only near players: the client is connected and its player stands within a few metres (`world_teleport "6095.4 107.65 6028.9"`). The stand's bots do not survive a restart: their seats stay kept, and a game that needs them again is folded and laid anew.
- **Stand calls** are written `oz_games {...}`: the MCP tool `world_exec` with `verb="oz_games"` and those `args`. Every value of `args` is a string. What is compared is the answer's `detail` (the bridge cuts it at 400 characters).
- **Settings at run time on the stand**: `oz_games {"op":"noticecfg",...}` (Task 1) sets the three settings of noticing in the server's memory only; a restart reads the file again, whose values stay the defaults.
- **Screenshots**: every stand run that has something on screen ends with its key shots sent to the owner (`SendUserFile`). Before a shot: the client window moved clear of the taskbar and the monitor on (a black shot means it is off: ask the owner).
- **The spec's numbers are copied, not re-decided**: `NoticeGrace` 5, `NoticeStep` 4, `NoticeMax` 40; the chance `min(NoticeMax, max(0, deals - NoticeGrace) * NoticeStep)` percent; on a success **one** card, picked at random among those he neither reads nor notices; nothing is said to him; the noticer is kept on the card, capped by `MarkKnowers` (16) with the oldest noticer giving way; the count of deals lives with the session and is lost on a restart.

## What the game's own scripts say

Read for this plan in `.dayz-mcp/corpus/scripts` (the 1.30 scripts the `dayz` MCP indexes; the paths are relative to it). Phase 3 calls nothing of the engine that phases 1 and 2 did not call already:

- `proto static int Math.RandomInt(int min, int max)` (`1_core/proto/enmath.c:38`): `min` inclusive, `max` **exclusive**; `OZG_Random.Below(100)` is therefore 0..99, and a roll succeeds when it is below the chance in percent (0 never, 100 always). `proto static float Math.Sqrt(float val)` (`enmath.c:305`).
- `static bool JsonFileLoader<T>.LoadData(string string_data, out T data, out string errorMessage)` (`3_game/tools/jsonfileloader.c:69`) and `static bool MakeData(T inputData, out string outputData, out string errorMessage, bool prettyPrint = true)` (`:85`): the head of a view is made and read so already (`OZG_Session.ViewBody`, `OZG_Table.Take`).
- `ImageWidget.LoadImageFile(int num, string name, bool noCache = false)` (`1_core/proto/enwidgets.c:257`), `ImageWidget.SetImage(int num)` (`:267`), `Widget.SetPos(float x, float y, bool immedUpdate = true)` (`:140`), `Widget.SetSize(float w, float h, bool immedUpdate = true)` (`:141`), `Widget.Show(bool show, bool immedUpdate = true)` (`:125`), `Widget.FindAnyWidget(string pathname)` (`:169`): as `OZG_GameMenu.MarkBack` uses them. Pictures with exact flags placed by `SetPos`/`SetSize` in pixels inside a turned card frame turn with it (measured in phase 2, the record's "The window").
- The transport: the core cuts every body into parts of `OZ_Const.RPC_STR_CHUNK` (900) bytes (`openzone-core/.../OZ_Rpc.c:206-222`), so a longer head travels whole; the core measured that `Substring` silently cuts its OUTPUT at 8191 bytes (`OZ_Rpc.c:228-231`), which `OZG_Text.Cut3` uses to part the three texts: the head must stay well under 8 KB. A deal of 36 cards gives a player 36 reads and notices at the most (every card lies in one place), about 1.7 KB more; Task 4 measures the head.

Pillow, for the speck's places (Task 5): `ImageFilter.MedianFilter`, `MinFilter`, `MaxFilter`, `ImageChops.subtract`, `lighter`, `darker`, `Image.point`, `Image.paste(colour, None, mask)`. The script of Task 5 was run so, its outputs pointed at a scratch copy of the repository, when this plan was written; the numbers it printed are the expected output of Task 5, Step 3.

## What this plan decides where the design is silent

1. **Which cards count.** "The deck held at least one marked card" is read as: one of the cards of the deal, the cards the game is handed (`CardsUsed`; for durak the sixes to the aces). A marked two in a durak deck, or a joker, is never dealt and never seen: it neither counts a deal nor can be noticed.
2. **The count is not reset by a success.** The design counts "how many deals he has been dealt in while ..." and says nothing of a reset: after the grace a player goes on noticing, at a chance of at most `NoticeMax` percent a deal, while marked cards he neither reads nor notices are dealt to the table. A deal with none of them is not counted.
3. **When the roll is.** Right after the deal, before the session sends anybody the deal, so that a card noticed now is in the first view of that deal ("after it is dealt"; "one back looks different from now on").
4. **Where the noticer goes.** On the card item (persistent; `MarkKnowers` cap, the oldest noticer giving way) and on the game's record of that card for this deal (so his view shows it at once, and the session file keeps it through a restart in mid-game). A reader is never made a noticer: the session does not count a card he reads, and the item refuses him.
5. **What a noticer is shown.** On a back -- the other seats' hands and the top of the stock, where a read lies -- a card he notices and does not read (a reader already sees the sign and his pencil). On a face -- his own hand, the table's attack and defence cards, the trump while it lies under the stock -- every card he notices, reader or not. Not the discards (a heap of backs, one for two cards) nor the stock below its top. A player who reads a card but does not notice it sees no speck on its face.
6. **The wire.** One list in the head of the player's own view, `OZG_WView.Notices` of `OZG_WNotice {Seat, Look, CardName, Sign}`, beside phase 2's `Reads` and for its reason (the game makes its private half by seat and knows no players; a notice belongs to the player): a back's entry carries the seat (-1 the stock's top), the look and the sign, never the card; a face's entry carries the card's name, which the player sees anyway, so the window finds that very card (`Seat` -2). The only string of an entry is that name, a few bytes; the head as a whole travels in the core's parts of 900 bytes and stays under 8 KB (Task 4 measures it).
7. **The speck.** One picture for every sign, put on a line 6 px in from the card's edge (of a 200 x 280 card) at the place the sign's number picks among 99 places per style, spread evenly over the stretches of that line where no face of the style carries ink and dealt to the signs by lot with a fixed seed: the same sign is the same place on every face, and two signs a player notices sit apart. It is new art in `tools/make_marks.py` and the same imageset `ozg_marks`, because it must be placed as the signs are (pixels of a card inside the turning frame, by style) and be round and soft-edged as the back's speck of ink; a widget without a picture draws a square. It goes into the frame's `OZG_CardSign` picture, which a face never uses: no layout changes. Its size and ink are this plan's (radius 2.2 px and the back's dark ink), as the pencil's were phase 2's: the owner may want them otherwise.
8. **The bounds.** `NoticeGrace` below 0, `NoticeStep` or `NoticeMax` outside 0..100: the default and a warning, as every setting of the series. 0 is allowed for each (a step or a most of 0 turns noticing off and leaves marking on). No client is told any of the three.
9. **The admin's trail.** A line in the server's log for every noticing (the player's id, the face, the sign, the count, the chance, the place) and a debug line for every roll; nothing in the game's log, nothing to the player.
10. **Cheating 0** stops the counting, the roll and the notices in views, as it stops the reads; noticers already on cards stay on them.
11. **The stand's bots** are players to the session: they count, roll and notice. A check that needs one noticer only makes the bots readers of every marked card first (`mark ... who=<bot>`).

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Settings.c` | `NoticeGrace`, `NoticeStep`, `NoticeMax`, file version 4, their bounds, `NoticeChance` | modify |
| `.../3_Game/.../OZG_Cards.c` | `OZG_DealCard.NoticedBy` | modify |
| `.../3_Game/.../OZG_Game.c` | `AddNoticer`, `NoticesFor` (a game without hidden cards says nothing) | modify |
| `.../3_Game/.../OZG_Durak.c` | durak's `AddNoticer` and `NoticesFor` | modify |
| `.../3_Game/.../OZG_Wire.c` | `OZG_WNotice`, `OZG_WView.Notices` | modify |
| `.../4_World/.../OZG_CardThings.c` | the noticers on the card item; a deal's card found again in the place | modify |
| `.../4_World/.../OZG_Session.c` | the count, the roll, the noticer on item and record; the notices into the player's own head; `HeadFor` for the stand | modify |
| `.../5_Mission/.../OZG_Table.c` | the client keeps the list whatever came | modify |
| `tools/make_marks.py` | the speck, its 99 places a style, its sheets | modify |
| `art/marks/SOURCES.md` | where the speck's places come from | modify |
| `art/marks/_view.png`, `art/marks/classic_specks.png`, `art/marks/stalker_specks.png`, `OpenZone_Games/gui/imagesets/ozg_marks.imageset`, `ozg_marks_ca.paa`, `.../5_Mission/.../OZG_MarkPlaces.c` | written by `make_marks.py` | modify, create |
| `tools/make_layouts.py` | the comment over a card frame's sign and pencil (the layout itself does not change) | modify |
| `.../5_Mission/.../OZG_GameMenu.c` | a noticed back's sign; a noticed face's speck in the hand, on the table, on the trump | modify |
| `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c`, `OZG_Verb.c` | stand ops `noticecfg`, `noticer`, `whole`, `redeal`, `notices`; `markof` tells noticers | modify |
| `docs/2026-11-25-noticing-record.md`, `docs/test-commands.md`, `README.md` | what was measured, what a person tries | create, modify |

---

### Task 1: The settings of noticing

Three settings of the design (§7), the settings file at version 4 with the defaults given to a file of version 3, their bounds, and the chance they make (§5). A client needs none of them: the server alone counts and rolls, so `OZG_ClientCfg` does not change. The stand gets an op to set them in the server's memory for a check.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Settings.c` (class `OZG_Settings`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (an op at the end of the class)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c` (dispatch and `KnownOps`)

**Interfaces:**
- Produces: `OZG_Settings.NoticeGrace`, `NoticeStep`, `NoticeMax` (ints, server only), `int OZG_Settings.NoticeChance(int deals)` (percent, 0..100). Stand op `noticecfg [grace=] [step=] [max=]`.

- [x] **Step 1: The fields**

In `OZG_Settings.c`, in `class OZG_Settings`, replace

```c
    int   LoupeSeconds;
    int   MarkKnowers;

    void OZG_Settings()
```

with

```c
    int   LoupeSeconds;
    int   MarkKnowers;
    // Noticing (card sharp's design §5, §7): the deals before a player can
    // notice a mark he does not read, the percent his chance grows by every
    // deal after them, and the most it grows to. The server rolls; a client
    // is told none of the three.
    int   NoticeGrace;
    int   NoticeStep;
    int   NoticeMax;

    void OZG_Settings()
```

- [x] **Step 2: The version, the defaults, the migration, the bounds**

Replace

```c
    // 2: the marks. 3: squatting is SFP's squat where SFP runs.
    override int LatestVersion()
    {
        return 3;
    }
```

with

```c
    // 2: the marks. 3: squatting is SFP's squat where SFP runs. 4: noticing.
    override int LatestVersion()
    {
        return 4;
    }
```

Replace

```c
        LoupeSeconds = 5;
        MarkKnowers = 16;
    }

    override bool Migrate(int from)
```

with

```c
        LoupeSeconds = 5;
        MarkKnowers = 16;
        NoticeGrace = 5;
        NoticeStep = 4;
        NoticeMax = 40;
    }

    override bool Migrate(int from)
```

In `Migrate`, replace

```c
        Version = LatestVersion();
        return true;
    }
```

with

```c
        // Version 3 had no noticing: its defaults.
        if (from < 4)
        {
            NoticeGrace = 5;
            NoticeStep = 4;
            NoticeMax = 40;
        }
        Version = LatestVersion();
        return true;
    }
```

In `Validate`, replace

```c
        if (DebugLog != 0)
            DebugLog = 1;
    }
```

with

```c
        // Noticing: no grace below none, a percent from 0 to 100. A step or a
        // most of 0 is noticing switched off with marking left on.
        if (NoticeGrace < 0)
        {
            NoticeGrace = 5;
            warnings++;
        }
        if (NoticeStep < 0 || NoticeStep > 100)
        {
            NoticeStep = 4;
            warnings++;
        }
        if (NoticeMax < 0 || NoticeMax > 100)
        {
            NoticeMax = 40;
            warnings++;
        }
        if (DebugLog != 0)
            DebugLog = 1;
    }
```

- [x] **Step 3: The chance**

Replace

```c
    bool GameOn(string gameId)
    {
        return GameIds.Find(gameId) >= 0;
    }
}
```

with

```c
    bool GameOn(string gameId)
    {
        return GameIds.Find(gameId) >= 0;
    }

    // The chance, in percent, that a player notices one more marked card
    // after `deals` deals that counted (design §5):
    // min(NoticeMax, max(0, deals - NoticeGrace) * NoticeStep).
    int NoticeChance(int deals)
    {
        int over = deals - NoticeGrace;
        if (over < 0)
            over = 0;
        int chance = over * NoticeStep;
        if (chance > NoticeMax)
            chance = NoticeMax;
        return chance;
    }
}
```

- [x] **Step 4: The stand sets them for a check**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before its closing brace, after `OpCraft`), add:

```c

    // noticecfg [grace=] [step=] [max=]: the settings of noticing (design
    // §7) set on the running server, through the settings' own bounds, for a
    // check: the file is not written, and a restart reads it again. Answers
    // the three as they stand and how many the bounds put right.
    static bool OpNoticeCfg(map<string, string> args, out string detail)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        string grace = OZG_VerbOps.Arg(args, "grace", "");
        string rise = OZG_VerbOps.Arg(args, "step", "");
        string most = OZG_VerbOps.Arg(args, "max", "");
        if (grace != "")
            cfg.NoticeGrace = grace.ToInt();
        if (rise != "")
            cfg.NoticeStep = rise.ToInt();
        if (most != "")
            cfg.NoticeMax = most.ToInt();
        int putRight;
        cfg.Validate(putRight);
        detail = "notice grace=" + cfg.NoticeGrace.ToString() + " step=" + cfg.NoticeStep.ToString();
        detail = detail + " max=" + cfg.NoticeMax.ToString() + " putright=" + putRight.ToString();
        return true;
    }
```

In `OZG_Verb.c`, before the line `        // -- ops of the later phases go above this line --`, add:

```c
        if (op == "noticecfg")
            return OZG_WorldOps.OpNoticeCfg(args, detail);
```

and in `KnownOps()` replace `, mark, markof, craft, reads";` with `, mark, markof, craft, reads, noticecfg";`.

- [x] **Step 5: Lint, build, check**

Before the build, `knowledge_find(kind="class", name=...)` for `NoticeGrace`, `NoticeStep`, `NoticeMax`, `NoticeChance`, `OpNoticeCfg`: none may be a class. Run `mod_lint`: no refusals. A stand cycle (Global Constraints). Then:

1. Read `E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json`. Expected: `"Version": 4`, `"NoticeGrace": 5`, `"NoticeStep": 4`, `"NoticeMax": 40`, and every value it held before kept (the poses, `Cheating` 1, `MarkKnowers` 16 ...). `log_tail(pattern="migrated games settings", n=600)` holds `migrated games settings v3 -> v4`; `log_verdict` says `pass` with no settings warning.
2. `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, then in order:

```
oz_games {"op":"noticecfg"}
oz_games {"op":"noticecfg","grace":"-1","step":"101","max":"150"}
oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}
oz_games {"op":"noticecfg","grace":"5","step":"4","max":"40"}
```

Expected: `notice grace=5 step=4 max=40 putright=0`; `notice grace=5 step=4 max=40 putright=3` (each out of its bounds and put back); `notice grace=0 step=100 max=100 putright=0`; `notice grace=5 step=4 max=40 putright=0`. The settings file is read again: still `"NoticeGrace": 5` (the op writes nothing).

- [x] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Settings.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Noticing: the three settings, version 4 of the file

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 2: Who noticed a card

The card keeps its noticers itself (design §4.1): the list was saved with the item since phase 2 (`CARD_SAVE` 2) and read into every deal's record (`OZG_MarkInto`), and nothing added to it. Now the item takes a noticer (capped by `MarkKnowers`, the oldest giving way, a reader refused), the deal's record answers whether an id notices it, and a deal's card can be found again in the place by its name. The stand gets an op that makes an id a noticer by hand, and `markof` tells the noticers.

**Files:**
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c` (class `OZG_Card_Base`; class `OZG_DeckRead`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c` (class `OZG_DealCard`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (`OpMarkOf`; an op at the end of the class), `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_Cfg.Get().MarkKnowers` (phase 2).
- Produces, on `OZG_Card_Base` (server): `bool OZG_Notices(string uid)`, `int OZG_NoticerCount()`, `void OZG_AddNoticer(string uid)`. `bool OZG_DealCard.NoticedBy(string uid)`. `static OZG_Card_Base OZG_DeckRead.CardNamed(OZG_Place place, string name)`. Stand op `noticer`; `markof` answers ` notices=<0|1> noticers=<n>` for each card and takes `only=notices`.

- [x] **Step 1: The item takes a noticer**

In `OZG_CardThings.c`, in `class OZG_Card_Base`, replace

```c
    // What a deal keeps of the mark.
    void OZG_MarkInto(OZG_DealCard rec)
```

with

```c
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
```

- [x] **Step 2: A deal's card found again in the place**

In `class OZG_DeckRead`, replace the end of `CardsOf` and of the class

```c
            OZG_DealCard rec = OZG_DealCard.Make(named, card.OZG_Look());
            card.OZG_MarkInto(rec);
            into.Insert(rec);
        }
        return things.Count();
    }
}
```

with

```c
            OZG_DealCard rec = OZG_DealCard.Make(named, card.OZG_Look());
            card.OZG_MarkInto(rec);
            into.Insert(rec);
        }
        return things.Count();
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
```

- [x] **Step 3: The record says who notices it**

In `OZG_Cards.c`, in `class OZG_DealCard`, replace

```c
    bool ReadBy(string uid)
    {
        if (Sign <= 0 || uid == "" || !Readers)
            return false;
        return Readers.Find(uid) >= 0;
    }
}
```

with

```c
    bool ReadBy(string uid)
    {
        if (Sign <= 0 || uid == "" || !Readers)
            return false;
        return Readers.Find(uid) >= 0;
    }

    bool NoticedBy(string uid)
    {
        if (Sign <= 0 || uid == "" || !Noticers)
            return false;
        return Noticers.Find(uid) >= 0;
    }
}
```

- [x] **Step 4: The stand makes a noticer, and markof tells them**

In `OZG_WorldOps.c`, replace the whole of `OpMarkOf`, from its comment

```c
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

with

```c
    // markof [id=<place> | in=me] face=<face>|all [who=me|<id>] [only=notices]:
    // each card of that face (all: each marked card) as its face, its sign,
    // whether `who` marked it, reads it and notices it, and how many notice
    // it. only=notices keeps the cards `who` notices, and counts them first.
    static bool OpMarkOf(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        string face = OZG_VerbOps.Arg(args, "face", "As");
        bool onlyNoticed = OZG_VerbOps.Arg(args, "only", "") == "notices";
        string list = "";
        int noticed = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card)
                continue;
            if (face == "all" && !card.OZG_Marked())
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            if (onlyNoticed && !card.OZG_Notices(uid))
                continue;
            if (card.OZG_Notices(uid))
                noticed++;
            string one = " " + card.OZG_Face() + " sign=" + card.OZG_Sign().ToString();
            one = one + " by_who=" + B(card.OZG_MarkedBy() == uid);
            one = one + " reads=" + B(card.OZG_Reads(uid));
            one = one + " notices=" + B(card.OZG_Notices(uid));
            one = one + " noticers=" + card.OZG_NoticerCount().ToString();
            list = list + one + ";";
        }
        detail = face + ":";
        if (onlyNoticed)
            detail = detail + " " + noticed.ToString() + " noticed:";
        detail = detail + list;
        return true;
    }
```

At the end of `class OZG_WorldOps` (before its closing brace), add:

```c

    // noticer [id=<place> | in=me] face=<face>|all [who=me|<id>] [many=0]:
    // `who` made a noticer of each marked card of that face (all: of every
    // marked card), as a won roll would (design §5); with many=k the ids
    // <who>1 .. <who>k are made noticers in turn. Only for the stand.
    static bool OpNoticer(map<string, string> args, out string detail)
    {
        array<EntityAI> things = new array<EntityAI>();
        if (!CardsIn(args, things, detail))
            return false;
        string uid;
        Who(args, uid);
        if (uid == "")
        {
            detail = "noticer: nobody to notice it";
            return true;
        }
        string face = OZG_VerbOps.Arg(args, "face", "As");
        int many = OZG_VerbOps.Arg(args, "many", "0").ToInt();
        array<string> ids = new array<string>();
        if (many <= 0)
            ids.Insert(uid);
        for (int m = 1; m <= many; m++)
            ids.Insert(uid + m.ToString());
        int done = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (!card || !card.OZG_Marked())
                continue;
            if (face != "all" && card.OZG_Face() != face)
                continue;
            for (int k = 0; k < ids.Count(); k++)
                card.OZG_AddNoticer(ids[k]);
            done++;
        }
        detail = "noticer: " + ids.Count().ToString() + " id(s)";
        detail = detail + " on " + done.ToString() + " card(s) of " + face;
        return true;
    }
```

In `OZG_Verb.c`, before `        // -- ops of the later phases go above this line --`, add:

```c
        if (op == "noticer")
            return OZG_WorldOps.OpNoticer(args, detail);
```

and in `KnownOps()` replace `, reads, noticecfg";` with `, reads, noticecfg, noticer";`.

- [x] **Step 5: Lint, build, check**

Before the build, `knowledge_find(kind="class", name=...)` for `NoticedBy`, `CardNamed`, `OpNoticer`, `OZG_Notices`, `OZG_NoticerCount`, `OZG_AddNoticer`. `mod_lint`, a stand cycle, a client, `world_teleport "6095.4 107.65 6028.9"`. Then, `<id>` the place laid, `<me>` the client's id:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"As","who":"marker1","sign":"12"}
oz_games {"op":"noticer","id":"<id>","face":"As"}
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"noticer","id":"<id>","face":"Qd"}
```

Expected: `laid <id> seats=7 kind=1`; `marked 1 of As`; `noticer: 1 id(s) on 1 card(s) of As`; `As: As sign=12 by_who=0 reads=0 notices=1 noticers=1;`; `noticer: 1 id(s) on 0 card(s) of Qd` (a clean card takes no noticer).

The record of a deal carries the noticer (phase 2 reads the list into it; this is the first time the list is not empty):

```
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"1","on":"0"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
```

Expected: `sat=1; ...`, `bots on=0 seated: <name>`, `ok; session <id> st=1 ...`.

The cap and the reader, on another card:

```
oz_games {"op":"mark","id":"<id>","face":"Kh","who":"marker1","sign":"20"}
oz_games {"op":"noticer","id":"<id>","face":"Kh"}
oz_games {"op":"noticer","id":"<id>","face":"Kh","who":"n","many":"17"}
oz_games {"op":"markof","id":"<id>","face":"Kh"}
oz_games {"op":"markof","id":"<id>","face":"Kh","who":"n1"}
oz_games {"op":"markof","id":"<id>","face":"Kh","who":"n2"}
oz_games {"op":"markof","id":"<id>","face":"Kh","who":"n17"}
oz_games {"op":"mark","id":"<id>","face":"Js","who":"me","sign":"30"}
oz_games {"op":"noticer","id":"<id>","face":"Js"}
oz_games {"op":"markof","id":"<id>","face":"Js"}
```

Expected, in order: `marked 1 of Kh`; `noticer: 1 id(s) on 1 card(s) of Kh`; `noticer: 17 id(s) on 1 card(s) of Kh`; `Kh: Kh sign=20 by_who=0 reads=0 notices=0 noticers=16;` (eighteen came, sixteen are kept: the client's id, the oldest, gave way, then `n1`); `... notices=0 ...` for `n1`; `... notices=1 noticers=16;` for `n2` and for `n17`; `marked 1 of Js`; `noticer: 1 id(s) on 1 card(s) of Js`; `Js: Js sign=30 by_who=1 reads=1 notices=0 noticers=0;` (a reader is not made a noticer).

A restart keeps them (design §8): `client_stop`, `server_stop` (it saves the world and the session file). Read `E:\dayzmod\testserver\games-nh\profiles\OpenZone\Games\<id>.json`: the game's state sits in it as JSON strings, its quotes escaped and cut into parts of 900 bytes (read the parts on both sides of an edge together): the record whose `Name` is `As` has `\"Sign\":12` and `\"Noticers\":[\"<me>\"]`; every other record's `Noticers` is `[]` (Kh, Js were marked after the deal). Then `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, and:

```
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"markof","id":"<id>","face":"Kh","who":"n17"}
oz_games {"op":"markof","id":"<id>","face":"Js"}
```

Expected: `As: As sign=12 by_who=0 reads=0 notices=1 noticers=1;`; `Kh: Kh sign=20 by_who=0 reads=0 notices=1 noticers=16;`; `Js: Js sign=30 by_who=1 reads=1 notices=0 noticers=0;`.

Fold the game (the client is its host): `oz_games {"op":"ask","id":"<id>","ask":"fold"}` → `folded; ...`; then `oz_games {"op":"undeck"}` deletes the deck that came back into the hands.

- [x] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_CardThings.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Cards.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Noticing: a card keeps who noticed it

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 3: The roll after a deal

The session counts and rolls for every player dealt in, right after the deal and before anybody is sent it (design §5): a deal counts for a player when one of its cards is marked and he neither reads nor notices it; the chance is the settings'; a success makes him a noticer of one such card picked at random, on the card item and on the game's record of it. Nothing is said to him; the server's log says it. The count lives in the session, in memory. The stand gets the whole deal in one line, and many deals at once with the noticing tallied against its chance.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c` (`AddNoticer`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c` (`AddNoticer`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (a member, the constructor, `Deal`, a section of its own)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (`NoticedIn`, `OpWhole`, `OpRedeal`), `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_Settings.NoticeChance`, `Cheating` (Task 1, phase 2); `OZG_Card_Base.OZG_AddNoticer`, `OZG_Notices`, `OZG_DealCard.NoticedBy`, `ReadBy`, `OZG_DeckRead.CardNamed` (Task 2, phase 2).
- Produces: `bool OZG_Game.AddNoticer(string card, string uid)`; `int OZG_Session.NoticeDeals(string uid)`; the log lines `notice: <uid> noticed <face>, sign <s>, deal <d> at <c>%, place <id>` (info) and `notice roll: <uid> deal <d> at <c>% rolled <r>` (debug). Stand ops `whole id=` and `redeal id= n=`.

- [x] **Step 1: The game takes a noticer into the deal's record**

In `OZG_Game.c`, replace

```c
    void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
    }
```

with

```c
    void ReadsFor(int seat, string uid, array<ref OZG_WRead> into)
    {
        into.Clear();
    }

    // From now on the player `uid` notices the card `card` of this deal: the
    // session rolled it (card sharp's design §5). False when the deal has no
    // such marked card.
    bool AddNoticer(string card, string uid)
    {
        return false;
    }
```

In `OZG_Durak.c`, replace

```c
    override string SaveState()
    {
```

with

```c
    override bool AddNoticer(string card, string uid)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || rec.Sign <= 0 || uid == "")
            return false;
        if (rec.Noticers.Find(uid) < 0)
            rec.Noticers.Insert(uid);
        return true;
    }

    override string SaveState()
    {
```

(Every record of `m_S.Dealt` is made by `OZG_DealCard.Make` or `Copy`, so its `Noticers` is never null.)

- [x] **Step 2: The session counts and rolls**

In `OZG_Session.c`, replace

```c
    protected int m_DeckDue;
    protected string m_DeckUid;
```

with

```c
    protected int m_DeckDue;
    protected string m_DeckUid;
    // Noticing (card sharp's design §5): for each player id, the deals he was
    // dealt in while one of the deal's cards was marked and he neither read
    // nor noticed it. Kept while the session lives and never written: a
    // restart, or a game laid out anew, starts every count again (§8).
    protected ref map<string, int> m_NoticeDeals;
```

In the constructor, replace

```c
        m_Moves = new map<string, int>();
    }
```

with

```c
        m_Moves = new map<string, int>();
        m_NoticeDeals = new map<string, int>();
    }
```

In `Deal`, replace

```c
        AddLog("deal");
        Settle();
        Changed();
```

with

```c
        AddLog("deal");
        Settle();
        // Noticing (design §5): after the deal and before anybody is sent it,
        // so that a card noticed now is in the first view of this deal.
        NoticeAfterDeal(seats, cards, chance);
        Changed();
```

Before the line `    // ------------------------------------------------------------ cards in hands`, add:

```c
    // ------------------------------------------------------------ noticing

    // The deals that counted towards noticing for a player so far.
    int NoticeDeals(string uid)
    {
        int n = 0;
        m_NoticeDeals.Find(uid, n);
        return n;
    }

    // After a deal (design §5): each player dealt in counts it when one of
    // the deal's cards is marked and he neither reads nor notices it, then
    // rolls the chance the settings give his count; on a success one such
    // card, picked at random, gets him as a noticer. Nothing is said to him
    // and nothing goes into the game's log.
    protected void NoticeAfterDeal(array<int> seats, array<ref OZG_DealCard> cards, OZG_Random dice)
    {
        OZG_Settings cfg = OZG_Cfg.Get();
        if (cfg.Cheating != 1 || !m_Game)
            return;
        array<int> unseen = new array<int>();
        int i;
        int k;
        for (i = 0; i < seats.Count(); i++)
        {
            int no = seats[i];
            if (no < 0 || no >= m_R.Seats.Count())
                continue;
            string uid = m_R.Seats[no].Uid;
            if (uid == "")
                continue;
            unseen.Clear();
            for (k = 0; k < cards.Count(); k++)
            {
                OZG_DealCard c = cards[k];
                if (!c || c.Sign <= 0)
                    continue;
                if (c.ReadBy(uid) || c.NoticedBy(uid))
                    continue;
                unseen.Insert(k);
            }
            if (unseen.Count() == 0)
                continue;
            int deals = NoticeDeals(uid) + 1;
            m_NoticeDeals.Set(uid, deals);
            int chance = cfg.NoticeChance(deals);
            int rolled = dice.Below(100);
            string roll = "notice roll: " + uid + " deal " + deals.ToString();
            roll = roll + " at " + chance.ToString() + "% rolled " + rolled.ToString();
            OZG_Log.Dbg(roll);
            if (rolled >= chance)
                continue;
            OZG_DealCard picked = cards[unseen[dice.Below(unseen.Count())]];
            NoticeOne(uid, picked, deals, chance);
        }
    }

    // The player `uid` notices `picked`: on the card item, which keeps him
    // through restarts and games (§4.1), and on the game's record of it, so
    // that his view shows it at once. The server's log says so, with his id,
    // for the admin.
    protected void NoticeOne(string uid, OZG_DealCard picked, int deals, int chance)
    {
        OZG_Card_Base thing = OZG_DeckRead.CardNamed(m_Place, picked.Name);
        if (thing)
            thing.OZG_AddNoticer(uid);
        else
            OZG_Log.Warn("notice: the card " + picked.Name + " of place " + m_R.PlaceId + " is not in it");
        m_Game.AddNoticer(picked.Name, uid);
        string line = "notice: " + uid + " noticed " + OZG_Cards.FaceOf(picked.Name);
        line = line + ", sign " + picked.Sign.ToString() + ", deal " + deals.ToString();
        line = line + " at " + chance.ToString() + "%, place " + m_R.PlaceId;
        OZG_Log.Info(line);
    }

```

- [x] **Step 3: The stand reads the whole deal and deals many times**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before its closing brace), add:

```c

    // whole id=<place>: every hidden card of the game in play, in one line:
    // each hand as <seat>[cards], the table, the stock from the card drawn
    // next to the trump at its end (the game's Everything; never sent to a
    // client).
    static bool OpWhole(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        if (!s.m_Game)
        {
            detail = "whole: no game";
            return true;
        }
        detail = s.m_Game.Everything();
        return true;
    }

    // How many cards of a place the id `uid` notices.
    protected static int NoticedIn(OZG_Place place, string uid)
    {
        array<EntityAI> things = new array<EntityAI>();
        OZG_DeckRead.ThingsOf(place, things);
        int n = 0;
        for (int i = 0; i < things.Count(); i++)
        {
            OZG_Card_Base card = OZG_Card_Base.Cast(things[i]);
            if (card && card.OZG_Notices(uid))
                n++;
        }
        return n;
    }

    // redeal id=<place> [n=10]: n deals in a row, as fast as the server runs
    // them. The game in play is ended where it stands (the stand's shortcut:
    // no move is played, nothing is said) and the host deals the next, as
    // "Deal" in the window does; the last deal is left in play. For every
    // seated player a deal that counted towards noticing is a trial at the
    // chance his count gives, and one more card he notices is a hit (design
    // §5). Answers the trials and hits at no chance (the grace), between, and
    // at the most, with the hits to expect (the sum of the chances) and their
    // spread (the root of the sum of p(1 - p)), overall and for the two.
    static bool OpRedeal(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        PlayerBase host = OZG_Sessions.PlayerOf(s.m_R.HostUid);
        if (!host)
        {
            detail = "redeal: the host is not here";
            return true;
        }
        int n = OZG_VerbOps.Arg(args, "n", "10").ToInt();
        OZG_Settings cfg = OZG_Cfg.Get();
        int top = cfg.NoticeMax;
        int graceTrials = 0;
        int graceHits = 0;
        int rampTrials = 0;
        int rampHits = 0;
        int topTrials = 0;
        int topHits = 0;
        float rampExpect = 0;
        float topExpect = 0;
        float spread = 0;
        array<string> uids = new array<string>();
        array<int> counts = new array<int>();
        array<int> seen = new array<int>();
        int k;
        for (int d = 0; d < n; d++)
        {
            uids.Clear();
            counts.Clear();
            seen.Clear();
            for (k = 0; k < s.m_R.Seats.Count(); k++)
            {
                OZG_SeatRec seat = s.m_R.Seats[k];
                if (seat.St != OZG_Const.SEAT_TAKEN || seat.Uid == "")
                    continue;
                uids.Insert(seat.Uid);
                counts.Insert(s.NoticeDeals(seat.Uid));
                seen.Insert(NoticedIn(s.m_Place, seat.Uid));
            }
            if (s.m_R.St == OZG_Const.ST_PLAYING)
                s.m_R.St = OZG_Const.ST_FINISHED;
            OZG_WAsk ask = new OZG_WAsk();
            ask.PlaceId = s.Id();
            string why = s.Ask(host, OZG_Const.OP_DEAL, ask);
            if (why != "")
            {
                detail = "redeal: deal " + d.ToString() + " refused " + why;
                return true;
            }
            for (k = 0; k < uids.Count(); k++)
            {
                int now = s.NoticeDeals(uids[k]);
                if (now == counts[k])
                    continue;
                int chance = cfg.NoticeChance(now);
                bool hit = NoticedIn(s.m_Place, uids[k]) > seen[k];
                float p = chance / 100.0;
                spread = spread + p * (1 - p);
                if (chance <= 0)
                {
                    graceTrials++;
                    if (hit)
                        graceHits++;
                }
                else if (chance >= top)
                {
                    topTrials++;
                    topExpect = topExpect + p;
                    if (hit)
                        topHits++;
                }
                else
                {
                    rampTrials++;
                    rampExpect = rampExpect + p;
                    if (hit)
                        rampHits++;
                }
            }
        }
        int trials = graceTrials + rampTrials + topTrials;
        int hits = graceHits + rampHits + topHits;
        float expect = rampExpect + topExpect;
        detail = "redeal " + n.ToString() + ": trials=" + trials.ToString();
        detail = detail + " hits=" + hits.ToString() + " expect=" + F2(expect);
        detail = detail + " sd=" + F2(Math.Sqrt(spread));
        detail = detail + " | grace " + graceTrials.ToString() + "/" + graceHits.ToString();
        detail = detail + " | ramp " + rampTrials.ToString() + "/" + rampHits.ToString();
        detail = detail + " of " + F2(rampExpect);
        detail = detail + " | top " + topTrials.ToString() + "/" + topHits.ToString();
        detail = detail + " of " + F2(topExpect);
        return true;
    }
```

In `OZG_Verb.c`, before `        // -- ops of the later phases go above this line --`, add:

```c
        if (op == "whole")
            return OZG_WorldOps.OpWhole(args, detail);
        if (op == "redeal")
            return OZG_WorldOps.OpRedeal(args, detail);
```

and in `KnownOps()` replace `, noticecfg, noticer";` with `, noticecfg, noticer, whole, redeal";`.

- [x] **Step 4: Lint, build, check one noticer (design §11.3: "with the grace set to 0 and the chance to 100")**

Before the build, `knowledge_find(kind="class", name=...)` for `AddNoticer`, `NoticeDeals`, `NoticeAfterDeal`, `NoticeOne`, `NoticedIn`, `OpWhole`, `OpRedeal`. `mod_lint`, a stand cycle, a client, `world_teleport "6095.4 107.65 6028.9"`. Every card marked by an id that sits nowhere, the bots made readers of every card, so that the client's player is the one who can notice:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all","who":"marker1"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"seats","id":"<id>"}
```

Expected: `laid <id> seats=7 kind=1`; `marked 52 of all`; `sat=1; ...`; `bots on=0 seated: <three names>`; `seats <id>: 0=.../1/<me> ...` with three more seats `.../1/<bot id>`. For each of the three bot ids `<b>`: `oz_games {"op":"mark","id":"<id>","face":"all","who":"<b>"}` → `marked 52 of all` (a card marked again keeps its sign and takes the id as a reader). Then:

```
oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}
```

Expected: `notice grace=0 step=100 max=100 putright=0`; `ok; session <id> st=1 ...`; `all: 1 noticed: <F> sign=<s> by_who=0 reads=0 notices=1 noticers=1;` (one card of the deal: `<F>` is a six to an ace). `log_tail(pattern="notice: ", n=600)` holds exactly one new line, `notice: <me> noticed <F>, sign <s>, deal 1 at 100%, place <id>`: the face and the sign of the log are those of the item, so `CardNamed` found the card the deal named. No bot id has a `notice:` line, and the client was told nothing (`client_shot`: no notification).

Four deals more:

```
oz_games {"op":"redeal","id":"<id>","n":"4"}
oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}
```

Expected: `redeal 4: trials=4 hits=4 expect=4 sd=0 | grace 0/0 | ramp 0/0 of 0 | top 4/4 of 4` (the bots read every card and count nothing); `all: 5 noticed: ...`, five faces of the sixes to the aces. `log_tail(pattern="notice: ", n=600)`: the four new lines say `deal 2` to `deal 5`, each `at 100%`.

A restart keeps the noticers on the cards (design §8): `client_stop`, `server_stop`, `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`; then

```
oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}
oz_games {"op":"noticecfg"}
```

Expected: `all: 5 noticed: ...`, the same five faces and signs as before the restart; `notice grace=5 step=4 max=40 putright=0` (the file's values are back). That the count of deals did not survive is checked in Task 4, which can show it. Fold this game (its bots did not survive the restart): `oz_games {"op":"ask","id":"<id>","ask":"fold"}`, `oz_games {"op":"undeck"}`.

- [x] **Step 5: Check the rate (design §11.3: "then with the defaults over many deals")**

Six players who notice (the bots are NOT made readers), the settings at their defaults (`noticecfg` answered `grace=5 step=4 max=40` above):

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all","who":"marker1"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"5","on":"0"}
oz_games {"op":"redeal","id":"<id>","n":"40"}
oz_games {"op":"redeal","id":"<id>","n":"40"}
```

The six players take the whole thirty-six cards a deal. The first `redeal` starts from the lobby: every player's first five deals are grace, the next nine the ramp (4 % to 36 %), the rest at 40 %. Add the two answers: the trials, the hits, the `expect`s; the spread is the root of the sum of the two `sd`s squared. Expected:

1. `grace` is `30/0`: six players, five deals each, no notice.
2. `ramp` is 54 trials (nine deals of six players) and its `of` is 10.8 (six times 4 + 8 + ... + 36 percent); its hits lie within 10.8 ± 8.5 (three spreads; the ramp's own spread is about 2.8).
3. `top` has its trials at 40 %: its hits over its trials within 0.40 ± 3 × √(0.24 / trials) (for about 390 trials, 0.33 to 0.47). A player who has noticed all thirty-six cards stops counting, so fewer trials than 6 × 66 is no fault.
4. All hits within the summed `expect` ± three spreads.

`log_tail(pattern="notice: ", n=1000)`: in every line the count after `deal` is 6 or more, and the chance after `at` is min(40, (count − 5) × 4) percent, as `deal 6 at 4%`, `deal 9 at 16%`, `deal 20 at 40%`. If a check falls outside its band, run the two `redeal`s once more on a place laid anew before calling it a fault (a band of three spreads is still missed about one time in three hundred and seventy), and write both runs into the record.

Fold the game and undeck: `oz_games {"op":"ask","id":"<id>","ask":"fold"}`, `oz_games {"op":"undeck"}`.

- [x] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Noticing: after each deal a player may notice one marked card

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 4: What a noticer is told

The game says what a player notices where it lies for him (design §4.5): on a back where a read would lie, a card he notices and does not read; on a face -- his own hand, the table, the trump under the stock -- every card he notices. The session puts it into the head made for that player alone, beside his reads; a watcher and every other seat get nothing of it, and the game's halves do not change (§4.6). `Cheating` 0 sends nothing. The stand reads the head a player would be sent.

In the head's JSON a player who notices a card in seat 3's hand, the top of the stock and the king of hearts in his own hand is sent, beside his `Reads`:

```
"Notices":[{"Seat":3,"Look":0,"CardName":"","Sign":17},{"Seat":-1,"Look":0,"CardName":"","Sign":42},{"Seat":-2,"Look":0,"CardName":"Kh","Sign":8}]
```

The backs name no card; the face names the card the player holds and sees.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Wire.c` (`OZG_WNotice`; `OZG_WView.Notices`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c` (`NoticesFor`)
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c` (`NoticesFor`, `NoticeBack`, `NoticeFace`)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`ViewBody`, `HeadFor`)
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Table.c` (`Take`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` (`OpNotices`), `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_DealCard.NoticedBy`, `ReadBy`, `Look`, `Sign` (Task 2, phase 2); `OZG_Durak.DealtCard`, `IndexOfSeat` (phase 1, 2); `OZG_Session.NoticeDeals` (Task 3).
- Produces: `class OZG_WNotice { int Seat; int Look; string CardName; int Sign; static OZG_WNotice MakeBack(int seat, int look, int sign); static OZG_WNotice MakeFace(string card, int look, int sign); }` -- a back's entry has `CardName` "" and `Seat` the hand's seat number or -1 for the top of the stock; a face's entry has the card's name and `Seat` -2. `OZG_WView.Notices` (`ref array<ref OZG_WNotice>`, never null on a client after `OZG_Table.Take`). `void OZG_Game.NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)`. `string OZG_Session.HeadFor(string uid)` (for the stand). Stand op `notices id= [seat= | who=]`.

- [x] **Step 1: A notice on the wire**

In `OZG_Wire.c`, replace

```c
// The head of a view.
class OZG_WView
```

with

```c
// A card this client's player notices (card sharp's design §4.5): the sign
// he sees on it, and where. On a back -- `CardName` empty -- in the hand of
// the seat `Seat`, or on top of the stock (`Seat` -1), on a back of the look
// `Look`; nothing says which card it is. On a face -- `CardName` the card's
// name, which the player sees anyway, `Seat` -2 -- that card wherever it
// lies face up for him: in his own hand, on the table, the trump under the
// stock; `Look` gives the style of its face.
class OZG_WNotice
{
    int    Seat;
    int    Look;
    string CardName;
    int    Sign;

    static OZG_WNotice MakeBack(int seat, int look, int sign)
    {
        OZG_WNotice n = new OZG_WNotice();
        n.Seat = seat;
        n.Look = look;
        n.CardName = "";
        n.Sign = sign;
        return n;
    }

    static OZG_WNotice MakeFace(string card, int look, int sign)
    {
        OZG_WNotice n = new OZG_WNotice();
        n.Seat = -2;
        n.Look = look;
        n.CardName = card;
        n.Sign = sign;
        return n;
    }
}

// The head of a view.
class OZG_WView
```

Replace

```c
    // What this client's player reads (design §4.4); empty for a watcher and
    // for a seat that reads nothing. Made apart for every player.
    ref array<ref OZG_WRead> Reads;
```

with

```c
    // What this client's player reads (design §4.4); empty for a watcher and
    // for a seat that reads nothing. Made apart for every player.
    ref array<ref OZG_WRead> Reads;
    // What this client's player notices (design §4.5); empty as the reads.
    ref array<ref OZG_WNotice> Notices;
```

and in its constructor replace `        Reads = new array<ref OZG_WRead>();` with

```c
        Reads = new array<ref OZG_WRead>();
        Notices = new array<ref OZG_WNotice>();
```

- [x] **Step 2: The game says what a seat notices**

In `OZG_Game.c`, replace

```c
    bool IsOver()
    {
        return true;
    }
```

with

```c
    // The cards the seat `seat`, held by the player `uid`, notices, where
    // they lie for that player (card sharp's design §4.5): on a back where a
    // read would lie, a card he notices and does not read (a reader sees
    // more); on a face -- his own hand, the table, the trump -- every card he
    // notices. A game without hidden cards says nothing.
    void NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)
    {
        into.Clear();
    }

    bool IsOver()
    {
        return true;
    }
```

In `OZG_Durak.c`, replace

```c
    override string SaveState()
    {
```

with

```c
    override void NoticesFor(int seat, string uid, array<ref OZG_WNotice> into)
    {
        into.Clear();
        if (!m_S || uid == "")
            return;
        int i;
        int h;
        // Backs: the other seats' hands and the top of the stock, as the reads.
        for (i = 0; i < m_S.SeatList.Count(); i++)
        {
            OZG_DurakSeat other = m_S.SeatList[i];
            if (other.SeatNo == seat)
                continue;
            for (h = 0; h < other.Held.Count(); h++)
                NoticeBack(other.SeatNo, other.Held[h], uid, into);
        }
        if (m_S.StockCards.Count() > 1)
            NoticeBack(-1, m_S.StockCards[0], uid, into);
        // Faces: the seat's own hand, the table, the trump while it lies
        // under the stock (the window draws it there only then).
        int mine = IndexOfSeat(seat);
        if (mine >= 0)
        {
            array<string> held = m_S.SeatList[mine].Held;
            for (h = 0; h < held.Count(); h++)
                NoticeFace(held[h], uid, into);
        }
        for (i = 0; i < m_S.AttackCards.Count(); i++)
        {
            NoticeFace(m_S.AttackCards[i], uid, into);
            NoticeFace(m_S.DefenceCards[i], uid, into);
        }
        if (m_S.StockCards.Count() > 0)
            NoticeFace(m_S.TrumpCard, uid, into);
    }

    protected void NoticeBack(int where, string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.NoticedBy(uid) || rec.ReadBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeBack(where, rec.Look, rec.Sign));
    }

    protected void NoticeFace(string card, string uid, array<ref OZG_WNotice> into)
    {
        OZG_DealCard rec = DealtCard(card);
        if (!rec || !rec.NoticedBy(uid))
            return;
        into.Insert(OZG_WNotice.MakeFace(card, rec.Look, rec.Sign));
    }

    override string SaveState()
    {
```

(An unbeaten attack card has `""` beside it, and `DealtCard("")` answers null.)

- [x] **Step 3: The session puts them into the player's own head**

In `OZG_Session.c`, in `ViewBody`, replace

```c
        // What this player reads: only a player dealt into the game, and only
        // into the head made for that player (design §4.6).
        v.Reads.Clear();
        bool dealt = seat >= 0 && m_R.Dealt.Find(seat) >= 0;
        bool reading = OZG_Cfg.Get().Cheating == 1;
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY && dealt && reading)
            m_Game.ReadsFor(seat, uid, v.Reads);
```

with

```c
        // What this player reads and notices: only a player dealt into the
        // game, and only into the head made for that player (design §4.6).
        // One head serves every player of a push: both lists are emptied
        // first, so nobody is sent another's.
        v.Reads.Clear();
        v.Notices.Clear();
        bool dealt = seat >= 0 && m_R.Dealt.Find(seat) >= 0;
        bool reading = OZG_Cfg.Get().Cheating == 1;
        if (m_Game && m_R.St != OZG_Const.ST_LOBBY && dealt && reading)
        {
            m_Game.ReadsFor(seat, uid, v.Reads);
            m_Game.NoticesFor(seat, uid, v.Notices);
        }
```

After the method `ViewTo`, add:

```c

    // For the stand: the head of the view this player would be sent now.
    string HeadFor(string uid)
    {
        string head;
        string shared;
        string own;
        OZG_Text.Cut3(ViewBody(ViewHead(), uid), head, shared, own);
        return head;
    }
```

- [x] **Step 4: The client keeps the list whatever came**

In `OZG_Table.c`, in `Take`, replace

```c
        if (!v.Reads)
            v.Reads = new array<ref OZG_WRead>();
```

with

```c
        if (!v.Reads)
            v.Reads = new array<ref OZG_WRead>();
        if (!v.Notices)
            v.Notices = new array<ref OZG_WNotice>();
```

- [x] **Step 5: The stand reads a player's head**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before its closing brace), add:

```c

    // notices id=<place> [seat=<n> | who=me|<id>]: the notices in the head of
    // the view that player would be sent now (the session's own head, design
    // §4.6): a back as "b<seat>:<look>/<sign>" (-1 the top of the stock), a
    // face as "f<card>/<sign>"; with his seat (mine=-1 for one who holds
    // none), the deals that counted towards noticing for him, how many reads
    // the head carries and its length in bytes.
    static bool OpNotices(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        string seatArg = OZG_VerbOps.Arg(args, "seat", "");
        if (seatArg != "")
        {
            int no = seatArg.ToInt();
            if (no < 0 || no >= s.m_R.Seats.Count())
            {
                detail = "notices: no seat " + seatArg;
                return true;
            }
            uid = s.m_R.Seats[no].Uid;
        }
        else
        {
            Who(args, uid);
        }
        string head = s.HeadFor(uid);
        OZG_WView v;
        string err;
        if (!JsonFileLoader<OZG_WView>.LoadData(head, v, err) || !v)
        {
            detail = "notices: the head did not read: " + err;
            return true;
        }
        int reads = 0;
        if (v.Reads)
            reads = v.Reads.Count();
        detail = "notices of " + uid + " mine=" + v.Mine.ToString();
        detail = detail + " deals=" + s.NoticeDeals(uid).ToString();
        detail = detail + " reads=" + reads.ToString() + " head=" + head.Length().ToString() + "B:";
        if (!v.Notices || v.Notices.Count() == 0)
        {
            detail = detail + " none";
            return true;
        }
        for (int i = 0; i < v.Notices.Count(); i++)
        {
            OZG_WNotice n = v.Notices[i];
            string one = " f" + n.CardName;
            if (n.CardName == "")
                one = " b" + n.Seat.ToString() + ":" + n.Look.ToString();
            detail = detail + one + "/" + n.Sign.ToString();
        }
        return true;
    }
```

In `OZG_Verb.c`, before `        // -- ops of the later phases go above this line --`, add:

```c
        if (op == "notices")
            return OZG_WorldOps.OpNotices(args, detail);
```

and in `KnownOps()` replace `, whole, redeal";` with `, whole, redeal, notices";`.

- [x] **Step 6: Lint, build, check where a notice lies (design §11.3, §4.6)**

Before the build, `knowledge_find(kind="class", name=...)` for `OZG_WNotice`'s members and methods (`CardName`, `MakeBack`, `MakeFace`), `Notices`, `NoticesFor`, `NoticeBack`, `NoticeFace`, `HeadFor`, `OpNotices`. `mod_lint`, a stand cycle, a client, `world_teleport "6095.4 107.65 6028.9"`.

**Two aces, each its own card.** The deal names a further card of a face `As.1`, and the item found for it must be that very card: the two aces get two signs, and the sign the view shows must be the sign of the ace the item says is noticed. A classic deck has no free slot, so the second ace lies beside it in the place (`stuff`), the last of the things a deal reads:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"stuff","id":"<id>","face":"As","style":"classic","n":"1"}
oz_games {"op":"mark","id":"<id>","face":"As","n":"1","who":"marker1","sign":"12"}
oz_games {"op":"mark","id":"<id>","face":"As","n":"2","who":"marker1","sign":"34"}
oz_games {"op":"markof","id":"<id>","face":"As"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"seats","id":"<id>"}
```

Expected: `stuffed As: deck=0 beside=1`; `marked 1 of As`; `marked 2 of As` (the first keeps 12, the second takes 34); `As: As sign=12 ...; As sign=34 ...;`; seated as in Task 3. For each bot id `<b>`: `oz_games {"op":"mark","id":"<id>","face":"As","n":"2","who":"<b>"}` → `marked 2 of As`. Then:

```
oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"markof","id":"<id>","face":"As","only":"notices"}
oz_games {"op":"whole","id":"<id>"}
oz_games {"op":"notices","id":"<id>","seat":"0"}
```

Expected: `ok; session <id> st=1 ...`; `As: 1 noticed: As sign=<s> ... notices=1 ...;` with `<s>` 12 or 34; `whole` names where `As` and `As.1` lie; `notices of <me> mine=0 deals=1 reads=0 head=<n>B:` followed by, for the noticed ace (`As` when `<s>` is 12, `As.1` when it is 34):
- in a bot's hand `[...]` of seat `<k>`: ` b<k>:0/<s>` (look 0 is the classic deck's, the first style the config names; the stalkers' is 1);
- in `0[...]`, the client's own hand: ` fAs/<s>` or ` fAs.1/<s>`;
- first in `stock[...]`, the stock holding more than that card: ` b-1:0/<s>`;
- last in `stock[...]`, the trump: ` f<name>/<s>`;
- deeper in the stock: ` none`.

The sign in `notices` and the sign `markof` gives the noticed ace are the same number: the right card was noticed. Then `oz_games {"op":"redeal","id":"<id>","n":"3"}` → `redeal 3: trials=1 hits=1 ...` (the other ace is noticed at the next deal; then nothing is left to notice and nothing counts); `markof face=As only=notices` → `As: 2 noticed: ...`; `notices seat=0` → `deals=2`, and every ace that lies in a hand, on top of the stock or as the trump is listed with its own sign. Fold and undeck.

**Every card, one player.** As in Task 3, Step 4, every card marked by an id that sits nowhere and the bots made readers of every card:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all","who":"marker1"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"seats","id":"<id>"}
```

then `oz_games {"op":"mark","id":"<id>","face":"all","who":"<b>"}` for each of the three bot ids `<b>` that `seats` names, and:

```
oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"redeal","id":"<id>","n":"40"}
oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}
oz_games {"op":"notices","id":"<id>","seat":"0"}
oz_games {"op":"whole","id":"<id>"}
```

Expected: `redeal 40: trials=35 hits=35 ... | top 35/35 of 35` (the thirty-six of the deal noticed one a deal, then nothing counts); `all: 36 noticed: ...`; `notices of <me> mine=0 deals=36 reads=0 head=<n>B:` and twenty-six entries: ` b<k>:0/<s>` for each of the eighteen cards of the three bots' hands, ` b-1:0/<s>` for the top of the stock, ` f<card>/<s>` for the six of the client's hand and for the trump (`whole` says which cards those are, the stock's last being the trump). `<n>` is under 8000 (`OZG_Text.Cut3` parts the texts with `Substring`, which cuts its output at 8191 bytes); write it into the record.

Nobody else is told (design §4.6):
- `oz_games {"op":"notices","id":"<id>","seat":"<k>"}` for each bot's seat: `notices of <b> mine=<k> deals=0 reads=19 head=<n>B: none` (the bots read every card back up for them and notice none);
- `oz_games {"op":"notices","id":"<id>","who":"watcher1"}`: `notices of watcher1 mine=-1 deals=0 reads=0 head=<n>B: none` (anybody who holds no seat is a watcher's head);
- `oz_games {"op":"hand","id":"<id>","seat":"0"}`: the seat's own half is `{"Held":[...],"CanPlay":[...],"HeldLooks":[...]}` and nothing more; the game's halves are what they were.

**A restart in mid-game** (design §8: who noticed what survives, the count of deals does not): `client_stop`, `server_stop`, `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client, `world_ready`, `world_teleport "6095.4 107.65 6028.9"` (the client's seat is taken again within a second of being near). Then `oz_games {"op":"notices","id":"<id>","seat":"0"}`: `deals=0` and the same twenty-six entries (the game came back from the session file, its records with their noticers); `markof face=all who=me only=notices` → `all: 36 noticed`. Leave this game laid out: Task 6 folds it.

- [x] **Step 7: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Wire.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Table.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Noticing: a player is told the cards he notices, and nobody else

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 5: The speck's art

A noticed face shows "a small dark speck in a margin" (design §4.5, §10). The design says no more; this plan makes it one picture for every sign, put on a line 6 px in from the card's edge (of a 200 x 280 card) at the place its sign picks among ninety-nine places per style, spread over the stretches of that line where no face of the style carries ink, and dealt to the signs by lot with a fixed seed: the same sign is the same place on every face, two signs sit apart. `tools/make_marks.py`, which drew phase 2's signs and pencil, draws it into the same imageset and writes its boxes into `OZG_MarkPlaces.c`; nothing it drew before changes.

**Files:**
- Modify: `tools/make_marks.py`
- Modify: `art/marks/SOURCES.md`
- Modify (written by the script): `art/marks/_view.png`, `OpenZone_Games/gui/imagesets/ozg_marks.imageset`, `OpenZone_Games/gui/imagesets/ozg_marks_ca.paa`, `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c`
- Create (written by the script): `art/marks/classic_specks.png`, `art/marks/stalker_specks.png`

**Interfaces:**
- Consumes: the faces `art/cards/<style>/c_*.png` (200 x 280, `tools/make_cards.py`).
- Produces: in `OZG_MarkPlaces`: `static bool SpeckBox(string style, int sign, out int x, out int y, out int w, out int h)` (keys `<style>_f<sign>`), `static string SpeckImage()` (`set:ozg_marks image:speck`). The picture `speck` in `ozg_marks`.

- [x] **Step 1: The script draws the speck and its places**

Edit `tools/make_marks.py` with the Edit tool (it holds backslashes; nothing here goes through a shell). Replace

```python
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
```

with

```python
# Every fault and every pencilled face is a picture of one ink on a clear
# ground, laid over the back where it was drawn, as the mock-up laid it.
#
# On a FACE a sign is a small dark speck in the margin (design §4.5): one
# picture for every sign, put where the sign says on a line MARGIN in from the
# card's edge. The ninety-nine places are spread evenly over the stretches of
# that line where no face of the style carries ink (every face read against
# the paper round it) and dealt to the signs by lot, with a fixed seed: two
# signs a player notices sit apart on the faces as on the backs.
#
# Usage: python -I tools/make_marks.py [--paa <ImageToPAA.exe>] [--no-paa]
#
# Reads art/cards/<style>/back.png and the faces art/cards/<style>/c_*.png
# (tools/make_cards.py). Writes art/marks/<style>_signs.png (the ninety-nine
# on that back, to look at), art/marks/<style>_specks.png (the ninety-nine
# specks on one face, and where the faces of the style carry ink),
# art/marks/_view.png (each kind, a read back and a noticed face, as the
# mock-up's sheet), build/marks/ozg_marks_ca.png,
# OpenZone_Games/gui/imagesets/ozg_marks.imageset and, given ImageToPAA,
# ozg_marks_ca.paa beside it, and
# OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c.
```

Replace `from PIL import Image, ImageDraw, ImageFont` with

```python
from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont
```

Replace

```python
GROW = 0.5
```

with

```python
GROW = 0.5
# The speck on a face (design §4.5): a dark ink, as the back's dot; its radius
# (GROW comes on top, as on the backs); how far in from a card's edge its
# middle lies, on a line that runs round the corners; the least distance
# between two specks' middles. CORNER is a card's corner radius, as
# tools/make_cards.py draws it.
SPECK = (20, 10, 8, 230)
SPECK_R = 2.2
MARGIN = 6.0
SPECK_GAP = 6.5
CORNER = 13.0
# What is ink on a face: a pixel this much darker than the paper round it (a
# median of MEDIAN px), or this much more coloured, in a patch OPEN px across
# at the least -- the dirt and the creases of a worn face are thinner, and
# are paper; a speck's middle keeps a square of CLEAR px round it free of ink.
INK_DARK = 28
INK_COLOUR = 45
MEDIAN = 21
OPEN = 3
CLEAR = 11
```

Replace the line `# ---------------------------------------------------------------- the sheets` with

```python
# ---------------------------------------------------------------- the specks

def face_names(style):
    """The faces a style has pictures of: art/cards/<style>/c_*.png, jokers too."""
    folder = os.path.join(CARDS, style)
    return sorted(n for n in os.listdir(folder) if n.startswith("c_") and n.endswith(".png"))


def inked(style):
    """Where any face of a style carries ink or a picture, at a card's size (255 inked,
    0 clear): each face read against the paper round every pixel, the darker or more
    coloured patches kept, all faces laid together. And how many faces there are."""
    out = None
    names = face_names(style)
    for name in names:
        rgb = Image.open(os.path.join(CARDS, style, name)).convert("RGB")
        if rgb.size != (W, H):
            raise SystemExit("error: %s/%s is %s, not %d x %d" % (style, name, rgb.size, W, H))
        lum = rgb.convert("L")
        paper = lum.filter(ImageFilter.MedianFilter(MEDIAN))
        dark = ImageChops.subtract(paper, lum).point(lambda v: 255 if v > INK_DARK else 0)
        r, g, b = rgb.split()
        colour = ImageChops.subtract(ImageChops.lighter(ImageChops.lighter(r, g), b),
                                     ImageChops.darker(ImageChops.darker(r, g), b))
        tinted = ImageChops.subtract(colour, colour.filter(ImageFilter.MedianFilter(MEDIAN)))
        ink = ImageChops.lighter(dark, tinted.point(lambda v: 255 if v > INK_COLOUR else 0))
        ink = ink.filter(ImageFilter.MinFilter(OPEN)).filter(ImageFilter.MaxFilter(OPEN))
        out = ink if out is None else ImageChops.lighter(out, ink)
    if out is None:
        raise SystemExit("error: no faces in " + os.path.join(CARDS, style))
    return out, len(names)


def margin_line():
    """The line a speck's middle may lie on: MARGIN in from a card's edge and round its
    corners, clockwise from the top left, a point about every half pixel."""
    pts = []
    r = CORNER - MARGIN

    def line(x0, y0, x1, y1):
        n = max(2, int(math.hypot(x1 - x0, y1 - y0) / 0.5))
        pts.extend((x0 + (x1 - x0) * k / n, y0 + (y1 - y0) * k / n) for k in range(n))

    def bend(cx, cy, a0):
        n = max(2, int(math.pi / 2 * r / 0.5))
        for k in range(n):
            a = math.radians(a0 + 90.0 * k / n)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))

    line(CORNER, MARGIN, W - CORNER, MARGIN)
    bend(W - CORNER, CORNER, -90)
    line(W - MARGIN, CORNER, W - MARGIN, H - CORNER)
    bend(W - CORNER, H - CORNER, 0)
    line(W - CORNER, H - MARGIN, CORNER, H - MARGIN)
    bend(CORNER, H - CORNER, 90)
    line(MARGIN, H - CORNER, MARGIN, CORNER)
    bend(CORNER, CORNER, 180)
    return pts


def speck_places(style, ink):
    """Ninety-nine places for a speck on the faces of a style, spread evenly along the
    stretches of the margin line clear of every face's ink and SPECK_GAP apart at the
    least, then dealt to the signs by lot: element n - 1 is sign n's middle. And the
    spacing they were spread at."""
    near = ink.filter(ImageFilter.MaxFilter(CLEAR))
    path = margin_line()
    clear = [near.getpixel((min(W - 1, int(round(x))), min(H - 1, int(round(y))))) == 0 for x, y in path]
    length = 0.0
    for k in range(1, len(path)):
        if clear[k] and clear[k - 1]:
            length += math.hypot(path[k][0] - path[k - 1][0], path[k][1] - path[k - 1][1])
    spacing = length / SIGNS
    while spacing >= SPECK_GAP:
        chosen, run = [], spacing
        for k, (x, y) in enumerate(path):
            if k > 0 and clear[k] and clear[k - 1]:
                run += math.hypot(x - path[k - 1][0], y - path[k - 1][1])
            if not clear[k] or run < spacing:
                continue
            if all(math.hypot(x - cx, y - cy) >= SPECK_GAP for cx, cy in chosen):
                chosen.append((x, y))
                run = 0.0
        if len(chosen) >= SIGNS:
            chosen = chosen[:SIGNS]
            random.Random("%s:specks" % style).shuffle(chosen)
            return chosen, spacing
        spacing *= 0.98
    raise SystemExit("error: %s: the margin has room for fewer than %d specks" % (style, SIGNS))


def speck():
    """The speck as one picture cut out round it, and the box it was cut from on a card
    where the speck's middle is the card's middle."""
    big = sheet(SPECK)
    d = ImageDraw.Draw(big)
    g, r = GROW, SPECK_R
    d.ellipse(s(CX - r - g, CY - r * 0.9 - g, CX + r * 0.95 + g, CY + r + g), fill=SPECK)
    return cut_out(reduce(big, SPECK))


def speck_box(box, place):
    """The box of the speck's picture on a card, for a speck whose middle is `place`."""
    bx, by, bw, bh = box
    return bx + int(round(place[0] - CX)), by + int(round(place[1] - CY)), bw, bh


def specks_sheet(style, ink, places, pic, box):
    """The queen of spades of a style with all ninety-nine specks on her, and beside her
    where the faces of the style carry ink (grey) with the specks laid on it; both twice a
    card's size."""
    on = Image.open(os.path.join(CARDS, style, "c_Qs.png")).convert("RGBA")
    grey = Image.new("RGBA", (W, H), (255, 255, 255, 255))
    grey.paste((150, 150, 150, 255), None, ink)
    for place in places:
        x, y, _, _ = speck_box(box, place)
        on.alpha_composite(pic, (x, y))
        grey.alpha_composite(pic, (x, y))
    big_on = on.resize((2 * W, 2 * H), LANCZOS)
    out = Image.new("RGB", (4 * W + 30, 2 * H + 20), (45, 45, 45))
    out.paste(big_on, (10, 10), big_on)
    out.paste(grey.resize((2 * W, 2 * H), LANCZOS).convert("RGB"), (2 * W + 20, 10))
    return out


# ---------------------------------------------------------------- the sheets
```

In `view_sheet`, replace its docstring

```python
    """A row a style: the back, a fault of each kind, a read back; at their size and as
    small as a fan shows them, as the mock-up's sheet."""
```

with

```python
    """A row a style: the back, a fault of each kind, a read back, a noticed face; at
    their size and as small as a fan shows them, as the mock-up's sheet."""
```

In `CODE_TOP`, replace

```
// Where the game window draws a mark on a back (card sharp's design §4.4,
// §4.5): the box of each sign's picture in set:ozg_marks, by the style of the
// back, and the box of the pencilled face, in pixels of a 200 x 280 card. A
// style with no signs here (another mod's deck) shows the pencil alone.
```

with

```
// Where the game window draws a mark (card sharp's design §4.4, §4.5), in
// pixels of a 200 x 280 card: on a back, the box of each sign's picture in
// set:ozg_marks, by the style of the back, and the box of the pencilled
// face; on a face, the box of the speck each sign puts in its margin, by the
// style of the face. A style with no signs here (another mod's deck) shows
// the pencil alone and no speck.
```

and

```
    static string PencilImage(string face)
    {
        return "set:" + IMAGESET + " image:p_" + face;
    }
```

with

```
    static string PencilImage(string face)
    {
        return "set:" + IMAGESET + " image:p_" + face;
    }

    // The box of the speck sign `sign` puts on a face of `style`; false:
    // none is drawn.
    static bool SpeckBox(string style, int sign, out int x, out int y, out int w, out int h)
    {
        return BoxOf(style + "_f" + sign.ToString(), x, y, w, h);
    }

    // The speck: one picture for every sign and every style.
    static string SpeckImage()
    {
        return "set:" + IMAGESET + " image:speck";
    }
```

In `code(boxes)`, replace

```python
    keys = ["pencil"] + ["%s_s%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
```

with

```python
    keys = ["pencil"] + ["%s_s%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
    keys += ["%s_f%d" % (st, n) for st in STYLES for n in range(1, SIGNS + 1)]
```

In `main()`, replace

```python
    rows = []
    for style in STYLES:
```

with

```python
    speck_pic, speck_cut = speck()
    pieces["speck"] = speck_pic
    rows = []
    for style in STYLES:
```

and

```python
        read = firsts["cut"].copy()
        read.alpha_composite(pencils["Kh"])
        rows.append([back] + [firsts[k] for k in KINDS] + [read])
        print("%s: %d signs, %d places of each kind" % (style, SIGNS, PER_KIND))
```

with

```python
        read = firsts["cut"].copy()
        read.alpha_composite(pencils["Kh"])
        ink, faces = inked(style)
        specks, spacing = speck_places(style, ink)
        for sign, place in enumerate(specks, start=1):
            boxes["%s_f%d" % (style, sign)] = speck_box(speck_cut, place)
        specks_sheet(style, ink, specks, speck_pic, speck_cut).save(os.path.join(ART, "%s_specks.png" % style))
        noticed = Image.open(os.path.join(CARDS, style, "c_Kh.png")).convert("RGBA")
        x, y, _, _ = boxes["%s_f1" % style]
        noticed.alpha_composite(speck_pic, (x, y))
        rows.append([back] + [firsts[k] for k in KINDS] + [read, noticed])
        print("%s: %d signs, %d places of each kind; %d specks round %d faces, %.1f px apart"
              % (style, SIGNS, PER_KIND, SIGNS, faces, spacing))
```

- [x] **Step 2: Where the speck's places come from**

Replace the whole of `art/marks/SOURCES.md` with:

```markdown
# Where the marks come from

`tools/make_marks.py` draws every picture in this folder and in
`OpenZone_Games/gui/imagesets/ozg_marks_ca.paa`: the faults from numbers, on the
backs `art/cards/<style>/back.png` that `tools/make_cards.py` draws, and the
pencil with two fonts of Windows, Segoe Print (the ranks) and Segoe UI Symbol
(the suits), as the table mock-up of 2026-11-24 wrote it. The fonts are not in
this repository; the script reads them from `C:\Windows\Fonts`.

The speck a noticed face shows is drawn from numbers too. Its ninety-nine places
on the faces of a style are found by reading every face
`art/cards/<style>/c_*.png` against the paper round it: a place lies on a line
6 px in from the card's edge, where no face of the style carries ink.
`<style>_specks.png` shows them on one face and on the ink of all the faces.
```

- [x] **Step 3: Draw them, and look at them**

Run `python -I tools/make_marks.py`. Expected output (the plan's edits were applied to a copy of the script and run so, against copies of `art/cards`, when the plan was written):

```
classic: 99 signs, 20 places of each kind; 99 specks round 52 faces, 7.9 px apart
stalker: 99 signs, 20 places of each kind; 99 specks round 54 faces, 6.9 px apart
ozg_marks: 251 pictures on 512 x 512; OZG_MarkPlaces.c: 397 boxes
ozg_marks_ca.paa: 65 KB
```

It takes about twenty seconds (every face is read twice through a median). An `error: <style>: the margin has room for fewer than 99 specks` means the faces changed: lower `SPECK_GAP` by 0.5, or `CLEAR` by 2, run again and say so in the record.

Then:
1. `git status --short art/marks OpenZone_Games/gui/imagesets OpenZone_Games/scripts`: `_view.png`, `ozg_marks.imageset`, `ozg_marks_ca.paa` and `OZG_MarkPlaces.c` modified, `classic_specks.png` and `stalker_specks.png` new, and nothing else: `classic_signs.png` and `stalker_signs.png` come out byte for byte as before (the faults' places draw by lot from seeds of their own).
2. `git diff OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c`: only lines added (the comment, `SpeckBox`, `SpeckImage`, 198 `PutBox("<style>_f<n>", ...)` lines, each box 12 x 12) and the header comment's six lines replaced; not one `_s<n>` or `pencil` box changed.
3. Look at `art/marks/classic_specks.png` and `stalker_specks.png`: on the left the queen of spades with all ninety-nine specks, a dotted frame just inside the edge; on the right the ink of every face of the style in grey, and no speck on the grey (none on an index, a pip, a frame or a caption). Look at `art/marks/_view.png`: the last column of each row, the king of hearts with sign 1's speck, a small dark dot in the margin; in the small copy, as a fan would show it, hardly to be seen.

- [x] **Step 4: Commit**

```bash
git add tools/make_marks.py art/marks OpenZone_Games/gui/imagesets/ozg_marks.imageset OpenZone_Games/gui/imagesets/ozg_marks_ca.paa OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MarkPlaces.c
git commit -m "Noticing: the speck a noticed face shows, its place the sign's

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 6: What a noticer sees

The window draws, on a back the player notices, the sign as a fault in the back's own pattern (phase 2's art, with no pencil: he does not know the face), and on a face he notices -- in his hand, on the table, the trump -- the speck in its margin (design §4.5, §10). Both go into the card's turning frame as phase 2's marks do; the face's speck takes the frame's sign picture, which a face never uses.

**Files:**
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c` (the marks' section; `PaintFan`, `PaintTable`, `PaintBout`, `PaintHand`)
- Modify: `tools/make_layouts.py` (the comment over the sign and the pencil of a card's frame)

**Interfaces:**
- Consumes: `OZG_WView.Notices`, `OZG_WNotice.Seat`, `Look`, `CardName`, `Sign` (Task 4); `OZG_MarkPlaces.SpeckBox`, `SpeckImage` (Task 5), `SignBox`, `SignImage`, `PencilBox`, `PencilImage` (phase 2).
- Produces: in `OZG_GameMenu`: `NoticeAt(int seat, int look, array<int> used)`, `NoticeOn(string card)`, `LayMark(...)`, `SignBack(...)`, `SpeckFace(...)`; `MarkBack` drawn through them.

- [x] **Step 1: The marks' drawing, in one place**

In `OZG_GameMenu.c`, replace the whole of `MarkBack`, from its comment

```c
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

with

```c
    // Which of this player's notices a back of the seat `seat` (-1: the top
    // of the stock) in the look `look` shows: the first notice of a back
    // there in that look that no back drawn before took (`used`); -1: none.
    // As a read, a notice lies on the first free back of its look.
    protected int NoticeAt(int seat, int look, array<int> used)
    {
        if (!m_V || !m_V.Notices)
            return -1;
        for (int i = 0; i < m_V.Notices.Count(); i++)
        {
            OZG_WNotice n = m_V.Notices[i];
            if (n.CardName != "" || n.Seat != seat || n.Look != look)
                continue;
            if (used.Find(i) >= 0)
                continue;
            used.Insert(i);
            return i;
        }
        return -1;
    }

    // The notice on the face of the card `card`, wherever it lies face up
    // for this player; -1: none.
    protected int NoticeOn(string card)
    {
        if (!m_V || !m_V.Notices || card == "")
            return -1;
        for (int i = 0; i < m_V.Notices.Count(); i++)
        {
            if (m_V.Notices[i].CardName == card)
                return i;
        }
        return -1;
    }

    // A picture of set:ozg_marks laid into the frame of a card of w x h
    // units, at a box given in pixels of a 200 x 280 card (OZG_MarkPlaces,
    // tools/make_marks.py), so that it turns with the card. The face lies in
    // the middle of the frame, 5 : 7, as tools/make_layouts.py lays it.
    protected void LayMark(ImageWidget pic, string image, float w, float h, int x, int y, int bw, int bh)
    {
        float side = BoxOf(w, h) * m_K;
        float d = CARD_BOX * Math.Sqrt(74);
        float faceW = side * 5 / d;
        float faceH = side * 7 / d;
        float left = (side - faceW) * 0.5;
        float top = (side - faceH) * 0.5;
        float kx = faceW / OZG_MarkPlaces.CARD_W;
        float ky = faceH / OZG_MarkPlaces.CARD_H;
        pic.LoadImageFile(0, image);
        pic.SetImage(0);
        pic.SetPos(left + x * kx, top + y * ky);
        pic.SetSize(bw * kx, bh * ky);
        pic.Show(true);
    }

    // A read on a back of w x h units: the sign where its fault lies and the
    // face in pencil in the top corner (design §4.4).
    protected void MarkBack(Widget card, float w, float h, OZG_WRead r)
    {
        SignBack(card, w, h, r.Look, r.Sign);
        ImageWidget pencil = ImageWidget.Cast(card.FindAnyWidget("OZG_CardPencil"));
        if (!pencil)
            return;
        int x;
        int y;
        int bw;
        int bh;
        if (OZG_MarkPlaces.PencilBox(x, y, bw, bh))
            LayMark(pencil, OZG_MarkPlaces.PencilImage(r.Face), w, h, x, y, bw, bh);
    }

    // A sign on a back of w x h units, the fault where it lies in the back's
    // pattern (design §4.5): beside a reader's pencil, and all that a noticer
    // sees there.
    protected void SignBack(Widget card, float w, float h, int look, int sign)
    {
        ImageWidget pic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        if (!pic)
            return;
        string style = OZG_Decks.StyleAt(look);
        int x;
        int y;
        int bw;
        int bh;
        if (OZG_MarkPlaces.SignBox(style, sign, x, y, bw, bh))
            LayMark(pic, OZG_MarkPlaces.SignImage(style, sign), w, h, x, y, bw, bh);
    }

    // A noticed card on its face of w x h units: a small dark speck in the
    // margin, where its sign puts it (design §4.5), in the frame's sign
    // picture -- a face shows no fault of the back.
    protected void SpeckFace(Widget card, float w, float h, OZG_WNotice n)
    {
        ImageWidget pic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardSign"));
        if (!pic)
            return;
        string style = OZG_Decks.StyleAt(n.Look);
        int x;
        int y;
        int bw;
        int bh;
        if (OZG_MarkPlaces.SpeckBox(style, n.Sign, x, y, bw, bh))
            LayMark(pic, OZG_MarkPlaces.SpeckImage(), w, h, x, y, bw, bh);
    }
```

- [x] **Step 2: The fans and the stock show a noticed back's sign**

Replace

```c
    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first), the hand of the seat `seat`: a
    // back this player reads there carries its mark (design §4.4).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks, int seat)
    {
        array<int> used = new array<int>();
```

with

```c
    // A fan of backs: `count` cards about (x, y), turned by `turn`, the cards'
    // own looks in `looks` (the lowest first), the hand of the seat `seat`: a
    // back this player reads there carries its mark (design §4.4), a back he
    // notices its sign (§4.5).
    protected void PaintFan(float x, float y, float turn, float scale, int count, array<int> looks, int seat)
    {
        array<int> used = new array<int>();
        array<int> noticed = new array<int>();
```

and in its loop replace

```c
            int readNo = ReadAt(seat, look, used);
            if (card && readNo >= 0)
                MarkBack(card, w, h, m_V.Reads[readNo]);
```

with

```c
            int readNo = ReadAt(seat, look, used);
            int noticeNo = -1;
            if (readNo < 0)
                noticeNo = NoticeAt(seat, look, noticed);
            if (card && readNo >= 0)
                MarkBack(card, w, h, m_V.Reads[readNo]);
            if (card && noticeNo >= 0)
                SignBack(card, w, h, look, m_V.Notices[noticeNo].Sign);
```

In `PaintTable`, replace

```c
        int k;
        array<int> stockUsed = new array<int>();
```

with

```c
        int k;
        array<int> stockUsed = new array<int>();
        array<int> stockNoticed = new array<int>();
```

and in its loop replace

```c
            // The top card: a back this player reads carries its mark (design §4.4).
            int topRead = -1;
            if (k == layers - 1)
                topRead = ReadAt(-1, look, stockUsed);
            if (layer && topRead >= 0)
                MarkBack(layer, pw, ph, m_V.Reads[topRead]);
```

with

```c
            // The top card: a back this player reads carries its mark (design
            // §4.4), a back he notices its sign (§4.5).
            int topRead = -1;
            int topNotice = -1;
            if (k == layers - 1)
                topRead = ReadAt(-1, look, stockUsed);
            if (k == layers - 1 && topRead < 0)
                topNotice = NoticeAt(-1, look, stockNoticed);
            if (layer && topRead >= 0)
                MarkBack(layer, pw, ph, m_V.Reads[topRead]);
            if (layer && topNotice >= 0)
                SignBack(layer, pw, ph, look, m_V.Notices[topNotice].Sign);
```

- [x] **Step 3: The faces show a noticed card's speck**

In `PaintTable`, replace

```c
                string trump = "c_" + OZG_Cards.FaceOf(m_Pub.TrumpCard);
                Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true);
```

with

```c
                string trump = "c_" + OZG_Cards.FaceOf(m_Pub.TrumpCard);
                Widget trumpPic = Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true);
                // A trump this player notices shows its speck (design §4.5).
                int trumpNotice = NoticeOn(m_Pub.TrumpCard);
                if (trumpPic && trumpNotice >= 0)
                    SpeckFace(trumpPic, pw, ph, m_V.Notices[trumpNotice]);
```

In `PaintBout`, replace

```c
            Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true);
```

with

```c
            Widget attackPic = Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true);
            // A card of the table this player notices shows its speck (design §4.5).
            int attackNotice = NoticeOn(m_Pub.AttackCards[i]);
            if (attackPic && attackNotice >= 0)
                SpeckFace(attackPic, w, h, m_V.Notices[attackNotice]);
```

and

```c
                Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true);
```

with

```c
                Widget beatenPic = Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true);
                int beatenNotice = NoticeOn(beaten);
                if (beatenPic && beatenNotice >= 0)
                    SpeckFace(beatenPic, w, h, m_V.Notices[beatenNotice]);
```

In `PaintHand`, replace

```c
            Widget pic = Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true);
            if (!pic)
                continue;
```

with

```c
            Widget pic = Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true);
            if (!pic)
                continue;
            // A card of the hand this player notices shows its speck (design §4.5).
            int handNotice = NoticeOn(card);
            if (handNotice >= 0)
                SpeckFace(pic, m_Look.CardW, m_Look.CardH, m_V.Notices[handNotice]);
```

- [x] **Step 4: The layout's comment says what the sign picture holds**

In `tools/make_layouts.py`, in `card()`, replace

```python
        # A read back's mark (tools/make_marks.py): the sign and the pencilled
        # face, placed by the script in pixels inside the frame, so that they
        # turn with the card.
```

with

```python
        # The marks (tools/make_marks.py), placed by the script in pixels
        # inside the frame, so that they turn with the card: on a back the
        # sign of a card read or noticed, and a read card's pencilled face; on
        # a face, in the same sign picture, a noticed card's speck.
```

Run `python -I tools/make_layouts.py`; `git status --short OpenZone_Games/gui/layouts` lists nothing (the layout files are written again the same).

- [x] **Step 5: Lint, build, look (design §11.3: "the sign on the right back and the speck on the right face")**

Before the build, `knowledge_find(kind="class", name=...)` for `NoticeAt`, `NoticeOn`, `LayMark`, `SignBack`, `SpeckFace`, `SpeckBox`, `SpeckImage`. `mod_lint`, a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Fold the game Task 4 left (`oz_games {"op":"ask","id":"<old id>","ask":"fold"}`, `undeck`). Then a table where the client's player notices every card but three, which he reads:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"mark","id":"<id>","face":"all","who":"marker1"}
oz_games {"op":"mark","id":"<id>","face":"Kh","who":"me"}
oz_games {"op":"mark","id":"<id>","face":"Qh","who":"me"}
oz_games {"op":"mark","id":"<id>","face":"Jh","who":"me"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}
oz_games {"op":"seats","id":"<id>"}
```

then `oz_games {"op":"mark","id":"<id>","face":"all","who":"<b>"}` for each of the three bot ids `<b>` that `seats` names (the bots read every card), and:

```
oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"redeal","id":"<id>","n":"40"}
oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}
oz_games {"op":"notices","id":"<id>","seat":"0"}
oz_games {"op":"reads","id":"<id>","seat":"0"}
```

Expected: `redeal 40: trials=32 hits=32 ... | top 32/32 of 32`; `all: 33 noticed: ...` (the client's player notices the thirty-three cards of the deal he does not read; the three hearts he reads); `notices` and `reads` between them name every back of the bots' fans and the top of the stock. Open the game's window (`client_type` with `j`) and `client_shot`. Crop the shot round each bot's fan, round the stock and the trump and round the hand (Pillow in the scratchpad) and go over them beside `art/marks/classic_signs.png`, `classic_specks.png` and `_view.png`, element by element, before anything is shown to the owner:
1. Every back of every bot's fan, and the top of the stock, carries one fault; a back read (a heart the `reads` answer places there) carries its pencil too, every other back none. The number of faults in a fan is the number of entries `notices` and `reads` give that seat.
2. A fault lies in the lattice as the sheet draws its sign (`notices` gives each back's sign): a speck in a cell, a block-out over a crossing, a scrape, a shaded cell, a nick at the card's edge; none on the emblem, none cut off.
3. Every face of the hand carries a speck, and the trump under the stock one too, but for a heart the player reads (he reads it and does not notice it: no speck): a small dark dot just inside the card's edge, clear of the index, at the place `classic_specks.png` shows for the sign `notices` gives that card (the box `classic_f<sign>` of `OZG_MarkPlaces.c`, in pixels of a 200 x 280 card); on the trump turned with it.
4. The whole shot, not cropped: at a glance the table is a table of plain cards.

Then the table's faces: `oz_games {"op":"auto","id":"<id>","n":"2"}` (an attack and, when it comes, its beating), `client_shot`: the attack card and the card over it carry their specks, turned with them.

Then:
- The stalkers' deck: fold, undeck; the same with `"deck":"OZG_Deck_Stalker"` (`marked 54 of all`: the two jokers are marked too, and never dealt), the window, `client_shot`, points 1 to 3 on the stalkers' lattice and faces, beside `stalker_signs.png` and `stalker_specks.png`.
- `Cheating` 0: fold, undeck; `server_stop`; in `E:\dayzmod\testserver\games-nh\profiles\OpenZone\OZ_Games_Settings.json` set `"Cheating": 0`; `server_start(timeout=600)`, `job_wait`, `log_verdict` = `pass`, the client, `world_teleport`. Lay, `mark face=all who=marker1`, sit, `bots n=3 on=0`, `noticecfg grace=0 step=100 max=100`, `redeal n=3` → `redeal 3: trials=0 hits=0 ...` (nothing counts); `markof face=all who=me only=notices` → `all: 0 noticed:`; `notices seat=0` → `... reads=0 ...: none`; the window (`client_type` with `j`) shows no mark at all (`client_shot`); `log_tail(pattern="notice: ", n=600)` has no new line. Fold, undeck, set `"Cheating": 1` back and restart.
- The owner's screen, if `client_start` gives a 3840 x 1600 window (`docs/2026-11-25-table-window-record.md` says how the table was shot at that size): one shot of a fan, the stock and the hand.

No string of the window changes in this task, so the Ukrainian client is not shot again.

Send the key shots to the owner (`SendUserFile`): a whole table, the crops of a fan and of the hand, the table's faces, the stalkers' table.

- [x] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c tools/make_layouts.py
git commit -m "Noticing: a noticer sees the sign on the backs and a speck on the faces

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 7: The record

What was built and measured goes where the next reader looks for it: the record, the commands to try it, the README, the design's status, the skills when something about the engine was measured, the memory.

**Files:**
- Create: `docs/2026-11-25-noticing-record.md`
- Modify: `docs/test-commands.md`, `README.md`, this plan (its ticks)
- Outside the repository: `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`; the memory `project-table-games.md`; the skills only as Step 5 says

- [x] **Step 1: The record**

Create `docs/2026-11-25-noticing-record.md` in the shape of `docs/2026-11-25-marks-record.md`: a heading `# Noticing (the card sharp's phase 3): record`, the date and how the owner asked for it, and these sections, each filled from the steps' answers and never from this plan's expectations:
- **What was built**: a table, a line a task, with its commit.
- **Measured on the stand**: the settings file's migration (Task 1); the cap of sixteen and the reader refused (Task 2); the single noticing and its log line, the five noticed cards back after a restart (Task 3, Step 4); the two `redeal` answers of the rate, summed, against their bands, and the log's chances (Task 3, Step 5; every run, if there were two); where the noticed ace lay and the sign the view gave it (Task 4); the twenty-six entries, the head's size in bytes, the bots' and the watcher's heads, `deals=0` after the restart (Task 4); the specks' spacing per style, the sheet's pictures and the `.paa`'s size (Task 5); what the window showed, crop by crop, and with `Cheating` 0 (Task 6).
- **Where the build differs from the design**: the notices in the head of the player's own view, not in the seat's private half (§4.6, §9; phase 2's reason); the cards that count are the deal's, not every card of the deck; the count is not reset by a success; the speck's place is its sign's, and the look of the speck is this plan's; a back the player reads shows his read, not his notice, and a reader who does not notice sees no speck; the noticing written to the server's log (§6.1 asks it of sleights and markings); the stand's bots notice too; what else came up.
- **Not done**: the sleights and the dealer's choice (phase 4); the owner's word on the speck's size and ink, and on the fan card's pencil, still open from phase 2.

- [x] **Step 2: What a person tries**

In `docs/test-commands.md`, in `## Marks and the loupe`, replace

```
    oz_games {"op":"markof","in":"me","face":"all"}     every marked card you carry: sign, by you, read by you
```

with

```
    oz_games {"op":"markof","in":"me","face":"all"}     every marked card you carry: sign, by you, read and noticed by you
```

and before `## The verb for scripts` add:

```markdown
## Noticing

A player dealt deal after deal with marked cards he does not read begins to notice them
(card sharp's design §5): five deals of nothing (`NoticeGrace`), then a chance of four
percent more a deal (`NoticeStep`) up to forty (`NoticeMax`); a success is one card,
picked at random among the deal's marked cards he neither reads nor notices. Nothing is
said to him. From then on, at the table, its back shows him its sign in the pattern (no
pencil: he does not know the face), and its face -- in his hand, on the table, as the
trump -- a small dark speck just inside the edge, where that sign puts it. What he
noticed stays on the card; the count of deals starts again with every restart and every
game laid out anew.

To see it at once, on the stand only (the server's memory; a restart reads the file
again):

    oz_games {"op":"noticecfg","grace":"0","step":"100","max":"100"}   every deal makes each player notice one more card
    oz_games {"op":"noticecfg"}                                        the three as they stand

With a game laid out (`<id>`) whose cards somebody who is not at the table marked:

    oz_games {"op":"mark","id":"<id>","face":"all","who":"marker1"}    every card marked by an id that sits nowhere
    oz_games {"op":"ask","id":"<id>","ask":"deal"}                      a deal: you notice one card
    oz_games {"op":"notices","id":"<id>","seat":"0"}                    what seat 0's player is shown: b<seat>:<look>/<sign> on a back, f<card>/<sign> on a face
    oz_games {"op":"markof","id":"<id>","face":"all","who":"me","only":"notices"}   the cards you notice, counted
    oz_games {"op":"whole","id":"<id>"}                                 where every card lies: the hands, the table, the stock in order
    oz_games {"op":"redeal","id":"<id>","n":"40"}                      forty deals at once, the noticing tallied against its chance

`mark ... "who":"<a bot's id>"` (the ids are in `seats`) makes that bot a reader of the
cards, and a reader never notices: a table where you alone notice. `noticer` makes an id
a noticer by hand, as a won roll would.

```

and in `## The verb for scripts`, after the sentence that ends `` `sweep=1` deletes them).``, add the sentence `` For noticing: `noticecfg`, `noticer`, `notices`, `whole` and `redeal` (above).``

- [x] **Step 3: The README**

In `README.md`, in the status list, replace

```markdown
  pattern, and nobody else is told anything. The pen and the loupe wear the game's own
  models for now.
```

with

```markdown
  pattern, and nobody else is told anything. The pen and the loupe wear the game's own
  models for now.
- **Noticing**: a player who is dealt, deal after deal, marked cards he does not read
  begins after a few deals to notice them, one card at a time and by chance, and is told
  nothing: from then on that card's back shows him its mark in the pattern, and its face a
  small dark speck near the edge. What he noticed stays on the card; the count of deals
  starts again with every restart and every game laid out anew.
```

replace

```markdown
`docs/specs/2026-11-24-openzone-games-cardsharp-design.md`; its second phase, marks, is
built ([docs/2026-11-25-marks-record.md](docs/2026-11-25-marks-record.md)), noticing and
sleights are not.
```

with

```markdown
`docs/specs/2026-11-24-openzone-games-cardsharp-design.md`; its second phase, marks, is
built ([docs/2026-11-25-marks-record.md](docs/2026-11-25-marks-record.md)), and its third,
noticing ([docs/2026-11-25-noticing-record.md](docs/2026-11-25-noticing-record.md));
sleights are not.
```

in `## What is where` replace `` `make_marks.py` (the marks a reader sees on a back) `` with `` `make_marks.py` (the marks on the cards: a sign on a back, a reader's pencil, a speck on a face) ``, in `## Settings` replace

```markdown
marked at all (`Cheating`), the pen's uses and how long marking and studying take.
```

with

```markdown
marked at all (`Cheating`), the pen's uses and how long marking and studying take, and
how soon and how fast a player begins to notice marks he does not read (`NoticeGrace`,
`NoticeStep`, `NoticeMax`).
```

and in `## Українською` replace

```markdown
нікому нічого не кажуть. Ручка й лупа поки що мають моделі самої гри. З колодою в руках
```

with

```markdown
нікому нічого не кажуть. Ручка й лупа поки що мають моделі самої гри. Хто роздачу за
роздачею грає міченими картами, яких не читає, з часом починає їх помічати — по одній
карті й випадково, і йому про це нічого не кажуть: відтоді на сорочці такої карти він
бачить мітку в узорі, а на її лиці — маленьку темну цятку біля краю. Помічене лишається
на карті, а лік роздач починається знову після кожного рестарту й кожної нової
розкладки. З колодою в руках
```

- [x] **Step 4: The design's status**

In `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (not a repository): the status line says phase 3 is built and checked on the stand, with the date, this plan's path and the record's path, and that phase 4 is not built.

- [ ] **Step 5: The skills, only for what was measured about the engine**

This phase calls nothing of the engine that phases 1 and 2 did not. If a run nevertheless turned up a fact about the engine (a compile error with a cause not in the skill, a widget that behaved otherwise than `references/gui-layouts.md` says), it goes into the public skill `E:\dayzmod\dayz-agentic-modding-mcp\skills\dayz-modding` with neutral names (`MyMod_`, "a stand mod"), the pointer to our code into the private `E:\dayzmod\dayz-modding-skill\openzone`, then `python -I openzone/scripts/check-generic.py` from `E:\dayzmod\dayz-modding-skill` prints nothing; the skill repositories hold other people's uncommitted changes: stage only this task's hunks, commit each repository once, do NOT push. If nothing was, nothing is written there, and the record says so.

- [ ] **Step 6: The memory**

`C:\Users\56869\.claude\projects\E--openzone\memory\project-table-games.md` gains a paragraph: phase 3 built (the date, the commits), what was measured, what is open; its `description` line says so. `MEMORY.md` stays under 17.1 KB.

- [x] **Step 7: Commit**

```bash
git add docs/2026-11-25-noticing-record.md docs/test-commands.md README.md docs/2026-11-25-noticing-plan.md
git commit -m "Noticing: the record, the commands to try it, the README

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

Do NOT push: the owner alone gives the push word.
