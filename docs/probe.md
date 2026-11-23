# Table games probe

Stand tooling for the table games research (`docs/specs/2026-11-23-table-games-research.md`
beside the repositories of the series, §8). Not part of the mod and never published: it
asks the engine which surfaces a placement hologram lands on and what it costs to work out
the places around one. It came into this repository from `E:\openzone\table-games-probe`
as it was, names and all.

## What is here

| Where | What |
|---|---|
| `TableGamesProbe/` | client and server: the rays (`TGP_Scan.c`), the ring of places (`TGP_Seat.c`), the live hologram's report and the stand-in deck (`TGP_Client.c`), the furniture sweep (`TGP_Sweep.c`), the ops (`TGP_Verb.c`) |
| `TableGamesProbe_Bridge/` | server only: the `tgp` verb for the MCP bridge |
| `tools/analyze.py` | reads what the probe wrote: `area`, `catalog`, `sweep` |
| `docs/measurements/2026-11-23/` | the rows of 2026-11-23, what `analyze.py` made of them, and four screenshots |

## Running it

The probe is among the project's mods (`dayz-mcp.toml`): `project_open` the repository,
`mod_build`, `server_start`. The stand boots on a world of its own,
`E:\dayzmod\testserver\serverDZ_games.cfg`, `instanceId = 83`, and always with CF and the
core loaded: a world saved with CF does not survive a boot without it. After every boot
check the `-config=` line and `Selected storage directory` in the RPT.

Everything is driven with `world_exec verb=tgp`; every value of `args` is a string.

| `op` | Does |
|---|---|
| `status`, `cancel` | the running job |
| `col` `pos="x z"` | one column: every surface under a point, by the three geometry kinds |
| `area` `pos="x z" r= pitch= up=` | a square of the world, column by column, into `area_*.csv` |
| `catalog` `prefix= limit= spot="x z"` | every config class of a prefix stood on the spot and read from above, into `catalog_*.csv` |
| `seat` `pos="x y z" label= repeat=` | the ring of places around a point, timed |
| `sweep` `prefix= limit= spot=` | every class of a prefix under the first player's live hologram, into `holo_sweep.csv`; the player needs `TGP_Deck` in hands |
| `show` `class=`, `top` `move=1` | one thing left standing on the spot; its top and a place beside it |
| `ask` `do= text=`, `told` | an op for the first player's client and the line it said back: `state`, `seat`, `aim`, `aimplace`, `place`, `unplace`, `holo`, `wipe` |

Rows land in `<stand profile>\TableGamesProbe\`; copy them before the next run of the same op.
