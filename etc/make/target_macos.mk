# target_macos.mk

macos_MIDDIR:=mid/macos
macos_OUTDIR:=out/macos

macos_OPT_ENABLE:=intf macos

macos_CCWARN:=-Werror -Wimplicit -Wno-parentheses -Wno-comment -Wno-deprecated-declarations
macos_CCINC:=-I/usr/include/libdrm
macos_CCDEF:=$(patsubst %,-DFMN_USE_%=1,$(macos_OPT_ENABLE))
macos_CC:=gcc -c -MMD -O3 -Isrc -I$(macos_MIDDIR) $(macos_CCWARN) $(macos_CCINC) $(macos_CCDEF)
macos_OBJC:=gcc -xobjective-c -c -MMD -O3 -Isrc -I$(macos_MIDDIR) $(macos_CCWARN) $(macos_CCINC) $(macos_CCDEF)
macos_LD:=gcc
macos_LDPOST:=-framework Cocoa -framework OpenGL -framework IOKit

macos_SRCFILES:=$(filter-out src/test/%,$(call OPTFILTER,$(macos_OPT_ENABLE),$(SRCFILES),$(macos_MIDDIR)))

macos_DATA_SRC:=$(filter src/data/%,$(macos_SRCFILES))
macos_DATA_C:=$(patsubst src/%,$(macos_MIDDIR)/%.c,$(macos_DATA_SRC))
$(macos_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$<
$(macos_MIDDIR)/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$<
$(macos_MIDDIR)/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
$(macos_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@

macos_CFILES:=$(filter %.c %.m,$(macos_SRCFILES) $(macos_DATA_C))
macos_OFILES:=$(patsubst src/%,$(macos_MIDDIR)/%,$(addsuffix .o,$(basename $(macos_CFILES))))
-include $(macos_OFILES:.o=.d)

$(macos_MIDDIR)/%.o:src/%.c            ;$(PRECMD) $(macos_CC) -o $@ $<
$(macos_MIDDIR)/%.o:$(macos_MIDDIR)/%.c;$(PRECMD) $(macos_CC) -o $@ $<
$(macos_MIDDIR)/%.o:src/%.m            ;$(PRECMD) $(macos_OBJC) -o $@ $<
$(macos_MIDDIR)/%.o:$(macos_MIDDIR)/%.m;$(PRECMD) $(macos_OBJC) -o $@ $<

macos_BUNDLE:=$(macos_OUTDIR)/FullMoon.app
macos_PLIST:=$(macos_BUNDLE)/Contents/Info.plist
macos_NIB:=$(macos_BUNDLE)/Contents/Resources/Main.nib
macos_EXE:=$(macos_BUNDLE)/Contents/MacOS/fullmoon
macos_ICON:=$(macos_BUNDLE)/Contents/Resources/appicon.icns

$(macos_PLIST):src/opt/macos/Info.plist;$(PRECMD) cp $< $@
$(macos_NIB):src/opt/macos/Main.xib;$(PRECMD) ibtool --compile $@ $<
macos_INPUT_ICONS:=$(wildcard src/opt/macos/appicon.iconset/*)
$(macos_ICON):$(macos_INPUT_ICONS);$(PRECMD) iconutil -c icns -o $@ src/opt/macos/appicon.iconset

$(macos_EXE):$(macos_PLIST) $(macos_NIB) $(macos_ICON)
all:$(macos_EXE)
$(macos_EXE):$(macos_OFILES);$(PRECMD) $(macos_LD) -o$@ $(macos_OFILES) $(macos_LDPOST)

#TODO Mac app bundle

macos-run:$(macos_EXE);open -W $(macos_BUNDLE) --args --reopen-tty=$$(tty) --chdir=$$(pwd)
