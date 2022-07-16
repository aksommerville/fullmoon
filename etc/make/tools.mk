# tools.mk
# Rules for building tools that we need at compile time.

TOOLS_MIDDIR:=mid/tool
TOOLS_OUTDIR:=out/tool

TOOLS_CC:=gcc -c -MMD -O3 -Isrc -Werror -Wimplicit
TOOLS_LD:=gcc
TOOLS_LDPOST:=-lz

TOOLS:=$(filter-out common,$(notdir $(wildcard src/tool/*)))

TOOLS_SRCFILES:=$(filter src/tool/%,$(SRCFILES))
TOOLS_CFILES:=$(filter %.c,$(TOOLS_SRCFILES))
TOOLS_OFILES:=$(patsubst src/%.c,$(TOOLS_MIDDIR)/%.o,$(TOOLS_CFILES))
-include $(TOOLS_OFILES:.o=.d)

$(TOOLS_MIDDIR)/%.o:src/%.c;$(PRECMD) $(TOOLS_CC) -o$@ $<

TOOLS_EXES:=

define TOOL_RULES # $1=name
  TOOL_$1_OFILES:=$(filter $(TOOLS_MIDDIR)/tool/common/% $(TOOLS_MIDDIR)/tool/$1/%,$(TOOLS_OFILES))
  TOOL_$1:=$(TOOLS_OUTDIR)/$1
  TOOLS_EXES+=$$(TOOL_$1)
  $$(TOOL_$1):$$(TOOL_$1_OFILES);$$(PRECMD) $(TOOLS_LD) -o$$@ $$^ $(TOOLS_LDPOST)
endef

$(foreach T,$(TOOLS),$(eval $(call TOOL_RULES,$T)))

all:$(TOOLS_EXES)
