# target_linux.mk

linux_MIDDIR:=mid/linux
linux_OUTDIR:=out/linux

linux_OPT_ENABLE:=genioc intf x11 drmfb evdev alsa pulse

linux_CCWARN:=-Werror -Wimplicit
linux_CCINC:=-I/usr/include/libdrm
linux_CCDEF:=$(patsubst %,-DFMN_USE_%=1,$(linux_OPT_ENABLE))
linux_CC:=gcc -c -MMD -O3 -Isrc -I$(linux_MIDDIR) $(linux_CCWARN) $(linux_CCINC) $(linux_CCDEF)
linux_LD:=gcc
linux_LDPOST:=-ldrm -lX11 -lasound -lpulse -lpulse-simple -lm -lpthread

linux_SRCFILES:=$(call OPTFILTER,$(linux_OPT_ENABLE),$(SRCFILES),$(linux_MIDDIR))

linux_DATA_SRC:=$(filter src/data/%,$(linux_SRCFILES))
linux_DATA_C:=$(patsubst src/%,$(linux_MIDDIR)/%.c,$(linux_DATA_SRC))
$(linux_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@
$(linux_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ $<

linux_CFILES:=$(filter %.c,$(linux_SRCFILES))
linux_OFILES:=$(patsubst src/%.c,$(linux_MIDDIR)/%.o,$(linux_CFILES))
-include $(linux_OFILES:.o=.d)

$(linux_MIDDIR)/%.o:src/%.c            ;$(PRECMD) $(linux_CC) -o $@ $<
$(linux_MIDDIR)/%.o:$(linux_MIDDIR)/%.c;$(PRECMD) $(linux_CC) -o $@ $<

linux_EXE:=$(linux_OUTDIR)/$(PROJECT_NAME)
all:$(linux_EXE)
$(linux_EXE):$(linux_OFILES);$(PRECMD) $(linux_LD) -o$@ $^ $(linux_LDPOST)

linux-run:$(linux_EXE);$(linux_EXE)
