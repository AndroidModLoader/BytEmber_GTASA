# GTA:SA examples

Each `.es` has compiled `.ebc` bytecode for GTA:SA 2.00 ARM32 and 2.10 ARM64.
Run `./examples/build.ps1` to rebuild with the supplied v6 engine; the root
`build.ps1` also runs it after building the libraries (`-SkipExamples` skips it).
It changes no engine source. Includes point to `../include/aml-psdk/`. Install
bytecode built by the same BytEmber version as the installed mod.

Copy selected gameplay `.ebc` files into
`Android/data/<game package>/bytember/`. Copy `global/*.ebc` files into its
`global/` child. The host creates both directories. Source and headers are needed
only for compilation. Gameplay scripts restart on new game/save load; global
scripts retain state and run in menus. Pick examples individually: running both
health examples together restores full health every second. `armour_regen`
also needs `player_lib.ebc` in the same directory.

| Example | Behavior and feature |
| --- | --- |
| `HealthRegen` | Per-frame `onUpdate`, timer GOT reads, health helpers; heals after seven seconds without damage. |
| `player_health` | `onUpdate` with a game-time interval; restores full health each second. |
| `player_status` | `onUpdate` logs health, armour and the standing bitfield every five game seconds. |
| `frame_clock` | Counts gameplay frames and reports game time/timestep every five seconds. |
| `movement` | Writes an embedded movement vector once; gives the player a small upward velocity. |
| `weapons` | `onUpdate` iterates the fixed weapon array and maintains at least 500 ammunition for non-unarmed weapons. |
| `player_target` | `onUpdate` follows a native pointer; heals the targeted living ped at 10 HP per game second and maintains armour at 100. |
| `world_matrix` | Exposes a class-qualified game call through `GetBoneMatrix`; empty `main` has no effect. Needs valid frame/matrix pointers from a caller. |
| `infinite_stamina` | `onUpdate` follows `m_pPlayerData` and calls `CStats`; keeps sprint stamina and underwater breath at the stat maximum. |
| `vehicle_repair` | `onUpdate` with `FindPlayerVehicle`; keeps the current vehicle at 1000 health so its engine never burns. Panel damage stays. |
| `always_day` | Gameplay `main()` loop with `wait()`; every two game seconds sets 12:00 when the clock leaves 07:00-20:00. |
| `player_lib` | Exports-only module: `GetPlayerArmour()` (-1 without a living player) and `SetPlayerArmour()`. Does nothing alone. |
| `armour_regen` | `require`s `player_lib`; adds 1 armour per game second up to 100. |
| `global/heartbeat` | Logs every five seconds using the persistent global scheduler. |
| `global/fps_monitor` | Global timer GOT reads; logs FPS and the pause state every ten wall-clock seconds, also in menus. |

`tests/psdk_examples.cpp` simulates the new examples against ARM64 layouts and
links every committed `.ebc` against the typed adapters for both ABIs.
Native game examples require the Android game process. `world_matrix` is a
function-call example; this host currently dispatches only `main` and
`onUpdate`, so it does not invoke `GetBoneMatrix` automatically. Bytecode
compilation and host simulation do not verify Android game execution.
