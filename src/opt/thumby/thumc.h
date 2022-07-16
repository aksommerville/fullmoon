/* thumc.h
 * Stripped-down interface to Thumby, with C linkage.
 */
 
#ifndef THUMC_H
#define THUMC_H

#include <stdint.h>

// Call once, early.
void thumby_begin();

/* sendCommand to send one byte over SPI to the screen -- you don't need to do that.
 * send_framebuffer to deliver the full 72x40 1-bit framebuffer.
 * If the length you supply is not 360, I'm not sure what happens.
 */
void thumby_sendCommand(uint8_t command);
void thumby_send_framebuffer(const void *v,int c);

// 0..127. Never black, 0 is legible. Above 20 or so, I don't see much difference.
void thumby_set_brightness(uint8_t brightness);

// Mask of all buttons in Full Moon's namespace.
uint16_t thumby_get_buttons();

/* For the serial link. I haven't used this ever, just copied from Thumby SDK.
 */
int8_t thumby_linkPack(uint8_t* dataBuf, uint16_t dataBufLen, uint8_t* packedBuf, uint16_t packedBufLen);
int8_t thumby_linkUnpack(uint8_t* packedBuf, uint16_t packedBufLen, uint8_t* dataBuf, uint16_t dataBufLen);

// Audio. Mostly broken.
void thumby_play(uint32_t freq, uint16_t duty);
void thumby_stop();

#endif
