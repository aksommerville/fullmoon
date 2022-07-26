# config.mk
# Defines things likely to change from one build host to another.
# Only a generic reference version should be committed; normally we will gitignore this file.

# What hosts are we building for?
# "linux" is fairly generic, should be the template for other PCs.
#FMN_TARGETS:=linux thumby web pico
FMN_TARGETS:=tiny

thumby_PICO_SDK:=/home/andy/proj/thirdparty/pico-sdk
thumby_ELF2UF2:=$(thumby_PICO_SDK)/build/elf2uf2/elf2uf2
thumby_GCCPFX:=arm-none-eabi-
thumby_MOUNTPOINT:=/media/andy/RPI-RP2

pico_PICO_SDK:=/home/andy/proj/thirdparty/pico-sdk
pico_ELF2UF2:=$(thumby_PICO_SDK)/build/elf2uf2/elf2uf2
pico_GCCPFX:=arm-none-eabi-
pico_MOUNTPOINT:=/media/andy/RPI-RP2

tiny_PORT:=ttyACM0
tiny_IDEROOT:=/home/andy/pkg/arduino-1.8.19
tiny_IDEVERSION:=10819
tiny_BUILDER_OPTS:=
tiny_PKGROOT:=$(wildcard ~/.arduino15/packages)
tiny_LIBSROOT:=$(wildcard ~/Arduino/libraries)
tiny_MENU_BIN:=etc/ArcadeMenu.ino.bin
tiny_BUILDER:=$(tiny_IDEROOT)/arduino-builder

run:linux-run

edit:;node src/editor/main.js --host=localhost --port=2040 --htdocs=$(abspath src/editor/www)
