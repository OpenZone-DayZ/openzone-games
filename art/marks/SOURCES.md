# Where the marks come from

`tools/make_marks.py` draws every picture in this folder and in
`OpenZone_Games/gui/imagesets/ozg_marks_ca.paa`: the faults from numbers, on the
backs `art/cards/<style>/back.png` that `tools/make_cards.py` draws, and the
pencil with two fonts of Windows, Segoe Print (the ranks) and Segoe UI Symbol
(the suits), as the table mock-up of 2026-11-24 wrote it. The fonts are not in
this repository; the script reads them from `C:\Windows\Fonts`.

The speck a noticed face shows is drawn from numbers too. Its ninety-nine places
on the faces of a style are found by reading every face
`art/cards/<style>/c_*.png` against the paper round it: a place lies on a line
6 px in from the card's edge, where no face of the style carries ink.
`<style>_specks.png` shows them on one face and on the ink of all the faces.
