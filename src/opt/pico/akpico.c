/* akpico.c
 * Copying selectively from the Picosystem SDK.
 * Original: https://github.com/pimoroni/picosystem
 */
 
#include "game/fullmoon.h"
#include "hardware/adc.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/vreg.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "screen.h"
#include <math.h>
#include <string.h>

/* picosystem/hardware.cpp
 */
 
enum button {
  UP    = 23,
  DOWN  = 20,
  LEFT  = 22,
  RIGHT = 21,
  A     = 18,
  B     = 19,
  X     = 17,
  Y     = 16
};

enum pin {
  RED = 14, GREEN = 13, BLUE = 15,                  // user rgb led
  CS = 5, SCK = 6, MOSI  = 7,                       // spi
  VSYNC = 8, DC = 9, LCD_RESET = 4, BACKLIGHT = 12, // screen
  AUDIO = 11,                                       // audio
  CHARGE_LED = 2, CHARGING = 24, BATTERY_LEVEL = 26 // battery / charging
};

typedef uint16_t color_t;

typedef struct buffer {
  int32_t w, h;
  color_t *data;

  //color_t *p(int32_t x, int32_t y) {
  //  return data + (x + y * w);
  //}
} buffer_t;

static color_t _fb[240 * 240] __attribute__ ((aligned (4))) = { };
static buffer_t SCREEN_obj={240,240,_fb};
static buffer_t *SCREEN = &SCREEN_obj;
static int32_t _cx = 0, _cy = 0, _cw = 240, _ch = 240;

static PIO               screen_pio  = pio0;
static uint              screen_sm   = 0;
static uint32_t          dma_channel;
static volatile int _in_flip = 0;
   
// once the dma transfer of the scanline is complete we move to the
// next scanline (or quit if we're finished)
void __isr dma_complete() {
  if(dma_channel_get_irq0_status(dma_channel)) {
    dma_channel_acknowledge_irq0(dma_channel); // clear irq flag
    _in_flip = false;
  }
}

uint16_t _gamma_correct(uint8_t v) {
  float gamma = 2.8;
  return (uint16_t)(pow((float)(v) / 100.0f, gamma) * 65535.0f + 0.5f);
}

static void backlight(uint8_t b) {
  pwm_set_gpio_level(BACKLIGHT, _gamma_correct(b));
}

static void _screen_command(uint8_t c, size_t len, const char *data) {
  gpio_put(CS, 0);
  gpio_put(DC, 0); // command mode
  spi_write_blocking(spi0, &c, 1);
  if(data) {
    gpio_put(DC, 1); // data mode
    spi_write_blocking(spi0, (const uint8_t*)data, len);
  }
  gpio_put(CS, 1);
}

static void _flip() {
  if(!_in_flip) {
    _in_flip = true;
    // if dma transfer already in process then skip
    uint32_t c = SCREEN->w * SCREEN->h / 2;
    dma_channel_transfer_from_buffer_now(dma_channel, SCREEN->data, c);
  }
}

static void _wait_vsync() {
  while(gpio_get(VSYNC))  {}  // if in vsync already wait for it to end
  while(!gpio_get(VSYNC)) {}  // now wait for vsync to occur
}

static void init_inputs(uint32_t pin_mask) {
  for(uint8_t i = 0; i < 32; i++) {
    uint32_t pin = 1U << i;
    if(pin & pin_mask) {
      gpio_set_function(pin, GPIO_FUNC_SIO);
      gpio_set_dir(pin, GPIO_IN);
      gpio_pull_up(pin);
    }
  }
}

static void init_outputs(uint32_t pin_mask) {
  for(uint8_t i = 0; i < 32; i++) {
    uint32_t pin = 1U << i;
    if(pin & pin_mask) {
      gpio_set_function(pin, GPIO_FUNC_SIO);
      gpio_set_dir(pin, GPIO_OUT);
      gpio_put(pin, 0);
    }
  }
}

/* picosystem/hardware.cpp:screen_program_init()
 * Using pio, so cool!
 */

static void screen_program_init(PIO pio, uint sm) {
  uint offset = pio_add_program(screen_pio, &screen_program);
  pio_sm_config c = screen_program_get_default_config(offset);

  pio_sm_set_consecutive_pindirs(pio, sm, MOSI, 2, true);

  // dividing the clock by two ensures we keep the spi transfer to
  // around 62.5mhz as per the st7789 datasheet when overclocking
  sm_config_set_clkdiv_int_frac(&c, 2, 1);

  // osr shifts left, autopull off, autopull threshold 32
  sm_config_set_out_shift(&c, false, false, 32);

  // configure out, set, and sideset pins
  sm_config_set_out_pins(&c, MOSI, 1);
  sm_config_set_sideset_pins(&c, SCK);

  pio_sm_set_pins_with_mask(
    pio, sm, 0, (1u << SCK) | (1u << MOSI));

  pio_sm_set_pindirs_with_mask(
    pio, sm, (1u << SCK) | (1u << MOSI), (1u << SCK) | (1u << MOSI));

  // join fifos as only tx needed (gives 8 deep fifo instead of 4)
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  pio_gpio_init(screen_pio, MOSI);
  pio_gpio_init(screen_pio, SCK);

  pio_sm_init(pio, sm, offset, &c);
  pio_sm_set_enabled(pio, sm, true);
}

/* picosystem/hardware.cpp:_init_hardware()
 */
 
static void _init_hardware() {
  // configure backlight pwm and disable backlight while setting up
  pwm_config cfg = pwm_get_default_config();
  pwm_set_wrap(pwm_gpio_to_slice_num(BACKLIGHT), 65535);
  pwm_init(pwm_gpio_to_slice_num(BACKLIGHT), &cfg, true);
  gpio_set_function(BACKLIGHT, GPIO_FUNC_PWM);
  backlight(0);

  // Apply a modest overvolt, default is 1.10v.
  // this is required for a stable 250MHz on some RP2040s
  vreg_set_voltage(VREG_VOLTAGE_1_20);
  sleep_ms(10);
  // overclock the rp2040 to 250mhz
  set_sys_clock_khz(250000, true);

  // configure control io pins
  init_inputs(A | B | X | Y | UP | DOWN | LEFT | RIGHT);
  init_outputs(CHARGE_LED);

  // configure adc channel used to monitor battery charge
  adc_init(); adc_gpio_init(BATTERY_LEVEL);

  // configure pwm channels for red, green, blue led channels
  pwm_set_wrap(pwm_gpio_to_slice_num(RED), 65535);
  pwm_init(pwm_gpio_to_slice_num(RED), &cfg, true);
  gpio_set_function(RED, GPIO_FUNC_PWM);

  pwm_set_wrap(pwm_gpio_to_slice_num(GREEN), 65535);
  pwm_init(pwm_gpio_to_slice_num(GREEN), &cfg, true);
  gpio_set_function(GREEN, GPIO_FUNC_PWM);

  pwm_set_wrap(pwm_gpio_to_slice_num(BLUE), 65535);
  pwm_init(pwm_gpio_to_slice_num(BLUE), &cfg, true);
  gpio_set_function(BLUE, GPIO_FUNC_PWM);

  // configure the spi interface used to initialise the screen
  spi_init(spi0, 8000000);

  // reset cycle the screen before initialising
  gpio_set_function(LCD_RESET, GPIO_FUNC_SIO);
  gpio_set_dir(LCD_RESET, GPIO_OUT);
  gpio_put(LCD_RESET, 0); sleep_ms(100); gpio_put(LCD_RESET, 1);

  // configure screen io pins
  gpio_set_function(DC, GPIO_FUNC_SIO); gpio_set_dir(DC, GPIO_OUT);
  gpio_set_function(CS, GPIO_FUNC_SIO); gpio_set_dir(CS, GPIO_OUT);
  gpio_set_function(SCK, GPIO_FUNC_SPI);
  gpio_set_function(MOSI, GPIO_FUNC_SPI);

  // setup the st7789 screen driver
  gpio_put(CS, 1);

  // initialise the screen configuring it as 12-bits per pixel in RGB order
  enum st7789 {
    SWRESET   = 0x01, TEON      = 0x35, MADCTL    = 0x36, COLMOD    = 0x3A,
    GCTRL     = 0xB7, VCOMS     = 0xBB, LCMCTRL   = 0xC0, VDVVRHEN  = 0xC2,
    VRHS      = 0xC3, VDVS      = 0xC4, FRCTRL2   = 0xC6, PWRCTRL1  = 0xD0,
    FRMCTR1   = 0xB1, FRMCTR2   = 0xB2, GMCTRP1   = 0xE0, GMCTRN1   = 0xE1,
    INVOFF    = 0x20, SLPOUT    = 0x11, DISPON    = 0x29, GAMSET    = 0x26,
    DISPOFF   = 0x28, RAMWR     = 0x2C, INVON     = 0x21, CASET     = 0x2A,
    RASET     = 0x2B, STE       = 0x44, DGMEN     = 0xBA,
  };

  _screen_command(SWRESET,0,0);
  sleep_ms(5);
  _screen_command(MADCTL,    1, "\x04");
  _screen_command(TEON,      1, "\x00");
  _screen_command(FRMCTR2,   5, "\x0C\x0C\x00\x33\x33");
  _screen_command(COLMOD,    1, "\x03");
  _screen_command(GAMSET,    1, "\x01");

  _screen_command(GCTRL,     1, "\x14");
  _screen_command(VCOMS,     1, "\x25");
  _screen_command(LCMCTRL,   1, "\x2C");
  _screen_command(VDVVRHEN,  1, "\x01");
  _screen_command(VRHS,      1, "\x12");
  _screen_command(VDVS,      1, "\x20");
  _screen_command(PWRCTRL1,  2, "\xA4\xA1");
  _screen_command(FRCTRL2,   1, "\x1E");
  _screen_command(GMCTRP1,  14, "\xD0\x04\x0D\x11\x13\x2B\x3F\x54\x4C\x18\x0D\x0B\x1F\x23");
  _screen_command(GMCTRN1,  14, "\xD0\x04\x0C\x11\x13\x2C\x3F\x44\x51\x2F\x1F\x1F\x20\x23");
  _screen_command(INVON,0,0);
  sleep_ms(115);
  _screen_command(SLPOUT,0,0);
  _screen_command(DISPON,0,0);
  _screen_command(CASET,     4, "\x00\x00\x00\xef");
  _screen_command(RASET,     4, "\x00\x00\x00\xef");
  _screen_command(RAMWR,0,0);

  // switch st7789 into data mode so that we can start transmitting frame
  // data - no need to issue any more commands
  gpio_put(CS, 0);
  gpio_put(DC, 1);

  // at this stage the screen is configured and expecting to receive
  // pixel data. each time we write a screen worth of data the
  // st7789 resets the data pointer back to the start meaning that
  // we can now just leave the screen in data writing mode and
  // reassign the spi pins to our pixel doubling pio. so long as
  // we always write the entire screen we'll never get out of sync.

  // enable vsync input pin, we use this to synchronise screen updates
  // ensuring no tearing
  gpio_init(VSYNC);
  gpio_set_dir(VSYNC, GPIO_IN);

  // setup the screen updating pio program
  screen_program_init(screen_pio, screen_sm);

  // initialise dma channel for transmitting pixel data to screen
  // via the screen updating pio program
  dma_channel = 0;
  dma_channel_config config = dma_channel_get_default_config(dma_channel);
  channel_config_set_bswap(&config, true);
  channel_config_set_dreq(&config, pio_get_dreq(screen_pio, screen_sm, true));
  dma_channel_configure(
    dma_channel, &config, &screen_pio->txf[screen_sm], 0, 0, false
  );
  dma_channel_set_irq0_enabled(dma_channel, true);
  irq_set_enabled(pio_get_dreq(screen_pio, screen_sm, true), true);

  irq_set_exclusive_handler(DMA_IRQ_0, dma_complete);
  irq_set_enabled(DMA_IRQ_0, true);
}
 
/* Init.
 */
 
void akpico_init() {
  _init_hardware();
  
  // picosystem.cpp does all this, is it necessary?
    // Keep the screen off...
    backlight(0);
    // Screen buffer is initialized clear; just flip it.
    _flip();
    // Wait for the DMA transfer to finish
    while (_in_flip);
    // wait for the screen to update
    _wait_vsync();
    _wait_vsync();
    // Turn the screen on
    backlight(75);
}

/* Update.
 */
 
void akpico_update() {
}

/* Read input off the GPIO pins, return in Fullmoon's namespace.
 */
 
uint16_t akpico_get_buttons() {
  uint32_t fullstate=gpio_get_all();
  uint16_t buttons=0;
  if (!(fullstate&(1<<LEFT))) buttons|=FMN_BUTTON_LEFT;
  if (!(fullstate&(1<<RIGHT))) buttons|=FMN_BUTTON_RIGHT;
  if (!(fullstate&(1<<UP))) buttons|=FMN_BUTTON_UP;
  if (!(fullstate&(1<<DOWN))) buttons|=FMN_BUTTON_DOWN;
  if (!(fullstate&(1<<B))) buttons|=FMN_BUTTON_A; // my humble opinion: (A,B) should be (south,west)
  if (!(fullstate&(1<<Y))) buttons|=FMN_BUTTON_B;
  if (!(fullstate&(1<<X))) buttons|=FMN_BUTTON_A; // might as well enable east and north too
  if (!(fullstate&(1<<A))) buttons|=FMN_BUTTON_B;
  return buttons;
}

/* Send framebuffer.
 */
 
static uint8_t skip=0;
 
void akpico_send_framebuffer(const void *src) {

  // Throw away every third frame (more to the point -- skip the vsync wait).
  // Our screen forces like 40 Hz timing and the game is designed to run at 60 Hz.
  // Experimentally, skipping 1/3 makes it run about as fast as Thumby and Linux.
  // I don't notice the skippage visually.
  if (++skip>=3) {
    skip=0;
    return;
  }
  
  _wait_vsync();
  _flip();
}

/* Initialize framebuffer.
 */
 
void fmn_platform_init_framebuffer(struct fmn_image *fb) {
  fb->v=(uint8_t*)(_fb+((240>>1)-(FMN_FBH>>1))*240+((240>>1)-(FMN_FBW>>1)));
  fb->stride=480;
}
