# Sounds and flights of cards (design §17, §11) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Every change of a game is heard by every client near the place, players, watchers and passers-by, whether a window is open or not: the deck put down on the mat and gathered up, the shuffle, the deal's quick run, the trump turned up, a card laid on the table and a card laid over it, the table taken or swept to the discards, cards drawn from the stock; the sounds are the mod's own, made by a script from numbers. In the window the cards fly from where they were to where the new view puts them. And the card sharp's fourth phase finds three things ready: a sound played on one client alone, a flight of one card between two places of the window, a small shake of a part of the table.

**Architecture:** The server plays nothing. A game says what each change made heard (`OZG_Game.HearSound`, ids of `OZG_SoundIds`); the session, after every change it accepts, puts that lot on the place: one synchronised int carries a count and up to three sounds (`OZG_Sounds.PackLot`), and goes quiet again (the count kept, no sounds) a second later. Every client near the place hears a new count with sounds in it once, plays its sounds one after another on the place (`SEffectManager`), and never plays a lot that came before it saw the place. The deck carries the one sound made as the place goes (gathered). In the window, a new view is drawn at once as before; then the window compares it with the view it showed (the log's new lines, the bout, the counts) and flies the cards that moved over it, in a layer of their own that takes no pointer, the card under a flight's landing hidden until it lands; a newer view lands every card still in the air before it is drawn. A shake moves every drawn card of one region for under half a second.

**Tech Stack:** Enforce Script (DayZ 1.30, diag build), `config.cpp` (`CfgSoundShaders`, `CfgSoundSets`), Python 3.13 with numpy 2.5 and `wave` (`tools/make_sounds.py`), ffmpeg 8.0 with `libvorbis` (`E:\Programs\ffmpeg\bin\ffmpeg.exe`), the `dayz` MCP server (`mod_lint`, `mod_build`, `client_compile_check`, `server_start`, `log_verdict`, `world_exec`, `client_*`, `ui_tree`), the stand's verb `oz_games`.

**The design:** `E:\openzone\docs\specs\2026-11-23-openzone-games-design.md`, §17 (sounds) and §11 (the window); the window as built: `docs/2026-11-25-table-window-record.md` (its "Not done" names the flights and the sounds). The card sharp's design `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md`, §6.1, §9, §10: what its fourth phase will ask of this one.

**What the fourth phase gets** (exact signatures in the tasks' Interfaces):

| For | Call | Task |
|---|---|---|
| a sound on this client alone | `static bool OZG_Sounds.PlayLocal(string name, Object where)` (3_Game; `where` null: this client's own player), with `IdOf`, `NameOf`, `SetOf`, `LengthOf` and the ids `OZG_SoundIds.*`; the client message `OZG_Const.OP_SHAKE` (`OZG_WShake`: `PlaceId`, `RegionName`, `SoundName`) already handled on the client | 2, 5 |
| a card in the air | `OZG_Flight OZG_GameMenu.Fly(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)`; places `SpotOfFan`, `SpotOfHand`, `SpotOfStock`, `SpotOfTrump`, `SpotOfDiscards`, `SpotOfBout`; pictures `PictureOfFace`, `PictureOfBack`; keys `SlotOfFan`, `SlotOfHand`, `SlotOfBout`, `SlotOfHeap`, `SLOT_TRUMP` | 5 |
| a small shake | `void OZG_GameMenu.ShakeRegion(string region)` with `RegionOfSeat(int seat)`, `REGION_STOCK` (the design's "deck"), `REGION_DISCARDS` | 5 |

## Global Constraints

- **English and Ukrainian only** in the repository: code, comments, commit messages, docs. No Russian anywhere.
- **Work in `main`**, no branches. **Do NOT push; the owner alone gives the push word.** No remote is added, nothing goes to the Workshop, nothing is downloaded.
- Commit author is the repository's own config (`Alexandru Covalschi`). Every commit message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- **Written beside the card sharp's phase 3.** This plan was written against `main` at `a54bc75` while phase 3 (noticing, `docs/2026-11-25-noticing-plan.md`) was being carried out in some of the same files: `OZG_Game.c`, `OZG_Durak.c`, `OZG_Wire.c`, `OZG_Session.c`, `OZG_Table.c`, `OZG_GameMenu.c` (`PaintFan`, `PaintTable`, `PaintBout`, `PaintHand`, the marks' section), `tools/make_layouts.py` (a comment), `OZG_WorldOps.c`, `OZG_Verb.c`, `docs/test-commands.md`, `README.md`. Every edit here is a small replacement meant to apply before or after phase 3's. Carry this plan out after phase 3 is committed whole, not between its tasks; never stage a file another session has changed and not committed (`git status` first; `git add` only this task's files). Where an anchor is not found word for word, find the same lines (phase 3's own lines may stand beside them now), make the same change there, and never remove a line of phase 3's.
- **No DayZ API is invented.** Every engine call of this plan is in "What the game's own scripts say" below, read in `.dayz-mcp/corpus/scripts` (the 1.30 scripts the `dayz` MCP indexes) and in the game's configs under `P:\DZ`. A call that does not compile is looked up with `knowledge_find`, never guessed at.
- **Enforce Script rules that bite here**: no ternary; a compound boolean goes into a local before it goes into a field; a local is never named `out`, `owned`, `local`; a field or a method is never named like a class of the game or of another mod (this plan gives two-word names wherever a name is called unqualified; before the first build of a task, `knowledge_find(kind="class", name=<the name>)` for every new name the task lists as one plain word); one expression holds about ten parts at most (long strings built in steps); `%` on ints only; `array.Remove(i)` swaps with the last (`RemoveOrdered` where order counts); variables of sibling blocks get different names.
- **The design's words are copied, not re-decided**: a sound is heard where the game is, played on the place on every client near it; the server plays nothing; the window plays nothing of its own; the place's pulse gains one synchronised number, the last sound and a count; a client plays it with `SEffectManager.PlaySoundOnObject` (here: its four steps, to read what `SoundPlay` answers); the range is the sound shader's, **15 m**, "this spec's number, not the owner's"; the deal is a quick run, not every card; a sleight's sound is heard only by the one who does it and by those who saw it, on their own clients, never through the place.
- **The sounds are judged by ear by the owner, not by the executor.** What is checked on the stand is that each sound was played (`started=1` in the client's log), at the moment it belongs to, in the order of its lot, and that no error names a sound set or a sample. How loud each is is the shader's `volume` in `tools/make_sounds.py`'s table.
- **Files are LF in the index; with `core.autocrlf` some are CRLF in the working tree** (`git ls-files --eol` says `w/crlf`): `OpenZone_Games/config.cpp`, every file of `3_Game/OpenZone_Games`, `4_World/.../OZG_Session.c`, `5_Mission/.../OZG_MissionGameplay.c`, `OpenZone_Games_Bridge/.../OZG_Bench.c` and `OZG_Verb.c`. Edit them with the Edit tool, never with `sed` (it strips the CR). A Python script that carries a backslash is written with the Write tool, not a Bash heredoc. New files are written LF.
- **The stand.** A stand cycle is `client_stop` (when a client runs), `server_stop`, `mod_build`, `job_wait`, `server_start(timeout=600)`, `job_wait`, `log_verdict(since=...)` which must say `pass`, `world_ready`. A task that changes client code also runs `client_compile_check`, `job_wait`, `log_verdict(source="client")` = `pass` before the stand cycle. A client, when a step needs one: `client_start(language="English")` or `language="russian"` (the Ukrainian texts), `job_wait`. End every working session with `client_stop`. After any edit of a `.toml`, `project_open` again.
- **The stand may be in the owner's hands** (it was while this plan was written). Ask before the first cycle of a working session which stand is free; never restart a stand the owner is playing on without the owner's word.
- **The checks are written for the NHChernobyl stand** (world 84, the active MCP profile): a place for seven seats at `6097.4 107.65 6028.9` (`lay` with `mode=2`, `from` two metres west); the player teleported to `6095.4 107.65 6028.9` before anything is laid (`world_teleport`). A server holds the map only near a connected player.
- **Stand calls** are written `oz_games {...}`: the MCP tool `world_exec` with `verb="oz_games"` and those `args`; every value of `args` is a string; what is compared is the answer's `detail` (cut at 400 characters).
- **The client's script log** is the newest `script_*.log` in `E:\dayzmod\testserver\clientprofile` (`testserver\games-nh\clientprofile` is a junction to it); read it with the Grep tool. Its lines carry milliseconds (`11:52:38.436`). The mod's debug lines need the core's debug switch, which the stand has on (the client's log says `sync received: ... debug=true`); they read `[OpenZone/Games] dbg: ...`. `log_tail` reads the server's log.
- **Opening the window** on the stand: the client presses `j` (`client_type` with `j`; it takes the foreground). The flights' layer is read with `ui_tree(root="OZG_Air", depth=1)`: `count` is the layer and the cards in it.
- **Screenshots**: every stand run that has something on screen ends with its key shots sent to the owner (`SendUserFile`).

## What the game's own scripts say

Read for this plan in `.dayz-mcp/corpus/scripts` (paths relative to it), in `P:\DZ` (the game's configs, extracted 2026-08-22 and held against 1.30) and in the series' radio. None of it was run for this plan; Tasks 2 and 3 measure what the design leans on.

**Playing a sound set on a client** (`3_game/effectmanager.c`):
- `static EffectSound CreateSound(string sound_set, vector position, float play_fade_in = 0, float stop_fade_out = 0, bool loop = false, bool enviroment = false)` (:153) makes and registers an effect; registered effects "will not be freed automatically" unless `SetAutodestroy(true)` is called (:40-41).
- `static EffectSound PlaySoundOnObject(string sound_set, Object parent_object, float play_fade_in = 0, float stop_fade_out = 0, bool loop = false)` (:256-265) is four steps: `CreateSound(sound_set, parent_object.GetPosition(), ...)`, `SetParent(parent_object)`, `SetLocalPosition(vector.Zero)`, `SoundPlay()` -- and drops what `SoundPlay` answers. This plan makes the same four steps itself to log that answer.
- `bool EffectSound.SoundPlay()` (`3_game/effects/effectsound.c:199`) answers whether the sound will start; a set the config does not know gives `ErrorEx("<effect>: SoundSetName: '<set>' :: Invalid sound set.")` (:313, :934-937) and false.
- `void Effect.SetParent(Object parent_obj)` (`3_game/effect.c:411`), `void SetLocalPosition(vector pos)` (:503), `void SetAutodestroy(bool auto_destroy)` (:242; `EffectSound` overrides it, effectsound.c:607). The game's own callers set autodestroy after playing (`3_game/entities/object.c:1247-1264`, `PlaySoundSet`; `4_world/entities/manbase/playerbase.c:1388-1389`).

**Time and calls later**: `proto int CGame.GetTime()`, mission time in ms (`3_game/global/game.c:1535`); `DayZPlayer GetPlayer()` (:944); `ScriptCallQueue GetCallQueue(int call_category)` (:1550); `CALL_CATEGORY_SYSTEM` runs always (`3_game/tools/tools.c:8`). `proto void CallLater(func fn, int delay = 0, bool repeat = false, void param1 = NULL, ...)` and `proto void Remove(func fn)` (`2_gamelib/tools.c:61, :65`); a static function as `fn` is already used in this repository (`OZG_PoseDriver.c:96`). A pending call on an object is removed in its destructor or `EEDelete` (the skill's pitfalls).

**Synchronised numbers and attachments** (`3_game/entities/entityai.c`): `proto native void RegisterNetSyncVariableInt(string variableName, int minValue = 0, int maxValue = 0)` (:2850; 0 and 0: the whole int, as `OZG_Place` already registers `m_OZG_IdA`); `void EEItemAttached(EntityAI item, string slot_name)` (:1159). `IEntity` and `Widget` are `Managed` (`1_core/proto/enentity.c:164`, `1_core/proto/enwidgets.c:107`): a plain reference does not keep one alive.

**How the game's own items send sounds** (`4_world/entities/itembase.c`): the legacy way, kept "for legacy modding compatibility", is a synchronised id (`m_SoundSyncPlay`, :117, registered :210) played in `OnVariablesSynchronized` and cleared by the server after 100 ms (`LegacyItemSoundSync`, :3297; `ClearStartItemSoundServer`, :4949, obsolete since 1.30). 1.30 sends them as an RPC on the object instead, batched once a frame (`StartItemSoundServer`, :4440; `FlushItemSoundQueue`, :4468-4480). The design asks for a synchronised number on the place; this plan keeps to it and takes the legacy way's one lesson: the number goes quiet again, so that a client that comes later finds nothing to play.

**Sound configs** (`P:\DZ\sounds\hpp\config.cpp`, the game's `sounds_hpp` config, CfgPatches `DZ_Sounds_Effects`, :24): `class baseCharacter_SoundShader { range = 35; };` (:1452); `class bleeding_SoundShader: baseCharacter_SoundShader { samples[] = {{"\DZ\sounds\Characters\actions\misc\blood_spill_1",1}, ...}; volume = 0.4; range = 15; };` (:2314) -- a sample is named without its extension; `class baseCharacter_SoundSet { sound3DProcessingType = "character3DProcessingType"; distanceFilter = "BaseCharacter_AttenuationFilter"; volumeCurve = "characterAttenuationCurve"; spatial = 1; doppler = 0; loop = 0; };` (:56838); `class bleeding_SoundSet: baseCharacter_SoundSet { soundShaders[] = {"bleeding_SoundShader"}; ... };` (:57826), which the game plays with `SEffectManager.PlaySoundOnObject("bleeding_SoundSet", this)` (playerbase.c:7889). The series' own precedent is OpenZone Radio (`E:\openzone\openzone-radio\OpenZone_Radio\config.cpp:35-47, :123-...`): its `.ogg` in `OpenZone_Radio\sounds`, `samples[] = {{"OpenZone_Radio\sounds\ozr_squelch", 1}}`, and `DZ_Sounds_Effects` in `requiredAddons` because its shaders inherit a game's shader ("without this line the base class is not found when the config is built").

**The window** (the `dayz-modding` skill, `references/gui-layouts.md`; `docs/2026-11-25-table-window-record.md`): `SetPos` and `SetSize` take pixels on a widget of exact flags; a card is a square frame 1.1 diagonals wide turned whole by `SetRotation(0, 0, deg)` (clockwise, about its middle), its children turn with it; a widget with `ignorepointer 1` takes no click; of two siblings of one priority the later drawn lies over the earlier (the trump drawn first lies under the stock). `Widget.Show`, `IsVisible`, `SetPos`, `SetSize`, `SetRotation`, `Unlink` (`1_core/proto/enwidgets.c:125-173`). `Math.PI`, `Math.DEG2RAD` (`1_core/proto/enmath.c:12, :17`).

**ffmpeg**: `E:\Programs\ffmpeg\bin\ffmpeg.exe -hide_banner -encoders` lists `libvorbis` (and the native `vorbis`, experimental): `libvorbis` is used, no `-strict -2`. With `-fflags +bitexact -flags:a +bitexact -map_metadata -1` two runs gave byte-identical files when this plan was written.

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `tools/make_sounds.py` | makes the ten sounds from numbers, encodes them, writes their ids and their config | create |
| `art/sounds/SOURCES.md` | where the sounds come from | create |
| `OpenZone_Games/sounds/ozg_*.ogg` | the ten sounds (written by the script) | create |
| `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_SoundIds.c` | the ids, names, sets, lengths (written by the script) | create |
| `OpenZone_Games/config.cpp` | `DZ_Sounds_Effects`; `CfgSoundShaders`, `CfgSoundSets` between the script's markers | modify |
| `.../3_Game/.../OZG_Sounds.c` | the sounds on a client: the table, the number, playing (`OZG_Sounds`, `OZG_SoundRun`, `OZG_SoundPulse`) | create |
| `.../3_Game/.../OZG_Const.c`, `OZG_Wire.c` | the client message `shake` (`OP_SHAKE`, `OZG_WShake`) | modify |
| `.../3_Game/.../OZG_Game.c`, `OZG_Durak.c` | what a change made heard | modify |
| `.../4_World/.../OZG_Items.c` | the place's and the deck's sound number; the deck put down; a place by its id | modify |
| `.../4_World/.../OZG_Session.c`, `OZG_Sessions.c` | the heard lot onto the place; the deck gathered | modify |
| `.../5_Mission/.../OZG_Flights.c` | a card's place in the window, a card in the air, the flights, the shakes | create |
| `.../5_Mission/.../OZG_FlightPlan.c` | what moved between two views of a durak table, flown | create |
| `.../5_Mission/.../OZG_GameMenu.c` | the registry of drawn cards, places, `Fly`, `ShakeRegion`, the ticks, the flights after a newer view | modify |
| `.../5_Mission/.../OZG_MissionGameplay.c` | the `shake` message | modify |
| `tools/make_layouts.py`, `gui/layouts/ozg_win_classic.layout`, `ozg_win_stalker.layout` | the layer of flights `OZG_Air` | modify |
| `OpenZone_Games_Bridge/.../OZG_WorldOps.c`, `OZG_Bench.c`, `OZG_Verb.c` | stand ops `shake`, `sound`, `heard` | modify |
| `docs/2026-11-25-sounds-and-flights-record.md`, `docs/test-commands.md`, `README.md` | what was measured, what a person tries | create, modify |

---

### Task 1: The table's sounds, made from numbers

Ten sounds, synthesized by a script with fixed seeds, encoded to Ogg Vorbis, and the config that names them. Nothing plays them yet.

**Files:**
- Create: `tools/make_sounds.py`, `art/sounds/SOURCES.md`
- Create (by the script): `OpenZone_Games/sounds/ozg_shuffle.ogg`, `ozg_deal.ogg`, `ozg_trump.ogg`, `ozg_lay.ogg`, `ozg_beat.ogg`, `ozg_take.ogg`, `ozg_sweep.ogg`, `ozg_draw.ogg`, `ozg_putdown.ogg`, `ozg_pickup.ogg`; `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_SoundIds.c`
- Modify: `OpenZone_Games/config.cpp` (`requiredAddons`; the sounds' block at the end of the file)

**Interfaces:**
- Produces: `class OZG_SoundIds` (3_Game) with `static const int SHUFFLE = 1, DEAL = 2, TRUMP = 3, LAY = 4, BEAT = 5, TAKE = 6, SWEEP = 7, DRAW = 8, PUTDOWN = 9, PICKUP = 10, LAST = 10` and `static void Fill(array<string> names, array<string> sets, array<int> lengths)` (from id 0, "none"); the sound sets `OZG_Shuffle_SoundSet`, `OZG_Deal_SoundSet`, `OZG_Trump_SoundSet`, `OZG_Lay_SoundSet`, `OZG_Beat_SoundSet`, `OZG_Take_SoundSet`, `OZG_Sweep_SoundSet`, `OZG_Draw_SoundSet`, `OZG_PutDown_SoundSet`, `OZG_PickUp_SoundSet`. The fourth phase adds its sleights' sounds as rows of `SOUNDS` in the script.

- [ ] **Step 1: The script**

Create `tools/make_sounds.py` with the Write tool (it carries backslashes):

```python
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
```

`np.random.RandomState` is numpy's frozen legacy stream: a seed gives the same numbers in every numpy version.

- [ ] **Step 2: The config's place for the sounds**

In `OpenZone_Games/config.cpp` (with the Edit tool), replace

```cpp
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "JM_CF_Scripts",
            "OpenZone_Core"
        };
```

with

```cpp
        requiredAddons[] =
        {
            "DZ_Data",
            "DZ_Scripts",
            "JM_CF_Scripts",
            "OpenZone_Core",
            // The game's sounds config: the table's sound shaders and sets
            // inherit its baseCharacter_SoundShader and baseCharacter_SoundSet,
            // and without it the base classes are not there when the config
            // is read (OpenZone Radio's config says the same). It is the
            // game's own pbo, on every server.
            "DZ_Sounds_Effects"
        };
```

and replace the end of the file

```cpp
    // A seat left with cards.
    class OZG_SeatMark_Left: OZG_SeatMark
    {
        hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(1.0,0.72,0.15,1.0,co)"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\mark\data\ozg_mark_amber.rvmat"};
    };
};
```

with

```cpp
    // A seat left with cards.
    class OZG_SeatMark_Left: OZG_SeatMark
    {
        hiddenSelectionsTextures[] = {"#(argb,8,8,3)color(1.0,0.72,0.15,1.0,co)"};
        hiddenSelectionsMaterials[] = {"OpenZone_Games\models\mark\data\ozg_mark_amber.rvmat"};
    };
};

// ------------------------------------------------------------ sounds
//
// The sounds of the table (design §17), every one made from numbers by
// tools/make_sounds.py (art/sounds/SOURCES.md), a mono .ogg in
// OpenZone_Games/sounds. A shader names its file without the extension, as
// the game's own do, and carries 15 m (the design's number); a set is what a
// client plays on the place (OZG_Sounds). The text between the markers is the
// script's: edit the script, then run it.
// >>> generated: sounds
// <<< generated: sounds
```

- [ ] **Step 3: Where the sounds come from**

Create `art/sounds/SOURCES.md`:

```markdown
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
```

- [ ] **Step 4: Make them**

Run `python -I tools/make_sounds.py`. Expected output (the script was run so, its outputs pointed at a scratch folder with `--root`, when this plan was written):

```
shuffle   1281 ms  ogg  17444 B  d5e77a9f6645
deal      1145 ms  ogg  14430 B  43afe37bd5a7
trump      258 ms  ogg   5730 B  3f5f5785126f
lay        189 ms  ogg   5244 B  1500544b39b7
beat       229 ms  ogg   5639 B  b10622f4204d
take       489 ms  ogg   7315 B  beb4327a0095
sweep      477 ms  ogg   7651 B  e307e4a35130
draw       297 ms  ogg   6071 B  a5389296a35f
putdown    151 ms  ogg   4777 B  c1a3d46b8c53
pickup     304 ms  ogg   5727 B  04d7fdbd5282
10 sounds, 4820 ms in all; OZG_SoundIds.c and config.cpp written
```

Run it a second time: the same ten digests (seeded). Then:

- `E:\Programs\ffmpeg\bin\ffprobe.exe -v error -show_entries stream=codec_name,channels,sample_rate -of compact OpenZone_Games/sounds/ozg_lay.ogg` prints `stream|codec_name=vorbis|sample_rate=44100|channels=1`.
- `git status --short` lists the ten `.ogg`, `OZG_SoundIds.c`, `config.cpp`, `art/sounds/SOURCES.md`, `tools/make_sounds.py`, and nothing under `build/`.
- `config.cpp` keeps its CRLF (`python -I -c "d=open(r'E:\openzone\openzone-games\OpenZone_Games\config.cpp','rb').read(); print(d.count(b'\r\n') == d.count(b'\n'))"` prints `True`), and between the markers stand `class CfgSoundShaders` with ten `OZG_<Name>_SoundShader` (each `samples[] = {{"OpenZone_Games\sounds\ozg_<name>", 1}};`, its volume, `range = 15;`) and `class CfgSoundSets` with ten `OZG_<Name>_SoundSet`.

A digest that differs from the table above while the second run repeats the first is not a fault (another numpy or ffmpeg build); write the new table into the record.

- [ ] **Step 5: Build and boot**

`mod_lint`: no refusals. A stand cycle (Global Constraints). Then:

- `python -I -c "d=open(r'E:\openzone\openzone-games\@OpenZone_Games\addons\OpenZone_Games.pbo','rb').read(); print([n for n in (b'shuffle',b'deal',b'trump',b'lay',b'beat',b'take',b'sweep',b'draw',b'putdown',b'pickup') if d.count(b'ozg_'+n+b'.ogg')!=1], d.count(b'.ogg'))"` prints `[] 10`: the ten files are in the pbo, once each.
- `log_verdict` says `pass`; `log_tail(pattern="SoundShader", n=600)` holds no line.
- `client_start(language="English")`, `job_wait`, `client_verdict(since=<client_start's value>)` says `pass`, and the client's newest `.RPT` in `E:\dayzmod\testserver\clientprofile` holds no line with `OZG_` and `Sound` (a base class not found, a file not found).

- [ ] **Step 6: Commit**

```bash
git add tools/make_sounds.py art/sounds/SOURCES.md OpenZone_Games/sounds OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_SoundIds.c OpenZone_Games/config.cpp
git commit -m "Sounds: the table's sounds, made from numbers

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 2: A sound for one client alone

A client plays a sound of the table on a thing, for its own player and nobody else; and the client half of the card sharp's `shake` message plays the sound it names (its region shakes from Task 5 on). The stand sends that message, so every sound can be tried on the client one by one.

**Files:**
- Create: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Sounds.c`
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c`, `OZG_Wire.c`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c` (`OZG_Place.OZG_ById`)
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MissionGameplay.c` (`OZG_SvcRes`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c`, `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_SoundIds.Fill` and the ids (Task 1).
- Produces (3_Game): `class OZG_Sounds` with `static const int QUIET_MS = 1000; SLOTS = 3; ID_ROOM = 32; COUNT_UNIT = 32768; COUNT_ROOM = 65536;`, `static int IdOf(string name)` (0: no such sound), `static string NameOf(int id)`, `static string SetOf(int id)`, `static int LengthOf(int id)` (ms), `static string NamesOf(array<int> ids)` ("-" for none), `static int PackLot(int was, array<int> ids)`, `static int QuietOf(int packed)`, `static int CountOf(int packed)`, `static void IdsOf(int packed, array<int> into)`, `static bool PlayLocal(string name, Object where)`, `static bool PlayIdLocal(int id, Object where, string why)`; `class OZG_SoundRun` with `void OZG_SoundRun(Object thing, string why)` and `void AddLot(array<int> ids)`. `OZG_Const.OP_SHAKE = "shake"`; `class OZG_WShake { string PlaceId; string RegionName; string SoundName; }`; `static OZG_Place OZG_Place.OZG_ById(string id)`. Stand op `shake id=<place> [region=] [sound=] [who=me]`.

- [ ] **Step 1: The sounds on a client**

Create `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Sounds.c`:

```c
// The sounds of the table (design §17) on a client. The server plays nothing.
// A place carries a number (OZG_Place.OZG_Hear) that tells every client near
// it what to play; a client plays a sound meant for its player alone through
// PlayLocal, the way a sleight is heard only by the one who does it and by
// those who saw it (card sharp's design §6.1, §9). The sounds, their sets and
// their lengths are written by tools/make_sounds.py (OZG_SoundIds.c,
// config.cpp).
class OZG_Sounds
{
    // How long a number carries a lot of sounds before it goes quiet, ms: a
    // client that sees the place later reads the count and no sounds, and so
    // never plays what was played before it came.
    static const int QUIET_MS = 1000;

    // One number carries up to SLOTS sounds and a count: five bits a sound,
    // the first played in the lowest bits, the count above them. The count
    // wraps at COUNT_ROOM, before count * COUNT_UNIT would pass 2^31.
    static const int SLOTS = 3;
    static const int ID_ROOM = 32;
    static const int COUNT_UNIT = 32768;
    static const int COUNT_ROOM = 65536;

    protected static ref array<string> s_Names;
    protected static ref array<string> s_Sets;
    protected static ref array<int> s_Lengths;

    protected static void LoadList()
    {
        s_Names = new array<string>();
        s_Sets = new array<string>();
        s_Lengths = new array<int>();
        OZG_SoundIds.Fill(s_Names, s_Sets, s_Lengths);
    }

    // The id of a sound by its name ("lay"); 0 for a name no sound has.
    static int IdOf(string name)
    {
        if (!s_Names)
            LoadList();
        if (name == "")
            return 0;
        int at = s_Names.Find(name);
        if (at < 0)
            return 0;
        return at;
    }

    static string NameOf(int id)
    {
        if (!s_Names)
            LoadList();
        if (id <= 0 || id >= s_Names.Count())
            return "";
        return s_Names[id];
    }

    // The sound set config.cpp names for it; "" for no sound.
    static string SetOf(int id)
    {
        if (!s_Sets)
            LoadList();
        if (id <= 0 || id >= s_Sets.Count())
            return "";
        return s_Sets[id];
    }

    // How long it sounds, ms.
    static int LengthOf(int id)
    {
        if (!s_Lengths)
            LoadList();
        if (id <= 0 || id >= s_Lengths.Count())
            return 0;
        return s_Lengths[id];
    }

    // "shuffle,deal,trump": the names of a lot, for the logs and the stand;
    // "-" for none.
    static string NamesOf(array<int> ids)
    {
        if (!ids || ids.Count() == 0)
            return "-";
        string line = "";
        for (int i = 0; i < ids.Count(); i++)
        {
            if (i > 0)
                line = line + ",";
            line = line + NameOf(ids[i]);
        }
        return line;
    }

    // ------------------------------------------------------------ the number a thing carries

    // The number after `was` that carries `ids` (the first SLOTS of them, the
    // first played first) and the next count.
    static int PackLot(int was, array<int> ids)
    {
        int count = CountOf(was) + 1;
        if (count >= COUNT_ROOM)
            count = 0;
        int sounds = 0;
        int unit = 1;
        int n = ids.Count();
        if (n > SLOTS)
            n = SLOTS;
        for (int i = 0; i < n; i++)
        {
            int id = ids[i];
            if (id < 0 || id >= ID_ROOM)
                id = 0;
            sounds = sounds + id * unit;
            unit = unit * ID_ROOM;
        }
        return count * COUNT_UNIT + sounds;
    }

    // The same count with no sounds: what the number says once it is quiet.
    static int QuietOf(int packed)
    {
        return CountOf(packed) * COUNT_UNIT;
    }

    static int CountOf(int packed)
    {
        if (packed < 0)
            return 0;
        return packed / COUNT_UNIT;
    }

    // The sounds a number carries, the first played first.
    static void IdsOf(int packed, array<int> into)
    {
        into.Clear();
        if (packed < 0)
            return;
        int rest = packed % COUNT_UNIT;
        for (int i = 0; i < SLOTS; i++)
        {
            int id = rest % ID_ROOM;
            rest = rest / ID_ROOM;
            if (id > 0)
                into.Insert(id);
        }
    }

    // ------------------------------------------------------------ playing, on this client

    // The sound `name` played on `where` on this client and on no other: on
    // the place for a sound of the table; on this client's own player when
    // `where` is null. False when there is no such sound or nothing to play it
    // on. The card sharp's fourth phase plays a sleight's sound with this.
    static bool PlayLocal(string name, Object where)
    {
        int id = IdOf(name);
        if (id == 0)
        {
            OZG_Log.Dbg("sound: no sound named '" + name + "'");
            return false;
        }
        return PlayIdLocal(id, where, "local");
    }

    // The same by id; `why` names in the log what played it. Every sound
    // played is named in the log at debug level, with what SoundPlay said.
    static bool PlayIdLocal(int id, Object where, string why)
    {
        if (!GetGame() || GetGame().IsDedicatedServer())
            return false;
        string soundSet = SetOf(id);
        if (soundSet == "")
            return false;
        Object thing = where;
        if (!thing)
            thing = GetGame().GetPlayer();
        if (!thing)
            return false;
        // SEffectManager.PlaySoundOnObject, step by step (effectmanager.c:256),
        // so that what SoundPlay answers reaches the log.
        EffectSound sound = SEffectManager.CreateSound(soundSet, thing.GetPosition());
        sound.SetParent(thing);
        sound.SetLocalPosition(vector.Zero);
        bool started = sound.SoundPlay();
        sound.SetAutodestroy(true);
        string said = "0";
        if (started)
            said = "1";
        string line = "sound " + NameOf(id) + " on " + why;
        OZG_Log.Dbg(line + ": started=" + said);
        return started;
    }
}

// Sounds played one after another on one thing, on this client: a lot of a
// place's number. Each waits for the one before it to end
// (OZG_Sounds.LengthOf); a lot that comes while another plays does not wait
// for it, the two sound together.
class OZG_SoundRun
{
    protected Object m_Thing;
    protected string m_Why;
    // Sounds still to play and when, in this client's mission time (ms).
    protected ref array<int> m_Ids;
    protected ref array<int> m_At;

    void OZG_SoundRun(Object thing, string why)
    {
        m_Thing = thing;
        m_Why = why;
        m_Ids = new array<int>();
        m_At = new array<int>();
    }

    void ~OZG_SoundRun()
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(PlayDue);
    }

    // `ids` from now on, each when the one before it ends.
    void AddLot(array<int> ids)
    {
        int at = GetGame().GetTime();
        for (int i = 0; i < ids.Count(); i++)
        {
            m_Ids.Insert(ids[i]);
            m_At.Insert(at);
            at = at + OZG_Sounds.LengthOf(ids[i]);
        }
        PlayDue();
    }

    // Plays what is due, and waits for the next.
    protected void PlayDue()
    {
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(PlayDue);
        int now = GetGame().GetTime();
        int soonest = -1;
        int i = 0;
        while (i < m_Ids.Count())
        {
            if (m_At[i] <= now)
            {
                if (m_Thing)
                    OZG_Sounds.PlayIdLocal(m_Ids[i], m_Thing, m_Why);
                m_Ids.RemoveOrdered(i);
                m_At.RemoveOrdered(i);
                continue;
            }
            if (soonest < 0 || m_At[i] < soonest)
                soonest = m_At[i];
            i++;
        }
        if (soonest >= 0)
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(PlayDue, soonest - now, false);
    }
}
```

- [ ] **Step 2: The message `shake`**

In `OZG_Const.c` (Edit tool), replace

```c
    static const string OP_CLOSED = "closed";  // this place is no longer yours to look at
```

with

```c
    static const string OP_CLOSED = "closed";  // this place is no longer yours to look at
    // A sleight was seen (card sharp's design §6.1, §9): a sound for this
    // player alone and a small shake of the part of the table where the cards
    // changed. The fourth phase sends it; the stand sends it now.
    static const string OP_SHAKE  = "shake";
```

In `OZG_Wire.c` (Edit tool), replace

```c
class OZG_WMine
{
    string PlaceId;
    int    SeatNo;

    void OZG_WMine()
    {
        SeatNo = -1;
    }
}
```

with

```c
class OZG_WMine
{
    string PlaceId;
    int    SeatNo;

    void OZG_WMine()
    {
        SeatNo = -1;
    }
}

// What a client is told when its player saw a sleight (card sharp's design
// §6.1, §9): where the cards changed, as a region of the window
// (OZG_GameMenu.RegionOfSeat(n), "discards", "stock"), and the sound to play
// for him alone (a name of OZG_SoundIds, "" for none).
class OZG_WShake
{
    string PlaceId;
    string RegionName;
    string SoundName;
}
```

- [ ] **Step 3: A place by its id**

In `OZG_Items.c`, in `class OZG_Place`, replace

```c
    static array<OZG_Place> All()
    {
        return s_All;
    }
```

with

```c
    static array<OZG_Place> All()
    {
        return s_All;
    }

    // The place of an id on this side, or null (a client has the places near
    // its player).
    static OZG_Place OZG_ById(string id)
    {
        if (id == "")
            return null;
        for (int i = 0; i < s_All.Count(); i++)
        {
            OZG_Place p = s_All[i];
            if (p && p.OZG_Id() == id)
                return p;
        }
        return null;
    }
```

- [ ] **Step 4: The client hears it**

In `OZG_MissionGameplay.c` (Edit tool), in `OZG_SvcRes`, replace

```c
        if (op == OZG_Const.OP_SEATS || op == OZG_Const.OP_VIEW)
```

with

```c
        if (op == OZG_Const.OP_SHAKE)
        {
            OZG_WShake shake;
            string wrong;
            if (!JsonFileLoader<OZG_WShake>.LoadData(json, shake, wrong) || !shake)
                return;
            // Heard by this player alone: played here, on the place when this
            // client has it, never through the place's own number.
            OZG_Place where = OZG_Place.OZG_ById(shake.PlaceId);
            if (shake.SoundName != "")
                OZG_Sounds.PlayLocal(shake.SoundName, where);
            return;
        }

        if (op == OZG_Const.OP_SEATS || op == OZG_Const.OP_VIEW)
```

- [ ] **Step 5: The stand sends it**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before the closing brace on the file's last line), add:

```c

    // shake [id=<place>] [region=seat<n>|discards|stock] [sound=<name>]
    // [who=me|<id>]: the card sharp's message to a player who saw a sleight
    // (card sharp's design §9), sent as the fourth phase will send it: his
    // client plays the sound for him alone and, when his window is open on
    // that place, shakes the region.
    static bool OpShake(map<string, string> args, out string detail)
    {
        OZG_Session s = Sess(args, detail);
        if (!s)
            return false;
        string uid;
        PlayerBase p = Who(args, uid);
        if (!p || !p.GetIdentity())
        {
            detail = "shake: nobody is '" + uid + "'";
            return false;
        }
        OZG_WShake w = new OZG_WShake();
        w.PlaceId = s.Id();
        w.RegionName = OZG_VerbOps.Arg(args, "region", "");
        w.SoundName = OZG_VerbOps.Arg(args, "sound", "");
        string body;
        string err;
        JsonFileLoader<OZG_WShake>.MakeData(w, body, err, false);
        OZ_Rpc.ServiceRespond(p.GetIdentity(), OZG_Const.SERVICE, OZG_Const.OP_SHAKE, true, body, "");
        detail = "shake sent to " + uid + ": region=" + w.RegionName;
        detail = detail + " sound=" + w.SoundName;
        return true;
    }
```

In `OZG_Verb.c` (Edit tool), replace

```c
        // -- ops of the later phases go above this line --
```

with

```c
        if (op == "shake")
            return OZG_WorldOps.OpShake(args, detail);
        // -- ops of the later phases go above this line --
```

and in `KnownOps()` append `, shake` to the end of its second line (the line that begins `known = known + ", ring, level`; it ends `..., craft, reads";`, or with the ops of the card sharp's phase 3 after `reads` once that phase has landed).

- [ ] **Step 6: Lint, compile, build, hear each**

Every new name of this task has two words or the mod's prefix (`PackLot`, `PlayDue`, `OZG_ById`, `OpShake`...): no `knowledge_find` is needed. `mod_lint`: no refusals. `client_compile_check`, `job_wait`, `log_verdict(source="client")` = `pass`. A stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, then:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"shake","id":"<id>","sound":"shuffle"}
```

Expected: `laid <id> seats=7 kind=1`; `shake sent to <uid>: region= sound=shuffle`. The client's newest script log holds `[OpenZone/Games] dbg: sound shuffle on local: started=1`. Then the same for `deal`, `trump`, `lay`, `beat`, `take`, `sweep`, `draw`, `putdown`, `pickup`, a second apart: ten lines `started=1`, one a name. Then `"sound":"nonsense"`: `dbg: sound: no sound named 'nonsense'`. The client's log holds no `SoundSetName: 'OZG_` (an invalid set) and `client_verdict` says `pass`.

Gather the game: `oz_games {"op":"gather","id":"<id>"}`.

- [ ] **Step 7: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Sounds.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Const.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Wire.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MissionGameplay.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Sounds: a sound for one client alone, and the client half of the shake

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 3: A place carries its sounds to every client near it

The place gains one synchronised number: a count and up to three sounds (design §10, §17). The server puts a lot on it and takes the sounds off a second later; every client near the place plays a new lot once, its sounds one after another on the place; a client that sees the place later plays nothing that came before. The deck carries the same number for the one sound made as the place goes. The stand reads both and puts a lot on a place.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Sounds.c` (`OZG_SoundPulse` at the end)
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c` (`OZG_Deck_Base`, `OZG_Place`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c`, `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_Sounds.PackLot`, `QuietOf`, `CountOf`, `IdsOf`, `NamesOf`, `QUIET_MS`, `OZG_SoundRun` (Task 2).
- Produces: `class OZG_SoundPulse` with `int NextNumber(int packed, array<int> ids)` (server), `string LastLot()`, `void HeardNumber(int packed, Object thing, string why)` (client). On `OZG_Place` and on `OZG_Deck_Base` alike (server): `void OZG_Hear(array<int> ids)`, `void OZG_HearOne(int id)`, `void OZG_Quiet()`, `int OZG_SoundNumber()`, `string OZG_SoundLast()`. Stand op `sound [id=<place> | in=me] [play=<name>[,<name>..]]`.

- [ ] **Step 1: What a thing does with its number**

At the end of `OZG_Sounds.c` (Edit tool: after the closing brace of `class OZG_SoundRun`) add:

```c

// What one thing of the world does with its sound number. On the server: the
// next number for a lot of sounds, and the lot remembered for the stand. On a
// client: a new lot played once, as it comes; a number seen for the first
// time with no sounds in it only noted, so that a lot played before the client
// saw the thing is never played (OZG_Sounds.QUIET_MS).
class OZG_SoundPulse
{
    protected int m_Heard;
    protected string m_Last;
    protected ref OZG_SoundRun m_Run;

    void OZG_SoundPulse()
    {
        m_Heard = -1;
        m_Last = "-";
    }

    // Server: the number after `packed` that carries `ids`.
    int NextNumber(int packed, array<int> ids)
    {
        array<int> kept = new array<int>();
        for (int i = 0; i < ids.Count() && i < OZG_Sounds.SLOTS; i++)
            kept.Insert(ids[i]);
        m_Last = OZG_Sounds.NamesOf(kept);
        return OZG_Sounds.PackLot(packed, kept);
    }

    string LastLot()
    {
        return m_Last;
    }

    // Client: the number as it came, played on `thing`.
    void HeardNumber(int packed, Object thing, string why)
    {
        int count = OZG_Sounds.CountOf(packed);
        if (count == m_Heard)
            return;
        array<int> ids = new array<int>();
        OZG_Sounds.IdsOf(packed, ids);
        // When a client first reads the number: at the thing's coming into its
        // world, or at the first change after (the record says which).
        if (m_Heard < 0)
        {
            string first = why + ": first sound number, count " + count.ToString();
            OZG_Log.Dbg(first + ", sounds " + OZG_Sounds.NamesOf(ids));
        }
        m_Heard = count;
        if (ids.Count() == 0)
            return;
        if (!m_Run)
            m_Run = new OZG_SoundRun(thing, why);
        m_Run.AddLot(ids);
    }
}
```

- [ ] **Step 2: The deck's number**

In `OZG_Items.c`, in `class OZG_Deck_Base`, replace

```c
    protected bool m_OZG_Stored;
    protected bool m_OZG_Filled;
```

with

```c
    protected bool m_OZG_Stored;
    protected bool m_OZG_Filled;

    // The sound of the deck gathered up (design §17), in one synchronised
    // number as a place carries its game's (OZG_Sounds.PackLot): the place is
    // gone by then, the deck is where it lay.
    protected int m_OZG_Sound;
    protected ref OZG_SoundPulse m_OZG_Ear;

    void OZG_Deck_Base()
    {
        RegisterNetSyncVariableInt("m_OZG_Sound");
    }

    // Server: a lot of sounds, heard on every client near the deck.
    void OZG_Hear(array<int> ids)
    {
        if (!GetGame() || !GetGame().IsServer() || !ids || ids.Count() == 0)
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Sound = m_OZG_Ear.NextNumber(m_OZG_Sound, ids);
        SetSynchDirty();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_Quiet, OZG_Sounds.QUIET_MS, false);
        OZG_Log.Dbg("deck " + GetType() + " heard " + m_OZG_Ear.LastLot());
    }

    void OZG_HearOne(int id)
    {
        array<int> one = new array<int>();
        one.Insert(id);
        OZG_Hear(one);
    }

    // Server: the number keeps its count and carries no sounds.
    void OZG_Quiet()
    {
        m_OZG_Sound = OZG_Sounds.QuietOf(m_OZG_Sound);
        SetSynchDirty();
    }

    int OZG_SoundNumber()
    {
        return m_OZG_Sound;
    }

    string OZG_SoundLast()
    {
        if (!m_OZG_Ear)
            return "-";
        return m_OZG_Ear.LastLot();
    }

    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        if (GetGame().IsDedicatedServer())
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Ear.HeardNumber(m_OZG_Sound, this, "deck");
    }

    override void EEDelete(EntityAI parent)
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        super.EEDelete(parent);
    }
```

- [ ] **Step 3: The place's number**

In `class OZG_Place`, replace

```c
    protected int m_OZG_Rev;
    protected int m_OZG_DeckNo;
```

with

```c
    protected int m_OZG_Rev;
    protected int m_OZG_DeckNo;
    // The sounds of the game (design §17): one synchronised number, a count
    // and up to three sounds (OZG_Sounds.PackLot), quiet again QUIET_MS after.
    protected int m_OZG_Sound;
    protected ref OZG_SoundPulse m_OZG_Ear;
```

replace

```c
        RegisterNetSyncVariableInt("m_OZG_DeckNo", -1, 126);
    }
```

with

```c
        RegisterNetSyncVariableInt("m_OZG_DeckNo", -1, 126);
        RegisterNetSyncVariableInt("m_OZG_Sound");
    }
```

replace

```c
        m_OZG_Rev = rev;
        SetSynchDirty();
    }
```

with

```c
        m_OZG_Rev = rev;
        SetSynchDirty();
    }

    // Server: a lot of sounds of the game, heard on every client near the
    // place (design §17), whether its window is open or not.
    void OZG_Hear(array<int> ids)
    {
        if (!GetGame() || !GetGame().IsServer() || !ids || ids.Count() == 0)
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Sound = m_OZG_Ear.NextNumber(m_OZG_Sound, ids);
        SetSynchDirty();
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(OZG_Quiet, OZG_Sounds.QUIET_MS, false);
        OZG_Log.Dbg("place " + OZG_Id() + " heard " + m_OZG_Ear.LastLot());
    }

    void OZG_HearOne(int id)
    {
        array<int> one = new array<int>();
        one.Insert(id);
        OZG_Hear(one);
    }

    // Server: the number keeps its count and carries no sounds.
    void OZG_Quiet()
    {
        m_OZG_Sound = OZG_Sounds.QuietOf(m_OZG_Sound);
        SetSynchDirty();
    }

    int OZG_SoundNumber()
    {
        return m_OZG_Sound;
    }

    string OZG_SoundLast()
    {
        if (!m_OZG_Ear)
            return "-";
        return m_OZG_Ear.LastLot();
    }

    // Client: what the number carries is played on the place.
    protected void OZG_Listen()
    {
        if (GetGame().IsDedicatedServer())
            return;
        if (!m_OZG_Ear)
            m_OZG_Ear = new OZG_SoundPulse();
        m_OZG_Ear.HeardNumber(m_OZG_Sound, this, "place " + OZG_Id());
    }
```

replace

```c
    override void EEDelete(EntityAI parent)
    {
        s_All.RemoveItem(this);
```

with

```c
    override void EEDelete(EntityAI parent)
    {
        if (GetGame())
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(OZG_Quiet);
        s_All.RemoveItem(this);
```

and replace

```c
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Look();
        OZG_PlaceWatch.Moved(this);
    }
```

with

```c
    override void OnVariablesSynchronized()
    {
        super.OnVariablesSynchronized();
        OZG_Look();
        OZG_Listen();
        OZG_PlaceWatch.Moved(this);
    }
```

- [ ] **Step 4: The stand reads a number and puts a lot on a place**

In `OZG_WorldOps.c`, at the end of `class OZG_WorldOps` (before its closing brace), add:

```c

    // sound [id=<place> | in=me] [play=<name>[,<name>..]]: the sound number of
    // a laid-out game (in=me: of the deck in the first player's hands): its
    // count, the sounds it carries now (none once OZG_Sounds.QUIET_MS has
    // passed since its last lot) and its last lot. play= puts up to three
    // sounds on the place, as a change of the game puts them.
    static bool OpSound(map<string, string> args, out string detail)
    {
        int packed;
        string last;
        string what;
        if (OZG_VerbOps.Arg(args, "in", "") == "me")
        {
            PlayerBase me = FirstPlayer();
            OZG_Deck_Base deck;
            if (me)
                deck = OZG_Deck_Base.Cast(me.GetItemInHands());
            if (!deck)
            {
                detail = "sound: no deck in the first player's hands";
                return true;
            }
            packed = deck.OZG_SoundNumber();
            last = deck.OZG_SoundLast();
            what = "deck";
        }
        else
        {
            OZG_Session s = Sess(args, detail);
            if (!s)
                return false;
            if (!s.m_Place)
            {
                detail = "sound: the session has no place";
                return true;
            }
            string play = OZG_VerbOps.Arg(args, "play", "");
            if (play != "")
            {
                array<string> names = new array<string>();
                play.Split(",", names);
                array<int> ids = new array<int>();
                for (int i = 0; i < names.Count(); i++)
                {
                    int id = OZG_Sounds.IdOf(names[i]);
                    if (id > 0)
                        ids.Insert(id);
                }
                s.m_Place.OZG_Hear(ids);
            }
            packed = s.m_Place.OZG_SoundNumber();
            last = s.m_Place.OZG_SoundLast();
            what = "place " + s.Id();
        }
        array<int> now = new array<int>();
        OZG_Sounds.IdsOf(packed, now);
        detail = what + ": count=" + OZG_Sounds.CountOf(packed).ToString();
        detail = detail + " now=" + OZG_Sounds.NamesOf(now) + " last=" + last;
        return true;
    }
```

In `OZG_Verb.c`, replace

```c
        if (op == "shake")
            return OZG_WorldOps.OpShake(args, detail);
```

with

```c
        if (op == "shake")
            return OZG_WorldOps.OpShake(args, detail);
        if (op == "sound")
            return OZG_WorldOps.OpSound(args, detail);
```

and in `KnownOps()` append `, sound` to the end of its second line.

- [ ] **Step 5: Lint, compile, build, check (design §17: "every client near it ... a client arriving later does not replay old sounds")**

`knowledge_find(kind="class", name=...)` is not needed: every new name has two words or a prefix. `mod_lint`, `client_compile_check` (`pass`), a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, `lay` as in Task 2 (`laid <id> ...`). Then, in order:

```
oz_games {"op":"sound","id":"<id>"}
oz_games {"op":"sound","id":"<id>","play":"lay"}
```
(two seconds)
```
oz_games {"op":"sound","id":"<id>"}
oz_games {"op":"sound","id":"<id>","play":"shuffle,deal,trump"}
oz_games {"op":"sound","id":"<id>","play":"beat"}
oz_games {"op":"sound","id":"<id>","play":"beat"}
```

Expected answers: `place <id>: count=0 now=- last=-`; `place <id>: count=1 now=lay last=lay`; `place <id>: count=1 now=- last=lay` (quiet); `count=2 now=shuffle,deal,trump last=shuffle,deal,trump`; `count=3 now=beat last=beat`; `count=4 now=beat last=beat`. `log_tail(pattern="heard", n=600)`: `place <id> heard lay`, `... heard shuffle,deal,trump`, `... heard beat` twice.

The client's log, in order:
1. `dbg: place <id>: first sound number, count 0, sounds -` (the place came into the client's world with its number; if instead the first such line names `count 1, sounds lay`, the client got no number at the place's coming: write down which, it is a measurement, and the lay must still be heard).
2. `dbg: sound lay on place <id>: started=1`.
3. `dbg: sound shuffle on place <id>: started=1`, then `sound deal ...` about 1281 ms later, then `sound trump ...` about 1145 ms after that (the lengths of `OZG_SoundIds.c`; a frame or two either way). Write the two gaps measured into the record.
4. `dbg: sound beat on place <id>: started=1` twice (one count apart, the same sound). If only one shows, the two lots came in one network update: write it down.

**A client arriving later**: `client_stop`; `oz_games {"op":"sound","id":"<id>","play":"sweep"}` → `count=5 now=sweep last=sweep`; wait two seconds; `client_start(language="English")`, `job_wait`, `world_ready`. The new client log holds `dbg: place <id>: first sound number, count 5, sounds -` and no `sound sweep` line. Then `oz_games {"op":"sound","id":"<id>","play":"draw"}` → the client plays `sound draw on place <id>: started=1`.

**The deck**: `oz_games {"op":"give","what":"OZG_Deck_Classic"}` (into the hands), `oz_games {"op":"sound","in":"me"}` → `deck: count=0 now=- last=-`. Gather the game: `oz_games {"op":"gather","id":"<id>"}`; `oz_games {"op":"undeck"}` deletes the decks the player carries.

- [ ] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Sounds.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_WorldOps.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Sounds: a place carries its sounds to every client near it

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 4: What a game makes heard

A game says which sounds each change made, in their order (design §17's list); the session puts that lot on the place after every change it accepts; the deck put down on the mat is heard when it lands in the place, and the deck gathered up as it leaves. A deal is one lot: the shuffle, the deal's run, the trump turned up.

**Files:**
- Modify: `OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c`, `OZG_Durak.c`
- Modify: `OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c` (`Changed`), `OZG_Sessions.c` (`ReturnDeck`), `OZG_Items.c` (`OZG_Place.EEItemAttached`)
- Modify: `OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Bench.c`, `OZG_Verb.c`

**Interfaces:**
- Consumes: `OZG_SoundIds.*` (Task 1); `OZG_Place.OZG_Hear`, `OZG_HearOne`, `OZG_Deck_Base.OZG_HearOne` (Task 3); `OZG_Sounds.NamesOf` (Task 2).
- Produces: on `OZG_Game`: `protected void HearSound(int sound)` and `void TakeHeard(array<int> into)`; on `OZG_Session`: `protected void PassHeard()`; durak's lots: Begin `shuffle,deal,trump`; `a:` and `t:` `lay`; `d:` `beat`; a bout closed `sweep` or `take`, then `draw` when the stock gave cards; a seat dropped `sweep` (when cards went) then `draw`. Bench op `heard`.

- [ ] **Step 1: A game hears**

In `OZG_Game.c` (Edit tool), replace

```c
    // ---- a game being played -------------------------------------------------
```

with

```c
    // ---- what is heard -------------------------------------------------------

    // The sounds the last change made, in the order it made them: ids of
    // OZG_SoundIds (design §17). The session takes them after every change it
    // accepts and puts them on the place, where every client near it plays
    // them. A game nobody asks (the stand's bench) keeps the last eight.
    protected ref array<int> m_Heard;

    protected void HearSound(int sound)
    {
        if (!m_Heard)
            m_Heard = new array<int>();
        // The same sound twice in a row is heard once: two cards laid in one
        // change sound as one.
        int n = m_Heard.Count();
        if (n > 0 && m_Heard[n - 1] == sound)
            return;
        if (n >= 8)
            m_Heard.RemoveOrdered(0);
        m_Heard.Insert(sound);
    }

    // What was heard since the last ask, added to `into`; the list emptied.
    void TakeHeard(array<int> into)
    {
        if (!m_Heard)
            return;
        for (int i = 0; i < m_Heard.Count(); i++)
            into.Insert(m_Heard[i]);
        m_Heard.Clear();
    }

    // ---- a game being played -------------------------------------------------
```

- [ ] **Step 2: What durak makes heard**

In `OZG_Durak.c` (Edit tool), replace

```c
        m_S = s;
        OpenBout(FirstOpener(hint));
        return true;
    }
```

with

```c
        m_S = s;
        OpenBout(FirstOpener(hint));
        // Heard: the shuffle, the deal's quick run, the trump turned up.
        HearSound(OZG_SoundIds.SHUFFLE);
        HearSound(OZG_SoundIds.DEAL);
        HearSound(OZG_SoundIds.TRUMP);
        return true;
    }
```

replace (in `MoveAttack`)

```c
        held.RemoveOrdered(at);
        m_S.AttackCards.Insert(card);
        m_S.DefenceCards.Insert("");
        ClearPassed();
        return true;
```

with

```c
        held.RemoveOrdered(at);
        m_S.AttackCards.Insert(card);
        m_S.DefenceCards.Insert("");
        ClearPassed();
        HearSound(OZG_SoundIds.LAY);
        return true;
```

replace (in `MoveDefend`)

```c
        held.RemoveOrdered(at);
        m_S.DefenceCards[target] = card;
        ClearPassed();
```

with

```c
        held.RemoveOrdered(at);
        m_S.DefenceCards[target] = card;
        ClearPassed();
        HearSound(OZG_SoundIds.BEAT);
```

replace (in `MoveTransfer`)

```c
        m_S.AttackLimit = LimitFor(m_S.Defender);
        ClearPassed();
        return true;
```

with

```c
        m_S.AttackLimit = LimitFor(m_S.Defender);
        ClearPassed();
        // Moved on: a card laid on the table like any other.
        HearSound(OZG_SoundIds.LAY);
        return true;
```

replace (in `CloseBout`)

```c
        m_S.AttackCards.Clear();
        m_S.DefenceCards.Clear();
        m_S.Taking = 0;
        m_S.FirstBout = 0;
```

with

```c
        if (took)
            HearSound(OZG_SoundIds.TAKE);
        else
            HearSound(OZG_SoundIds.SWEEP);
        m_S.AttackCards.Clear();
        m_S.DefenceCards.Clear();
        m_S.Taking = 0;
        m_S.FirstBout = 0;
```

replace the whole method `DrawAll`

```c
    protected void DrawAll(int from, int defender)
    {
        int n = m_S.SeatList.Count();
        for (int k = 0; k < n; k++)
        {
            int i = (from + k) % n;
            if (i == defender)
                continue;
            DrawUp(i);
        }
        if (defender >= 0)
            DrawUp(defender);
    }
```

with

```c
    protected void DrawAll(int from, int defender)
    {
        int before = m_S.StockCards.Count();
        int n = m_S.SeatList.Count();
        for (int k = 0; k < n; k++)
        {
            int i = (from + k) % n;
            if (i == defender)
                continue;
            DrawUp(i);
        }
        if (defender >= 0)
            DrawUp(defender);
        if (m_S.StockCards.Count() < before)
            HearSound(OZG_SoundIds.DRAW);
    }
```

and in `DropSeat` replace

```c
        int i;
        for (i = 0; i < dropped.Held.Count(); i++)
            m_S.DiscardCards.Insert(dropped.Held[i]);
```

with

```c
        int i;
        int before = m_S.DiscardCards.Count();
        for (i = 0; i < dropped.Held.Count(); i++)
            m_S.DiscardCards.Insert(dropped.Held[i]);
```

and

```c
        m_S.Taking = 0;
        ClearPassed();

        DrawAll((at + 1) % m_S.SeatList.Count(), -1);
```

with

```c
        m_S.Taking = 0;
        ClearPassed();
        // Its hand and the table swept to the discards.
        if (m_S.DiscardCards.Count() > before)
            HearSound(OZG_SoundIds.SWEEP);

        DrawAll((at + 1) % m_S.SeatList.Count(), -1);
```

- [ ] **Step 3: The session passes the lot to the place**

In `OZG_Session.c` (Edit tool), replace

```c
    void Changed()
    {
        m_R.Rev = m_R.Rev + 1;
        Save();
        Pulse();
        Push();
    }
```

with

```c
    void Changed()
    {
        m_R.Rev = m_R.Rev + 1;
        Save();
        Pulse();
        PassHeard();
        Push();
    }

    // What the last change made heard goes onto the place, where every client
    // near it plays it (design §17): a deal, a move, a bout closed, a seat
    // dropped, an absent seat passed over.
    protected void PassHeard()
    {
        if (!m_Game || !m_Place)
            return;
        array<int> heard = new array<int>();
        m_Game.TakeHeard(heard);
        if (heard.Count() > 0)
            m_Place.OZG_Hear(heard);
    }
```

- [ ] **Step 4: The deck put down and gathered up**

In `OZG_Items.c`, in `class OZG_Place`, replace

```c
    OZG_Deck_Base OZG_Deck()
    {
        return OZG_Deck_Base.Cast(GetInventory().FindAttachmentByName(OZG_Const.SLOT_HOLD));
    }
```

with

```c
    OZG_Deck_Base OZG_Deck()
    {
        return OZG_Deck_Base.Cast(GetInventory().FindAttachmentByName(OZG_Const.SLOT_HOLD));
    }

    // Server: the deck put down on the mat is heard (design §17): laid out by
    // hand (it lands a tick or more after the place is made), by the stand,
    // or given to a place that lost its own. A place read from the world's
    // storage has no session yet when its deck comes back, and is not heard.
    override void EEItemAttached(EntityAI item, string slot_name)
    {
        super.EEItemAttached(item, slot_name);
        if (!GetGame() || !GetGame().IsServer())
            return;
        if (!OZG_Deck_Base.Cast(item) || !OZG_Sessions.Of(this))
            return;
        OZG_HearOne(OZG_SoundIds.PUTDOWN);
    }
```

In `OZG_Sessions.c`, in `ReturnDeck`, replace

```c
        if (!deck)
        {
            GiveDeck(to, deckClass, at);
            return;
        }
        if (to && to.IsAlive() && !to.GetItemInHands())
            to.ServerTakeEntityToHands(deck);
```

with

```c
        if (!deck)
        {
            GiveDeck(to, deckClass, at);
            return;
        }
        // Gathered up (design §17): the place goes in this frame, so the deck
        // carries the sound. A deck made anew for a place that held none
        // (GiveDeck) comes into the clients' world with it and is not heard.
        deck.OZG_HearOne(OZG_SoundIds.PICKUP);
        if (to && to.IsAlive() && !to.GetItemInHands())
            to.ServerTakeEntityToHands(deck);
```

- [ ] **Step 5: The bench says what its game made heard**

In `OZG_Bench.c` (Edit tool), replace

```c
    static bool OpBrief(out string detail)
    {
        if (!HasGame(detail))
            return false;
        detail = s_Game.Brief();
        return true;
    }
```

with

```c
    static bool OpBrief(out string detail)
    {
        if (!HasGame(detail))
            return false;
        detail = s_Game.Brief();
        return true;
    }

    // heard: what the game on the bench made heard since the last ask (the
    // sounds of design §17, by name), and the list emptied.
    static bool OpHeard(out string detail)
    {
        if (!HasGame(detail))
            return false;
        array<int> ids = new array<int>();
        s_Game.TakeHeard(ids);
        detail = "heard " + OZG_Sounds.NamesOf(ids);
        return true;
    }
```

In `OZG_Verb.c` (Edit tool), replace

```c
        if (op == "brief")
            return OZG_Bench.OpBrief(detail);
```

with

```c
        if (op == "brief")
            return OZG_Bench.OpBrief(detail);
        if (op == "heard")
            return OZG_Bench.OpHeard(detail);
```

and in `KnownOps()` append `, heard` to the end of its first line (`string known = "status, ..., fuzz";`).

- [ ] **Step 6: Lint, build, the rules' sounds on the bench**

`mod_lint`, a stand cycle (no client needed for this step), `world_ready`. The deal of the phase 1 measurements' table 1 (`docs/measurements/2026-11-24/rules.md`, rows 1.1 to 1.8), then its second bout, one change at a time:

```
oz_games {"op":"begin","seats":"0,1,2","deck":"6s 6h 7s Th Jd 7c 8s 8h Ts Td Qh Kc 6d 7h 9s 8d Js Ah Qs 9h 7d 8c Ks Jh 9d 9c As Kh Qd Tc Kd Ad Jc Qc Ac 6c"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"0 a:6s"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"1 d:8s:1"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"0 a:6h; 1 d:8h:2; 2 a:6d; 1 d:Td:3; 2 a:8d; 0 a:Th; 1 d:Qh:5"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"1 d:Kc:4"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"1 a:9d"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"2 take"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"0 a:9h"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"1 pass"}
oz_games {"op":"heard"}
```

Expected, the `heard` answers in order: `heard shuffle,deal,trump`; `heard lay`; `heard beat`; `heard lay,beat,lay,beat,lay,beat` (the two attacks in a row, `2 a:8d; 0 a:Th`, sound once); after `1 d:Kc:4` (whose `play` answer ends `stock=8 dis=10 table=- op=1 def=2 ...`, row 1.8) `heard beat,sweep,draw`; `heard lay`; `heard -` (saying "take" moves no card); `heard lay` (thrown in to the taker); after `1 pass` (its answer `... stock=6 ... hands=0:6,1:6,2:8 ...`) `heard take,draw`. Every `play` answer starts `+` for each move.

The variant and a dropped seat (two players, the trump `6c`, seat 0 holds the lowest trump and opens):

```
oz_games {"op":"begin","seats":"0,1","opt":"variant=perevodnoy","deck":"6s 6h 7s Th Jd 7c 6d 8h Ts Td Qh Kc 8s 9s Js Qs Ks As 7h 9h Jh Kh Ah 7d 8d 9d Qd Kd Ad 8c 9c Tc Jc Qc Ac 6c"}
oz_games {"op":"heard"}
oz_games {"op":"play","moves":"0 a:6s; 1 t:6d"}
oz_games {"op":"heard"}
oz_games {"op":"drop","seat":"0"}
oz_games {"op":"heard"}
```

Expected: `heard shuffle,deal,trump`; `+ +` then `heard lay` (the attack and the card that moved it on, one sound); `dropped 0 ...` (the game over with no durak) then `heard sweep,draw`.

A row that answers otherwise is written down with what it said, and the rule it touches is read again before anything is changed.

The rules are untouched: `oz_games {"op":"fuzz","games":"300","seed":"11","drop":"5","away":"30"}` answers `fuzz: 300 games, ...` with no `DEFECT`.

- [ ] **Step 7: A game heard on the stand**

`client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"sound","id":"<id>"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"2","on":"0"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"sound","id":"<id>"}
```

Expected: `laid <id> ...`; `place <id>: count=1 now=putdown last=putdown` (or `now=-` when a second has passed); the client's log `sound putdown on place <id>: started=1`. After the deal: `count=2 ... last=shuffle,deal,trump`; the client's log: the three in order with the two gaps of Task 3.

Then `oz_games {"op":"auto","id":"<id>","n":"1"}` again and again, each time `oz_games {"op":"sound","id":"<id>"}` and the client's log: every `a:` or `t:` gives `lay`, every `d:` `beat`; a bout that closes gives `sweep,draw` or `take,draw` (`draw` only while the stock gives cards; `sess` shows `dis=` grow, or the defender's hand grow). Play until one bout closed each way (a bot takes now and then; `auto` moves for the player too).

Then the player stands up with his cards discarded: `oz_games {"op":"ask","id":"<id>","ask":"stand","how":"discard"}` → `last=sweep,draw` (or `sweep` alone with the stock empty) and the client's log says so. Then `oz_games {"op":"bots","id":"<id>","n":"0","on":"1"}` lets the bots play on a minute; `log_tail(pattern="heard", n=600)` shows a lot for every change.

Gathering: `oz_games {"op":"chat","text":"!ozg clear"}` (the bots go), `oz_games {"op":"gather","id":"<id>"}` (nobody sits now), then at once `oz_games {"op":"sound","in":"me"}` → `deck: count=1 now=pickup last=pickup` (the deck in the hands, a second at most later) and the client's log `sound pickup on deck: started=1`; `log_tail(pattern="heard pickup", n=600)` holds `deck OZG_Deck_Classic heard pickup`. `oz_games {"op":"undeck"}`.

The ear is the owner's: when the owner takes the stand over, a game laid out by hand (`!ozg deck`, "Choose placement", the hologram, "Lay out the game") is the real path of the deck put down (it lands in the place a tick after the place is made), and every sound of a game can be heard with two bots (`!ozg bots`). Say so in the report of the working session, with the commands of Task 7's section of `docs/test-commands.md`.

- [ ] **Step 8: Commit**

```bash
git add OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Game.c OpenZone_Games/scripts/3_Game/OpenZone_Games/OZG_Durak.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Session.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Sessions.c OpenZone_Games/scripts/4_World/OpenZone_Games/OZG_Items.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Bench.c OpenZone_Games_Bridge/scripts/5_Mission/OpenZone_Games_Bridge/OZG_Verb.c
git commit -m "Sounds: a game says what each change made heard, and the place plays it

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 5: Cards in the air and shakes over the window

The window gains a layer of flights over the table and the plates, a registry of every card it drew (the key a flight lands on it by, the region a shake moves it with, where it rests), the places of every kind of card as a public API, `Fly` and `ShakeRegion`, and their ticks. The drawing itself does not change: every card lies where it lay. The `shake` message now shakes its region.

**Files:**
- Create: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Flights.c`
- Modify: `tools/make_layouts.py`; by it `OpenZone_Games/gui/layouts/ozg_win_classic.layout`, `ozg_win_stalker.layout`
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c`
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MissionGameplay.c`

**Interfaces:**
- Consumes: `OZG_WShake` (Task 2); `OZG_TableLook` and the window as they are.
- Produces (5_Mission): `class OZG_CardSpot { float X; float Y; float W; float H; float Turn; static OZG_CardSpot Make(float x, float y, float w, float h, float turn); }` (the middle, the size and the clockwise turn of a card, in layout units of the 1600 x 900 window); `class OZG_Flight`; `class OZG_Flights` with `OZG_Flight Launch(Widget card, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)`, `void Tick(float dt, float k, array<string> landed)`, `void Finish(array<string> landed)`, `int Count()`, `static float EaseOf(float t)`; `class OZG_Shakes` with `void Start(string region)`, `bool Busy()`, `void Tick(float dt, array<string> ended)`, `bool Offset(string region, out float dx, out float dy)`, `void Clear()` (`SECONDS` 0.45, `SIZE` 4 units, `HZ` 17). On `OZG_GameMenu`: `static const string SLOT_TRUMP = "t"`, `REGION_STOCK = "stock"`, `REGION_DISCARDS = "discards"`; `static string SlotOfFan(int seat, int index)`, `SlotOfHand(int index)`, `SlotOfBout(int index, bool defence)`, `SlotOfHeap(int k)`, `RegionOfSeat(int seat)`; `OZG_CardSpot SpotOfFan(OZG_WView v, int seat, int index, int count)` (null: no fan of that seat in `v`; index -1: the fan's middle), `SpotOfHand(int index, int count)`, `SpotOfStock(int stockCount)`, `SpotOfTrump()`, `SpotOfDiscards(int k)`, `SpotOfBout(int count, int index, bool defence)`; `string PictureOfFace(string card, int look)`, `PictureOfBack(int look)`; `int HeapOf(int discardCount)`; `void SortHand(array<string> held, string trumpSuit, array<string> into)`; `OZG_Flight Fly(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)`; `void HideSlot(string key)`; `void ShakeRegion(string region)`; `protected void LandAll()`.

- [ ] **Step 1: The engine**

Create `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Flights.c`:

```c
// Cards in the air over the game window, and small shakes of a part of it
// (design §11; card sharp's design §6.1, §10). The window draws what the
// server said at once; these only move pictures over it, so nothing waits for
// them: a click goes through them to the table, and a newer view lands every
// card still in the air before it is drawn (OZG_GameMenu.LandAll).

// Where a card lies in the window: its middle, its size and its turn
// (degrees, clockwise), in layout units of the 1600 x 900 window -- the
// numbers OZG_TableLook gives.
class OZG_CardSpot
{
    float X;
    float Y;
    float W;
    float H;
    float Turn;

    static OZG_CardSpot Make(float x, float y, float w, float h, float turn)
    {
        OZG_CardSpot s = new OZG_CardSpot();
        s.X = x;
        s.Y = y;
        s.W = w;
        s.H = h;
        s.Turn = turn;
        return s;
    }
}

// One card on its way: a frame of ozg_card.layout in the window's layer of
// flights (OZG_Air).
class OZG_Flight
{
    Widget CardFrame;
    ImageWidget FacePic;
    // What the face shows from halfway on; "" keeps the first picture.
    string LandPicture;
    ref OZG_CardSpot FromSpot;
    ref OZG_CardSpot ToSpot;
    float WaitSecs;
    float FlySecs;
    float AgeSecs;
    // The window's key shown when the card lands; "" for none.
    string RevealKey;
    bool Turned;
}

class OZG_Flights
{
    // The side of a card's turning frame, in diagonals of the card
    // (tools/make_layouts.py, card()).
    static const float BOX = 1.1;
    // How much larger a card is halfway, as if lifted off the table.
    static const float LIFT = 0.08;

    protected ref array<ref OZG_Flight> m_Air;

    void OZG_Flights()
    {
        m_Air = new array<ref OZG_Flight>();
    }

    int Count()
    {
        return m_Air.Count();
    }

    // A card frame already made in the layer of flights sets off from `from`
    // after `delay` seconds (hidden until then) and lands on `to` after
    // `seconds` more, eased in and out, a little lifted on its way.
    // `pictureLand`, when not "", is put on its face halfway: a back that
    // turns face up, or a face that goes down. `reveal` comes back from Tick
    // or Finish when it lands.
    OZG_Flight Launch(Widget card, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        OZG_Flight f = new OZG_Flight();
        f.CardFrame = card;
        f.FacePic = ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace"));
        f.LandPicture = pictureLand;
        f.FromSpot = from;
        f.ToSpot = to;
        f.WaitSecs = delay;
        f.FlySecs = seconds;
        if (f.FlySecs < 0.05)
            f.FlySecs = 0.05;
        f.RevealKey = reveal;
        m_Air.Insert(f);
        card.Show(false);
        return f;
    }

    // `t` of the way, eased: slow away, quick in the middle, slow to land.
    static float EaseOf(float t)
    {
        if (t <= 0)
            return 0;
        if (t >= 1)
            return 1;
        if (t < 0.5)
            return 4 * t * t * t;
        float u = 2 - 2 * t;
        return 1 - u * u * u * 0.5;
    }

    // From one turn to another the short way round, degrees.
    static float TurnBy(float from, float to)
    {
        float d = to - from;
        while (d > 180)
            d = d - 360;
        while (d < -180)
            d = d + 360;
        return d;
    }

    // One frame: every card moved; the ones that landed taken away and their
    // keys put into `landed`. `k` is real pixels in one layout unit.
    void Tick(float dt, float k, array<string> landed)
    {
        for (int i = m_Air.Count() - 1; i >= 0; i--)
        {
            OZG_Flight f = m_Air[i];
            f.AgeSecs = f.AgeSecs + dt;
            if (!f.CardFrame)
            {
                LandAt(i, landed);
                continue;
            }
            float t = (f.AgeSecs - f.WaitSecs) / f.FlySecs;
            if (t < 0)
                continue;
            if (t >= 1)
            {
                LandAt(i, landed);
                continue;
            }
            PutAt(f, t, k);
        }
    }

    // Every card at once where it lands.
    void Finish(array<string> landed)
    {
        for (int i = m_Air.Count() - 1; i >= 0; i--)
            LandAt(i, landed);
    }

    protected void LandAt(int i, array<string> landed)
    {
        OZG_Flight f = m_Air[i];
        if (f.CardFrame)
            f.CardFrame.Unlink();
        if (f.RevealKey != "")
            landed.Insert(f.RevealKey);
        m_Air.RemoveOrdered(i);
    }

    // The card `t` of its way.
    protected void PutAt(OZG_Flight f, float t, float k)
    {
        float e = EaseOf(t);
        OZG_CardSpot a = f.FromSpot;
        OZG_CardSpot b = f.ToSpot;
        float x = a.X + (b.X - a.X) * e;
        float y = a.Y + (b.Y - a.Y) * e;
        float w = a.W + (b.W - a.W) * e;
        float h = a.H + (b.H - a.H) * e;
        float turn = a.Turn + TurnBy(a.Turn, b.Turn) * e;
        float rise = 1 + LIFT * Math.Sin(t * Math.PI);
        float side = BOX * Math.Sqrt(w * w + h * h) * rise;
        float half = side * 0.5;
        f.CardFrame.SetSize(side * k, side * k);
        f.CardFrame.SetPos((x - half) * k, (y - half) * k);
        f.CardFrame.SetRotation(0, 0, turn);
        if (!f.CardFrame.IsVisible())
            f.CardFrame.Show(true);
        if (t < 0.5 || f.Turned || f.LandPicture == "" || !f.FacePic)
            return;
        f.FacePic.LoadImageFile(0, f.LandPicture);
        f.FacePic.SetImage(0);
        f.Turned = true;
    }
}

// Small shakes of parts of the window: a seat's fan, the discards or the
// stock, where a sleight was seen (card sharp's design §6.1). A region is a
// name; the window moves every card it drew in that region by the offset
// asked here, and puts it back when the shake ends.
class OZG_Shakes
{
    static const float SECONDS = 0.45;
    // How far a card goes at the start, layout units, and how fast it goes to
    // and fro.
    static const float SIZE = 4.0;
    static const float HZ = 17.0;

    protected ref array<string> m_Region;
    protected ref array<float> m_Age;

    void OZG_Shakes()
    {
        m_Region = new array<string>();
        m_Age = new array<float>();
    }

    // The region starts shaking, or starts anew when it shakes already.
    void Start(string region)
    {
        int at = m_Region.Find(region);
        if (at >= 0)
        {
            m_Age[at] = 0;
            return;
        }
        m_Region.Insert(region);
        m_Age.Insert(0);
    }

    bool Busy()
    {
        return m_Region.Count() > 0;
    }

    // One frame; the regions whose shake ended go into `ended`.
    void Tick(float dt, array<string> ended)
    {
        for (int i = m_Region.Count() - 1; i >= 0; i--)
        {
            m_Age[i] = m_Age[i] + dt;
            if (m_Age[i] < SECONDS)
                continue;
            ended.Insert(m_Region[i]);
            m_Region.RemoveOrdered(i);
            m_Age.RemoveOrdered(i);
        }
    }

    // Where the region stands now, units off its rest; false when it is not
    // shaking.
    bool Offset(string region, out float dx, out float dy)
    {
        dx = 0;
        dy = 0;
        int at = m_Region.Find(region);
        if (at < 0)
            return false;
        float t = m_Age[at];
        float fade = 1 - t / SECONDS;
        float size = SIZE * fade * fade;
        float phase = t * HZ * 2 * Math.PI;
        dx = size * Math.Sin(phase);
        dy = size * 0.5 * Math.Sin(phase * 1.3 + 1);
        return true;
    }

    void Clear()
    {
        m_Region.Clear();
        m_Age.Clear();
    }
}
```

- [ ] **Step 2: The layer of flights**

In `tools/make_layouts.py`, replace

```python
    k.append(frame("OZG_Seats", exact(0, 0, W, H), 30))
```

with

```python
    k.append(frame("OZG_Seats", exact(0, 0, W, H), 30))
    # The cards in the air (OZG_Flights.c): over the table and the plates,
    # under the buttons; a frame takes no pointer, and nor does a card.
    k.append(frame("OZG_Air", exact(0, 0, W, H), 35))
```

Run `python -I tools/make_layouts.py`; `git status --short OpenZone_Games/gui/layouts` lists `ozg_win_classic.layout` and `ozg_win_stalker.layout` alone, each with one `FrameWidgetClass OZG_Air` more (`ignorepointer 1`, `priority 35`).

- [ ] **Step 3: The window's registry, places and flights**

In `OZG_GameMenu.c`:

(a) Replace

```c
    protected static const float PLATE_PAD = 4;
```

with

```c
    protected static const float PLATE_PAD = 4;

    // The key the trump's card is drawn under (SlotOfFan and the rest below),
    // and the parts of the table a shake moves (ShakeRegion): the stock with
    // the trump under it (the card sharp's design calls it the deck), the
    // discards, a seat's fan (RegionOfSeat).
    static const string SLOT_TRUMP = "t";
    static const string REGION_STOCK = "stock";
    static const string REGION_DISCARDS = "discards";
```

(b) Replace

```c
    // The bout's attack cards as drawn: middle and size.
    protected ref array<float> m_AtkX;
    protected ref array<float> m_AtkY;
    protected ref array<float> m_AtkW;
    protected ref array<float> m_AtkH;
```

with

```c
    // The bout's attack cards as drawn: middle and size.
    protected ref array<float> m_AtkX;
    protected ref array<float> m_AtkY;
    protected ref array<float> m_AtkW;
    protected ref array<float> m_AtkH;

    // Every card drawn with a key or a region (Card), for the flights and the
    // shakes: its frame, the key a flight lands on it by, the region a shake
    // moves it with, and where it rests, in pixels of its parent.
    protected ref array<Widget> m_DrawnW;
    protected ref array<string> m_DrawnKey;
    protected ref array<string> m_DrawnRegion;
    protected ref array<float> m_DrawnX;
    protected ref array<float> m_DrawnY;
    // The keys whose card waits for a flight to land, drawn hidden until then:
    // a key is here once for every flight on its way to it.
    protected ref array<string> m_Hidden;
    // The layer the flying cards are made in (OZG_Air), and what moves them.
    protected Widget m_Air;
    protected ref OZG_Flights m_Flights;
    protected ref OZG_Shakes m_Shakes;
```

(c) In the constructor replace

```c
        m_AtkW = new array<float>();
        m_AtkH = new array<float>();
        if (!s_Reslashed)
            s_Reslashed = new array<string>();
```

with

```c
        m_AtkW = new array<float>();
        m_AtkH = new array<float>();
        m_DrawnW = new array<Widget>();
        m_DrawnKey = new array<string>();
        m_DrawnRegion = new array<string>();
        m_DrawnX = new array<float>();
        m_DrawnY = new array<float>();
        m_Hidden = new array<string>();
        m_Flights = new OZG_Flights();
        m_Shakes = new OZG_Shakes();
        if (!s_Reslashed)
            s_Reslashed = new array<string>();
```

(d) In `Build` replace

```c
        if (style != "stalker")
            style = "classic";
        ClearPictures();
        ClearList(m_SeatW);
```

with

```c
        if (style != "stalker")
            style = "classic";
        // Nothing of the old look flies or shakes on: its cards go with it.
        LandAll();
        m_Shakes.Clear();
        ClearPictures();
        ClearList(m_SeatW);
```

and

```c
        m_DiscardLabel = TextWidget.Cast(Find("OZG_DiscardLabel"));
        // As many log lines as the look has.
```

with

```c
        m_DiscardLabel = TextWidget.Cast(Find("OZG_DiscardLabel"));
        m_Air = Find("OZG_Air");
        // As many log lines as the look has.
```

(e) In `OnHide` replace

```c
        if (m_V && m_V.Mine < 0)
            Send(OZG_Const.OP_UNWATCH, "", -1, "", "", "", 0);
        ClearPictures();
```

with

```c
        if (m_V && m_V.Mine < 0)
            Send(OZG_Const.OP_UNWATCH, "", -1, "", "", "", 0);
        // No card stays in the air, and no part of the table shaken.
        LandAll();
        m_Shakes.Clear();
        ClearPictures();
```

(f) In `Update` replace

```c
        TrackHover();

        // The pulse on the place says a newer revision than the one shown:
```

with

```c
        TickAir(timeslice);
        TickShakes(timeslice);
        TrackHover();

        // The pulse on the place says a newer revision than the one shown:
```

(g) In `ClearPictures` replace

```c
    protected void ClearPictures()
    {
        ClearList(m_FanPics);
```

with

```c
    protected void ClearPictures()
    {
        m_DrawnW.Clear();
        m_DrawnKey.Clear();
        m_DrawnRegion.Clear();
        m_DrawnX.Clear();
        m_DrawnY.Clear();
        ClearList(m_FanPics);
```

(h) Replace the head of `Card`

```c
    // A card on the table: its picture with the middle at (cx, cy), w x h
    // units, turned clockwise by `turn` degrees, with its shadow under it when
    // `shadow`, and its glow in the colour `glow` when that is not 0.
    protected Widget Card(array<Widget> into, string picture, float cx, float cy, float w, float h, float turn, int glow, bool shadow)
    {
```

with

```c
    // A card on the table: its picture with the middle at (cx, cy), w x h
    // units, turned clockwise by `turn` degrees, with its shadow under it when
    // `shadow`, and its glow in the colour `glow` when that is not 0. `slot`
    // is the key a flight lands on it by, `region` the part of the table a
    // shake moves it with; "" for none.
    protected Widget Card(array<Widget> into, string picture, float cx, float cy, float w, float h, float turn, int glow, bool shadow, string slot = "", string region = "")
    {
```

and its end

```c
        Place(card, cx, cy, BoxOf(w, h), turn);
        into.Insert(card);
        return card;
    }
```

with

```c
        Place(card, cx, cy, BoxOf(w, h), turn);
        into.Insert(card);
        KeepDrawn(card, slot, region, cx, cy, BoxOf(w, h));
        return card;
    }
```

(i) Before

```c
    // ------------------------------------------------------------ marks
```

insert:

```c
    // ------------------------------------------------------------ what was drawn, for flights and shakes

    // The keys a flight lands on a card by: the index-th back of a seat's fan
    // (as PaintFan lays it), the index-th card of the player's hand (in the
    // order it is shown), an attack card of the bout or the card that beat
    // it, the k-th back of the discards' heap; the trump is SLOT_TRUMP.
    static string SlotOfFan(int seat, int index)
    {
        return "f" + seat.ToString() + "." + index.ToString();
    }

    static string SlotOfHand(int index)
    {
        return "h" + index.ToString();
    }

    static string SlotOfBout(int index, bool defence)
    {
        if (defence)
            return "d" + index.ToString();
        return "a" + index.ToString();
    }

    static string SlotOfHeap(int k)
    {
        return "x" + k.ToString();
    }

    // The region of a seat's fan, for ShakeRegion.
    static string RegionOfSeat(int seat)
    {
        return "seat" + seat.ToString();
    }

    // A card drawn into the registry; hidden when a flight still has to land
    // on its key. Its place at rest is kept in pixels, for a shake.
    protected void KeepDrawn(Widget card, string slot, string region, float cx, float cy, float side)
    {
        if (!card)
            return;
        if (slot == "" && region == "")
            return;
        float half = side * 0.5;
        m_DrawnW.Insert(card);
        m_DrawnKey.Insert(slot);
        m_DrawnRegion.Insert(region);
        m_DrawnX.Insert((cx - half) * m_K);
        m_DrawnY.Insert((cy - half) * m_K);
        if (slot != "" && m_Hidden.Find(slot) >= 0)
            card.Show(false);
    }

    // The registry forgets the cards of one part of the table before that
    // part is drawn again and its pictures go.
    protected void ForgetDrawn(array<Widget> part)
    {
        for (int i = m_DrawnW.Count() - 1; i >= 0; i--)
        {
            Widget w = m_DrawnW[i];
            if (w && part.Find(w) < 0)
                continue;
            m_DrawnW.RemoveOrdered(i);
            m_DrawnKey.RemoveOrdered(i);
            m_DrawnRegion.RemoveOrdered(i);
            m_DrawnX.RemoveOrdered(i);
            m_DrawnY.RemoveOrdered(i);
        }
    }

    // ------------------------------------------------------------ where cards lie

    // The bottom seat of a view (BottomSeat for any view): the player's own;
    // for a watcher the first seat somebody holds; -1 for none.
    protected int BottomOf(OZG_WView v)
    {
        if (!v)
            return -1;
        if (v.Mine >= 0)
            return v.Mine;
        for (int i = 0; i < v.Seats.Count(); i++)
        {
            if (v.Seats[i].St != OZG_Const.SEAT_FREE)
                return i;
        }
        return -1;
    }

    // The seats round the table in a view, from the bottom one's left: every
    // seat somebody holds but the bottom one (PaintSeats).
    protected void RoundOf(OZG_WView v, int bottom, array<int> into)
    {
        into.Clear();
        int n = v.Seats.Count();
        for (int k = 1; k < n; k++)
        {
            int no = (bottom + k) % n;
            if (v.Seats[no].St != OZG_Const.SEAT_FREE)
                into.Insert(no);
        }
    }

    // The fan of the seat `no` as the view `v` lays it out: its middle, turn
    // and scale. False when that seat has no fan there: a free seat, or the
    // player's own (his hand is the faces at the bottom).
    protected bool FanOf(OZG_WView v, int no, out float x, out float y, out float turn, out float scale)
    {
        x = 0;
        y = 0;
        turn = 0;
        scale = 1;
        if (!v || no < 0 || no >= v.Seats.Count())
            return false;
        int bottom = BottomOf(v);
        if (no == bottom)
        {
            if (v.Mine >= 0)
                return false;
            x = m_Look.HandX;
            y = m_Look.HandY + 20;
            return true;
        }
        array<int> round = new array<int>();
        RoundOf(v, bottom, round);
        int at = round.Find(no);
        if (at < 0)
            return false;
        array<ref OZG_TableSpot> spots = new array<ref OZG_TableSpot>();
        m_Look.Spots(round.Count(), spots);
        OZG_TableSpot spot = spots[at];
        x = spot.X;
        y = spot.Y;
        turn = spot.Turn;
        scale = spot.Scale;
        return true;
    }

    // Where the i-th of `count` backs of a fan about (x, y), turned by `turn`,
    // rests: its middle and its own turn (PaintFan).
    protected void FanCardAt(float x, float y, float turn, float scale, int count, int i, out float cx, out float cy, out float a)
    {
        float spread = m_Look.FanSpread;
        if (count > 1 && spread * (count - 1) > m_Look.FanFan)
            spread = m_Look.FanFan / (count - 1);
        float r = m_Look.FanR * scale;
        float tr = turn * Math.DEG2RAD;
        float c = Math.Cos(tr);
        float s = Math.Sin(tr);
        float mid = (count - 1) * 0.5;
        float lean = (i - mid) * spread;
        float ar = lean * Math.DEG2RAD;
        float lx = r * Math.Sin(ar);
        float ly = r - r * Math.Cos(ar);
        cx = x + lx * c - ly * s;
        cy = y + lx * s + ly * c;
        a = lean + turn;
    }

    // Where the i-th of n cards of the player's hand rests before the pointer
    // lifts it: its middle and its turn (PaintHand).
    protected void HandCardAt(int n, int i, out float x, out float y, out float a)
    {
        float spread = m_Look.HandSpread;
        if (n > 1 && spread * (n - 1) > m_Look.HandFan)
            spread = m_Look.HandFan / (n - 1);
        float r = m_Look.HandR;
        float mid = (n - 1) * 0.5;
        a = (i - mid) * spread;
        float ar = a * Math.DEG2RAD;
        x = m_Look.HandX + r * Math.Sin(ar);
        y = m_Look.HandY + r - r * Math.Cos(ar);
    }

    // Where the index-th attack card of a bout of `n` lies, or the card that
    // beat it (`defence`: laid over it and turned a little): middle, size and
    // turn (PaintBout). One row of up to three; two rows of smaller cards from
    // four.
    protected void BoutCardAt(int n, int index, bool defence, out float cx, out float cy, out float w, out float h, out float turn)
    {
        float scale = 1;
        float pitch = 150;
        int perRow = n;
        int rows = 1;
        if (n == 3)
            pitch = 116;
        if (n >= 4)
        {
            rows = 2;
            perRow = (n + 1) / 2;
            scale = 0.82;
            pitch = 104;
            if (perRow > 3)
                pitch = 312.0 / (perRow - 1);
        }
        w = m_Look.BoutW * scale;
        h = m_Look.BoutH * scale;
        float ox = 18 * scale;
        float oy = 22 * scale;
        int row = 0;
        int col = index;
        int inRow = perRow;
        if (rows == 2 && index >= perRow)
        {
            row = 1;
            col = index - perRow;
            inRow = n - perRow;
        }
        float span = pitch * (inRow - 1) + w + ox;
        float left = m_Look.BoutX - span * 0.5 + col * pitch;
        float top = m_Look.BoutY;
        if (rows == 2)
            top = m_Look.BoutY - 66 + row * 118;
        cx = left + w * 0.5;
        cy = top + h * 0.5;
        turn = 0;
        if (defence)
        {
            cx = cx + ox;
            cy = cy + oy;
            turn = 12;
        }
    }

    // The middle of the stock's back number `k` from the bottom (PaintTable).
    protected void StockLayerAt(int k, out float sx, out float sy)
    {
        sx = m_Look.StockX - k * 1.2 + m_Look.PileW * 0.5;
        sy = m_Look.StockY - k * 1.6 + m_Look.PileH * 0.5;
    }

    // How many backs the stock shows for so many cards in it: the trump lies
    // apart, and six backs at most.
    protected int StockLayers(int stockCount)
    {
        int layers = stockCount - 1;
        if (layers > 6)
            layers = 6;
        return layers;
    }

    // A card of the seat `no`'s fan as the view `v` lays it out: the index-th
    // of `count` backs, or the fan's middle (index -1). Null when that seat has
    // no fan there.
    OZG_CardSpot SpotOfFan(OZG_WView v, int no, int index, int count)
    {
        float x;
        float y;
        float turn;
        float scale;
        if (!FanOf(v, no, x, y, turn, scale))
            return null;
        float w = m_Look.FanW * scale;
        float h = m_Look.FanH * scale;
        if (index < 0 || count < 1)
            return OZG_CardSpot.Make(x, y, w, h, turn);
        float cx;
        float cy;
        float a;
        FanCardAt(x, y, turn, scale, count, index, cx, cy, a);
        return OZG_CardSpot.Make(cx, cy, w, h, a);
    }

    // The index-th of `count` cards of the player's own hand, faces, in the
    // order the window shows them (SortHand).
    OZG_CardSpot SpotOfHand(int index, int count)
    {
        float x;
        float y;
        float a;
        HandCardAt(count, index, x, y, a);
        return OZG_CardSpot.Make(x, y, m_Look.CardW, m_Look.CardH, a);
    }

    // The top of a stock of `stockCount` cards: where a card dealt or drawn
    // leaves from; with no back left on it, the trump's place.
    OZG_CardSpot SpotOfStock(int stockCount)
    {
        int layers = StockLayers(stockCount);
        if (layers < 1)
            return SpotOfTrump();
        float sx;
        float sy;
        StockLayerAt(layers - 1, sx, sy);
        return OZG_CardSpot.Make(sx, sy, m_Look.PileW, m_Look.PileH, 0);
    }

    // The trump, turned under the stock.
    OZG_CardSpot SpotOfTrump()
    {
        float tx = m_Look.TrumpX + m_Look.PileW * 0.5;
        float ty = m_Look.TrumpY + m_Look.PileH * 0.5;
        return OZG_CardSpot.Make(tx, ty, m_Look.PileW, m_Look.PileH, 90);
    }

    // The k-th back of the discards' heap.
    OZG_CardSpot SpotOfDiscards(int k)
    {
        float dx;
        float dy;
        float dt;
        OZG_TableLook.HeapAt(k, dx, dy, dt);
        float hx = m_Look.DiscardX + dx;
        float hy = m_Look.DiscardY + dy;
        return OZG_CardSpot.Make(hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt);
    }

    // The index-th attack card of a bout of `count` cards, or the card that
    // beat it.
    OZG_CardSpot SpotOfBout(int count, int index, bool defence)
    {
        float cx;
        float cy;
        float w;
        float h;
        float turn;
        BoutCardAt(count, index, defence, cx, cy, w, h, turn);
        return OZG_CardSpot.Make(cx, cy, w, h, turn);
    }

    // How many backs the heap shows for so many cards discarded: a back for
    // every two, OZG_TableLook.HEAP at most.
    int HeapOf(int discardCount)
    {
        int heap = (discardCount + 1) / 2;
        if (heap > OZG_TableLook.HEAP)
            heap = OZG_TableLook.HEAP;
        return heap;
    }

    // The pictures a flight wears: a card's face in a look, a look's back
    // (-1: the deck's own).
    string PictureOfFace(string card, int look)
    {
        return PictureOf("c_" + OZG_Cards.FaceOf(card), look);
    }

    string PictureOfBack(int look)
    {
        return PictureOf("back", look);
    }

    // ------------------------------------------------------------ flights and shakes

    // A card flies over the table (OZG_Flights): `picture` on its face at
    // first, `pictureLand` from halfway ("" keeps it), from `from` to `to`,
    // after `delay` seconds, for `seconds`. The card drawn under the key
    // `reveal` is hidden until it lands. Nothing waits for it, and a newer
    // view lands it at once. The card sharp's fourth phase flies the sleights
    // with this.
    OZG_Flight Fly(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        if (!m_Air || !from || !to)
            return null;
        Widget card = MakeLayout(LAYOUT_CARD, m_Air, "OZG_Card");
        if (!card)
            return null;
        ImageWidget face = ImageWidget.Cast(card.FindAnyWidget("OZG_CardFace"));
        Widget halo = card.FindAnyWidget("OZG_CardGlow");
        if (face)
        {
            face.LoadImageFile(0, picture);
            face.SetImage(0);
        }
        if (halo)
            halo.Show(false);
        if (reveal != "")
            HideSlot(reveal);
        return m_Flights.Launch(card, pictureLand, from, to, delay, seconds, reveal);
    }

    // The card drawn under `key` waits, hidden, for one more flight.
    void HideSlot(string key)
    {
        m_Hidden.Insert(key);
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            if (m_DrawnKey[i] == key && m_DrawnW[i])
                m_DrawnW[i].Show(false);
        }
    }

    // One flight onto `key` landed: the card drawn there shows once no other
    // is on its way to it.
    protected void RevealSlot(string key)
    {
        m_Hidden.RemoveItem(key);
        if (m_Hidden.Find(key) >= 0)
            return;
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            if (m_DrawnKey[i] == key && m_DrawnW[i])
                m_DrawnW[i].Show(true);
        }
    }

    // Every card in the air at once where it lands, and every hidden card
    // shown: a newer view, a look built anew, the window closing.
    protected void LandAll()
    {
        int n = m_Flights.Count();
        array<string> landed = new array<string>();
        m_Flights.Finish(landed);
        for (int i = 0; i < landed.Count(); i++)
            RevealSlot(landed[i]);
        m_Hidden.Clear();
        if (n > 0)
            OZG_Log.Dbg("flights: " + n.ToString() + " landed at once");
    }

    // A small shake of a region of the table: a seat's fan (RegionOfSeat),
    // the discards (REGION_DISCARDS) or the stock (REGION_STOCK). What a
    // player who saw a sleight is shown (card sharp's design §6.1).
    void ShakeRegion(string region)
    {
        if (region == "")
            return;
        int cards = 0;
        for (int i = 0; i < m_DrawnRegion.Count(); i++)
        {
            if (m_DrawnRegion[i] == region)
                cards++;
        }
        m_Shakes.Start(region);
        OZG_Log.Dbg("shake " + region + ": " + cards.ToString() + " cards");
    }

    // The cards in the air, one frame on; a landed card shows its key.
    protected void TickAir(float dt)
    {
        if (m_Flights.Count() == 0)
            return;
        array<string> landed = new array<string>();
        m_Flights.Tick(dt, m_K, landed);
        for (int i = 0; i < landed.Count(); i++)
            RevealSlot(landed[i]);
        if (m_Flights.Count() == 0)
            OZG_Log.Dbg("flights: all landed");
    }

    // The shaking regions, one frame on: their cards moved off their rest,
    // and put back when the shake ends.
    protected void TickShakes(float dt)
    {
        if (!m_Shakes.Busy())
            return;
        array<string> ended = new array<string>();
        m_Shakes.Tick(dt, ended);
        float dx;
        float dy;
        for (int i = 0; i < m_DrawnW.Count(); i++)
        {
            string region = m_DrawnRegion[i];
            Widget w = m_DrawnW[i];
            if (region == "" || !w)
                continue;
            bool moving = m_Shakes.Offset(region, dx, dy);
            if (!moving && ended.Find(region) < 0)
                continue;
            w.SetPos(m_DrawnX[i] + dx * m_K, m_DrawnY[i] + dy * m_K);
        }
    }

```

(j) Replace the whole method `BottomSeat` (its comment stays) with

```c
    protected int BottomSeat()
    {
        return BottomOf(m_V);
    }
```

(k) In `PaintSeats` replace

```c
    protected void PaintSeats()
    {
        ClearList(m_FanPics);
```

with

```c
    protected void PaintSeats()
    {
        ForgetDrawn(m_FanPics);
        ClearList(m_FanPics);
```

and

```c
        array<int> round = new array<int>();
        for (int k = 1; k < n; k++)
        {
            int no = (bottom + k) % n;
            if (m_V.Seats[no].St != OZG_Const.SEAT_FREE)
                round.Insert(no);
        }
```

with

```c
        array<int> round = new array<int>();
        RoundOf(m_V, bottom, round);
```

(l) In `PaintFan` replace the one line

```c
            Widget card = Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true);
```

with

```c
            // A flight's slot and the seat's region (OZG_Flights.c).
            Widget card = Card(m_FanPics, back, cx, cy, w, h, a + turn, 0, true, SlotOfFan(seat, i), RegionOfSeat(seat));
```

`PaintFan` keeps its own arithmetic; `FanCardAt` repeats it for the flights, and the two must agree (Task 6's shots show a card landing exactly on its back).

(m) In `PaintTable` replace

```c
    protected void PaintTable()
    {
        ClearList(m_TablePics);
```

with

```c
    protected void PaintTable()
    {
        ForgetDrawn(m_TablePics);
        ClearList(m_TablePics);
```

and, each a part of one line (whatever else stands on that line stays, such as a `Widget ... =` the card sharp's phase 3 put before a call):

- `Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true);` becomes `Card(m_TablePics, PictureOf(trump, m_Pub.TrumpLook), tx, ty, pw, ph, 90, 0, true, SLOT_TRUMP, REGION_STOCK);`
- `Card(m_TablePics, Picture(suit), tx, ty, 56, 78, 0, 0, false);` becomes `Card(m_TablePics, Picture(suit), tx, ty, 56, 78, 0, 0, false, "", REGION_STOCK);`
- `Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true);` becomes `Card(m_TablePics, PictureOf("back", look), sx, sy, pw, ph, 0, 0, true, "", REGION_STOCK);`
- `Card(m_TablePics, PictureOf("back", -1), hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt, 0, true);` becomes `Card(m_TablePics, PictureOf("back", -1), hx, hy, m_Look.DiscardW, m_Look.DiscardH, dt, 0, true, SlotOfHeap(k), REGION_DISCARDS);`

`PaintTable` keeps its own arithmetic of the stock's backs and the heap; `StockLayers`, `StockLayerAt`, `SpotOfTrump` and `HeapOf` repeat it for the flights, and they must agree.

(n) In `PaintBout`:

- `Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true);` becomes `Card(m_TablePics, PictureOf(attackFace, attackLook), cx, cy, w, h, 0, glow, true, SlotOfBout(i, false));`
- `Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true);` becomes `Card(m_TablePics, PictureOf(beatenFace, beatenLook), cx + ox, cy + oy, w, h, 12, 0, true, SlotOfBout(i, true));`

`BoutCardAt` repeats `PaintBout`'s arithmetic for the flights, and the two must agree.

(o) After the method `SortKey` add

```c

    // A hand in the order the window shows it (SortKey). The flights find a
    // card's place in it.
    void SortHand(array<string> held, string trumpSuit, array<string> into)
    {
        into.Clear();
        if (!held)
            return;
        into.Copy(held);
        int n = into.Count();
        for (int i = 1; i < n; i++)
        {
            string moved = into[i];
            int key = SortKey(moved, trumpSuit);
            int j = i - 1;
            while (j >= 0 && SortKey(into[j], trumpSuit) > key)
            {
                into[j + 1] = into[j];
                j--;
            }
            into[j + 1] = moved;
        }
    }
```

In `PaintHand` replace

```c
    protected void PaintHand()
    {
        ClearList(m_HandPics);
```

with

```c
    protected void PaintHand()
    {
        ForgetDrawn(m_HandPics);
        ClearList(m_HandPics);
```

replace its sort

```c
        array<string> cards = new array<string>();
        cards.Copy(m_Own.Held);
        int n = cards.Count();
        int i;
        int j;
        for (i = 1; i < n; i++)
        {
            string moved = cards[i];
            int key = SortKey(moved, trumpSuit);
            j = i - 1;
            while (j >= 0 && SortKey(cards[j], trumpSuit) > key)
            {
                cards[j + 1] = cards[j];
                j--;
            }
            cards[j + 1] = moved;
        }
```

with

```c
        // The flights find a card's place by the same order (SortHand).
        array<string> cards = new array<string>();
        SortHand(m_Own.Held, trumpSuit, cards);
        int n = cards.Count();
        int i;
```

and the part of a line `Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true);` with `Card(m_HandPics, PictureOf(cardFace, cardLook), x, y, m_Look.CardW, m_Look.CardH, a, glow, true, SlotOfHand(i));`. `HandCardAt` repeats `PaintHand`'s arithmetic for the flights, and the two must agree.

- [ ] **Step 4: The `shake` message shakes its region**

In `OZG_MissionGameplay.c` (Edit tool) replace

```c
            if (shake.SoundName != "")
                OZG_Sounds.PlayLocal(shake.SoundName, where);
            return;
```

with

```c
            if (shake.SoundName != "")
                OZG_Sounds.PlayLocal(shake.SoundName, where);
            if (menu && menu.PlaceId() == shake.PlaceId && shake.RegionName != "")
                menu.ShakeRegion(shake.RegionName);
            return;
```

- [ ] **Step 5: Lint, compile, build, look**

Before the build, `knowledge_find(kind="class", name=...)` for `Launch`, `Tick`, `Finish`, `Count`, `Start`, `Busy`, `Offset`, `Clear`, `Fly`: none may be a class of the game or of a mod the stand loads (CF, the core, VPP, SFP, the NewHorizon mods). `mod_lint` (it reads the two `.layout` too), `client_compile_check` = `pass`, a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`. Then:

```
oz_games {"op":"lay","at":"6097.4 107.65 6028.9","from":"6095.4 107.65 6028.9","mode":"2","deck":"OZG_Deck_Classic"}
oz_games {"op":"sit","id":"<id>","seat":"0"}
oz_games {"op":"bots","id":"<id>","n":"2","on":"0"}
oz_games {"op":"ask","id":"<id>","ask":"deal"}
oz_games {"op":"sess","id":"<id>"}
```

`sess` says which seats the bots hold (`seats=0:1,...`, 1 taken). Open the window (`client_type` with `j`), `client_shot`. The table is drawn as before this task: the two bots' fans of six backs, the stock with the trump turned under it, the plates, the hand of six faces with the playable ones lit (compare with a shot of the table window record or the marks record: the same places). `ui_tree(root="OZG_Air", depth=1)` answers one node, the layer itself, `visible` true and empty.

The shakes (the window open):

```
oz_games {"op":"shake","id":"<id>","region":"seat<n>","sound":"lay"}
oz_games {"op":"shake","id":"<id>","region":"stock"}
oz_games {"op":"shake","id":"<id>","region":"discards"}
oz_games {"op":"shake","id":"<id>","region":"seat7"}
```

(`<n>` a bot's seat.) Right after the first, `client_shot` twice, as fast as the tool answers, and once a second later. Expected in the client's log: `sound lay on local: started=1` and `shake seat<n>: 6 cards`; `shake stock: 7 cards` (six backs and the trump, three players dealt 18 of 36); `shake discards: 0 cards` (nothing discarded yet); `shake seat7: 0 cards` (no such seat held). The first shots show that bot's fan a few pixels off where the last shot has it (at 1920 x 1080 a unit is a pixel: up to four); nothing else moved. The fan is back exactly at rest on the last shot.

The stalkers' look: `oz_games {"op":"ask","id":"<id>","who":"me","ask":"fold"}` (the host folds); lay again with `"deck":"OZG_Deck_Stalker"`, `sit`, `bots`, `deal`, the window, `client_shot`: the stalkers' table as before; one `shake region=seat<n>`, `client_shot`. Fold.

Send the owner the shots (`SendUserFile`).

- [ ] **Step 6: Commit**

```bash
git add OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_Flights.c tools/make_layouts.py OpenZone_Games/gui/layouts/ozg_win_classic.layout OpenZone_Games/gui/layouts/ozg_win_stalker.layout OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_MissionGameplay.c
git commit -m "Flights: cards in the air and small shakes over the game window

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 6: A durak table's cards fly where the new view puts them

When a newer view of the same game arrives, the window draws it as before, then flies what moved: at a deal, from the stock a seat at a time while the deal's run sounds, the trump turned up as it ends; a card laid or beaten, from its seat (from its place in the player's own hand) onto the table; a bout that closes, its cards to the discards or into the taker's hand; cards drawn, from the stock into each hand in drawing order, the last of the stock being the trump; a dropped seat's hand to the discards. A card another seat holds flies as a back, and turns face up halfway when it lands face up. A view that comes while cards are in the air lands them first.

**Files:**
- Create: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_FlightPlan.c`
- Modify: `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c` (`Redraw`)

**Interfaces:**
- Consumes: from Task 5 every public name of `OZG_GameMenu` listed there and `OZG_CardSpot`; `OZG_Sounds.LengthOf`, `OZG_SoundIds.SHUFFLE`, `DEAL` (Tasks 1, 2); `OZG_WView`, `OZG_DurakPublic`, `OZG_DurakPrivate`, `OZG_Cards.OfMove`, `SuitOf`.
- Produces: `class OZG_FlightPlan` with `static void Make(OZG_GameMenu menu, OZG_WView wasV, OZG_DurakPublic was, OZG_DurakPrivate wasOwn, OZG_WView nowV, OZG_DurakPublic now, OZG_DurakPrivate nowOwn)`; a debug line a view `flights rev <r>: dealt n, trump 1 | laid n, swept n | taken n, drawn n`.

- [ ] **Step 1: The plan of a view's flights**

Create `OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_FlightPlan.c`:

```c
// The flights of a durak table (design §11, §17): what moved between the view
// the window showed and the one it shows now flies from where it was to where
// it is. The window has drawn the new view already; a card landing where a
// card is drawn hides that card until it lands (OZG_GameMenu.Fly).
//
// What moved is read from three things: the lines the log gained (who laid
// which card, a seat dropped), the bout (did it go on or close), and the
// counts (the stock, the discards, every hand). A card nobody at this table
// can know (another seat's draw) flies as a back; a view that came after
// several changes is flown as far as those three things tell.
class OZG_FlightPlan
{
    static const float ONE = 0.35;        // a card laid, beaten or moved on
    static const float GROUP = 0.45;      // the table swept off or taken
    static const float GROUP_GAP = 0.04;  // between two cards of the group
    static const float DRAW = 0.35;       // a card drawn
    static const float DRAW_GAP = 0.09;   // between two cards drawn
    static const float DEAL = 0.30;       // a card dealt
    static const float LAID_GAP = 0.08;   // between two cards laid in one view
    // More lines than this gained at once: too much happened to tell who laid
    // what, and only the counts are flown.
    static const int MOST_LINES = 6;

    protected OZG_GameMenu m_Menu;
    protected OZG_WView m_WasV;
    protected OZG_DurakPublic m_Was;
    protected OZG_DurakPrivate m_WasOwn;
    protected OZG_WView m_NowV;
    protected OZG_DurakPublic m_Now;
    protected OZG_DurakPrivate m_NowOwn;
    protected string m_Trump;
    protected int m_Flown;
    protected string m_Said;

    // The cards laid in this change, as the log names them: the seat, the
    // card, and the attack card it beat (from 0; -1 for an attack card).
    protected ref array<int> m_LaidSeat;
    protected ref array<string> m_LaidCard;
    protected ref array<int> m_LaidOn;
    protected int m_Dropped;
    // My new hand as the window shows it, the places a card already flies
    // to, the cards I took, the cards I drew.
    protected ref array<string> m_Hand;
    protected ref array<int> m_HandUsed;
    protected ref array<string> m_Taken;
    protected ref array<string> m_MineDrawn;

    void OZG_FlightPlan()
    {
        m_LaidSeat = new array<int>();
        m_LaidCard = new array<string>();
        m_LaidOn = new array<int>();
        m_Hand = new array<string>();
        m_HandUsed = new array<int>();
        m_Taken = new array<string>();
        m_Dropped = -1;
    }

    static void Make(OZG_GameMenu menu, OZG_WView wasV, OZG_DurakPublic was, OZG_DurakPrivate wasOwn, OZG_WView nowV, OZG_DurakPublic now, OZG_DurakPrivate nowOwn)
    {
        if (!menu || !wasV || !nowV || !now)
            return;
        OZG_FlightPlan p = new OZG_FlightPlan();
        p.m_Menu = menu;
        p.m_WasV = wasV;
        p.m_Was = was;
        p.m_WasOwn = wasOwn;
        p.m_NowV = nowV;
        p.m_Now = now;
        p.m_NowOwn = nowOwn;
        p.RunPlan();
    }

    protected void RunPlan()
    {
        if (m_NowV.PlaceId != m_WasV.PlaceId)
            return;
        if (m_NowV.GameId != OZG_DurakConst.GAME_ID)
            return;
        if (m_NowV.St == OZG_Const.ST_LOBBY)
            return;
        m_Trump = OZG_Cards.SuitOf(m_Now.TrumpCard);
        if (m_NowOwn)
            m_Menu.SortHand(m_NowOwn.Held, m_Trump, m_Hand);
        bool dealt = m_NowV.St == OZG_Const.ST_PLAYING && m_WasV.St != OZG_Const.ST_PLAYING;
        if (dealt)
            FlyDeal();
        else if (m_Was && m_WasV.St == OZG_Const.ST_PLAYING)
            FlyMoves();
        if (m_Flown > 0)
            OZG_Log.Dbg("flights rev " + m_NowV.Rev.ToString() + ": " + m_Said);
    }

    // ------------------------------------------------------------ small things

    protected void FlyOne(string picture, string pictureLand, OZG_CardSpot from, OZG_CardSpot to, float delay, float seconds, string reveal)
    {
        if (!from || !to)
            return;
        if (m_Menu.Fly(picture, pictureLand, from, to, delay, seconds, reveal))
            m_Flown++;
    }

    protected void SayCount(string what, int n)
    {
        if (n <= 0)
            return;
        if (m_Said != "")
            m_Said = m_Said + ", ";
        m_Said = m_Said + what + " " + n.ToString();
    }

    protected static int LookIn(array<int> looks, int i)
    {
        if (!looks || i < 0 || i >= looks.Count())
            return -1;
        return looks[i];
    }

    protected static OZG_DurakSeatPublic SeatIn(OZG_DurakPublic pub, int no)
    {
        if (!pub || !pub.SeatList)
            return null;
        for (int i = 0; i < pub.SeatList.Count(); i++)
        {
            if (pub.SeatList[i].SeatNo == no)
                return pub.SeatList[i];
        }
        return null;
    }

    protected static int CountIn(OZG_DurakPublic pub, int no)
    {
        OZG_DurakSeatPublic g = SeatIn(pub, no);
        if (!g)
            return 0;
        return g.CardCount;
    }

    // The look of a card of a hand of mine (HeldLooks lies beside Held).
    protected static int MyLook(OZG_DurakPrivate own, string card)
    {
        if (!own || !own.Held)
            return -1;
        return LookIn(own.HeldLooks, own.Held.Find(card));
    }

    // The place of `card` in my new hand that no flight lands on yet; -1 when
    // it is not there.
    protected int HandPlaceOf(string card)
    {
        for (int i = 0; i < m_Hand.Count(); i++)
        {
            if (m_Hand[i] != card || m_HandUsed.Find(i) >= 0)
                continue;
            m_HandUsed.Insert(i);
            return i;
        }
        return -1;
    }

    // ------------------------------------------------------------ the deal

    // A new deal: the shuffle is heard first (OZG_SoundIds.SHUFFLE), then the
    // cards fly from the stock a seat at a time, as durak deals them, through
    // the run the deal sounds; the trump turns up as the run ends.
    protected void FlyDeal()
    {
        float start = OZG_Sounds.LengthOf(OZG_SoundIds.SHUFFLE) / 1000.0;
        float run = OZG_Sounds.LengthOf(OZG_SoundIds.DEAL) / 1000.0;
        int total = 0;
        int s;
        for (s = 0; s < m_Now.SeatList.Count(); s++)
            total = total + m_Now.SeatList[s].CardCount;
        if (total == 0)
            return;
        float gap = 0.05;
        if (total > 1)
            gap = (run - DEAL) / (total - 1);
        if (gap < 0.02)
            gap = 0.02;
        OZG_CardSpot from = m_Menu.SpotOfStock(m_Now.StockCount);
        int k = 0;
        for (s = 0; s < m_Now.SeatList.Count(); s++)
        {
            OZG_DurakSeatPublic g = m_Now.SeatList[s];
            if (g.SeatNo == m_NowV.Mine)
            {
                for (int h = 0; h < m_Hand.Count(); h++)
                {
                    int myLook = MyLook(m_NowOwn, m_Hand[h]);
                    string myBack = m_Menu.PictureOfBack(myLook);
                    string myFace = m_Menu.PictureOfFace(m_Hand[h], myLook);
                    OZG_CardSpot mine = m_Menu.SpotOfHand(h, m_Hand.Count());
                    FlyOne(myBack, myFace, from, mine, start + k * gap, DEAL, OZG_GameMenu.SlotOfHand(h));
                    k++;
                }
                continue;
            }
            for (int i = 0; i < g.CardCount; i++)
            {
                OZG_CardSpot theirs = m_Menu.SpotOfFan(m_NowV, g.SeatNo, i, g.CardCount);
                string theirBack = m_Menu.PictureOfBack(LookIn(g.Backs, i));
                FlyOne(theirBack, "", from, theirs, start + k * gap, DEAL, OZG_GameMenu.SlotOfFan(g.SeatNo, i));
                k++;
            }
        }
        SayCount("dealt", k);
        // The trump, turned up as the run ends; six seats take the whole deck
        // and then it is the last card dealt, in a hand.
        if (m_Now.TrumpCard == "" || m_Now.StockCount == 0)
            return;
        string trumpBack = m_Menu.PictureOfBack(m_Now.TrumpLook);
        string trumpFace = m_Menu.PictureOfFace(m_Now.TrumpCard, m_Now.TrumpLook);
        FlyOne(trumpBack, trumpFace, from, m_Menu.SpotOfTrump(), start + run, DEAL, OZG_GameMenu.SLOT_TRUMP);
        SayCount("trump", 1);
    }

    // ------------------------------------------------------------ a change in play

    // The lines the log gained since the view shown before. The log keeps its
    // last OZG_Const.LOG_LINES lines: the fewest lines added that make the two
    // agree are the new ones.
    protected void ReadLog()
    {
        array<string> was = m_WasV.Log;
        array<string> now = m_NowV.Log;
        int nw = was.Count();
        int nn = now.Count();
        int added = -1;
        for (int k = 0; k <= nn && added < 0; k++)
        {
            int keep = nn - k;
            if (keep > nw)
                continue;
            bool same = true;
            for (int i = 0; i < keep && same; i++)
            {
                if (now[i] != was[nw - keep + i])
                    same = false;
            }
            if (same)
                added = k;
        }
        if (added < 0 || added > MOST_LINES)
            return;
        for (int j = nn - added; j < nn; j++)
            ReadLine(now[j]);
    }

    // "<seat> a:<card>", "<seat> t:<card>", "<seat> d:<card>:<n>" are cards
    // laid; "drop <seat>" a seat whose cards went to the discards.
    protected void ReadLine(string line)
    {
        int space = line.IndexOf(" ");
        if (space <= 0)
            return;
        string head = line.Substring(0, space);
        string rest = line.Substring(space + 1, line.Length() - space - 1);
        if (head == "drop")
        {
            m_Dropped = rest.ToInt();
            return;
        }
        if (rest.Length() < 3)
            return;
        string kind = rest.Substring(0, 2);
        if (kind != "a:" && kind != "t:" && kind != "d:")
            return;
        string card = OZG_Cards.OfMove(rest);
        if (card == "")
            return;
        int target = -1;
        if (kind == "d:")
        {
            array<string> parts = new array<string>();
            rest.Split(":", parts);
            if (parts.Count() == 3)
                target = parts[2].ToInt() - 1;
        }
        m_LaidSeat.Insert(head.ToInt());
        m_LaidCard.Insert(card);
        m_LaidOn.Insert(target);
    }

    protected void FlyMoves()
    {
        ReadLog();
        int wasN = m_Was.AttackCards.Count();
        int nowN = m_Now.AttackCards.Count();
        bool goesOn = nowN >= wasN;
        for (int a = 0; a < wasN && goesOn; a++)
        {
            if (m_Now.AttackCards[a] != m_Was.AttackCards[a])
                goesOn = false;
        }
        bool closed = wasN > 0 && !goesOn;

        // Cards laid: from their seats onto the table where they lie now. One
        // the table never showed (the bout closed in the same change) leaves
        // with the table.
        array<int> unseenSeat = new array<int>();
        array<string> unseenCard = new array<string>();
        float next = 0;
        int laid = 0;
        for (int c = 0; c < m_LaidCard.Count(); c++)
        {
            bool beats = m_LaidOn[c] >= 0;
            int where = -1;
            if (!closed)
                where = OnTable(m_LaidCard[c], m_LaidOn[c]);
            if (where < 0)
            {
                unseenSeat.Insert(m_LaidSeat[c]);
                unseenCard.Insert(m_LaidCard[c]);
                continue;
            }
            int look = TableLook(where, beats);
            string face = m_Menu.PictureOfFace(m_LaidCard[c], look);
            string first = FirstPicture(m_LaidSeat[c], m_LaidCard[c], look);
            OZG_CardSpot from = FromSeat(m_LaidSeat[c], m_LaidCard[c]);
            OZG_CardSpot to = m_Menu.SpotOfBout(nowN, where, beats);
            float at = laid * LAID_GAP;
            FlyOne(first, face, from, to, at, ONE, OZG_GameMenu.SlotOfBout(where, beats));
            next = at + ONE;
            laid++;
        }
        SayCount("laid", laid);

        int taker = -1;
        int taken = 0;
        if (closed || m_Dropped >= 0)
            next = FlyLeaving(closed, unseenSeat, unseenCard, next, taker, taken);
        FlyDraws(next, taker, taken);
    }

    // Where a card laid now lies on the new table: over the attack card it
    // beat (`target`, from 0), or the last attack card of that name; -1 when
    // it is not there.
    protected int OnTable(string card, int target)
    {
        if (target >= 0)
        {
            if (target < m_Now.DefenceCards.Count() && m_Now.DefenceCards[target] == card)
                return target;
            return -1;
        }
        for (int i = m_Now.AttackCards.Count() - 1; i >= 0; i--)
        {
            if (m_Now.AttackCards[i] == card)
                return i;
        }
        return -1;
    }

    protected int TableLook(int where, bool beats)
    {
        if (beats)
            return LookIn(m_Now.DefenceLooks, where);
        return LookIn(m_Now.AttackLooks, where);
    }

    // My card shows its face all the way; another seat's is a back until
    // halfway.
    protected string FirstPicture(int seat, string card, int look)
    {
        if (seat == m_NowV.Mine)
            return m_Menu.PictureOfFace(card, look);
        return m_Menu.PictureOfBack(look);
    }

    // Where a card a seat laid comes from: mine from its place in my hand as
    // it was, another seat's from the middle of its fan as it was.
    protected OZG_CardSpot FromSeat(int seat, string card)
    {
        if (seat == m_WasV.Mine && m_WasOwn)
        {
            array<string> before = new array<string>();
            m_Menu.SortHand(m_WasOwn.Held, m_Trump, before);
            int at = before.Find(card);
            if (at >= 0)
                return m_Menu.SpotOfHand(at, before.Count());
        }
        OZG_CardSpot fan = m_Menu.SpotOfFan(m_WasV, seat, -1, 0);
        if (fan)
            return fan;
        return m_Menu.SpotOfFan(m_NowV, seat, -1, 0);
    }

    // The table, and a dropped seat's hand, leave: to the discards, or into
    // the hand of the defender who took. Returns when the last card lands;
    // `taker` and `taken` say who took how many.
    protected float FlyLeaving(bool closed, array<int> unseenSeat, array<string> unseenCard, float start, out int taker, out int taken)
    {
        taker = -1;
        taken = 0;
        array<ref OZG_CardSpot> froms = new array<ref OZG_CardSpot>();
        array<string> cards = new array<string>();
        array<int> looks = new array<int>();
        int i;
        if (closed)
        {
            int wasN = m_Was.AttackCards.Count();
            for (i = 0; i < wasN; i++)
            {
                froms.Insert(m_Menu.SpotOfBout(wasN, i, false));
                cards.Insert(m_Was.AttackCards[i]);
                looks.Insert(LookIn(m_Was.AttackLooks, i));
                string beatBy = "";
                if (i < m_Was.DefenceCards.Count())
                    beatBy = m_Was.DefenceCards[i];
                if (beatBy == "")
                    continue;
                froms.Insert(m_Menu.SpotOfBout(wasN, i, true));
                cards.Insert(beatBy);
                looks.Insert(LookIn(m_Was.DefenceLooks, i));
            }
            for (i = 0; i < unseenCard.Count(); i++)
            {
                froms.Insert(FromSeat(unseenSeat[i], unseenCard[i]));
                cards.Insert(unseenCard[i]);
                looks.Insert(-1);
            }
        }
        if (m_Dropped >= 0)
        {
            OZG_DurakSeatPublic g = SeatIn(m_Was, m_Dropped);
            int held = 0;
            if (g)
                held = g.CardCount;
            for (i = 0; i < held; i++)
            {
                froms.Insert(m_Menu.SpotOfFan(m_WasV, m_Dropped, i, held));
                cards.Insert("");
                looks.Insert(LookIn(g.Backs, i));
            }
        }
        int n = froms.Count();
        if (n == 0)
            return start;
        bool swept = m_Now.DiscardCount > m_Was.DiscardCount;
        if (!swept)
        {
            taker = m_Was.DefenderSeat;
            taken = n;
            return FlyTaken(froms, cards, looks, start);
        }
        int heapWas = m_Menu.HeapOf(m_Was.DiscardCount);
        int heapNow = m_Menu.HeapOf(m_Now.DiscardCount);
        for (i = 0; i < n; i++)
        {
            int k = heapWas + i / 2;
            if (k >= heapNow)
                k = heapNow - 1;
            if (k < 0)
                k = 0;
            string down = m_Menu.PictureOfBack(looks[i]);
            string up = down;
            if (cards[i] != "")
                up = m_Menu.PictureOfFace(cards[i], looks[i]);
            FlyOne(up, down, froms[i], m_Menu.SpotOfDiscards(k), start + i * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfHeap(k));
        }
        SayCount("swept", n);
        return start + (n - 1) * GROUP_GAP + GROUP;
    }

    // The table taken: into my hand, at the places the cards hold there now;
    // or into the taker's fan, as its newest backs.
    protected float FlyTaken(array<ref OZG_CardSpot> froms, array<string> cards, array<int> looks, float start)
    {
        int n = froms.Count();
        int taker = m_Was.DefenderSeat;
        if (taker == m_NowV.Mine)
        {
            for (int i = 0; i < n; i++)
            {
                if (cards[i] == "")
                    continue;
                m_Taken.Insert(cards[i]);
                int place = HandPlaceOf(cards[i]);
                if (place < 0)
                    continue;
                int myLook = MyLook(m_NowOwn, cards[i]);
                string myFace = m_Menu.PictureOfFace(cards[i], myLook);
                OZG_CardSpot mine = m_Menu.SpotOfHand(place, m_Hand.Count());
                FlyOne(myFace, "", froms[i], mine, start + i * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfHand(place));
            }
        }
        else
        {
            int count = CountIn(m_Now, taker);
            int drew = DrewNow(taker, taker, n);
            int firstIdx = count - drew - n;
            if (firstIdx < 0)
                firstIdx = 0;
            OZG_DurakSeatPublic g = SeatIn(m_Now, taker);
            for (int j = 0; j < n; j++)
            {
                int idx = firstIdx + j;
                if (idx >= count)
                    idx = count - 1;
                int backLook = -1;
                if (g)
                    backLook = LookIn(g.Backs, idx);
                string down = m_Menu.PictureOfBack(backLook);
                string up = down;
                if (cards[j] != "")
                    up = m_Menu.PictureOfFace(cards[j], looks[j]);
                OZG_CardSpot fan = m_Menu.SpotOfFan(m_NowV, taker, idx, count);
                FlyOne(up, down, froms[j], fan, start + j * GROUP_GAP, GROUP, OZG_GameMenu.SlotOfFan(taker, idx));
            }
        }
        SayCount("taken", n);
        return start + (n - 1) * GROUP_GAP + GROUP;
    }

    // How many cards the seat `no` drew in this change: what its hand gained,
    // with the cards it laid given back and the cards it took kept out.
    protected int DrewNow(int no, int taker, int taken)
    {
        int gained = CountIn(m_Now, no) - CountIn(m_Was, no);
        for (int i = 0; i < m_LaidSeat.Count(); i++)
        {
            if (m_LaidSeat[i] == no)
                gained++;
        }
        if (no == taker)
            gained = gained - taken;
        if (gained < 0)
            gained = 0;
        return gained;
    }

    // The seats in the order they draw: from the first attacker of the bout
    // round the table, the defender last (durak's rule 8).
    protected void DrawOrder(array<int> order)
    {
        order.Clear();
        int n = m_Was.SeatList.Count();
        int from = 0;
        for (int i = 0; i < n; i++)
        {
            if (m_Was.SeatList[i].SeatNo == m_Was.OpenerSeat)
                from = i;
        }
        for (int k = 0; k < n; k++)
        {
            int no = m_Was.SeatList[(from + k) % n].SeatNo;
            if (no != m_Was.DefenderSeat)
                order.Insert(no);
        }
        if (m_Was.DefenderSeat >= 0)
            order.Insert(m_Was.DefenderSeat);
    }

    // Cards drawn after a bout or a drop, each from the top of the stock as it
    // was; the last card of the stock is the trump. A dropped seat draws none.
    protected void FlyDraws(float start, int taker, int taken)
    {
        int drawn = m_Was.StockCount - m_Now.StockCount;
        if (drawn <= 0)
            return;
        array<int> order = new array<int>();
        DrawOrder(order);
        int k = 0;
        for (int i = 0; i < order.Count(); i++)
        {
            int no = order[i];
            if (no == m_Dropped)
                continue;
            int got = DrewNow(no, taker, taken);
            for (int j = 0; j < got && k < drawn; j++)
            {
                bool last = k == drawn - 1 && m_Now.StockCount == 0;
                FlyDrawn(no, got, j, k, last, start);
                k++;
            }
        }
        SayCount("drawn", k);
    }

    // The j-th of the `got` cards the seat `no` drew, the k-th drawn of all.
    protected void FlyDrawn(int no, int got, int j, int k, bool last, float start)
    {
        OZG_CardSpot from = m_Menu.SpotOfStock(m_Was.StockCount - k);
        if (last)
            from = m_Menu.SpotOfTrump();
        float at = start + k * DRAW_GAP;
        if (no == m_NowV.Mine)
        {
            string card = MineDrawn(j);
            int place = -1;
            if (card != "")
                place = HandPlaceOf(card);
            if (place < 0)
                return;
            int myLook = MyLook(m_NowOwn, card);
            string myFace = m_Menu.PictureOfFace(card, myLook);
            string myFirst = m_Menu.PictureOfBack(myLook);
            if (last)
                myFirst = myFace;
            OZG_CardSpot mine = m_Menu.SpotOfHand(place, m_Hand.Count());
            FlyOne(myFirst, myFace, from, mine, at, DRAW, OZG_GameMenu.SlotOfHand(place));
            return;
        }
        int count = CountIn(m_Now, no);
        int idx = count - got + j;
        if (idx < 0)
            idx = 0;
        OZG_DurakSeatPublic g = SeatIn(m_Now, no);
        int backLook = -1;
        if (g)
            backLook = LookIn(g.Backs, idx);
        string down = m_Menu.PictureOfBack(backLook);
        string up = down;
        if (last)
            up = m_Menu.PictureOfFace(m_Now.TrumpCard, m_Now.TrumpLook);
        OZG_CardSpot fan = m_Menu.SpotOfFan(m_NowV, no, idx, count);
        FlyOne(up, down, from, fan, at, DRAW, OZG_GameMenu.SlotOfFan(no, idx));
    }

    // The j-th card I drew in this change: my new hand, less what I held
    // before and what I took, in the order my hand is shown.
    protected string MineDrawn(int j)
    {
        if (!m_MineDrawn)
        {
            m_MineDrawn = new array<string>();
            array<string> known = new array<string>();
            if (m_WasOwn && m_WasOwn.Held)
                known.Copy(m_WasOwn.Held);
            for (int t = 0; t < m_Taken.Count(); t++)
                known.Insert(m_Taken[t]);
            for (int i = 0; i < m_Hand.Count(); i++)
            {
                int at = known.Find(m_Hand[i]);
                if (at >= 0)
                {
                    known.Remove(at);
                    continue;
                }
                m_MineDrawn.Insert(m_Hand[i]);
            }
        }
        if (j < 0 || j >= m_MineDrawn.Count())
            return "";
        return m_MineDrawn[j];
    }
}
```

- [ ] **Step 2: The window flies after a newer view**

In `OZG_GameMenu.c`, in `Redraw`, replace

```c
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
        m_V = view.HeadPart;
        m_ShownRev = m_V.Rev;
```

with

```c
        OZG_TableView view = OZG_Table.Of(m_PlaceId);
        if (!view)
            return;
        // The view shown until now, for the flights: in locals, so that it
        // lives through this call while the members take the new one.
        OZG_WView wasV = m_V;
        OZG_DurakPublic wasPub = m_Pub;
        OZG_DurakPrivate wasOwn = m_Own;
        bool newer = false;
        if (wasV && view.HeadPart.Rev > m_ShownRev)
            newer = true;
        m_V = view.HeadPart;
        m_ShownRev = m_V.Rev;
```

replace

```c
        if (style != m_Style || !m_Win)
            Build(style);
        if (!m_Win)
            return;
```

with

```c
        bool rebuilt = false;
        if (style != m_Style || !m_Win)
        {
            Build(style);
            rebuilt = true;
        }
        if (!m_Win)
            return;
        // A newer view lands every card still in the air before it is drawn:
        // nothing flies on to a place the new view may have moved.
        if (newer)
            LandAll();
```

and replace the method's last lines

```c
        PaintLog();
        PaintColumn();
    }
```

with

```c
        PaintLog();
        PaintColumn();

        // What moved since the view shown before flies to where it is now:
        // only for a newer view of the game in the same look.
        if (newer && !rebuilt && durak)
            OZG_FlightPlan.Make(this, wasV, wasPub, wasOwn, m_V, m_Pub, m_Own);
    }
```

(`durak` is the local `Redraw` already declares.)

- [ ] **Step 3: Lint, compile, build, watch the cards fly**

`knowledge_find(kind="class", name="Make")`: no class of the game or of a loaded mod. `mod_lint`, `client_compile_check` = `pass`, a stand cycle, `client_start(language="English")`, `job_wait`, `world_ready`, `world_teleport "6095.4 107.65 6028.9"`, then `lay` (classic), `sit` seat 0, `bots` `n=2` `on=0` (they wait), and open the window (`client_type` with `j`).

**The deal** (design §17's quick run): `oz_games {"op":"ask","id":"<id>","ask":"deal"}`, then at once `client_shot` four times as fast as the tool answers, `ui_tree(root="OZG_Air", depth=1)` once in between, and `client_shot` and `ui_tree` again four seconds later. Expected:
- the first `ui_tree`: the layer and 19 cards (18 dealt to three seats and the trump), some `visible` false (not set off yet);
- the shots: during the shuffle (about 1.3 s) the stock alone and no fans; then backs flying from the stock to each bot's fan and backs turning into the player's own faces on their way to his hand, a seat at a time; the trump turning up under the stock last;
- four seconds later: the layer alone, the table complete as Task 5 drew it;
- the client's log: `flights rev <r>: dealt 18, trump 1`, then `flights: all landed`; the sounds shuffle, deal, trump of Task 4 under it.

**Moves**: `oz_games {"op":"auto","id":"<id>","n":"1"}`, then `client_shot` at once and twice more. Each time the client's log says `flights rev <r>: laid 1` (an attack, a throw-in, a beat: the card from that seat's fan, or from the player's own hand, onto its place on the table; another seat's card a back that turns face up halfway). Go on with `auto n=1` until:
- a bout swept: `flights rev <r>: ... swept <n>, drawn <m>`: the table's cards to the heap (faces turning down), then backs from the stock into the hands, the first attacker first, the defender last; when the closing card was a beat the table never showed, it is one of the `<n>` and flies from the defender straight to the heap;
- a bout taken: `... taken <n>, drawn <m>`: the table into the taker's fan (or the player's hand, faces where they now lie in it);
- the stock's last card: the trump flies from under the stock into a hand (`drawn` counts it).
Send the shots of each kind to the owner.

**A view in mid-flight**: `oz_games {"op":"auto","id":"<id>","n":"1"}` twice in one go (two calls without a wait). Expected in the client's log: `flights: 1 landed at once` (or more) before the second `flights rev`; `ui_tree(root="OZG_Air", depth=1)` two seconds later: the layer alone.

**The window closed in mid-deal**: fold (`oz_games {"op":"ask","id":"<id>","who":"me","ask":"fold"}`); lay; `sit` seat 0; `bots n=2 on=0`; open the window (`j`); `ui_find(name="OZG_BtnClose")` gives the close button's path; `oz_games {"op":"ask","id":"<id>","ask":"deal"}` and at once `ui_click(path=<that path>, expect_name="OZG_BtnClose")` (the click goes to the window's own handler, no foreground needed). Expected in the client's log: `flights: <n> landed at once` as the window closes. Open it again (`j`): `ui_tree(root="OZG_Air", depth=1)` shows the layer alone and the table is whole, every card shown.

**Perevodnoy**: fold; lay; `sit` seat 0; `bots n=2 on=0`; `oz_games {"op":"ask","id":"<id>","ask":"opt","opt":"variant","val":"perevodnoy"}`; deal; `auto n=1` until the attack is moved on (`t:` in the window's log): `flights rev <r>: laid 1`, the card onto the table beside the attack, which is now the next seat's to answer.

**A watcher, and a dropped seat** (a player who stands up has his window closed, so a drop is watched): fold; lay; `oz_games {"op":"bots","id":"<id>","n":"3","on":"0"}` (three bots and no player: the first bot to sit is the host); `oz_games {"op":"seats","id":"<id>"}` names the bots' uids. `world_action(action_class="OZG_ActionWatch", target_class="OZG_Place")` opens the watcher's window (the table window record says a watcher's window was never opened live: if the action is refused, write down its answer and see the rest from a seat instead). `oz_games {"op":"ask","id":"<id>","who":"<the host bot's uid>","ask":"deal"}`: the deal flies to three fans, the bottom one too (the first seat held, at the hand's place), and `ui_tree` shows 19 cards in the air. Then the host removes another bot with its cards discarded: `oz_games {"op":"ask","id":"<id>","who":"<the host bot's uid>","ask":"kick","seat":"<that bot's seat>","how":"discard"}` → `flights rev <r>: swept <n>, drawn <m>`: that bot's fan and the table to the heap, then the draws.

**The stalkers' look, and Ukrainian**: the same deal and a few moves with `"deck":"OZG_Deck_Stalker"` (shots); then `client_stop`, `client_start(language="russian")`, one deal and one move in the classic look (shots): the flights carry no text, the window's own words are the same Ukrainian as before.

Send the owner the key shots: the deal in the air, a card laid, a sweep, a take, the stalkers' table.

- [ ] **Step 4: Commit**

```bash
git add OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_FlightPlan.c OpenZone_Games/scripts/5_Mission/OpenZone_Games/OZG_GameMenu.c
git commit -m "Flights: a durak table's cards fly where the new view puts them

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

### Task 7: The record

What was built and measured goes where the next reader looks for it.

**Files:**
- Create: `docs/2026-11-25-sounds-and-flights-record.md`
- Modify: `docs/test-commands.md`, `README.md`, this plan (its ticks)
- Outside the repository: `E:\openzone\docs\specs\2026-11-23-openzone-games-design.md` (§17's status), `E:\openzone\docs\specs\2026-11-24-openzone-games-cardsharp-design.md` (§10, one line); the skills `E:\dayzmod\dayz-agentic-modding-mcp\skills\dayz-modding` and `E:\dayzmod\dayz-modding-skill\openzone`; the memory `project-table-games.md`

- [ ] **Step 1: The record**

Create `docs/2026-11-25-sounds-and-flights-record.md` in the shape of `docs/2026-11-25-marks-record.md`: a heading `# Sounds and flights of cards: record`, the date and what the owner asked, and these sections, each filled from the steps' answers and never from this plan's expectations:
- **What was built**: a table, a task a row, with its commit.
- **Measured on the stand**: the ten sounds' lengths, sizes and digests (Task 1, Step 4); `started=1` for each (Task 2); when a client first reads a place's number, at the place's coming or at its first change (Task 3, Step 5, the `first sound number` line); the two gaps of a deal's lot; whether two lots in quick succession were both heard; the late client; the bench's rows (Task 4, Step 6) and the fuzz; the live game's lots, the stand-up, the gathering; the shakes' card counts and the shots (Task 5); the deal's `ui_tree` counts, the flights of each kind, a view in mid-flight, the window closed in mid-deal, the watcher, both looks, Ukrainian (Task 6).
- **Where the build differs from the design**: the number carries up to three sounds, not one ("the last sound"), so that a deal and a closing move are one lot played in order; it goes quiet a second after each lot; the deck, not the place, carries the gathering's sound (the place is deleted in the same frame); a deck made anew for a place that held none is not heard; the client half of the card sharp's `shake` built before its fourth phase (the stand sends it); the deal's cards wait for the shuffle's length and spread over the run's; a card another seat holds flies as a back; the drawing order of the flights after a drop starts from the opener, not from the seat after the dropped one; what else came up.
- **Not done**: the sleights' own sounds (the fourth phase adds them to `tools/make_sounds.py`'s table); the owner's ear on the loudness of each sound.

- [ ] **Step 2: What a person tries**

In `docs/test-commands.md`, before `## The verb for scripts`, add:

```markdown
## Sounds and flights

Every change of a game is heard by every client near the place, whether its window
is open or not: the deck put down when a game is laid out and gathered up when it is
folded, the shuffle, the deal's quick run and the trump turned up, a card laid on the
table and a card laid over it, the table taken or swept to the discards, cards drawn
from the stock. The sounds are the mod's own, made from numbers by
`tools/make_sounds.py`. With the core's debug switch on, a client's script log names
every sound it plays (`dbg: sound lay on place <id>: started=1`).

In the window the cards fly: from the stock into every hand at a deal, while the deal
is heard, the trump turned up last; from a hand onto the table; from the table to the
discards or into the taker's hand; from the stock into the hands after a bout. The
log says what flew (`dbg: flights rev 12: laid 1, swept 4, drawn 3`).

    oz_games {"op":"sound","id":"<id>"}                              the place's sound number: its count, what it carries now, its last lot
    oz_games {"op":"sound","id":"<id>","play":"shuffle,deal,trump"}  a lot put on the place, as a deal puts it
    oz_games {"op":"sound","in":"me"}                                the number of the deck in your hands (gathered up)
    oz_games {"op":"shake","id":"<id>","region":"seat1","sound":"lay"}   what a player who saw a sleight will be sent: the sound for him alone, seat 1's fan shaken
    oz_games {"op":"heard"}                                          the bench: what its game made heard since the last ask

The regions of a shake are `seat<n>`, `discards` and `stock`.
```

In the paragraph under `## The verb for scripts`: in its list of the world's ops, after `` `face`, `` add `` `sound`, `shake`, ``; in its sentence on the ops of the rules alone (`` (`begin`, `play`, `fuzz` and the rest) ``) add `` `heard` `` after `` `fuzz` ``. (The card sharp's phase 3 adds its own ops to the same paragraph; keep them.)

- [ ] **Step 3: The README**

In `README.md`, in the status list, after the bullet `- **Marks**: ...` (the whole bullet, and the bullet of noticing when the card sharp's phase 3 has added one after it), add:

```markdown
- **Sounds and flights**: a game is heard by everybody near it, players, watchers and
  passers-by: the deck put down and gathered up, the shuffle, the deal's quick run,
  the trump turned up, a card laid and a card laid over it, the table taken or swept
  to the discards, cards drawn. Every sound is the mod's own, made by a script from
  numbers. In the window the cards fly: from the stock into the hands at a deal and a
  draw, from a hand onto the table, from the table to the discards or into the taker's
  hand.
```

In the `## What is where` table replace

```markdown
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, the table's textures and fonts, models |
```

with

```markdown
| `OpenZone_Games/` | the mod: scripts, config, layouts, card imagesets, the table's textures and fonts, models, sounds |
```

in the row of `tools/`, right after the entry of `make_marks.py` (whatever its words are then), add `` `make_sounds.py` (the table's sounds, made from numbers), ``, and replace

```markdown
| `art/` | sources of the cards, of the tables (`art/table`) and the fonts (Oswald, PT Mono; SIL Open Font License; their list in `art/fonts/fonts.json`) |
```

with

```markdown
| `art/` | sources of the cards, of the tables (`art/table`) and the fonts (Oswald, PT Mono; SIL Open Font License; their list in `art/fonts/fonts.json`); how the sounds are made (`art/sounds/SOURCES.md`) |
```

In `## Українською`, after the sentence `Ручка й лупа поки що мають моделі самої гри.` (and after what the card sharp's phase 3 added right after it, if anything) add:

```markdown
Гру чути всім поруч — гравцям, глядачам і перехожим: колоду кладуть на килимок і
збирають, тасують, швидко роздають, відкривають козир, карту кладуть на стіл і б'ють
іншою, стіл забирає той, хто відбивався, або його змітають у відбій, добирають карти з
колоди. Кожен звук — власний: його робить скрипт моду з чисел, без жодного запису. У
вікні карти летять: з колоди в руки під час роздачі й добору, з руки на стіл, зі столу
у відбій або в руку того, хто бере.
```

- [ ] **Step 4: The designs' status**

In `E:\openzone\docs\specs\2026-11-23-openzone-games-design.md` (not a repository), §17: "Not built yet." becomes the date it was built and checked, the record's path, and the open line (the sleights' sounds wait for the card sharp's fourth phase; the loudness is the owner's ear). In the card sharp's design, §10, one line: the shake, the local sound and the flight are built (`OZG_GameMenu.ShakeRegion`, `OZG_Sounds.PlayLocal`, `OZG_GameMenu.Fly`), and the client already handles `shake`.

- [ ] **Step 5: The skills**

Each fact measured here about the engine goes into the public skill with neutral names (`MyMod_`, "a stand mod"): into `references/items-and-processing.md`, by "Custom sounds for devices", what Tasks 1 to 3 measured (a mono Ogg Vorbis from ffmpeg's `libvorbis` plays from a mod's shader; `DZ_Sounds_Effects` is needed for shaders and sets that inherit the game's `baseCharacter_*`, now measured; what `SoundPlay` answered; 1.30's own item sounds go as an object RPC once a frame, `StartItemSoundServer` → `FlushItemSoundQueue`, the netsync id being legacy); into `references/persistence-networking.md`, whether a client gets `OnVariablesSynchronized` when an entity comes into its world (Task 3's `first sound number` line) and the pattern of a synchronised count with a quiet window. The private `openzone` skill gets the pointers to our code (`OZG_Sounds.c`, `OZG_Flights.c`, `OZG_FlightPlan.c`, `tools/make_sounds.py`) in `references/ui.md` and `references/networking-and-items.md`. Then `python -I openzone/scripts/check-generic.py` from `E:\dayzmod\dayz-modding-skill` prints nothing. The skill repositories hold other people's uncommitted changes: stage only this task's hunks, commit each repository once, do NOT push.

- [ ] **Step 6: The memory**

`C:\Users\56869\.claude\projects\E--openzone\memory\project-table-games.md` gains a paragraph: sounds and flights built (the date, the commits), what was measured, what is open; its `description` line says so. `MEMORY.md` stays under 17.1 KB.

- [ ] **Step 7: Commit**

```bash
git add docs/2026-11-25-sounds-and-flights-record.md docs/test-commands.md README.md docs/2026-11-25-sounds-and-flights-plan.md
git commit -m "Sounds and flights: the record, the commands to try them, the README

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

Do NOT push: the owner alone gives the push word.
