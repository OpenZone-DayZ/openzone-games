# The mockups moved into the game: record

2026-11-26. The design is `2026-11-26-boards-and-dice-design.md` (§15 and the owner's
words 34 to 37); the mockups are the design canvas's boards of that day (chess classic and
stalker, draughts classic and stalker, corners, long and short nardy, thousand, dice poker,
pig, high roll, perudo, the card sharp's kit). The owner's words (chat, Russian; here in
English):

- "That is it, go on: the mockups, and then move them into the game. Work by yourself."
  "Do everything in this session."
- "Don't forget to test everything on the stand in the end."

The stand: NHChernobyl, diag 1.30 (`E:\dayzmod\testserver\games-nh`), the client in the
"Russian" slot (the Ukrainian texts) at 1920 x 1080. Nothing was pushed or published
that day; both were done on 2026-11-27 (`2026-11-27-dice-tables-record.md`, "Published").

## What is built

Commits `4941b94` (the board window, the stalker set as items), `fc5238f` (nardy),
`5e5edc9` (the dice games' window), `9ff533e` (the kit's window), `ce160da` (sounds and
the dice's tumble), `ca6d3f9` (what the stand found), and the commit that carries this
record.

- **The windows** are generated from the canvas's measurements by `tools/make_board_win.py`
  (every place is the browser's rectangle of the element on the canvas, in its px, which are
  the layout's units): `ozg_bwin` (chess, draughts, giveaway), `ozg_bwin_corners` (corners:
  its plates carry a bar of the discs in the corner they go to, the move's panel stands
  where the moves were), `ozg_bwin_nardy`, `ozg_dwin` (the five dice games), `ozg_kwin` with
  `ozg_kcard` and `ozg_kdie` (the kit). The game window picks the style
  (`OZG_GameMenu.BoardStyle`: `board`, `board_corners`, `board_nardy`, `dice`); the
  painters are `OZG_GameMenuBoard.c`, `OZG_GameMenuNardy.c`, `OZG_GameMenuDice.c`; the kit
  has a menu of its own (`OZG_KitMenu.c`) and a server half (`OZG_Kit.c`).
- **The stalker set as items**: twelve figurines (`OZG_Chess_Stalker_*`, the trilogy's
  models, under 4 000 triangles each), the stalker discs (`OZG_Disc_Stalker_W/B`, the
  faces on the disc), the dice in three colours and with the stalker sign
  (`OZG_Die_*`), the stalker board and can; in the window the figurines stand inscribed in
  their classic pieces (word 36).
- **The kit's window** (design §10): "Open the kit" on the kit in the hands. The middle is
  every card and die the player carries, grouped where they are (the pockets, a deck, a
  can), each with what he knows of it; a card picked shows its face and its back with the
  mark where its sign lies; a die picked takes a face and a weight. The kit works on it for
  the canvas's 2.3 s (a bar), then the server marks or loads it and spends a use; another's
  mark or load opens to him when he picks the thing, for nothing.
- **Sounds** (`tools/make_sounds.py`, `art/sounds/SOURCES.md`): seven of the canvas's
  sounds, made out of the same numbers its browser code uses -- a piece set down, a piece
  taken, a piece crowned, check, dice thrown, a game over, the kit's click. The games hear
  them (`OZG_Game.HearSound`) and the place carries them to every client near it, as the
  cards' sounds are carried; the kit's click is played on its owner's client alone.
- **Animation**: a move slides (0.35 s), a piece taken fades (0.25 s), and every die in the
  window tumbles for 0.6 s when a throw is heard on the place (the canvas's keyframes, each
  stretch eased by its cubic-bezier).

## What the stand answered

The server and the client compiled once the modded chain was put right (below); after that
every boot judged clean (the only warnings SFP's `Credits.json`). Looked at, each against
its canvas board: chess classic (a game with a bot, moves by the bridge), the stalker chess
(two bots), the stalker draughts (two bots, to the end), long nardy (two bots), corners
(two bots, to the end: the home's check lost it), thousand with three and with six bots,
and the kit's window with a full classic deck, thirteen loose marked cards, a red die and a
can: a card picked, a die picked, a die loaded (the bar, the badge, the use spent, the
words). The server's log has the place's sounds (`place ... heard piece`).

What the stand changed:

- **The modded chain.** `OZG_GameMenu` is reopened in three files, which load in order;
  the board's file called methods the later two declare: "Undefined function
  'OZG_GameMenu#382#12.DiceWindow'". They are declared in the board's file and overridden
  in the later ones (dayz-modding mod-structure.md took the fact).
- **The board's title**: a game's rules after its set, in a second line when they need one
  (the box 146 high and what stands under it lower, as the canvas); the stalker board's
  discs are "сталкерські шашки".
- **The plates**: how many pieces stand on the board (draughts, giveaway), where the side
  goes (corners); a watcher before the game sees the seats taken, not the first two.
- **Corners** got its own window (above), with both homes lit as the canvas lights them.
- **Nardy**: "Кості: господаря", the throw's "кості дошки (господаря)".
- **The dice**: each seat's last throw on its plate (not perudo's, under the cups), smaller
  on a narrow plate; thousand's aside as dice and its turn's points apart; the thrown dice
  on the canvas's five spots, each a little turned, the words hidden while they lie there.
- **The kit**: its middle clipped and scrolled by the wheel (a full deck is taller than the
  panel); the face tiles' ring was clipped by the tile, a button clipping its children as a
  frame does (dayz-modding gui-layouts.md took the fact).

## The afternoon: the kit's scroll and the dice games' own faces

The owner, on a shot of the kit's window with a full deck running past its frame: "the
cards run out of the frame, how do you scroll them? Did you check what you made in the
game? Why did you stop when I said do it to the end and test?" And on the swap panel:
"the dice's pictures belong here, as with the cards, because the dice may differ!!"

- **The kit's middle** is the engine's ScrollWidget now (the frame's clip of the morning
  was never looked at after it was made): it clips the things inside the panel's edge,
  takes the wheel and draws a gold bar while they are taller than the panel. Checked with
  the real mouse on the stand: two decks and two cans clipped, three notches of the wheel,
  a card picked after the scroll (the right one), the thumb dragged to the end.
- **Dice poker, pig, high roll and perudo** have the canvas's faces: each game's parts are
  layouts of their own (`ozg_dg_<game>.layout` on the felt, `ozg_dg_<game>col.layout` in the
  column), made into the dice window when its game is played (the four together would have
  doubled the window to 2 465 widgets). Dice poker: the others' cards (name, rounds won,
  chips, dice, hand, stake or fold), the pot's pill, "У ВАС" with the hand, the round's
  panel with its five steps, the hands from the strongest; pig: a row a seat with the bar
  of the goal and the turn's points after it, the big die (a one with a red pip and a red
  ring), the turn's pill; high roll: a card a seat (whose dice, chips, the dice and their
  total, the stake), the pot; perudo: the others' cards with their cans (the dice shown
  after a showdown, what counts ringed, until the next bid), the bid, the player's own box,
  the bid's panel with the count's stepper, the six faces, "Поставити" and "Не вірю!".
- **The games say their lines for the log** (`OZG_Game.Say`, `SayMove`): a throw and what
  came up, a turn burnt, a round won, a showdown and who loses a die. A move's own words
  stand for its line ("Тарас кидає 4." instead of "Тарас кидає кості").
- **The swap panel of the four** has two rows of dice, both picked by the player (the
  owner: "when it is active there is always a choice of which die for which; the game does
  not decide for the person"): the dice carried as pictures (their colour, the face a known
  load is on, in a grey disc) and the dice he is about to throw (their colour, their place);
  nothing is picked for him and the button lives once both are. The view's head says the
  swaps left, whether one may be done now, what the player knows of each die and the colour
  of every die a seat throws; every die on the felt is drawn in its own colour. Perudo takes
  a swap at a round's start, before the first bid; the die lies from the next round.
- **An engine fault found on the way, in perudo since it was made**: its bid line and its
  showdown wrote the bidder's or the loser's seat for the count -- `W3(key,
  count.ToString(), face.ToString(), OZG_Words.SeatArg(seat))`, two calls of `ToString` in
  one argument list, the later inside `SeatArg` (read back on the stand's bench: a bid of
  three fours by seat 6 as "6 4 s:6"; after the fix "3 4 s:6", and a showdown's count right).
  The words are made into locals first now (dayz-modding enforce-script.md took the fact);
  the faces' painters copy a seat's fields into locals before handing them on.

## Not checked, and open (as it stood at midday)

Settled later the same day, in the afternoon above and the evening below: the kit's wheel,
the tumble's motion, short nardy and giveaway, and dice poker, pig, high roll and perudo,
which have faces of their own now.

- Not checked on the stand: the kit's wheel (the MCP has no wheel); the sounds by ear (the
  server packs them, the client plays them as it plays the cards'); a board's moves by the
  real mouse (the moves went through the bridge as the window sends them); the tumble's
  motion (the shots are stills); short nardy, giveaway, dice poker, pig, high roll, perudo.
- Open against the canvas: the dice games' own parts (dice poker's bank and stakes, pig's
  turn panel, high roll's bets, perudo's bid and its stepper) are drawn by the generic dice
  window, not as their boards are; corners' header has "Згорнути гру" where the canvas has
  "Варіант: 9 шашок" (the variant is the host's row's); the kit's 2.3 s are the client's.
- Found in passing: the client's report says `Cannot activate "UAGear" non-existent
  exclusion set` whenever the game window or the kit opens; `OZG_GameMenu.ShutOut` says the
  game takes single inputs as well as groups. Left as it was: the owner's word on voice
  under the window (2026-11-25) is what that list serves.
- The two `NULL pointer to instance` of the client and the server are vanilla's
  `DayZPlayerInventory.OnHandleStoredJunctureData`, each at a moment the bridge moved the
  kit between the hands and the backpack.

## The evening: the dice finished and tested

The owner: "Finish the dice." And, the stand given back: "the stand is yours, finish and
test everything to the end." Everything below was looked at in the game, on the final
build, at 1920 x 1080 (Ukrainian and English) and at 3840 x 1600 (1.48 pixels a unit, the
owner's window); what was not is named at the end.

### Found and set right

- **A turned die lost its corners.** A turned ImageWidget is cut to its own unturned box
  (dayz-modding gui-layouts §38): every die, its shadow and its ring lie in a frame 1 / 0.6
  of their side now, and the frame is turned (`ozg_bwturn.layout`, `BWTurnPic`). Seen at
  rest and in the tumble (eight frames grabbed).
- **A seat's name read "місце N" on dice poker's cards.** Not the painter: the server's
  `PublicView` handed `m_Seats[i]` and a word made of `m_Hand[i]` to one call, and the
  callee got the hand's last die for the seat. An element read in an argument list is a
  "call" too (dayz-modding enforce-script.md took the fact); the views make locals first.
- **The engine blends in linear light.** The canvas's half-transparent gold over the track
  came out a third brighter (one opaque colour now); a 0.6 black dim left 64 % of the board
  (the alpha that leaves the canvas's 40 % is 1 - 0.4^2.2 = 0.87); a card dimmed by
  alpha wants another alpha for its box than for the words on it (`DGDim`: 0.59 / 0.66 for
  the canvas's .55, 0.53 / 0.62 for .5, 0.48 / 0.58 for .45).
- **Rounded boxes at 1.48 pixels a unit.** A panel fills whole pixels from the floor of its
  place; a picture is drawn at the place itself. Past half a pixel the two part by one
  pixel: a card at 1392.59, 355.56 had its bands from pixel 1392, 355 and its corner
  pictures from 1393, 356, and the felt showed along two sides of three corners. So:
  the bands of an opaque shape lie a whole unit over its corners (`make_board_win.py`
  `shape`), a shape fitted by the script is laid in whole pixels with the bands a pixel
  over (`BWFit`, the kit's `Fit7`), and a box drawn through -- a dimmed card, where an
  overlap shows twice as dense -- is laid with no overlap and moved to a quarter of a pixel
  on the screen (`DGBoxSnug`), where both rules agree. Seen clean on the cards of a folded
  seat, of a seat out of a tie's throw and on pig's row of a seat that left.
- **A tight bold atlas.** `MetronBook-Bold28` drawn larger than it is shows the row of
  glyphs under each letter as specks (dashes under the h, d, n and l of a card's name at
  size 20, which is 29.6 px here; at 19, 28.1 px, none): no bold text above 19.
- **The game over** in the dice window: the canvas's "Гру закінчено" with the game's own
  words, "Нова гра" for the host and "Подивитися на стіл".
- **Before the first deal** a face is drawn from the seats (`DGLobbyView`), as the canvas's
  boards stand before their first stake.
- **Dice poker**: the round that was decided stays four seconds as it lay (phase
  "розкриття", the hands of everybody, the folded seats dimmed), kept by the server until
  somebody throws in the next round; the stakes' step and its hint.
- **Perudo**: the showdown stays 2.6 s with everybody's dice, what counts ringed in gold,
  and stays for good when the game is over; a seat's targets are the dice it has left; a
  player out of the game is told so ("У вас не лишилося кісток — ви вибули.") and has no
  swap panel; the can's picture lost a faint line (its corners were cut with pie slices).
  The canvas's "Вскриття" is "Розкриття" in the game: the Ukrainian word.
- **High roll**: a single highest no longer drops the others from the table's list (the
  losers of the last throw stay to be seen); a seat's stake is said while the stakes are
  made only.
- **Pig** with six seats: the die, the pill and the words stayed on the felt's rim; the gap
  above the die gives way first, then the die (112 for 128).
- **A name longer than its room** is cut with an ellipsis, its "(ви)" kept (`DGNameIn`):
  "Survivor (ви)" ran into "кості господаря" on a card of six.
- **The swap panel**: any number of dice carried (seven a row, two rows in sight, the
  engine's scroll for more: the wheel and real clicks seen on eighteen); the words in
  three lines when they need them; the panel repaints when a swap is sent.
- **Thousand** has the same panel now (its own had room for four pocket dice, each drawn
  as a six), its thrown dice lie in their own colours, and the dice to put aside are picked
  on the felt and put aside by one button, "Відкласти обрані", as its canvas has it (its
  moves came as a row of small buttons, their words cut to nothing); a picked die has the
  canvas's outline of its own shape.
- **A refusal said nothing in the dice window**: its note box was named `OZG_DSeen`, which
  no script knew. It is `OZG_BSeen` as the board window's. The swap refused for room has
  words of its own ("У кишенях немає вільного місця для кістки, яку знімаєте").
- **Words of a set without cards**: "Місце N вибуло з гри" for the log's "Карти місця N
  скинуто", the fold's question about a can instead of a board with pieces, "Встати з-за
  столу" at the felt.
- **English**: three label boxes were too narrow for their translations ("THIS TURN" lost
  half its N); the five dice windows were read through in English.
- **The log** shows the newest entries that fit and wraps a long one (three lines at most).

### Tried and left: a swap with no free pocket

It is refused, as before the evening. An exchange in place was tried three ways on the
stand. Over the ground (the outgoing die to the player's feet, the incoming one into the
slot, the outgoing one into the place it left) the last step failed whenever the incoming
die had sat in a slot of a can the player carries -- where a die given to full pockets
goes: `LocationCanAddEntity` said no to that slot for the die on the ground, at once and
again a second later, and the die stayed at his feet. `GameInventory.ServerLocationSwap`
answered false for the pair and moved nothing. The plain `LocationSwap` moved both on the
server and told no client. `OZG_DiceCheat.Exchange` has the same words.

### What the stand answered

- Server and client verdicts pass on the final build (the only warnings SFP's
  `Credits.json`).
- The rules alone, on the final server: dice poker 60 deals of 5 (3289 moves), perudo 40
  of 5 (4807), high roll 200 of 6 (1338), pig 60 of 4 (8487), thousand 2 of 3 with a save
  loaded back every 150 moves (2687): 0 defects.
- Played with bots and looked at: pig (four and six seats, a seat removed by the host),
  high roll (four and six seats, a tie's throw-off, the winner, the game over), dice poker
  (six seats, two folded, the showdown's step, dice picked with the real mouse), perudo
  (four and six seats, palifico, a showdown, a player out), thousand (a swap by the real
  mouse refused for room and done, three fours picked on the felt and put aside).
- After the changes to every rounded shape, at 3840 x 1600: chess with its game over,
  giveaway, corners, short nardy (never looked at before), the kit's window.

### Where the game is not the canvas, on purpose

- The dice window's game over has two buttons (a new game for the host, a look at the
  table for everybody); the canvas's boards have "Почати знову" in their header.
- Before a deal the right column has the table's own panel (begin, pose, leave, fold), not
  the canvas's "Поставити N".
- Dice poker's showdown passes by itself after four seconds; the canvas waits for
  "Наступний раунд", and at a table nobody holds the others up.
- The swap panel has two rows of dice and a button, in thousand too (the owner: the dice's
  pictures belong there, the dice may differ; the game does not decide for the person).
- High roll's cards stand three in a row from five seats on, pig's die is smaller with six
  (the canvas has four seats).
- The bold name on high roll's card is 19 for the canvas's 20 (the atlas, above).

### Not checked, and open

- Not checked: the sounds by ear; anything that needs a second person (what another player
  is shown when a swap is seen); perudo's card of a player out at 3840 x 1600 (the same
  code as the three that were); the board games and the kit in English; chess and draughts
  at 1920 x 1080 after the shapes' change (seen at 3840 x 1600); the stalker sets.
- A step of one pixel where a corner meets a band, on opaque boxes at 1.48 pixels a unit
  whose place has a fraction past a half: the overlap hides the gap, not the step. The cure
  is what the dimmed cards have -- every shape at a quarter of a pixel -- and wants a pass
  over every window.
- Thousand: the dice put aside are drawn in the set's colour (their zone carries faces
  only); the swap's targets are all five dice though fewer may be thrown; after a server
  restart in mid-game the swap panel stays away until the next deal (the server no longer
  holds the deal's dice).
- The pocket row is as fresh as the last view: a die given by the bridge shows after the
  next change of the game.
- Nardy's own "Підмінити кістку" was not looked at this evening.
