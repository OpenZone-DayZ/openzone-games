# Marks (the card sharp's phase 2): record

2026-11-25. The owner asked for the plan of the second phase ("the plan of phase 2"), asked
what it adds, and chose to have it carried out in this session. The plan is
`docs/2026-11-25-marks-plan.md`, the design `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`
(§4.1 to §4.4, §4.6, §7, §8, §11.2). The tool's and the loupe's models stay the game's own
stand-ins, the plan's default; the owner did not ask for others.

## What was built

| Task | Commit | What |
|---|---|---|
| plan | 7688a56 | the plan, eight tasks with their stand checks |
| 1 | d61c0c3 | a card keeps its mark: the sign 1..99, the marker, the readers, the noticers; written to the world's storage after the style (`CARD_SAVE` 2), never synchronised; stand ops `mark`, `markof` (a laid-out game's deck, or `in=me`) |
| 2 | f3b8176 | the settings `Cheating` 1, `MarkUses` 20, `MarkSeconds2` 8, `LoupeSeconds` 5, `MarkKnowers` 16; the settings file at version 2; a client is told `Cheating` and the two lengths |
| 3 | 879da0e | the card sharp's pen (`OZG_MarkTool`): a recipe of the game's own crafting that makes nothing, the pen in the hands and a card the player carries; twenty uses counted as the sewing kit counts them; every marking in the server's log; stand op `craft`, `give what=` |
| 4 | 00abab1 | the loupe (`OZG_Loupe`): "the card is marked" or "nothing found" to the player alone, through the core's notice; a marked card gets the player as a reader; logged too |
| 5 | f2d84be | a deal reads every card's mark into its record (`OZG_DealCard` keeps `Sign`, `Readers`, `Noticers`); durak's state at version 3, a state of version 2 read with clean records |
| 6 | 06f68d5 | the game says which cards a seat reads where they lie back up (`ReadsFor`); the session puts them into the head of that player's own view (`OZG_WView.Reads`), only with `Cheating` 1; stand op `reads` |
| 7 | ac4f2e6 | `tools/make_marks.py` draws ninety-nine faults on each back and the pencilled faces, `ozg_marks` (512 x 512) and `OZG_MarkPlaces.c`; a card's frame gains a sign and a pencil, placed by `OZG_GameMenu.MarkBack` in pixels inside the turning frame, on the other seats' fans and on the stock's top |

## Measured on the stand

NHChernobyl, diag 1.30, a client at 1920 x 1080 and at 3840 x 1600.

- **"Tool and card, nothing made, the card changed"** (design §4.2): the recipe system takes
  it. A card dragged with the real mouse from a pocket or a backpack onto the pen in the
  hands shut the inventory and offered "Mark the card [HOLD]" ("KING OF HEARTS" above it).
- **The gesture**: held 5 s, nothing (the gesture broke off, and the offer went with it: a
  second try is a second drag); held 11 s, the card marked. From the press to the server's
  marking: 10.3 s for `MarkSeconds2` 8. The loupe: 3 s, nothing; 7.3 s from the press to
  "The card is marked" for `LoupeSeconds` 5. So the gesture takes the setting and about
  2.3 s more (the crouch first).
- **A card in a slot of a deck the player holds** (design §4.2): a deck in a pocket or a
  backpack is drawn as one picture, with no slots and no way to open them, so there is
  nothing to drag from; a deck in the hands shows its slots, but then neither the tool nor
  the card is in the hands, which the recipe needs. A card is marked out of its deck.
- **A recipe's override of `CanDo` that hands `ingredients` on to `super.CanDo` found the
  array empty after the call**: "Index out of range (0 >= 0) 'ingredients'" in the
  override, and the virtual machine's exception left its answer to chance, so a craft passed
  or failed at random (the first build: the same pen and card refused, then after a restart
  marked). The two items go into locals and the base's check is done by script
  (`OZG_Recipes.NoAttachments`); no exception since, every craft as expected. The game's own
  recipes that override `CanDo` do not call super either.
- **The uses**: a full pen made twenty marks (95, 90 .. 0) and was gone after the
  twentieth (`craft mark: hands=0`); a card marked twenty times kept its sign.
- **Refusals**: a card in a laid-out game's deck (`root=0`); `Cheating` 0, on the server
  (`cando=0`, everything else yes) and on the client (a card dropped on the pen or the loupe
  made no action, the inventory stayed open).
- **The loupe**: "Nothing found" on a clean card, "The card is marked" on one another id
  marked, and the player became its reader; the game's notification at the top of the
  screen; in Ukrainian "Карта мічена", "Роздивитися карту", the loupe's and the pen's names
  and descriptions.
- **Restarts**: a card's mark and readers came back from the world's storage (tasks 1 and
  4); a durak game saved at version 2 loaded after the update with its hands and trump; a
  session of version 3 loaded; its reads came back (`reads seat 0: 3:As/12`, the ace in the
  bot's hand).
- **Reads**: with the 52 cards marked, seat 0's reads were the three bots' hands card for
  card and the stock's top, nineteen; the bots read nothing; with one card marked, that card
  alone.
- **The marks' sheet**: 250 pictures on 512 x 512, `ozg_marks_ca.paa` 65 KB, no `GAP`
  lowered. The faults drawn four times larger and reduced came out thinner than the
  mock-up's, which Pillow drew pixel by pixel: every shape is drawn half a pixel larger all
  round (`GROW`), and the scrape and the speck then matched the mock-up's sheet.
- **The window**: every back of the bots' fans and the stock's top carried a fault and a
  pencil, turned with its card (the far seats at 180, the side seats at -90 and 90), none
  cut off; a sign's kind as its number says (the stock's top read sign 8, a scrape); at a
  glance the table is a table of plain backs; with `Cheating` 0 none was drawn. Pictures
  with exact flags, placed by `SetPos`/`SetSize` in pixels inside a turned frame, turn with
  it, as the frame's fractional children do.

## Where the build differs from the design and the plan

- **Stand-in models**: the pen (`\dz\gear\tools\Pen.p3d`, 1 x 1, 12 g) and, for the loupe,
  the pistol scope (2 x 1, 120 g); the sizes and the weights are the plan's.
- **The reads in the head of a player's view**, not in the seat's private half (§4.6, §9):
  the game makes its private half by seat and knows no players, and a read belongs to the
  player. Nobody else is sent them all the same.
- **The loupe's studies are logged** beside the markings (§6.1 asks it of the markings).
- **A read lies on the first back of its look** in a fan: a fan shows its backs by look,
  not in the hand's order.
- **The pencil at the table's size**: on a fan card at 1920 x 1080 the face is about 8 px
  tall and reads as a smudge; at 3840 x 1600 it can be made out when looked at closely. It
  is the mock-up's own size and ink; how large and how dark it should be is the owner's
  call.
- **The plan's checks**, where they ran otherwise: task 1's restart with task 2's build;
  the refusals with `Cheating` 0 of tasks 3 and 4 in one cycle; task 7's shot of two marked
  cards left out (the whole table with every card marked shows it at a glance, and one
  marked card was read by the stand); a place the stand lays has no host, so it is gathered
  (`gather`), not folded (the plan amended); the pen's action in Ukrainian was not shot
  (its name and description were, and the loupe's action).

## Seen in passing

- Right after a client connects, the first opening of the inventory drew the cards that
  came from the world's storage as blank white pictures; opened again, they wore their
  faces. A card made after connecting was drawn at once. That is the cards of phase 1, not
  changed here.

## Not done

- Noticing (phase 3): the list of noticers is kept and saved, nothing adds to it yet, and a
  noticer's sign is not drawn.
- The sleights and the dealer's choice (phase 4).
- The pen's and the loupe's own models (design §12).
