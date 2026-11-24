# Trying OpenZone Games on the stand

One person can see a whole game: the stand's mod (`OpenZone_Games_Bridge`, server only,
never published) makes characters that play by themselves and takes a few chat commands.
A real server has neither.

## Joining the stand

The stand is world 83 on port 2302, the diag build, with CF, the core and this
repository's four mods. Through the MCP it is `server_start` and `client_start`. On the
series' own machine two launchers in this repository's root, kept out of git, run the
same two lines by hand: `stand-server.local.cmd` (only when no server is up) and
`stand-client.local.cmd` (`ua` or `en` after it sets the client's language first; the
engine's "Russian" slot carries the Ukrainian texts).

### The stand on the NHChernobyl terrain

Since 2026-11-24 the stand has a second world, on the terrain of "STALKER NewHorizon
ChernobylZone" (Workshop 2727569951; its world is `NHChernobyl`). The terrain's `.wrp`
names models of the game and of "STALKER NewHorizon Objects (Legacy)" (2673371735) only,
so those two Workshop items are all the map needs.

- **The world**: mission `ozgames.NHChernobyl` in the stand's `mpmissions` (the map mod's
  own `MapFiles\mission.NHChernobyl`, with `db\economy.xml` switched to spawn nothing: no
  loot, no infected, no animals, no vehicles), instance 84, the clock at 10:00 on every
  start and running at a watch's pace.
- **A stand root of its own**, `testserver\games-nh`, with its own server profile. The mod
  deletes a session file whose place is not in the world, so two worlds sharing one
  `-profiles` directory would each throw away the other's games.
- **The mods**: this repository's four, the terrain and its objects, CF, the core, VPP
  Admin Tools with the core's window in it, and the MCP bridge.
- **Switching**: the machine half of the MCP profile has two variants beside it,
  `dayz-mcp.local.chernarus.toml` and `dayz-mcp.local.nh.toml`; copy the wanted one over
  `dayz-mcp.local.toml` and call `project_open` again. By hand: `stand-server-nh.local.cmd`
  and `stand-client-nh.local.cmd [ua|en]`.
- **Home on the diag client is the game's own key**: it toggles the free camera, and
  leaving the free camera moves the character to where the camera was (`Insert` moves it
  to the point under the crosshair). With VPP loaded the same press opens VPP's panel too.
- **Places with furniture** (from the terrain's own names): Rostok with the bar at
  5979 5878, Yanov 5297 15715, Skadovsk 2974 19089, the village of the Cordon 5197 902.
  A fresh character appears in the south of the map.

## The chat commands

Typed in the game's chat. The server is told only the sender's name, so a command runs
when exactly one connected player bears it.

| Command | Does |
|---|---|
| `!ozg deck` | a deck of cards (classic) into your hands, or your inventory when they are full |
| `!ozg deck zone` | the worn deck |
| `!ozg table` | a kitchen table two metres in front of you |
| `!ozg bots` | two bots sit down at the nearest laid-out game (within 30 m) and start playing |
| `!ozg bots 4` | four of them (as many as there are free seats) |
| `!ozg pause` | the bots wait, or play again |
| `!ozg clear` | every bot stands up and goes; the tables made by `!ozg table` are removed |
| `!ozg noon` | the clock set to noon |
| `!ozg day` | noon, and daylight kept until the server stops (the stand's clock runs twelve times faster than a watch); again to let the night come |
| `!ozg feed` | your character fed, watered and whole (a stand's character starves while you look at a window) |

A bot moves once every two and a half seconds, so a person can follow the game. It beats
with its cheapest card, opens with its cheapest card, throws in cheap cards only and never
a trump, and now and then takes instead of spending a dear trump.

## A game from nothing

1. `!ozg deck`. The deck is in your hands.
2. Find a table, or `!ozg table`. Any immovable flat thing will do, the ground too.
3. With the deck in hands take "Choose placement" (the item's own action). The mat's
   hologram follows your aim. The panel at the bottom says how many seats the spot has
   and of what kind ("Seats: 8 — standing, guessed"), or why the spot is refused.
   **J** steps through the seat modes: guessed, standing, on the ground. The key is in
   Settings, Controls, under OpenZone Games.
4. Hold "Lay out the game" for about five seconds. The mat with the deck lies on the
   table, you are the host and, when a seat is within a metre, you are seated and the
   window opens.
5. `!ozg bots`. Two bots sit down.
6. In the window: "Variant" (podkidnoy or perevodnoy), then "Deal".
7. Play. A card that can be played now is lit; a click plays it. When a click could mean
   more than one move (beat one of two cards, or beat and move the attack on) the card is
   picked up first: click the card on the table it beats, or "Move on".
8. Esc closes the window; the seat stays yours. Look at the mat and take "To the game" to
   come back.

## What else there is to see

| To see | Do |
|---|---|
| the seats' marks, and both steps of sitting | stand up ("Stand up" in the window, between games), look at the mat, take "Sit down to the game": the free seats are marked for 20 s. Stand on a mark and take the action again ("Sit here") |
| a seat left with cards | in mid-game take "Stand up", then "Stand up: leave the seat with the cards". The seat's mark is amber; sitting on it gives you its cards back |
| discarding in mid-game | "Stand up: discard my cards": your hand and the table go to the discards and the game goes on without you |
| the host's tools | click a seat on the strip at the top: "Remove the player" (discard his cards, or leave the seat with them), "Discard the cards left here" |
| watching | with no seat at the game take "Watch" on the mat: the same window without a hand and without buttons. It closes when you walk more than five metres away |
| the fan of cards | while a game runs every present seated player whose hands are empty holds a fan, as many cards as he has (eight at most). It goes when the hands are needed for something else |
| a restart | stop the server in mid-bout and start it: the place, the seats, the hands and whose move it is are as they were |
| gathering the deck | "Gather the deck" on a mat where no seat is taken, kept or left with cards; the host folds at any time from the window |

## The verb for scripts

The same and more through the MCP bridge, `world_exec(verb="oz_games", args={"op": ...})`:
`ring`, `lay`, `places`, `sess`, `seats`, `puppet`, `move`, `ask`, `sit`, `ticks`,
`gather`, `hand`, `give`, `chat`, `bots`, `auto`. The header of each op in
`OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` says what
it takes. The ops of the rules alone (`begin`, `play`, `fuzz` and the rest) are in the
plan of phase 1.
