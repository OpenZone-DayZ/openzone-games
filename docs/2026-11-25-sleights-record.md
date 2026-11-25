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

## Task 2: the sleights' sounds

Built: six rows more in `tools/make_sounds.py`'s table, ids 11 to 16 -- `slip` (a card out of
a pocket and into the hand), `swap` (a card slid out of the fan, the pocket, the other slid
in), `toss` (a card flicked onto the heap), `deckin` (pushed along the deck's edge and
home), `deckout` (caught by a nail and drawn out, into the pocket), `pick` (two cards set
on top, the deck squared); shaped noise and damped tones as the table's ten, at `volume`
0.30 (the table's are 0.35 to 0.45): each is played on one client alone (design §6.1).
`art/sounds/SOURCES.md` says so.

The stand:

- the table: `slip 296 ms 6431 B a77e22625e89`, `swap 375 ms 7350 B d864fa51fb7a`, `toss 199
  ms 5461 B 81f95f3df9fa`, `deckin 255 ms 6018 B 0e02b3fd4a37`, `deckout 330 ms 6777 B
  03633e518bd8`, `pick 386 ms 6913 B e27e1e0509e8`; the same digests on a second run, and
  the ten older sounds' digests the same as their record's;
- a client (English, 1920 x 1080) sent `shake ... sound=<name>` for the six, two seconds
  apart: six `dbg: sound <name> on local: started=1` in its script log; no line naming
  `OZG_` or `SoundSetName` in its script log, its RPT or its error log.
