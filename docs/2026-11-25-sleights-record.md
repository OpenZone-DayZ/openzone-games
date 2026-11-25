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

## Task 3: the game bears sleights (§6.7)

Built, in the game's contract (`OZG_Game`) and durak: `SleightTakes(face)` (durak: any card
with a rank and a suit; a deuce is simply a low card, a joker none), `SleightName(face)` (a
name the deal never used: `As.1` when `As` is dealt, `Kh.1` when `Kh` was taken out),
`SleightHolds`, `SleightIn` (a record joins a hand and the deal), `SleightOut` (a card leaves
the hand and the deal), `SleightSwap` (both at once, the new card in the old one's place),
`SleightDiscard` (hand to discards, still a card of the deal), `DealtRecords`, and the
dealer's choice `DealFirst(seat, names)`, which `Begin` honours by swapping the chosen cards
into the first places of the dealer's six after the shuffle. Durak's state is version 4: it
keeps the names sleights took out (`Gone`), never given again in the deal; a state of version
3 loads with none. After every sleight `Mend` settles what the rules would have: with cards
on the table the bout may close (`Settle`); with the table empty an opener whose hand a
sleight emptied draws up from the stock, or with the stock gone is out, and the bout opens
anew from the next seat with cards (the defender and the limit worked out again). No sleight
is heard through the place and none goes into the game's log. Stand ops: `cheat` (a
sleight's change told to the bench's game), `begin ... first=<seat>:<cards>`, `fuzz ...
sleights=<per mille>`; and for shots of the window, `ask ... after=<ms>` and `shake ...
repeat=<n> gap=<ms>`.

The stand (the bench, a seeded three-seat deal `0[8c 7s 8s Ks Ah 9d] 1[Ac Js Kh Jc Qc 9h]
2[Ad 9c Tc 8d As Ts]`, trump `Jd`):

- `in As` to seat 0 -> `As.1`, the hand 7; `in 2c` -> `2c`, 8; `out Kh` (seat 1) -> 5 cards,
  `gone=1`; `swap Jc -> Kh.1` (the gone `Kh` not reused), the count kept and `Kh.1` in `Jc`'s
  place; `discard Ts` (seat 2) -> `dis=1`; `in Xr` refused `STR_OZG_NO_CARD`; every answer
  `sound=yes`; `all` showed `gone[Kh Jc]`;
- the opener (seat 2) threw away his five cards one by one with the table empty: at the last
  one he drew six from the stock (`stock=18` -> `12`) and still opened;
- `first=1:Jd As`: seat 1 dealt `[Jd As Kh Jc Qc 9h]`, the trump now `Ac` (the card swapped
  into the deck's last place); six seats `first=5:Ad Kd`: seat 5 `[Ad Kd ...]`;
- `fuzz games=300 seed=11 sleights=60`: `49176 moves, longest 339, sleights 2644 (refused
  546), with a loser 300, 0 defects`; `seed=12 sleights=150 drop=5 away=20 perevodnoy`:
  `35347 moves, drops 206, skipped 947, sleights 4916 (refused 1136), with a loser 258,
  without 42, 0 defects` -- every step's state sound, a save and load every tenth move, no
  game that waits for nobody or for a seat with no move, every game ended;
- the rules untouched: `fuzz games=300 seed=11` and `seed=13 perevodnoy drop=5 away=50`
  answered phase 1's numbers exactly (`52915 moves, longest 411`; `41866 moves, longest 346,
  drops 181, skipped 1485, with a loser 254, without 46`);
- a save with a card gone loaded back (`gone=1`).

Seen in passing: the bench's `load swap=<a>><b>` (phase 1's tool to show a broken save
refused) fails even for `Transfer>Transfer`, a swap that changes nothing, with `JSON ERROR` in
the log, while the same text loads without a swap; it only ever showed refusals. A state of
version 3 is loaded in the restart check instead (task 6).
