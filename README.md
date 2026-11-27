# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck, a board or a can
of dice out on a surface of their choosing: a table, a counter, the floor. The mod works out
where others can sit or stand around it, and the game is played in a window while the
characters sit at it. The games: durak (podkidnoy and perevodnoy), and since 2026-11-25
ochko, Texas hold'em, preferans and bridge; since 2026-11-27 chess, draughts, giveaway,
corners and long and short nardy on a board, and thousand, dice poker, pig, high roll and
perudo with dice. Class prefix `OZG_`; runs with `OpenZone_Core` alone; with VPP
Admin Tools its settings are a module of the series' admin window
(`OpenZone_Games_VPP`).

> **The stalkers' deck and table and the stalker set of chess and draughts use pictures
> and models from the S.T.A.L.K.E.R. games.** They are
> created and fully owned by GSC Game World and are not covered by this mod's licence.
> OpenZone Games is a fan-made mod, not official, with no affiliation with GSC Game World.
> The original games: <https://store.steampowered.com/franchise/stalker>. Details in
> [NOTICE](NOTICE).
>
> **Сталкерська колода, сталкерський стіл і сталкерський набір шахів та шашок містять
> зображення й моделі з ігор S.T.A.L.K.E.R.** Їх
> створила і повністю ними володіє GSC Game World; ліцензія моду на них не поширюється.
> OpenZone Games -- фанатський неофіційний мод, не пов'язаний із GSC Game World.
> Оригінальні ігри: <https://store.steampowered.com/franchise/stalker>. Подробиці в
> [NOTICE](NOTICE).

## Status: a game can be played (2026-11-24); on the Workshop since 2026-11-25

The Workshop items: [OpenZone Games](https://steamcommunity.com/sharedfiles/filedetails/?id=3845773524) and, for VPP Admin Tools,
[OpenZone Games for VPP Admin Tools](https://steamcommunity.com/sharedfiles/filedetails/?id=3845883015) (since 2026-11-25 too).

- **A deck** is a container of its cards, fifty-two items in fifty-two slots, in two
  looks (the classic deck, and the stalkers' deck with stalkers and mutants on its picture
  cards, which holds two jokers besides: fifty-four cards); a card sharp's deck of either
  look has ten empty slots more. No game deals a joker. A game is dealt
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
- **Marks**: a card sharp's kit marks a card through the game's crafting, three cards to a
  kit unless the server says otherwise (`MarkUses`); a card held in the hands can be
  studied closely (an action of the card's own, a few seconds), which tells whether it is
  marked; whoever marked a card, or studied it, sees its face pencilled on its back
  wherever it lies back up in a game, with the mark drawn into the back's pattern, and
  nobody else is told anything. The kit is an open tin of handmade things, a model of the
  mod's own: Indian ink, glue, an awl, a file, sandpaper, a razor blade, lead pips for
  dice and picks bound with tape.
- **Noticing**: a player who is dealt, deal after deal, marked cards he does not read
  begins after a few deals to notice them, one card at a time and by chance, and is told
  nothing: from then on that card's back shows him its mark in the pattern, and its face a
  small dark speck near the edge. What he noticed stays on the card; the count of deals
  starts again with every restart and every game laid out anew.
- **Sleights**: a seated player may cheat, three times a deal: slip a card from his
  pockets into his hand, swap one for a card of his hand, throw one or two cards of his
  hand to the discards; the dealer, before his deal, puts a card of his pockets into the
  deck, draws one out of it, and deals himself cards he reads. A sleight may show, and then
  each other player at the table may see it: his own window shakes that part of the table
  with a sound, and nobody is told anything else; the server's log keeps every sleight with
  the player's id. In the window: the strip "In your pockets", the panel "Sleight of hand"
  under the log, the dealer's frame on the table; the cards fly. A game bears what sleights
  leave: copies, hands of any length, a short stock, and survives a restart with them.
- **Sounds and flights**: a game is heard by everybody near it, players, watchers and
  passers-by: the deck put down and gathered up, the shuffle, the deal's quick run,
  the trump turned up, a card laid and a card laid over it, the table taken or swept
  to the discards, cards drawn; a piece set down, taken or crowned, check, dice thrown, a
  game over. Every sound is the mod's own, made by a script from
  numbers. In the window the cards fly: from the stock into the hands at a deal and a
  draw, from a hand onto the table, from the table to the discards or into the taker's
  hand.
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
- **Four more games** (2026-11-25): ochko (twenty-one with a bank), Texas hold'em with no
  limit, preferans by the Sochi convention (three players) and bridge (four, Chicago
  vulnerability, each deal scored as in duplicate). The host chooses the game between
  deals; the window draws them from one table view every game shares. Their rules were
  played by script on the stand, thousands of deals each with every card and every chip
  accounted for after every move, and all four were played in the window, in English and
  in Ukrainian
  ([docs/2026-11-25-stakes-and-games-record.md](docs/2026-11-25-stakes-and-games-record.md)).
- **Stakes** (ochko and hold'em; of the dice games, dice poker and high roll): no money is
  touched. The host writes how many chips a
  player brings in, lets in those who ask to sit and gives rebuys; when the game is folded
  or the deck gathered, a paper (the game's own note) says who brought in what and who had
  what at the end. Settling up is the players' own business.
- **Boards** (2026-11-26): a "3 in 1" board is a container of its pieces as a deck is of its
  cards -- the chess set and the discs, each in a slot of its own -- in two looks: the
  classic set, and the stalkers' one, stalkers against mutants, with the trilogy's faces on
  its discs. Laid out, it plays chess (the FIDE laws, no clock), draughts, giveaway and
  corners, and on its other side, which the place turns up when nardy is chosen, long and
  short nardy. A game is dealt from the pieces the board really holds and refused with what
  is missing. The board takes most of the window; a click on a piece lights its moves; a
  draw is offered and accepted, a game resigned. Chess agrees with the published move
  counts (perft), and every board game was played by script, thousands of moves with every
  piece accounted for.
- **Dice** (2026-11-26; the tables 2026-11-27): a can of five dice lays out as a dice
  table. Thousand with its house rules, dice poker by The Witcher's rules for chips, pig,
  high roll for chips, perudo with palifico. Whose dice is the host's choice: those of the
  can laid out, each player's own can, or by wish; perudo is played on own dice only. In
  the window a can stands at the seat that throws; everybody sees it shaken and tipped and
  the dice fly out and tumble, and throws made one after another are shown one after
  another. The dice come in three colours and with the stalkers' sign, and a die keeps its
  look wherever it lies.
- **Cheating at a board and with dice**: at a board a player may, once in so many minutes
  (`BoardSleightMinutes`), move a piece, swap two, put a taken one back or take one off;
  whoever watches the board then sees it at once, and one who was away sees nothing. The
  card sharp's kit loads a die on a face at one of three weights (`LoadWeights`: the face
  comes up 30, 50 or 70 times of a hundred); a die held can be studied closely; a loaded
  die that shows its face may be noticed by the others (`NoticeLoaded`), who from then see
  a badge on it. A player may swap a die he carries for one he is about to throw
  (`DiceSwaps` a game; a die carried apart from the game -- no die of a can the game is
  thrown from is offered), in a window of its own: he sees his die hop into the can, and so
  does each player who noticed (`ChanceDiceSwap`), who keeps an eye on that die. Until it
  is thrown a die slipped in looks, to those who did not see, like the one it replaced; in
  perudo, where the dice lie under the cans, a loaded face is noticed when the dice are
  opened. The server's log keeps every one of these with the player's id.
- **The kit's window**: "Open the kit" on the kit in the hands lists the cards and dice
  the player carries, where they are (his pockets, a deck, a can): pick a card to mark it,
  a die to load it on a face and a weight; the kit works a moment and a use is spent.
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
built ([docs/2026-11-25-marks-record.md](docs/2026-11-25-marks-record.md)), its third,
noticing ([docs/2026-11-25-noticing-record.md](docs/2026-11-25-noticing-record.md)), and its
fourth, sleights ([docs/2026-11-25-sleights-record.md](docs/2026-11-25-sleights-record.md)).
The table's sounds and the cards in the air:
[docs/2026-11-25-sounds-and-flights-record.md](docs/2026-11-25-sounds-and-flights-record.md).
The four later games, the stakes and the admin pages: the design
[docs/2026-11-25-stakes-and-games-design.md](docs/2026-11-25-stakes-and-games-design.md)
and the record [docs/2026-11-25-stakes-and-games-record.md](docs/2026-11-25-stakes-and-games-record.md).
The boards and the dice: the design
[docs/2026-11-26-boards-and-dice-design.md](docs/2026-11-26-boards-and-dice-design.md) and
the records of its rules
([docs/2026-11-26-boards-and-dice-record.md](docs/2026-11-26-boards-and-dice-record.md)),
of its windows
([docs/2026-11-26-board-window-record.md](docs/2026-11-26-board-window-record.md)) and of
the dice tables
([docs/2026-11-27-dice-tables-record.md](docs/2026-11-27-dice-tables-record.md)). One
thing of that design is not built: the pieces and the dice shown in the world on the thing
laid out (they are seen in the window).

## What is where

| Path | What |
|---|---|
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, the table's textures and fonts, models, sounds |
| `OpenZone_Games_VPP/` | glue: the module "Ігри" of the OpenZone window in VPP Admin Tools (settings, card sharping, poses); needs VPP and the core's `OpenZone_VPP` |
| `ui/` | how the VPP pages are converted from the series' mockup (`ui/css2layout.json`, the converter's reports); the boards themselves live in `docs/specs/2026-11-12-openzone-vpp-mockup` beside the repositories |
| `OpenZone_Games_Bridge/` | stand tool, server only: the `oz_games` verb, bots, the `!ozg` chat commands, `stage` (a table set out to be looked at), `gfuzz` and the known hands and scores |
| `TableGamesProbe/`, `TableGamesProbe_Bridge/` | stand tool of the first measurements ([docs/probe.md](docs/probe.md)) |
| `tools/` | `make_cards.py` (the two decks' art), `make_marks.py` (the marks on the cards: a sign on a back, a reader's pencil, a speck on a face), `make_sounds.py` (the table's sounds, made from numbers), `make_card_items.py` (the card items, their slots and names), `make_table.py` (the window's tables, frames, plates, shadows; `steel.py` is the trade screen's steel), `build_fonts.py` (the window's own fonts), `make_layouts.py` (the window), `make_board_items.py` (the boards', pieces' and dice's items), `make_board_art.py` (their pictures), `make_board_win.py` and `make_dice_win.py` (the boards', the dice tables' and the kit's windows, with their textures), `stringtable.py`, `analyze.py` |
| `art/` | sources of the cards, of the tables (`art/table`) and the fonts (Oswald, PT Mono; SIL Open Font License; their list in `art/fonts/fonts.json`); how the sounds are made (`art/sounds/SOURCES.md`) |
| `models/` | sources of the models and how they are built ([models/README.md](models/README.md)) |
| `docs/` | plans, records, measurements, an example for a server's economy |
| `packaging/`, `package.ps1`, `keys/` | what a published `@OpenZone_Games` (and `@OpenZone_Games_VPP`) holds besides its pbo (below) |

The three stand tools are never published and no server should load them.

## Publishing

`mod_build` packs the mod and signs it with the series key `OpenZone`: the public
`keys/OpenZone.bikey` is committed, the private key beside it never is. `package.ps1` then
puts the rest into `@OpenZone_Games`: `mod.cpp` and `meta.cpp` from
`packaging/OpenZone_Games/` (`meta.cpp` holds the Workshop item's id, so an update goes to
the same item) and the key. `.\package.ps1 -Check` says what is missing or stale.

`@OpenZone_Games_VPP` is put together the same way from `packaging/OpenZone_Games_VPP/`.

| Workshop item | id | tags | Required items |
|---|---|---|---|
| OpenZone Games | 3845773524 | Mod, Mechanics, Props | Community Framework, OpenZone Core |
| OpenZone Games for VPP Admin Tools | 3845883015 | Mod, Mechanics | OpenZone Games, OpenZone Core, OpenZone Core VPP, VPP Admin Tools |

Their pages are kept here: the text in `packaging/<Mod>.workshop.bbcode` (Steam takes at
most 8000 bytes, and the Ukrainian half counts two bytes a letter), the main picture
`packaging/<Mod>.workshop.png` and the strip `packaging/<Mod>.shot<N>.<en|uk>.jpg`, English
and Ukrainian in pairs. The glue goes up after the games: it is built against the games of
the same commit.

## Settings

`$profile:OpenZone\OZ_Games_Settings.json`, written with its defaults at the first start
and editable in the core's admin window -- with `OpenZone_Games_VPP` as the forms of the
module "Ігри" (Налаштування, Шулерство, Пози), and always as the file itself under Файли
конфігів; a save reaches the players online at once. It holds the poses (a stance or the number of a gesture,
so an animation mod's gestures can be added), the radii, the spacing of seats and of
places, how long a place may stand idle, which games are on; whether cards may be
marked at all (`Cheating`), how many cards a kit marks and how long marking and studying take, and
how soon and how fast a player begins to notice marks he does not read (`NoticeGrace`,
`NoticeStep`, `NoticeMax`); how many sleights a deal (`SleightsPerDeal`), the chance each
kind shows (`CaughtSlip`, `CaughtReplace`, `CaughtThrow`, `CaughtDeckIn`, `CaughtDeckOut`,
`CaughtPick`), the chance a player sees one that showed (`SpotPercent`) and how many cards
a dealer may deal himself (`DealerPicks`); for the boards and the dice, how many minutes
lie between a player's sleights on a board (`BoardSleightMinutes`), how often a loaded die
shows its face at each of the three weights (`LoadWeights`), how readily a loaded face is
noticed (`NoticeLoaded`), how many dice a player may swap in a game (`DiceSwaps`) and the
chance a swap shows (`ChanceDiceSwap`) -- these five among the module's forms too, on its
page "Шулерство". `Cheating` 0 stops every sleight too. A client is told what its own
windows word: the radii, the times, the poses, the wait between sleights on a board and the
three weights of a load.

The decks, the boards, the cans and what they hold are not in any loot table:
[docs/examples](docs/examples) has the lines for `types.xml`, with a nominal of 0 for an
admin to raise.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан на 2026-11-24: у гру можна грати. Колода — вмістище своїх карт: п'ятдесят дві
карти-предмети у п'ятдесяти двох слотах (два вигляди: класична і сталкерська, зі сталкерами та мутантами на картах; у сталкерській ще два джокери, п'ятдесят чотири карти, і жодна гра їх не роздає; шулерська колода має ще десять
порожніх слотів). Карта теж предмет: одна модель на всі, кожна з власним лицем і сорочкою
своєї колоди; у слотах колоди видно лиця, сама колода показує сорочку (на класичній — знак
OpenZone), колоду тримають у долоні, одну карту — за край, і обидві лягають на землю
сорочкою догори. Роздають з того, що в колоді справді є, а зібравши гру, гравець
отримує назад ту саму колоду. Шулерський набір мітить карту через звичайне крафтування
гри, три карти на набір, якщо сервер не скаже інакше (`MarkUses`); карту в руках можна
детально роздивитися (власна дія карти, кілька секунд), і це покаже, чи вона мічена; хто
карту позначив або роздивився, бачить її лице олівцем на сорочці, де б вона не лежала
сорочкою догори, і саму мітку в узорі сорочки; більше нікому нічого не кажуть. Набір —
відкрита бляшанка з кустарним крамом, модель самого мода: туш, клей, шило, надфіль,
наждачка, лезо, свинцеві очки для кісток і відмички, обмотані ізолентою. Хто роздачу за
роздачею грає міченими картами, яких не читає, з часом починає їх помічати — по одній
карті й випадково, і йому про це нічого не кажуть: відтоді на сорочці такої карти він
бачить мітку в узорі, а на її лиці — маленьку темну цятку біля краю. Помічене лишається
на карті, а лік роздач починається знову після кожного рестарту й кожної нової
розкладки. Гравець за столом може шахраювати, тричі на роздачу: підкинути карту з кишені
собі в руку, підмінити карту руки картою з кишені, скинути одну-дві карти руки у відбій;
той, хто роздає, перед роздачею кладе в колоду карту з кишені, витягає з колоди карту в
кишеню і здає собі карти, які читає. Хитрість може попастися на очі, і тоді кожен інший
гравець за столом може її помітити: у його вікні здригається та частина столу і чути
звук, і більше нікому нічого не кажуть; журнал сервера зберігає кожну хитрість з
ідентифікатором гравця. У вікні — смуга «У кишенях», панель «Спритність рук» під журналом і
рамка того, хто роздає, на столі; карти летять. Гра витримує те, що лишають хитрощі:
копії, руки будь-якої довжини, коротку колоду, — і переживає з ними рестарт. Гру чути всім
поруч — гравцям, глядачам і перехожим: колоду кладуть на
килимок і збирають, тасують, швидко роздають, відкривають козир, карту кладуть на стіл і
б'ють іншою, стіл забирає той, хто відбивався, або його змітають у відбій, добирають
карти з колоди. Кожен звук — власний: його робить скрипт моду з чисел, без жодного
запису. У вікні карти летять: з колоди в руки під час роздачі й добору, з руки на стіл,
зі столу у відбій або в руку того, хто бере. З колодою в руках
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

З 2026-11-25 є ще чотири гри: очко (з банком), техаський холдем без ліміту, преферанс за
сочинською конвенцією (троє гравців) і бридж (четверо, уразливість «Чикаго», кожна роздача
рахується як у дуплікаті). Господар обирає гру між роздачами. Очко й холдем грають на
фішки: гроші мод не чіпає; господар вписує, на скільки гравець заходить, пускає тих, хто
проситься за стіл, і дає докупити; коли гру згорнули або колоду зібрали, з'являється
папірець — хто скільки заніс і в кого скільки було наприкінці, а розраховуються гравці
самі. Правила цих ігор зіграні скриптом на стенді, тисячі роздач кожна, і всі чотири
зіграні у вікні, англійською та українською. З VPP Admin Tools налаштування ігор — це модуль «Ігри» в
адмінському вікні серії (`OpenZone_Games_VPP`): які ігри є, стіл і місця, час, шулерство,
пози; збережене одразу доходить до гравців у грі.

З 2026-11-27 є дошки й кості. Дошка «3 в 1» — вмістище своїх фігур і шашок, як колода —
своїх карт; два набори: класичний і сталкерський, сталкери проти мутантів, з обличчями
трилогії на шашках. На ній грають у шахи (за правилами ФІДЕ, без годинника), шашки,
піддавки й кутки, а на звороті, який місце перевертає, коли обрано нарди, — у довгі й
короткі нарди. Грають тими фігурами, які на дошці справді є; дошка займає майже все вікно,
клік по фігурі підсвічує її ходи. Банка з п'ятьма кістками розкладається як стіл для
костей: тисяча, свиня, перудо, а на фішки — покер на кістках і «Хто більше». Чиї кості —
вирішує господар: банки на столі, кожен своєї банки або за бажанням; у перудо — лише
свої. У вікні банка стоїть біля місця того, хто кидає: усі бачать, як її струшують і
перехиляють і як кості вилітають та перекочуються; кидки, зроблені один за одним, так і
показуються один за одним. Кості трьох кольорів і зі сталкерським знаком.

Шахраювати можна й тут. За дошкою гравець раз на кілька хвилин (`BoardSleightMinutes`) може
переставити фігуру, поміняти дві місцями, повернути зняту або зняти одну; хто саме дивиться
на дошку, побачить це одразу, а хто відійшов — ні. Шулерський набір заряджає кістку на
грань з однією з трьох ваг (`LoadWeights`: грань випадає 30, 50 або 70 разів зі ста);
кістку в руках можна детально роздивитися; заряджену кістку, що лягла своєю гранню, інші
можуть помітити (`NoticeLoaded`) і відтоді бачать на ній позначку. Свою кістку можна
підкласти замість тієї, яку зараз кидатимеш (`DiceSwaps` на гру; свою — це ту, що лежить
окремо від гри: костей банки, з якої йде гра, вікно не пропонує), в окремому вікні: гравець
бачить, як його кістка летить у банку, і так само це бачить кожен, хто помітив
(`ChanceDiceSwap`), і далі тримає цю кістку на оці. Доки підкладену кістку не кинули, для
тих, хто не бачив, вона виглядає як та, яку замінила; у перудо, де кості лежать під
банками, заряджену грань помічають, коли кості відкривають. Вікно набору («Відкрити
набір» на наборі в руках) показує всі карти й кості, які гравець несе: карту можна
позначити, кістку — зарядити на грань і вагу. П'ять налаштувань дошок і костей є і в
модулі VPP, на сторінці «Шулерство». З того задуму не зроблено одного: фігур і костей у
світі на розкладці (їх видно у вікні).

Моди в Steam Workshop з 2026-11-25: [OpenZone Games](https://steamcommunity.com/sharedfiles/filedetails/?id=3845773524) і
[OpenZone Games для VPP Admin Tools](https://steamcommunity.com/sharedfiles/filedetails/?id=3845883015).

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
