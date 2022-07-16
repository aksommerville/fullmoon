/* thumbc.c
 * Copied from TinyCircuits Thumby.cpp; adapting to eliminate C++.
 * Eliminated Arduino APIs, and compiling in C now.
 */

#include "pico/stdlib.h"
#include "game/fullmoon.h"
#include "thumc.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include <string.h>

// https://github.com/TinyCircuits/TinyCircuits-TinierScreen-Lib/blob/master/src/TinierScreen.h#L33-L62
#define SSD1306_DEFAULT_ADDRESS 0x3C
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_SWITCHCAPVCC 0x2
#define SSD1306_NOP 0xE3

#define THUMBY_CS_PIN 16
#define THUMBY_SCK_PIN 18
#define THUMBY_SDA_PIN 19
#define THUMBY_DC_PIN 17
#define THUMBY_RESET_PIN 20

#define THUMBY_LINK_TX_PIN 0
#define THUMBY_LINK_RX_PIN 1
#define THUMBY_LINK_PU_PIN 2

#define THUMBY_BTN_LDPAD_PIN 3
#define THUMBY_BTN_RDPAD_PIN 5
#define THUMBY_BTN_UDPAD_PIN 4
#define THUMBY_BTN_DDPAD_PIN 6
#define THUMBY_BTN_B_PIN 24
#define THUMBY_BTN_A_PIN 27

#define THUMBY_AUDIO_PIN 28

// Setup pins for link, buttons, and audio
void thumby_begin() {
  
  gpio_init_mask(
    (1<<THUMBY_LINK_TX_PIN)|
    (1<<THUMBY_LINK_PU_PIN)|
    (1<<THUMBY_CS_PIN)|
    (1<<THUMBY_DC_PIN)|
    (1<<THUMBY_SCK_PIN)|
    (1<<THUMBY_SDA_PIN)|
    (1<<THUMBY_RESET_PIN)|
    (1<<THUMBY_LINK_RX_PIN)|
    (1<<THUMBY_BTN_LDPAD_PIN)|
    (1<<THUMBY_BTN_RDPAD_PIN)|
    (1<<THUMBY_BTN_UDPAD_PIN)|
    (1<<THUMBY_BTN_DDPAD_PIN)|
    (1<<THUMBY_BTN_B_PIN)|
    (1<<THUMBY_BTN_A_PIN)|
  0);
  gpio_set_dir_all_bits(
    (1<<THUMBY_LINK_TX_PIN)|
    (1<<THUMBY_LINK_PU_PIN)|
    (1<<THUMBY_CS_PIN)|
    (1<<THUMBY_DC_PIN)|
    (1<<THUMBY_SDA_PIN)|
    (1<<THUMBY_RESET_PIN)|
  0);
  
  gpio_put(THUMBY_LINK_PU_PIN,1);

  gpio_set_function(THUMBY_AUDIO_PIN, GPIO_FUNC_PWM);
  
  gpio_set_function(/*_RX*/THUMBY_CS_PIN,GPIO_FUNC_SPI);
  gpio_set_function(/*_SCK*/THUMBY_SCK_PIN,GPIO_FUNC_SPI);
  gpio_set_function(/*_TX*/THUMBY_SDA_PIN,GPIO_FUNC_SPI);
  spi_init(spi0,4000000);
  
  // Reset the screen
  gpio_put(THUMBY_RESET_PIN,0);
  busy_wait_us_32(10000);
  gpio_put(THUMBY_RESET_PIN,1);

  // SPI.cpp does this on each byte. Seems OK to do just once.
  spi_set_format(spi0,8,SPI_CPOL_0,SPI_CPHA_0,SPI_MSB_FIRST);
  
  // Init half-duplex UART for link cable
  //Serial1.begin(115200);//TODO aks: We'll need to figure out how this works, for link cable.

  gpio_set_pulls(THUMBY_BTN_LDPAD_PIN,1,0);
  gpio_set_pulls(THUMBY_BTN_RDPAD_PIN,1,0);
  gpio_set_pulls(THUMBY_BTN_UDPAD_PIN,1,0);
  gpio_set_pulls(THUMBY_BTN_DDPAD_PIN,1,0);
  gpio_set_pulls(THUMBY_BTN_B_PIN,1,0);
  gpio_set_pulls(THUMBY_BTN_A_PIN,1,0);

  // Init the screen
  thumby_sendCommand(SSD1306_DISPLAYOFF);
  thumby_sendCommand(SSD1306_SETDISPLAYCLOCKDIV);
  thumby_sendCommand(0x80);
  thumby_sendCommand(SSD1306_SETDISPLAYOFFSET);
  thumby_sendCommand(0x00);
  thumby_sendCommand(SSD1306_SETSTARTLINE | 0x00);
  thumby_sendCommand(SSD1306_DISPLAYALLON_RESUME);
  thumby_sendCommand(SSD1306_NORMALDISPLAY);
  thumby_sendCommand(SSD1306_CHARGEPUMP);
  thumby_sendCommand(0x14);
  thumby_sendCommand(SSD1306_MEMORYMODE);
  thumby_sendCommand(0x00);
  thumby_sendCommand(SSD1306_SEGREMAP|1);
  thumby_sendCommand(SSD1306_COMSCANDEC);
  thumby_sendCommand(SSD1306_SETCONTRAST);
  thumby_sendCommand(30);
  
  thumby_sendCommand(SSD1306_SETPRECHARGE);
  thumby_sendCommand(0xF1);

  thumby_sendCommand(SSD1306_SETVCOMDETECT);
  thumby_sendCommand(0x20);

  thumby_sendCommand(SSD1306_SETMULTIPLEX);
  thumby_sendCommand(40 - 1);

  thumby_sendCommand(SSD1306_SETCOMPINS);
  thumby_sendCommand(0x12);

  thumby_sendCommand(0xAD);
  thumby_sendCommand(0x30);

  thumby_sendCommand(SSD1306_DISPLAYON);
}


void thumby_sendCommand(uint8_t command){
  gpio_put(THUMBY_CS_PIN,1);
  gpio_put(THUMBY_DC_PIN,0);
  gpio_put(THUMBY_CS_PIN,0);
  spi_write_blocking(spi0,&command,1);
  gpio_put(THUMBY_CS_PIN,1);
}

void thumby_send_framebuffer(const void *v,int c) {

  thumby_sendCommand(SSD1306_COLUMNADDR);
  thumby_sendCommand(28);
  thumby_sendCommand(99);

  thumby_sendCommand(SSD1306_PAGEADDR);
  thumby_sendCommand(0x00);
  thumby_sendCommand(0x05);

  gpio_put(THUMBY_CS_PIN,1);
  gpio_put(THUMBY_DC_PIN,1);
  gpio_put(THUMBY_CS_PIN,0);
  spi_write_blocking(spi0,v,c);
  gpio_put(THUMBY_CS_PIN,1);
}

void thumby_set_brightness(uint8_t brightness){
  if (brightness>127) brightness=127;
  thumby_sendCommand(0x81);
  thumby_sendCommand(brightness);
}

uint16_t thumby_get_buttons() {
  return (
    (gpio_get(THUMBY_BTN_LDPAD_PIN)?0:FMN_BUTTON_LEFT)|
    (gpio_get(THUMBY_BTN_RDPAD_PIN)?0:FMN_BUTTON_RIGHT)|
    (gpio_get(THUMBY_BTN_UDPAD_PIN)?0:FMN_BUTTON_UP)|
    (gpio_get(THUMBY_BTN_DDPAD_PIN)?0:FMN_BUTTON_DOWN)|
    (gpio_get(THUMBY_BTN_A_PIN)?0:FMN_BUTTON_A)|
    (gpio_get(THUMBY_BTN_B_PIN)?0:FMN_BUTTON_B)|
  0);
}

// Pack dataBuf into packedBuf (adds 2 size bytes, 
// 1 checksum, and returns false if size too large to
// fit in packet, or too large to fit in packedBuf)
//TODO aks: Get a link cable and play with this. I bet I've broke it...
int8_t thumby_linkPack(uint8_t* dataBuf, uint16_t dataBufLen, uint8_t* packedBuf, uint16_t packedBufLen){
  uint16_t packetLength = dataBufLen+3;

  // Check that the data length can be indexed by two bytes and
  // that it will fit into the packed buffer with header bytes
  if(dataBufLen > 512 || packetLength > packedBufLen){
    return -1;
  }

  // Prepare packet header
  packedBuf[0] = (dataBufLen >> 8) & 0xff;
  packedBuf[1] = dataBufLen & 0xff;
  packedBuf[2] = 0;

  // Generate checksum and copy data
  for(uint16_t b=0; b<dataBufLen; b++){
    packedBuf[2] ^= dataBuf[b];
    packedBuf[b+3] = dataBuf[b];
  }

  return packetLength;
}

// Unpack packedBuf to dataBuf (removes 1 checksum byte, and 2 size 
// bytes but returns false if checksum or size check fails, or if
// stripped packedBuf data cannot fit in dataBuf)
int8_t thumby_linkUnpack(uint8_t* packedBuf, uint16_t packedBufLen, uint8_t* dataBuf, uint16_t dataBufLen){
  uint16_t dataLength = (packedBuf[0] << 8) + packedBuf[1];

  // Check that packet data will fit in data buffer and that
  // the received data length is the same as the actual received
  // packet length minus the 3 header bytes
  if(packedBufLen-3 > dataBufLen || dataLength != packedBufLen-3){
    return -1;
  }

  // Copy data and generate checksum off received data
  uint8_t checksum = 0;
  for(uint16_t b=0; b<dataLength; b++){
    dataBuf[b] = packedBuf[b+3];
    checksum ^= dataBuf[b];
  }

  // Return false if received and generated checksums are not the same
  if(checksum != packedBuf[2]){
    return -1;
  }

  return dataLength;
}

// Start playing a sound through the buzzer using pwm (does not block)
//...aks: I can't make much out of this.
void thumby_play(uint32_t freq, uint16_t duty){
  //TODO uint32_t wrap = clock_get_hz(clk_sys)/freq - 1;
  uint32_t wrap=1000000/freq-1;
  //uint32_t level = (uint32_t)(wrap * (duty / 65535.0f));
  uint16_t level=wrap>>1;

  uint8_t slice_num = pwm_gpio_to_slice_num(THUMBY_AUDIO_PIN);
  pwm_set_clkdiv_int_frac(slice_num,1,0);
  pwm_set_wrap(slice_num,wrap);
  pwm_set_chan_level(slice_num,PWM_CHAN_A,level);
  
  // Set the PWM running
  pwm_set_enabled(slice_num, true);
}

// Turn off sound that's currently playing on pwm
void thumby_stop(){
  uint8_t slice_num = pwm_gpio_to_slice_num(THUMBY_AUDIO_PIN);
  
  // Set the PWM not running
  pwm_set_enabled(slice_num, false);
}

