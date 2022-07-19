# Full Moon

Adventure game for Thumby. [https://tinycircuits.com/]

It also builds for Linux, to facilitate development.
I do intend to extend that, and also build for WebAssembly, MacOS, and Windows.
But not a high priority.

## TODO

- [x] thumby timing
- [x] genioc timing
- [ ] I'm burning thru tilesheets with the multiple directions... Can we reconsider xform blits?
- [ ] Is there some kind of vsync signal on Thumby? I get tearing, visible if you strobe b/w really fast.
- [ ] Map editor.
- [ ] Game logic.
- - [x] broom
- - [x] wand
- - [x] umbrella
- - [x] static collisions
- - [ ] monsters
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
