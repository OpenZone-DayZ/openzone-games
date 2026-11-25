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
| Ace | Стрілець | Дегтярьов | Шрам | Скіф |
| King | Сахаров | Сидорович | Борода | Бармен |
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

## The jokers (2026-11-25)

The owner asked for two jokers on the stalkers' deck, named for the Doctor and Faust, and
for decks of 54 and 64 cards.

- **The faces** `Xr`, the red joker, and `Xb`, the black one (`OZG_Cards.Jokers`,
  `IsJoker`). A joker is a card (`IsCard`, `FaceOf`) with no rank and no suit (`RankOf`
  -1, `SuitOf` ""). No game deals one: a game is handed only the cards of its own set
  (`OZG_Game.CardsUsed`; durak's set is from the six up), so the jokers stay in the deck.
- **The decks**: a deck class with `ozgJokers = 1` fills itself with the two jokers
  besides the fifty-two (`OZG_Decks.HasJokers`, `OZG_FillNow`). `OZG_Deck_Stalker` has it
  and 54 slots; `OZG_Deck_Stalker_Sharp` 64. The classic decks keep 52 and 62.
- **The items**: `OZG_Card_Xr` "Червоний джокер" / "Red joker", `OZG_Card_Xb` "Чорний
  джокер" / "Black joker", wearing the stalkers' look (`tools/make_card_items.py`).
- **The picture** (`tools/make_cards.py`, `joker`): JOKER down two corners, the mark of the
  series over a frame, the name under it: ДОКТОР on the red one, ФАУСТ on the black one.
  The frame holds the mark of the series, large, until `PICTURES` gives the joker a line of
  its own (`"Xr": ("photo", "ДОКТОР", "<file>")`), as a king's; then it holds that picture.
- **Their pictures**: the owner wrote the lines into `PICTURES` and drew the deck the
  same day: the Doctor's portrait on the red joker, Faust's on the black one, and on
  the king of diamonds, the Doctor's old card, Beard of Call of Prypiat. The game's files
  hold no picture of Faust; his is a file the owner gave (`src/SOURCES.md`).

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

Not ours. The twenty-two pictures show the S.T.A.L.K.E.R. games and are GSC Game World's:
eight portraits from the files of the trilogy (Enhanced Edition), and from S.T.A.L.K.E.R. 2
the twelve mutants and Skif, cut from its files, and Faust, from a file the owner gave
(2026-11-25). The files of S.T.A.L.K.E.R. 2 name no picture
after Skif: his card carries the hooded stalker of the difficulty screen, the first of its
five pictures (the owner asked for Skif on that ace the same evening, and for the Doctor on
the king of diamonds in the Forester's place; on 2026-11-25, for Sakharov on the king of
spades in Ghost's place and for the Barman on the king of clubs in Wolf's, both portraits
of Shadow of Chornobyl). `art/cards/stalker/src/SOURCES.md` names the game, the file and the cell
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
