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

- [ ] Is there some kind of vsync signal on Thumby? I get tearing, visible if you strobe b/w really fast.
- [ ] Map editor.
- - [ ] Some kind of tool to validate point doors, normally one expects a mutual link. A button to jump to its target?
- - [ ] Delete map
- - [x] MapEditor: zoom
- - [ ] Gamepad controls for MapEditor? :D that would be so cool
- - [ ] Tilesheet, cache group lists etc
- - [ ] Setting negative poi.q (eg edge door), highly painful today.
- - [ ] mapcvt validate FMN_POI_EDGE_DOOR aligns to screen sizes.
- [ ] Game logic.
- - [ ] monsters
- - [ ] environmental puzzles
- - [x] death
- - [ ] I haven't been exploiting POI sort, eg looking for treadles we linear-search the whole list.
- [ ] Audio: Are we doing audio at all? The Thumby is pretty limited.
- - YES. Not for Thumby or Pico, but it's worth having for Tiny, Web, and the PCs.
- - [ ] Consider a limited set of sound effects for Thumby and Pico.
- [ ] PulseAudio driver.
- [ ] Build for other targets
- - [ ] MacOS
- - - [ ] Review deprecated WM functions: -Wno-deprecated-declarations
- - - [ ] machid
- - - [ ] macaudio
- - - [ ] Can we drop OpenGL? It's deprecated, and really not necessary, if we can get a plain old window framebuffer. (see x11 or drmfb)
- - - [ ] Update for hi-res framebuffer
- - [ ] Windows -- just drivers
- - [x] WebAssembly, highly desirable
- - - [ ] Clean up web wrapper
- - - [ ] It currently exports every function. Can we limit that?
- - - [ ] Using ScriptProcessorNode for audio despite deprecation. How does AudioWorklet work?
- - [ ] TTY video, and.... something? for input. just for fun
- - [ ] 68k Mac?
- - [ ] Pippin?
- [ ] Review passwords. Seems like only one letter is changing with each treasure i pick up.
- - It should involve convolution somehow, so changing one letter forces the others to change (even if just 1 bit of plaintext).
- [ ] Design-rule validation for passwords, eg can't have the wand if you don't have the feather
- [ ] Some kind of fanfare, acknowledgement when you pick up a treasure.
