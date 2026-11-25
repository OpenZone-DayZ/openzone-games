# Sounds and flights of cards: record

2026-11-25. The owner asked for the table's sounds (design §17: "Card sounds have to be
added too: dealing, and so on.") and for the cards to fly in the window (design §11; the
table window record's "Not done" named both). The plan is
`docs/2026-11-25-sounds-and-flights-plan.md`, written beside the card sharp's phase 3 and
carried out after that phase was committed whole (from e402a31). Every anchor of the plan
was found word for word in the code as phase 3 left it; where phase 3 had put
`Widget ... =` before a `Card(...)` call, only the call's part of the line changed, as the
plan says.

## What was built

| Task | Commit | What |
|---|---|---|
| plan | 133a38a | the plan, seven tasks with their stand checks |
| 1 | 38ce256 | `tools/make_sounds.py`: ten sounds made from numbers (shaped noise and damped tones, fixed seeds), encoded by ffmpeg's `libvorbis` into `OpenZone_Games/sounds/ozg_*.ogg`; it writes `OZG_SoundIds.c` and the `CfgSoundShaders` and `CfgSoundSets` between markers in `config.cpp` (15 m, inheriting the game's `baseCharacter_*`, `DZ_Sounds_Effects` required); `art/sounds/SOURCES.md` |
| 2 | 686e946 | `OZG_Sounds` (3_Game): ids, names, sets, lengths; the number a thing carries (`PackLot`, `QuietOf`, `CountOf`, `IdsOf`); `PlayLocal` and `PlayIdLocal`, the four steps of `PlaySoundOnObject` with `SoundPlay`'s answer in the log; `OZG_SoundRun` (a lot played one sound after another); the client message `shake` (`OP_SHAKE`, `OZG_WShake`), its sound played on that client; `OZG_Place.OZG_ById`; stand op `shake` |
| 3 | d877323 | `OZG_SoundPulse`; one synchronised number on the place and on the deck (`OZG_Hear`, `OZG_HearOne`, `OZG_Quiet` a second later, `OZG_SoundNumber`, `OZG_SoundLast`), a new lot played once on every client near; stand op `sound` |
| 4 | c5880db | `OZG_Game.HearSound` and `TakeHeard`; durak's lots; `OZG_Session.PassHeard` after every change it accepts; the deck put down (`OZG_Place.EEItemAttached`) and gathered up (`OZG_Sessions.ReturnDeck`, on the deck); bench op `heard` |
| 5 | 5fb653d | `OZG_Flights.c` (`OZG_CardSpot`, `OZG_Flight`, `OZG_Flights`, `OZG_Shakes`); the layer `OZG_Air` (`tools/make_layouts.py`, both window layouts); in `OZG_GameMenu` the registry of drawn cards, the places of every kind of card, `Fly`, `HideSlot`, `ShakeRegion`, their ticks, `LandAll`; the `shake` message shakes its region |
| 6 | 26b1269 | `OZG_FlightPlan.c`: what moved between two views of a durak table, flown; `Redraw` keeps the view it showed in locals, lands every card still in the air before a newer view is drawn, and flies after drawing it |
| 7 | this commit | this record, `docs/test-commands.md`, the README, the plan's ticks |

## What the card sharp's fourth phase gets, as built

- **A sound on one client**: `static bool OZG_Sounds.PlayLocal(string name, Object where)`
  (`where` null: this client's own player; false for no such sound), `static bool
  PlayIdLocal(int id, Object where, string why)`, `static int IdOf(string name)`,
  `static string NameOf(int id)`, `static string SetOf(int id)`, `static int LengthOf(int
  id)` (ms), `static string NamesOf(array<int> ids)`; the ids `OZG_SoundIds.SHUFFLE` (1) to
  `PICKUP` (10), `LAST`. The client message `OZG_Const.OP_SHAKE` with `class OZG_WShake {
  string PlaceId; string RegionName; string SoundName; }`, handled on the client: the sound
  for that player alone, and the region shaken when his window is open on that place.
- **A card in the air**: `OZG_Flight OZG_GameMenu.Fly(string picture, string pictureLand,
  OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)`; places
  `OZG_CardSpot SpotOfFan(OZG_WView v, int seat, int index, int count)` (null: no fan of
  that seat; index -1: the fan's middle), `SpotOfHand(int index, int count)`,
  `SpotOfStock(int stockCount)`, `SpotOfTrump()`, `SpotOfDiscards(int k)`, `SpotOfBout(int
  count, int index, bool defence)`; `static OZG_CardSpot OZG_CardSpot.Make(float x, float y,
  float w, float h, float turn)` (layout units of the 1600 x 900 window, turn clockwise);
  pictures `string PictureOfFace(string card, int look)`, `string PictureOfBack(int look)`;
  keys `static string SlotOfFan(int seat, int index)`, `SlotOfHand(int index)`,
  `SlotOfBout(int index, bool defence)`, `SlotOfHeap(int k)`, `SLOT_TRUMP`; `void
  HideSlot(string key)`, `int HeapOf(int discardCount)`, `void SortHand(array<string> held,
  string trumpSuit, array<string> into)`.
- **A small shake**: `void OZG_GameMenu.ShakeRegion(string region)` with `static string
  RegionOfSeat(int seat)`, `REGION_STOCK` ("stock", the design's "deck"),
  `REGION_DISCARDS`; 0.45 s, 4 units, 17 Hz (`OZG_Shakes.SECONDS`, `SIZE`, `HZ`).

## Measured on the stand

NHChernobyl, diag 1.30, a client at 1920 x 1080 (the window 1600 x 900 at 160, 90: a layout
unit is a pixel). Every server boot of the day judged `fail` for SFP's two `Credits.json`
warnings alone (clean); every live client's `client_verdict` passed.

**The files** (task 1): the plan's table exactly, and the same digests on a second run:

| sound | ms | bytes | sha256, first 12 |
|---|---|---|---|
| shuffle | 1281 | 17444 | d5e77a9f6645 |
| deal | 1145 | 14430 | 43afe37bd5a7 |
| trump | 258 | 5730 | 3f5f5785126f |
| lay | 189 | 5244 | 1500544b39b7 |
| beat | 229 | 5639 | b10622f4204d |
| take | 489 | 7315 | beb4327a0095 |
| sweep | 477 | 7651 | e307e4a35130 |
| draw | 297 | 6071 | a5389296a35f |
| putdown | 151 | 4777 | c1a3d46b8c53 |
| pickup | 304 | 5727 | 04d7fdbd5282 |

4820 ms in all; `ffprobe` on `ozg_lay.ogg`: `stream|codec_name=vorbis|sample_rate=44100|channels=1`;
`config.cpp` kept its CRLF. The pbo holds each of the ten files once. The plan's check
counted `.ogg` eleven times, not ten: the eleventh is `config.cpp`'s own comment ("a mono
.ogg in"), which the build packs as text (the pbo has `config.cpp` and no `config.bin`).
No `SoundShader` line in the server's log, no `OZG_` line in the client's RPT.

**A sound on one client** (task 2): `shake ... sound=<name>` for the ten, two seconds
apart: ten `dbg: sound <name> on local: started=1`; `sound=nonsense` gave `dbg: sound: no
sound named 'nonsense'`; no `SoundSetName: 'OZG_` in the client's log. A sound set that
loads cleanly leaves nothing in the client's RPT: it names a set (`AUDIO : Loading SoundSet
'<set>'`) only beside an `(E)` line, as it did that day for other mods' broken sets.

**A place's number** (task 3): the answers `count=0 now=- last=-`, `count=1 now=lay
last=lay`, two seconds later `count=1 now=- last=lay` (quiet), `count=2
now=shuffle,deal,trump`, `count=3 now=beat`, `count=4 now=beat`; the server's log `place
<id> heard lay`, `... heard shuffle,deal,trump`, `... heard beat` twice. On the client:

- the number was first read when the place came into the client's world, and before its
  id: `dbg: place : first sound number, count 0, sounds -`. At that first call the place's
  synchronised id was still 0; the values the server set in the frame that made the place
  came with the next call. So at all nine places the stand laid that day with a client
  present;
- `sound lay on place <id>: started=1` 20 ms after the server put it there;
- the shuffle, the deal 1282 ms later, the trump 1148 ms after that (lengths 1281 and
  1145);
- both `beat` lots played. The stand's calls come two seconds apart (a bridge tick), so
  the first came while the deal still sounded, and the two lots sounded together, as
  designed.

**A client arriving later**: the client stopped, `sweep` put on the place (`count=5`), the
client started two seconds later. Its log held `sound draw` after `play=draw` and no
`sound sweep`, but no `first sound number` line either: the core's debug switch reached
the client (`sync received: ... debug=true`, 13:48:33.424) after the place had come into
its world, and the line was not written. That the draw brought no `first` line shows the
number had been read before. Measured again directly: the player went 400 m away (out of
the network bubble; the stand's `networkRange*` are the defaults), `sweep` was put on the
place (`count=7`), and three seconds later the player came back: `place <id>: first sound
number, count 7, sounds -`, and no sweep played. A thing coming back into a client's world
brought its number as it stood.

**The deck's number**: a new deck in the hands, `deck: count=0 now=- last=-`.

**What durak makes heard** (task 4, the bench), every row as the plan expected: the deal
`heard shuffle,deal,trump`; `0 a:6s` `heard lay`; `1 d:8s:1` `heard beat`; the seven moves
`0 a:6h; 1 d:8h:2; 2 a:6d; 1 d:Td:3; 2 a:8d; 0 a:Th; 1 d:Qh:5` `heard
lay,beat,lay,beat,lay,beat`; `1 d:Kc:4` (`stock=8 dis=10 table=- op=1 def=2`) `heard
beat,sweep,draw`; `1 a:9d` `heard lay`; `2 take` `heard -`; `0 a:9h` `heard lay`; `1 pass`
(`stock=6 ... hands=0:6,1:6,2:8`) `heard take,draw`. Perevodnoy: `heard
shuffle,deal,trump`; `0 a:6s; 1 t:6d` (`+ +`) `heard lay`; `drop seat=0` (`dropped 0 |
... over nodurak`) `heard sweep,draw`. The rules untouched: `fuzz: 300 games, 41104 moves,
longest 354, drops 196, skipped 1377, with a loser 259, without 41, 0 defects, 7171 ms`.

**A game heard** (task 4, live, the player and two bots):

- laid by the stand: `count=1 now=- last=putdown`; the client played `sound putdown` 26 ms
  after its first read of the place's number;
- the deal: `last=shuffle,deal,trump`; the client: shuffle, deal 1287 ms later, trump 1145
  ms after that;
- every attack, throw-in and beat a `lay` or a `beat`; a bout closed by a pass `sweep,draw`
  (the draw 476 ms after the sweep); a bout taken (`ask who=bot2 ... move=take`, the bout
  closed at once) `take,draw` (494 ms apart); a pass made no sound (the count stayed);
- the player stood up discarding: `last=sweep` alone. The stock still held seven cards, but
  both bots held six or more, so nothing was drawn and no `draw` is right;
- a minute of the bots alone: a lot for every change; once the stock was out, `beat,sweep`
  with no draw;
- gathered (`!ozg clear`, `gather`): `deck: count=1 now=- last=pickup` (the stand answers
  two seconds later, the lot already quiet); the server's `deck OZG_Deck_Classic heard
  pickup` at 13:59:53.039, the client's `deck: first sound number, count 1, sounds pickup`
  and `sound pickup on deck: started=1` at 13:59:53.049. The client had not read the deck's
  number in the three minutes it lay in the place: its first call came with its first
  change, and so for all eight decks gathered or folded that day (a fold returns the deck
  the same way, and is heard the same).

**Shakes** (task 5): no new one-word name is a class anywhere (`knowledge_find(kind=class)`
for `Launch`, `Tick`, `Finish`, `Count`, `Start`, `Busy`, `Offset`, `Clear`, `Fly` and task
6's `Make`: the project, CF, the core, VPP, SFP and the game). The table drawn as before:
`OZG_Table` holds 25 cards, two fans of six (110 px squares), the trump and six stock backs
(159 px), a hand of six (189 px); `OZG_Air` one node, visible and empty. The shakes:
`sound lay on local: started=1` with `shake seat2: 6 cards`; `shake stock: 7 cards`;
`shake discards: 0 cards`; `shake seat7: 0 cards`; at the stalkers' table `sound beat on
local: started=1` with `shake seat3: 6 cards`. Read 1.7 s after a shake, the fan's six
cards stood exactly on their rest rects.

**Flights** (task 6):

- the deal: `flights rev 7: dealt 18, trump 1` at 14:17:27.489 with the shuffle's sound
  (27.490); the deal's sound 1.292 s later and the trump's 1.149 s after it; `flights: all
  landed` 2.701 s after the view (planned 2.726: the shuffle's 1.281 s, the deal's run of
  1.145 s, the trump's flight of 0.30 s). `ui_tree` two seconds after the deal: 9 cards
  left in the layer, 6 in flight (squares between the stock's 161 px and the fans' 114 px)
  and 3 not yet off (`visible` false), 10 landed; four seconds later the layer alone;
- in play: `laid 1` for an attack, for the player's own beat (from his hand) and for a
  throw-in; `taken 3, drawn 2` (the player took three into his hand); `swept 4, drawn 3`;
  `swept 10, drawn 9` (a five-card bout); `swept 4, drawn 2` as the stock's last two went
  (the last from the trump's place under the stock); `swept 2` with the stock out; each
  followed by `flights: all landed` (a lone card after 0.33 s, a group after up to 1.07 s);
- a view in mid-flight (`auto n=2`): `flights rev 18: laid 1`, then `flights: 1 landed at
  once` in the same millisecond as the next view and `flights rev 19: laid 1` 4 ms after;
  `auto n=6` gave six views in 0.2 s, each landing the one before (`19 landed at once` when
  the view after a ten-card sweep and nine draws came); the layer alone two seconds later;
- the window closed in mid-deal: `flights rev 7: dealt 18, trump 1`, then `flights: 7
  landed at once` 2.1 s later as the close button's click (`ui_click`, the window's own
  handler) shut it; opened again, the layer alone and all 25 cards `visible`;
- perevodnoy: `flights rev 13: laid 1` for `t:Jd`, from seat 2's fan onto the table beside
  the attack, the defender now seat 3;
- a watcher: the window opened by the place's own action
  (`world_action(OZG_ActionWatch)`: started and ended), the first watcher's window opened
  live; the host bot's deal `dealt 18, trump 1`, two seconds on 15 cards in the layer, two
  of them landing on the bottom fan at the hand's place; the host removed seat 2
  discarding: `flights rev 8: swept 6`, its six backs to the heap (nothing drawn: the table
  was empty and both other hands held six);
- the stalkers' look: `dealt 18, trump 1`, `laid 1` three times, `swept 2, drawn 2`;
- Ukrainian (`language="russian"`): `dealt 18, trump 1`; a move two seconds into the deal
  `flights: 10 landed at once` and `laid 1`; the deal's sounds went on regardless (the
  trump's 0.45 s after the landing: they are the place's, not the window's).
  `client_verdict` passed.

**No pictures.** Through tasks 5 and 6 both of the owner's monitors were off
(`Get-PnpDevice -Class Monitor`: `Present` False), and every `client_shot` was black
(`lit_fraction` 0) while the client drew. The window was read from the widget tree and
the client's log instead; the flights and the shakes have not been looked at.

## Where the build differs from the design and the plan

- **The number carries up to three sounds**, not one ("the last sound"), so that a deal
  and a closing move are one lot played in order; it **goes quiet a second** after each
  lot, so that a client coming later plays nothing old.
- **The deck carries the gathering's sound**, not the place: the place is deleted in the
  same frame. A deck made anew for a place that held none is not heard.
- **The client half of the card sharp's `shake`** is built before its fourth phase; the
  stand sends it.
- **The deal's cards wait for the shuffle's length** and spread over the deal's run; the
  trump turns up as the run ends.
- **A card another seat holds flies as a back**, turning face up halfway when it lands
  face up.
- **The drawing order of the flights after a drop** starts from the opener, not from the
  seat after the dropped one.
- **The plan's checks, where they ran otherwise**: the `.ogg` count (above);
  `log_verdict(source="client")` after `client_compile_check` cannot say `pass` on this
  stand, since it applies the server's ready line and counters, which a client never
  prints, and counts SFP's two warnings: the job's own `client compiles` with no error was
  taken as the pass (phase 1's plan did the same), and task 4 ran one too (its 3_Game
  changed); the late client measured twice (above); task 4's taken bout asked for by the
  stand, not waited for; task 6's view in mid-flight made by `auto n=2` (two stand calls
  are two seconds apart and a card lands in 0.35 s); the first `ui_tree` after a deal
  comes two seconds later, so it found 9 of 19 cards (15 at the watcher's), not 19; at
  perevodnoy the move on was made by hand (`a:Js` by the player, `t:Jd` by the bot), since
  the bots never move an attack on (they beat, lead, pass or take); in Ukrainian the
  window's own words could not be read through the bridge (its texts came back empty, their
  `text_size` drawn); no shots (above).

## Seen in passing

- SFP4's `Virtual Machine Exception` ("String CORRUPTED - FIX OnStoreLoad()",
  `sfp_music_project/playerbase.c:54`, `OnRPC`) in every client log of the day, right at
  connecting, before this work as well: SFP's, not this mod's.

## Not done

- The sleights' own sounds: the fourth phase adds them to `tools/make_sounds.py`'s table.
- The owner's ear on each sound's loudness (its shader's `volume`), and the owner's eye on
  the flights and the shakes: no picture was taken.
- Task 7's steps 5 and 6 (the skills and the memory) are the session's that ran this plan,
  not this repository's.
