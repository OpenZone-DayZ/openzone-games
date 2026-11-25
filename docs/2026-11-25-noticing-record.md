# Noticing (the card sharp's phase 3): record

2026-11-25. The plan `docs/2026-11-25-noticing-plan.md` was written and checked on paper
by another session the same day; the owner asked for it to be carried out to the end, and
it was, task by task, each with its stand checks. The design is
`E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (§5, §4.1, §4.5,
§4.6, §7, §8, §9, §10, §11.3).

## What was built

| Task | Commit | What |
|---|---|---|
| plan | 72fc630 | the plan, seven tasks with their stand checks |
| 1 | a54bc75 | the settings `NoticeGrace` 5, `NoticeStep` 4, `NoticeMax` 40, the file at version 4 (a file of version 3 gets the defaults), their bounds, `OZG_Settings.NoticeChance`; no client is told them; stand op `noticecfg` |
| 2 | c44297b | a card item takes a noticer (`OZG_AddNoticer`: capped by `MarkKnowers`, the oldest giving way, a reader refused), a deal's record answers `NoticedBy`, `OZG_DeckRead.CardNamed` finds a deal's card again in the place; stand op `noticer`, `markof` tells the noticers and takes `only=notices` |
| 3 | 4a0b9f9 | after each deal and before anybody is sent it, the session counts and rolls for every player dealt in (`NoticeAfterDeal`) and on a success makes him a noticer of one card on the item and on the game's record (`OZG_Game.AddNoticer`); the count lives in the session's memory; a line in the server's log; stand ops `whole`, `redeal` |
| 4 | 1a0bf67 | the game says what a player notices where it lies for him (`NoticesFor`: backs where a read would lie, faces in his hand, on the table and the trump); the session puts it into the head of that player's own view (`OZG_WView.Notices` of `OZG_WNotice`), only with `Cheating` 1; `OZG_Session.HeadFor`; stand op `notices` |
| 5 | 408c820 | `tools/make_marks.py` draws the speck (one picture, `speck` in `ozg_marks`) and its ninety-nine places a style on a line 6 px in from a card's edge, clear of every face's ink; `OZG_MarkPlaces.SpeckBox`/`SpeckImage`; `art/marks/<style>_specks.png` |
| 6 | aeb0c2c | the window draws a noticed back's sign (`SignBack`, phase 2's art without the pencil) on the other seats' fans and the stock's top, and a noticed face's speck (`SpeckFace`) in the hand, on the table and on the trump, in the card's turning frame (`LayMark`) |
| 7 | (this commit) | this record, the commands to try it, the README |

## Measured on the stand

NHChernobyl, diag 1.30 (with SFP 2952842170 and 2623995836 loaded), a client at 1920 x 1080.
Every server boot's verdict held only SFP's own two `Credits.json` warnings.

- **The settings file** (task 1): `migrated games settings v3 -> v4` in the log; the file
  at `"Version": 4` with `NoticeGrace` 5, `NoticeStep` 4, `NoticeMax` 40 and every value it
  held kept (the poses with SFP's squat 65008, `Cheating` 1, `MarkKnowers` 16). `noticecfg`
  answered `grace=5 step=4 max=40 putright=0`; with -1, 101 and 150, `putright=3` and the
  defaults back; 0, 100, 100 taken as they are; the file was not written by any of them.
- **A card keeps its noticers** (task 2): a marked ace took the client's id
  (`notices=1 noticers=1`), a clean queen none (`on 0 card(s)`). The cap: eighteen ids came
  to one king, sixteen were kept, the client's id and `n1` (the two oldest) gave way, `n2`
  and `n17` stayed. A card the client marked (so reads) refused him as a noticer
  (`noticers=0`). The session file kept the deal's ace record with `"Sign":12` and
  `"Noticers":["<the client's id>"]`, every other record `[]`. After a restart the three
  cards answered as before.
- **One noticing** (task 3, the grace at 0 and the chance at 100, the three bots readers of
  every card): one card after the deal, `Td sign=84`, and one log line `notice: <the
  client's id> noticed Td, sign 84, deal 1 at 100%, place <id>`, no line for a bot; the
  client was told nothing (a shot showed only the usual "You are at the game"). Four deals
  more: `redeal 4: trials=4 hits=4 expect=4 sd=0 | grace 0/0 | ramp 0/0 of 0 | top 4/4 of 4`,
  five cards noticed (Td 84, Ah 7, As 24, Th 43, 6d 16), the log's lines `deal 2` to
  `deal 5`, each `at 100%`. After a restart the same five, and `noticecfg` back at the file's
  5, 4, 40.
- **The rate** (task 3, the defaults, six players, one run of two `redeal 40`):
  `trials=240 hits=73 expect=73.2 sd=6.75 | grace 30/0 | ramp 54/13 of 10.8 | top 156/60 of 62.4`
  and `trials=240 hits=86 expect=96 sd=7.59 | grace 0/0 | ramp 0/0 of 0 | top 240/86 of 96`.
  Summed: grace 30/0; the ramp 13 hits of 10.8 expected, inside 2.3 to 19.3; the top 146 of
  396 trials, 0.369, inside 0.326 to 0.474 (nobody noticed all thirty-six, so 6 x 66
  trials); all 159 hits of 169.2 expected, inside 138.7 to 199.7 (the spread 10.16). The
  log's 159 lines: every count 6 or more, every chance `min(40, (count - 5) x 4)` (4 % once,
  16 % once, 20 % twice, 24 % once, 28 % three times, 32 % three times, 36 % twice, 40 % 146
  times).
- **Two aces, each its own card** (task 4): the deck's ace took sign 12 and is the deal's
  `As`, the ace laid beside it sign 34 and `As.1`. The first deal noticed `As` (log: `As,
  sign 12`), which lay twelfth of thirteen in the stock: `notices ... deals=1 reads=0
  head=1239B: none`. The next deal noticed the other (log: `As, sign 34`), and then nothing
  was left to count (`redeal 3: trials=1 hits=1`); the head said `deals=2 ... head=1353B:
  b5:0/12 fAs.1/34`, the deck's ace in seat 5's hand and `As.1` in the client's own: each
  ace with the sign of its own item.
- **Every card, one player** (task 4): `redeal 40: trials=35 hits=35 ... top 35/35 of 35`,
  thirty-six noticed; the head `deals=36 reads=0 head=2465B` with twenty-six entries:
  eighteen backs of the bots' hands, `b-1:0/25` (the stock's top, 7d, sign 25 on the item),
  the six faces of the client's hand and `fQh/20` (the trump, sign 20 on the item). Nobody
  else: each bot's head `deals=0 reads=19 head=2098B: none`, a watcher's `mine=-1 deals=0
  reads=0 head=1301B: none`; the seat's own half `{"Held":[...],"CanPlay":[...],
  "HeldLooks":[...]}`, 120 bytes. After a restart in mid-game: `deals=0`, the same
  twenty-six entries, thirty-six noticed.
- **The speck's art** (task 5): `classic: ... 99 specks round 52 faces, 7.9 px apart`,
  `stalker: ... 99 specks round 54 faces, 6.9 px apart`, `ozg_marks: 251 pictures on 512 x
  512; OZG_MarkPlaces.c: 397 boxes`, `ozg_marks_ca.paa: 65 KB` (67 097 bytes), 19 s. 198
  speck boxes, each 12 x 12; no sign or pencil box changed and `<style>_signs.png` came out
  the same; the imageset's places moved where the speck was packed in. On the sheets no
  speck lies on the ink of any face.
- **The window** (task 6, a classic table where the client notices every card but three
  hearts, which he reads; `redeal 40: trials=32 hits=32`, thirty-three noticed, the head
  `reads=2 head=2457B`, twenty-four notices): read by the widgets themselves (a fan card at
  1920 x 1080 is about 70 px wide) and by crops of the shot. Every back of the three fans
  and the stock's top showed one sign; a pencil showed on exactly two backs, the first of
  seat 2's fan and of seat 5's (the queen and the jack of hearts the client reads); the
  sign pictures' sizes went by their kinds (seat 3's fan: shade 7 x 9, nick 2 x 3, block 7 x
  8, cut 6 x 5, block 7 x 8, nick 2 x 3 for signs 89, 10, 47, 33, 67, 5); the stock's top
  showed its scrape where its picture lay. All six cards of the hand and the trump had a
  speck laid. Seen on the shot: the king of diamonds' speck at its left edge a little below
  the middle, the eight of hearts' on its top edge near the right corner, as their signs
  place them; the other four specks of the hand lay under the next card of the fan, and the
  trump's under the stock. At a glance the table is a table of plain cards. After `auto
  n=2` (an eight of spades beaten by a ten of spades) the head named both as faces
  (`f8s/89 fTs/41`); the ten's speck showed on its bottom edge, mid-card, turned 12 degrees
  with it; the eight's lay under the ten's edge.
- **The stalkers' table** (task 6): `marked 54 of all` (the jokers too, never dealt); the
  head's notices of look 1, eighteen backs, the stock's top and five faces of the hand, and
  none for the queen of hearts in the hand nor for the trump, the jack of hearts, which the
  client reads; in the window eighteen signs on the fans and one on the stock's top, five
  specks in the hand, none on the queen or the trump, no pencil (the third read card, the
  king of hearts, lay deep in the stock). All five specks' places fell under the next card
  of the fan.
- **`Cheating` 0** (task 6): `redeal 3: trials=0 hits=0`, `all: 0 noticed:`, the head
  `deals=0 reads=0 head=1253B: none`, every sign picture of the window hidden, no `notice:`
  line in the log. `Cheating` was set back to 1 and the server restarted.

## Where the build differs from the design

- **The notices travel in the head of the player's own view**, not in the seat's private
  half (§4.6, §9), for phase 2's reason: the game makes its half by seat and knows no
  players, and a notice belongs to the player.
- **The cards that count are the deal's**: a marked card the game does not deal (a two in a
  durak deck, a joker) neither counts a deal nor can be noticed.
- **The count is not reset by a success**: after the grace a player goes on noticing, at
  `NoticeMax` percent at most, while the deal holds marked cards he neither reads nor
  notices.
- **The speck's place is its sign's**, one of ninety-nine on a line 6 px in from the edge;
  its look (a radius of 2.2 px and half a pixel more, the back's dark ink) is the plan's.
- **A back the player reads shows his read, not his notice**, and a reader who does not
  notice a card sees no speck on its face (the stalkers' table showed it).
- **Every noticing is written to the server's log** with the player's id (§6.1 asks it of
  sleights and markings); the player is told nothing.
- **The stand's bots count, roll and notice** like players.
- **A speck can lie where the table covers it.** In a fanned hand the next card covers the
  right part of every card but the last, and the stock covers all of the trump but its top
  end: two of six specks of the classic hand were in sight, none of five on the stalkers'
  hand, the trump's lay under the stock, and an attack card's under the card that beat it.
  The design says "a speck in a margin" and not which margin; whether the specks should keep
  to the parts a fan leaves uncovered is the owner's call.
- **The plan's checks, where they ran otherwise**: a lobby left on the stand with the
  client's player seated (a stalkers' deck 28 m from the test spot) was folded first, since
  the checks seat that player; the knowledge index's layer of the dependencies was rebuilt
  to take in SFP, VPP and the terrain's mods before the names were checked; the loose cards
  a folded game leaves were swept between tables; the window was checked through its widgets
  besides the crops; the shot at 3840 x 1600 was not made (the client's window is the
  profile's 1920 x 1080), and the Ukrainian client was not shot (no string changed).

## Seen in passing

- A widget inside a frame turned by `SetRotation` reports its unturned rectangle: the ten of
  spades' speck (the card turned 12 degrees) was listed at x 780 and drawn at about x 770,
  the place the turn about the card's middle gives; the king of diamonds' (turned a few
  degrees) was listed about 3 px above the dot drawn, as such a turn moves a point at the
  card's left edge. This goes to the skill's section on turned widgets; this run wrote
  nothing there.

## Not done

- The sleights and the dealer's choice (phase 4).
- The owner's word on the speck's size, ink and margin, and on the fan card's pencil, still
  open from phase 2.
