# Writes what there is one of for every card: sixty-four slots, fifty-four card
# classes (the fifty-two and the two jokers), the slot lists of the decks, the
# cards' names, and the cards' lines for a server's economy.
#
#   python -I tools/make_card_items.py
#
# It rewrites the text between pairs of marker lines in
# OpenZone_Games/config.cpp, OpenZone_Games/strings.csv and
# docs/examples/types-openzone-games.xml; everything outside the markers is
# left as it is. Run tools/stringtable.py after it.
import io
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MOD = os.path.join(ROOT, "OpenZone_Games")
TYPES = os.path.join(ROOT, "docs", "examples", "types-openzone-games.xml")

RANKS = "23456789TJQKA"
SUITS = "shdc"
SLOTS = 64
PLAIN = 52
SHARP_EXTRA = 10                   # the empty slots of a card sharp's deck
# The jokers, red and black (OZG_Cards.Jokers). Only a deck whose config says
# ozgJokers = 1 holds them, and only the stalkers' look draws them, so their
# classes wear that look's pictures until the script paints them.
JOKERS = ["Xr", "Xb"]
JOKER_UA = {"Xr": "Червоний джокер", "Xb": "Чорний джокер"}
JOKER_EN = {"Xr": "Red joker", "Xb": "Black joker"}
JOKER_LOOK = "OpenZone_Games\\data\\cards\\stalker\\ozg_stalker_"

RANK_UA = ["Двійка", "Трійка", "Четвірка", "П'ятірка", "Шістка", "Сімка", "Вісімка", "Дев'ятка",
           "Десятка", "Валет", "Дама", "Король", "Туз"]
RANK_EN = ["Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Ten", "Jack", "Queen",
           "King", "Ace"]
SUIT_UA = {"s": "пік", "h": "чирв", "d": "бубн", "c": "треф"}
SUIT_EN = {"s": "of spades", "h": "of hearts", "d": "of diamonds", "c": "of clubs"}


# The pictures a card's two sides wear in the config: those of the first look,
# the classic deck's. A card of another look wears its own once the script has
# painted it (OZG_Card_Base.OZG_Paint). tools/make_cards.py draws them all and
# names them ozg_<style>_<face>_co.paa: FIRST_LOOK is the classic deck's
# ozgCards, the path up to the face.
FIRST_LOOK = "OpenZone_Games\\data\\cards\\classic\\ozg_classic_"
CARD_MATERIAL = "OpenZone_Games\\data\\cards\\ozg_card.rvmat"


def look_of(face):
    return JOKER_LOOK if face in JOKERS else FIRST_LOOK


def face_texture(face):
    return look_of(face) + face + "_co.paa"


def back_texture(face="As"):
    return look_of(face) + "back_co.paa"


def faces():
    # The order of OZG_Cards.Whole: a suit at a time, the two first.
    out = []
    for s in SUITS:
        for r in RANKS:
            out.append(r + s)
    return out


def all_faces():
    # Every face a card class is made for: the fifty-two, then the jokers.
    return faces() + JOKERS


def name_of(face):
    if face in JOKERS:
        return JOKER_UA[face], JOKER_EN[face]
    r = RANKS.index(face[0])
    return "%s %s" % (RANK_UA[r], SUIT_UA[face[1]]), "%s %s" % (RANK_EN[r], SUIT_EN[face[1]])


def read(path):
    raw = io.open(path, "rb").read()
    bom = raw.startswith(b"\xef\xbb\xbf")
    text = raw.decode("utf-8-sig")
    crlf = "\r\n" in text
    return text.replace("\r\n", "\n"), bom, crlf


def write(path, text, bom, crlf):
    if crlf:
        text = text.replace("\n", "\r\n")
    data = text.encode("utf-8")
    if bom:
        data = b"\xef\xbb\xbf" + data
    io.open(path, "wb").write(data)


def between(text, opener, closer, lines):
    i = text.index(opener)
    j = text.index(closer)
    i = text.index("\n", i) + 1
    j = text.rindex("\n", 0, j) + 1
    return text[:i] + "\n".join(lines) + "\n" + text[j:]


def rows(names, indent):
    out = []
    for k in range(0, len(names), 10):
        part = ", ".join('"%s"' % n for n in names[k:k + 10])
        if k + 10 < len(names):
            part += ","
        out.append(indent + part)
    return out


def slot_list(names):
    return ["        attachments[] =", "        {"] + rows(names, "            ") + ["        };"]


def config():
    names = ["OZG_Card%02d" % i for i in range(1, SLOTS + 1)]

    slots = []
    for n in names:
        slots.append('    class Slot_%s { name = "%s"; displayName = "$STR_OZG_CARD_SLOT"; '
                     'ghostIcon = "set:dayz_inventory image:missing"; };' % (n, n))

    cards = ["    class OZG_Card_Base: Inventory_Base",
             "    {",
             "        scope = 0;",
             '        descriptionShort = "$STR_OZG_CARD_DESC";',
             '        model = "\\OpenZone_Games\\models\\card\\ozg_card.p3d";',
             "        itemSize[] = {1, 1};",
             "        weight = 2;",
             "        rotationFlags = 16;",
             '        hiddenSelections[] = {"face", "back"};',
             '        hiddenSelectionsTextures[] = {"%s", "%s"};' % (face_texture("As"), back_texture()),
             '        hiddenSelectionsMaterials[] = {"%s", "%s"};' % (CARD_MATERIAL, CARD_MATERIAL),
             '        ozgFace = "";',
             "        inventorySlot[] =",
             "        {"]
    cards += rows(names, "            ")
    cards += ["        };", "    };"]
    for f in all_faces():
        cards.append('    class OZG_Card_%s: OZG_Card_Base { scope = 2; displayName = "$STR_OZG_CARD_%s"; '
                     'ozgFace = "%s"; hiddenSelectionsTextures[] = {"%s", "%s"}; };'
                     % (f, f.upper(), f, face_texture(f), back_texture(f)))

    jokered = PLAIN + len(JOKERS)
    path = os.path.join(MOD, "config.cpp")
    text, bom, crlf = read(path)
    for name, lines in (("slots", slots),
                        ("cards", cards),
                        ("deck slots", slot_list(names[:PLAIN])),
                        ("deck slots, stalker", slot_list(names[:jokered])),
                        ("sharp slots, classic", slot_list(names[:PLAIN + SHARP_EXTRA])),
                        ("sharp slots, stalker", slot_list(names[:jokered + SHARP_EXTRA]))):
        text = between(text, "// >>> generated: " + name, "// <<< generated: " + name, lines)
    write(path, text, bom, crlf)
    return len(names)


def strings():
    lines = []
    for f in all_faces():
        ua, en = name_of(f)
        lines.append("STR_OZG_CARD_%s,%s,%s" % (f.upper(), ua, en))
    path = os.path.join(MOD, "strings.csv")
    text, bom, crlf = read(path)
    text = between(text, "# >>> generated: cards", "# <<< generated: cards", lines)
    write(path, text, bom, crlf)
    return len(lines)


def types():
    lines = []
    for f in all_faces():
        lines += ['    <type name="OZG_Card_%s">' % f,
                  "        <nominal>0</nominal>",
                  "        <lifetime>14400</lifetime>",
                  "        <restock>0</restock>",
                  "        <min>0</min>",
                  "        <quantmin>-1</quantmin>",
                  "        <quantmax>-1</quantmax>",
                  "        <cost>100</cost>",
                  '        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" '
                  'count_in_player="0" crafted="0" deloot="0"/>',
                  '        <category name="tools"/>',
                  "    </type>"]
    text, bom, crlf = read(TYPES)
    text = between(text, "<!-- >>> generated: cards -->", "<!-- <<< generated: cards -->", lines)
    write(TYPES, text, bom, crlf)
    return len(all_faces())


print("slots %d, names %d, types %d" % (config(), strings(), types()))
