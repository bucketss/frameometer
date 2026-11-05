# frameometer

AMXX module that measures each player's client fps by counting usercmds in a
`pfnCmdStart` hook. The last 60 seconds are kept per player and survive map changes.

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

## fpspeek

`sma/fpspeek.sma`: `amx_fps [name|#userid]` (ADMIN_KICK), the same table as `fps_peek` in the client console.

## Build

Windows:

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFRAMEOMETER_AMXX_ROOT=C:/Users/servor/c/amxmodx
cmake --build build
```

Linux: same commands with g++ multilib. The AMXX root defaults to `../amxmodx`.

The release `.so` is built in the `quay.io/pypa/manylinux2014_i686` container
(glibc 2.17 floor), so it loads on old and new distros alike.

## Install

Copy `frameometer_amxx.dll` / `frameometer_amxx_i386.so` to `addons/amxmodx/modules/`
and `frameometer.inc` to `scripting/include/`. Compile `fpspeek.sma`,
and add `fpspeek.amxx` to `plugins.ini`.
