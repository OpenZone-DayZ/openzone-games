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
"Russian" slot (the Ukrainian texts) at 1920 x 1080. Nothing is pushed or published.

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

## Not checked, and open

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
