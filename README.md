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
- - [ ] Giant spider
- - [ ] lots more sprites
- - [ ] Spell of Animation
- - [ ] Spell of Trailhead Teleportation
- - [ ] Spell of Home Teleportation
- - [ ] Spell of Slow Motion
- - [ ] Spell of Invisibility
- - [ ] fmn_password_check_business_rules(): reenable once ready
- [ ] Map editor.
- - [x] Some kind of tool to validate point doors, normally one expects a mutual link. A button to jump to its target?
- - xxx "new" for all types
- - xxx Delete map
- - xxx Gamepad controls for MapEditor? :D that would be so cool
- - xxx Tilesheet, cache group lists etc
- - [x] Setting negative poi.q (eg edge door), highly painful today.
- - [ ] mapcvt validate FMN_POI_EDGE_DOOR aligns to screen sizes.
- - [ ] is there some venue where we could validate that edge doors are mutually consistent?
- - [x] sidebar: Make each resource type an accordion
- - [x] sprite editor
- [ ] Design
- - [ ] Fill forest with puzzles
- - [ ] Dead space in home NE, what should go there?
- - [ ] Houses in home, what are they for?
- - [ ] caves
- - [ ] swamp
- - [ ] desert
- - [ ] castle
- - - [x] open outer door
- - - [ ] the two tablesaw right next to each other -- no way is this possible on a thumby, nerf it
- - - [ ] populate upper floors
- - [ ] win-game ui
- [ ] Before releasing for Thumby, validate FMN_PASSWORD_SEQUENCE hard. Is it possible to violate sequence?
- [ ] Audio
- - [ ] Low quality synth suitable for Tiny
- - [ ] Higher quality synth for PCs?
- - [ ] WebAudio
- - [ ] Consider a limited set of sound effects for Thumby and Pico.
- - [ ] write music
- - - [ ] splash: Full Moon Theme (TODO catchier name)
- - - [ ] home: tbd
- - - [ ] forest: tbd
- - - [ ] caves: Infinite Stories Deep
- - - [ ] desert: tbd
- - - [ ] swamp: tbd
- - - [ ] castle: The Seven Circles of a Witch's Soul
- - - [ ] denouement
- - - [ ] game over (same as splash?)
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
