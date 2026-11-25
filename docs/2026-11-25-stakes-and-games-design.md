# Stakes and four more games -- design as built

Date: 2026-11-25. The owner's words, in this order (chat, Russian; here in English):

1. "Add bridge to what we have in the plans."
2. Games played for stakes: the players agree on the buy-in before the game; the host lays
   the game out, and when somebody asks to join, the host decides whether to let him in;
   where a game has rebuys, the host decides each of them too; the host enters how much a
   player buys in or rebuys for.
3. Correcting my reading of 2: "we do not touch money; the players settle up themselves
   afterwards. But for a game played for money, once the game has been interrupted or the
   deck gathered, make a paper: how much each player brought in all told, and how much each
   had at the end of the game."
4. "Write it down and start building, without the stand for now: as much code for the card
   games as there can be; we check later." Then: "poker is Texas hold'em"; "everything in
   this session, without subagents"; "you may test on the server stand, but do not start
   the client".

So this note is not a draft for approval: it is what was built and why, with every choice the
words leave open named in §8. The design of the platform and of durak is
`docs/specs/2026-11-23-openzone-games-design.md` beside the repositories (§0.17, §0.18 and
§16 there point here).

## 1. What is built

- **Four games beside durak**: ochko (twenty-one with a bank), Texas hold'em (no limit),
  preferans (the Sochi convention, three players) and bridge (four players, Chicago
  vulnerability, each deal scored as in duplicate). Each is a module of 3_Game in the
  form of durak's (design §7): rules and nothing else, chance handed in, seats as numbers.
- **The host chooses the game** at a place between deals, as he chooses durak's variant;
  the settings' `GameIds` says which games a server offers (settings version 7 adds the
  four to a file that listed durak alone).
- **Stakes** for the games that need them (ochko and hold'em): no money is touched; a stack
  is a number the game plays with; the host lets players in and gives rebuys, entering
  each amount; the paper when the game is folded or the deck gathered (§3).
- **One table view for the four**: each game describes its table in the same shape, and the
  window draws that shape (§4). Durak keeps its own view and its own drawing untouched.
- **What the card sharp built stays durak's**: marks are read and noticed on the backs of
  hands in every game (the view's reads and notices are the platform's), but sleights and
  the dealer's three are refused by the four new games (`HasSleights` false).
- **On the stand**: a script plays each game by chance and checks it after every move
  (`gfuzz`), and the stand's bots play the new games by their simplest moves.

## 2. The module, grown

What `OZG_Game` adds (all with defaults, so durak is untouched):

| member | for |
|---|---|
| `UsesStakes()` | the session's stakes (§3); false by default |
| `TakeStacks(seats, amounts)` / `StackOf(seat)` | the stacks handed to a deal, and read back after every change |
| `HasSleights()` | whether sleights and the dealer's three may touch this game; durak true |
| `MoveWords(move, out parts)` | how the log words a move: a key and its arguments (§4) |
| `BotMove(seat, chance)` | the stand's bots: a sensible legal move |
| `ShownHand(seat)` | cards of another seat that lie face up for everybody (the dummy, an open whist, a showdown, ochko's settled hands) |

A new base class, `OZG_CardGame`, holds what the four share: the cards of the deal handed
over (records with their marks), shuffling, the reads and notices of hidden hands, the check
that every card of the deal is somewhere exactly once, and the table view's helpers.

## 3. Stakes

- A game that uses stakes asks every seat to bring chips. **Nothing of a player's money is
  touched**: the session keeps, for every player who sat at the place, how much he brought
  in all told and his stack now (by his id, so a player who stands up and comes back is the
  same line).
- **Asking to sit**: at a game with stakes a free seat is not taken by "Sit here": the
  action asks the host instead ("asked the host"), and the window of the host shows the
  request with a field for the amount, "Let in" and "Refuse". Letting in seats the player
  (on the seat he asked for, or the nearest free one) and adds the amount to his stack and
  to what he brought in. The host seats himself as before and gives himself chips the same
  way (below).
- **Chips for a seat**: between deals the host picks a seated player's plate and enters an
  amount: a buy-in for a newcomer who has none, or a rebuy. A seated player may ask for a
  rebuy with a button; the host sees the request and decides it as above.
- **Amounts** are whole numbers from 1 to 1 000 000; letting a returning player in with 0
  gives him back the stack he left with.
- **A deal** takes only the seats with chips; a seat with none sits out until it gets a
  rebuy. The session hands the stacks to the game and reads them back after every move, so
  a restart in mid-deal loses nothing (the game's state holds them too).
- **The paper**: when the host folds the game, or the deck is gathered, and somebody brought
  chips in, the mod makes a paper: the game's own `Paper` item, its note's text set by the
  server (saved with it, read in the game's own note window). It goes into the hands or the
  inventory of whoever folded or gathered, else onto the ground at the place. Its text:
  the game, the date of the world, then a line a player: his name, brought in, at the end,
  and the difference. The mod writes nothing about money: what a chip is worth is the
  players' agreement.

## 4. The table view

A game's public half and a seat's own half are JSON of fixed classes (`OZG_GPublic`,
`OZG_GPrivate`):

- **seats**: for each seat dealt in, the count and looks of its hidden cards (a fan of backs),
  its face-up cards (a dummy, a showdown), a role word and a line of numbers (a stack and a
  bet, tricks, a score);
- **zones** in the middle: piles of cards with a word above them -- the board of hold'em,
  the trick being played with the seat of each card, the talon of preferans, the bank and
  the pot as numbers;
- **a status line**, the trump, a score sheet of short lines;
- in the seat's own half: its hand with the looks of the backs, the moves its cards make,
  **buttons** for the moves that are no card (bids, "Hit", "Call 40") laid as a grid, and an
  **amount** (a bet, a raise, a bank) with its bounds.

Every text of the view is a key of the string table with its arguments (`KEY|arg|arg`); an
argument `c:As` is worded as a card, `s:3` as the name of a seat. The window draws durak as
before and the four through this view.

## 5. Ochko

- 36 cards, six to ace. Six to ten count by their number, the jack 2, the queen 3, the king
  4, the ace 11. Two to eight players.
- **The bank.** One seat banks: at the first deal the host's seat (or the first seat dealt);
  then the bank stays with its banker from deal to deal until it is broken or closed. A
  banker with no bank on the table puts one up from his stack (`bank:<n>`). Before a round
  he may close the bank (`close`) -- not in the first round of his banking: it goes back to
  his stack and the next seat banks.
- **A round** is one deal: from the banker's left, each player bets (`bet:<n>`, at least 1,
  no more than what is left of the bank after the bets before his, nor than his stack) or
  sits the round out (`skip`). A player who bet takes a card and goes on taking (`hit`) or
  stops (`stand`); over 21 he loses his bet to the bank at once; at 21, or with two aces
  ("golden ochko"), he stops by himself. Then the banker takes his own cards in view of all.
  Over 21, he pays every player still in; otherwise more points than his wins, the same or
  fewer loses; two aces beat everything but two aces, and a tie goes to the bank.
- A bank that reaches nothing is broken and passes to the next seat. An empty stock stops
  the taking. Every bet is taken from the stack when it is made.

## 6. Texas hold'em

- No limit; 52 cards; two to eight players. The blinds are an option the host chooses
  (1/2, 2/5, 5/10, 10/20, 25/50, 50/100, 100/200). The button moves a seat each hand; heads
  up, the button posts the small blind and acts first before the flop.
- Moves: `fold`, `check`, `call`, `raise:<to>` (a bet when nothing is bet yet; the amount is
  the total the seat puts in this round), `allin`. A raise is at least the size of the last
  full raise; an all-in short of that does not reopen the betting for those who acted.
- The board: three, one, one. All folded but one: he takes the pot unseen. All but one
  all-in: the board runs out. At the showdown every hand still in shows; the best five of
  seven win each pot -- the main pot and the side pots by what each seat put in -- split
  evenly, an odd chip to the first winner after the button.
- A hand is one deal; the host deals the next.

## 7. Bridge and preferans

**Bridge.** Four players; the seats pair across the mat by their order (first and third,
second and fourth). The dealer moves a seat each deal; deals go in sets of four with Chicago
vulnerability (none; the dealer's side; the dealer's side; both). The auction: `bid:<level>
<C|D|H|S|N>`, `pass`, `dbl`, `rdbl`; four passes throw the deal in. The declarer is the
first of the contracting side who named the strain; his left-hand opponent leads; then the
dummy's hand lies open and the declarer plays it (`play:<card>` from his own seat). Each
deal is scored as in duplicate (trick score, game or partscore bonus, slams, the insult,
overtricks and undertricks doubled and redoubled); the sheet keeps the two sides' totals.

**Preferans, Sochi.** Three players; 32 cards, seven to ace; ten each and two in the talon;
the dealer moves a seat each deal and the first hand (the dealer's left) speaks and leads
first. The auction: 6 spades up to 10 no trumps (spades, clubs, diamonds, hearts, no trumps),
`misere` only as a player's first word and between 8 no trumps and 9 spades, `pass`; each bid
higher than the last. The declarer takes the talon in view of all, discards two and names a
contract no lower than his bid. Whist or pass, in turn; one whister alone plays open or
closed (open: both defenders' hands lie face up and he plays both). Follow suit, else trump,
else anything. Misère: the declarer must take nothing; the defenders play open and owe
nothing. All passed: a pass-round (raspasy) without trumps, the talon's cards showing the
suit of the first two tricks.

Scoring, Sochi: a contract is worth 2, 4, 6, 8, 10 (6 to 10 tricks; misère 10). Made: its
worth into the declarer's pool; each whister writes on him whists, worth times his tricks (a
lone whister takes the defenders' tricks). Failed: the worth times the missing tricks into
the declarer's mountain, and every whister writes as much again on him besides his tricks.
A whister owes tricks: 4 on six, 2 on seven, 1 on eight and nine, none on ten (two whisters
owe half each); what he leaves short goes into his mountain at the contract's worth a trick.
Both defenders passing gives the declarer his contract unplayed. Misère made: 10 into the
pool; failed: 10 a trick into the mountain. Raspasy: the fewest tricks taken count as none,
every further trick goes into the mountain, and a player who took none writes one into his
pool; repeated raspasy count twice, then three times. The pool closes when every pool holds
the target (an option: 10, 20, 30, 50; 20 by default): the mountain, less what a pool holds
beyond the target, is turned into whists at ten a point against the others, and the sheet
shows each player's balance in whists.

## 8. Choices the words leave open

These were decided in building, to be changed on the owner's word:

1. The paper goes to whoever folds or gathers (§3); its text is bilingual where it has words.
2. A returning player let in with 0 takes back the stack he left with.
3. Ochko: the house rules of §5 (ties to the bank, the bank stays with its banker, closing
   not in the first round). Hold'em: no time limit for a move (the platform has none); an
   absent player is checked or folded when his turn comes (design §6.3).
4. Preferans: three players only (four, with the dealer sitting out, is not built); no
   "here" (holding an equal bid), no half-whist, the pool's overflow not passed to another's
   pool; misère is played open from the first trick; in raspasy the winner of a trick leads
   the next.
5. Bridge: duplicate scoring per deal with Chicago vulnerability, rather than rubber bridge
   with honours.
6. Bids are worded in words ("6 spades", "1 NT"): whether the window's fonts draw the suit
   symbols is not measured.

## 9. Checked

What was checked, and how, is in `2026-11-25-stakes-and-games-record.md` beside this note.
