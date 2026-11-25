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
