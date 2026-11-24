# The stalkers' deck: what was built (2026-11-24)

A third deck beside `classic` and `zone`. The owner asked for "a plain stalker deck with
pictures of mutants and legendary stalkers", then set its order and its roster by looking
at mock-ups: stalkers stand over mutants ("stalkers are more dangerous"), the tens carry
mutants too, and every card is worn its own way ("all cards are worn alike, randomise it").

## What there is

- **The deck**: `OZG_Deck_Stalker`, look `stalker`, and a card sharp's
  `OZG_Deck_Stalker_Sharp` with ten slots more. Names: "Колода карт (сталкерська)" /
  "Deck of cards (stalker)". On the stand: `!ozg deck stalker`, `!ozg deck stalkersharp`.
- **Twenty picture cards.** An ace carries one photograph between its two indices and the
  name under it. A king, a queen, a jack and a ten carry the picture twice, the lower one
  turned, and the name between the two; a ten has no pips, its rank is read from the
  index. A photograph fills its frame; a mutant is a figure cut out of its ground and
  stands on a panel tinted by rank (queens rose, jacks green, tens grey-blue).
- **The other thirty-two** are the pip cards of `zone`: the same stock and inks.
- **The back**: red under a pale diagonal lattice, a border, the trefoil in a disc. It
  follows the back of the playing cards GSC drew for S.T.A.L.K.E.R. 2 and is drawn by the
  script, not copied from them.

| | spades | hearts | diamonds | clubs |
|---|---|---|---|---|
| Ace | Стрілець | Дегтярьов | Шрам | Доктор |
| King | Привид | Сидорович | Лісник | Вовк |
| Queen | Химера | Полтергейст | Контролер | Псевдогігант |
| Jack | Бюрер | Снорк | Кровосос | Псевдособака |
| Ten | Тушкан | Сліпий пес | Плоть | Кабан |

The names are the Ukrainian ones of the games' own texts and are printed on the cards, so
a client in English shows them in Ukrainian too.

## Changing a card

`tools/make_cards.py`, the table `PICTURES`: a line a card, `"Kh": ("photo", "СИДОРОВИЧ",
"sydorovych")`. The third word is a file in `art/cards/stalker/src` without `.png`; the
first is `photo` for a rectangular picture and `figure` for a cut-out with a transparent
ground. Then `python -I tools/make_cards.py --style stalker` and a build. Nothing else
names a card's picture: no script, no config.

A style gets picture cards by having an entry in `PICTURES`, and uneven wear by
`uneven=True`; `classic` and `zone` have neither and are drawn as before, to the byte
(checked: a full run left their pictures and imagesets unchanged in git).

## The wear

A worn deck used to age every card by one recipe: the stains, specks and scratches
differed, the strength, the dark edges and the tone of the stock did not. An uneven deck
has every card draw a lot first, seeded by its name (`finish_uneven`): how hard it was
used (0.3 to 1.95, most near 0.8), the tone of its stock, the side the dirt sat on, up to
four stains, a fold or two, a bent corner, a print faded all over or rubbed off in a
patch. In this deck: 41 of the 53 pictures stained, 18 folded, 13 dog-eared, 19 faded, 24
rubbed. The same card always draws the same lot. The back is one picture for the whole
deck, so a face's wear tells nothing from behind.

## Whose the pictures are

Not ours. The twenty pictures are cut from the files of the S.T.A.L.K.E.R. games and are
GSC Game World's: eight portraits from the trilogy (Enhanced Edition), twelve mutants from
S.T.A.L.K.E.R. 2. `art/cards/stalker/src/SOURCES.md` names the game, the file and the cell
of each. `NOTICE` and `README.md` carry the statement GSC's terms ask for (the mod is
unofficial, the assets are GSC's, where the games are sold) and keep the pictures outside
the mod's licence. The owner decided on 2026-11-24 to take GSC's textures on those terms.
Where the statement shows inside the game, and the text for a Workshop page, are not
decided: nothing is published.

## What was checked on the stand

NHChernobyl stand, diag 1.30, 1920x1080, client in English and in Ukrainian:

- the deck is given, laid out by script, dealt from and played (a bot opposite): the back
  on the stock and on the seats' strip, the trump, the hand and the table draw the new
  pictures; the names on the cards are legible at this window's card size;
- the game folded: the very deck back in hands with its fifty-two cards
  (`cards`: `slots=52 cards=52 looks=2:52`);
- the names of the deck: "DECK OF CARDS (STALKER)" was not looked at in English (the hands
  held the fan), "КОЛОДА КАРТ (СТАЛКЕРСЬКА)" was read off the Ukrainian client;
- the deck on the mat shows its red back in the world;
- the card sharp's deck is created by `give`.

One line in the server log failed the verdict: a `NULL pointer to instance` in the game's
own `DayZPlayerInventory.OnHandleStoredJunctureData` (line 670) when the fold put the deck
into the player's hands. That line is the game's diag-only branch asking for a debug
plugin a server does not have; a retail server has no such branch. No class of the mod is
in the stack, and the deck arrived.

## What is not done

- A card has no picture of its own in the inventory and no model: all cards of every deck
  still show the deck's model (as before).
- The 3D fan and the 3D deck show the back and a blank face only (as before).
- Not looked at: the window at 3840x1600, a second real client, the Chernarus stand.
- `zone` keeps its even wear; whether it should draw lots too was not asked.
