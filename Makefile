all:
.SILENT:
.SECONDARY:
PRECMD=echo "  $(@F)" ; mkdir -p $(@D) ;

# The first thing we include is etc/config.mk, which might not be in the repo.
# It should define:
#  FMN_TARGETS:=linux thumby # corresponds to etc/make/target_*.mk
#  run:linux-run # your preferred 'run' command
#  ...whatever else the selected targets need.

ifeq ($(MAKECMDGOALS),clean)
  clean:;rm -rf mid out
else
  include etc/config.mk
  include etc/make/common.mk
  include etc/make/tools.mk
  include etc/make/test.mk
  $(foreach T,$(FMN_TARGETS),$(eval include etc/make/target_$T.mk))
endif
