"""The dice games' window (boards and dice design §6 to §9): ozg_dwin.layout, the games'
own faces, the parts of a dice table and their textures.

    python -I tools/make_dice_win.py [--no-paa] [--layouts]

The window is the design canvas's boards "Тисяча", "Покер на кістках", "Свиня", "Хто більше"
and "Перудо" as redone on 2026-11-27, and their parts "Кістка", "Банка", "Вікно підміни
кістки": the felt tray with the players' plates round it, a can of dice at the seat that
throws, the dice flying out of it onto the felt; right the score, what this player may do,
the swap's button, the log; over the tray the swap's own window. Every place is the
canvas's, in its px (the layout's units, 1600 x 900), from the tray's corner (52, 86 of the
window) or from the right column's (1164, 86).

What moves is laid by the script (OZG_GameMenuDiceTable.c): the dice and the cans are
small layouts of their own (ozg_ddie.layout, ozg_dcan.layout) made into the tray and placed
every frame; a plate, a pill, a button is placed once a view.

The helpers that build a layout's blocks are tools/make_board_win.py's, which also writes
nardy's window with this file's swap window, note and can in it.
"""
import io
import math
import os
import sys

from PIL import Image, ImageChops, ImageDraw, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
if HERE not in sys.path:
    sys.path.insert(0, HERE)

import make_board_win as bw  # noqa: E402
from make_board_win import (block, exact, panel, image, frame, text, button, shape, box, btn, dims, rgb,  # noqa: E402
                            dialogs, soft_shadow, write, save,
                            BTN, BTN_EDGE, INK, MUTED, FAINT, GOLD, LOG_INK, SL_BTN, RED_BTN, RED_INK, CLOSE,
                            CLOSE_INK, DIALOG, DIALOG_EDGE, LOG_LINES, LANCZOS, IMAGE_TO_PAA)

CARD = "#110b06"      # a plate on the felt (the canvas: rgba(18, 11, 6, .82..88) over it)
CHIPS = "#e7c27a"
HINT = "#b6ae9b"      # words on the felt (the canvas: rgba(239, 228, 204, .75) over it)
PANEL = "#2a1d12"     # the action panel
SOFT = "#221911"      # the score, the log, the round


# ---------------------------------------------------------------- small parts

def chip(name, x, y, prio=3):
    """A tag on a plate (the canvas: 22 high, a pill, 12 px in bold): the script sizes it
    to its words and colours it."""
    return frame(name, exact(x, y, 120, 22), prio, [
        shape(name + "Bg", 0, 0, 120, 22, 11, rgb("#1f1811"), 0, overlap=0),
        text(name + "Text", 8, 3.5, 200, 15, 12, "b", MUTED, 1)], [("visible", 0)])


def name_row(p, w, y, right_w=138):
    """A plate's name, "(ви)" after it on this player's own (the script places it)."""
    return [text(p + "Name", 12, y, w - 24 - 40, 20, 16, "b", INK, 3),
            text(p + "You", 12, y, 60, 20, 16, "s", MUTED, 3, "left", "", [("visible", 0)])]


def turn_pill(name, x, y):
    """The turn's points (the canvas: a pill 50 high, "ЗА ХІД" and the number): as wide as
    its words, centred by the script."""
    return frame(name, exact(x, y, 113, 50), 10, [
        box(name + "Box", 0, 0, 113, 50, 25, CARD, GOLD, prio=0),
        text(name + "Label", 20, 17.5, 160, 15, 12, "s", GOLD, 3),
        text(name + "Value", 76.5, 7.5, 80, 35, 26, "p", INK, 3)])


def bank_pill(name, x, y):
    """The pot as the canvas draws it: a pill 264 x 70, its chip, "БАНК", the number."""
    return frame(name, exact(x, y, 264, 70), 10, [
        box(name + "Box", 0, 0, 264, 70, 35, CARD, GOLD, prio=0),
        image(name + "Icon", exact(75.5, 20, 30, 30), 2, "ozg_chip_ca"),
        # a label's box is as wide as its longest translation wants: a text is cut at its
        # widget's edge (the stand in English, 2026-11-26: "THIS TURN" lost half its N)
        text(name + "Label", 117.5, 27, 160, 16, 13, "s", GOLD, 3),
        text(name + "Value", 169, 15, 120, 40, 30, "p", INK, 3)], [("visible", 0)])


def act_panel(name, y, h, kids):
    """A panel of the right column where this player acts: its ground, its ring (gold while
    it is his turn), what it holds."""
    return frame(name, exact(12, y, 360, h), 6, [box(name + "Box", 0, 0, 360, h, 6, PANEL, GOLD, prio=0)] + kids)


def hint_lines(p, y, n=3):
    return [text(p + "Hint%d" % i, 12, y + i * 19, 336, 19, 14, "s", LOG_INK, 2) for i in range(n)]


# ---------------------------------------------------------------- the faces on the felt

OTHERS = 7     # the plates of the other players a face has (high roll seats eight)


def face_thousand():
    """Thousand (the canvas's "Тисяча"): a plate a player (name, total, tags, his last
    throw), this player's own under the felt's right, "ЗА ХІД" in the middle."""
    k = []
    for i in range(OTHERS):
        p = "OZG_DThP%d" % i
        kids = [box(p + "Box", 0, 0, 280, 148, 8, CARD, "#4a3726", prio=0)] + name_row(p, 280, 16)
        kids.append(text(p + "Total", 130, 10, 138, 32, 24, "p", INK, 3, "right"))
        kids += [chip(p + "Tag%d" % t, 12, 49) for t in range(5)]
        kids.append(frame(p + "Dice", exact(12, 108, 256, 26), 3))
        kids.append(text(p + "Note", 180, 113.5, 120, 15, 12, "s", MUTED, 3))
        k.append(frame(p, exact(78, 74, 280, 148), 10, kids, [("visible", 0)]))
    p = "OZG_DThMe"
    kids = [box(p + "Box", 0, 0, 280, 81, 8, CARD, BTN_EDGE, prio=0)] + name_row(p, 280, 16)
    kids.append(text(p + "Total", 130, 10, 138, 32, 24, "p", INK, 3, "right"))
    kids += [chip(p + "Tag%d" % t, 12, 49) for t in range(5)]
    k.append(frame(p, exact(738, 610, 280, 81), 10, kids, [("visible", 0)]))
    k.append(turn_pill("OZG_DThPill", 493.5, 490))
    k.append(text("OZG_DThAside", 218, 590, 200, 15, 12, "s", GOLD, 6, "left", "", [("visible", 0)]))
    return k


def face_pig():
    """Pig (the canvas's "Свиня"): a plate a player (name, score, the bar to the goal with
    the turn's points after it, tags), "ЗА ХІД"."""
    k = []
    plates = ["OZG_DPgP%d" % i for i in range(OTHERS)] + ["OZG_DPgMe"]
    for p in plates:
        me = p.endswith("Me")
        kids = [box(p + "Box", 0, 0, 280, 100, 8, CARD, "#4a3726", prio=0)] + name_row(p, 280, 16)
        kids.append(text(p + "Score", 130, 10, 138, 32, 24, "p", INK, 3, "right"))
        kids.append(frame(p + "Bar", exact(12, 50, 256, 10), 3, [
            shape(p + "BarTrack", 0, 0, 256, 10, 5, rgb("#3a2c1c"), 0, overlap=0),
            frame(p + "BarFill", exact(0, 0, 0, 10), 1, [
                shape(p + "BarFillShape", 0, 0, 256, 10, 5, rgb(GOLD if me else "#8f7d62"), 0, overlap=0)], clip=1),
            frame(p + "BarTurn", exact(0, 0, 0, 10), 2, [
                shape(p + "BarTurnShape", 0, 0, 256, 10, 5, rgb("#8f7742"), 0, overlap=0)], clip=1)], clip=1))
        kids += [chip(p + "Tag%d" % t, 12, 68) for t in range(2)]
        k.append(dims(frame(p, exact(738 if me else 78, 602 if me else 74, 280, 100), 10, kids, [("visible", 0)])))
    k.append(turn_pill("OZG_DPgPill", 493.5, 490))
    return k


def face_high():
    """High roll (the canvas's "Хто більше"): a plate a player (name, chips, what he does,
    his total), the pot."""
    k = []
    plates = ["OZG_DHP%d" % i for i in range(OTHERS)] + ["OZG_DHMe"]
    for p in plates:
        me = p.endswith("Me")
        kids = [box(p + "Box", 0, 0, 260, 73, 8, CARD, "#4a3726", prio=0)] + name_row(p, 260, 9)
        kids.append(text(p + "Chips", 80, 11, 168, 16, 13, "s", CHIPS, 3, "right"))
        kids.append(text(p + "State", 12, 41.5, 180, 15, 12, "s", MUTED, 3))
        kids.append(text(p + "Sum", 148, 30.5, 100, 37, 28, "p", INK, 3, "right"))
        k.append(dims(frame(p, exact(768 if me else 68, 606 if me else 74, 260, 73), 10, kids, [("visible", 0)])))
    k.append(bank_pill("OZG_DHBank", 416, 384))
    return k


def face_poker():
    """Dice poker (the canvas's "Покер на кістках"): a plate a player (name, the rounds he
    won, chips; his hand, what he put in), the pot."""
    k = []
    plates = ["OZG_DPP%d" % i for i in range(OTHERS)] + ["OZG_DPMe"]
    for p in plates:
        me = p.endswith("Me")
        kids = [box(p + "Box", 0, 0, 280, 73, 8, CARD, BTN_EDGE, prio=0)] + name_row(p, 280, 10)
        kids.append(image(p + "Win0", exact(150, 14, 12, 12), 3, "ozg_ring6_ca", rgb(GOLD)))
        kids.append(image(p + "Win1", exact(167, 14, 12, 12), 3, "ozg_ring6_ca", rgb(GOLD)))
        kids.append(text(p + "Chips", 100, 12, 168, 16, 13, "s", CHIPS, 3, "right"))
        kids.append(text(p + "Hand", 12, 37, 190, 26, 19, "p", INK, 3))
        kids.append(text(p + "State", 100, 44.5, 168, 15, 12, "s", MUTED, 3, "right"))
        k.append(dims(frame(p, exact(738 if me else 78, 608 if me else 74, 280, 73), 10, kids, [("visible", 0)])))
    k.append(bank_pill("OZG_DPBank", 416, 388))
    return k


def face_perudo():
    """Perudo (the canvas's "Перудо"): a plate a player (name, the dice he has left, a
    line), the bid in the middle, the words under this player's own dice."""
    k = []
    plates = ["OZG_DRP%d" % i for i in range(OTHERS)] + ["OZG_DRMe"]
    for p in plates:
        me = p.endswith("Me")
        kids = [box(p + "Box", 0, 0, 260, 59, 8, CARD, "#4a3726", prio=0)] + name_row(p, 260, 9)
        kids.append(text(p + "Count", 80, 11, 168, 16, 13, "s", MUTED, 3, "right"))
        kids.append(text(p + "Line", 12, 34.5, 236, 15, 12, "s", MUTED, 3))
        k.append(dims(frame(p, exact(768 if me else 68, 606 if me else 74, 260, 59), 10, kids, [("visible", 0)])))
    k.append(frame("OZG_DRBid", exact(300, 354, 500, 111), 10, [
        box("OZG_DRBidBox", 0, 0, 500, 111, 10, CARD, GOLD, prio=0),
        text("OZG_DRBidLabel", 0, 14, 500, 15, 12, "s", GOLD, 3, "center"),
        text("OZG_DRBidText", 0, 35, 500, 40, 30, "p", INK, 3, "center"),
        text("OZG_DRBidSub", 0, 81, 500, 16, 13, "s", MUTED, 3, "center")]))
    k.append(text("OZG_DRUnder", 210, 660, 420, 15, 12, "s", "#aca391", 6, "left", "", [("visible", 0)]))
    return k


# ---------------------------------------------------------------- the faces in the column

def col_thousand():
    """Thousand's panel: whose throw and how many dice are in it, what to do, "put aside",
    throw and write."""
    kids = [text("OZG_DThTitle", 12, 12, 230, 22, 17, "p", INK, 2),
            text("OZG_DThLeft", 150, 15.5, 198, 15, 12, "s", MUTED, 2, "right")]
    kids += hint_lines("OZG_DTh", 44)
    kids += [btn("OZG_DThAsideBtn", 12, 92, 336, 42, 4, SL_BTN, BTN_EDGE, "", 15, "b", FAINT, prio=6),
             btn("OZG_DThThrow", 12, 144, 164, 44, 4, GOLD, None, "", 15, "b", "#1a120a", prio=6),
             btn("OZG_DThWrite", 184, 144, 164, 44, 4, "#241a12", "#3d2d1d", "", 15, "b", FAINT, prio=6)]
    return [act_panel("OZG_DThAct", 152, 248, kids)]


def col_pig():
    kids = [text("OZG_DPgTitle", 12, 12, 336, 22, 17, "p", INK, 2)] + hint_lines("OZG_DPg", 44)
    kids += [btn("OZG_DPgThrow", 12, 92, 336, 46, 4, GOLD, None, "#STR_OZG_DICE_BTN_THROW", 16, "b", "#1a120a", prio=6),
             btn("OZG_DPgHold", 12, 148, 336, 46, 4, BTN, "#3d2d1d", "", 16, "b", FAINT, prio=6)]
    return [act_panel("OZG_DPgAct", 12, 254, kids)]


def col_high():
    kids = [text("OZG_DHTitle", 12, 12, 336, 22, 17, "p", INK, 2)] + hint_lines("OZG_DH", 44)
    kids += [btn("OZG_DHThrow", 12, 92, 336, 46, 4, GOLD, None, "", 16, "b", "#1a120a", prio=6),
             button("OZG_DHCount", 12, 148, 336, 38, 6, [
                 box("OZG_DHCountBox", 0, 0, 336, 38, 4, BTN, "#3d2d1d", prio=0),
                 text("OZG_DHCountText", 12, 11, 220, 16, 13, "s", FAINT, 2, "left", "#STR_OZG_DWIN_HR_PER_THROW"),
                 text("OZG_DHCountValue", 224, 11, 100, 16, 13, "b", FAINT, 2, "right")])]
    return [act_panel("OZG_DHAct", 12, 246, kids)]


def col_poker():
    """Dice poker's round: its number and step, five segments, what goes on. Its buttons
    are the window's own action panel under it."""
    rd = [shape("OZG_DPRoundBg", 0, 0, 360, 94, 6, rgb(SOFT), 0),
          text("OZG_DPRoundTitle", 12, 12, 336, 14, 11, "s", GOLD, 2)]
    for i in range(5):
        rd.append(shape("OZG_DPSeg%d" % i, 12 + 68 * i, 31, 64, 6, 3, rgb("#3a2c1c"), 2, overlap=0))
    # two lines of words (the canvas keeps room for two); a third when they run long,
    # the box and all under it a line lower
    rd += [text("OZG_DPRHint0", 12, 43.5, 336, 18, 14, "s", LOG_INK, 2),
           text("OZG_DPRHint1", 12, 62.5, 336, 18, 14, "s", LOG_INK, 2),
           text("OZG_DPRHint2", 12, 81.5, 336, 18, 14, "s", LOG_INK, 2)]
    return [frame("OZG_DPRound", exact(12, 12, 360, 94), 6, rd)]


def col_perudo():
    """Perudo's bid: whose it is, how many dice of which face, bid or do not believe."""
    ask = [box("OZG_DRAskBox", 0, 0, 360, 323, 6, PANEL, GOLD, prio=0),
           text("OZG_DRAskTitle", 12, 12, 250, 22, 17, "p", INK, 2),
           frame("OZG_DRPal", exact(260, 12, 88, 22), 3, [
               shape("OZG_DRPalBg", 0, 0, 88, 22, 11, rgb("#3a2f1c"), 0, overlap=0),
               text("OZG_DRPalText", 8, 3.5, 200, 15, 12, "b", CHIPS, 1, "left", "#STR_OZG_DWIN_PR_PALIFICO")],
                 [("visible", 0)]),
           text("OZG_DRAskSub", 12, 44, 336, 15, 12, "s", MUTED, 2, "left", "#STR_OZG_DWIN_PR_HOW"),
           btn("OZG_DRMinus", 12, 69, 40, 40, 4, BTN, BTN_EDGE, "−", 20, "s", INK, prio=6),
           shape("OZG_DRValueBg", 60, 69, 240, 40, 4, rgb("#1a120a"), 2),
           text("OZG_DRValue", 60, 73, 240, 32, 24, "p", INK, 3, "center"),
           btn("OZG_DRPlus", 308, 69, 40, 40, 4, BTN, BTN_EDGE, "+", 20, "s", INK, prio=6)]
    for i in range(6):
        f = "OZG_DRFace%d" % i
        ask.append(dims(button(f, 12 + 58 * i, 119, 46, 46, 6, [
            shape(f + "Ring", -3, -3, 52, 52, 11, rgb(GOLD), 0, extra=[("visible", 0)]),
            image(f + "Pic", exact(0, 0, 46, 46), 1, None)], clip=0)))
    ask += [text("OZG_DRNote0", 12, 175, 336, 17, 13, "s", MUTED, 2),
            text("OZG_DRNote1", 12, 192, 336, 17, 13, "s", MUTED, 2),
            btn("OZG_DRBidBtn", 12, 219, 164, 44, 4, SL_BTN, None, "#STR_OZG_DWIN_PR_BID", 15, "b", FAINT, prio=6),
            btn("OZG_DRDudoBtn", 184, 219, 164, 44, 4, RED_BTN, None, "#STR_OZG_DWIN_PR_DUDO", 15, "b", RED_INK, prio=6)]
    return [frame("OZG_DRAsk", exact(12, 12, 360, 323), 6, ask)]


def face_layout(root, w, h, kids):
    """A game's face: a frame the size of its parent (the tray or the column) holding its
    parts, made into that parent by the script."""
    return block("FrameWidgetClass", root,
                 [("ignorepointer", 1), ("clipchildren", 0)] + exact(0, 0, w, h) + [("priority", 10)], kids)


def dice_game_faces():
    tray = {"thousand": face_thousand(), "pig": face_pig(), "high": face_high(), "poker": face_poker(),
            "perudo": face_perudo()}
    col = {"thousand": col_thousand(), "pig": col_pig(), "high": col_high(), "poker": col_poker(),
           "perudo": col_perudo()}
    out = []
    for gid in ("thousand", "poker", "pig", "high", "perudo"):
        out.append(("ozg_dg_%s.layout" % gid, face_layout("OZG_DG_%s" % gid, 1100, 774, tray[gid])))
        out.append(("ozg_dg_%scol.layout" % gid, face_layout("OZG_DGC_%s" % gid, 384, 774, col[gid])))
    return out


# ---------------------------------------------------------------- what every table shares

def toast(x, y):
    """A note over the table (the canvas: 356 wide, a ring of two -- orange for a swap
    seen --, a caption, the words, smaller words under them); as high as what it says."""
    k = [shape("OZG_DToastEdge", -2, -2, 360, 64, 8, rgb("#6b5233"), 0),
         shape("OZG_DToastBg", 0, 0, 356, 60, 6, rgb("#1c120a"), 1),
         text("OZG_DToastCap", 14, 10, 328, 15, 12, "b", "#ff8a3d", 3, "center")]
    for i in range(3):
        k.append(text("OZG_DToastText%d" % i, 14, 10 + i * 21, 328, 21, 16, "s", "#f6e8d6", 3, "center"))
    for i in range(4):
        k.append(text("OZG_DToastSub%d" % i, 14, 40 + i * 16, 328, 16, 12, "s", MUTED, 3, "center"))
    return frame("OZG_DToast", exact(x, y, 356, 60), 70, k, [("visible", 0)])


def swap_button(x, y, w=336):
    """"Підмінити кістку" and how many swaps are left (the canvas: 38 high, a ring of one);
    it opens the swap's window. The script places it in whichever panel holds it."""
    return button("OZG_DSwapBtn", x, y, w, 38, 30, [
        box("OZG_DSwapBtnBox", 0, 0, w, 38, 4, BTN, "#3d2d1d", prio=0),
        text("OZG_DSwapBtnText", 12, 11, w - 24, 16, 13, "s", FAINT, 2, "left", "#STR_OZG_BWIN_SWAP"),
        text("OZG_DSwapBtnNote", 12, 11.5, w - 24, 15, 12, "s", MUTED, 2, "right")], visible=0)


def step_disc(name, x, y, digit):
    return frame(name, exact(x, y, 22, 22), 3, [
        image(name + "Bg", exact(0, 0, 22, 22), 0, "ozg_disc_ca", rgb(GOLD)),
        text(name + "Text", 0, 0, 22, 22, 13, "b", "#1a120a", 1, "center", digit)])


SWAP_W = 720


def swap_dialog(x, y, w, h):
    """The swap's window (the canvas's "Вікно підміни кістки"), the same in every dice game
    and at nardy: over a dim of what it covers (`x`, `y`, `w`, `h` of the window), a box
    720 wide with two steps side by side -- which of his own dice goes into the can (a row a
    die: its picture in its colour with the loaded face up, what he knows of it in words,
    how heavy as three bars and a word) and which of the can's dice it replaces --, what
    the bars mean, what will be done, "Скасувати" and "Підкласти".

    He may carry any number of dice: the rows are the engine's scroll, four in sight. The
    script lays a row a die (ozg_dsrow.layout), the can's dice (ozg_ddie.layout), sizes the
    box to what it holds and centres it."""
    k = [soft_shadow("OZG_DSwShadow", 0, 0, SWAP_W, 400, 18, 0.65, 0),
         box("OZG_DSwBox", 0, 0, SWAP_W, 400, 8, DIALOG, DIALOG_EDGE, outside=True, prio=1),
         text("OZG_DSwTitle", 18, 16, 300, 30, 22, "p", INK, 3, "left", "#STR_OZG_DWIN_SWAP_TITLE"),
         text("OZG_DSwCount", 200, 23, 400, 16, 13, "s", MUTED, 3),
         button("OZG_DSwClose", SWAP_W - 50, 16, 32, 30, 6, [
             box("OZG_DSwCloseBox", 0, 0, 32, 30, 4, "#33251a", prio=0),
             image("OZG_DSwCloseX", exact(10, 9, 12, 12), 2, "ozg_icon_x_ca", rgb(LOG_INK))]),
         # step 1: the dice he carries
         step_disc("OZG_DSwStep1", 18, 60, "1"),
         text("OZG_DSwLabel1", 48, 62.5, 354, 17, 14, "b", INK, 3, "left", "#STR_OZG_DWIN_SWAP_STEP1"),
         frame("OZG_DSwNone", exact(18, 90, 384, 64), 3, [
             shape("OZG_DSwNoneBg", 0, 0, 384, 64, 6, rgb("#241a12"), 0),
             text("OZG_DSwNone0", 14, 14, 356, 18, 13, "s", MUTED, 1),
             text("OZG_DSwNone1", 14, 32, 356, 18, 13, "s", MUTED, 1)], [("visible", 0)]),
         ("ScrollWidgetClass", "OZG_DSwRows",
          exact(18, 90, 394, 250) + [("color", rgb(GOLD, 0.9)), ("priority", 5), ("clipchildren", 1),
                                     ("Scrollbar V", 1), ("style", "blank")],
          [("FrameWidgetClass", "OZG_DSwRowsIn",
            [("ignorepointer", 1), ("clipchildren", 0)] + exact(0, 0, 384, 250) + [("priority", 1)], None)]),
         # step 2: the dice of the can
         step_disc("OZG_DSwStep2", 420, 60, "2"),
         text("OZG_DSwLabel2", 450, 62.5, 252, 17, 14, "b", INK, 3, "left", "#STR_OZG_DWIN_SWAP_STEP2"),
         frame("OZG_DSwCan", exact(420, 90, 282, 86), 3, [
             shape("OZG_DSwCanBg", 0, 0, 282, 86, 6, rgb("#241a12"), 0),
             frame("OZG_DSwCanDice", exact(0, 0, 282, 86), 1)])]
    for i in range(5):
        k.append(text("OZG_DSwCanNum%d" % i, 430 + i * 54, 150, 46, 16, 13, "b", LOG_INK, 6, "center", "", [("visible", 0)]))
        k.append(button("OZG_DSwCanHit%d" % i, 430 + i * 54, 100, 46, 68, 7, None, visible=0))
    for i in range(4):
        k.append(text("OZG_DSwNote%d" % i, 420, 184 + i * 17, 282, 17, 12, "s", MUTED, 3))
    # what the bars mean
    leg = [shape("OZG_DSwLegendBg", 0, 0, SWAP_W - 36, 36, 6, rgb("#241a12"), 0),
           text("OZG_DSwLegendLabel", 12, 10.5, 420, 15, 12, "s", MUTED, 1, "left", "#STR_OZG_DWIN_SWAP_LEGEND")]
    for i in range(3):
        leg.append(image("OZG_DSwLegendBars%d" % i, exact(300 + i * 110, 12, 14.25, 12), 1, "ozg_sbars_%d_ca" % (i + 1)))
        leg.append(text("OZG_DSwLegendText%d" % i, 320 + i * 110, 10.5, 160, 15, 12, "s", LOG_INK, 1))
    k.append(frame("OZG_DSwLegend", exact(18, 300, SWAP_W - 36, 36), 3, leg))
    # what will be done, and the two buttons
    k += [text("OZG_DSwSum0", 18, 350, 520, 19, 14, "s", MUTED, 3),
          text("OZG_DSwSum1", 18, 369, 520, 19, 14, "s", MUTED, 3),
          btn("OZG_DSwCancel", 460, 340, 110, 42, 4, BTN, BTN_EDGE, "#STR_OZG_BWIN_CANCEL", 14, "s", INK, prio=6),
          btn("OZG_DSwGo", 582, 340, 120, 42, 4, SL_BTN, None, "#STR_OZG_DWIN_SWAP_GO", 15, "b", FAINT, prio=6)]
    # the dim: the canvas's rgba(8, 5, 3, .55) leaves 45 percent; the engine blends in
    # linear light, where the alpha that leaves as much is 1 - (1 - a) ^ 2.2
    d = [panel("OZG_DSwDim", exact(0, 0, w, h), "0.031 0.02 0.012 0.83", 0),
         # a press beside the box shuts nothing and plays nothing through it
         button("OZG_DSwBack", 0, 0, w, h, 1, None),
         frame("OZG_DSwPane", exact((w - SWAP_W) / 2.0, (h - 400) / 2.0, SWAP_W, 400), 2, k)]
    return frame("OZG_DSwapDlg", exact(x, y, w, h), 85, d, [("visible", 0)])


def swap_row():
    """A die he carries, a row of the swap's window (the canvas: 58 high, a ring of one --
    gold when it is the one picked): its picture, its name by its colour, what he knows of
    it; loaded: how heavy, as bars and a word."""
    n = "OZG_DSRow"
    kids = [box(n + "Box", 0, 0, 384, 58, 6, "#2a1d12", "#4a3726", prio=0),
            image(n + "Sh", exact(12, 12, 40, 40), 1, None),
            image(n + "Pic", exact(12, 9, 40, 40), 2, None),
            text(n + "Title", 64, 11.5, 250, 17, 14, "b", INK, 2),
            text(n + "Sub", 64, 31.5, 300, 15, 12, "s", MUTED, 2),
            image(n + "Bars", exact(353, 12.5, 19, 16), 2, None, extra=[("visible", 0)]),
            text(n + "Word", 252, 32.5, 120, 14, 11, "s", "#c9d3dd", 2, "right")]
    attrs = exact(0, 0, 384, 58) + [("text", '""'), ("priority", 6), ("style", "Empty"), ("clipchildren", 0)]
    return block("ButtonWidgetClass", n, attrs, kids)


def inherit(node):
    """`inheritalpha 1` on the node and all it holds: a part of a die or of a can follows
    the alpha the script gives the whole."""
    kind, name, attrs, kids = node
    if not any(key == "inheritalpha" for key, _ in attrs):
        attrs = [("inheritalpha", 1)] + list(attrs)
    return (kind, name, attrs, [inherit(c) for c in kids] if kids else kids)


def pic(name, prio, texture=None, color="1 1 1 1"):
    """A picture the script places and sizes in pixels."""
    return inherit(image(name, exact(0, 0, 10, 10), prio, texture, color))


def die_actor():
    """A die of a dice table (the canvas's "Кістка"): a frame that turns whole (a turned
    picture is cut to its own box, a turned frame's children are not: dayz-modding
    gui-layouts.md) holding the ring of a die picked, its shadow, the die, and over its
    corners what this player knows of it -- the load as a tiny die with the loaded face and
    one to three bars, an eye for a die he saw slipped in. The script sizes the frame and
    lays its parts in pixels about its middle (OZG_GameMenuDiceTable.c)."""
    load = inherit(frame("OZG_DDieLoad", exact(0, 0, 10, 10), 3, [
        pic("OZG_DDieLoadBg", 0), pic("OZG_DDieLoadFace", 1), pic("OZG_DDieLoadBars", 1)], [("visible", 0)]))
    eye = inherit(frame("OZG_DDieEye", exact(0, 0, 10, 10), 3, [
        pic("OZG_DDieEyeRing", 0, "ozg_disc_ca", rgb("#1a120a")),
        pic("OZG_DDieEyeBg", 1, "ozg_disc_ca", rgb("#ff8a3d")),
        pic("OZG_DDieEyeIcon", 2, "ozg_eye_ca")], [("visible", 0)]))
    kids = [pic("OZG_DDieRing", 0), pic("OZG_DDieSh", 1), pic("OZG_DDiePic", 2), load, eye]
    attrs = [("ignorepointer", 1), ("clipchildren", 0)] + exact(0, 0, 80, 80) + [("priority", 30)]
    return block("FrameWidgetClass", "OZG_DDie", attrs, kids)


def can_actor():
    """A can of dice at a seat (the canvas's "Банка"): what stays on the felt under it (the
    ring of whose throw it is, the pulse of a swap seen, its shadow) and the can itself in
    a frame that turns whole -- its body, the dice lying in it (seven places, each a
    picture of one die in the can's mouth, coloured by the script)."""
    turn = [pic("OZG_DCanBody", 0)]
    for i in range(7):
        turn.append(pic("OZG_DCanIn%d" % i, 1, "ozg_can_in%d_ca" % i))
    kids = [pic("OZG_DCanGlow", 0, "ozg_can_glow_ca", rgb(GOLD)),
            pic("OZG_DCanPulse", 1, "ozg_can_pulse_ca", rgb("#ff8a3d")),
            pic("OZG_DCanShadow", 2, "ozg_can_shadow_ca"),
            inherit(frame("OZG_DCanTurn", exact(0, 0, 10, 10), 3, turn))]
    attrs = [("ignorepointer", 1), ("clipchildren", 0)] + exact(0, 0, 80, 100) + [("priority", 20)]
    return block("FrameWidgetClass", "OZG_DCan", attrs, kids)


def throw_can(x, y):
    """Nardy's can in the throw's panel: a button the size of the can drawn 1.1 times
    (84 x 101); the script lays the can itself over it."""
    return button("OZG_BCanHit", x, y, 84, 101, 7, None)


# ---------------------------------------------------------------- the window

def window_dice():
    """The dice games' window: the header (the game, its rules, the game's button and whose
    dice); the felt tray; the right column with the score, the action panel between games,
    the swap's button and the log. Each game's own parts are its face (dice_game_faces)."""
    k = [panel("OZG_DBack", exact(0, 0, 1600, 900), rgb("#0b0a08"), 0),
         box("OZG_DBox", 40, 28, 1520, 844, 8, "#15100b", "#4a3726", outside=True, prio=1),
         shape("OZG_DHead", 40, 28, 1520, 46, 8, rgb("#1c140d"), 2),
         panel("OZG_DHeadFoot", exact(40, 50, 1520, 23), rgb("#1c140d"), 2),
         panel("OZG_DHeadLine", exact(40, 73, 1520, 1), rgb("#8a6a3e"), 3),
         text("OZG_DTitle", 60, 35, 420, 31, 23, "p", INK, 5),
         text("OZG_DSub", 200, 42.5, 760, 16, 13, "s", MUTED, 5),
         panel("OZG_DSep", exact(1250, 37.5, 1, 26), rgb("#4a3726"), 5),
         btn("OZG_DBtnGame", 1265, 35.5, 160, 30, 4, BTN, BTN_EDGE, "", 13, "s", INK, prio=6),
         btn("OZG_DBtnDice", 1375.5, 35.5, 132.5, 30, 4, "#241a12", BTN_EDGE, "", 13, "s", MUTED, prio=6),
         button("OZG_DBtnClose", 1518, 35.5, 32, 30, 30, [
             box("OZG_DBtnCloseBox", 0, 0, 32, 30, 4, CLOSE, prio=0),
             image("OZG_DBtnCloseX", exact(10, 9, 12, 12), 2, "ozg_icon_x_ca", rgb(CLOSE_INK))])]
    # the tray: its felt, the plates of the game's face (made in by the script), what the
    # script lays on the felt -- the places of the cans, the cans and the dice --, the
    # button that takes its clicks
    tray = [shape("OZG_DTrayBg", 0, 0, 1100, 774, 6, rgb("#120c08"), 0),
            image("OZG_DTrayPic", exact(0, 0, 1100, 774), 1, None),
            frame("OZG_DField", exact(0, 0, 1100, 774), 20),
            frame("OZG_DTable", exact(0, 0, 1100, 774), 22),
            text("OZG_DTHint", 300, 362, 500, 20, 16, "s", HINT, 6, "center"),
            toast(370, 76),
            button("OZG_DHit", 0, 0, 1100, 774, 30, None)]
    k.append(frame("OZG_DTray", exact(52, 86, 1100, 774), 4, tray, clip=1))

    # the right column
    col = [box("OZG_DColBox", 0, 0, 384, 774, 6, "#1e1610", "#3d2d1d", prio=0)]
    sc = [shape("OZG_DScoreBg", 0, 0, 360, 130, 6, rgb(SOFT), 0),
          text("OZG_DScoreTitle", 12, 10, 336, 14, 11, "s", GOLD, 2, "left", "")]
    for i in range(6):
        y = 30 + i * 32
        sc += [shape("OZG_DScoreRow%d" % i, 12, y, 336, 26, 4, rgb("#3d2f1d"), 1, extra=[("visible", 0)]),
               text("OZG_DScoreName%d" % i, 18, y + 4.5, 176, 17, 14, "s", INK, 2),
               text("OZG_DScoreMe%d" % i, 18, y + 4.5, 176, 17, 14, "b", INK, 2),
               text("OZG_DScoreNote%d" % i, 202, y + 4.5, 76, 17, 12, "s", MUTED, 2),
               text("OZG_DScoreTotal%d" % i, 286, y + 3.5, 56, 19, 15, "b", INK, 2, "right")]
    col.append(frame("OZG_DScore", exact(12, 12, 360, 130), 5, sc))
    act = [box("OZG_DActBox", 0, 0, 360, 150, 6, PANEL, GOLD, prio=0),
           text("OZG_DActTitle", 12, 12, 200, 22, 17, "p", INK, 2, "left", ""),
           text("OZG_DActInfo", 150, 15.5, 198, 15, 12, "s", MUTED, 2, "right")]
    for i in range(16):
        act.append(btn("OZG_DAct%d" % i, 12, 44, 336, 42, 4, SL_BTN, BTN_EDGE, "", 15, "b", INK, prio=6, visible=0))
    act.append(frame("OZG_DAmount", exact(12, 44, 336, 42), 6, [
        box("OZG_DAmountBox", 0, 0, 120, 42, 4, "#120c08", BTN_EDGE, prio=0),
        ("EditBoxWidgetClass", "OZG_DAmountEdit", exact(6, 6, 108, 30) + [
            ("color", rgb(INK)), ("priority", 2), ("style", "Editor"),
            ("font", '"gui/fonts/sdf_MetronLight24"'), ("exact text", 1), ("exact text size", 18),
            ("text halign", "center")], None),
        btn("OZG_DAmountBtn", 128, 0, 208, 42, 4, GOLD, None, "", 15, "b", "#1a120a", prio=3)], [("visible", 0)]))
    col.append(frame("OZG_DAct", exact(12, 152, 360, 150), 6, act))
    lg = [shape("OZG_DLogBg", 0, 0, 360, 265, 6, rgb(SOFT), 0),
          text("OZG_DLogTitle", 12, 10, 336, 14, 11, "s", GOLD, 2, "left", "")]
    for i in range(LOG_LINES):
        lg.append(text("OZG_DLog%d" % i, 12, 28 + i * 21, 336, 17, 13, "s", LOG_INK, 2, "left", "", [("visible", 0)]))
    col.append(frame("OZG_DLog", exact(12, 497, 360, 265), 5, lg, clip=1))
    col.append(swap_button(24, 400))
    k.append(frame("OZG_DCol", exact(1164, 86, 384, 774), 6, col))
    k.append(swap_dialog(52, 86, 1100, 774))
    k += dialogs(menu_y=70)
    attrs = [("ignorepointer", 1), ("clipchildren", 0), ("position", "0 0"), ("size", "1600 900"),
             ("halign", "center_ref"), ("valign", "center_ref"),
             ("hexactpos", 1), ("vexactpos", 1), ("hexactsize", 1), ("vexactsize", 1), ("priority", 3)]
    return block("FrameWidgetClass", "OZG_DWin", attrs, k)


# ---------------------------------------------------------------- textures

def hexa(code, alpha=255):
    h = code.lstrip("#")
    return (int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16), alpha)


def chip_icon(size=128, ss=4):
    """The pot's chip (the canvas's svg of 30: a red disc of 12 in a gold ring of 2, a
    dashed gold ring of 6 inside)."""
    n = size * ss
    k = n / 30.0
    img = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    c = n / 2.0
    gold = (231, 194, 122, 255)
    d.ellipse((c - 13 * k, c - 13 * k, c + 13 * k, c + 13 * k), fill=gold)
    d.ellipse((c - 11 * k, c - 11 * k, c + 11 * k, c + 11 * k), fill=(124, 50, 38, 255))
    # the dashed ring: dashes of 3 and gaps of 3 along a circle of 6
    turns = 2 * math.pi * 6
    dashes = int(turns // 6)
    for i in range(dashes):
        a0 = i * 6 / 6.0 * 360 / (turns / 6.0)
        a1 = a0 + 3 / 6.0 * 360 / (turns / 6.0)
        d.arc((c - 7 * k, c - 7 * k, c + 7 * k, c + 7 * k), a0, a1, fill=gold, width=int(2 * k))
    return img.resize((size, size), LANCZOS)


def arc_low(cx, cy, rx, ry, x0, x1, steps=64):
    """The lower half of an ellipse from x0 to x1: the points of a can's round edge."""
    pts = []
    for i in range(steps + 1):
        x = x0 + (x1 - x0) * i / float(steps)
        u = max(-1.0, min(1.0, (x - cx) / rx))
        pts.append((x, cy + ry * math.sqrt(1 - u * u)))
    return pts


CAN_BOX = 92.0     # a can's picture: 92 units square, the can's own box (76 x 92) 8 in


def can_picture(army, closed, size=512, ss=3):
    """The can as the canvas draws it (its svg of 76 x 92 and the mouth over it): a tin
    with a paper label, or the army's green one; open with its dark inside, or turned over
    -- its bottom up -- for perudo."""
    n = size * ss
    k = n / CAN_BOX
    img = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    mid, dark, light, edge = "#a9adb0", "#7b7f83", "#d3d6d8", "#5c6064"
    label, label_dark, ink, rim, top = "#b4975a", "#8f7743", "#5a4322", "#dfe2e4", "#b9bcbf"
    if army:
        mid, dark, light, edge = "#5d6a4a", "#47523a", "#74825e", "#36402c"
        label, label_dark, ink, rim, top = "#4c573c", "#3b4530", "#c9c19a", "#8a9670", "#66734f"

    def P(points):
        return [((8 + x) * k, y * k) for x, y in points]

    def strip(x0, x1, y_top, cy, fill):
        d.polygon(P([(x0, y_top), (x1, y_top)] + list(reversed(arc_low(38, cy, 32, 9, x0, x1)))), fill=fill)

    # the body down to its round foot, its shaded sides, the light down its front
    strip(6, 70, 16, 78, mid)
    strip(6, 20, 16, 78, dark)
    strip(56, 70, 16, 78, dark)
    strip(27, 36, 16, 78, light)
    # the label: a band between two round edges, darker at the sides

    def band(x0, x1, fill):
        d.polygon(P(arc_low(38, 34, 32, 9, x0, x1) + list(reversed(arc_low(38, 62, 32, 9, x0, x1)))), fill=fill)

    band(6, 70, label)
    band(6, 20, label_dark)
    band(56, 70, label_dark)
    for cy, wd in ((27, 1.2), (69, 1.2), (78, 1.6)):
        d.line(P(arc_low(38, cy, 32, 9, 6, 70)), fill=edge, width=max(1, int(round(wd * k))), joint="curve")
    for x0, x1, y in ((24, 52, 47), (24, 46, 53)):
        w = 2.4 * k
        d.line(P([(x0, y), (x1, y)]), fill=ink, width=int(round(w)))
        for x in (x0, x1):
            d.ellipse(((8 + x) * k - w / 2, y * k - w / 2, (8 + x) * k + w / 2, y * k + w / 2), fill=ink)
    # the mouth: a rim of two round what is inside
    d.ellipse(((8 + 6) * k, 7 * k, (8 + 70) * k, 25 * k), fill=rim)
    inside = top if closed else "#17110c"
    d.ellipse(((8 + 8) * k, 9 * k, (8 + 68) * k, 23 * k), fill=inside)
    if closed:
        x0, y0, w, h = 8 + 8 + 60 * 0.18, 9 + 14 * 0.22, 60 * 0.64, 14 * 0.56
        d.ellipse((x0 * k, y0 * k, (x0 + w) * k, (y0 + h) * k), outline=edge, width=max(1, int(round(k))))
    return img.resize((size, size), LANCZOS)


# The dice lying in a can's mouth (the canvas: squares of 9, turned, cut by the mouth):
# the middle of each in the can's units, and its turn.
CAN_SPOTS = ((25.5, 18.5, -8), (38.5, 22.5, 10), (50.5, 17.5, 4), (32.5, 12.5, 14), (45.5, 11.5, -12),
             (20.5, 13.5, 6), (56.5, 21.5, -5))
CAN_IN = (6.0, 0.0, 64.0, 32.0)    # the part of a can's box their pictures cover


def can_inside(i, size=(256, 128), ss=4):
    """One die lying in the can's mouth: white, to be coloured by the script, with its
    shadow under it (black stays black under any colour), cut by the mouth's inner edge."""
    w, h = size[0] * ss, size[1] * ss
    kx, ky = w / CAN_IN[2], h / CAN_IN[3]
    cx, cy, turn = CAN_SPOTS[i]
    a = math.radians(turn)
    pts = []
    for dx, dy in ((-4.5, -4.5), (4.5, -4.5), (4.5, 4.5), (-4.5, 4.5)):
        x = cx + dx * math.cos(a) - dy * math.sin(a)
        y = cy + dx * math.sin(a) + dy * math.cos(a)
        pts.append(((x - CAN_IN[0]) * kx, (y - CAN_IN[1]) * ky))
    square = Image.new("L", (w, h), 0)
    ImageDraw.Draw(square).polygon(pts, fill=255)
    sh = Image.new("L", (w, h), 0)
    sh.paste(square, (0, int(round(ky))))
    sh = sh.filter(ImageFilter.GaussianBlur(0.5 * ky)).point(lambda v: int(v * 0.5))
    mouth = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mouth).ellipse(((8 - CAN_IN[0]) * kx, 9 * ky, (68 - CAN_IN[0]) * kx, 23 * ky), fill=255)
    out = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    black = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    black.putalpha(sh)
    out = Image.alpha_composite(out, black)
    white = Image.new("RGBA", (w, h), (255, 255, 255, 255))
    white.putalpha(square)
    out = Image.alpha_composite(out, white)
    alpha = out.getchannel("A")
    out.putalpha(ImageChops.multiply(alpha, mouth))
    return out.resize(size, LANCZOS)


CAN_GLOW = (-18.0, 50.0, 112.0, 62.0)     # the can's units the glow's picture covers
CAN_PULSE = (-2.0, 66.0, 80.0, 30.0)
CAN_SHADOW = (-8.0, 64.0, 96.0, 40.0)


def ellipse_mask(area, rect, size, ss, grow=0.0):
    w, h = size[0] * ss, size[1] * ss
    kx, ky = w / area[2], h / area[3]
    m = Image.new("L", (w, h), 0)
    x, y, ew, eh = rect
    ImageDraw.Draw(m).ellipse(((x - grow - area[0]) * kx, (y - grow - area[1]) * ky,
                               (x + ew + grow - area[0]) * kx, (y + eh + grow - area[1]) * ky), fill=255)
    return m, kx, ky


def can_glow(drawn=1.0, size=(512, 256), ss=2):
    """Whose throw it is: a ring of two round the can's foot and its glow (the canvas:
    box-shadow 0 0 0 2px, 0 0 14px 2px on an ellipse of 72 x 22, 85 percent), white. The
    canvas's shadow keeps its two and fourteen units whatever the can's size, so the
    picture is made for the size the can is `drawn` at (1.5 at the tables of three, 1.25
    at those of four; one more serves every smaller can). The engine blends in linear
    light, where a light layer over the felt is as strong as the canvas's at its alpha to
    the power of 1.5 (dayz-modding gui-layouts.md, "A see-through layer")."""
    foot = (2.0, 70.0, 72.0, 22.0)
    inner, kx, ky = ellipse_mask(CAN_GLOW, foot, size, ss)
    outer, _, _ = ellipse_mask(CAN_GLOW, foot, size, ss, grow=2.0 / drawn)
    glow = outer.filter(ImageFilter.GaussianBlur(7 / drawn * (kx + ky) / 2.0))
    a = ImageChops.lighter(glow, outer)
    a = ImageChops.multiply(a, inner.point(lambda v: 255 - v))
    a = a.point(lambda v: int(round(255 * (v / 255.0 * 0.85) ** 1.5)))
    out = Image.new("RGBA", a.size, (255, 255, 255, 0))
    out.putalpha(a)
    return out.resize(size, LANCZOS)


def can_pulse(size=(256, 128), ss=4):
    """A swap seen: a ring round the can's foot, which the script grows and fades (the
    canvas: three units at any size of the can; two of the can's own here, which is three
    at the tables of three)."""
    foot = (2.0, 70.0, 72.0, 22.0)
    inner, _, _ = ellipse_mask(CAN_PULSE, foot, size, ss)
    outer, _, _ = ellipse_mask(CAN_PULSE, foot, size, ss, grow=2.0)
    a = ImageChops.multiply(outer, inner.point(lambda v: 255 - v))
    out = Image.new("RGBA", a.size, (255, 255, 255, 0))
    out.putalpha(a)
    return out.resize(size, LANCZOS)


def can_shadow(size=(256, 128), ss=2):
    """The can's shadow on the felt (the canvas: an ellipse of 72 x 16, 45 percent black,
    blurred 4)."""
    m, kx, ky = ellipse_mask(CAN_SHADOW, (4.0, 76.0, 72.0, 16.0), size, ss)
    a = m.filter(ImageFilter.GaussianBlur(4 * (kx + ky) / 2.0)).point(lambda v: int(v * 0.45))
    out = Image.new("RGBA", a.size, (0, 0, 0, 0))
    out.putalpha(a)
    return out.resize(size, LANCZOS)


def seat_spot(size=(256, 128), ss=4):
    """Where the table's one can stands when it is another's turn (the canvas: an ellipse of
    98 x 28, rgba(8, 5, 3, .18) with an inset ring of one, rgba(239, 228, 204, .16)). The
    engine blends in linear light: a dark layer wants 1 - (1 - a) ^ 2.2 to darken as much,
    a light one far less than its number (dayz-modding gui-layouts.md §39)."""
    area = (-1.0, -1.0, 100.0, 30.0)
    whole, _, _ = ellipse_mask(area, (0.0, 0.0, 98.0, 28.0), size, ss)
    inner, _, _ = ellipse_mask(area, (1.0, 1.0, 96.0, 26.0), size, ss)
    ring = ImageChops.multiply(whole, inner.point(lambda v: 255 - v))
    out = Image.new("RGBA", whole.size, (8, 5, 3, 0))
    out.putalpha(whole.point(lambda v: int(v * 0.35)))
    light = Image.new("RGBA", whole.size, (239, 228, 204, 0))
    light.putalpha(ring.point(lambda v: int(v * 0.16)))
    out = Image.alpha_composite(out, light)
    return out.resize(size, LANCZOS)


PIPS = {0: (), 1: (4,), 2: (0, 8), 3: (0, 4, 8), 4: (0, 2, 6, 8), 5: (0, 2, 4, 6, 8), 6: (0, 2, 3, 5, 6, 8)}


def mini_die(face, size=64, ss=4):
    """The loaded face in a load's badge (the canvas: a die of 16, #e4e9ee, its pips of 3
    in #1d232a)."""
    n = size * ss
    k = n / 16.0
    img = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle((0, 0, n - 1, n - 1), radius=2 * k, fill=hexa("#e4e9ee"))
    for c in PIPS[face]:
        cx = (2 + 2 + 4 * (c % 3)) * k
        cy = (2 + 2 + 4 * (c // 3)) * k
        d.ellipse((cx - 1.5 * k, cy - 1.5 * k, cx + 1.5 * k, cy + 1.5 * k), fill=hexa("#1d232a"))
    return img.resize((size, size), LANCZOS)


def badge_bg(w, h, r, size, ss=4):
    """A load's badge: a pill of #4d5661 in a ring of two, #1a120a (the picture holds the
    ring: `w` + 4 by `h` + 4 units)."""
    # drawn square to the unit and then stretched to the picture: the corners stay round
    # on the screen, where the picture is laid `w` + 4 by `h` + 4 again
    sq = Image.new("RGBA", (int((w + 4) * 16 * ss), int((h + 4) * 16 * ss)), (0, 0, 0, 0))
    s = 16.0 * ss
    q = ImageDraw.Draw(sq)
    q.rounded_rectangle((0, 0, sq.width - 1, sq.height - 1), radius=(r + 2) * s, fill=hexa("#1a120a"))
    q.rounded_rectangle((2 * s, 2 * s, sq.width - 1 - 2 * s, sq.height - 1 - 2 * s), radius=r * s, fill=hexa("#4d5661"))
    return sq.resize(size, LANCZOS)


def bars(step, widths, heights, gap, lit, off, size=64, ss=4):
    """How heavy a load is: three bars, the first `step` lit."""
    total_w = 3 * widths + 2 * gap
    total_h = heights[2]
    s = 16 * ss
    img = Image.new("RGBA", (int(total_w * s), int(total_h * s)), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    for i in range(3):
        x0 = i * (widths + gap) * s
        fill = hexa(lit) if step > i else hexa(off)
        d.rectangle((x0, (total_h - heights[i]) * s, x0 + widths * s - 1, total_h * s - 1), fill=fill)
    return img.resize((size, size), LANCZOS)


def bezier(p0, p1, p2, p3, steps=24):
    pts = []
    for i in range(steps + 1):
        t = i / float(steps)
        u = 1 - t
        pts.append((u ** 3 * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t ** 3 * p3[0],
                    u ** 3 * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t ** 3 * p3[1]))
    return pts


def eye_icon(size=64, ss=8):
    """The eye of a die he saw slipped in (the canvas's svg of 16: its outline of 1.6, the
    pupil of 2.3, #1a120a)."""
    n = size * ss
    k = n / 16.0
    img = Image.new("RGBA", (n, n), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    ink = hexa("#1a120a")
    path = (bezier((1.5, 8), (3.5, 4.6), (5.6, 3.3), (8, 3.3)) + bezier((8, 3.3), (10.4, 3.3), (12.5, 4.6), (14.5, 8)) +
            bezier((14.5, 8), (12.5, 11.4), (10.4, 12.7), (8, 12.7)) + bezier((8, 12.7), (5.6, 12.7), (3.5, 11.4), (1.5, 8)))
    # the outline runs on a little past its start: a line's two ends are not joined, and
    # the eye's left corner showed a notch
    path = path + path[1:4]
    d.line([(x * k, y * k) for x, y in path], fill=ink, width=int(round(1.6 * k)), joint="curve")
    d.ellipse(((8 - 2.3) * k, (8 - 2.3) * k, (8 + 2.3) * k, (8 + 2.3) * k), fill=ink)
    return img.resize((size, size), LANCZOS)


def dice_textures(paa):
    save(chip_icon(), "ozg_chip_ca", paa)
    for army in (False, True):
        for closed in (False, True):
            stem = "ozg_can_%s_%s_ca" % ("army" if army else "tin", "closed" if closed else "open")
            save(can_picture(army, closed), stem, paa)
    for i in range(len(CAN_SPOTS)):
        save(can_inside(i), "ozg_can_in%d_ca" % i, paa)
    save(can_glow(0.85), "ozg_can_glow_ca", paa)
    save(can_glow(1.25), "ozg_can_glow125_ca", paa)
    save(can_glow(1.5), "ozg_can_glow15_ca", paa)
    save(can_pulse(), "ozg_can_pulse_ca", paa)
    save(can_shadow(), "ozg_can_shadow_ca", paa)
    save(seat_spot(), "ozg_seat_spot_ca", paa)
    for f in range(1, 7):
        save(mini_die(f), "ozg_mini_%d_ca" % f, paa)
    # a load's badge by the die's size: with bars (74 and more: 38 x 24; under 60: 30 x 18),
    # without (24 x 24, 20 x 18), and the small die's (14 x 14, its corners of 4)
    save(badge_bg(38, 24, 6, (128, 64)), "ozg_dbadge_l_ca", paa)
    save(badge_bg(24, 24, 6, (64, 64)), "ozg_dbadge_ln_ca", paa)
    save(badge_bg(30, 18, 6, (128, 64)), "ozg_dbadge_m_ca", paa)
    save(badge_bg(20, 18, 6, (64, 64)), "ozg_dbadge_mn_ca", paa)
    save(badge_bg(14, 14, 4, (64, 64)), "ozg_dbadge_s_ca", paa)
    for step in range(1, 4):
        save(bars(step, 3, (6, 10, 14), 1, "#eef2f6", "#2b323a"), "ozg_dbars_%d_ca" % step, paa)
        save(bars(step, 5, (7, 11, 16), 2, "#c9d3dd", "#3a342c"), "ozg_sbars_%d_ca" % step, paa)
    save(eye_icon(), "ozg_eye_ca", paa)


def write_layouts():
    write("ozg_dwin.layout", window_dice())
    write("ozg_ddie.layout", die_actor())
    write("ozg_dcan.layout", can_actor())
    write("ozg_dsrow.layout", swap_row())
    for name, body in dice_game_faces():
        write(name, body)


def main():
    paa = IMAGE_TO_PAA
    if "--no-paa" in sys.argv or not os.path.isfile(paa):
        paa = None
    if "--layouts" not in sys.argv:
        dice_textures(paa)
    write_layouts()


if __name__ == "__main__":
    main()
