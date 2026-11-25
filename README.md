# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck out on a surface
of their choosing: a table, a counter, the floor. The mod works out where others can sit
or stand around it, and the game is played in a window while the characters sit at it. The
first game is durak, podkidnoy and perevodnoy. Class prefix `OZG_`; runs with
`OpenZone_Core` alone.

> **The stalkers' deck and table use pictures from the S.T.A.L.K.E.R. games.** They are
> created and fully owned by GSC Game World and are not covered by this mod's licence.
> OpenZone Games is a fan-made mod, not official, with no affiliation with GSC Game World.
> The original games: <https://store.steampowered.com/franchise/stalker>. Details in
> [NOTICE](NOTICE).
>
> **Сталкерська колода і сталкерський стіл містять зображення з ігор S.T.A.L.K.E.R.** Їх
> створила і повністю ними володіє GSC Game World; ліцензія моду на них не поширюється.
> OpenZone Games -- фанатський неофіційний мод, не пов'язаний із GSC Game World.
> Оригінальні ігри: <https://store.steampowered.com/franchise/stalker>. Подробиці в
> [NOTICE](NOTICE).

## Status: a game can be played (2026-11-24); nothing is published

- **A deck** is a container of its cards, fifty-two items in fifty-two slots, in two
  looks (the classic deck, and the stalkers' deck with stalkers and mutants on its picture
  cards); a card sharp's deck of either look has ten empty slots more. A game is dealt
  from the cards the deck really holds: fewer, or with further cards of a face. Gathering
  the game gives the very deck back. With it in hands a player takes "Choose placement":
  a hologram of a mat follows the aim, a panel says how many seats the spot has and of
  what kind, or why the spot is refused. A key steps through the seat modes (guessed,
  standing, sitting, on the floor). "Lay out the game", held, puts the mat with the deck
  there.
- **A card is a thing too**: one model for all, each card wearing its own face and its
  deck's back. A deck's slots show the faces, the deck shows its back (the classic one
  carries the mark of OpenZone); a deck is held in the palm, a single card by its end, and
  both lie back up on the ground.
- **Marks**: a card sharp's pen marks a card through the game's crafting, a loupe tells
  whether a card is marked; whoever marked a card, or studied it, sees its face pencilled
  on its back wherever it lies back up in a game, with the mark drawn into the back's
  pattern, and nobody else is told anything. The pen and the loupe wear the game's own
  models for now.
- **Seats** are worked out around the mat in one of three layouts: standing or sitting on
  the floor below a raised thing the mat lies on (a table, a wall's top, a crate), or
  sitting on the level of the mat itself. A player turns to the mat, is shown the free
  seats for a few seconds, stands on one and sits.
- **The game** is held by the server and shown in a window that is a card table in the
  look of the deck: the classic deck's green felt in a dark green window, or for the
  stalkers' deck a piece of a stalker's suit with an old loners' patch in frames of worn
  rusty steel. The other seats sit round it as fans of backs to be counted by eye; the
  stock lies with the trump turned under it, the discards as a heap; the hand fans out
  along an arc, the playable cards lit; beside it the log and the host's tools. Durak in
  both variants, two to six players. Others may watch.
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
`docs/specs/2026-11-24-openzone-games-cardsharp-design.md`; its second phase, marks, is
built ([docs/2026-11-25-marks-record.md](docs/2026-11-25-marks-record.md)), noticing and
sleights are not.

No key, no `mod.cpp`, no Workshop item: nothing is published without the owner's word.

## What is where

| Path | What |
|---|---|
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, the table's textures and fonts, models |
| `OpenZone_Games_Bridge/` | stand tool, server only: the `oz_games` verb, bots, the `!ozg` chat commands, `stage` (a table set out to be looked at) |
| `TableGamesProbe/`, `TableGamesProbe_Bridge/` | stand tool of the first measurements ([docs/probe.md](docs/probe.md)) |
| `tools/` | `make_cards.py` (the two decks' art), `make_marks.py` (the marks a reader sees on a back), `make_card_items.py` (the card items, their slots and names), `make_table.py` (the window's tables, frames, plates, shadows; `steel.py` is the trade screen's steel), `build_fonts.py` (the window's own fonts), `make_layouts.py` (the window), `stringtable.py`, `analyze.py` |
| `art/` | sources of the cards, of the tables (`art/table`) and the fonts (Oswald, PT Mono; SIL Open Font License; their list in `art/fonts/fonts.json`) |
| `models/` | sources of the five models and how they are built ([models/README.md](models/README.md)) |
| `docs/` | plans, records, measurements, an example for a server's economy |

The three stand tools are never published and no server should load them.

## Settings

`$profile:OpenZone\OZ_Games_Settings.json`, written with its defaults at the first start
and editable in the core's admin window: the poses (a stance or the number of a gesture,
so an animation mod's gestures can be added), the radii, the spacing of seats and of
places, how long a place may stand idle, which games are on; whether cards may be
marked at all (`Cheating`), the pen's uses and how long marking and studying take.

The decks and the cards are not in any loot table: [docs/examples](docs/examples) has
the lines for `types.xml`, with a nominal of 0 for an admin to raise.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан на 2026-11-24: у гру можна грати. Колода — вмістище своїх карт: п'ятдесят дві
карти-предмети у п'ятдесяти двох слотах (два вигляди: класична і сталкерська, зі сталкерами та мутантами на картах; шулерська колода має ще десять
порожніх слотів). Карта теж предмет: одна модель на всі, кожна з власним лицем і сорочкою
своєї колоди; у слотах колоди видно лиця, сама колода показує сорочку (на класичній — знак
OpenZone), колоду тримають у долоні, одну карту — за край, і обидві лягають на землю
сорочкою догори. Роздають з того, що в колоді справді є, а зібравши гру, гравець
отримує назад ту саму колоду. Ручка шулера мітить карту через звичайне крафтування гри,
лупа показує, чи карта мічена; хто карту позначив або роздивився, бачить її лице олівцем
на сорочці, де б вона не лежала сорочкою догори, і саму мітку в узорі сорочки; більше
нікому нічого не кажуть. Ручка й лупа поки що мають моделі самої гри. З колодою в руках
гравець обирає місце, голограма килимка показує, скільки там місць і яких, або чому місце
не годиться. Місця навколо обчислює мод: стоячи біля столу або сидячи на землі. Партію
тримає сервер, а вікно — картярський стіл у вигляді колоди: зелене сукно в темно-зеленому
вікні для класичної, шматок тканини сталкерського костюма зі старою нашивкою вільних
сталкерів у рамках потертої іржавої сталі — для сталкерської. Інші гравці сидять навколо
віялами сорочок, які рахують оком; колода лежить із козирем упоперек під нею, відбій —
купою; рука розкладена дугою, карти, якими можна ходити, підсвічені; поруч журнал та
інструменти господаря. Місце чекає на гравця, який відійшов; хто йде посеред партії,
скидає карти або лишає місце з картами іншому. Усе переживає рестарт сервера. Гравець у
партії тримає віяло карт, пози задаються в налаштуваннях.

Нічого не опубліковано: ні ключа, ні предмета в Workshop — без слова власника.

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
