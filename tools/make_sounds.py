# Makes the sounds of the table (design §17) from numbers: a riffle shuffle, a
# deal's quick run, the trump turned up, a card laid on the table, a card slid
# over another, the table taken, a bout swept to the discards, cards drawn, the
# deck put down on the mat and picked up. Every sound is shaped noise and a few
# damped tones, drawn with a fixed seed: the same files every run. No recording
# and nobody else's sound is in them (art/sounds/SOURCES.md).
#
# Usage: python -I tools/make_sounds.py [--ffmpeg <ffmpeg.exe>] [--root <dir>]
#
# Writes build/sounds/<name>.wav (16-bit mono, 44.1 kHz) and, through ffmpeg's
# libvorbis, OpenZone_Games/sounds/ozg_<name>.ogg; then
# OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_SoundIds.c (an id, a name, a
# sound set and a length a sound) and the text between the markers
# "// >>> generated: sounds" and "// <<< generated: sounds" in
# OpenZone_Games/config.cpp (CfgSoundShaders and CfgSoundSets). --root writes
# all of it under another folder of the same layout instead (a check).
import argparse
import hashlib
import io
import os
import subprocess
import wave

import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
FFMPEG = r"E:\Programs\ffmpeg\bin\ffmpeg.exe"

SR = 44100          # samples a second
PEAK = 0.5          # every sound is brought to this peak, -6 dBFS: card sounds are quiet
TAIL_DB = -60.0     # a sound ends where it has fallen this far under its peak
RANGE = 15          # metres a sound carries: the design's number (§17), not the owner's

# The sounds in the order of their ids (OZG_SoundIds.c): the name, its part
# of the class names, the seed, the volume of its shader, what it is.
SOUNDS = [
    ("shuffle", "Shuffle", 1101, 0.45, "a riffle shuffle: riffled, bridged, squared"),
    ("deal", "Deal", 1102, 0.40, "a deal: a quick run of cards flicked onto the cloth"),
    ("trump", "Trump", 1103, 0.35, "the trump turned up"),
    ("lay", "Lay", 1104, 0.40, "a card laid on the table"),
    ("beat", "Beat", 1105, 0.40, "a card slid over another to beat it"),
    ("take", "Take", 1106, 0.40, "the table gathered into the defender's hand"),
    ("sweep", "Sweep", 1107, 0.40, "a bout swept off to the discards"),
    ("draw", "Draw", 1108, 0.35, "cards drawn from the stock"),
    ("putdown", "PutDown", 1109, 0.45, "the deck put down on the mat"),
    ("pickup", "PickUp", 1110, 0.40, "the deck picked up"),
]


# ---------------------------------------------------------------- what sounds are made of

def n_of(ms):
    """Samples in `ms` milliseconds."""
    return int(round(ms * SR / 1000.0))


def band(rng, n, lo, hi):
    """n samples of white noise kept between lo and hi Hz, each edge falling off
    over a third of an octave; scaled to a peak of 1."""
    size = max(n, 64)
    spec = np.fft.rfft(rng.standard_normal(size))
    octave = np.log2(np.maximum(np.fft.rfftfreq(size, 1.0 / SR), 1.0))
    rise = np.clip((octave - np.log2(lo)) / 0.33 + 1.0, 0.0, 1.0)
    fall = np.clip((np.log2(hi) - octave) / 0.33 + 1.0, 0.0, 1.0)
    shaped = np.fft.irfft(spec * rise * rise * fall * fall, size)[:n]
    return shaped / (np.max(np.abs(shaped)) + 1e-12)


def snap(rng, lo, hi, decay_ms):
    """An edge or a face meeting a card or the cloth: a burst of noise that
    starts at once and falls quickly."""
    n = n_of(decay_ms * 7)
    t = np.arange(n) * 1000.0 / SR
    start = np.minimum(t / 0.5, 1.0)
    return band(rng, n, lo, hi) * start * np.exp(-t / decay_ms)


def rub(rng, ms, lo, hi, top=0.4):
    """A card sliding: soft noise that swells to its top at `top` of the way and
    fades, a little uneven, as a hand is."""
    n = n_of(ms)
    x = np.linspace(0.0, 1.0, n)
    up = np.sin(0.5 * np.pi * np.minimum(x / top, 1.0))
    down = np.cos(0.5 * np.pi * np.clip((x - top) / (1.0 - top), 0.0, 1.0))
    k = max(1, n_of(6))
    wobble = np.convolve(rng.standard_normal(n), np.ones(k) / k, mode="same")
    wobble = 1.0 + 0.6 * wobble / (np.max(np.abs(wobble)) + 1e-12)
    return band(rng, n, lo, hi) * up * down * wobble


def knock(rng, hz, decay_ms):
    """Cards landing on cloth: a low, short knock and a breath of noise."""
    n = n_of(decay_ms * 6)
    t = np.arange(n) / SR
    tone = np.sin(2.0 * np.pi * hz * t) * np.exp(-t * 1000.0 / decay_ms)
    breath = band(rng, n, 120.0, 900.0) * np.exp(-t * 1000.0 / (decay_ms * 0.5))
    return 0.8 * tone + 0.35 * breath


def put(out, sound, at_ms, gain):
    """`sound` added into `out` from `at_ms` on."""
    i = n_of(at_ms)
    j = min(len(out), i + len(sound))
    if j > i:
        out[i:j] += gain * sound[:j - i]


def flick(rng, out, at, gain):
    """One card dealt: it leaves the hand with a short rub and lands on the
    cloth with a slap."""
    put(out, rub(rng, rng.uniform(38, 52), 2000, 7000, 0.3), at, 0.35 * gain)
    land = at + rng.uniform(40, 50)
    put(out, snap(rng, 700, 4200, rng.uniform(10, 15)), land, gain)
    put(out, knock(rng, rng.uniform(130, 170), 12), land, 0.25 * gain)


# ---------------------------------------------------------------- the sounds

def shuffle(rng):
    out = np.zeros(n_of(1500))
    # The riffle: the cards fall from both thumbs, slowly at either end and
    # quickly in the middle.
    at = 30.0
    while at < 860.0:
        x = (at - 30.0) / 830.0
        put(out, snap(rng, 2500, 9500, rng.uniform(3.5, 6.5)), at, rng.uniform(0.35, 0.8))
        at += (12.0 + 20.0 * (2.0 * x - 1.0) ** 2) * rng.uniform(0.85, 1.15)
    put(out, rub(rng, 830, 3000, 8000, 0.5), 30, 0.10)      # the air between the cards
    put(out, rub(rng, 210, 1500, 6000, 0.3), 880, 0.40)     # the bridge: the cards run together
    put(out, knock(rng, 150, 16), 1080, 0.55)               # squared on the cloth
    put(out, snap(rng, 1200, 6000, 8.0), 1081, 0.35)
    put(out, knock(rng, 175, 12), 1190, 0.35)               # and once more
    put(out, snap(rng, 1200, 6000, 6.0), 1191, 0.25)
    return out


def deal(rng):
    out = np.zeros(n_of(1300))
    at = 20.0
    for _ in range(10):
        flick(rng, out, at, rng.uniform(0.7, 1.0))
        at += rng.uniform(95, 120)
    return out


def trump(rng):
    out = np.zeros(n_of(400))
    put(out, rub(rng, 70, 2000, 8000, 0.6), 0, 0.35)        # slid out from under the stock
    put(out, snap(rng, 3000, 10000, 6.0), 75, 0.70)         # turned over: a crisp flick
    put(out, snap(rng, 800, 4500, 13.0), 150, 0.80)         # laid face up
    put(out, knock(rng, 160, 10), 150, 0.20)
    return out


def lay(rng):
    out = np.zeros(n_of(300))
    put(out, rub(rng, 55, 1500, 6000, 0.35), 0, 0.30)
    put(out, snap(rng, 600, 4000, 17.0), 55, 1.00)
    put(out, knock(rng, 140, 12), 55, 0.30)
    return out


def beat(rng):
    out = np.zeros(n_of(400))
    put(out, rub(rng, 150, 2500, 9500, 0.55), 0, 0.45)      # card on card: brighter and longer
    put(out, snap(rng, 1500, 7000, 9.0), 150, 0.55)         # it settles on the card beneath
    return out


def take(rng):
    out = np.zeros(n_of(650))
    for at in (0.0, 70.0, 150.0):                           # the cards drawn together
        put(out, rub(rng, rng.uniform(80, 110), 1500, 6000, 0.4), at, 0.40)
    put(out, knock(rng, 160, 14), 330, 0.45)                # squared in the hand
    put(out, snap(rng, 1200, 5000, 7.0), 331, 0.30)
    put(out, knock(rng, 185, 10), 410, 0.30)
    return out


def sweep(rng):
    out = np.zeros(n_of(650))
    put(out, rub(rng, 330, 800, 6000, 0.6), 0, 0.55)        # the pile pushed along the cloth
    put(out, knock(rng, 120, 18), 350, 0.45)                # dropped onto the heap
    put(out, snap(rng, 700, 3500, 12.0), 352, 0.35)
    return out


def draw(rng):
    out = np.zeros(n_of(500))
    for at in (0.0, 170.0):
        put(out, rub(rng, 65, 2000, 7000, 0.3), at, 0.35)   # slid off the top of the stock
        put(out, snap(rng, 1500, 6000, 6.0), at + 70, 0.30) # into the hand
    return out


def putdown(rng):
    out = np.zeros(n_of(400))
    put(out, knock(rng, 110, 22), 0, 0.85)                  # the deck on the mat
    put(out, snap(rng, 900, 4000, 10.0), 2, 0.30)
    put(out, rub(rng, 45, 1000, 5000, 0.2), 5, 0.25)        # its edges settle
    return out


def pickup(rng):
    out = np.zeros(n_of(400))
    put(out, rub(rng, 85, 1000, 5000, 0.3), 0, 0.35)        # lifted off the mat
    put(out, knock(rng, 170, 11), 160, 0.35)                # squared in the hand
    put(out, snap(rng, 1200, 5000, 6.0), 161, 0.25)
    put(out, knock(rng, 190, 9), 230, 0.25)
    return out


MAKERS = {"shuffle": shuffle, "deal": deal, "trump": trump, "lay": lay, "beat": beat,
          "take": take, "sweep": sweep, "draw": draw, "putdown": putdown, "pickup": pickup}


def finish(x):
    """Brought to PEAK, ended where it falls under TAIL_DB (and 20 ms more),
    the last 10 ms faded out: what is written."""
    x = x / (np.max(np.abs(x)) + 1e-12) * PEAK
    floor = PEAK * 10.0 ** (TAIL_DB / 20.0)
    loud = np.nonzero(np.abs(x) > floor)[0]
    end = len(x)
    if len(loud):
        end = min(len(x), int(loud[-1]) + n_of(20))
    x = x[:end].copy()
    fade = min(len(x), n_of(10))
    x[len(x) - fade:] *= np.linspace(1.0, 0.0, fade)
    return x


# ---------------------------------------------------------------- files

def write_wav(path, x):
    data = np.clip(np.round(x * 32767.0), -32768, 32767).astype("<i2").tobytes()
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(SR)
        w.writeframes(data)


def encode(ffmpeg, wav_path, ogg_path):
    """Ogg Vorbis through libvorbis, quality 4, with nothing in the file that
    changes from run to run (no encoder version, no random stream number)."""
    subprocess.run([ffmpeg, "-hide_banner", "-loglevel", "error", "-y", "-i", wav_path,
                    "-map_metadata", "-1", "-c:a", "libvorbis", "-q:a", "4",
                    "-fflags", "+bitexact", "-flags:a", "+bitexact", ogg_path], check=True)


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


def config_lines():
    lines = ["class CfgSoundShaders", "{", "    class baseCharacter_SoundShader;"]
    for name, part, _seed, volume, _what in SOUNDS:
        lines += ["    class OZG_%s_SoundShader: baseCharacter_SoundShader" % part,
                  "    {",
                  '        samples[] = {{"OpenZone_Games\\sounds\\ozg_%s", 1}};' % name,
                  "        volume = %.2f;" % volume,
                  "        range = %d;" % RANGE,
                  "    };"]
    lines += ["};", "class CfgSoundSets", "{", "    class baseCharacter_SoundSet;"]
    for _name, part, _seed, _volume, _what in SOUNDS:
        lines += ["    class OZG_%s_SoundSet: baseCharacter_SoundSet" % part,
                  "    {",
                  '        soundShaders[] = {"OZG_%s_SoundShader"};' % part,
                  "    };"]
    lines.append("};")
    return lines


def ids_code(lengths):
    out = ["// GENERATED by tools/make_sounds.py -- edit the script, not this file.",
           "//",
           "// The sounds of the table (design §17), an id each: what a game hears",
           "// (OZG_Game.HearSound), what a place's number carries (OZG_Sounds.PackLot)",
           "// and what a client plays (OZG_Sounds.PlayLocal). Beside each id its name,",
           "// its sound set in config.cpp and its length in milliseconds.",
           "class OZG_SoundIds",
           "{"]
    for i, (name, _part, _seed, _volume, what) in enumerate(SOUNDS, 1):
        out.append("    static const int %s = %d;    // %s" % (name.upper(), i, what))
    out.append("    static const int LAST = %d;" % len(SOUNDS))
    out.append("")
    out.append("    // Beside every id from 0 (none): its name, its set and its length.")
    out.append("    static void Fill(array<string> names, array<string> sets, array<int> lengths)")
    out.append("    {")
    out.append('        names.Insert("");')
    out.append('        sets.Insert("");')
    out.append("        lengths.Insert(0);")
    for (name, part, _seed, _volume, _what), ms in zip(SOUNDS, lengths):
        out.append('        names.Insert("%s");' % name)
        out.append('        sets.Insert("OZG_%s_SoundSet");' % part)
        out.append("        lengths.Insert(%d);" % ms)
    out.append("    }")
    out.append("}")
    return "\n".join(out) + "\n"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ffmpeg", default=FFMPEG)
    parser.add_argument("--root", default=REPO)
    args = parser.parse_args()
    root = os.path.abspath(args.root)
    wavs = os.path.join(root, "build", "sounds")
    oggs = os.path.join(root, "OpenZone_Games", "sounds")
    code = os.path.join(root, "OpenZone_Games", "scripts", "3_Game", "OpenZone_Games", "OZG_SoundIds.c")
    config = os.path.join(root, "OpenZone_Games", "config.cpp")
    for folder in (wavs, oggs, os.path.dirname(code)):
        os.makedirs(folder, exist_ok=True)

    lengths = []
    for name, _part, seed, _volume, _what in SOUNDS:
        x = finish(MAKERS[name](np.random.RandomState(seed)))
        wav_path = os.path.join(wavs, name + ".wav")
        ogg_path = os.path.join(oggs, "ozg_" + name + ".ogg")
        write_wav(wav_path, x)
        encode(args.ffmpeg, wav_path, ogg_path)
        ms = int(round(len(x) * 1000.0 / SR))
        lengths.append(ms)
        digest = hashlib.sha256(io.open(ogg_path, "rb").read()).hexdigest()[:12]
        print("%-8s %5d ms  ogg %6d B  %s" % (name, ms, os.path.getsize(ogg_path), digest))

    with io.open(code, "w", encoding="utf-8", newline="\n") as f:
        f.write(ids_code(lengths))
    text, bom, crlf = read(config)
    text = between(text, "// >>> generated: sounds", "// <<< generated: sounds", config_lines())
    write(config, text, bom, crlf)
    print("%d sounds, %d ms in all; OZG_SoundIds.c and config.cpp written" % (len(SOUNDS), sum(lengths)))


if __name__ == "__main__":
    main()
