# Stakes, four more games and the games' VPP pages: record

2026-11-25. What was built after the design note `2026-11-25-stakes-and-games-design.md`
(beside this file), what the stand answered, and what has not been seen yet. The owner's
words that started each part are in the design note (§0 there) and, for the admin pages,
in §4 below.

The stand throughout: NHChernobyl, diag 1.30.164289, with SFP 2952842170 and 2623995836,
VPP Admin Tools 1828439124 and the core's `OpenZone_VPP` loaded (the machine profile
`dayz-mcp.local.toml`). A server verdict whose only reasons are SFP's two `Credits.json`
warnings is clean. The client was English, 1920 x 1080; the last runs had it in Ukrainian
(the engine's "Russian" slot carries the Ukrainian texts).

## 1. What is built

- **The module, grown** (design §2): `UsesStakes`, `TakeStacks`/`StackOf`, `HasSleights`,
  `MoveWords`, `BotMove`, `ShownHand` on `OZG_Game`; `OZG_CardGame` under the four new games.
- **Four games** (design §5 to §7): ochko (`OZG_Ochko.c`), Texas hold'em, no limit
  (`OZG_Poker.c`), bridge with Chicago vulnerability and duplicate scoring (`OZG_Bridge.c`),
  preferans by the Sochi convention (`OZG_Preferans.c`). The settings at version 7 list all
  five games; a file of version 6 gets the four added.
- **Stakes in the session** (design §3): the request to sit and its "Let in" / "Refuse", the
  host's chips for a seat, the player's request for a rebuy, a deal of the seats with chips
  only, the stacks read back after every change; the paper (`OZG_StakePaper.c`) when the
  game is folded or the deck gathered.
- **The table view** (design §4): `OZG_GPublic`/`OZG_GPrivate` and the words of the log
  (`KEY|arg|arg`); the window draws the four from it (`OZG_GameMenu.c`, the generic branch:
  seats' fans and face-up cards, zones, status, buttons in a grid, an amount box, the host's
  game button and the chips rows), with three new layout kinds made by `tools/make_layouts.py`
  for both looks (`ozg_gbtn_*`, `ozg_gtext_*`, `ozg_gedit_*`).
- **The stand's tools**: `gfuzz` (a game played by chance and checked after every move),
  `phands`, `bscores`, `pscores` (known hands and scores), and the bots playing the four by
  the games' own `BotMove`, seated with 1000 chips at a game for stakes.

## 2. What the stand answered

### The rules, by chance (`gfuzz`)

Each run: deals one after another, each deal's hint handed to the next, every move the bot's
or, forty times in a hundred, a move picked by chance among those the game offers; after
every move the cards (every card of the deal somewhere exactly once), the chips (a game for
stakes neither makes nor loses any), both halves of the view (they parse), and every seventh
move a save loaded back into a fresh game. A defect is any of these failing.

| game | runs (deals x players) | moves | longest deal | defects |
|---|---|---|---|---|
| ochko | 500 x 2, 300 x 6, 300 x 8 | 2583 + 4406 + 6366 | 11, 24, 34 | 0 |
| hold'em | 600 x 2, 400 x 6, 300 x 8 | 1847 + 5048 + 5725 | 12, 33, 62 | 0 |
| bridge | 300 x 4 | 15 939 | 67 | 0 |
| preferans | 400, 600, 800, 1000 (x 3) | 11 438 + 20 836 + 21 437 + 34 546 | 43, 45, 41, 44 | 0 |

The ochko runs gave 51, 46 and 36 rebuys, the hold'em runs 80, 408 and 386 (a seat with no
chips is given 1000 again, as a host would). The last preferans run (a pool of 10) closed 7
pools. The slowest run took 13.7 s for 1000 deals.

### Known answers

- `phands`: 19 hold'em hands of known rank and order, all right.
- `bscores`: 18 bridge deals of known score (made and set, doubled, redoubled, vulnerable,
  slams), all right.
- `pscores`: 11 preferans deals of known pools, mountains and whists, and the balance of a
  closed pool (`106.7 6.7 -113.3` whists), all right.

Found and put right by these runs, before the numbers above: the preferans bots bid from the
wrong move of their list; the balance was ten times too small; a contract both defenders
passed was scored as failed (it is now given unplayed); the line of a whister's shortfall.

### The boot

`games loaded: games=5 places=3 problems=0`, the verdict clean but for SFP's two warnings.

### The window

Seen on the client, hold'em at a classic place with the host seated: the title "Texas
hold'em — 5/10", the host's rows "Game: Texas hold'em", "Your chips: 0 (brought in 0)",
"Variant: 5/10", "Deal"; after the host gave himself 1000 chips and three bots sat down
(`bots on=0`, waiting): every plate "chips 1000", the log "Survivor gets 1000 chips", "Ostap
sits down with 1000 chips" and so on, the status "Choose the variant and deal".

After the server and the client were restarted (to load the VPP glue, §4) the place came
back with its stakes: the host's "Your chips: 1000 (brought in 1000)", the three bots' seats
kept with "chips 1000 · away" (the stand's bots live one run of the server), the log as it
was, the status "Players needed: 2".

The owner stopped the stand work at this point (leave the stand alone for now) and gave it
back the same evening (go on). Then, the client in English first:

- **Hold'em**: a hand dealt and played to the showdown, the hint right on every street (the
  host held a high card to the river), the pot under the row of cards. No side pot came up.
- **Bridge**: 1NT by the host, the bots bidding and playing, the dummy's fan played by
  clicks (only a card that may be played is lit): three down, 150 to East-West. Another
  deal, 1♠ by East-West, one down: 50 to North-South.
- **Ochko**: a round against a bank of 240, the points counted as the cards came, busts,
  the banker standing; the line "Survivor holds the bank: 474".
- **Preferans**: 6♥ by the host, the talon taken and two cards discarded, the contract
  named, one whister; made, 2 to the host's pool, the balances adding up to nought.
- **Stakes**: a character asking to sit ("Mykola asks to sit", the box, "Let in",
  "Refuse"), let in with 500; his request for chips ("Mykola asks for chips") refused; the
  host picked his plate and gave him 500 from the column ("Chips for Mykola", "Give chips",
  beside "Remove the player").

Then in Ukrainian: the same request to sit and letting in, a hand of hold'em, preferans
dealt to the bidding, the VPP pages (§4), and the fold of the game for stakes: the paper
went into the host's inventory, its text in the server log.

Put right after looking (`da6109f` for the English runs, `a2abfae` for the Ukrainian): the
fans of the seats round the table bulge towards the table, as a player holds his fan in
front of him (the owner, 2026-11-25: they must bulge towards the table, a player sets his
fan in front of him); a plate grows to hold its line, the player's own too, and a
Ukrainian line needs wider letters; the pot and a deal's result stand under the row of
cards; hold'em has only the dealt cards on the board and a bet when nothing is bet; the
bid grid lies beside preferans' talon, not over it; a new game starts a new log; the log's
seat lines carry the name of whoever sat there then; a game with no variant has no variant
button; the bids agree with their number ("1 трефа", "2 трефи", "5 треф").

### The book of chips between deals

The paper of that fold added up wrong: 23 500 chips brought in, 17 500 at the end, though a
game for stakes makes and loses none. Every change at the table copied the stacks of the
deal into the book, the last deal's too after it had ended, so chips given since were
taken back at the next change: six bots seated again after restarts had lost 1000 each.
Measured with no player at all: a hold'em deal played out, 500 given to a seat of it, and
the next deal dealt that seat its old stack.

Now the stacks go into the book while a deal is played and once more as it ends; between
deals the book is the truth (`61ed53a`). The same steps then: the seat was dealt 1490 (990
and the 500), and the paper of the fold after it had 4500 in and 4500 at the end, headed
with the game's name in both languages (it had named preferans by its bare id). The steps
are in `test-commands.md`, "The book of chips, with no player at all".

## 3. The settings reach the players online

Found while building §4: a save of the settings from the core's admin window wrote the file
and the server's settings, but a client keeps what it was sent when it connected (the poses,
the radii, whether card sharping is on) and never asks again, so players online went on with
the old numbers until they reconnected. `OZG_SettingsApplier` now sends the core's sync
packet to everyone online after a save (`OZ_SyncSender.SendAll`), as the radio's profiles
do; the module's card on Ядро › Огляд does the same after "reload every module". Seen on
the stand: a save from the window, and the server's log line `sync: re-sent to 1
player(s): games settings edited`.

## 4. The games in the OpenZone window of VPP

The owner, 2026-11-25: make a VPP submod that fits the existing family of OpenZone's VPP
submods in style and code -- for the games' settings.

Built as the family builds a page (`docs/specs/2026-11-13-openzone-vpp-page-guide.md` beside
the repositories):

- **Three boards** in the series' mockup, `docs/specs/2026-11-12-openzone-vpp-mockup/project`:
  `GamesSettings.dc.html`, `GamesSharp.dc.html`, `GamesPoses.dc.html`, drawn after
  КПК › Налаштування and Рація › Профілі рацій; the module "Ігри" added to `Main.dc.html`
  and the three boards to `canvas.json`.
- **Converted** by css2layout (`ui/css2layout.json`, reports in `ui/css2layout/*.txt`):
  `ozg_vpp_settings`, `ozg_vpp_sharp`, `ozg_vpp_poses` and two row templates
  (`ozg_vpp_game_row`, `ozg_vpp_pose_row`); every caption has its key, every button and box
  its name.
- **The glue pbo `OpenZone_Games_VPP`** (5_Mission only): a layer of the core's
  `OZ_VppAdminMenu` (`OZG_VppPane.c`). The module "Ігри" with three pages over one file,
  `OZ_Games_Settings.json` (its name in the core's console: `games_settings`):
  - **Налаштування**: a row for every game the machine knows (the games of another mod too)
    with its tick and how many play it; the table (most seats, seat spacing, seat reach,
    place spacing), presence (who plays, who sees), time (a game with no players folds,
    the host's role passes, free seats marked), and the mod's detailed log;
  - **Шулерство**: card sharping on or off, the marks' four numbers, noticing with a line
    that works out the chance from the boxes, sleight of hand, the six chances of being
    seen;
  - **Пози**: the list in the order a player steps through it, a card with the id, the
    name's key and how this client words it, the kind, the stance, the gesture; higher,
    lower, new, delete; the checks (how many standing and sitting, an id empty or twice,
    whether this client has the gesture: `OZG_PoseDriver.GestureKnown`);
  - **Файли конфігів** joins the module by itself (the core adds it under every module that
    owns a file of its console).

  One bar at the bottom of every page names what is not saved on any of the three; saving
  writes them all after the window's confirm strip, and a box the server's own check would
  put right without a word (`OZG_Settings.Validate`: most seats from 2 to 12, a percent from
  0 to 100, who sees no nearer than who plays, ...) is refused with the rule named.
- **The games' card on Ядро › Огляд** (`OZG_Facts.c`, in the mod itself): games on, games
  laid out, games being played, players seated, card sharping; the settings' problems.
- **Strings**: 113 keys in `OpenZone_Games/strings.csv`, Ukrainian and English.

The stand: `mod_lint` clean; the converter's reports with no caption without a key; the
family's `lint_scripts.py` silent; the server boots clean with the glue; the client's first
compile stopped on a local named `debug` (a word of the language: renamed, and the fact is in
the `dayz-modding` skill), the second compiled (`client_compile_check`: "client compiles").
Then on the client, in English and in Ukrainian: the three pages draw as their boards; a
save goes through the confirm strip, the server writes the file and sends the sync packet
again (§3), the page reads the file back; a new pose, higher, delete and the checks work;
"Open as a file" shows the file raw. Put right after looking (`4b52506`): the noticing
line lost its percent signs (`string.Format` drops a `%` no digit follows; the skill has
the fact); the games' rows follow the file's order and a save keeps it; the overview card
keeps four short facts, the core writing them on one line of its width; the games list a
little taller and the note under the poses on two lines.

## 5. Left open

- Not seen yet: a side pot in hold'em; a saved setting in a player's window (the packet is
  sent; a pose added has not been looked for in the pose switch); the paper read from the
  item in the game (its text was read in the server log).
- A finished trick of bridge and preferans leaves the table at once, with no pause to see
  it.
- The lines of moves name the seat's player now, as the seat lines did before `a2abfae`: a
  seat taken over in mid-deal (durak, preferans, bridge) renames the moves made before.
- The mockup's design canvas on claude.ai has not been republished with the three boards.

## 6. Published

The owner, 2026-11-25: publish the VPP mod, update OpenZone Games, update the descriptions
and the readmes everywhere, push. Both from the `dayz` MCP, the games first (the glue is
built against them):

- **OpenZone Games** 3845773524: the build of `7c9bb98` (13 115 346 bytes, 16 s), a change
  note in English and Ukrainian, four pictures added to the strip (hold'em and preferans,
  English and Ukrainian), then the new page (`packaging/OpenZone_Games.workshop.bbcode`,
  7915 bytes: five games, the stakes, the VPP module).
- **OpenZone Games for VPP Admin Tools** 3845883015, created public: tags Mod and Mechanics;
  Required items OpenZone Games, OpenZone Core, OpenZone Core VPP and VPP Admin Tools; the
  page `packaging/OpenZone_Games_VPP.workshop.bbcode`; the main picture the English
  Settings page, the strip the three pages in English and Ukrainian.

The public API read both pages back equal to the files.
