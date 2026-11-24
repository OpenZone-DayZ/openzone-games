# The game window becomes the table of the mock-up: record

2026-11-24 night to 2026-11-25. The owner approved the design canvas of the two tables
(memory of the mock-up: `research_notes/.../table-mock/canvas`, published as a Design
artifact) and chose "the table into the game" over the plan of the second phase. Later,
looking at the first build: "it does not look like the mock-up -- the cards are cut off,
everything is more angular, not as pretty"; then "test more than three cards thrown onto
the field: 3, 6, 9, 12", and "let's do 24 cards".

## What was built

- **Two looks of one window**, chosen by the deck the game was laid out from
  (`OZG_WView.Style`, before the first view by the deck that lies in the place):
  `gui/layouts/ozg_win_classic.layout` and `ozg_win_stalker.layout`, the same widget names
  in both. `OZG_TableLook` (5_Mission) holds the places of each look: where the hand fans
  out, the stock, the trump, the discards, the bout, the buttons, the seats.
  - classic: the mat's own felt with round corners, a dark green window with a grey-green
    rim, cream and gold, the game's own `amorserifpro-bold28` for the title and
    `etelkatextpro` for the rest; plates and buttons with round corners (white shapes,
    tinted).
  - stalker: the cloth of a stalker's suit (olive twill, a seam, a pocket, two small
    holes, stains) with an old loners' patch sewn on the pocket, in a frame of the trade
    screen's worn steel with rust; the side panel's dark plate; Oswald capitals
    (`ozg_oswaldb42`), the log in PT Mono (`ozg_type30`), names on dog tags.
- **Round the table** sit the seats somebody holds, in the ring's order from the player's
  left shoulder: the seat after his in the ring (the ring counts clockwise seen from
  above, `OZG_Ring`) sits on his left, and in durak it defends against him. Up to five
  others in the mock-up's places; more go two down each side and the rest along the far
  side, with smaller fans. A free seat takes no place in the window: the world's marks show
  where one may sit. A watcher looks from the side of the first held seat.
- **Every seat's hand is a fan of backs**, each back in its own look, to be counted by eye
  (no number). The stock lies with the trump turned under it; the discards as a careless
  heap, a back for every two cards; the bout one row of up to three pairs, two rows of
  smaller cards from four. The hand fans out along an arc (seven degrees between cards,
  forty-four at most for the whole fan), the cards that can be played glow, the card under
  the pointer rises, the card picked up rises further.
- **Every card** is `gui/layouts/ozg_card.layout`: a square frame 1.1 diagonals of the card
  wide, turned whole, with the card's shadow, its glow (hidden until lit) and its face
  inside.
- **The cloth takes the clicks** (`OZG_Hit`, `style Empty`): the window finds the card under
  the pointer itself, turn and all (`OnCard`), the uppermost first.
- Art and fonts are built by this repository's tools: `tools/make_table.py` (the felt, the
  cloth, the steel frames after `tools/steel.py`, the tags, the round shapes, the glow and
  the shadow; ImageToPAA), `tools/build_fonts.py` (Oswald 600 and PT Mono with the
  converter of OpenZone Research; `OFL.txt` beside the fonts), `tools/make_layouts.py`.
- **A stand op `stage`** (`OpenZone_Games_Bridge`, `OZG_Stage.c`): a table set out to be
  looked at, through the game's own `SaveState`/`LoadState`. See `docs/test-commands.md`.

## Measured on the stand

NHChernobyl world (instance 84), the diag server and client of DayZ 1.30, client
1920x1080 in Ukrainian and 3840x1600 in English.

1. **A turned picture is cut to its own unturned box.** With every card an `ImageWidget`
   turned by `Widget.SetRotation(0, 0, deg)`, the trump turned under the stock stuck out
   30 units where it should have shown 47: exactly to the edge of its unturned box. The
   fanned cards lost their corners (the index "10" of the leftmost card read "0"). The
   owner saw it at once.
2. **The children of a turned frame turn with it** (as the game's radial menu turns its
   layouts' roots): the same pictures inside a square `FrameWidget` with room round them
   turn whole -- the trump shows its full width, the fans keep their corners, the shadows
   and glows turn along.
3. **`SetRotation(0, 0, deg)` turns clockwise for a positive angle, about the widget's
   middle**: the trump, placed by its middle and turned 90, lay centred across the stock.
4. **A style-less `ButtonWidget` over the table paints a light box over everything under
   it**: the first build's felt and cards were washed out to grey. `style Empty` paints
   nothing (dayz-modding already said so for a button under the mouse).
5. A `_co` texture shows as it should in an `ImageWidget`; the grey of the first build was
   finding 4, not the texture.
6. The script's own hit test works on turned cards: the third card of a fan rose under a
   real cursor and a real click played it (`Survivor: plays 6 of spades` in the log).

## Checked on the stand

- Both looks dealt and played with bots: fans, stock and trump, discards heap, bout,
  glows, the moment buttons ("Take", "Pass (done)"), the host's column, the plates' gold
  edge for the seats the game waits for, the own plate.
- The bout with 3, 6 and 9 cards in real play (a deck stuffed with sixes, sevens and eights,
  so that throwing in was always possible), 12 cards (six beaten pairs) set out by `stage`.
- A hand of 24 cards and another seat's fan of 24 backs (`stage`): the indices of all 24
  readable, the hand within the cloth, the fan within its side.
- 24 cards on the table (`stage`, beyond the rules): two rows of six pairs, squeezed over
  the trump and the discards. Durak never lays more than six attack cards (design note,
  section 8, rule 4: "no more than six; in the first bout of a game, five"), twelve cards
  with those that beat them; the window was not changed for more.

## Where the build differs from the mock-up

- The mock-up's pink demo buttons, its pockets and its panel of sleights are not in the
  window: they belong to the fourth phase of the card sharp's design, which is not built.
- The title of the classic look is in the game's `AmorSerifPro-Bold` (the mock-up's
  Playfair Display is not among the fonts the mod carries), the rest in `etelkatextpro`
  (the mock-up's Inter); the stalkers' statuses and hints are in `etelkatextpro` too.
- The discards show no number and the attack limit is not written on the table: neither
  was in the mock-up.

## Not done

- No card flies from a hand to the table: the window redraws on the server's answer.
- Marks on backs (phases two and three of the card sharp's design), sounds (design note,
  section 17), the sleights' animations and shakes (phase four).
- A watcher's window was not opened live.
