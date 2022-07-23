# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

It also builds for Linux, to facilitate development.
I do intend to extend that, and also build for WebAssembly, MacOS, and Windows.
But not a high priority.

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
- - [ ] Key flags, display at pause.
- - [ ] Start point in global state (eg 3 bits, with 8 hard coded starting maps)
- - [ ] I haven't been exploiting POI sort, eg looking for treadles we linear-search the whole list.
- [ ] Audio: Are we doing audio at all? The Thumby is pretty limited.
- [ ] PulseAudio driver. If we're still doing audio.
- [ ] Build for other targets
- - [ ] MacOS -- just drivers
- - [ ] Windows -- just drivers
- - [ ] WebAssembly, highly desirable
- - - [ ] Build wasm
- - - [ ] Wrapper web app
- - [ ] TTY video, and.... something? for input. just for fun
- - [ ] Tiny?
- - [ ] Pico?
- - [ ] 68k Mac?
- - [ ] Pippin?
