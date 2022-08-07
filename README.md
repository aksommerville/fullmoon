# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

Also for TinyArcade, PicoSystem, Linux, MacOS, and WebAssembly.
Will eventually add Windows and maybe Playdate, if it ever arrives...
Also I do intend to at least give it a shot, building for 68k and PowerPC MacOS (and thence Pippin!).

## Setup

Check `etc/config.mk`, that will hold your host-specific settings.
If you're building for the lil guys, you'll need to indicate where the various toolchains and SDKs are located.

Try `make run`

## TODO

- [ ] Finish all dev for Thumby
- - [x] Sprite: werewolf
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
- - [ ] tiny: 8c graphics -- finish 8b first
- - [ ] tiny: content for SD card. splash and readme
- - [ ] 24c graphics -- finish 8c first
- - [ ] linux: PulseAudio
- - [ ] machid
- - [ ] macaudio
- - [ ] macos: Can we drop OpenGL
- - [ ] macos: review deprecated WM functions
- - [ ] mswin
- - [ ] web: Clean up wrapper
- - [ ] web: Reduce exported symbols
- - [ ] web: Can we replace ScriptProcessorNode with AudioWorklet? Or scrap that whole design and write the synth in WebAudio?
- - [ ] TTY video, and.... something? for input. just for fun
- - [ ] 68k Mac?
- - [ ] Pippin?
- - [ ] Best way to run on mobile devices, native or web?
