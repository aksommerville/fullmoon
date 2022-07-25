# target_thumby.mk
# There is an opportunity for cleanliness improvement here, to say the least.
# Would it be reasonable to use CMake for just this target?
# Or what would it take to reorganize pico-sdk to use regular make?

thumby_MIDDIR:=mid/thumby
thumby_OUTDIR:=out/thumby

# Generate this list by creating a dummy CMake project, follow the pico-sdk instructions.
# Copy its list, then: sed -En 's/^\t@echo.*pico-sdk\/(.*)\.obj"$/  \1.c \\/p' etc/make/target_thumby.mk
# You'll also get pico/version.h at that step, copy it to src/opt/thumby/pico/version.h. (and config_autogen.h)
thumby_EXTCFILES:=$(addprefix $(thumby_PICO_SDK)/, \
  src/common/pico_sync/critical_section.c \
  src/common/pico_sync/lock_core.c \
  src/common/pico_sync/mutex.c \
  src/common/pico_sync/sem.c \
  src/common/pico_time/time.c \
  src/common/pico_time/timeout_helper.c \
  src/common/pico_util/datetime.c \
  src/common/pico_util/pheap.c \
  src/common/pico_util/queue.c \
  src/rp2_common/hardware_claim/claim.c \
  src/rp2_common/hardware_clocks/clocks.c \
  src/rp2_common/hardware_divider/divider.S \
  src/rp2_common/hardware_gpio/gpio.c \
  src/rp2_common/hardware_irq/irq.c \
  src/rp2_common/hardware_irq/irq_handler_chain.c \
  src/rp2_common/hardware_pll/pll.c \
  src/rp2_common/hardware_sync/sync.c \
  src/rp2_common/hardware_timer/timer.c \
  src/rp2_common/hardware_uart/uart.c \
  src/rp2_common/hardware_vreg/vreg.c \
  src/rp2_common/hardware_watchdog/watchdog.c \
  src/rp2_common/hardware_xosc/xosc.c \
  src/rp2_common/pico_bit_ops/bit_ops_aeabi.c \
  src/rp2_common/pico_bootrom/bootrom.c \
  src/rp2_common/pico_divider/divider.c \
  src/rp2_common/pico_double/double_aeabi.c \
  src/rp2_common/pico_double/double_init_rom.c \
  src/rp2_common/pico_double/double_math.c \
  src/rp2_common/pico_double/double_v1_rom_shim.c \
  src/rp2_common/pico_float/float_aeabi.c \
  src/rp2_common/pico_float/float_init_rom.c \
  src/rp2_common/pico_float/float_math.c \
  src/rp2_common/pico_float/float_v1_rom_shim.c \
  src/rp2_common/pico_int64_ops/pico_int64_ops_aeabi.c \
  src/rp2_common/pico_malloc/pico_malloc.c \
  src/rp2_common/pico_mem_ops/mem_ops_aeabi.c \
  src/rp2_common/pico_platform/platform.c \
  src/rp2_common/pico_printf/printf.c \
  src/rp2_common/pico_runtime/runtime.c \
  src/rp2_common/pico_standard_link/binary_info.c \
  src/rp2_common/pico_standard_link/crt0.S \
  src/rp2_common/pico_stdio/stdio.c \
  src/rp2_common/pico_stdio_uart/stdio_uart.c \
  src/rp2_common/pico_stdlib/stdlib.c \
  src/rp2_common/hardware_spi/spi.c \
)
# excluded:
#  src/rp2_common/pico_standard_link/new_delete.cpp

# Generate this one the hard way. Let it fail, and find each of the missing headers.
thumby_EXTHDIRS:=$(addprefix $(thumby_PICO_SDK)/, \
  src/rp2_common/hardware_sync/include \
  src/common/pico_sync/include \
  src/common/pico_base/include \
  src/rp2_common/pico_platform/include \
  src/rp2040/hardware_regs/include \
  src/common/pico_time/include \
  src/rp2_common/hardware_timer/include \
  src/common/pico_util/include \
  src/rp2_common/hardware_claim/include \
  src/rp2_common/hardware_base/include \
  src/rp2_common/hardware_clocks/include \
  src/rp2040/hardware_structs/include \
  src/rp2_common/hardware_watchdog/include \
  src/rp2_common/hardware_pll/include \
  src/rp2_common/hardware_xosc/include \
  src/rp2_common/hardware_irq/include \
  src/rp2_common/hardware_gpio/include \
  src/rp2_common/pico_platform/include \
  src/rp2_common/hardware_resets/include \
  src/rp2_common/hardware_uart/include \
  src/rp2_common/hardware_vreg/include \
  src/rp2_common/pico_bootrom/include \
  src/rp2_common/hardware_divider/include \
  src/rp2_common/pico_double/include \
  src/rp2_common/pico_float/include \
  src/rp2_common/pico_malloc/include \
  src/rp2_common/pico_printf/include \
  src/common/pico_binary_info/include \
  src/rp2_common/boot_stage2/include \
  src/rp2_common/pico_stdio/include \
  src/rp2_common/pico_stdio_uart/include \
  src/common/pico_stdlib/include \
  src/rp2_common/hardware_spi/include \
  src/rp2_common/hardware_pwm/include \
)

thumby_LINKWRAP:= \
  -Wl,--wrap=sprintf -Wl,--wrap=snprintf -Wl,--wrap=vsnprintf -Wl,--wrap=__clzsi2 -Wl,--wrap=__clzdi2 -Wl,--wrap=__ctzsi2 \
  -Wl,--wrap=__ctzdi2 -Wl,--wrap=__popcountsi2 -Wl,--wrap=__popcountdi2 -Wl,--wrap=__clz -Wl,--wrap=__clzl -Wl,--wrap=__clzll \
  -Wl,--wrap=__aeabi_idiv -Wl,--wrap=__aeabi_idivmod -Wl,--wrap=__aeabi_ldivmod -Wl,--wrap=__aeabi_uidiv -Wl,--wrap=__aeabi_uidivmod \
  -Wl,--wrap=__aeabi_uldivmod -Wl,--wrap=__aeabi_dadd -Wl,--wrap=__aeabi_ddiv -Wl,--wrap=__aeabi_dmul -Wl,--wrap=__aeabi_drsub \
  -Wl,--wrap=__aeabi_dsub -Wl,--wrap=__aeabi_cdcmpeq -Wl,--wrap=__aeabi_cdrcmple -Wl,--wrap=__aeabi_cdcmple -Wl,--wrap=__aeabi_dcmpeq \
  -Wl,--wrap=__aeabi_dcmplt -Wl,--wrap=__aeabi_dcmple -Wl,--wrap=__aeabi_dcmpge -Wl,--wrap=__aeabi_dcmpgt -Wl,--wrap=__aeabi_dcmpun \
  -Wl,--wrap=__aeabi_i2d -Wl,--wrap=__aeabi_l2d -Wl,--wrap=__aeabi_ui2d -Wl,--wrap=__aeabi_ul2d -Wl,--wrap=__aeabi_d2iz -Wl,--wrap=__aeabi_d2lz \
  -Wl,--wrap=__aeabi_d2uiz -Wl,--wrap=__aeabi_d2ulz -Wl,--wrap=__aeabi_d2f -Wl,--wrap=sqrt -Wl,--wrap=cos -Wl,--wrap=sin -Wl,--wrap=tan \
  -Wl,--wrap=atan2 -Wl,--wrap=exp -Wl,--wrap=log -Wl,--wrap=ldexp -Wl,--wrap=copysign -Wl,--wrap=trunc -Wl,--wrap=floor -Wl,--wrap=ceil \
  -Wl,--wrap=round -Wl,--wrap=sincos -Wl,--wrap=asin -Wl,--wrap=acos -Wl,--wrap=atan -Wl,--wrap=sinh -Wl,--wrap=cosh -Wl,--wrap=tanh \
  -Wl,--wrap=asinh -Wl,--wrap=acosh -Wl,--wrap=atanh -Wl,--wrap=exp2 -Wl,--wrap=log2 -Wl,--wrap=exp10 -Wl,--wrap=log10 -Wl,--wrap=pow \
  -Wl,--wrap=powint -Wl,--wrap=hypot -Wl,--wrap=cbrt -Wl,--wrap=fmod -Wl,--wrap=drem -Wl,--wrap=remainder -Wl,--wrap=remquo -Wl,--wrap=expm1 \
  -Wl,--wrap=log1p -Wl,--wrap=fma -Wl,--wrap=__aeabi_lmul -Wl,--wrap=__aeabi_fadd -Wl,--wrap=__aeabi_fdiv -Wl,--wrap=__aeabi_fmul \
  -Wl,--wrap=__aeabi_frsub -Wl,--wrap=__aeabi_fsub -Wl,--wrap=__aeabi_cfcmpeq -Wl,--wrap=__aeabi_cfrcmple -Wl,--wrap=__aeabi_cfcmple \
  -Wl,--wrap=__aeabi_fcmpeq -Wl,--wrap=__aeabi_fcmplt -Wl,--wrap=__aeabi_fcmple -Wl,--wrap=__aeabi_fcmpge -Wl,--wrap=__aeabi_fcmpgt \
  -Wl,--wrap=__aeabi_fcmpun -Wl,--wrap=__aeabi_i2f -Wl,--wrap=__aeabi_l2f -Wl,--wrap=__aeabi_ui2f -Wl,--wrap=__aeabi_ul2f \
  -Wl,--wrap=__aeabi_f2iz -Wl,--wrap=__aeabi_f2lz -Wl,--wrap=__aeabi_f2uiz -Wl,--wrap=__aeabi_f2ulz -Wl,--wrap=__aeabi_f2d \
  -Wl,--wrap=sqrtf -Wl,--wrap=cosf -Wl,--wrap=sinf -Wl,--wrap=tanf -Wl,--wrap=atan2f -Wl,--wrap=expf -Wl,--wrap=logf -Wl,--wrap=ldexpf \
  -Wl,--wrap=copysignf -Wl,--wrap=truncf -Wl,--wrap=floorf -Wl,--wrap=ceilf -Wl,--wrap=roundf -Wl,--wrap=sincosf -Wl,--wrap=asinf \
  -Wl,--wrap=acosf -Wl,--wrap=atanf -Wl,--wrap=sinhf -Wl,--wrap=coshf -Wl,--wrap=tanhf -Wl,--wrap=asinhf -Wl,--wrap=acoshf -Wl,--wrap=atanhf \
  -Wl,--wrap=exp2f -Wl,--wrap=log2f -Wl,--wrap=exp10f -Wl,--wrap=log10f -Wl,--wrap=powf -Wl,--wrap=powintf -Wl,--wrap=hypotf -Wl,--wrap=cbrtf \
  -Wl,--wrap=fmodf -Wl,--wrap=dremf -Wl,--wrap=remainderf -Wl,--wrap=remquof -Wl,--wrap=expm1f -Wl,--wrap=log1pf -Wl,--wrap=fmaf -Wl,--wrap=malloc \
  -Wl,--wrap=calloc -Wl,--wrap=realloc -Wl,--wrap=free -Wl,--wrap=memcpy -Wl,--wrap=memset -Wl,--wrap=__aeabi_memcpy -Wl,--wrap=__aeabi_memset \
  -Wl,--wrap=__aeabi_memcpy4 -Wl,--wrap=__aeabi_memset4 -Wl,--wrap=__aeabi_memcpy8 -Wl,--wrap=__aeabi_memset8 \
  -Wl,--wrap=printf -Wl,--wrap=vprintf -Wl,--wrap=puts -Wl,--wrap=putchar -Wl,--wrap=getchar 

thumby_OPT_ENABLE:=thumby

thumby_CCWARN:=-Werror -Wimplicit
thumby_CCINC:=$(addprefix -I,$(thumby_EXTHDIRS)) -Isrc -Isrc/opt/thumby
thumby_CCDEF:=-DNDEBUG $(patsubst %,-DFMN_USE_%=1,$(thumby_OPT_ENABLE))
thumby_CCOPT:=-c -MMD -O3 -mcpu=cortex-m0plus -mthumb
thumby_CC:=$(thumby_GCCPFX)gcc $(thumby_CCOPT) -I$(thumby_MIDDIR) $(thumby_CCWARN) $(thumby_CCINC) $(thumby_CCDEF)
thumby_AS:=$(thumby_GCCPFX)gcc -xassembler-with-cpp $(thumby_CCOPT) -I$(thumby_MIDDIR) $(thumby_CCWARN) $(thumby_CCINC) $(thumby_CCDEF)

thumby_LDOPT:=-Wl,-z,max-page-size=4096 -Wl,--gc-sections -mcpu=cortex-m0plus -mthumb -O3 -DNDEBUG -Wl,--build-id=none --specs=nosys.specs 
thumby_LDOPT+=-Wl,--script=$(thumby_PICO_SDK)/src/rp2_common/pico_standard_link/memmap_default.ld 
thumby_LD:=$(thumby_GCCPFX)gcc $(thumby_LDOPT) $(thumby_LINKWRAP)
thumby_LDPOST:=

thumby_SRCFILES:=$(filter-out src/test/%,$(call OPTFILTER,$(thumby_OPT_ENABLE),$(SRCFILES),$(thumby_MIDDIR)))

thumby_DATA_SRC:=$(filter src/data/%,$(thumby_SRCFILES))
thumby_DATA_SRC:=$(filter-out src/data/image/appicon.png,$(thumby_DATA_SRC))
thumby_DATA_C:=$(patsubst src/%,$(thumby_MIDDIR)/%.c,$(thumby_DATA_SRC))
$(thumby_MIDDIR)/%.c:src/%;$(PRECMD) cp $< $@
$(thumby_MIDDIR)/%.png.c:src/%.png $(TOOL_imgcvt);$(PRECMD) $(TOOL_imgcvt) -o$@ -i$<
$(thumby_MIDDIR)/data/map/%.c:src/data/map/% $(TOOL_mapcvt);$(PRECMD) $(TOOL_mapcvt) -o$@ -i$<
$(thumby_MIDDIR)/%_props.txt.c:src/%_props.txt $(TOOL_tileprops);$(PRECMD) $(TOOL_tileprops) -o$@ -i$<
$(thumby_MIDDIR)/%.sprite.c:src/%.sprite $(TOOL_spritecvt);$(PRECMD) $(TOOL_spritecvt) -o$@ -i$<

thumby_CFILES:=$(filter %.c,$(thumby_SRCFILES)) $(thumby_DATA_C)
thumby_OFILES:= \
  $(patsubst src/%,$(thumby_MIDDIR)/%,$(addsuffix .o,$(basename $(thumby_CFILES)))) \
  $(patsubst $(thumby_PICO_SDK)/%,$(thumby_MIDDIR)/pico/%,$(addsuffix .o,$(basename $(thumby_EXTCFILES))))
-include $(thumby_OFILES:.o=.d)

$(thumby_MIDDIR)/%.o     :src/%.c               ;$(PRECMD) $(thumby_CC) -o $@ $<
$(thumby_MIDDIR)/%.o     :$(thumby_MIDDIR)/%.c  ;$(PRECMD) $(thumby_CC) -o $@ $<
$(thumby_MIDDIR)/pico/%.o:$(thumby_PICO_SDK)/%.c;$(PRECMD) $(thumby_CC) -o $@ $<
$(thumby_MIDDIR)/pico/%.o:$(thumby_PICO_SDK)/%.S;$(PRECMD) $(thumby_AS) -o $@ $<

thumby_BS2SRC:=$(thumby_PICO_SDK)/build/src/rp2_common/boot_stage2/bs2_default_padded_checksummed.S 

thumby_BIN:=$(thumby_OUTDIR)/$(PROJECT_NAME)
thumby_EXE:=$(thumby_OUTDIR)/$(PROJECT_NAME).uf2
all:$(thumby_EXE)
$(thumby_BIN):$(thumby_OFILES);$(PRECMD) $(thumby_LD) -o$@ $^ $(thumby_BS2SRC) $(thumby_LDPOST)
$(thumby_EXE):$(thumby_BIN);$(PRECMD) $(thumby_ELF2UF2) $< $@

thumby-run:$(thumby_EXE); \
  while true ; do \
    if cp $(thumby_EXE) $(thumby_MOUNTPOINT) ; then break ; fi ; \
    echo "Failed to copy to $(thumby_MOUNTPOINT). Will retry after 1 second..." ; \
    sleep 1 ; \
  done
