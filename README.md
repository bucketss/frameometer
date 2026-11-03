# frameometer

AMXX module. Measures each player's client fps by counting usercmds in a
`pfnCmdStart` hook, in 1 second windows of server time. The last 60 windows are
kept per player and survive map changes. Bots and HLTV are not counted.

Commands over 50 msec that ReHLDS splits in two are counted once, and lastcmd
replays filling packet loss are not counted. This needs
`sv_usercmd_custom_random_seed 0` (the default).

A window whose usercmd msec covers less than 90% of its server time lost
commands (packet loss, engine-dropped commands) and is discarded.

## Natives

See `include/frameometer.inc`.

```
Float:fps_get(player, windows = 1)   average over the last 1-60 windows
Float:fps_msec(player, windows = 1)  same windows, timed by usercmd msec
fps_windows(player)                  completed windows held, up to 60
fps_reset(player)                    clear history
```

## Server command

Server console or rcon:

```
fps_peek              list all players
fps_peek <name>       players whose name contains <name>
fps_peek #<userid>    one player
```

Shows the last window, the last window timed by usercmd msec, and the 5 and 30 window averages. `?` means no data yet.

## fpspeek

`sma/fpspeek.sma`: `amx_fps [name|#userid]` (ADMIN_KICK), the same table as `fps_peek` in the client console.

## Build

Windows (x86 Developer prompt):

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFRAMEOMETER_AMXX_ROOT=C:/Users/servor/c/amxmodx
cmake --build build
```

Linux: same commands with g++ multilib. The AMXX root defaults to `../amxmodx`.

## Install

Copy `frameometer_amxx.dll` / `frameometer_amxx_i386.so` to `addons/amxmodx/modules/`
and `frameometer.inc` to `scripting/include/`. The include autoloads the module.
Compile `fpspeek.sma` and add `fpspeek.amxx` to `plugins.ini`.
