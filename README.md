# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck out on a surface
of their choosing: a table, a counter, the floor. The mod works out where others can sit
or stand around it, and the game is played in a window while the characters sit at it. The
first game is durak, podkidnoy and perevodnoy. Class prefix `OZG_`; runs with
`OpenZone_Core` alone.

## Status: a game can be played (2026-11-24); nothing is published

- **A deck** is an item, in two looks. With it in hands a player takes "Choose placement":
  a hologram of a mat follows the aim, a panel says how many seats the spot has and of
  what kind, or why the spot is refused. A key steps through the seat modes (guessed,
  standing, on the ground). "Lay out the game", held, puts the mat with the deck there.
- **Seats** are worked out around the mat, standing at a table or on the ground. A player
  turns to the mat, is shown the free seats for a few seconds, stands on one and sits.
- **The game** is held by the server and shown in a window: the table, the other seats,
  the hand with the playable cards lit, the log, the host's tools. Durak in both variants,
  two to six players. Others may watch.
- **A seat** is kept for a player who walks off; whoever leaves in mid-game discards his
  cards or leaves the seat with them for another to play on. The host's role passes on.
- **Everything survives a restart**: the place, the seats, the hands, whose move it is.
- A player in a game holds **a fan of cards**; poses are settings.

What was seen on the stand and what was not is in
[docs/2026-11-24-phases-2-6-record.md](docs/2026-11-24-phases-2-6-record.md); how one
person tries it alone is in [docs/test-commands.md](docs/test-commands.md). The rules were
checked by script and by chance in phase 1 ([docs/measurements](docs/measurements)). The
design note lives beside the repositories of the series, in
`docs/specs/2026-11-23-openzone-games-design.md`.

No key, no `mod.cpp`, no Workshop item: nothing is published without the owner's word.

## What is where

| Path | What |
|---|---|
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, models |
| `OpenZone_Games_Bridge/` | stand tool, server only: the `oz_games` verb, bots, the `!ozg` chat commands |
| `TableGamesProbe/`, `TableGamesProbe_Bridge/` | stand tool of the first measurements ([docs/probe.md](docs/probe.md)) |
| `tools/` | `make_cards.py` (the two decks), `make_layouts.py` (the window), `stringtable.py`, `analyze.py` |
| `art/` | sources of the cards and the font of their letters (Oswald, SIL Open Font License) |
| `models/` | sources of the four models and how they are built ([models/README.md](models/README.md)) |
| `docs/` | plans, records, measurements, an example for a server's economy |

The three stand tools are never published and no server should load them.

## Settings

`$profile:OpenZone\OZ_Games_Settings.json`, written with its defaults at the first start
and editable in the core's admin window: the poses (a stance or the number of a gesture,
so an animation mod's gestures can be added), the radii, the spacing of seats and of
places, how long a place may stand idle, which games are on.

Decks are not in any loot table: [docs/examples](docs/examples) has the lines for
`types.xml`, with a nominal of 0 for an admin to raise.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан на 2026-11-24: у гру можна грати. Колода — предмет (два вигляди); з нею в руках
гравець обирає місце, голограма килимка показує, скільки там місць і яких, або чому місце
не годиться. Місця навколо обчислює мод: стоячи біля столу або сидячи на землі. Партію
тримає сервер, а вікно показує стіл, інших гравців, руку з підсвіченими картами, журнал
та інструменти господаря. Місце чекає на гравця, який відійшов; хто йде посеред партії,
скидає карти або лишає місце з картами іншому. Усе переживає рестарт сервера. Гравець у
партії тримає віяло карт, пози задаються в налаштуваннях.

Нічого не опубліковано: ні ключа, ні предмета в Workshop — без слова власника.

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
