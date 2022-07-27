# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

It also builds for Linux, to facilitate development.
I do intend to extend that, and also build for WebAssembly, MacOS, and Windows.
But not a high priority.

## Setup

Create `etc/config.mk` if it's not there (TODO: put a generic example in this repo).
In there, you must define `FMN_TARGETS:=` with the names of targets you want to build for.
See `etc/make/target_*.mk`.
Most targets will want some additional config, location of toolchains etc.

Making a new target? Best idea is to copy `etc/make/target_linux.mk`.
It's stable and pretty generic.
Define `{TARGET}_OPT_ENABLE` with the names of directories under `src/opt/` to include in the source.
You must include one opt unit with a `main()`: `genioc`, `macos`, `thumby`.
Or if you need to write your own `main()`, you can make a new opt unit for it.
See `src/game/fullmoon.h` for a few other hooks you must implement there.

Targets must individually declare the rules for generating data sources, but these will probably be identical everywhere.

## TODO

- [ ] Is there some kind of vsync signal on Thumby? I get tearing, visible if you strobe b/w really fast.
- [ ] Map editor.
- - [ ] Some kind of tool to validate point doors, normally one expects a mutual link. A button to jump to its target?
- - [ ] Delete map
- - [ ] MapEditor: zoom
- - [ ] Gamepad controls for MapEditor? :D that would be so cool
- - [ ] Tilesheet, cache group lists etc
- - [x] MapEditor: position tattle
- - [ ] Setting negative poi.q (eg edge door), highly painful today.
- - [ ] mapcvt validate FMN_POI_EDGE_DOOR aligns to screen sizes.
- [ ] Game logic.
- - [x] Blit with xform. I think we do need it.
- - [ ] monsters
- - [x] Edge doors
- - [x] Point doors
- - [x] POI triggers
- - [x] Proximity triggers
- - [ ] environmental puzzles
- - [ ] password/state
- - [ ] Flags to enable items -- you don't start with all 4.
- - [ ] Start point in global state (eg 3 bits, with 8 hard coded starting maps)
- - [ ] I haven't been exploiting POI sort, eg looking for treadles we linear-search the whole list.
- [ ] Audio: Are we doing audio at all? The Thumby is pretty limited.
- [ ] PulseAudio driver. If we're still doing audio.
- [x] 8x8x8 color framebuffer and images, for Tiny
- [x] 24x24x16 color framebuffer and images, for Pico and PCs
- [ ] Optimized blitters. It matters, now that we have more pixel formats.
- [ ] Build for other targets
- - [ ] MacOS
- - - [ ] Review deprecated WM functions: -Wno-deprecated-declarations
- - - [ ] machid
- - - [x] App icon, using Plunder Squad's for now
- - - [ ] Can we drop OpenGL? It's deprecated, and really not necessary, if we can get a plain old window framebuffer. (see x11 or drmfb)
- - [ ] Windows -- just drivers
- - [x] WebAssembly, highly desirable
- - - [x] Build wasm
- - - [x] Wrapper web app -- copied from ivand. works but definitely not ideal
- - [ ] TTY video, and.... something? for input. just for fun
- - [x] Tiny
- - [x] Pico
- - - [x] Runs at about 2/3 speed due to vsync. What can we do to work around that? (we have plenty of cpu to spare) ...skip every third vsync
- - [ ] 68k Mac?
- - [ ] Pippin?
