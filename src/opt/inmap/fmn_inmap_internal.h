#ifndef FMN_INMAP_INTERNAL_H
#define FMN_INMAP_INTERNAL_H

#include "game/fullmoon.h"
#include "fmn_inmap.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define FMN_BUTTON_HORZ (FMN_BUTTON_LEFT|FMN_BUTTON_RIGHT)
#define FMN_BUTTON_VERT (FMN_BUTTON_UP|FMN_BUTTON_DOWN)

struct fmn_inmap {

  uint16_t state;

  int handshake_devid;
  int handshake_vid;
  int handshake_pid;
  char *handshake_name;
  struct fmn_inmap_rules {
    int refc;
    int vid,pid; // zero matches all
    char *name; // null matches all
    struct fmn_inmap_rules_button {
      int srcbtnid;
      int srclo,srchi;
      uint16_t dstbtnid;
    } *buttonv;
    int buttonc,buttona;
  } *handshake_rules;

  struct fmn_inmap_button {
    int srcbtnid;
    int srcvalue;
    int srclo,srchi;
    int dstvalue;
    uint16_t dstbtnid;
  } *handshake_buttonv;
  int handshake_buttonc,handshake_buttona;

  struct fmn_inmap_device {
    int devid;
    uint16_t state;
    struct fmn_inmap_button *buttonv;
    int buttonc,buttona;
  } **devicev;
  int devicec,devicea;

  struct fmn_inmap_rules **rulesv;
  int rulesc,rulesa;
};

void fmn_inmap_rules_del(struct fmn_inmap_rules *rules);
int fmn_inmap_rules_ref(struct fmn_inmap_rules *rules);
struct fmn_inmap_rules *fmn_inmap_rules_new();

int fmn_inmap_rules_buttonv_search(const struct fmn_inmap_rules *rules,int srcbtnid);

struct fmn_inmap_rules *fmn_inmap_find_rules(struct fmn_inmap *inmap,int vid,int pid,const char *name);

int fmn_inmap_guess_dstbtnid(int *srclo,int *srchi,const struct fmn_inmap *inmap,int usage,int devsrclo,int devsrchi);

int fmn_inmap_add_handshake_button(
  struct fmn_inmap *inmap,
  int srcbtnid,
  uint16_t dstbtnid,
  int cfgsrclo,int cfgsrchi, // range from rules, eg explicit dead zone for sticks -- usually ignore
  int devsrclo,int devsrchi // actual full range from device
);

int fmn_inmap_handshake_buttonv_search(const struct fmn_inmap *inmap,int srcbtnid);
struct fmn_inmap_button *fmn_inmap_handshake_buttonv_insert(struct fmn_inmap *inmap,int p,int srcbtnid);

void fmn_inmap_device_del(struct fmn_inmap_device *device);
struct fmn_inmap_device *fmn_inmap_device_new();

// If the device's state changes, return nonzero and fill (*mask,*bits) with the changed state.
int fmn_inmap_device_event(uint16_t *mask,uint16_t *bits,struct fmn_inmap_device *device,int btnid,int value);

int fmn_inmap_devicev_search(const struct fmn_inmap *inmap,int devid);
struct fmn_inmap_device *fmn_inmap_devicev_add(struct fmn_inmap *inmap,int p,int devid);
void fmn_inmap_devicev_remove(struct fmn_inmap *inmap,int p);

#endif
