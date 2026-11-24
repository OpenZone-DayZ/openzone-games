# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck out on a surface
of their choosing: a table, a counter, the floor. The mod works out where others can sit
or stand around it, and the game is played in a window while the characters sit at it. The
first game is durak, podkidnoy and perevodnoy. Class prefix `OZG_`; runs with
`OpenZone_Core` alone.

> **The stalkers' deck uses pictures from the S.T.A.L.K.E.R. games.** They are created and
> fully owned by GSC Game World and are not covered by this mod's licence. OpenZone Games is
> a fan-made mod, not official, with no affiliation with GSC Game World. The original games:
> <https://store.steampowered.com/franchise/stalker>. Details in [NOTICE](NOTICE).
>
> **Сталкерська колода містить зображення з ігор S.T.A.L.K.E.R.** Їх створила і повністю
> ними володіє GSC Game World; ліцензія моду на них не поширюється. OpenZone Games --
> фанатський неофіційний мод, не пов'язаний із GSC Game World. Оригінальні ігри:
> <https://store.steampowered.com/franchise/stalker>. Подробиці в [NOTICE](NOTICE).

## Status: a game can be played (2026-11-24); nothing is published

- **A deck** is a container of its cards, fifty-two items in fifty-two slots, in three
  looks (classic, worn, and the stalkers' deck with stalkers and mutants on its picture
  cards); a card sharp's deck of either look has ten empty slots more. A game is dealt
  from the cards the deck really holds: fewer, or with further cards of a face. Gathering
  the game gives the very deck back. With it in hands a player takes "Choose placement":
  a hologram of a mat follows the aim, a panel says how many seats the spot has and of
  what kind, or why the spot is refused. A key steps through the seat modes (guessed,
  standing, sitting, on the floor). "Lay out the game", held, puts the mat with the deck
  there.
- **Seats** are worked out around the mat in one of three layouts: standing or sitting on
  the floor below a raised thing the mat lies on (a table, a wall's top, a crate), or
  sitting on the level of the mat itself. A player turns to the mat, is shown the free
  seats for a few seconds, stands on one and sits.
- **The game** is held by the server and shown in a window: the table, the other seats,
  the hand with the playable cards lit, the log, the host's tools. Durak in both variants,
  two to six players. Others may watch.
- **A seat** is kept for a player who walks off; whoever leaves in mid-game discards his
  cards or leaves the seat with them for another to play on. The host's role passes on.
- **Everything survives a restart**: the place, the seats, the hands, whose move it is.
- A player in a game holds **a fan of cards**; poses are settings.

What was seen on the stand and what was not is in
[docs/2026-11-24-phases-2-6-record.md](docs/2026-11-24-phases-2-6-record.md) and, for
the cards as items, in
[docs/2026-11-24-cards-as-things-record.md](docs/2026-11-24-cards-as-things-record.md);
how one person tries it alone is in [docs/test-commands.md](docs/test-commands.md). The rules were
checked by script and by chance in phase 1 ([docs/measurements](docs/measurements)). The
design note lives beside the repositories of the series, in
`docs/specs/2026-11-23-openzone-games-design.md`; the cards as items are the first phase of
`docs/specs/2026-11-24-openzone-games-cardsharp-design.md` (marks, noticing and sleights
are its later phases; nothing of them is built).

No key, no `mod.cpp`, no Workshop item: nothing is published without the owner's word.

## What is where

| Path | What |
|---|---|
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, models |
| `OpenZone_Games_Bridge/` | stand tool, server only: the `oz_games` verb, bots, the `!ozg` chat commands |
| `TableGamesProbe/`, `TableGamesProbe_Bridge/` | stand tool of the first measurements ([docs/probe.md](docs/probe.md)) |
| `tools/` | `make_cards.py` (the three decks' art), `make_card_items.py` (the card items, their slots and names), `make_layouts.py` (the window), `stringtable.py`, `analyze.py` |
| `art/` | sources of the cards and the font of their letters (Oswald, SIL Open Font License) |
| `models/` | sources of the four models and how they are built ([models/README.md](models/README.md)) |
| `docs/` | plans, records, measurements, an example for a server's economy |

The three stand tools are never published and no server should load them.

## Settings

`$profile:OpenZone\OZ_Games_Settings.json`, written with its defaults at the first start
and editable in the core's admin window: the poses (a stance or the number of a gesture,
so an animation mod's gestures can be added), the radii, the spacing of seats and of
places, how long a place may stand idle, which games are on.

The decks and the cards are not in any loot table: [docs/examples](docs/examples) has
the lines for `types.xml`, with a nominal of 0 for an admin to raise.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан на 2026-11-24: у гру можна грати. Колода — вмістище своїх карт: п'ятдесят дві
карти-предмети у п'ятдесяти двох слотах (три вигляди: класична, потерта і сталкерська, зі сталкерами та мутантами на картах; шулерська колода має ще десять
порожніх слотів). Роздають з того, що в колоді справді є, а зібравши гру, гравець
отримує назад ту саму колоду. З нею в руках
гравець обирає місце, голограма килимка показує, скільки там місць і яких, або чому місце
не годиться. Місця навколо обчислює мод: стоячи біля столу або сидячи на землі. Партію
тримає сервер, а вікно показує стіл, інших гравців, руку з підсвіченими картами, журнал
та інструменти господаря. Місце чекає на гравця, який відійшов; хто йде посеред партії,
скидає карти або лишає місце з картами іншому. Усе переживає рестарт сервера. Гравець у
партії тримає віяло карт, пози задаються в налаштуваннях.

Нічого не опубліковано: ні ключа, ні предмета в Workshop — без слова власника.

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
