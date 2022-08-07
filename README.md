# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

It also builds for Linux, to facilitate development.
I do intend to extend that, and also build for WebAssembly, MacOS, and Windows.
But not a high priority.

## Setup

Create `etc/config.mk` if it's not there.
In there, you must define `FMN_TARGETS:=` with the names of targets you want to build for.
See `etc/make/target_*.mk`.
Most targets will want some additional config, location of toolchains etc.

Making a new target? Best idea is to copy `etc/make/target_linux.mk`.
It's stable and pretty generic.
Define `{TARGET}_OPT_ENABLE` with the names of directories under `src/opt/` to include in the source.
You must include one opt unit with a `main()`: `genioc`, `macos`, `thumby`.
Or if you need to write your own `main()`, you can make a new opt unit for it.
See `src/game/fullmoon.h` for a few other hooks you must implement there.

Targets must individually declare the rules for generating data sources, even data like maps that won't change from platform to platform.

## TODO

- [ ] Finish all dev for Thumby
- - [x] Is there some kind of vsync signal on Thumby? I get tearing, visible if you strobe b/w really fast. ...meh not a big deal
- - [ ] Sprite: werewolf
- - [x] Sprite: table saw
- - [ ] lots more sprites
- - [ ] Rain should extinguish fires
- - [ ] Can we prevent rain from working indoors?
- - [ ] Spell of Opening
- - [ ] Spell of Animation
- - [ ] Spell of Trailhead Teleportation
- - [ ] Spell of Home Teleportation
- - [ ] Spell of Slow Motion
- - [ ] Spell of Invisibility
- [ ] Map editor.
- - [ ] Some kind of tool to validate point doors, normally one expects a mutual link. A button to jump to its target?
- - [ ] Delete map
- - [ ] Gamepad controls for MapEditor? :D that would be so cool
- - [ ] Tilesheet, cache group lists etc
- - [ ] Setting negative poi.q (eg edge door), highly painful today.
- - [ ] mapcvt validate FMN_POI_EDGE_DOOR aligns to screen sizes.
- [ ] Design
- - [ ] Fill forest with puzzles
- - [ ] Dead space in home NE, what should go there?
- - [ ] Houses in home, what are they for?
- - [ ] caves
- - [ ] swamp
- - [ ] desert
- - [ ] castle
- [ ] Before releasing for Thumby, validate FMN_PASSWORD_SEQUENCE hard. Is it possible to violate sequence?
- [ ] Audio
- - [ ] Low quality synth suitable for Tiny
- - [ ] Higher quality synth for PCs?
- - [ ] WebAudio
- - [ ] Consider a limited set of sound effects for Thumby and Pico.
- - [ ] write music
- [ ] Other platforms
- - [ ] pico: firewall does something, flickering and middle tiles out of place... the hell?
- - [ ] tiny: 8c graphics -- finish 8b first
- - [ ] tiny: content for SD card. splash and readme
- - [ ] 24c graphics -- finish 8c first
- - [ ] linux: PulseAudio
- - [ ] machid
- - [ ] macaudio
- - [ ] macos: Can we drop OpenGL
- - [ ] macos: review deprecated WM functions
- - [ ] macos: Update for hi-res
- - [ ] mswin
- - [ ] web: Clean up wrapper
- - [ ] web: Reduce exported symbols
- - [ ] web: Can we replace ScriptProcessorNode with AudioWorklet? Or scrap that whole design and write the synth in WebAudio?
- - [ ] TTY video, and.... something? for input. just for fun
- - [ ] 68k Mac?
- - [ ] Pippin?
- - [ ] Best way to run on mobile devices, native or web?
