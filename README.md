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
- - [ ] Spell of Invisibility
- - [ ] fmn_password_check_business_rules(): reenable once ready
- - [ ] We are likely to breach the size limit for Pico and Tiny. What can we do to reduce size?
- - [ ] Cosmetic feedback for animation spell. Grow flowers or something?
- - [ ] Retain password on Quit
- - [ ] Retain action on Continue
- [ ] Second pass populating the world
- - [ ] swamp: First half needs some hazards or puzzles
- - [ ] treadle to reset alpha blocks
- [ ] Before releasing for Thumby, validate FMN_PASSWORD_SEQUENCE hard. Is it possible to violate sequence?
- [ ] Audio
- - [ ] songcvt: loop point
- - [x] Condense delays
- - [x] Trim for the Logic bug
- - [ ] Configurable waves for minisyni
- - [ ] minisyni foley
- - [x] Why are some notes so much louder than others?
- - [ ] Low quality synth suitable for Tiny
- - [ ] Higher quality synth for PCs?
- - [ ] WebAudio
- - [ ] Consider a limited set of sound effects for Thumby and Pico.
- - [ ] write music
- - - [ ] splash: Full Moon Theme (TODO catchier name)
- - - [ ] home: The Forever Mule
- - - [ ] forest: Lela Marien
- - - [ ] caves: Infinite Stories Deep
- - - [ ] desert: Only on Full Moons
- - - [ ] swamp: The People Under the Garden
- - - [ ] castle: The Seven Circles of a Witch's Soul
- - - [ ] denouement
- - - [ ] game over (same as splash?)
- [ ] Other platforms
- - [ ] tiny: 8c graphics -- finish 8b first
- - [ ] tiny: content for SD card. splash and readme
- - [ ] 24c graphics -- finish 8c first
- - [ ] linux: PulseAudio
- - [ ] macaudio
- - [ ] mswin
- - [ ] web: Clean up wrapper
- - [ ] web: Reduce exported symbols
- - [ ] web: Can we replace ScriptProcessorNode with AudioWorklet? Or scrap that whole design and write the synth in WebAudio?
- - [ ] TTY video, and.... something? for input. just for fun
- - [ ] 68k Mac?
- - [ ] Pippin?
- - [ ] Best way to run on mobile devices, native or web?

## Passwords

HWDDBD home () ()
FURRRT home (feather,wand,broom,umbrella) ()
RE3QT1 castle (feather,wand,broom,umbrella) (basement door)
RG1ZT1 castle (feather,wand,broom,umbrella) (door,wolf dead)
VU3QRS caves (feather,wand,broom,umbrella) (basement door)
XUDDRC caves (feather) ()
BEBTT3 desert (feather,wand,broom) ()
BUBTR3 swamp (feather,wand,broom) ()
