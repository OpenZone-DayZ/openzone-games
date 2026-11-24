# Cards and decks as 3D things: what was built (2026-11-24)

The owner's words, in order: make sure every card has its model ("one model for all the
cards will do, just different textures"), draw the cards in the inventory facing the
player, fix how the deck itself is drawn and how a deck and a card are held, and ship two
decks, an ordinary one and the stalkers' ("more decks will come"). Later the same evening:
the logo of OpenZone on the back of the classic deck.

"Ordinary" was read as the classic deck: `zone`, the worn deck of the first phases, left
the mod (its palette stays as the ground the stalkers' deck is drawn on).

## What there is

- **A card is one model**, `models/card/ozg_card.p3d` (`models/src/card.py`): a card of
  0.063 x 0.088 with rounded corners, two sides, `face` and `back`, each one hidden
  selection that takes a whole picture. The fifty-two classes differ by their pictures
  only. The config names the classic deck's; `OZG_Card_Base.OZG_Paint` puts on those of
  the card's own look, on the server and on each client.
- **Pictures for every card of every deck**: `OpenZone_Games/data/cards/<style>/
  ozg_<style>_<face>_co.paa` and `ozg_<style>_back_co.paa`, 256 x 512, drawn by
  `tools/make_cards.py` from the same art as the window's (the same wear, the same marks).
  A deck's config names the path up to the face, `ozgCards`; `OZG_Decks.CardFaceAt` and
  `CardBackAt` end it.
- **The inventory shows faces and backs, not edges**: the slots of a deck show each card's
  face, a deck shows its back. The camera of an item's picture is the `invView` point of
  its Memory LOD; the card's is in front of its face, the deck's in front of its back.
- **The deck stands in its model** with its back towards +Z and shows its back on both
  large sides; it is held in the palm with the game's Matchbox pose. A single card is held
  by its end with the PunchedCard pose, face outwards. The fan keeps the Paper pose.
- **On the ground** a card and a deck lie back up (`rotationFlags = 16`).
- **Two decks**: `OZG_Deck_Classic` and `OZG_Deck_Stalker`, each with a card sharp's
  version. `OZG_Deck_Zone`, its sharp version, its string, its imageset, sheet and art are
  gone.
- **The classic back** carries the mark of the series, the OZ-COM mark the radios carry:
  a pale ring, the dot and the wave in 4FB5E8, where the lozenge was. It lies on the
  lattice itself, which only stands back a little from its strokes: first it sat in a
  medallion, and the owner said "no separate circle round the logo". It is one picture for
  the whole deck, in the window and on the 3D things alike. The mark leans up and right,
  so this back no longer reads the same either way up.

## A bug and what caused it

With both decks in one inventory, each card of a face drew the same picture in both decks:
for some faces the classic one, for others the stalkers'. The server and every client set
the right picture for every card (checked by printing the path in `OZG_Paint` on both
sides, every card of both decks), and the files in the pbo were the right ones (decoded
back from `.paa`). The pictures were named alike in two folders, `classic\c_2s_co.paa` and
`stalker\c_2s_co.paa`. Renamed to `ozg_classic_2s_co.paa` and `ozg_stalker_2s_co.paa`,
with nothing else changed, both decks in one inventory drew their own. So the game took
two textures of one file name in two folders for one texture; which deck's picture won
differed from face to face. Hence the rule in `make_cards.py` (`card_file`) and in the
config's comment: a deck's pictures carry its style in their names.

## Measured on the stand

- `rotationFlags = 16` lays an item with its model's Z axis upright and +Z on top. The
  deck as it lay in its model stood on its end on the ground; stood with its back towards
  -Z it lay face up. `rotationFlags = 32` left it standing on its end.
- The Matchbox pose holds an item with the model's -Z towards the thigh: with the back
  towards +Z, the underside faces the body, hence the back on both sides.
- Without `invView` and the bounding box points in the Memory LOD the inventory looked at
  the deck and the card edge-on (along Z).

The values of `rotationFlags` in the game's own configs, for what they are worth: 1 (168
classes: books, mushrooms), 2 (69: bags, masks, ammunition), 4 and 8 (car doors of the two
sides), 12, 16 (55: hats, optics, Paper), 17 (223: weapon parts), 32 (3: TunaCan,
CarRadiator, LegsCover_Improvised), 34 (56: eyewear, bolts), 63 (7: tins), 64 (6:
bottles, fireplaces), 256 (1). Only 16 and 32 were tried.

## What was checked

NHChernobyl stand, diag 1.30, 1920x1080, a client in Ukrainian:

- a classic deck in hands and a stalkers' deck beside it, both open: every slot of each
  shows its own deck's face; the deck in hands and in the tooltip shows its back;
- a classic queen of spades in hands: held by its end, the face outwards; the inventory
  shows the face;
- a card, a classic deck and a stalkers' deck dropped or spawned on the ground: all three
  back up;
- the deck in hand seen in the first person: the back towards the body;
- the game window on a classic deck: the stock and the seats' strip show the back with
  the mark;
- the server's verdict on every boot: pass, no errors, no warnings.

Not checked: the fan in a hand during a game (its model and pose did not change); a client
in English (no string changed but the zone deck's, which is gone).
