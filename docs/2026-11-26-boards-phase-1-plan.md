# Boards, phase 1: the board and chess -- plan

The design: `docs/2026-11-26-boards-and-dice-design.md` (§2, §3.1, §3.2, §4, §5.1, §7, §13
phase 1). Carried out in the session that wrote it; where the run goes otherwise, the
phase's record says so.

**Goal.** A "3 in 1" board as an item holding its 64 pieces in slots of their own; laid
out on a table it is a place of its own class with the chess side up; two seats play
chess by the FIDE laws in the table window; everything survives a restart.

**Architecture.** A place holds a *set*: the deck as now, or a board. A game names the set
kind it plays with (`SetKind`); the session deals a board game from the faces of the
pieces its board holds (`TakePieces`) as it deals a card game from the deck. The chess
rules are a 3_Game module like the card games; the window draws a board table beside the
card tables. Models and sprites come from the repository's Blender scripts.

## Global constraints

- English and Ukrainian only; work in `main`; push and Workshop only on the owner's word.
- No DayZ API invented: every engine call is one the mod already makes, or read in
  `.dayz-mcp/corpus/scripts` before it is written.
- Enforce Script: no ternary, compound booleans into a local first, `string.ToLower()`
  in place, `Substring` counts bytes, no field named `Step`, `Debug`, `Hud`, `Slot`; a
  method named like any loaded class compiles as its constructor (prefix chess methods).
- The card games, marks and sleights keep working unchanged; every touched session path
  keeps its card branch first.
- Generated config blocks (slots, piece classes, names, types) come from a tool script with
  marker lines, as `tools/make_card_items.py` does for cards.
- Every stand run ends with its key screenshots sent to the owner, in EN and UA.

## Files

| file | what |
|---|---|
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Chess.c` | the chess rules: board, legal moves, the end, the state, perft |
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c` | `SetKind`, `TakePieces` with defaults |
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_BoardView.c` | the board games' view on the wire (`OZG_BPublic`) |
| `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Boards.c` | the board item, the board place, the pieces, reading a board |
| `OZG_Items.c`, `OZG_Sessions.c`, `OZG_Session.c` | the place class from the set; dealing a board game; the board's way back |
| `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_BoardTable.c` | drawing the board and its clicks in the table window |
| `OZG_GameMenu.c` | the board table beside the card tables |
| `tools/make_board_items.py` | slots, piece classes, names, types lines |
| `models/src/board.py`, `chess.py` | the board (folded and laid out) and the six chess pieces |
| `tools/make_board_art.py` | the piece sprites of the window, rendered from the models |
| `config.cpp`, `strings.csv`, `OZG_Settings.c` | classes, names, settings version 9 (`GameIds` + chess) |

## Tasks

1. **Chess rules** (`OZG_Chess.c`). 0x88-free 64-square board, piece letters as FEN; legal
   moves by pseudo-moves and a king-in-check test; castling rights, en passant square,
   promotion; mate, stalemate; draws: agreement, threefold repetition (position keys
   kept), fifty moves, insufficient material; resign. Moves on the wire `e2e4`, `e7e8q`,
   `resign`, `draw` (offer), `accept`, `decline`. Log words in long algebraic. Stand op
   `perft`: from the start position depth 1..4 gives 20, 400, 8902, 197281; from
   "Kiwipete" depth 1..3 gives 48, 2039, 97862. Done when those numbers come out.
2. **Pieces and the board as items** (`tools/make_board_items.py`, `config.cpp`,
   `OZG_Boards.c`). 64 slots `OZG_Bd01..64`, each for its kind; piece classes per look:
   12 chess, 2 discs, `OZG_Die`; `OZG_Board_Base` / `OZG_Board_Classic` full when made;
   names EN/UA. Done when a board given on the stand shows 64 pieces in its slots.
3. **The set on a place** (`OZG_Items.c`, `OZG_Sessions.c`, `OZG_Session.c`). A set names
   its place class (`ozgPlace`) and kind (`ozgSetKind`); the board's place
   `OZG_BoardPlace`, its hologram `OZG_BoardPlacePlacing`; a game's `SetKind`; the host's
   choice among the games of the place's kind; dealing a board game from the faces the
   board holds, refusing with what is missing; folding gives the board back. Done when a
   board laid out on the stand opens a session that offers chess and begins a game.
4. **Models** (`models/src/board.py`, `chess.py`). The folded board, the laid-out board
   (chess side), the six pieces in light and dark wood, the disc, the die; built, checked
   clean, seen in the inventory, in the hand and on the ground.
5. **The board table** (`OZG_BoardTable.c`, `OZG_GameMenu.c`, sprites). The field the
   way the seat sees it, pieces as sprites, a click lights the moves, a second plays;
   last move and check marked; promotion asks; resign and draw; the taken pieces and the
   move list. Done when a full game is played to mate in the window on the stand.
6. **Settings, bots, strings, stand ops**. Version 9 adds chess to `GameIds`; the bot
   plays a random legal move, a capture first; `oz_games` ops for the board; EN/UA names.
7. **The stand**. Lay out, two seats (a puppet), a game to mate in the window, a draw by
   agreement, a restart in mid-game, the board folded and given back whole; EN and UA;
   screenshots to the owner; the record.
