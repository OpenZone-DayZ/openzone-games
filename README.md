# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck out on a surface
of their choosing: a table, a counter, the floor. The mod works out where others can sit
or stand around it, and the game is played in a window while the characters sit at it. The
first game is durak, podkidnoy and perevodnoy. Class prefix `OZG_`; runs with
`OpenZone_Core` alone.

## Status: phase 1 done, nothing to play yet

The rules work: cards, what a game is, and durak in both variants with save and load,
played on the test stand by script and by chance
([docs/measurements](docs/measurements)). Nothing of it is in the world yet: the deck, the
place and the seats around it are phase 2, the window is phase 4. The plans are in
[docs](docs); the design note lives beside the repositories of the series, in
`docs/specs/2026-11-23-openzone-games-design.md`.

`TableGamesProbe`, `TableGamesProbe_Bridge` and `OpenZone_Games_Bridge` are stand tools.
They are never published and no server should load them.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан: фазу 1 завершено. Правила працюють: карти, контракт гри та дурень в обох варіантах
зі збереженням; на стенді їх перевірено сценарними партіями й партіями навмання. У світі
ще нічого немає, тож грати ще нема в що.

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
