# common.mk
# A few things we take care of early in the make configuration process.

PROJECT_NAME:=fullmoon

SRCFILES:=$(shell find src -type f)

ALWAYS_FILTER_OUT:=src/tool/%

define OPTFILTER # $1=units, $2=files, $3=middir
  $(foreach F,$(filter-out $(ALWAYS_FILTER_OUT),$2), \
    $(if $(findstring /opt/,$F), \
      $(filter $(addprefix $3/opt/,$(addsuffix /%,$1)) $(addprefix src/opt/,$(addsuffix /%,$1)),$F) \
    ,$F) \
  )
endef
