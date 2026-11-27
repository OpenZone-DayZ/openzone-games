# Where the sounds come from

`tools/make_sounds.py` makes every sound in `OpenZone_Games/sounds` from numbers:
white noise shaped into the bands a card makes, a few damped low tones for cards
landing on cloth, envelopes and timings written in the script, and fixed seeds,
so that every run gives the same files. Nothing was recorded for them and they
contain no audio of anybody else's: no sample, no sound library, no download.
They are the mod's own, under its licence.

The script writes 16-bit mono WAV files at 44.1 kHz into `build/sounds` (outside
git) and encodes each with ffmpeg's `libvorbis`, quality 4, with ffmpeg's
bit-exact flags, into the `.ogg` the mod carries. The design gives a sound
15 m (design §17); the loudness of each is the `volume` of its shader in the
script's table.

The six sounds of the sleights (card sharp's design §6.1: `slip`, `swap`, `toss`,
`deckin`, `deckout`, `pick`, added 2026-11-25) are made the same way and are
quieter (`volume` 0.30): each is played on the client of the one who does the
sleight and on the clients of those who saw it, never through the place.

The seven sounds of the boards and the dice (boards and dice design §15: `piece`,
`capture`, `crown`, `check`, `roll`, `over`, `click`, added 2026-11-26) are the
design canvas's mockups' own, made the same way: the mockups build them in the
browser out of band-passed noise bursts and short oscillator tones, and the script
builds them out of the same numbers -- the same filter (the browser's band-pass
biquad), the same envelopes, the same frequencies and timings. All but `click` are
heard on the place, as the cards are; `click`, the card sharp's kit at work, only on
the client of the one who works it.

The three sounds of the dice tables (`rattle`, `land`, `dieslip`, added 2026-11-27) are
the redone canvas's own (its boards of that day), made the same way out of its numbers:
the dice rattling in a can shaken (thirteen clacks), the dice landing on the felt, a die
clacking into a can. None of them is heard through the place. The window of a client plays
`rattle` and `land` with the throw it shows -- the can shaken, the dice flying out -- in
place of the place's `roll`, which that client then leaves unplayed; `dieslip` sounds on
the clients that see a die slipped in (the one who does it and those who noticed), as the
die gets to the can.
