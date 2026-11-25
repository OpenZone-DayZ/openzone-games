# Sleights (the card sharp's phase 4): record

2026-11-25. The owner's word on how: no plan document with the code in it; the work goes
task by task straight in the code, each task checked on the stand before the next and
committed after it, and this record is kept as it goes: what was built, what the stand
answered, what went otherwise than the design and why. The design is
`E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (§6, §7, §8, §9,
§10, §11.4); the window's mock-up is the design canvas the owner approved on 2026-11-24
(`research_notes/Карты и столы группировок сталкера/table-mock/canvas`, boards Main and
Stalker).

The stand throughout: NHChernobyl, diag 1.30, with SFP 2952842170 and 2623995836 loaded
(the machine profile `dayz-mcp.local.toml`). A server verdict whose only reasons are SFP's
two `Credits.json` warnings is clean.

## Task 1: the settings, version 5 of the file

Built: nine settings in `OZG_Settings` (design §7) -- `SleightsPerDeal` 3, `CaughtSlip` 30,
`CaughtReplace` 25, `CaughtThrow` 30, `CaughtDeckIn` 20, `CaughtDeckOut` 20, `CaughtPick`
25, `SpotPercent` 50, `DealerPicks` 2 -- the file at version 5, a file of version 4 given
the defaults; their bounds (no sleights below none, every chance a percent from 0 to 100,
no picks below none); `CaughtOf(kind)`; the kinds of sleight and their ops in `OZG_Const`.
Stand op `sleightcfg` (the nine set on the running server through the same bounds, the
file not written; `all=` sets the seven chances at once).

The stand:

- the log `[OpenZone] migrated games settings v4 -> v5`; the file at `"Version": 5` with the
  nine defaults, and every value it held kept (`Cheating` 1, `MarkKnowers` 16, noticing
  5/4/40, the squat's gesture 65008); the core's loader left the old file in
  `Backup\games settings.bak.json`;
- `sleightcfg`: `per=3 slip=30 swap=25 throw=30 deckin=20 deckout=20 pick=25 spot=50
  picks=2 cheating=1 putright=0`; given -1, 101, -5, 0, 100, 150, 77, 100, -2 it answered
  `per=3 slip=30 swap=25 throw=0 deckin=100 deckout=20 pick=77 spot=100 picks=2 putright=5`
  (the five out of bounds put back to their defaults, 0 and 100 kept); `all=100` and
  `all=0` set the seven; the file kept the time it was migrated at.
