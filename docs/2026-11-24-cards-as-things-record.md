# OpenZone Games: cards as things (the card sharp's design, phase 1), the record

Date: 2026-11-24. The design is
`E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`; where this note says
"§" it means that file. The plan, with every step's own lines of what was seen, is
[2026-11-24-cards-as-things-plan.md](2026-11-24-cards-as-things-plan.md). Everything is on
`main`; nothing was pushed.

**Phase 1 is built and checked on the stand.** The spec's first draft made one more
measurement a condition of calling it done, "a heavy modpack's slot table"; that
condition is withdrawn ("A condition that had nothing behind it", below).

How to try it alone: [test-commands.md](test-commands.md).

## What exists now

1. **Fifty-two card items**, `OZG_Card_<face>`, one by one, written by
   `tools/make_card_items.py` together with their names in two languages and their lines
   for the economy. A card keeps the look of the deck it was made for: the style's name in
   the world's storage, its number on the wire (`OZG_CardThings.c`, `OZG_Decks`).
2. **Four decks**: `OZG_Deck_Classic` and `OZG_Deck_Zone` with 52 slots,
   `OZG_Deck_Classic_Sharp` and `OZG_Deck_Zone_Sharp` with 62. The slots are `OZG_Card01`
   to `OZG_Card62`, every card names them all. A new deck fills itself with the fifty-two
   cards of its look; a deck that was ever stored is never filled again. A deck takes
   cards only.
3. **Laying out keeps the deck.** The action spares the kit the game would delete, the
   place asks it out of the placer's hands into its own slot (`OZG_DeckHold`), the session
   waits four seconds for it and unlays a place it never reached.
4. **A place is shut**: on the server it takes and gives nothing except in the five
   seconds the session opens it for its own hand; on a client it shows no inventory.
5. **A deal is dealt from what the deck holds**, and from what lies beside it in the
   place: fewer cards, further cards of a face, a deck of aces alone. A game that cannot be
   played with so few refuses the deal with "not enough cards in the deck".
6. **A card of a deal is a record** (`OZG_DealCard`): the name that tells it apart (`As`,
   then `As.1`, `As.2`, with no end) and the look of its back. The mark joins it in phase 2.
7. **The window draws every card in the art of its own look**; a seat's strip says how
   many of its backs are strange ones (`6 (1*)`).
8. **Gathering and folding** put the cards that lay beside the deck back into it; when
   there are more cards than slots, as many as do not fit stay on the ground where the
   place lay, and chance picks them among all the cards. The deck goes into the folder's
   free hands, or lies there too.
9. **The stand's tools**: ops `cards`, `pull`, `undeck`, `stuff`, `held`, `loose` and
   `give` of the verb `oz_games`, `begin` with `cards=` and `looks=`, the chat words
   `!ozg deck sharp` and `!ozg deck zonesharp`; and a bot whose game is folded goes by
   itself.
10. **No count.** Counting the deck from the window was built, checked and taken out the
    same day (below).

Commits, oldest first: `ff37fb0`, `429932b`, `848c10f` (the plan); `d44ade7` (the probe);
`9991633` (names and the record of a card); `a27a591` (cards and decks); `ef3e720` (the
place holds the deck); `0e5736c` (a deal from the deck); `cbbd383` (the window); `d31e3d7`
(counting); `283675c` (the leftovers); `ec20992` (counting taken out).

## Decisions made on the way

1. **A further card of a face is named by the face and its number, with no limit.** The
   owner: "There may be as many copies as you like: you can put together any deck, even of
   aces alone." So a name has no fixed width and nothing cuts it by position:
   `OZG_Cards.RankOf`, `SuitOf`, `FaceOf`, `CopyOf` and `OfMove` take it apart. A click on
   `As.1` plays `As.1`, not `As.10`.
2. **A card of a deal is a record, not a string.** The look rides with the name now; the
   mark (the sign, who reads it, who notices it) is the next phase's field in the same
   record.
3. **The deck is spared by the action, not by a clock.** The game deletes a placed kit in
   `ActionDeployObject.OnEndServer`; `OZG_ActionLayOut` makes the deck "no kit" for the
   length of that one call.
4. **The deck is asked out of the hands and waited for.** A move out of a player's hands
   is the game's hand event: stored, sent, and done a moment later or not at all, and the
   call's answer says nothing. So the session notes when it asked (`AwaitDeck`), a deal is
   refused in that time, and a place its deck did not reach in `DECK_WAIT` (4 s) is unlaid
   and the placer told.
5. **The place is shut on the server and says nothing on a client.** The placer's own
   client is asked whether the deck may go in (`HandGuardCanMove`); a "no" there would
   leave his hands believing they still hold it. The client hides the place's inventory
   instead, and the server is the only judge.
6. **The deck comes back by the ground.** `DropEntityWithTransform` in server mode is done
   inside the call, so the place can be deleted in the same tick without taking the deck
   with it; then free hands are asked to take it up.
7. **A card's look is stored as a style's name.** A number would shift when a server adds
   a mod with a deck of its own.
8. **A deck is told new from old by having been stored.** Nothing of the deck's own is
   saved for it: `OnStoreLoad` running at all is the sign. What follows: **a deck from
   before this build is empty**, on every stand; `!ozg deck` gives a new one.
9. **The cards left behind are picked among all of them**, those in the slots and those
   beside: not "the ones that were brought in".
10. **Counting was taken out**, the owner's word (next part).

## Counting: built, checked, taken out

Task 7 built what §3.5 first said: between deals a seated player pressed "Count the deck",
everybody's log said so, and six seconds later he alone was told how many cards the deck
held, how many of the game's set were missing and how many were copies; with
`CountNamesCards` 1, also which faces were missing. It was seen working in both modes.

The owner then: "I think, let us take the 'count' function out: let them close the game
and count for themselves, looking at the cards in the deck's slots." `ec20992` reverts the
whole of it: the op and its answer, `OZG_WCount`, the two settings (the settings are
version 1 again), the button and four strings. On the build without it the window's lobby
holds no such button (`ui_find` for `OZG_BtnCount`: 0 of 170 widgets), `ask count` answers
`refused STR_OZG_SAY_CANNOT`, and the stand's settings file stays as it is.

**What a player sees when he counts by the slots today**: the deck in the hands opens in
the inventory as a grid of its slots, eight to a row, an empty slot as a ghost icon. Every
card's picture there is the same white bar (the deck's model seen from its edge): how many
slots are full can be counted, which card is which cannot be seen at a glance. The cursor
over a slot names its card ("Двійка пік", with the card's line of description; an empty
slot says "Карта"), and on the ground the cursor names a loose card too ("Two of Clubs",
"Take"). The spec called faces on a card's inventory picture "art for later" (§3.1,
§12); with the count gone that picture is how a deck is read.

**By hand of the client, with the real mouse** (world 84, a sharp's deck of twenty-seven
cards in the hands): a card dragged from its slot into the shirt left the deck
(`cards=27` became `cards=26`, the slot a ghost icon); dragged from the shirt onto an
empty slot it was back (`cards=27`). A card dropped on the deck's own picture is not put
in: the game swaps the hands (the card into the hands, the deck with its cards into the
shirt, where it takes its two cells). A double click on a card in its slot takes nothing out.

## Where the build differs from the spec

- **§3.3, "in random order while there are free slots"**: the cards in the deck's slots
  stay where they are; chance picks the ones that are left (among all), and the others
  that lay beside the deck go into the slots those left. Which cards stay behind is chance,
  as the spec says; no order is shuffled, because a deal shuffles anyway.
- **§3.4 and §8, "the session keeps, for each card in the game, which item it is"**: it
  does not. A game keeps the names and the looks it was dealt in its own state (durak's
  state is version 2 and carries `Dealt`); the items are read anew at each deal. A restart
  in mid-game goes on from the saved state and never looks at the deck. Phase 2 needs the
  table from a card of a game to its item, for the marks; "a card the file names and the
  deck no longer holds is a broken session" comes with it.
- **§3.5**: no count (above). The spec is amended.
- **§3.1**: the card's item wears the deck's own model; nothing is drawn for a single card.

## What was measured

All on the diag build 1.30.164289, one client on the same machine, English; world 83
(Chernarus, the runway at 4701.5 339.0 10196.0) unless said otherwise. A boot to ready took
32 to 40 s, the client 32 to 40 s more.

| What | The line |
|---|---|
| the probe: a kit into what it placed | `hold: mat atts=1 deck=TGP_Deck cards=52 hands=none ms=73; asked=1 parentAtOnce=SurvivorM_Seth`; after a restart the same; `holdback: dropped=1 parentAfterDrop=none cards=52`, then `hands=TGP_Deck/52` |
| names | `copy(As)=0 copy(As.7)=7 face(As.61)=As rank(As.61)=12 suit(As.61)=s named(As,3)=As.3 move(d:As.3:2)=As.3 wrong=0/5` |
| the rules on given cards | the whole set `stock=24`; fourteen aces `trump=As.9 stock=2`; three cards `no STR_OZG_NO_CARDS`; `fuzz: 20 games, 3032 moves, longest 242, 0 defects, 465 ms` |
| a new deck | `deck OZG_Deck_Classic slots=52 cards=52 free=0 faces=52 looks=0:52`; `deck OZG_Deck_Zone_Sharp slots=62 cards=52 free=10 faces=52 looks=1:52` |
| a deck short of two, through a restart | `slots=62 cards=50 free=12 faces=50 looks=0:50` before and after |
| laid out by script | `held: deck OZG_Deck_Classic slots=52 cards=52 free=0 faces=52 looks=0:52 beside=0` |
| laid out by hand, the deck short of its ace | `held: ... cards=51 free=1 faces=51 looks=0:51 beside=0`, the client's hands empty, the window open, nothing unlaid; the same line after a restart, read before any client joined |
| a deal from the deck | whole `trump=Ad stock=24`; short of an ace `stock=23`; a sharp's deck with a second ace `cards=53 ... looks=0:52,1:1`, `stock=25`; aces alone `trump=As.33 stock=25`, played to its end with thirty-seven aces in one hand; too few `refused STR_OZG_NO_CARDS` |
| a restart in mid-game | `st=1 ... trump=Kc stock=24 ... hands=0:6,1:6` before and after |
| the window | a hand `['7h','Td','As.3','Qd','Ac','Ks']` with `HeldLooks [0,0,1,0,0,0]` draws its ace in the worn art; the strip says `6 (1*)` and `6 (2*)`; a click on the second of `As.4 As.1 As.22 As.10 As.23 As.11` played `t:As.1` |
| more cards than slots, an ordinary deck | `stuffed As: deck=0 beside=3`; three runs left `5d/0 Qs/0 Ad/0`, `Ad/0 7c/0 As/1`, `4d/0 9c/0 3s/0`; the deck `cards=52 free=0` each time |
| more cards than slots, a sharp's deck | `stuffed Kh: deck=10 beside=2`; left `Ah/0 Kc/0`; the deck `cards=62 free=0 faces=50 looks=0:50,1:12` |
| room for all | `stuffed Qd: deck=2 beside=0`; `loose: 0` |
| a restart with cards beside two decks | `... looks=0:52 beside=3` and `... slots=62 cards=62 ... looks=0:52,1:10 beside=2`, the same before and after; the worn aces kept their look |
| a gatherer twelve metres away | the hands stayed empty, the deck lay where the place had lain |
| the live pass, by hand of the client | laid out on a kitchen table by the hologram ("Seats: 8 — standing, guessed"): `held: ... cards=52 ... beside=0`; two bots; "Deal" pressed: `trump=Td stock=18 hands=0:6,3:6,5:6`; three moves by clicks (`7c` beaten by `Jc`; `9c` thrown in and beaten by `Ac`; "Pass"); "Fold the game" and its "yes": `sessions=0 places=0`, the deck in the hands `cards=52 free=0` |
| the same on the other terrain | world 84 (NHChernobyl), the wall in Rostok, the mat on the wall's top at 6080.0 108.41 6030.2 and the placer on its step: "Seats: 3 — standing, guessed", `held: ... cards=52 ... beside=0`, three seats on the step at 107.55, a deal `trump=7c stock=18`, a card thrown in by a click, folded by the button, the deck in the hands `cards=52` |
| a deck from before this build | the deck the character of world 84 carried read `slots=52 cards=0 free=52` |
| the other language | a client in Ukrainian: the panel while aiming ("Щось заважає", "[J] — змінити режим місць"), the deck's name ("Колода карт (класична)", the same for a sharp's deck), the window ("Роздати", "Згорнути гру", no count), the refusal of a deal from a deck of twenty-seven cards, eleven of the game's set, for two players: "У колоді замало карт для цієї гри" |
| a game folded under its bots | the stand's tool left their bodies standing (three of them were in the way of the next hologram: "Щось заважає"); since the fix a bot whose game is folded is gone within four seconds (`world_entities` for `SurvivorBase`: 0) |

## Engine facts found

They are in the `dayz-modding` skill, `references/items-and-processing.md`: "A kit that
goes into what it placed", and "Universal slots, a container that is shut, and hands that
are too far". A line each:

- The game deletes a placed kit when the placing action ends; a flag raised for the length
  of that call spares it.
- A move out of a player's hands asked by the server is done a moment later (73 ms here),
  and the call's answer says nothing.
- A drop in server mode is done inside the call; hands asked to take a thing up take it a
  moment later, and not at all when the thing lies farther than the hands reach (2.5 m in
  the game's scripts; twelve metres measured).
- An item that names sixty-two slots goes into the first free one.
- A container whose script says no to every attachment and cargo item still gets its own
  back from the world's storage.
- An item in the hands shows its attachment slots in the inventory, eight to a row.

## A condition that had nothing behind it

The spec's first draft, the plan (Task 9, Step 2) and the first version of this record
said that phase 1 was not done until sixty-three more slots had been measured "against
the slot table of a server with a heavy modpack". The owner asked: "Where did this come
from at all, that slots on a server are limited by the modpack?"

From the draft's author and from nowhere else. When the stand's probe found every one of
fifty-two new slots answered by the engine, he noted that a server with many mods had not
been tried, since the list of slot names is one for all mods; in the spec that note
became a condition. No script of the game, no documentation and nobody's report says
that the list has a ceiling.

Counted then, in the PBOs on this machine, nothing booted (the `Slot_*` class names of
every `config.bin`): the game's own configs declare 470 slots; the seventy mod folders
of a live server's pack (the list its stand of 2026-11-14 was booted with) 1535 more,
two thousand in all, on a server that runs every day; this mod adds sixty-three. On the
stand `InventorySlots.GetSlotIdFromString("Body")` reads 5119774: a slot's id is no
small index, and the engine's own script (`inventoryslots.c`) names a count of 32
"pre-populated" slots and no maximum.

The condition is withdrawn; nothing was booted for it.

## What is not done, and what was not measured

- **Marks, noticing, sleights**: phases 2 to 4. Nothing of them is built.
- **A card's own model and picture.** In the world a loose card is the deck's model, a
  white bar from most sides; in the inventory all cards look alike (above).
- **The table from a card of a game to its item** (above): phase 2.
- **Traders** taking a deck with its cards as one item: the reason for slots, not tried.
- **How long a loose card lies on the ground**: the economy's example has a line for every
  card with a nominal of 0; a card left by a gathering is an item like any other and the
  server's own clean-up decides. Not measured.
- **Between 2.5 and 3.2 m**: "Gather the deck" reaches farther than the hands do. By the
  two numbers a gatherer in that band finds the deck on the ground, not in his hands; the
  band itself was not tried.
- **A second person.** Every check had one client; the others at a table were puppets and
  bots. What a second player's client sees of a place being laid out was not looked at.
- **Taking a card out of a deck that lies on the ground**, and a deck in a backpack: only
  the deck in the hands was tried by hand.
- **A slow link.** `DECK_WAIT` is four seconds against 73 ms measured on one machine.
- **A "no" on the client**: the place never says it there, so what it would do is unknown.
- `OZG_Session.Blank` still works a ring out when the world is read, with nobody near (an
  older defect, untouched).
