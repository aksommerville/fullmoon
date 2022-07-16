# test.mk
# Rules for building and running unit tests.
# TODO Integration and automation tests? Tests for the web app?

test_MIDDIR:=mid/test
test_OUTDIR:=out/test

test_CC:=gcc -c -MMD -O0 -Werror -Wimplicit -Isrc
test_LD:=gcc
test_LDPOST:=

test_SRCFILES:=$(filter src/test/%,$(SRCFILES))
test_CFILES:=$(filter %.c,$(test_SRCFILES))
test_OFILES:=$(patsubst src/%.c,$(test_MIDDIR)/%.o,$(test_CFILES))
-include $(test_OFILES:.o=.d)

$(test_MIDDIR)/%.o:src/%.c;$(PRECMD) $(test_CC) -o$@ $<

test_OFILES_COMMON:=$(filter $(test_MIDDIR)/test/common/%,$(test_OFILES))
test_OFILES_UNIT:=$(filter $(test_MIDDIR)/test/unit/%,$(test_OFILES))
test_EXES_UNIT:=$(patsubst $(test_MIDDIR)/test/unit/%.o,$(test_OUTDIR)/unit/%,$(test_OFILES_UNIT))
all:$(test_EXES_UNIT)
$(test_OUTDIR)/unit/%:$(test_MIDDIR)/test/unit/%.o $(test_OFILES_COMMON);$(PRECMD) $(test_LD) -o$@ $^ $(test_LDPOST)

test:$(test_EXES_UNIT);etc/tool/runtests.sh $^
