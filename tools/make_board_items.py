# Writes what there is one of for every piece of the "3 in 1" board (boards and
# dice design §3): the board's 64 slots, the piece classes of each look, the
# board's slot list, the pieces' names, and their lines for a server's economy.
#
#   python -I tools/make_board_items.py
#
# It rewrites the text between pairs of marker lines ("board ...") in
# OpenZone_Games/config.cpp, OpenZone_Games/strings.csv and
# docs/examples/types-openzone-games.xml; everything outside the markers is
# left as it is. Run tools/stringtable.py after it.
import io
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
MOD = os.path.join(ROOT, "OpenZone_Games")
TYPES = os.path.join(ROOT, "docs", "examples", "types-openzone-games.xml")

LOOKS = ["Classic", "Stalker"]
# The stalker chess (design §15): each figurine a model and a picture of its own, the
# stalkers white and the mutants black; its name says who it is.
STALKER_TEXTURE = "OpenZone_Games\\models\\chess_stalker\\data\\ozg_chess_stalker_%s_co.paa"
STALKER_MODEL = "\\OpenZone_Games\\models\\chess_stalker\\ozg_chess_stalker_%s.p3d"
STALKER_MATERIAL = "OpenZone_Games\\models\\chess_stalker\\data\\ozg_chess_stalker.rvmat"
FIGURES = {
    "wK": ("Сидорович", "Sidorovich"), "wQ": ("Стрілок", "Strelok"),
    "wR": ("сталкер в екзоскелеті", "a stalker in an exoskeleton"), "wB": ("науковець", "a scientist"),
    "wN": ("провідник", "a guide"), "wP": ("новачок", "a rookie"),
    "bK": ("контролер", "a controller"), "bQ": ("химера", "a chimera"), "bR": ("псевдогігант", "a pseudogiant"),
    "bB": ("кровосос", "a bloodsucker"), "bN": ("снорк", "a snork"), "bP": ("кабан", "a boar"),
}
# The stalker draughts (words 30 to 32): a face on each side of the disc.
STALKER_DISC_MODEL = "\\OpenZone_Games\\models\\disc\\ozg_disc_faces.p3d"
STALKER_DISC_TEXTURE = "OpenZone_Games\\models\\disc\\data\\ozg_disc_stalker_%s_co.paa"
# The dice (word 33): three colours, and the stalker variant with radiation signs. The
# class, the look the window draws it in (art/dice/<look>_<n>.png), its picture and
# material, its names.
DICE_LOOKS = [
    ("OZG_Die", "white", "ozg_die_co", "ozg_die", "Гральна кістка", "Die"),
    ("OZG_Die_Black", "black", "ozg_die_black_co", "ozg_die", "Чорна гральна кістка", "Black die"),
    ("OZG_Die_Red", "red", "ozg_die_red_co", "ozg_die", "Червона гральна кістка", "Red die"),
    ("OZG_Die_Stalker", "stalker", "ozg_die_stalker_co", "ozg_die_flat", "Сталкерська кістка", "Stalker die"),
    ("OZG_Die_Stalker_Black", "stalker_black", "ozg_die_stalker_black_co", "ozg_die_flat",
     "Чорна сталкерська кістка", "Black stalker die"),
    ("OZG_Die_Stalker_Red", "stalker_red", "ozg_die_stalker_red_co", "ozg_die_flat",
     "Червона сталкерська кістка", "Red stalker die"),
]
# Chess: the kind's letter, how many a side has, the model's base class, the names.
CHESS = [
    ("K", 1, "OZG_Chess_K", "король", "king"),
    ("Q", 1, "OZG_Chess_Q", "ферзь", "queen"),
    ("R", 2, "OZG_Chess_R", "тура", "rook"),
    ("B", 2, "OZG_Chess_B", "слон", "bishop"),
    ("N", 2, "OZG_Chess_N", "кінь", "knight"),
    ("P", 8, "OZG_Chess_P", "пішак", "pawn"),
]
DISCS = 15
DICE = 2
SIDES = [("w", "White", "Білий", "Біла", "white"), ("b", "Black", "Чорний", "Чорна", "black")]
CHESS_TEXTURE = "OpenZone_Games\\models\\chess\\data\\ozg_chess_%s_co.paa"
DISC_TEXTURE = "OpenZone_Games\\models\\disc\\data\\ozg_disc_%s_co.paa"


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
    head = text[:i + len(opener)]
    return head + "\n" + "\n".join(lines) + ("\n" if lines else "") + text[j:]


def slot_names():
    """The 64 slots, in the order of the board's insert: (slot, face)."""
    out = []
    for side, _, _, _, _ in SIDES:
        for letter, count, _, _, _ in CHESS:
            for n in range(count):
                suffix = "" if count == 1 else str(n + 1)
                out.append(("OZG_Bd_%s%s%s" % (side, letter, suffix), side + letter))
    for side, _, _, _, _ in SIDES:
        for n in range(DISCS):
            out.append(("OZG_Bd_%sD%02d" % (side, n + 1), side + "D"))
    for n in range(DICE):
        out.append(("OZG_Bd_Die%d" % (n + 1), "Di"))
    return out


# The can of dice's five slots (boards and dice design §3.3): any die goes into
# them as into the board's.
CAN_SLOTS = ["OZG_Cn_Die%d" % (n + 1) for n in range(5)]


def slots_of(face):
    return [s for s, f in slot_names() if f == face]


def quoted(names):
    return ", ".join('"%s"' % n for n in names)


def rows(names, indent):
    lines = []
    for i in range(0, len(names), 8):
        part = quoted(names[i:i + 8])
        last = i + 8 >= len(names)
        lines.append(indent + part + ("" if last else ","))
    return lines


def config():
    slots = []
    for slot, face in slot_names():
        slots.append('    class Slot_%s { name = "%s"; displayName = "$STR_OZG_PIECE_SLOT"; '
                     'ghostIcon = "set:dayz_inventory image:missing"; };' % (slot, slot))
    for slot in CAN_SLOTS:
        slots.append('    class Slot_%s { name = "%s"; displayName = "$STR_OZG_DIE_SLOT"; '
                     'ghostIcon = "set:dayz_inventory image:missing"; };' % (slot, slot))

    pieces = []
    for look in LOOKS:
        for side, _, _, _, colour in SIDES:
            for letter, _, base, _, _ in CHESS:
                face = side + letter
                if look == "Stalker":
                    f = face.lower()
                    pieces.append('    class OZG_Chess_%s_%s: %s { scope = 2; displayName = "$STR_OZG_PIECE_S_%s"; '
                                  'ozgFace = "%s"; ozgStyle = "stalker"; model = "%s"; hiddenSelectionsTextures[] = {"%s"}; '
                                  'hiddenSelectionsMaterials[] = {"%s"}; inventorySlot[] = {%s}; };'
                                  % (look, face, base, face.upper(), face, STALKER_MODEL % f, STALKER_TEXTURE % f,
                                     STALKER_MATERIAL, quoted(slots_of(face))))
                    continue
                pieces.append('    class OZG_Chess_%s_%s: %s { scope = 2; displayName = "$STR_OZG_PIECE_%s"; '
                              'ozgFace = "%s"; hiddenSelectionsTextures[] = {"%s"}; inventorySlot[] = {%s}; };'
                              % (look, face, base, face.upper(), face, CHESS_TEXTURE % colour,
                                 quoted(slots_of(face))))
            face = side + "D"
            if look == "Stalker":
                pieces.append('    class OZG_Disc_%s_%s: OZG_Disc_Base { scope = 2; displayName = "$STR_OZG_PIECE_S_%s"; '
                              'ozgFace = "%s"; ozgStyle = "stalker"; model = "%s"; hiddenSelectionsTextures[] = {"%s"}; '
                              'inventorySlot[] = {%s}; };'
                              % (look, side.upper(), face.upper(), face, STALKER_DISC_MODEL,
                                 STALKER_DISC_TEXTURE % colour, quoted(slots_of(face))))
                continue
            pieces.append('    class OZG_Disc_%s_%s: OZG_Disc_Base { scope = 2; displayName = "$STR_OZG_PIECE_%s"; '
                          'ozgFace = "%s"; hiddenSelectionsTextures[] = {"%s"}; inventorySlot[] = {%s}; };'
                          % (look, side.upper(), face.upper(), face, DISC_TEXTURE % colour,
                             quoted(slots_of(face))))
    for cls, die_look, tex, mat, _, _ in DICE_LOOKS:
        style = "stalker" if die_look.startswith("stalker") else "classic"
        key = "DI" if cls == "OZG_Die" else "DI_" + die_look.upper()
        pieces.append('    class %s: OZG_Die_Base { scope = 2; displayName = "$STR_OZG_PIECE_%s"; '
                      'ozgFace = "Di"; ozgStyle = "%s"; ozgDieLook = "%s"; '
                      'hiddenSelectionsTextures[] = {"OpenZone_Games\\models\\die\\data\\%s.paa"}; '
                      'hiddenSelectionsMaterials[] = {"OpenZone_Games\\models\\die\\data\\%s.rvmat"}; '
                      'inventorySlot[] = {%s}; };'
                      % (cls, key, style, die_look, tex, mat, quoted(slots_of("Di") + CAN_SLOTS)))

    attachments = ["        attachments[] ="] + ["        {"] + rows([s for s, _ in slot_names()], "            ") + ["        };"]

    path = os.path.join(MOD, "config.cpp")
    text, bom, crlf = read(path)
    for name, lines in (("board slots", slots), ("board pieces", pieces), ("board attachments", attachments)):
        text = between(text, "// >>> generated: " + name, "// <<< generated: " + name, lines)
    write(path, text, bom, crlf)
    return len(slots), len(pieces)


def strings():
    lines = []
    for side, _, ua_m, ua_f, colour in SIDES:
        for letter, _, _, ua, en in CHESS:
            adjective = ua_m
            if letter in ("R",):
                adjective = ua_f
            lines.append("STR_OZG_PIECE_%s%s,%s %s,%s %s" % (side.upper(), letter, adjective, ua, colour.capitalize(), en))
        lines.append("STR_OZG_PIECE_%sD,%s шашка,%s checker" % (side.upper(), ua_f, colour.capitalize()))
    for side, _, ua_m, ua_f, colour in SIDES:
        for letter, _, _, ua, en in CHESS:
            adjective = ua_f if letter in ("R",) else ua_m
            who_ua, who_en = FIGURES[side + letter]
            lines.append('STR_OZG_PIECE_S_%s%s,"%s %s: %s","%s %s: %s"' % (
                side.upper(), letter, adjective, ua, who_ua, colour.capitalize(), en, who_en))
    lines.append("STR_OZG_PIECE_S_WD,Сталкерська шашка: новачок,Stalker checker: a rookie")
    lines.append("STR_OZG_PIECE_S_BD,Сталкерська шашка: бандит,Stalker checker: a bandit")
    for cls, die_look, _, _, ua, en in DICE_LOOKS:
        key = "DI" if cls == "OZG_Die" else "DI_" + die_look.upper()
        lines.append("STR_OZG_PIECE_%s,%s,%s" % (key, ua, en))
    path = os.path.join(MOD, "strings.csv")
    text, bom, crlf = read(path)
    text = between(text, "# >>> generated: board", "# <<< generated: board", lines)
    write(path, text, bom, crlf)
    return len(lines)


def entry(name, restock, usages):
    """One class's lines for types.xml, the decks' numbers: nothing spawns until an
    admin raises the nominal."""
    lines = ['    <type name="%s">' % name,
             "        <nominal>0</nominal>",
             "        <lifetime>14400</lifetime>",
             "        <restock>%d</restock>" % restock,
             "        <min>0</min>",
             "        <quantmin>-1</quantmin>",
             "        <quantmax>-1</quantmax>",
             "        <cost>100</cost>",
             '        <flags count_in_cargo="0" count_in_hoarder="0" count_in_map="1" '
             'count_in_player="0" crafted="0" deloot="0"/>',
             '        <category name="tools"/>']
    lines += ['        <usage name="%s"/>' % u for u in usages]
    lines.append("    </type>")
    return lines


def types():
    lines = ["    <!-- The board and the can of dice: small things of a house, as the decks. -->"]
    count = 0
    for look in LOOKS:
        lines += entry("OZG_Board_%s" % look, 1800, ["Town", "Village"])
        lines += entry("OZG_DiceCan_%s" % look, 1800, ["Town", "Village"])
        count += 2
    lines.append("    <!-- A piece out of its board or can is a thing of its own; one entry a class. -->")
    for look in LOOKS:
        for side, _, _, _, _ in SIDES:
            for letter, _, _, _, _ in CHESS:
                lines += entry("OZG_Chess_%s_%s%s" % (look, side, letter), 0, [])
                count += 1
            lines += entry("OZG_Disc_%s_%s" % (look, side.upper()), 0, [])
            count += 1
    for cls, _, _, _, _, _ in DICE_LOOKS:
        lines += entry(cls, 0, [])
        count += 1
    text, bom, crlf = read(TYPES)
    text = between(text, "<!-- >>> generated: board -->", "    <!-- <<< generated: board -->", lines)
    write(TYPES, text, bom, crlf)
    return count


s, p = config()
print("slots %d, pieces %d, names %d, types %d" % (s, p, strings(), types()))
