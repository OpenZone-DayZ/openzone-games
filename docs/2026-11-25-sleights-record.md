# Sleights (the card sharp's phase 4): record

2026-11-25. The owner's word on how: no plan document with the code in it; the work goes
task by task straight in the code, each task checked on the stand before the next and
committed after it, and this record is kept as it goes: what was built, what the stand
answered, what went otherwise than the design and why. The design is
`E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (§6, §7, §8, §9,
§10, §11.4); the window's mock-up is the design canvas the owner approved on 2026-11-24
(`research_notes/Карты и столы группировок сталкера/table-mock/canvas`, boards Main and
Stalker).

The stand throughout: NHChernobyl, diag 1.30, with SFP 2952842170 and 2623995836 loaded
(the machine profile `dayz-mcp.local.toml`). A server verdict whose only reasons are SFP's
two `Credits.json` warnings is clean.

## Task 1: the settings, version 5 of the file

Built: nine settings in `OZG_Settings` (design §7) -- `SleightsPerDeal` 3, `CaughtSlip` 30,
`CaughtReplace` 25, `CaughtThrow` 30, `CaughtDeckIn` 20, `CaughtDeckOut` 20, `CaughtPick`
25, `SpotPercent` 50, `DealerPicks` 2 -- the file at version 5, a file of version 4 given
the defaults; their bounds (no sleights below none, every chance a percent from 0 to 100,
no picks below none); `CaughtOf(kind)`; the kinds of sleight and their ops in `OZG_Const`.
Stand op `sleightcfg` (the nine set on the running server through the same bounds, the
file not written; `all=` sets the seven chances at once).

The stand:

- the log `[OpenZone] migrated games settings v4 -> v5`; the file at `"Version": 5` with the
  nine defaults, and every value it held kept (`Cheating` 1, `MarkKnowers` 16, noticing
  5/4/40, the squat's gesture 65008); the core's loader left the old file in
  `Backup\games settings.bak.json`;
- `sleightcfg`: `per=3 slip=30 swap=25 throw=30 deckin=20 deckout=20 pick=25 spot=50
  picks=2 cheating=1 putright=0`; given -1, 101, -5, 0, 100, 150, 77, 100, -2 it answered
  `per=3 slip=30 swap=25 throw=0 deckin=100 deckout=20 pick=77 spot=100 picks=2 putright=5`
  (the five out of bounds put back to their defaults, 0 and 100 kept); `all=100` and
  `all=0` set the seven; the file kept the time it was migrated at.

## Task 2: the sleights' sounds

Built: six rows more in `tools/make_sounds.py`'s table, ids 11 to 16 -- `slip` (a card out of
a pocket and into the hand), `swap` (a card slid out of the fan, the pocket, the other slid
in), `toss` (a card flicked onto the heap), `deckin` (pushed along the deck's edge and
home), `deckout` (caught by a nail and drawn out, into the pocket), `pick` (two cards set
on top, the deck squared); shaped noise and damped tones as the table's ten, at `volume`
0.30 (the table's are 0.35 to 0.45): each is played on one client alone (design §6.1).
`art/sounds/SOURCES.md` says so.

The stand:

- the table: `slip 296 ms 6431 B a77e22625e89`, `swap 375 ms 7350 B d864fa51fb7a`, `toss 199
  ms 5461 B 81f95f3df9fa`, `deckin 255 ms 6018 B 0e02b3fd4a37`, `deckout 330 ms 6777 B
  03633e518bd8`, `pick 386 ms 6913 B e27e1e0509e8`; the same digests on a second run, and
  the ten older sounds' digests the same as their record's;
- a client (English, 1920 x 1080) sent `shake ... sound=<name>` for the six, two seconds
  apart: six `dbg: sound <name> on local: started=1` in its script log; no line naming
  `OZG_` or `SoundSetName` in its script log, its RPT or its error log.

## Task 3: the game bears sleights (§6.7)

Built, in the game's contract (`OZG_Game`) and durak: `SleightTakes(face)` (durak: any card
with a rank and a suit; a deuce is simply a low card, a joker none), `SleightName(face)` (a
name the deal never used: `As.1` when `As` is dealt, `Kh.1` when `Kh` was taken out),
`SleightHolds`, `SleightIn` (a record joins a hand and the deal), `SleightOut` (a card leaves
the hand and the deal), `SleightSwap` (both at once, the new card in the old one's place),
`SleightDiscard` (hand to discards, still a card of the deal), `DealtRecords`, and the
dealer's choice `DealFirst(seat, names)`, which `Begin` honours by swapping the chosen cards
into the first places of the dealer's six after the shuffle. Durak's state is version 4: it
keeps the names sleights took out (`Gone`), never given again in the deal; a state of version
3 loads with none. After every sleight `Mend` settles what the rules would have: with cards
on the table the bout may close (`Settle`); with the table empty an opener whose hand a
sleight emptied draws up from the stock, or with the stock gone is out, and the bout opens
anew from the next seat with cards (the defender and the limit worked out again). No sleight
is heard through the place and none goes into the game's log. Stand ops: `cheat` (a
sleight's change told to the bench's game), `begin ... first=<seat>:<cards>`, `fuzz ...
sleights=<per mille>`; and for shots of the window, `ask ... after=<ms>` and `shake ...
repeat=<n> gap=<ms>`.

The stand (the bench, a seeded three-seat deal `0[8c 7s 8s Ks Ah 9d] 1[Ac Js Kh Jc Qc 9h]
2[Ad 9c Tc 8d As Ts]`, trump `Jd`):

- `in As` to seat 0 -> `As.1`, the hand 7; `in 2c` -> `2c`, 8; `out Kh` (seat 1) -> 5 cards,
  `gone=1`; `swap Jc -> Kh.1` (the gone `Kh` not reused), the count kept and `Kh.1` in `Jc`'s
  place; `discard Ts` (seat 2) -> `dis=1`; `in Xr` refused `STR_OZG_NO_CARD`; every answer
  `sound=yes`; `all` showed `gone[Kh Jc]`;
- the opener (seat 2) threw away his five cards one by one with the table empty: at the last
  one he drew six from the stock (`stock=18` -> `12`) and still opened;
- `first=1:Jd As`: seat 1 dealt `[Jd As Kh Jc Qc 9h]`, the trump now `Ac` (the card swapped
  into the deck's last place); six seats `first=5:Ad Kd`: seat 5 `[Ad Kd ...]`;
- `fuzz games=300 seed=11 sleights=60`: `49176 moves, longest 339, sleights 2644 (refused
  546), with a loser 300, 0 defects`; `seed=12 sleights=150 drop=5 away=20 perevodnoy`:
  `35347 moves, drops 206, skipped 947, sleights 4916 (refused 1136), with a loser 258,
  without 42, 0 defects` -- every step's state sound, a save and load every tenth move, no
  game that waits for nobody or for a seat with no move, every game ended;
- the rules untouched: `fuzz games=300 seed=11` and `seed=13 perevodnoy drop=5 away=50`
  answered phase 1's numbers exactly (`52915 moves, longest 411`; `41866 moves, longest 346,
  drops 181, skipped 1485, with a loser 254, without 46`);
- a save with a card gone loaded back (`gone=1`).

Seen in passing: the bench's `load swap=<a>><b>` (phase 1's tool to show a broken save
refused) fails even for `Transfer>Transfer`, a swap that changes nothing, with `JSON ERROR` in
the log, while the same text loads without a swap; it only ever showed refusals. A state of
version 3 is loaded in the restart check instead (task 6).

## Tasks 4 to 6: the server does the sleights, the dealer's three, a restart

Built in one pass, the three sharing the session's code, and checked one after the other on
one stand:

- **In play** (§6.1 to §6.4, §6.6): ops `slip`, `swap`, `throw` of the service. A sleight is
  open to a seated player dealt into the deal in play, present, with `Cheating` 1 and fewer
  than `SleightsPerDeal` done this deal; the second card of one throw comes as `throw` with
  How `more` straight after the first (nothing else changed since) and is not counted
  again. The card of the pockets is named by its item's network id (`OZG_Keys`) and must be
  a card the player carries outside any deck; the server moves it into a free slot of the
  deck in the place, else into the place's own holding, and a card of the hand going out to
  his inventory, else to the ground at his feet -- the synchronous server move
  (`LocationCanAddEntity`, `LocationSyncMoveEntity`, `SendServerMove`), the place opened for
  it. A thrown card stays in the deck. A card of the game is found again by its record's
  face, look, sign and readers (`OZG_DeckRead.MatchDeal`), never by the names a deal gave
  by the walk of the place, which shift once a sleight takes a card out. Then the two rolls
  (`Caught<kind>`, a roll a card; then every other seated player present, `SpotPercent`
  each), the server's log line with the player's id and every roll, the views, a `shake`
  to each who saw it (his hand's region for slip and swap, the discards for throw) and to
  the doer the message `sleight` (what came into his hand and went out, the sound). Nothing
  goes into the game's log.
- **The dealer's three** (§6.5): the deal carries `Stack` ("in=..;out=..;pick=.."). The
  dealer is the host on his own seat; the stack is checked whole (sleights open, no more
  than `SleightsPerDeal` sleights -- a card in one, a card out one, the picks one -- and
  `DealerPicks` picks; a card put in in his pockets; a card taken out or chosen in the place
  and read by him; a chosen card of the game's set; nothing named twice; `*` a card out by
  chance, picked now among the rest), and the deal is first tried on a game of paper with
  the deck as the stack will leave it: a deal it would leave short is refused with nothing
  moved. Then the cards out to his pockets, the cards in into the deck, the deal read anew,
  the chosen cards dealt to him first, the counts of the deal set (his three belong to it),
  and after everybody was sent the deal the rolls of each of his sleights and one shake of
  the stock for each who saw any. The head of the view tells the dealer between deals the
  cards of the deck he reads (`DeckReads`: key, face, look, sign), how many sleights and
  picks he may; in play every dealt player how many he did and may.
- **A restart** (§8): the session file keeps the deal's number and every player's sleights
  in it; the game's state keeps the cards sleights brought in and the names they took out;
  a game in mid-play loads only when every card of its deal is found in the place, the
  deck and its holding, else it is a broken game, made an empty lobby as before.
- Stand ops: `pockets`, `sleight` (a sleight asked for a player, a puppet too, by the face
  he carries), `stack` (a deal with the dealer's three, by faces), `deckreads` (the head's
  sleight fields), `sleights` (the counts), `give ... style=` (a card of a look, into the
  pockets).

The stand (NHChernobyl, the client's player and two puppets in jeans at a classic table,
`sleightcfg all=100`):

- `slip As` by a puppet holding six: his hand `[... As.1]`, seven cards (the deck's own ace
  was in the client's hand); the deck full, the item beside it (`beside=1`); his pockets
  without the ace; the log `sleight: puppet1 seat 0 slip As, place <id>, deal 1: at 100%
  rolled 80: shown; spot at 100% <the client's id>=16 puppet2=45; seen by <the client's
  id>,puppet2`; the client's log a view at 15:14:29.275, then `op=shake`, `sound slip on
  local: started=1` and `shake seat0: 7 cards` 3 ms later; the puppet's fan in the world
  seven cards a second later;
- `swap Kh` (a stalkers' card) for his `9h`: `Kh.1` in the nine's place, `gone[9h]`, the nine
  in his pockets (`(pocket)` in the log line), the king beside the deck; the client shaken
  at `seat0` with `swap`;
- `throw 6d`, then `throw Jd more=1`: the discards 1, 2; the count stayed 3 (`used 3`); two
  log lines `(card 1)`, `(card 2)`; the client shaken twice at `discards` with `toss`; a
  fourth sleight refused `STR_OZG_SAY_SLEIGHT_NO_MORE`;
- at `all=0`: `slip Ad`, `throw 8s` by the other puppet and `swap` of `6c` for `Qd` by the
  client's player: `at 0% rolled 83: not shown` and alike, no shake; the client, the doer
  of the swap, sent `op=sleight`;
- the game with its copies and its strange backs played to its end by the stand (`auto`):
  `over durak=6 ... dis=35 ... gone=2`;
- with every card of the place marked by the client's player, the head between deals: `sleights
  0 of 3 picks 2 deckreads 38` (the 36 of durak's set and the copies the sleights left
  there, the stalkers' king `Kh/1`); a puppet's head `0 of 0 picks 0 deckreads 0`;
- `stack in=6c out=1 picks=As Ah` at `all=100`: `used 3`; the client's hand dealt `[As Ah 7c
  Ad.1 7d Qs]`; his six of clubs dealt to seat 0; the card by chance out of the deck a
  `3d`, now in his pockets; the deck `cards=51 free=1 beside=3` before and after (one out of
  a slot, one into it); three log lines `deckin 6c`, `deckout 3d`, `pick As,Ah ... rolled
  20,80: shown`, each seen by both puppets;
- a restart in mid-play (deal 2, after a slip and a swap by the puppets, the game's state
  rewritten by hand to version 3 with no `Gone`): the game came back, the hands, the stock
  and the counts (`deal 2 sleights: <the client's id>=3 puppet1=1 puppet2=1`), the place
  `cards=51 free=1 ... beside=4` as before;
- a card of that game deleted from the deck (`pull Kc`) and the server restarted: `WARNING:
  session <id>: the game does not add up (1 of its 40 cards are not in the place)`, the
  place an empty lobby (`st=0 rev=1`).

## A restart after a stalkers' card came in from the pockets

Found while the window was tried (below): after a deal with a slip, a swap and four
throws done from the window, the next restart said `the game does not add up (1 of its 40
cards are not in the place)` and the game was lost. The same sleights asked by the stand
(`sleight kind=slip|swap|throw`) on a classic deck came back whole, so the warning was
first made to say what it lacked: each record no item matched and each card item no record
took. Then, step by step:

- a stalkers' nine of clubs given into the client player's pockets read `9c/1` (look 1, the
  stalkers'); after a restart and a reconnect the same card read `9c/0`;
- slipped into a hand in that state and the server restarted: `lacking 9c.1/0/0; left over
  ... 9c/1/stalker/0` -- the game's record said look 0, the item, loaded from the world's
  storage, look 1.

The cause: a card keeps the NAME of its look (the style) in the storage and computed the
number from it in `EEOnAfterLoad`; an item loaded with a character from the players'
database never had it computed and kept the number of a new card, 0, the classic look. The
window drew such a card classic, and a sleight wrote that look into the game's record; the
world's load gave the item its real look, and the record no longer matched it. The fix:
the look is computed in `OnStoreLoad` from the style the moment it is read (the item also
repainted and marked for its clients). Measured after it: a stalkers' queen of hearts given,
the server restarted, the card read `Qh/1`; put into the deck from the window by "Put into
the deck from a pocket", dealt, and the server restarted: `restored: session ... st=1`,
`problems=0`. The warning keeps its new tail (what is lacking, what is left over).

## Task 7: the window

Built, after the canvas's boards Main and Stalker and, for the dealer, the earlier mock-up's
`table_deal.png` (the canvas has no deal):

- **The pockets** ("In your pockets"): the cards the player carries outside any deck, read
  from his own inventory every half second (`OZG_Pockets.c`), faces up, 68 units apart
  while three fit and closer when more must; its rim pink (classic) or rust (stalker) and
  its cards glowing while slip, swap or put-in waits for a card of it; the card picked for
  a swap with the mock-up's solid ring round it.
- **The panel "Sleight of hand"** under the log: "Sleights this deal: n of m", "Slip in from
  a pocket", "Swap a card", "Throw to the discards" (it reads "Done" while a throw is open),
  two lines of hint in the mock-up's words; the armed button's rim and ground, spent
  buttons dimmed with "No more sleights in this deal". Shown in play to a seat dealt into
  the deal; it yields to the host's buttons between deals.
- **The doer's window**: a click asks the server (the card of the pockets by its item's key,
  the card of the hand by its name); when the server's `sleight` comes, his sound plays and
  the cards fly -- pocket to hand for a slip, both ways for a swap, hand to the discards
  (turning to its back) for a throw; a refusal flies nothing.
- **The dealer's frame** (host on his seat, sleights on, before "Deal"): "You deal. Before
  the deal:", the count, "Put into the deck from a pocket", "Draw out of the deck into a
  pocket", and the cards he reads (ten at most) as backs with their sign and the face on a
  pale label in the corner, a tick box under each (as many ticks as `DealerPicks`); the
  deck at the stock's place, lit while a card is to be drawn. Every plan flies at once with
  its sound (pocket to deck, deck or a read card to the pockets, a ticked card to the deck),
  the pockets show what will be there (a back for a card by chance), and "Deal" carries the
  plan as the stack. Nothing of the game that ended is drawn on his table meanwhile.
- 22 strings in English and Ukrainian; the art (`tools/make_table.py`: the pockets' box and
  rims, a button's rim, the tick box and the tick, the ring) and the layouts
  (`tools/make_layouts.py`: `sleights`, `face_label`, `tick`, `rim`).

The stand (shots in `.dayz-mcp/shots`):

- classic, English: "Slip in from a pocket", then a card of the pockets: the server's
  `slip 8h ... at 30% rolled 44: not shown`, the client's `sound slip on local:
  started=1` and `sleight slip flown in his window: 1 card(s)`; swap (the pocket's nine of
  hearts for the six of spades): `swap 6s for 9h (pocket)`, two cards flown; a throw of two:
  `throw 6d (card 1)`, `throw Ad (card 2)`, one card flown each, the count at "3 of 3", the
  buttons spent (`client-1795613830749.png`, `client-1795613883834.png`,
  `client-1795613997299.png`, `client-1795614013199.png`, `client-1795614032098.png`);
- the spotter: a puppet's throw at `throw=100 spot=100`: `seen by bot2,<the client's id>,bot3`,
  the client `shake discards: 2 cards` with the throw's sound; the discards in and out of the
  shake 0.3 s apart (`sleight-shake-classic-discards-pair.png`);
- the dealer's frame, classic, English: a stalkers' queen put in from the pockets ("1 of 3"),
  a card by chance drawn out ("2 of 3"), two tiles ticked ("3 of 3", the buttons spent),
  "Deal": `deckin Qh`, `deckout 5c`, `pick 9h,7s ... rolled 50,88: not shown`, his hand dealt
  with the nine of hearts and the seven of spades (`client-1795615302208.png`,
  `client-1795615399127.png`, `client-1795615428587.png`, `client-1795615456832.png`, the deal
  in flight `client-1795615479884.png`);
- stalker, English: the frame (`client-1795615838055.png`), a deal in flight
  (`client-1795615881159.png`), the ring of a picked card (`client-1795615919667.png`,
  `sleight-swap-ring-stalker-crop.png`), a card laid in flight (`client-1795616100469.png`);
- stalker, Ukrainian (the client's language "Russian"): the panel in the mock-up's words,
  "Оберіть карту в кишені — вона ляже вам у руку." while a slip waits, the dealer's frame
  (`client-1795616318420.png`, `client-1795616329786.png`, `client-1795616585823.png`).

Found on the stand and put right before this commit: the dealer's two buttons were never
shown (the layout's `visible 0`); the faint pencil of a read face on a 58-unit tile was not
to be read, so the tile carries its sign and the face on a label, as the deal's mock-up
drew it; a card picked for a swap had only a brighter glow, now the mock-up's ring; the
stalkers' pockets box went rust all over while armed (its hot rim was a plate under a
see-through ground, now four strips); the stalkers' frame had its title on the cloth's
seam (forty units lower now); the hand the last game left was drawn over the dealer's
second row of tiles.

Otherwise than the mock-ups, and why:

- Between deals the panel of sleights is not shown and "Deal" stays in the column: the
  column holds the host's five buttons then ("Variant" and "Deal" more than in play), and the
  dealer's sleights are in his frame on the table. The deal's mock-up drew "Deal" under the
  frame and the panel dimmed; it knew no "Variant".
- A card taken out by chance is picked among all the cards of the place, not only the faces
  the game deals (a five of clubs came out once); the design does not say.
- At most ten tiles (two rows of five); the dealer with more read cards sees the first ten.

## Push-to-talk under the window (the owner's word, 2026-11-25)

The players must be able to talk while they play. The window shut out the game's "menu"
group of inputs while it was open, and that group (`bin\specific.xml` in `dta\bin.pbo`)
names `UAVoiceOverNet`, `UAVoiceOverNetToggle`, `UAVoiceOverNetMute` and `UAVoiceLevel`
among its inputs; the inventory's group names none of them. The window now shuts out what
"menu" holds less the voice: the groups `inventory` (gestures, aiming, stances, optics,
actions), `hotkey` and `movement`, and the single input `UAGear` -- the game's own script
console hands single inputs to `AddActiveInputExcludes` beside groups, as this does. A
mod's own push-to-talk sits in none of these groups.

Measured with a line the client now writes on every change of its own voice (`dbg: voice
on|off, the game's window open|closed`, from the mission's `IsVoNActive`, which the game's
VON state event sets) and Caps Lock, the game's push-to-talk, held 1.5 s by the stand
(`client_key`):

- before, window closed: `voice on, the game's window closed`, two seconds later `off`;
- before, window open: nothing at all;
- after, window open: `voice on, the game's window open` at 16:22:57.699, `off` at
  16:22:59.699. Walking stays shut out (the `movement` group).
