# config.mk
# Defines things likely to change from one build host to another.
# Only a generic reference version should be committed; normally we will gitignore this file.

# What hosts are we building for?
# "linux" is fairly generic, should be the template for other PCs.
FMN_TARGETS:=linux thumby web

thumby_PICO_SDK:=/home/andy/proj/thirdparty/pico-sdk
thumby_ELF2UF2:=$(thumby_PICO_SDK)/build/elf2uf2/elf2uf2
thumby_GCCPFX:=arm-none-eabi-
thumby_MOUNTPOINT:=/media/andy/RPI-RP2

run:linux-run

edit:;node src/editor/main.js --host=localhost --port=2040 --htdocs=$(abspath src/editor/www)
