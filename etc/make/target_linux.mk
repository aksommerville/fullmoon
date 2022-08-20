# target_linux.mk

linux_MIDDIR:=mid/linux
linux_OUTDIR:=out/linux

linux_OPT_ENABLE:=genioc intf x11 drmfb evdev alsa synth

linux_CCWARN:=-Werror -Wimplicit
linux_CCINC:=-I/usr/include/libdrm
linux_CCDEF:=$(patsubst %,-DFMN_USE_%=1,$(linux_OPT_ENABLE)) -DFMN_IMAGE_SET_$(linux_IMAGE_SET)=1 -DFMN_FRAMEBUFFER_$(linux_FBFMT)=1
linux_CC:=gcc -c -MMD -O3 -Isrc -I$(linux_MIDDIR) $(linux_CCWARN) $(linux_CCINC) $(linux_CCDEF)
linux_LD:=gcc
linux_LDPOST:=-ldrm -lX11 -lXinerama -lasound -lm -lpthread

linux_SRCFILES:=$(filter-out src/test/% src/www/% src/editor/%,$(call OPTFILTER,$(linux_OPT_ENABLE),$(SRCFILES),$(linux_MIDDIR)))

linux_DATA_SRC:=$(filter src/data/%,$(linux_SRCFILES))
linux_DATA_SRC:=$(filter-out %.png,$(linux_DATA_SRC)) $(filter %-$(linux_IMAGE_SET).png,$(linux_DATA_SRC))
linux_DATA_C:=$(patsubst src/%,$(linux_MIDDIR)/%.c,$(linux_DATA_SRC))
$(linux_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@
$(linux_MIDDIR)/data/image/appicon-%.png.c:src/data/image/appicon-%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$< --format=rgba8888
$(linux_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$< --format=$(linux_FBFMT)
$(linux_MIDDIR)/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$<
$(linux_MIDDIR)/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
$(linux_MIDDIR)/%.sprite.c:src/%.sprite $(TOOL_spritecvt);$(PRECMD) $(TOOL_spritecvt) -o$@ -i$<
$(linux_MIDDIR)/%.mid.c:src/%.mid $(TOOL_songcvt);$(PRECMD) $(TOOL_songcvt) -o$@ -i$<

linux_CFILES:=$(filter %.c,$(linux_SRCFILES) $(linux_DATA_C))
linux_OFILES:=$(patsubst src/%,$(linux_MIDDIR)/%,$(addsuffix .o,$(basename $(linux_CFILES))))
-include $(linux_OFILES:.o=.d)

$(linux_MIDDIR)/%.o:src/%.c            ;$(PRECMD) $(linux_CC) -o $@ $<
$(linux_MIDDIR)/%.o:$(linux_MIDDIR)/%.c;$(PRECMD) $(linux_CC) -o $@ $<

linux_EXE:=$(linux_OUTDIR)/$(PROJECT_NAME)
all:$(linux_EXE)
$(linux_EXE):$(linux_OFILES);$(PRECMD) $(linux_LD) -o$@ $^ $(linux_LDPOST)

linux-run:$(linux_EXE);$(linux_EXE)
