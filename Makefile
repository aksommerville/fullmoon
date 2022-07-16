all:
.SILENT:
.SECONDARY:
PRECMD=echo "  $(@F)" ; mkdir -p $(@D) ;

ifeq ($(MAKECMDGOALS),clean)
  clean:;rm -rf mid out
else
  include etc/config.mk
  include etc/make/common.mk
  include etc/make/tools.mk
  include etc/make/test.mk
  $(foreach T,$(FMN_TARGETS),$(eval include etc/make/target_$T.mk))
endif
