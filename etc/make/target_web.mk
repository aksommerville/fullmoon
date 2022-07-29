# target_web.mk

web_MIDDIR:=mid/web
web_OUTDIR:=out/web

web_OPT_ENABLE:=web minisyni

web_CCOPT:=-nostdlib -c -MMD -Isrc -Wno-comment -Wno-parentheses -Wno-constant-conversion -DFMN_IMAGE_SET_$(web_IMAGE_SET)=1 -DFMN_FRAMEBUFFER_$(web_FBFMT)=1
web_LDOPT:=-nostdlib -Xlinker --no-entry -Xlinker --import-undefined -Xlinker --export-all
web_CC:=$(web_WASI_SDK)/bin/clang $(web_CCOPT)
web_LD:=$(web_WASI_SDK)/bin/clang $(web_LDOPT)
web_LDPOST:=

web_SRCFILES:=$(filter-out src/test/%,$(call OPTFILTER,$(web_OPT_ENABLE),$(SRCFILES),$(web_MIDDIR)))

web_DATA_SRC:=$(filter src/data/%,$(web_SRCFILES))
web_DATA_SRC:=$(filter-out src/data/image/appicon.png,$(web_DATA_SRC))
web_DATA_SRC:=$(filter-out %.png,$(web_DATA_SRC)) $(filter %-$(web_IMAGE_SET).png,$(web_DATA_SRC))
web_DATA_C:=$(patsubst src/%,$(web_MIDDIR)/%.c,$(web_DATA_SRC))
$(web_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@
$(web_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$< --format=$(web_FBFMT)
$(web_MIDDIR)/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$<
$(web_MIDDIR)/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
$(web_MIDDIR)/%.sprite.c:src/%.sprite $(TOOL_spritecvt);$(PRECMD) $(TOOL_spritecvt) -o$@ -i$<

web_CFILES:=$(filter %.c,$(web_SRCFILES)) $(web_DATA_C)
web_OFILES:=$(patsubst src/%,$(web_MIDDIR)/%,$(addsuffix .o,$(basename $(web_CFILES))))
-include $(web_OFILES:.o=.d)

$(web_MIDDIR)/%.o:src/%.c          ;$(PRECMD) $(web_CC) -o $@ $<
$(web_MIDDIR)/%.o:$(web_MIDDIR)/%.c;$(PRECMD) $(web_CC) -o $@ $<

web_EXE:=$(web_OUTDIR)/$(PROJECT_NAME).wasm
all:$(web_EXE)
$(web_EXE):$(web_OFILES);$(PRECMD) $(web_LD) -o$@ $^ $(web_LDPOST)

web_WWW_SRCFILES:=$(filter src/www/%,$(SRCFILES))
web_WWW_DSTFILES:=$(patsubst src/www/%,$(web_OUTDIR)/%,$(web_WWW_SRCFILES))
all:$(web_WWW_DSTFILES)
$(web_OUTDIR)/%:src/www/%;$(PRECMD) cp $< $@

web-run:$(web_EXE) $(web_WWW_DSTFILES);http-server $(web_OUTDIR) -c-1
