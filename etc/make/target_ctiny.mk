# target_ctiny.mk
# Replacement for target_tiny, not using arduino-builder.

# Run arduino-builder with '-verbose' to scrape its gcc invocation:
#$(ctiny_ARDUINO_HOME)/packages/arduino/tools/arm-none-eabi-gcc/7-2017q4/bin/arm-none-eabi-gcc
#  -mcpu=cortex-m0plus -mthumb -c -g -Os -Wall -Wextra -Wno-expansion-to-defined -std=gnu11 -ffunction-sections 
#  -fdata-sections -nostdlib --param max-inline-insns-single=500 -MMD 
#  -DF_CPU=48000000L -DARDUINO=10819 -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -D__SAMD21G18A__ -DUSB_VID=0x03EB -DUSB_PID=0x8009 
#  -DUSBCON "-DUSB_MANUFACTURER=\"TinyCircuits\"" "-DUSB_PRODUCT=\"TinyArcade\"" -DUSBCON -DCRYSTALLESS 
#  -I$(ctiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include/ 
#  -I$(ctiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS-Atmel/1.2.0/CMSIS/Device/ATMEL/ 
#  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino 
#  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade 
#  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/avr/dtostrf.c 
#  -o /home/andy/proj/fullmoon/mid/tiny/build-hosted/core/avr/dtostrf.c.o

ctiny_MIDDIR:=mid/ctiny
ctiny_OUTDIR:=out/ctiny

ctiny_OPT_ENABLE:=tiny synth

ctiny_TOOLCHAIN:=$(ctiny_ARDUINO_HOME)/packages/arduino/tools/arm-none-eabi-gcc/$(ctiny_GCC_VERSION)

# -Wp,-w disables warnings about redefined macros, which we need because digitalPinToInterrupt gets defined twice in the TinyCircuits headers.
ctiny_CCOPT:=-mcpu=cortex-m0plus -mthumb -c -g -Os -MMD -std=gnu11 -ffunction-sections -fdata-sections \
  -nostdlib --param max-inline-insns-single=500 -Wp,-w
ctiny_CXXOPT:=$(filter-out -std=gnu11,$(ctiny_CCOPT)) -std=gnu++11  -fno-threadsafe-statics -fno-rtti -fno-exceptions
ctiny_CCWARN:=-Wno-expansion-to-defined -Wno-redundant-decls
ctiny_CCDEF:=-DF_CPU=48000000L -DARDUINO=10819 -DARDUINO_SAMD_ZERO -DARDUINO_ARCH_SAMD -D__SAMD21G18A__ -DUSB_VID=0x03EB -DUSB_PID=0x8009 \
  -DUSBCON "-DUSB_MANUFACTURER=\"TinyCircuits\"" "-DUSB_PRODUCT=\"TinyArcade\"" -DCRYSTALLESS \
  $(foreach U,$(ctiny_OPT_ENABLE),-DFMN_USE_$U=1) \
  -DFMN_IMAGE_SET_$(ctiny_IMAGE_SET)=1 -DFMN_FRAMEBUFFER_$(ctiny_FBFMT)=1
  
ctiny_CCINC:=-Isrc \
  -I$(ctiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include/ \
  -I$(ctiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS-Atmel/1.2.0/CMSIS/Device/ATMEL/ \
  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino \
  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade \
  -I$(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src \
  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/Wire \
  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/HID \
  -I$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/SPI 
  
ctiny_CC:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-gcc $(ctiny_CCOPT) $(ctiny_CCWARN) $(ctiny_CCDEF) $(ctiny_CCINC)
ctiny_CXX:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(ctiny_CXXOPT) $(ctiny_CCWARN) $(ctiny_CCDEF) $(ctiny_CCINC)
ctiny_AS:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-gcc -xassembler-with-cpp $(ctiny_CCOPT) $(ctiny_CCWARN) $(ctiny_CCDEF) $(ctiny_CCINC)

ctiny_LDOPT_SOLO:=-T$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/linker_scripts/gcc/flash_with_bootloader.ld 
ctiny_LDOPT_HOSTED:=-T$(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/linker_scripts/gcc/link_for_menu.ld 
ctiny_LDOPT:=-Os -Wl,--gc-sections -save-temps \
  --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m0plus -mthumb \
  -Wl,--check-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align 
ctiny_LDPOST:=-Wl,--start-group -L$(ctiny_ARDUINO_HOME)/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Lib/GCC/ -larm_cortexM0l_math -lm -Wl,--end-group
ctiny_LD_SOLO:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(ctiny_LDOPT_SOLO) $(ctiny_LDOPT)
ctiny_LD_HOSTED:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-g++ $(ctiny_LDOPT_HOSTED) $(ctiny_LDOPT)

ctiny_OBJCOPY:=$(ctiny_TOOLCHAIN)/bin/arm-none-eabi-objcopy

#TODO Are any of these unnecessary?
ctiny_EXTFILES:= \
  $(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src/File.cpp \
  $(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src/SD.cpp \
  $(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/Sd2Card.cpp \
  $(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/SdFile.cpp \
  $(ctiny_ARDUINO_IDE_HOME)/libraries/SD/src/utility/SdVolume.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/Wire/Wire.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/HID/HID.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/libraries/SPI/SPI.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/variants/tinyarcade/variant.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/pulse_asm.S \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/samd21_host.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WInterrupts.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/avr/dtostrf.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/cortex_handlers.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/hooks.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/itoa.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/delay.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/pulse.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/startup.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_analog.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_shift.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_digital.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/wiring_private.c \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/IPAddress.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Print.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Reset.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/SERCOM.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Stream.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Tone.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/CDC.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/PluggableUSB.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/USB/USBCore.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/Uart.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WMath.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/WString.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/abi.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/main.cpp \
  $(ctiny_ARDUINO_HOME)/packages/TinyCircuits/hardware/samd/1.1.0/cores/arduino/new.cpp

# Now do our usual build stuff...

ctiny_SRCFILES:=$(filter-out src/test/% src/www/% src/editor/%,$(call OPTFILTER,$(ctiny_OPT_ENABLE),$(SRCFILES),$(ctiny_MIDDIR)))

ctiny_DATA_SRC:=$(filter src/data/%,$(ctiny_SRCFILES))
ctiny_DATA_SRC:=$(filter-out %.png,$(ctiny_DATA_SRC)) $(filter %-$(ctiny_IMAGE_SET).png,$(ctiny_DATA_SRC))
ctiny_DATA_SRC:=$(filter-out %.adjust,$(ctiny_DATA_SRC))
ctiny_DATA_C:=$(patsubst src/%,$(ctiny_MIDDIR)/%.c,$(ctiny_DATA_SRC))
$(ctiny_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@
$(ctiny_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$< --format=$(ctiny_FBFMT)
$(ctiny_MIDDIR)/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$<
$(ctiny_MIDDIR)/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
$(ctiny_MIDDIR)/%.sprite.c:src/%.sprite $(TOOL_spritecvt);$(PRECMD) $(TOOL_spritecvt) -o$@ -i$<
$(ctiny_MIDDIR)/%.mid.c:src/%.mid src/%.adjust $(TOOL_songcvt);$(PRECMD) $(TOOL_songcvt) -o$@ -i$<

ctiny_CFILES:=$(filter %.c %.cpp %.S,$(ctiny_SRCFILES) $(ctiny_DATA_C)) $(ctiny_EXTFILES)
ctiny_OFILES:=$(patsubst src/%,$(ctiny_MIDDIR)/%, \
  $(patsubst $(ctiny_ARDUINO_IDE_HOME)/%,$(ctiny_MIDDIR)/%, \
  $(patsubst $(ctiny_ARDUINO_HOME)/%,$(ctiny_MIDDIR)/%, \
  $(addsuffix .o,$(basename $(ctiny_CFILES))) \
)))
-include $(ctiny_OFILES:.o=.d)

define ctiny_SRCRULES
  $(ctiny_MIDDIR)/%.o:$1/%.c  ;$$(PRECMD) $(ctiny_CC) -o $$@ $$<
  $(ctiny_MIDDIR)/%.o:$1/%.cpp;$$(PRECMD) $(ctiny_CXX) -o $$@ $$<
  $(ctiny_MIDDIR)/%.o:$1/%.S  ;$$(PRECMD) $(ctiny_AS) -o $$@ $$<
endef
$(foreach S,src $(ctiny_MIDDIR) $(ctiny_ARDUINO_HOME) $(ctiny_ARDUINO_IDE_HOME),$(eval $(call ctiny_SRCRULES,$S)))

ctiny_OFILES_HOSTED:=$(ctiny_OFILES)
ctiny_OFILES_SOLO:=$(ctiny_OFILES)

ctiny_HOSTED_ELF:=$(ctiny_MIDDIR)/$(PROJECT_NAME)-hosted.elf
$(ctiny_HOSTED_ELF):$(ctiny_OFILES_HOSTED);$(PRECMD) $(ctiny_LD_HOSTED) -o$@ $(ctiny_OFILES_HOSTED) $(ctiny_LDPOST)

ctiny_SOLO_ELF:=$(ctiny_MIDDIR)/$(PROJECT_NAME)-solo.elf
$(ctiny_SOLO_ELF):$(ctiny_OFILES_SOLO);$(PRECMD) $(ctiny_LD_SOLO) -o$@ $(ctiny_OFILES_SOLO) $(ctiny_LDPOST)

ctiny_HOSTED_BIN:=$(ctiny_OUTDIR)/$(PROJECT_NAME)-hosted.bin
ctiny_SOLO_BIN:=$(ctiny_OUTDIR)/$(PROJECT_NAME)-solo.bin
all:$(ctiny_HOSTED_BIN) $(ctiny_SOLO_BIN)
$(ctiny_HOSTED_BIN):$(ctiny_HOSTED_ELF);$(PRECMD) $(ctiny_OBJCOPY) -O binary $< $@
$(ctiny_SOLO_BIN):$(ctiny_SOLO_ELF);$(PRECMD) $(ctiny_OBJCOPY) -O binary $< $@

ctiny-run:$(ctiny_SOLO_BIN); \
  stty -F /dev/$(ctiny_PORT) 1200 ; \
  sleep 2 ; \
  $(ctiny_ARDUINO_HOME)/packages/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(ctiny_PORT) -U true -i -e -w $(ctiny_SOLO_BIN) -R
