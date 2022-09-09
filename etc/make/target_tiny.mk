# target_tiny.mk
# Tiny Circuits TinyArcade and PocketArcade

tiny_PROJECT_NAME:=fullmoon

tiny_IMAGE_SET:=8c

tiny_TMPDIR_SOLO:=mid/tiny/build-solo
tiny_TMPDIR_HOSTED:=mid/tiny/build-hosted
tiny_CACHEDIR_SOLO:=mid/tiny/cache-solo
tiny_CACHEDIR_HOSTED:=mid/tiny/cache-hosted
$(tiny_TMPDIR_SOLO) $(tiny_TMPDIR_HOSTED) $(tiny_CACHEDIR_SOLO) $(tiny_CACHEDIR_HOSTED):;mkdir -p $@

tiny_BIN_SOLO:=out/tiny/$(PROJECT_NAME)-solo.bin
tiny_BIN_HOSTED:=out/tiny/$(PROJECT_NAME)-hosted.bin
tiny_PACKAGE:=out/$(PROJECT_NAME).zip

tiny_OPT_ENABLE:=tiny minisyni
tiny_SRCFILES:=$(filter-out src/test/% src/www/% src/editor/%,$(call OPTFILTER,$(tiny_OPT_ENABLE),$(SRCFILES),mid/tiny))

tiny_DATA_SRC:=$(filter src/data/%,$(tiny_SRCFILES))
tiny_SRCFILES:=$(filter-out src/data/%,$(tiny_SRCFILES))

# oh no i finally breached the tiny flash limit. drop a few files
#tiny_DATA_SRC:=$(filter-out %/baltic.mid %/cobweb.mid %/infinite.mid %/appicon-8c.png,$(tiny_DATA_SRC))

tiny_DATA_SRC:=$(filter-out src/data/image/appicon.png,$(tiny_DATA_SRC))
tiny_DATA_SRC:=$(filter-out %.png,$(tiny_DATA_SRC)) $(filter %-$(tiny_IMAGE_SET).png,$(tiny_DATA_SRC))
tiny_DATA_SRC:=$(filter-out %.adjust,$(tiny_DATA_SRC))
tiny_DATA_C:=$(patsubst src/%,mid/tiny/%.c,$(tiny_DATA_SRC))
mid/tiny/%.c:src/%;$(PRECMD) cp $< $@
mid/tiny/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$< --progmem --format=bgr332
mid/tiny/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$< --progmem
mid/tiny/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
mid/tiny/%.sprite.c:src/%.sprite $(TOOL_spritecvt);$(PRECMD) $(TOOL_spritecvt) -o$@ -i$<
mid/tiny/%.mid.c:src/%.mid src/%.adjust $(TOOL_songcvt);$(PRECMD) $(TOOL_songcvt) -o$@ -i$<

tiny_SRCFILES+=$(tiny_DATA_C)

# All the C files get copied here to simplify our request to arduino-builder.
tiny_SCRATCHDIR:=mid/tiny/scratch
tiny_SCRATCHFILES:=
define tiny_SCRATCHRULE_FLAT
  tiny_SCRATCHFILES+=mid/tiny/scratch/$(notdir $1)
  mid/tiny/scratch/$(notdir $1):$1;$$(PRECMD) cp $$< $$@
endef
define tiny_SCRATCHRULE_NESTED
  tiny_SCRATCHFILES+=$(patsubst src/%,mid/tiny/scratch/%,$1)
  $(patsubst src/%,mid/tiny/scratch/%,$1):$1;$$(PRECMD) cp $$< $$@
endef
$(foreach F,$(tiny_SRCFILES), \
  $(if $(filter %.h,$F),$(eval $(call tiny_SCRATCHRULE_NESTED,$F)), \
    $(eval $(call tiny_SCRATCHRULE_FLAT,$F)) \
  ) \
)
mid/tiny/scratch/dummy.cpp:;$(PRECMD) touch $@
tiny_SCRATCHFILES:=mid/tiny/scratch/dummy.cpp $(tiny_SCRATCHFILES)

define BUILD # 1=goal, 2=tmpdir, 3=cachedir, 4=BuildOption
$1:$2 $3 $(tiny_SCRATCHFILES); \
  $(tiny_BUILDER) \
  -compile \
  -logger=machine \
  -hardware $(tiny_IDEROOT)/hardware \
  $(if $(tiny_PKGROOT),-hardware $(tiny_PKGROOT)) \
  -tools $(tiny_IDEROOT)/tools-builder \
  -tools $(tiny_IDEROOT)/hardware/tools/avr \
  $(if $(tiny_PKGROOT),-tools $(tiny_PKGROOT)) \
  -built-in-libraries $(tiny_IDEROOT)/libraries \
  $(if $(tiny_LIBSROOT),-libraries $(tiny_LIBSROOT)) \
  -fqbn=TinyCircuits:samd:tinyarcade:BuildOption=$4 \
  -ide-version=$(tiny_IDEVERSION) \
  -build-path $2 \
  -warnings=none \
  -build-cache $3 \
  -prefs=build.warn_data_percentage=75 \
  $(tiny_BUILDER_OPTS) \
  $(tiny_SCRATCHFILES) \
  2>&1 | etc/tool/reportstatus.py
endef

# For inclusion in a TinyArcade SD card.
tiny_PRODUCT_HOSTED:=$(tiny_TMPDIR_HOSTED)/dummy.cpp.bin
$(tiny_BIN_HOSTED):build-hosted;$(PRECMD) cp $(tiny_PRODUCT_HOSTED) $@
$(eval $(call BUILD,build-hosted,$(tiny_TMPDIR_HOSTED),$(tiny_CACHEDIR_HOSTED),TAgame))

# For upload.
tiny_PRODUCT_SOLO:=$(tiny_TMPDIR_SOLO)/dummy.cpp.bin
$(tiny_BIN_SOLO):build-solo;$(PRECMD) cp $(tiny_PRODUCT_SOLO) $@
$(eval $(call BUILD,build-solo,$(tiny_TMPDIR_SOLO),$(tiny_CACHEDIR_SOLO),TAmenu))
  
$(tiny_PACKAGE):$(tiny_BIN_HOSTED) $(DATA_INCLUDE_TINY);$(PRECMD) \
  rm -rf out/tiny/$(PROJECT_NAME) ; \
  mkdir out/tiny/$(PROJECT_NAME) || exit 1 ; \
  echo TODO cp -r out/tiny/data/* out/tiny/$(PROJECT_NAME) || exit 1 ; \
  cp $(tiny_BIN_HOSTED) out/tiny/$(PROJECT_NAME)/$(PROJECT_NAME).bin || exit 1 ; \
  cd out/tiny ; \
  zip -r $(PROJECT_NAME).zip $(PROJECT_NAME) >/dev/null || exit 1 ; \
  rm -r $(PROJECT_NAME)
  
all:$(tiny_BIN_HOSTED) $(tiny_BIN_SOLO) $(tiny_PACKAGE)

ifneq (,$(tiny_MENU_BIN))
  tiny-deploy-menu:; \
    stty -F /dev/$(tiny_PORT) 1200 ; \
    sleep 2 ; \
    $(tiny_PKGROOT)/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(tiny_PORT) -U true -i -e -w $(tiny_MENU_BIN) -R
endif

tiny-run:$(tiny_BIN_SOLO); \
  stty -F /dev/$(tiny_PORT) 1200 ; \
  sleep 2 ; \
  $(tiny_PKGROOT)/arduino/tools/bossac/1.7.0-arduino3/bossac -i -d --port=$(tiny_PORT) -U true -i -e -w $(tiny_BIN_SOLO) -R
