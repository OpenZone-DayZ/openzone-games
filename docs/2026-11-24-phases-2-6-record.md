# OpenZone Games: phases 2 to 6, the record

Date: 2026-11-24, the night after phase 1. The owner's word that evening: finish every
phase without asking, until a mod that can be shown exists -- the basic deck, the game and
every mechanic of durak. So these phases have no plan written before them: this note is
written after, and says what was decided, built and measured. The design is
`E:\openzone\docs\specs\2026-11-23-openzone-games-design.md`; where this note says "§" it
means that file. Everything is on `main`; nothing was pushed.

How to try it alone: `docs/test-commands.md`.

## What exists now

| Phase | What | Where |
|---|---|---|
| 2 | the deck (a kit), the hologram's additions, the place, the ring of seats, the place saved with the world, "Gather the deck" | `OZG_Items.c`, `OZG_Hologram.c`, `OZG_Aim.c`, `OZG_Ring.c`, `OZG_Actions.c`, `OZG_AimHud.c` |
| 3 | the session, both steps of sitting with marks, the host, a seat's four states, the service `games`, the pulse, the session file, the restart | `OZG_Session.c`, `OZG_Sessions.c`, `OZG_Wire.c`, `OZG_MissionGameplay.c` |
| 4 | the window for players and watchers, two decks of cards, Ukrainian and English | `OZG_GameMenu.c`, `OZG_Table.c`, `gui/layouts`, `gui/imagesets`, `tools/make_cards.py`, `tools/make_layouts.py` |
| 5 | poses from the settings, the fan of cards in hands | `OZG_PoseDriver.c`, `OZG_Hand.c` |
| 6 | settings through the core's loader, its admin window and its sync packet; an example for the economy | `OZG_Settings.c`, `docs/examples` |
| art | the mat with a deck, the deck, the fan, a seat's mark | `models/README.md`, `OpenZone_Games/models` |

The stand's mod gained the world's ops, bots and chat commands (`OZG_WorldOps.c`,
`OZG_Bots.c`).

## Decisions made on the way

1. **The server's second look lives in the hologram.** The deploy action asks the server's
   own hologram whether the spot is good, on every tick of the hold and once more at the
   end; a refusal there leaves the deck in the hands, because the kit is deleted only after
   it was placed. So `modded class Hologram.EvaluateCollision` runs the mod's checks on the
   server too, caches the ring for the spot (it does not move during the hold) and says the
   reason once, with `OZ_Rpc.Notice`.
2. **The mode travels in the action's own data.** `OZG_ActionLayOut` writes the placer's
   mode after the position and orientation the game writes, and reads it back on the
   server.
3. **What the hologram shows is a class of its own script.** `OZG_PlacePlacing` is not an
   `OZG_Place`: on the server the game makes the projection as a real, networked thing for
   as long as the deploy lasts, and it must hold no session and never be saved.
4. **The interactions with a place are decided by the server.** One action,
   `OZG_ActionSit`, means "the seats, please", "sit me here" or "open my window" depending
   on who asks; the client only words it. `Watch` and `Gather the deck` are two more.
5. **A view is three texts in one body**, joined by a line feed: the session's head and the
   game's two halves as the game made them. Nesting them as strings would escape every
   quote; compact JSON holds no raw line feed (measured: `lf=-1` on every view).
6. **The place's id is two synchronised numbers**, so a client can tell "the place I sit
   at" from another without a string in the pulse.
7. **A place restored before the games are known waits.** The world's storage is read
   before the mission starts and the games register; `OZG_Sessions.Restore` queues the
   place and `Start` reads its session.
8. **The window draws durak directly.** The frame (seats, log, host) is the platform's;
   the table, the hand and the buttons of the moment are durak's. Another game brings its
   own drawing of the two halves of a view.
9. **A game says how many cards a seat holds** (`OZG_Game.HeldCount`, new in the
   contract): the fan in hands needs a number and nothing else.
10. **The models are made by scripts**, Blender to MLOD to binarize, through the MCP's
    asset tools; the cards are drawn by a script too. Both are in the repository.

## Where the build differs from the design

| Design | Built | Why |
|---|---|---|
| a ground seat from 0.7 m (§5.1) | from 0.95 m, to 1.5 m | at 0.7 m the ring holds five seats 0.7 m apart and no more (measured); durak is for up to six. At 0.95 m it holds eight |
| the place of scope 1, out of spawn lists | scope 2 | a class of scope 1 is never written to the world's storage (measured below) |
| a deploy time of 2 s (§4) | 1 s | the hold is longer than the progress: the body kneels first. At 2 s a hold of 5 s placed nothing |
| the session file under two kilobytes (§9) | 3.5 KB for eight seats | the loader writes it pretty; no string in it is longer than 900 bytes, which is what the wall is about |
| the ring computed again after a restart (§5.1) | the seats come from the session file; the ring is computed anew only when the file gives none | the file is the truth of who sat where; a ring computed anew could move a seat from under a player |
| "more than ten moves a second are dropped" (§10) | refused with a word ("too fast") | a dropped move with no word looks like a broken window |
| the fan: "look at the thing in hand" gesture (§12) | not built | off by default in the design; the fan is held in the lowered hand |
| settings re-sent to clients when an admin saves them | sent when a client connects | the core's sync packet is filled on connection; a changed radius reaches a client at its next connection |

## Measured on the stand, 2026-11-24 (DayZ diag 1.30.164289, world 83)

**The ring** (`oz_games op=ring`, 20 passes each):

| Spot | Mode | Kind | Seats | ms |
|---|---|---|---|---|
| top of `StaticObj_Furniture_kitchen_table_a` (walkable top, 0.77 m) | auto | standing, raised | 8, at 1.04 to 1.34 m | 1.19 |
| the same | ground | ground | 0 | 1.28 |
| open runway concrete | auto | ground | 5 at 0.7 m; 8 at 0.95 m after the change | 0.43 |
| the same | standing | standing | 0 (nothing lower around) | 0.95 |

Under the measurement of 2026-11-23 the same table gave two places: the floor was read
under the mat, and the mat lies on a walkable top.

**The whole path, live client.** Deck in hands, "Choose placement", the hologram on the
table white where it may lie, the panel "Seats: 8 — standing, guessed", **J** to
"standing, chosen", the hold, the mat with the deck on the table, the placer seated at the
nearest seat and the window open. Hold needed: 6 s at a deploy time of 2 s (5 s placed
nothing); the deck now asks for 1 s.

**A game through the window.** A card played by a click, beaten by a character the
script moves for; throwing in, taking, passing; perevodnoy chosen in the lobby; a game of
four played to its end ("Game over. The durak: Bohdan"), 80 moves.

**Seats.** Both steps with marks (green: free; amber: left with cards); standing up
between games; "leave the seat with the cards" in mid-game and the same player taking it
over; the host's role passing to the next seated player, and taken over after the host's
three minutes away (`ticks n=200`, then `host`); a seat removed by the host with its cards
discarded, and the game ending with no durak when one seat was left.

**Restart.** A game stopped in mid-bout (perevodnoy, one card on the table), the server
stopped and started: `place ... restored: session ... st=1 rev=8 ... table=Jc/- ...
hands=0:5,3:6 waits=3`, the line it had before. Seats that had players came back kept.

**The fan.** Three characters at a table, each with a fan of six in the lowered right
hand, backs outwards; gone when its holder stood up.

**The second deck.** A game laid out from `OZG_Deck_Zone`: the worn cards and the olive
back in the window, the same back on the deck on the mat.

## Engine facts found (written into the skill)

- **A class of scope 1 is never written to the world's storage.** Three things made by
  one call on one spot, a server stopped by its window and started again: the two of
  scope 1 were gone, the one of scope 2 stood. `OnStoreSave` of the scope 1 class was
  never called.
- **`Math.RandomInt` starts every server from the same place.** The first deal of two
  boots gave the same trump and the same hands; `Math.Randomize(-1)` at mission start
  ends that.
- **The server's `SetOrientation` does not turn a player.** The body is put on the point
  by `SetPosition`, but where it faces is its client's word: the client turns it through
  one frame of `HeadingModel`.
- **A method or a field named like a vanilla class does not compile**, two more cases:
  a method `Head()` ("Method 'Object' is private"), a field `Head`, a static `Player()`.
- **`UIScriptedMenu` has a `Refresh`** of its own: a menu's own method of that name needs
  `override`, or another name.
- **The language `Original` shows the English column** of a string table.
- **A hidden selection blanked before its texture was ever set stays blank**: a fan whose
  selections were emptied at `EEInit` and filled on the first synchronisation drew
  nothing; left as the config made it until the server's number came, it drew.

## What is not done

- **Signing, `mod.cpp`, a Workshop item.** Nothing is published and no key was made for
  this repository; the owner's word comes first.
- **The window in other sizes.** Seen at 1920 by 1080 only. The layout is in units of a
  1080-row screen and should follow; "should" is not "seen".
- **Watching, seen by a second person.** The window opens for a watcher and the server
  drops a watcher who walks off; no second client was there to see it beside a player.
- **Two real players at one place.** Every other player so far was a character the script
  moved for: the same server code, but no second client.
- **The live hologram on furniture inside houses** (§15.6): tried on a table set on the
  runway, not indoors.
- **The idle rule** was not waited for: thirty minutes with nobody seated fold the place
  into its deck; the code path is the fold's, which was seen.
- **Chairs and benches, other games, stakes**: later specs (§16).
