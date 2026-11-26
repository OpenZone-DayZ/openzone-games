# Boards and dice -- design

Date: 2026-11-25/26. The owner's words, in this order (chat, Russian; here in English):

1. "Now let's do dice, draughts and chess. Start with draughts and chess: classic boards
   first, then perhaps 'stalker' chess, but to begin with just boards. The same principle:
   the board itself is an object with containers, and inside them are the game's items."
2. Asked one board for both games or two: "one board". "I also plan to add nardy: a board
   of its own for them, or the same?" -- the same, as the classic wooden "3 in 1" sets:
   the 8 x 8 field on one side, the nardy field on the other, the pieces inside.
3. What goes onto the board in the first version: "plus the variants" -- chess, draughts,
   giveaway, corners, long nardy and short nardy.
4. The game shown with pieces on the board in the world: "yes, as a phase of its own".
5. "What is the difference of 'Russian' draughts from ordinary ones?" -- answered: ours
   are what the world calls Russian draughts, or draughts-64. "Call them just 'draughts'."
6. "Do not make the stalker chess yourself, wait for me. Make the dice."
7. "A separate game, dice, let us discuss here too. I want the can of dice to be an item
   of its own, for the game 'dice'" -- "and the dice in it interchangeable with the ones in
   the set with nardy."
8. Of thousand, dice poker, general, pig and high roll: "all of them but general".
9. "I want each of these games to have the choice: play with your own dice or with the
   host's." Then a third choice, "by wish, where it can be: the host may allow players to
   use their own dice if they have them; who has none rolls the host's." And "a mechanic
   of swapping dice: when it is your turn and you roll the host's dice, you may swap one
   of them for one of your own; and just the same you may swap your own dice."
10. "And yes, we add perudo."
11. For dice poker: "take the rules that were in The Witcher".
12. "In chess and draughts add sleight of hand too: swap pieces round, put a piece already
    taken off the board back somewhere on an empty square or in place of another piece,
    take a piece off the board, move a piece; and the same for draughts. But of course the
    opponent must see it at once (not as with cards), and the limit is not a game's but a
    time's: the point is that the opponent has walked off somewhere and does not see."
    Then: "and in nardy too".
13. "For loaded dice you have to choose which face to make loaded (which number comes up
    more often) and how much more often. Perhaps make a separate interface for marking
    and loading?"
14. "I think a spot for the mark on a card need not be chosen, but the marking of the
    cards you know must be seen there. And trying to mark a card or load a die that is
    already loaded or marked and unknown to you, they 'open' to you."
15. "That is, the marked cards and dice known to you in your inventory, not ALL of them."
16. (2026-11-26, before the mockups of the windows) "I want to release several different
    models of chess sets and boards (as with decks). And the pieces on one board may be
    from different sets."
17. (2026-11-26, on the mockups) "The knight pieces are absolutely awful. Think more about
    how to make the 'stalker' chess and what to replace the piece models with." "The board
    needs no green field: the board by itself, and it must take almost all (most) of our
    mod's game overlay." "The stalker chess is a board in the stalker style and pieces in
    the stalker style, the rules the same."
18. (2026-11-26, of three directions proposed) "Stalkers against mutants, but the list of
    pieces is to be approved."
19. (2026-11-26, on the list) Black: "pawns -- boars, rook -- pseudogiant, knight --
    snork, bishop -- bloodsucker, queen -- chimera, king -- controller". "The stalkers --
    yes, I like it. You may take the models from GSC's games, they allow it."
20. (2026-11-26) "The classic set: download the CC0 set of Poly Haven, but reduce the
    triangles." "The pieces must not be high-poly."
21. (2026-11-26, on the first figurines) "Give the stalkers' pawn-rookies a double-barrelled
    shotgun in their hands, Strelok an AK-74, the exoskeletons a Gauss rifle." "The
    bishop-scientists -- pistols."
22. (2026-11-26, of two finishes shown side by side) "Game colours": the figurines wear the
    games' own colours, and the base shows the side.
23. (2026-11-26, after a try with S.T.A.L.K.E.R. 2's models) "We take the models and
    textures of the original trilogy, aiming at 2,500-4,000 triangles." Earlier the same
    day: "do not mix the models and skeletons of the first and the second part"; "I do not
    want the chess set to lag"; a figure must not reach past its square.
24. (2026-11-26, on the trilogy set) Sidorovich does not sit on a crate: he holds a chicken
    leg at his chest, bringing it to his mouth; then "the leg in the fist, and darker", the
    other hand "calmly along the body"; of a calmer stance tried beside it -- "the calm pose
    is worse"; "turn Sidorovich (just about the axis, the whole model) to look ahead".
25. (2026-11-26) The exoskeleton in its helmet, the face not seen; every armed figure aims
    forward, not with the barrels up ("the weapons are OK"); "put the Gauss gun's scope on
    the exoskeleton".
26. (2026-11-26) The guide does not throw the bolt but holds it -- thrown, the figure came
    out too small; "the bolt is too big, make it smaller after all, and it does not sit
    straight in the fist"; "better remove the green patch under his feet"; "turn the guide
    too", "and the chimera" (to look ahead).
27. (2026-11-26) "Now line up the heights."
28. (2026-11-26, of the set at ~4,000, a third less and half, shown side by side) "We take
    the triangles halved; check them for unseen ones there too" -- the poses are frozen,
    "remove the triangles that are not needed".
29. (2026-11-26, on the window mockup of the stalker set) "Make the figures fit in the
    square"; of four ways to show them, "at the table": your own figures stand with their
    backs to you, the other side's face you -- "for black it is the other way round". "We
    also need the classic chess pieces": the classic set stays beside the stalker one. One
    material a figure: "yes, let's do it".
30. (2026-11-26) "Now the stalker draughts: rookies against bandits -- the white are
    stalkers, the black bandits. Just stick on the piece the face of a rookie or a bandit,
    and when it turns into a king, a gang boss or an experienced stalker. Look at the faces
    of the original trilogy's portraits (the portraits first, not the textures)."
31. (2026-11-26) "The ordinary draughts are needed too": the plain discs stay beside the
    stalker ones.

What the words leave open is decided below and named in §14.

## 0. What is built, in short

- **The "3 in 1" board**: one item holding 32 chess pieces, 15 white and 15 black discs
  and two dice, each its own item in a slot of its own. Laid out on a table it lies open:
  the 8 x 8 field up for chess, draughts, giveaway and corners, turned over for nardy.
- **The can of dice**: one item holding five dice, laid out like a deck, for five dice
  games: thousand, dice poker, pig, high roll and perudo.
- **One die** for everything: a die from a can goes into a board's slot and back.
- **Whose dice** (§8): in every dice game the host chooses own dice, the host's, or by wish.
- **Swapping a die** (§9) is a sleight like the card sleights, with **loaded dice** (§10)
  made by the card sharp's kit, as its description has promised since 2026-11-25.
- **The game in the world** (§12): the position on the board and the dice on the table, as
  objects of each client's own, from a line the server keeps.
- **Not here**: the stalker chess -- the owner's to design.

## 1. Words

| word | meaning |
|---|---|
| set | what a place is laid out from: a deck, a board, a can of dice |
| board | the folding "3 in 1" board, and as a set the board with what it holds |
| piece | an item a board game is played with: a chess piece, a disc, a die |
| disc | the round man of draughts, giveaway, corners and nardy (UA «шашка») |
| can | the can of dice (UA «банка з кістками») |
| own dice / the host's dice / by wish | §8 |
| loaded die | a die the kit made heavy on one face (§10) |
| swap | the dice sleight of §9 |

The UI names (EN / UA): Chess / Шахи; Draughts / Шашки; Giveaway / Піддавки; Corners /
Кутки; Long nardy / Довгі нарди; Short nardy / Короткі нарди; Thousand / Тисяча;
Dice poker / Покер на кістках; Pig / Свиня; High roll / Хто більше; Perudo / Перудо.

## 2. What changes, and what does not

- **A place holds a set**, not only a deck. The place's hold slot (`OZG_DeckHold`) keeps
  its name, so places in worlds are untouched; a board and a can go into it too.
- **A game says what it is played with**: `SetKind()` -- `cards` (every game so far),
  `board`, `dice`. The host chooses among the games of his place's kind (and of the
  server's `GameIds`), as he chooses among the card games now.
- **Pieces are dealt as cards are**: the session reads what the set holds and hands the
  game the things it plays with (`CardsUsed` / `TakeCards`, design §7): a piece has a face
  code as a card has (`wK`, `bP`, `wD`, `Di`...). A game whose set lacks a piece refuses
  to begin and says what is missing.
- **The card games, their marks and sleights are untouched.** No card code learns about
  boards: what is shared is the place, the seats, the session, the window's frame.

## 3. Items

### 3.1 The board

A folding wooden board of the classic "3 in 1" kind (the owner's word 2).

- **Folded** (the item): a flat box, 0.32 x 0.16 x 0.06 m, two halves hinged on a long
  side, the 8 x 8 field in two halves on its outside, brass catches. The inventory shows
  it closed.
- **Unfolded** (laid out, §4): 0.32 x 0.32 m. One side is the chess field: 8 x 8 squares
  of 35 mm in light and dark wood within a border with the files a-h and ranks 1-8. The
  other side is the nardy field: two halves of 12 points each, a rim round each half, the
  bar between them.
- **64 slots inside**, one piece each, each slot for its kind, so the inside of the board
  reads like a real insert and a piece always goes back where it came from: per colour
  the king, the queen, two rooks, two bishops, two knights, eight pawns (32); 15 white
  and 15 black discs (30); two dice (2).
- **Looks**: the classic one now; another look is a config class and its art (the stalker
  set of §15 is the owner's). Several boards and chess sets will ship, as decks do (the
  owner's word 16), and the window takes its look from the board's.

### 3.2 Pieces

- **Chess**: 6 kinds x 2 colours. The classic set is Poly Haven's "Chess Set" by Riley
  Queen (CC0), white and black, each piece reduced to 260-640 triangles (word 20); the
  king 59 mm high, the pawn 33, on squares of 35. Classes per look and kind
  (`OZG_Chess_Classic_wK` ...), a face code each (`wK` ... `bP`).
- **Discs**: 30 mm across, 8 mm high, ridged edge; white and black (`wD`, `bD`).
- **The die**: 16 mm, rounded edges, bone white, black pips; one class, `OZG_Die`
  (`Di`), whatever holds it -- a board, a can, a pocket (the owner's word 7).

Every piece is an item of its own: it can be carried, lost, swapped (§9), and a game is
played with what its set really holds.

**Pieces of several sets on one board** (the owner's word 16). A piece of any look fits
the slot of its kind on a board of any look: a lost queen is made up by another set's
queen. The game itself knows only kinds and colours; the window draws every piece in the
look of the item it stands for (on the board and among the taken ones), as it draws a
card of another deck in its own art.

### 3.3 The can of dice

An item of its own (the owner's word 7): a tin can with a lid, 75 mm across and 90 mm
high, five slots for dice. Shaken in a hand it rattles;
in the window the dice come out of it onto the table.

### 3.4 The card sharp's kit and dice

The kit (`OZG_MarkTool`) loads a die (§10): one use, the same as marking a card.

## 4. Laying out

The deck's flow, unchanged (design §4): choose placement, the hologram, deploy held; the
server decides again; a place, a session, the placer its host.

- **A board** lays out as a place of its own class (`OZG_BoardPlace`: the unfolded
  board; a set names its place class in its config, `ozgPlace`). The board item goes into
  the place's hold slot, as the deck does. The place's model carries both sides; the side
  that is up follows the chosen game (nardy: the nardy field, the rest: the chess field).
- **A can** lays out as a place of its own class (`OZG_DicePlace`: the can on a felt
  square where the dice land).
- **Seats** come from the same ring (design §5). The board games are for two; the dice
  games for two to six, high roll to eight.

## 5. The board games

Each is a module of 3_Game in durak's form (design §7): rules and nothing else, seats as
numbers, chance handed in. Moves on the wire as text; every game has resigning, offering
a draw and accepting it.

### 5.1 Chess

The FIDE Laws without a clock: castling, en passant, promotion with the choice of the
piece, check, mate and stalemate; draws by agreement, threefold repetition, the
fifty-move rule and insufficient material. Moves as `e2e4`, `e7e8q`. White is the host's
by default; the next game swaps colours.

### 5.2 Draughts

Draughts-64 (what is called Russian draughts elsewhere; the owner's word 5): 8 x 8, the
dark squares, 12 a side, white first. A man moves one square forward and captures forward
and backward; capturing is compulsory, any capturing line may be chosen, and the one chosen
is played to its end; a man reaching the last rank becomes a king, during a capture too,
and goes on capturing as a king; kings move along a diagonal any distance and land on any
free square beyond what they take (on one from which the capture goes on, when there is
one); pieces taken come off when the move ends (the Turkish strike rule: none jumped
twice). Draws: by agreement, threefold repetition, fifteen moves of kings alone with no
capture. Moves as `c3-d4`, captures as `c3:e5:c7`.

### 5.3 Giveaway

The same board, pieces and moves; the one left with no pieces or no move wins.

### 5.4 Corners

Each side's discs in its corner: 9 (3 x 3) by default, 12 (3 x 4) as the host's option.
A move is one step to an adjacent free square along a rank or file, or a chain of jumps
over adjacent discs of either colour onto free squares. The first to fill the other's
corner wins; if black fills it on the move right after white, it is a draw. A disc that
has not left its own corner by the 40th move loses the game for its side.

### 5.5 Long nardy

Fifteen discs a side from the head, both sides moving the same way round, no hitting: a
point with one disc of the other side is closed. One disc off the head a turn, two on the
first throw of 6-6, 4-4 or 3-3. Both dice used where they can be, the higher where only
one can; doubles played four times. No block of six points in a row unless a disc of the
other side stands ahead of it. Bearing off from home; a game won before the loser has
borne off any is won double (mars).

### 5.6 Short nardy

Backgammon without the doubling cube: the classic setup, hitting, the bar and entering,
bearing off; single, gammon (mars) and backgammon (koks) scored 1, 2, 3.

### 5.7 Nardy dice

The two dice of the board by default. Whose dice (§8) and swapping (§9) apply as in the
dice games: a choice of the host for nardy as for them.

## 6. The dice games

Five dice from a can, or from the players' own cans (§8). Every game logs its throws.

### 6.1 Thousand

Five dice; a throw scores 1 as 10, 5 as 5, three of a kind as its face x 10 (three ones
100), four of a kind double three, five of a kind ten times three (five ones: 1000, the
game), 1-2-3-4-5 as 125 and 2-3-4-5-6 as 250. Scoring dice are put aside and the rest
thrown again, or the turn's points written down; a throw that scores nothing loses the
turn's points (a bolt); five dice put aside are all thrown again. The first to 1000 wins.
The host's options, from the house rules (all on by default): entry (the first points
need 50 in one turn), pits (a total may not stay in 200-300 or 600-700: a turn ending
there is lost), the barrel (from 880 a player's turn counts only when it reaches 1000;
after three turns on the barrel that do not, he falls back 120), overtaking (the one
overtaken loses 50), three bolts in a row lose 100. Each house rule is the host's to switch
on or off, the entry 50 or none; the numbers are fixed (built so on 2026-11-26, §14).

### 6.2 Dice poker

The Witcher's dice poker (the owner's word 11), for two to six. A game is played for a
pot of chips (the stakes of the stakes-and-games design) and is won by the first to win
two rounds. A round: each puts in his bet; each throws his five dice; in turn each may
raise, and the others call or fold; each picks any of his dice and throws them once
more; the best hand wins the round. Hands from the top: five of a kind, four of a kind,
full house, the six-high straight (2-3-4-5-6), the five-high straight (1-2-3-4-5),
three of a kind, two pairs, a pair, nothing. The same hand is decided by its dice (the
higher pair, then the higher second pair, the higher three in a full house), then a
draw: the round is played again.

### 6.3 Pig

One die: thrown until the player stops or throws a 1, which loses the turn's points. The
first to 100 wins.

### 6.4 High roll

Each throws two dice (one to three as the host's option); the highest total takes the
pot; a tie is thrown again by those tied. For chips.

### 6.5 Perudo

Each player's own five dice under his can (§8: own dice only), seen by him alone. In
turn each bids on the dice of the whole table ("four fives"), ones being any face; a bid
must raise the last (more dice, or as many of a higher face); bidding on ones halves the
count. Instead of raising, a player says "I do not believe"; all show; whoever was wrong
loses a die (the bidder if the bid was short, the doubter if not). A player down to one
die opens a round in which ones are not wild (palifico). The last with dice wins.

## 7. The window

The frame of the table window stays; a game of another set kind draws another table.

- **The board takes most of the window** (the owner's word 17): the board itself, as large
  as the window's height allows, with nothing round it -- no table, felt or field; the
  players, the moves, the log and the buttons in narrow columns at its sides.
- **The board**: the 8 x 8 field the way its side has it (black sees it turned), a click
  on a piece lights its moves, a click on a lit square plays the move; the last move
  marked, a check marked; the pieces taken beside the board, the moves listed; promotion
  asks the piece; resign and draw buttons. Draughts, giveaway and corners on the same
  field with discs.
- **Nardy**: the two halves of points, the bar, the borne-off discs, the dice of the
  throw; a click on a disc lights where it can go with the dice left.
- **Dice**: each seat's dice of the last throw, the dice put aside, the scores or the
  bids, the buttons the game allows (throw, keep, stop, bid, "I do not believe"); in
  perudo every seat's dice hidden but the player's own.

## 8. Whose dice

A choice of the host in every dice game (and nardy, §5.7):

- **Own dice**: each throws the dice of a can he carries; without a can holding enough
  dice he is not dealt in. Perudo is played this way only.
- **The host's dice**: everyone throws the dice of the can laid out (of the board, in
  nardy).
- **By wish**: who carries a can with enough dice throws his own; the rest throw the
  host's.

The server throws: it reads the dice the throw is made with, the actual items, and a
loaded one weighs on its face (§10).

## 9. Sleights

### 9.1 On the board

Chess, draughts, giveaway, corners and nardy (the owner's word 12). At any time in a game,
a player at the board may:

- **move** a piece to an empty square (a disc to a point, in nardy);
- **swap** two pieces;
- **put back** a piece the game took off -- onto an empty square, or in place of a piece,
  which is taken off in its turn; in nardy a disc from the bar or from those borne off;
- **take off** a piece, which joins the taken ones (in nardy: is borne off).

**Seen at once, by whoever watches.** Unlike a card sleight, nothing is rolled: everyone
whose window is open on the place -- the other player and the onlookers -- is told at that
moment who did what, and the squares are lit. Whoever does not watch (his window shut,
walked off) is told nothing: the pieces simply stand where they stand now, the move list
does not show it, and only the server's log keeps it. That is the whole trick: to do it
while the other has gone.

**Limited by time, not by game**: one sleight a player every `BoardSleightMinutes`
(5 by default).

**What may not be done**: a king taken off or put back twice, a pawn put on the first or
last rank, a position left with the side not to move in check (chess); a disc on a light
square (draughts); more discs of a colour than the game has. A moved king or rook loses
its castling; a man put on the far rank is crowned.

### 9.2 Swapping a die

The card sharp's sleights (cardsharp design §6), for dice: when it is his throw, a
player may swap one of the dice he is about to throw -- the host's or his own -- for a
die he carries. The items move: his die goes in, the other into his pocket. One swap a
throw, `DiceSwaps` a game; it may show (`ChanceDiceSwap`, %), and then each other seated
player sees it at `SpotPercent`, the card sleights' own; the server's log keeps every swap.
Without loaded dice a swap changes nothing, which is the point of §10.

## 10. Loaded dice, and the kit's window

**The kit's window** (the owner's words 13 to 15). The kit in the hands has an action of
its own, "Open the kit": a window of the cards and dice the player carries -- his own
inventory, not every marked card there is.

- **What he knows shows**: on a card he reads, its mark (the face, as a reader sees it on
  the back); on a die he knows, its load (the face and how heavy).
- **Marking a card**: a card picked in the window is marked as now (its sign drawn by
  chance; no spot to choose).
- **Loading a die**: a die picked in the window gets the loaded face, picked among its
  six, and how heavy: *a little*, *well*, *heavily* -- the face comes up 30, 50 or 70% of
  throws (`LoadWeights`) instead of one in six. The heavier, the sooner it is noticed
  (below).
- **What is already done opens**: a card already marked, or a die already loaded, by
  somebody else and not known to him, picked in the window, opens to him: he reads that
  mark from then on, or knows that load; nothing is spent. A card keeps its mark as it was
  (cardsharp design §4.1); a die may then be loaded anew.
- Marking and loading are done by the crafting gesture after the choice, one use of the
  kit each, as marking is now.

What a loaded die is:

- **It shows its face** as often as it was loaded for; the server throws it so.
- **Studying a die closely** (the action cards have) tells whether it is loaded and to
  what.
- **Noticing**: after a throw in which a loaded die showed its face, each other seated
  player notices it with `NoticeLoaded` (%) times the load's weight step (1, 2, 3), at
  every such throw (it does not grow deal after deal as the cards' noticing does: built so
  on 2026-11-26, §14); a noticed die is marked for him in the window.

## 11. Settings

Settings version 9: the eleven games join `GameIds`; `BoardSleightMinutes`,
`LoadWeights`, `NoticeLoaded`, `DiceSwaps`, `ChanceDiceSwap`; each of them in VPP's
module "Games".

## 12. The game in the world

A phase of its own (the owner's word 4). The real pieces stay in their slots; what the
world shows are objects of each client's own (as the seat marks are), set out from a line
the place keeps synchronised: the position on the board (64 squares, or the nardy points),
the dice of the last throw on the felt. A client within the watch radius draws them; a
move or a throw changes the line, and every such client sets the pieces again.

## 13. Phases

Each built and checked on the stand by itself, in English and Ukrainian.

1. **The board and chess**: the set on a place; the board, its 64 slots, the chess pieces
   and their models; laying out with the side up; the board window; chess.
2. **Draughts, giveaway, corners**: discs, their model and the three modules.
3. **Dice and nardy**: the die, the can and their models; long and short nardy; whose dice
   in nardy.
4. **The dice games**: the dice place and window; thousand, dice poker, pig, high roll,
   perudo; whose dice.
5. **Cheating**: the sleights on the board (§9.1); loading dice with the kit, the swap,
   studying a die, noticing.
6. **The game in the world**.
7. **VPP, README, Workshop pages**.

## 14. Decided here, the owner's to overrule

- The board games ship as one board (the owner's word 2) holding both sets and two dice.
- 15 discs a side (nardy's need; draughts take 12 of them, corners 9 or 12).
- Draughts-64 named "Draughts" / "Шашки"; giveaway on draughts-64 rules.
- Corners: 3 x 3 by default, steps and jumps along ranks and files, the 40th-move rule.
- No chess clock; no doubling cube in short nardy.
- Thousand's house rules as options, all on; dice poker and high roll for chips.
- Dice poker for more than two (The Witcher's is for two): every round the best hand of
  all, the pot to the first with two rounds.
- The can 75 x 90 mm with a lid, five dice; perudo on own dice only.
- The kit's window: three weights of a load, 30, 50 and 70%, the heavier noticed sooner;
  one use of the kit whatever is chosen; opening what somebody else did spends nothing.
- Whose dice and swaps in nardy too (§5.7).

Decided while building (2026-11-26, `2026-11-26-boards-and-dice-record.md`):

- A dice game's throw is never passed over for an absent player: the table waits, or the
  host lets him go, and a seat let go is kept among the moves.
- Noticing a loaded die: `NoticeLoaded` times its step at each throw it shows its face,
  not grown deal after deal (§10).
- A swap seen at `SpotPercent` (§9.2).
- Thousand's house rules on or off, their numbers fixed (§6.1).
- The window's look follows the board's (or the can's) look, as the card table's follows
  the deck's (§3.1).
- Until the mockups the window's "Variant" changes a game's first option only, and the
  sleights on a board, the swap and the kit's window have no buttons: their server half
  and the stand's ops are there.
- The classic set (word 20): one piece of each kind of the Poly Haven set, the white
  ones, reduced by Blender's own decimation, with the set's own pictures at 512 px and
  its white pieces' normal map; the knight looks towards +Z, and the window draws it from
  its left, the muzzle to the left as chess diagrams do (`models/README.md`).

## 15. Not in this design

- **The stalker chess**: the owner's (word 6). It is a look, not a game (word 17): a
  board and pieces in the stalker style, the rules of chess. Chosen (word 18): stalkers
  against mutants, figurines, the models taken from GSC's own games (word 19, on GSC's
  terms for DayZ: the mod free, marked unofficial, links to GSC's store, NOTICE says the
  assets are GSC's). The list, approved (word 19):

  | piece | stalkers (white) | mutants (black) |
  |---|---|---|
  | king | Sidorovich, eating a chicken leg (word 24) | controller |
  | queen | Strelok | chimera |
  | rook | a stalker in an exoskeleton | pseudogiant |
  | bishop | a scientist in an Ecologist suit | bloodsucker |
  | knight | a guide, a bolt in his fist (word 26) | snork |
  | pawn | a rookie ("vidmychka") in a hooded jacket | boar |

  The stalkers hold their weapons (word 21): the rookies a BM-16 double-barrel, Strelok an
  AK-74, the exoskeletons a Gauss rifle with its scope (word 25), the scientists a pistol.
  The figurines keep the games' colours, the stalkers on cream bases and the mutants on
  black ones (word 22).

  The set as rendered on 2026-11-26 (word 23: the trilogy's models and textures only; the
  scenes are the research tools `ogf_make_full.py`, `ogf_scene.py`):
  - **Poses** from the games' own motions. Sidorovich (Clear Sky's
    `stalker_trader_1`) has a skeleton of his own -- no `spine2`, a shorter spine, the
    pelvis 0.83 m high against a stalker's 0.99 -- so a stalker's motion is carried over
    as turns only, each bone keeping its own length (a motion places bones at the lengths
    of the skeleton it was made for: put on as it is, his legs reached to his neck); his
    fingers come from his own motions (a fist round the leg, a hand at rest). The guide's
    bolt sits in a fist taken from the knife's grip.
  - **Facing**: an armed figure faces where its muzzle points, Sidorovich, the guide and
    the chimera where their heads look -- the whole model turned about the upright.
  - **The square is the limit**: every part of a figure stays within its square (the
    square, not the circle in it); a figure that would reach past it is made smaller.
  - **Heights** (word 27): kings 56 mm, bishops 45.5, knights 45, pawns 40. A queen and a
    rook are as big as their square holds: a rifle and the body behind it hold Strelok at
    46.4 mm and the exoskeleton at 44.6; a body's length holds the chimera at 29.9 and the
    pseudogiant at 32.7; the boar is 20.6 high (40 asked, held by its length).
  - **Triangles** (word 28): every budget halved -- the body, the weapon or held thing,
    the base (156) -- and then the faces no eye above the board sees are cut: none of
    288 directions over the board (5-88 degrees up, every 10 degrees round) reaches the
    sky from any of seven points of the face. The base keeps its bottom (it shows when a
    piece is turned over in the inventory). Each figure, all in: Sidorovich 1,827, Strelok
    1,856, the exoskeleton 1,907, the scientist 1,662, the guide 1,539, the rookie 1,492;
    the controller 1,896, the chimera 1,926, the pseudogiant 1,929, the bloodsucker 1,629,
    the snork 1,644, the boar 1,377 -- 45-50% of the approved ~4,000 (that variant is
    kept). The whole board 51,077 (halved alone 53,636; at ~4,000, 107,392). Before and
    after the cut, 72 views of 700x900 differ in 214 pixels, shadows of the cut faces in
    the renderer (the game draws shadows from a shadow level of its own). The levels of
    detail below this one are made when the set goes into the game.
- A chess clock, the doubling cube, general (word 8).
- Bots stronger than the first legal move a game offers.
