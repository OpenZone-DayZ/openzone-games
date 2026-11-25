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
| `!ozg deck stalker` | the stalkers' deck: stalkers on the aces and kings, mutants on the queens, jacks and tens, and two jokers (54 cards) |
| `!ozg deck sharp` | a card sharp's deck (classic): the same fifty-two cards and ten empty slots more |
| `!ozg deck stalkersharp` | a card sharp's deck in the stalkers' look: the 54 and ten empty slots (64) |
| `!ozg table` | a kitchen table two metres in front of you |
| `!ozg bots` | two bots sit down at the nearest laid-out game (within 30 m) and start playing |
| `!ozg bots 4` | four of them (as many as there are free seats) |
| `!ozg pause` | the bots wait, or play again |
| `!ozg clear` | every bot stands up and goes; the tables made by `!ozg table` are removed. A bot whose game is folded goes by itself within three seconds |
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
   **J** steps through the seat modes: guessed, standing, sitting, on the floor.
   "Standing" and "sitting" put the seats on the floor below the thing the mat lies on,
   "on the floor" on the level of the mat itself. The key is in Settings, Controls, under
   OpenZone.
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

## The deck is a box of cards

Since 2026-11-24 a deck holds its cards as items, in slots: fifty-two on an ordinary
deck, sixty-two on a card sharp's. **A deck from before that day is empty** (it never
fills itself again): take a new one with `!ozg deck`.

1. With the deck in your hands open the inventory: its slots are under it, eight to a
   row, an empty one as a round ghost. Drag a card out into your pockets; drag it back
   onto an empty slot. (Dropped on the deck's own picture a card is not put in: the game
   swaps your hands. All cards show the same picture for now, a card's own is not drawn
   yet: the cursor over a slot names the card.)
2. Lay the deck out short of that card and deal: the game deals what there is and says
   nothing. With too few cards for the players at the table the deal is refused: "not
   enough cards in the deck".
3. "Fold the game" (the host, from the window) or "Gather the deck" on the mat: the
   very deck comes back into your hands with the cards it had. There is no count in
   the window: a deck is counted by its slots, in the hands of whoever holds it.
4. More cards than slots: put the cards of a second deck into a card sharp's deck
   until it is full (sixty-two). What a game holds beyond its deck's slots (the
   script's `stuff` puts cards there; the sleights of a later phase will) stays on the
   ground where the mat lay when the game is gathered, as many cards as found no slot,
   picked by chance among all of them.
5. A card of another deck's look keeps its back: in the window it is drawn in its own
   art, and a seat's strip says how many strange backs its hand holds (`6 (1*)`).

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
| gathering the deck | "Gather the deck" on a mat where no seat is taken, kept or left with cards; the host folds at any time from the window. The deck that was laid out comes back, into free hands or onto the spot |

## Marks and studying a card

The card sharp's kit (until 2026-11-25 a pen) marks a card through the game's crafting,
three cards to a kit (`MarkUses`), and an empty kit is gone: the kit in your hands, a card you
carry dragged onto it, then hold the left button (8 s, `MarkSeconds2`; the gesture takes
about 2 s more). A card in a deck cannot be dragged: take it out first. A card in your
hands is studied closely by its own action, "Study closely" (5 s, `StudySeconds`): you
are told alone "The card is marked" or "Nothing found", and you read a marked card from
then on. (The loupe did this until 2026-11-25; it is gone.)

    oz_games {"op":"give","what":"OZG_MarkTool"}        the kit, into the hands
    oz_games {"op":"give","what":"OZG_Card_As"}         a card, into the hands (the inventory when they are full)

Without the gesture, the server's half of it:

    oz_games {"op":"craft","what":"mark","face":"As"}   the thing in the hands on the ace of spades you carry
    oz_games {"op":"craft","what":"study","face":"As"}  the end of "Study closely" on the ace of spades you carry
    oz_games {"op":"markof","in":"me","face":"all"}     every marked card you carry: sign, by you, read and noticed by you

At the table, with the window open, every back you read in another seat's fan and on
the stock carries its fault and your pencil:

    oz_games {"op":"mark","id":"<id>","face":"all"}     every card of a laid-out game marked by you
    oz_games {"op":"reads","id":"<id>","seat":"0"}      what seat 0's player reads now

`Cheating` 0 in `OZ_Games_Settings.json` switches all of it off: nothing is marked,
studied or drawn.

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

## Sounds and flights

Every change of a game is heard by every client near the place, whether its window
is open or not: the deck put down when a game is laid out and gathered up when it is
folded, the shuffle, the deal's quick run and the trump turned up, a card laid on the
table and a card laid over it, the table taken or swept to the discards, cards drawn
from the stock. The sounds are the mod's own, made from numbers by
`tools/make_sounds.py`. With the core's debug switch on, a client's script log names
every sound it plays (`dbg: sound lay on place <id>: started=1`).

In the window the cards fly: from the stock into every hand at a deal, while the deal
is heard, the trump turned up last; from a hand onto the table; from the table to the
discards or into the taker's hand; from the stock into the hands after a bout. The
log says what flew (`dbg: flights rev 12: laid 1, swept 4, drawn 3`).

    oz_games {"op":"sound","id":"<id>"}                              the place's sound number: its count, what it carries now, its last lot
    oz_games {"op":"sound","id":"<id>","play":"shuffle,deal,trump"}  a lot put on the place, as a deal puts it
    oz_games {"op":"sound","in":"me"}                                the number of the deck in your hands (gathered up)
    oz_games {"op":"shake","id":"<id>","region":"seat1","sound":"lay"}   what a player who saw a sleight will be sent: the sound for him alone, seat 1's fan shaken
    oz_games {"op":"heard"}                                          the bench: what its game made heard since the last ask

The regions of a shake are `seat<n>`, `discards` and `stock`.

## Sleights (the card sharp's phase 4)

With `Cheating` 1 a player dealt into the deal in play sees, under the log, the panel
"Sleight of hand" and, on the table, the strip "In your pockets" (the cards he carries
outside any deck). Three sleights a deal (`SleightsPerDeal`):

1. **Slip in from a pocket**: the button, then a card of the strip: it flies into the hand.
2. **Swap a card**: the button, a card of the strip (it gets a ring), then a card of the
   hand: they change places.
3. **Throw to the discards**: the button (it reads "Done" now), then one or two cards of the
   hand; "Done" ends it. One sleight however many cards.

Before a deal the host on his seat (the dealer) sees his frame on the table instead: "Put
into the deck from a pocket" (then a card of the strip), "Draw out of the deck into a
pocket" (then the deck for a card by chance, or a card of the row he reads), and a tick
under each card he reads to deal it to himself (`DealerPicks`, 2). Each act is one
sleight; the ticks together are one. "Deal" does them all.

Every sleight rolls (`Caught<kind>`): if it shows, every other seated player present rolls
`SpotPercent`; who saw it is shaken that part of the table with the sound, and nobody's log
says anything. The server's log has a line for every sleight with the player's id
(`sleight: <id> seat <n> <kind> <what>, place <id>, deal <n>: <the rolls>`).

The stand's ops (the place's id `id=`, a player `who=me|<id>`; a puppet holds cards in jeans):

    oz_games {"op":"sleightcfg","all":"100","spot":"100"}               every sleight shows and everybody sees it (all=0: nobody)
    oz_games {"op":"sleightcfg","per":"10"}                             ten a deal (the file is not written: a restart reads it again)
    oz_games {"op":"give","what":"OZG_Card_Qh","style":"stalker"}       a card of a look into the pockets (hands=1: into the hands)
    oz_games {"op":"pockets"}                                           what you carry: face/look/sign and where
    oz_games {"op":"sleight","kind":"slip","face":"Th"}                 slip the ten of hearts you carry into your hand
    oz_games {"op":"sleight","kind":"swap","face":"Ks","card":"Tc"}     your king of spades for the ten of clubs of your hand
    oz_games {"op":"sleight","kind":"throw","card":"Ad"}                a card of your hand to the discards
    oz_games {"op":"sleight","kind":"throw","card":"Ac","more":"1"}     its second card
    oz_games {"op":"sleight","who":"bot1","kind":"throw","card":"Jh","after":"1500"}   a puppet's, in 1.5 s
    oz_games {"op":"stack","in":"6c","out":"1","picks":"As Ah"}          deal with the dealer's three
    oz_games {"op":"deckreads"}                                         what the dealer may choose or draw by name
    oz_games {"op":"sleights"}                                          the deal's number and everybody's count
    oz_games {"op":"ask","ask":"act","move":"a:6c","after":"2500"}      a move in 2.5 s (for a shot of a card in the air)

A dealer only ever chooses cards he reads (marked by him or studied closely), never one he
only noticed.

## The later games and their stakes

Ochko, Texas hold'em, preferans and bridge are laid out like durak (a deck, "Choose
placement", "Lay out the game"). In the window the host's column has "Game: <name>":
between deals each press steps to the next game the server offers (the settings'
`GameIds`, in the order durak, ochko, hold'em, preferans, bridge). Preferans needs three
seated players and bridge four; ochko and hold'em two to eight.

**Chips** (ochko and hold'em). A seated player's column says "Your chips: N (brought in M)".

- The host gives chips between deals: a click on a seated player's plate shows "Chips for
  <name>", a box for the amount and "Give chips" (the host gives himself chips the same
  way, from his own plate).
- Somebody who takes "Sit here" at a game for stakes is not seated: he is told "You asked
  the host to let you in", and the host's column shows "<name> asks to sit" with the box,
  "Let in" and "Refuse". Letting in with 0 gives back the stack a returning player left
  with.
- A seated player who is not the host has "Ask for chips"; the host sees "<name> asks for
  chips" and gives or refuses as above.
- A deal takes only the seats that have chips.
- Folding the game or gathering the deck leaves a paper (the game's own note) with whoever
  did it, else on the ground: the game, the world's date, and a line a player -- brought
  in, at the end, the difference. Read it with the game's own "Read".

**Bots**: `!ozg bots 3` at a game for stakes seats three bots with 1000 chips each; they play
the later games by each game's own simplest sensible move.

The same through the MCP bridge (`<place>` is the id `places` names):

    oz_games {"op":"ask","id":"<place>","who":"me","ask":"game","val":"poker"}
    oz_games {"op":"ask","id":"<place>","who":"me","ask":"chips","seat":"0","val":"1000"}
    oz_games {"op":"bots","id":"<place>","n":"3","on":"0"}       three bots, waiting
    oz_games {"op":"bots","id":"<place>","n":"0","on":"1"}       the bots play
    oz_games {"op":"ask","id":"<place>","who":"me","ask":"deal"}

`val` of `game` is a game's id: `durak`, `ochko`, `poker`, `preferans`, `bridge`. The
host answers a request with `ask=let` (`how` the asker's id, `val` the amount) or
`ask=refuse` (`how` the asker's id); a seated player asks for chips with `ask=rebuy`.

**The book of chips, with no player at all**: a place laid by script, a puppet for its
host, two bots; a deal played out, then chips given to a seat of that deal. The next deal
must deal that seat its last stack and the chips given (until 2026-11-25 the chips given
between deals were taken back at the next change); the fold leaves a paper whose two
columns add up to the same sum (the server log has it: `place <id> paper: ...`).

    oz_games {"op":"lay","at":"<x y z>"}                              a place by script (flat ground, two metres from any other); its id
    oz_games {"op":"puppet","id":"<place>","seat":"0","name":"Hosta"}  a character on seat 0, the host; its id (puppet1 on a fresh server)
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"game","val":"poker"}
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"chips","seat":"0","val":"1000"}
    oz_games {"op":"bots","id":"<place>","n":"2","on":"0"}
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"deal"}
    oz_games {"op":"auto","id":"<place>"}                             the deal played to its end; the stacks
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"chips","seat":"2","val":"500"}
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"deal"}  seat 2: its stack and 500
    oz_games {"op":"ask","id":"<place>","who":"puppet1","ask":"fold"}  the paper

**The rules alone, by script**: `gfuzz` plays one game deal after deal with no world at
all (the bots' moves and, forty times in a hundred, a move by chance) and checks after
every move that every card is somewhere exactly once, that no chip is made or lost, that
both halves of the view read, and every seventh move that a save loads back; it answers
the moves, the longest deal, the rebuys it gave and the defects (0 expected). `phands`,
`bscores` and `pscores` play hands and deals whose answer is known (hold'em's ranks,
bridge's scores, preferans' pools and a closed pool's balance):

    oz_games {"op":"gfuzz","game":"poker","deals":"300","players":"8","chips":"1000","seed":"1","opt":"blinds=5-10"}
    oz_games {"op":"gfuzz","game":"preferans","deals":"1000","players":"3","opt":"pool=10"}
    oz_games {"op":"phands"}    {"op":"bscores"}    {"op":"pscores"}

The numbers of 2026-11-25 are in `2026-11-25-stakes-and-games-record.md`.

## The games in the OpenZone window of VPP

With `OpenZone_Games_VPP` loaded (it needs VPP Admin Tools and the core's
`OpenZone_VPP`; the NHChernobyl stand loads all three):

1. Click into the client's window once (keys reach a freshly started client only after
   that), then **End** (VPP's admin mode; it may already be on) and **Home** (VPP's panel;
   on the diag client Home also toggles the free camera).
2. The OpenZone button (the OZ-COM sign) on VPP's toolbar opens the series' window; the
   rail on its left has the module **Ігри / Games**.
3. **Налаштування**: a tick for every game (switch one off, and the host's "Game" button
   skips it), the table, presence, time, the mod's detailed log. **Шулерство**: card
   sharping on or off, the marks, noticing (the line under it works the chance out of the
   three boxes), sleight of hand, the chances of being seen. **Пози**: the list, a card,
   higher / lower / new / delete, and the checks (the last asks this client whether it has
   the pose's gesture). **Файли конфігів**: the same file raw.
4. Change something on any page: the bar at the bottom of all three names it in the alert
   colour. "Зберегти / Save" asks on the window's strip; "yes" sends the file, the journal
   says it was applied, the boxes read it back. A box out of the server's bounds (most
   seats outside 2 to 12, a percent above 100, who sees nearer than who plays) is refused
   with the rule named, and nothing is sent.
5. A player online gets the saved settings at once (the core's sync packet is sent to
   everyone): a pose added shows in his window's pose switch without reconnecting.
6. **Ядро › Огляд** has the card **Ігри**: games on, games laid out, being played, players
   seated, card sharping, and the settings' problems.

## The verb for scripts

The same and more through the MCP bridge, `world_exec(verb="oz_games", args={"op": ...})`:
`ring`, `level`, `lay`, `places`, `sess`, `seats`, `puppet`, `move`, `ask`, `sit`,
`ticks`, `gather`, `hand`, `give`, `chat`, `bots`, `auto`, `face`, `sound`, `shake`, and
for the cards `cards` (the deck a player carries, counted), `pull` (cards of the named
faces out of a carried deck or a laid-out one, and gone), `undeck` (a player's decks
deleted), `stuff` (cards of a face and a look into a laid-out game: the deck's free slots, then beside
it), `held` (what a place holds: its deck, and how many cards beside it), `loose` (the
cards lying on the ground near a point; `sweep=1` deletes them). For noticing:
`noticecfg`, `noticer`, `notices`, `whole` and `redeal` (above). `lay` takes `deck=`
(a deck's class), `give` too. The header of each op in
`OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c` says what
it takes. The ops of the rules alone (`begin`, `play`, `fuzz`, `heard` and the rest) are
in the plan of phase 1; `begin` now takes `cards=` and `looks=`, the cards a deal is dealt
from and the looks of their backs, as a session hands them over. `auto` plays fifteen
to twenty moves a call (a session lets a player make so many a second): ask again
until the session says `st=2`.

## A table set out to be looked at

`stage` (`OpenZone_Games_Bridge/.../OZG_Stage.c`) changes a running game so that the
window shows what is wanted, without playing to it: `hand=n` (the seat `seat`, 0 by
default, holds n cards), `fan=n fanseat=k` (another seat holds n, a fan of n backs),
`table=n` (the bout is n cards: (n + 1) / 2 attack cards, n / 2 of them beaten). It goes
through the game's own state (`SaveState`/`LoadState`): every card added is a further copy
of a face written into the deal, every card taken away goes to the discards, so the
game's soundness check holds. The rules' limit is not checked: `table=24` lays more than
durak ever does. Fold the game after looking.

    world_exec verb=oz_games args={"op":"stage","id":"<place>","table":"12"}
    world_exec verb=oz_games args={"op":"stage","id":"<place>","hand":"24","fan":"24","fanseat":"2"}

The bout of real play grows best from a deck of few ranks: `pull` every card from the
nine up out of a laid-out game and `stuff` two more copies of each six, seven and eight;
then almost every card can be thrown in (3, 6 and 9 cards were seen so, 2026-11-25).
