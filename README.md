# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

It also builds for Linux, to facilitate development.
I do intend to extend that, and also build for WebAssembly, MacOS, and Windows.
But not a high priority.

## TODO

- [x] thumby timing
- [x] genioc timing
- [ ] Is there some kind of vsync signal on Thumby? I get tearing, visible if you strobe b/w really fast.
- [ ] Map editor.
- - [x] Launch game?
- - [x] Rainbow pencil
- - [x] POI
- - [x] Set tilesheet
- - [x] Tilesheet decode and make available to map editor
- - [x] Tilesheet editor
- - [x] Sort resources list.
- - [x] Show images when selected, or filter from list.
- - [ ] Some kind of tool to validate point doors, normally one expects a mutual link. A button to jump to its target?
- - [x] Resize map
- - [x] New map
- - [ ] Delete map
- - [ ] MapEditor: zoom
- - [x] MapToolbox: nearest neighbor palette, don't interpolate
- - [ ] Gamepad controls for MapEditor? :D that would be so cool
- - [ ] Tilesheet, cache group lists etc
- [ ] Build tools
- - [ ] mapcvt: Sort and validate POI.
- [ ] Game logic.
- - [x] broom
- - [x] wand
- - [x] umbrella
- - [x] static collisions
- - [ ] Blit with xform. I think we do need it.
- - [ ] monsters
- - [ ] Edge doors
- - [ ] Point doors
- - [ ] POI triggers
- - [ ] environmental puzzles
- - [ ] password/state
- [x] evdev
- [ ] Build for WebAssembly.
- [ ] Web app to wrap the wasm build.
- [ ] MacOS drivers (new IoC unit)
- [ ] Windows drivers (genioc)
- [ ] Audio: Are we doing audio at all? The Thumby is pretty limited.
- [ ] PulseAudio driver. If we're still doing audio.
- [ ] Can I make a TTY driver for video and input?
- [ ] Can we make this run on a 68k Mac?
