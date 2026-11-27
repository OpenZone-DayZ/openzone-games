# The dice tables moved into the game: record

2026-11-27. The mockups are the design canvas's boards as they were redone that day (its
version 28): thousand, dice poker, pig, high roll, perudo and long nardy, and the parts they
share -- "Банка" (a can), "Кістка" (a die), "Вікно підміни кістки" (the swap's window). The
design is `2026-11-26-boards-and-dice-design.md` (§7, §9.2, §10); the windows this replaces
are in `2026-11-26-board-window-record.md`. The owner's words (chat, Russian; here in
English):

- On the night of 2026-11-26, of the dice window as it then was: he did not like it at
  all; cheating at dice and how it is noticed were to be thought through first. The
  mockups were redone for that: a table with cans, a throw that is seen, the swap as a
  window, the swap noticed as a die seen flying into the can, the "eye".
- "Finish everything by the mockups and test; work in this session until it is completely
  done, and do not ask until it is done; the stand is yours."
- While it was being done: "After an unnoticed swap the die's colour in the can is seen by
  the whole table -- well, if a die is swapped for a die of another colour, after the throw
  everybody must see it, whether the swap was noticed or not."

The stand: NHChernobyl, diag 1.30 (`E:\dayzmod\testserver\games-nh`), the client in the
"Russian" slot (the Ukrainian texts) and in English, at 1920 x 1080 and at 3840 x 1600.
Pushed and published the same day, on the owner's word: "Published", the last section.

## What is built

One commit, the one that carries this record (before it `06491bf`).

### The table

- **A can of dice stands at the seat that throws.** On a seat's own dice the can is its
  own, in its own look (the tin, or the army one of the stalker set), with its dice seen in
  its mouth in their colours; on the host's dice the table has one can, which goes to the
  seat whose throw it is. Between games the cans stand by the variant chosen.
- **A throw is seen.** The dice that lie go back into the can (0.28 to 0.32 s), the table's
  can comes to the seat if it stands elsewhere (0.62 s), the can is shaken (0.48 to
  0.66 s) and tipped (0.13 to 0.15 s), the dice fly out along the canvas's curve (0.52 to
  0.54 s) and tumble as they fall. The window shows the view it had until the dice fly:
  nothing of a throw is told before it is seen. Dice the new view no longer has on the
  table (a bolt at thousand, a one at pig, a tie thrown again) lie a moment (1.1 to 1.3 s)
  before they go. The times are the canvas's, a set a game (`DTFxTimes`).
- **Views wait their turn.** A view that comes while a throw is played waits behind it,
  three at most, each shown when the one before it has been seen: bots that throw one
  after another are seen one after another. More than three, another game or another
  place: the newest is shown at once.
- **Every die is itself**: its colour, its place among the dice its seat throws, what this
  player knows of it -- a load he knows as a badge (the face, how heavy as bars), a die he
  saw slipped in as an eye while it lies in the can it went into.
- **The swap is a window** over the table ("Підмінити кістку" in the action panel, how
  many are left beside it): step 1 the dice he carries, a row a die (its picture in its
  colour, the loaded face up, what he knows in words, how heavy as bars and a word; four
  rows in sight, the engine's scroll for more), step 2 the dice he is about to throw, by
  their places; what the bars mean; what will be done; "Скасувати" and "Підкласти", alive
  once both are picked. It may be opened when a throw is his next move (perudo: in his
  turn before the round's first bid; nardy: before his throw).
- **A swap seen.** The one who does it sees a die hop from his plate into the can and the
  one taken out hop back (0.5 s of 0.64), and a note of what he did. Each player who
  noticed sees the same with an orange ring on the die, the can's foot pulsing, a note
  ("%1 підмінив кістку!") -- and from then an eye on that die. A player whose window is
  shut is told in words, as before.
- **The five games' faces** stand on that table: thousand has a face of its own now
  (`ozg_dg_thousand.layout`, `ozg_dg_thousandcol.layout`: the throw on the felt, the dice
  put aside beside the can, the rest lying where they fell, the turn's points, the barrel
  and the bolts on the plates); dice poker, pig, high roll and perudo are laid again
  (perudo's cans down on the felt, shaken at a round's start, the player's own lifted for
  him alone, all lifted at a showdown).
- **Nardy** has the can in its "Кидок" panel: it is shaken and tipped, the two dice fly
  onto the board; the swap's button and window are the dice games'.
- **Sounds, with what is seen** (the canvas's `sfx`): the can rattles as it is shaken
  (`rattle`), the dice land as they fly out (`land`), a die clacks into the can 0.38 s
  after it leaves the hand (`dieslip`), the game over sounds when its box shows. The window
  plays them itself, on its place; the place's own `roll` (and, at a dice table, its
  `over`) is then left unplayed on that client (`OZG_Sounds.TakeOn`, said every frame, so
  a window gone without a word takes nothing a second later). With no window open the
  place sounds as before.

### The rules and the server

- A game counts its throws (`OZG_GPublic.Throws`, `ThrowSeat`; a place its deals,
  `OZG_WView.DealNo`): a window that sees the number grow plays the throw. The dice of a
  seat's last throw carry their places (`OZG_GSeat.FacePlaces`), a zone of dice its seat
  and places (`OZG_GZone.DiceSeat`, `DicePlaces`); nardy says whose die each of its two is
  (`DiceSeats`, `DiceAt`) and whether they lie (`Thrown`).
- Thousand keeps which of the five each die put aside is, the rest of a throw lying until
  it is thrown again (the zone `lie`), and picks all five up once all are aside; its plates
  carry the tries on the barrel. `OZG_Game.DiceThrownAt` says which dice a seat throws now
  (thousand: those left on the table): what the swap offers and what the session lets go.
- The view says the can each seat throws from (`SeatCanLooks`), what this player knows of
  every die a seat throws and of the dice as it threw them last (`SeatDiceMarks`,
  `ThrownDiceMarks`: the loaded face times ten plus how heavy, a hundred more for a die he
  saw slipped in), and his own dice by their places (`MyDicePlaces`).
- A swap is told to the one who did it and to each who noticed (`OP_DSAW`,
  `OZG_WDiceSaw`: whose can, the looks of the die in and of the die out, its place).
- **A die slipped in is hidden in the can until it is thrown**: to who neither slipped it
  nor saw it go in, the can's mouth shows the die it replaced (`m_SlipLooks`, `m_SlipBy`,
  `DiceLooksHead`). Once thrown everybody sees it as it is -- on the felt, in the seat's
  last throw, and in the can after.
- **Perudo's dice are noticed when they are opened**: a throw under the cans is kept
  (the die itself, its face, who threw it) and each die that lay on its loaded face is
  noticed at the showdown, by the same roll as any other game's at its throw
  (`OZG_Game.ThrowsHidden`, `TakeShown`; `NoticeHidden`). Before, perudo said "a die comes
  up six too often" at a round's start, of dice nobody could see.
- **After a restart** the dice each seat throws are read again when a window is opened,
  not only at the first move (`DiceAgain`): a game that came back from the world's storage
  showed one can, "кістка господаря" on every plate and no dice to swap for until somebody
  moved. While a seat's own dice are still not known (its player not back, his can with
  him) they are read again when asked, three seconds apart.

### The window's code

- `OZG_GameMenuCans.c` (new; loads after the board's file and before the dice window's
  and nardy's): the dice and the cans as widgets kept from one paint to the next
  (`ozg_ddie.layout`, `ozg_dcan.layout`), the clock that moves them the way the canvas's
  CSS does, the throw's steps, the view kept back and the views that wait, the note, a die
  slipped in, the swap's button and window (`ozg_dsrow.layout`), the sounds.
- `OZG_GameMenuDice.c`, `OZG_GameMenuDiceGames.c`, `OZG_GameMenuNardy.c`: the five faces
  and nardy laid on it. `OZG_GameMenu.Noticed` takes a notice by its kind; a log line of
  an option is worded as the option and the value chosen.
- Gone: the dice window's own badges and pocket row (`ozg_dbadge.layout`,
  `ozg_dspocket.layout`), the cup's picture, and the board window's turning pictures and
  their tumble (`ozg_bwturn.layout`, `BWTurnPic`, `BWTumble*`, `OZG_Sounds.s_RollOn`):
  nothing called them once the dice were the table's own.
- `tools/make_dice_win.py` (new, imported by `make_board_win.py`) writes the dice
  window's layouts and its 35 textures: the two cans open and closed, a can's mouth with
  none to six dice, its glow at three drawn sizes, its pulse and shadow, a seat's spot,
  the known-load badges, the bars, the small dice, the eye.
  `python -I tools/make_board_win.py --dice-textures` makes the textures and the layouts,
  `--layouts` the layouts alone. `tools/make_sounds.py` makes the three new sounds from the
  canvas's numbers (26 sounds; the 23 older files come out byte for byte the same).
- 1 176 strings (`python -I tools/stringtable.py OpenZone_Games`).

## What was decided beyond the canvas

The canvas shows one player's table; these it does not settle, or settles only for that
player.

1. On the host's dice the one can stands at the seat whose throw it is, in high roll and
   dice poker too (the canvas draws it by the player).
2. A seat's own row of dice never dims; the last throw stays bright; at a showdown and at a
   game's end every row is bright.
3. The rightmost seat of the top row is mirrored (its dice left of its can) for any number
   of players, and three top seats stand closer: a fourth player's row left the felt.
4. High roll's "dice a throw" is a row of the panel, also between games, where the host
   steps it from one to three; the header's chip says whose dice in every game.
5. Dice poker's hint may take a third line (the canvas's wording is longer than two).
6. A die slipped in is hidden in the can until it is thrown (above). The owner's word
   settles the half after the throw.
7. A notice of a loaded face waits for the throw it speaks of to be seen: it came a second
   and a half before the dice flew.
8. Views wait their turn (above), so that no throw is cut short.
9. Between games the cans stand by the variant; nardy's can is in its panel between games
   too.
10. Nardy says "Кидаєте…" or "%1 кидає…" while the can is shaken.
11. The texts are the canvas's wording (the games' subtitles, dice poker's hints, perudo's
    showdown line).
12. The place's own sound of a throw is not played on a client whose window plays that
    throw; nardy's dice land with `land` (the canvas plays its older `roll` there, a second
    rattle after the can's).
13. Perudo's loaded dice are noticed at the showdown (above).
14. Each board's sleight panel says its own rule, as each canvas words it: chess, draughts
    and giveaway ("a disc put on the last row is a king at once", which the game does),
    corners, nardy. They shared the chess one.
15. A window opened after a restart reads the seats' dice (above).

## What the stand answered

Every screen was walked against its canvas source (the `.dc.html` of each board; some of
the canvas's own screenshots are older than its sources): thousand (throws, dice put
aside, all five aside and thrown again), pig, high roll (the dice a throw stepped between
games, a tie thrown again, the game over), dice poker (four players on the host's can;
three on their own dice, a tin and an army can, stalker dice; stakes, the dice thrown
again, a showdown, the game over), perudo (between games, the shake, the player's own
dice, bids, a showdown, a die lost, a swap under the can), long nardy (between games, a
throw, a die spent, the other side's throw). In Ukrainian and in English, at 1920 x 1080
and at 3840 x 1600.

The last build is `build-1795759116-693`. Its clean boot judged pass on both sides (server:
`games loaded: games=16 places=1 problems=0`, no errors; client: no errors, no crashes).
On it, by the client's script log and the server's:

- a throw's sounds come with its steps: pig `rattle` then `land` 0.61 s later, high roll
  0.73 s, nardy 0.75 s, perudo 0.66 s (the shake, and the tip where there is one); the
  place's `roll` is not played while the window is open, and is with it shut; `dieslip`
  0.37 s after the swap was told; high roll's `over` 1.14 s after its last `land`, with
  the box;
- perudo with a bot whose three dice were loaded on six, every load noticed
  (`noticecfg loaded=100`): nothing at the deal (`a throw under the cans kept: 10 dice`),
  three `notice-load` lines at the showdown (`the dice under the cans opened: 10 kept`),
  the note and the three badges on the opened dice; pig on own dice: `notice-load` at the
  throw, the note with the die on the felt;
- the swap's list with fourteen dice carried, scrolled by the real wheel to its end and
  back; the kit's window with a 52-card deck scrolled the same way;
- a game left in play through a restart, its window opened before any move: the player's
  own can and "своя кістка", his die in the swap's step 2.

A boot in which the bridge's `world_move` is used fails both verdicts with a NULL pointer
of vanilla's `DayZPlayerInventory.OnHandleStoredJunctureData` (dayzplayerinventory.c:670,
`PluginInventoryDebug` not registered on the diag build), one each side a move: the stand's
tool, no line of the mod in the trace. The verdicts above are of a boot without it.

## What the stand found

- The swap's summary was cut by its 420-wide line (520 now).
- The can's glow came out heavier than the canvas's: its picture grew with the can, and
  the engine blends in linear light. Three pictures by the drawn size, the alpha raised to
  1.5 (dayz-modding gui-layouts.md §40).
- A notice held for the throw was never shown: its timer was counted below zero inside a
  block that asked for zero or more.
- A swap nobody noticed was given away by the die's colour in the can's mouth (the die
  hidden until thrown, above).
- The next throw cut the one being played (the views' queue).
- Dice poker's showdown read the first seat's hand as the list of the folded: a line's
  empty part is dropped by `Split` (a "-" stands there now; dayz-modding enforce-script.md).
- High roll and dice poker: the host's can was not at the seat throwing; a seat's own hand
  dimmed while the can stood elsewhere; with four players the right seat's dice left the
  felt.
- Perudo between games showed one can; nardy between games showed an empty panel, and
  listed the dice left without commas.
- Perudo's first round was never noticed at its showdown: the deal's own reset wiped the
  throw kept, because the first throw is made, and taken, inside the deal (the reset stands
  before the deal now).
- A row of the swap's window far below the scroll's edge drew one line of itself on the
  felt (the ninth of fourteen, at both window sizes, gone once the list was scrolled):
  rows out of sight are hidden now (dayz-modding gui-layouts.md §40).
- After a restart the table did not know whose dice were thrown until the first move
  (above).
- **A swap into a player's own can took a die out of it and put nothing back** (found the
  same day, while the pictures for the Workshop page were taken: a deal of dice poker on
  own dice was refused, the can held four dice). The swap moves the die out into a pocket
  and the new one into the slot; a can that lies in a pocket or a backpack refused the
  slot to both -- an item in cargo takes no attachment (`ItemBase.CanReceiveAttachment`),
  though one leaves it -- so the swap answered "no room" and the can stayed a die short,
  once an attempt. It had been there since the swap was built (2026-11-26): every swap
  checked till then went into the can of the place, which lies in no cargo. The can now
  lets the session's own hand fill its slot for the length of the exchange
  (`OZG_Deck_Base.OZG_HandIn`, `OZG_DiceCheat.Exchange`), and an exchange that cannot put
  the first die back says so as an error in the server's log. Checked: two swaps into a
  can in a backpack (a red die in, then out again for a white one, the can's count
  unchanged, the can's mouth showing each), one by a bot into the can of the place
  (dayz-modding items-and-processing.md, "The slot's holder lying in cargo").

## Open

- The sounds were checked by the logs -- which sound, and when -- and not by ear.
- Nardy's throw hint breaks a word earlier than the canvas's ("Ваш кидок. Перед ним /
  можна підмінити кістку."): the box is the canvas's, the game's font is wider.
- After a restart a seat whose player is not back yet is drawn with the host's can until
  he is; on "by wish" dice such a seat is read as the host's and stays so for that game.
- A view dropped by the queue (more than three waiting) plays no throw and so no sound of
  it.

## Published

The owner, 2026-11-27, after the report: "push and publish".

- **GitHub**: `a8cbbc2..c6f21bc`, the 31 commits made since 2026-11-26 -- the boards, the
  dice, cheating with them, their windows and what this record tells.
- **OpenZone Games** 3845773524, from the `dayz` MCP: the build of `c6f21bc` (23 676 353
  bytes, 20 s), version 0.3; a change note in English and Ukrainian; six pictures added to
  the strip (chess, long nardy and dice poker, English and Ukrainian -- fourteen now); the
  new page (`packaging/OpenZone_Games.workshop.bbcode`, 7647 bytes: the boards, the dice,
  cheating with them). Before the upload: the pbo's 462 entries against `c6f21bc`, every
  one the same; the signature against the key; a boot of that build judged good on the
  server and on the client.
- **OpenZone Games for VPP Admin Tools** 3845883015 was not uploaded again: its sources
  have not changed since its upload of 2026-11-25 (the folder built now has the item's
  182 242 bytes), and the stand loads it beside the new games.

The public API read the games' page back equal to the file (updated 2026-11-27 12:22:38Z);
the item's page shows the fourteen pictures and Required items CF and OpenZone Core.
