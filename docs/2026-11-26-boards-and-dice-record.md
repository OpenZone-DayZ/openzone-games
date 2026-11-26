# Boards and dice, phases 1 to 5: record

2026-11-26. The design is `2026-11-26-boards-and-dice-design.md`; phase 1 had a plan
(`2026-11-26-boards-phase-1-plan.md`), the rest went task by task in the code, each checked
on the stand. The owner's words on the way (chat, Russian; here in English):

- "I do not like how the chess looks: why the card background there and so on. Finish the
  code for the games, then we make mockups for all the new games." So the windows of the
  eleven games work but are not drawn to any look: their look waits for the mockups.
- "The kit's window too: a mockup first." So loading a die and marking from the kit's
  window (design §10) have their server half and stand ops, and no window yet.
- The stalker chess is the owner's (design §15); nothing of it is here.

The stand throughout: NHChernobyl, diag 1.30 (`E:\dayzmod\testserver\games-nh`). A server
verdict whose only reasons are SFP's `Credits.json` warnings is clean.

## What is built

Commits `1dec815` (phase 1), `57a923d` (phase 2 and nardy), `47f522a` (dice), and the
commit of phase 5 that carries this record. Nothing is pushed or published.

- **Phase 1, the board and chess.** A place holds a set (`ozgSetKind` cards / board / dice,
  `ozgPlace` the place's class); `OZG_Board_Classic` holds its 64 pieces in slots of their
  own and lays out as `OZG_BoardPlace`. Chess by the FIDE laws without a clock
  (`OZG_Chess.c`). The session deals a board game from the pieces the board really holds
  and refuses with what is missing. Models of the board, the pieces, the disc and the die
  (`models/README.md`).
- **Phase 2, draughts, giveaway, corners** (`OZG_Draughts.c`, `OZG_Corners.c`) on
  `OZG_BoardDuel`, what every two-sided board game shares: colours, resigning, a draw
  offered and accepted, the game kept as its moves and replayed on load.
- **Phase 3, nardy** (`OZG_Nardy.c`): long and short on the board's other side, which the
  place turns up when nardy is chosen; dice through `OZG_DiceCup`, whose dice an option.
- **Phase 4, the dice games** (`OZG_DiceGame.c` and one file a game): the can
  (`OZG_DiceCan_Classic`, five slots, laid out as `OZG_DicePlace`); thousand with its house
  rules, dice poker by The Witcher's rules for chips, pig, high roll for chips, perudo on
  own dice with palifico; whose dice: the host's, own, by wish (design §8). A dice game is
  kept as its moves and every die as it fell; loading plays it again handing out the dice
  recorded instead of throwing.
- **Phase 5, cheating** (`OZG_SessionCheat.c`): the board sleights (§9.1), the dice swap
  (§9.2), loading a die with the kit (§10, the server's half), studying a die closely, and
  noticing a loaded die. The window is told of a board sleight it sees (`OP_BSAW`: the words,
  the squares lit); it has no buttons to do one, nor a swap, until the mockups.
- **Settings version 9**: the eleven games in `GameIds`, `BoardSleightMinutes` 5,
  `LoadWeights` 30/50/70, `NoticeLoaded` 4, `DiceSwaps` 2, `ChanceDiceSwap` 30; a file of an
  older version is given them. Not yet in VPP's module (phase 7).
- **Stand**: `!ozg board`, `!ozg can`, `!ozg die`, `!ozg kit`; ops `perft`, `dperft`,
  `split`, `splitlen`, `dice`, `loaddie`, `begin ... pos=`, `gfuzz ... reload=`, `ask`
  with `key=`, `noticecfg loaded= swaps= swapshow=`, `sleightcfg board=`
  (`test-commands.md`, "Boards and dice").
- **Economy lines**: the board, the can and every piece class in
  `examples/types-openzone-games.xml` (written by `tools/make_board_items.py`).

## What the rules answered

- **Chess perft** agrees with the published counts: the start position to depth 4 (20,
  400, 8902, 197281), Kiwipete to 3, positions 3 to 5 of the usual table.
- **Draughts perft** from the setup: 7 at depth 1, 7482 at 5, 37986 at 6, 190146 at 7.
  No published table for draughts-64 was found to compare with, so these numbers are the
  rules' own and nothing more. The rules were checked on hand-built positions instead: a man
  taking backward; a chain of takings; a man crowned in the middle of a taking and going on
  as a king; a king landing only where its taking goes on, when it can.
- **gfuzz** (every move checked: the pieces or dice sound, no chip made or lost, both
  halves of the view read, a save loading back), all with 0 defects:

  | game | deals x players | moves | longest |
  |---|---|---|---|
  | chess, bots only | 20 x 2 | 3430 | 258 |
  | draughts, bots only | 100 x 2 | 4699 | 85 |
  | giveaway, bots only | 100 x 2 | 4802 | 81 |
  | corners (size 12), bots only | 60 x 2 | 4748 | 80 |
  | long nardy, bots only | 6 x 2 | 1847 | 339 |
  | short nardy, bots only | 6 x 2 | 1390 | 296 |
  | short nardy | 20 x 2 | 209 | 50 |
  | pig | 40 x 2; 20 x 4 | 3398; 2914 | 134; 207 |
  | perudo | 30 x 2; 20 x 5 | 866; 2368 | 38; 136 |
  | high roll | 60 x 2; 60 x 6 | 124; 398 | 4; 10 |
  | dice poker | 40 x 2; 30 x 4 | 611; 1300 | 24; 76 |
  | thousand | 2 x 3; 4 x 3 bots only | 3116; 6044 | 2215; 2277 |

  Every run of the table is of the games' code as committed with this record (before it, phases 2
  to 4 had 200 games of draughts and of giveaway and 60 of corners clean too). Without
  `wild=0` a board game mostly ends in a resignation picked by chance, so the board rows
  that matter are the bots' own. A thousand of three with all the house rules is long: 1511 moves a game on
  average for the bots, up to 2277. With a save loaded back every seventh move such a
  game stalled the stand's tick for minutes (each load plays the game again), so its runs
  load back every 150th.

## What the stand answered

- **The board**: a chess game dealt from the pieces the board holds, played, kept through a
  restart; folding gave the board back whole.
- **The can**: pig with the host's dice; perudo refused while the seats carried no can of
  their own, and dealt when they did.
- **A board sleight**: `off` on d8 took the queen off; a second one within
  `BoardSleightMinutes` was refused with the wait; the king off was refused; the position
  stood so after a restart.
- **Loading**: a die loaded 6 at 70% threw sixes (nine of eleven throws, then on); its load
  was kept through a restart (`die ... stored`, `die read back` at Dbg); with
  `noticecfg loaded=` raised the other player noticed it (`notice-load:`).
- **The swap**: puppet5's die carried and loaded 2 at 70% went into slot `OZG_Cn_Die1` of
  the can, and the can's die there (6 at 70%) into his pocket; the throw after it showed a 2
  (`sleight: ... dice-swap 369.0 for 378.0 ... at 30% rolled 84; seen by nobody`).
- **The client compiles** (`client_compile_check`, after the last change of 5_Mission).

## What went otherwise, and why

Defects found on the way, each fixed and checked again:

- **A save's hint** held `;`, the save's own separator: written `&` in a save.
- **High roll made chips**: what a seat put in the pot is now its own until the pot is won
  (`m_Put`).
- **A long thousand came back cut**: `string.Split` hands back no part over 8191 bytes
  (measured; dayz-modding enforce-script.md). Long fields are read in place
  (`OZG_Text.SplitSpan`, `FieldSpan`, `FieldPieces`).
- **A thousand never ended** with the first bot: the bot now writes its points down as the
  dice left and the pits say.
- **"Variable name 'set' already used as type name"** stopped the World module: a local
  named `set` (dayz-modding SKILL.md).
- **A dice turn was skippable**, so an absent player's turn was passed over and the pass was
  never recorded: pig lost the turn's points, perudo's rev 209 went on without him, and a
  restart played the game otherwise. A throw is never skipped now (the table waits, or the
  host lets the player go), and a seat let go is recorded among the moves (`<seat> ~drop`).
- **After a restart a loaded die was not thrown**: the order a holder lists its attachments
  in is not kept across a restart (dayz-modding items-and-processing.md, "Read a holder's
  attachments by slot"). The dice are read in the order of their slots.
- **After a restart a session did not know its place's dice** until a new game: it reads
  them again before a move or a swap (`DiceAgain`).
- **The first swap corrupted the dice**: the leaving die was listed in the slot and in the
  pocket, the arriving one nowhere. The destination had been the leaving die's own location,
  which names that die (dayz-modding items-and-processing.md, "A location names its item").
  A fresh location for the arriving die put it right.

Decided here, the owner's to overrule (also in the design's §14):

- **Noticing a loaded die** is a chance at each throw in which it shows its face,
  `NoticeLoaded` times the load's step, for each other seated player; it does not grow deal
  after deal as the cards' noticing does.
- **A swap shows** at `ChanceDiceSwap`, and then each other seated player sees it at
  `SpotPercent`, the card sleights' own (the design named a `NoticeOther` that no setting
  has).
- **Thousand's house rules** are each on or off, the entry 50 or none; the design's
  "numbers as the host's options" are the defaults, not options.
- **The window's "Variant"** steps through a game's first option only (whose dice, in the
  dice games); the others are set through the bridge until the mockups.

## Not done

- The mockups of the eleven games' windows and of the kit's window, then the windows to
  them: the board's sleights, the swap and loading in the window.
- Phase 6, the game in the world; phase 7, VPP's fields for the new settings, the README,
  the Workshop pages.
- The models in the world were not looked at closely (`models/README.md`, "Checked, and
  not").
