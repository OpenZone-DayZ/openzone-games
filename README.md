# OpenZone Games

Table games for DayZ in the OpenZone family of mods. A player lays a deck out on a surface
of their choosing: a table, a counter, the floor. The mod works out where others can sit
or stand around it, and the game is played in a window while the characters sit at it. The
first game is durak, podkidnoy and perevodnoy. Class prefix `OZG_`; runs with
`OpenZone_Core` alone.

## Status: phase 1, nothing to play yet

The repository holds the plan of the first phase
([docs/2026-11-24-phase-1-plan.md](docs/2026-11-24-phase-1-plan.md)) and the stand probe
that measured which surfaces take a laid-out game and what working out the seats costs
([docs/probe.md](docs/probe.md),
[docs/measurements/2026-11-23](docs/measurements/2026-11-23)). Phase 1 builds the rules
alone: cards, what a game is, durak, played on the test stand by script. The design note
lives beside the repositories of the series, in
`docs/specs/2026-11-23-openzone-games-design.md`.

`TableGamesProbe`, `TableGamesProbe_Bridge` and `OpenZone_Games_Bridge` are stand tools.
They are never published and no server should load them.

## Українською

Настільні ігри для DayZ із серії модів OpenZone. Гравець розкладає колоду на поверхні, яку
обирає сам: стіл, прилавок, підлога. Мод обчислює, де навколо можуть сісти чи стати інші,
а гра йде у вікні, поки персонажі сидять за нею. Перша гра — дурень, підкидний і
перевідний.

Стан: фаза 1. У репозиторії план першої фази та стендовий зонд; грати ще нема в що.

## Licence

CC BY-NC-SA 4.0 with the additional permission in [NOTICE](NOTICE).
