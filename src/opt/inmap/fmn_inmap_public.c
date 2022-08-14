#include "fmn_inmap_internal.h"

/* Cleanup.
 */
 
void fmn_inmap_del(struct fmn_inmap *inmap) {
  if (!inmap) return;

  if (inmap->handshake_name) free(inmap->handshake_name);
  fmn_inmap_rules_del(inmap->handshake_rules);
  if (inmap->handshake_buttonv) free(inmap->handshake_buttonv);

  if (inmap->devicev) {
    while (inmap->devicec-->0) fmn_inmap_device_del(inmap->devicev[inmap->devicec]);
    free(inmap->devicev);
  }

  if (inmap->rulesv) {
    while (inmap->rulesc-->0) fmn_inmap_rules_del(inmap->rulesv[inmap->rulesc]);
    free(inmap->rulesv);
  }

  free(inmap);
}

/* New.
 */
 
struct fmn_inmap *fmn_inmap_new() {
  struct fmn_inmap *inmap=calloc(1,sizeof(struct fmn_inmap));
  if (!inmap) return 0;

  return inmap;
}

/* Trivial accessors.
 */

uint16_t fmn_inmap_get_state(const struct fmn_inmap *inmap) {
  if (!inmap) return 0;
  return inmap->state;
}

/* Handshake.
 */

int fmn_inmap_connect(struct fmn_inmap *inmap,int devid) {
  if (!inmap) return -1;
  if (devid<1) return -1;
  
  inmap->handshake_devid=devid;
  inmap->handshake_vid=0;
  inmap->handshake_pid=0;
  inmap->handshake_buttonc=0;
  
  if (inmap->handshake_name) {
    free(inmap->handshake_name);
    inmap->handshake_name=0;
  }

  fmn_inmap_rules_del(inmap->handshake_rules);
  inmap->handshake_rules=0;
  
  return 0;
}

int fmn_inmap_set_ids(struct fmn_inmap *inmap,int devid,int vid,int pid,const char *name) {
  if (!inmap) return -1;
  if (devid!=inmap->handshake_devid) return -1;
  inmap->handshake_vid=vid;
  inmap->handshake_pid=pid;
  if (name) {
    if (inmap->handshake_name) free(inmap->handshake_name);
    inmap->handshake_name=strdup(name);
  }
  fmn_inmap_rules_del(inmap->handshake_rules);
  if (inmap->handshake_rules=fmn_inmap_find_rules(inmap,vid,pid,name)) {
    fprintf(stderr,"%s: Configuring device %04x:%04x '%s' from explicit rules.\n",__FILE__,vid,pid,name);
  } else {
    fprintf(stderr,"%s: Configuring device %04x:%04x '%s' with default rules.\n",__FILE__,vid,pid,name);
  }
  return 0;
}

/* Receive notice of a button on the handshake device.
 */

int fmn_inmap_add_button(struct fmn_inmap *inmap,int devid,int btnid,int hidusage,int value,int lo,int hi) {
  if (!inmap) return -1;
  if (devid!=inmap->handshake_devid) return -1;

  // If we have rules, we only map the buttons named there.
  if (inmap->handshake_rules) {
    int p=fmn_inmap_rules_buttonv_search(inmap->handshake_rules,btnid);
    if (p>=0) {
      const struct fmn_inmap_rules_button *button=inmap->handshake_rules->buttonv+p;
      if (fmn_inmap_add_handshake_button(
        inmap,btnid,button->dstbtnid,button->srclo,button->srchi,lo,hi
      )<0) return -1;
    }

  // No rules, we'll guess a mapping one at a time.
  } else {
    int srclo,srchi;
    uint16_t dstbtnid=fmn_inmap_guess_dstbtnid(&srclo,&srchi,inmap,hidusage,lo,hi);
    if (dstbtnid) {
      if (fmn_inmap_add_handshake_button(
        inmap,btnid,dstbtnid,srclo,srchi,lo,hi
      )<0) return -1;
    }

  }
  return 0;
}

/* Commit handshake device, or drop it.
 */

int fmn_inmap_device_ready(struct fmn_inmap *inmap,int devid) {
  if (!inmap) return -1;
  if (devid!=inmap->handshake_devid) return -1;
  inmap->handshake_devid=0;

  // Confirm it has all of our favorite buttons.
  const uint16_t required=FMN_BUTTON_LEFT|FMN_BUTTON_RIGHT|FMN_BUTTON_UP|FMN_BUTTON_DOWN|FMN_BUTTON_A|FMN_BUTTON_B;
  uint16_t caps=0;
  const struct fmn_inmap_button *button=inmap->handshake_buttonv;
  int i=inmap->handshake_buttonc;
  for (;i-->0;button++) {
    caps|=button->dstbtnid;
  }
  if ((caps&required)!=required) {
    fprintf(stderr,
      "Ignoring input device %04x:%04x '%s' as it does not have all 6 required buttons.\n",
      inmap->handshake_vid,inmap->handshake_pid,inmap->handshake_name
    );
    return 0;
  }

  // Create a new device.
  int p=fmn_inmap_devicev_search(inmap,devid);
  if (p>=0) return 0; // oops
  p=-p-1;
  struct fmn_inmap_device *device=fmn_inmap_devicev_add(inmap,p,devid);
  if (!device) return 0;

  // Handoff the button list.
  device->buttonv=inmap->handshake_buttonv;
  device->buttonc=inmap->handshake_buttonc;
  device->buttona=inmap->handshake_buttona;
  inmap->handshake_buttonv=0;
  inmap->handshake_buttonc=0;
  inmap->handshake_buttona=0;
  
  return 0;
}

/* Disconnect.
 */

int fmn_inmap_disconnect(struct fmn_inmap *inmap,int devid) {
  if (!inmap) return 0;
  int p=fmn_inmap_devicev_search(inmap,devid);
  if (p<0) return 0;

  // Release any buttons held by this device.
  inmap->state&=~inmap->devicev[p]->state;
  
  fmn_inmap_devicev_remove(inmap,p);
  return 0;
}

/* Event.
 */
 
int fmn_inmap_event(struct fmn_inmap *inmap,int devid,int btnid,int value) {
  if (!inmap) return 0;
  int p=fmn_inmap_devicev_search(inmap,devid);
  if (p<0) return 0;
  struct fmn_inmap_device *device=inmap->devicev[p];
  uint16_t mask=0,bits=0;
  if (fmn_inmap_device_event(&mask,&bits,device,btnid,value)) {
    inmap->state=(inmap->state&~mask)|bits;
  }
  return 0;
}

/* Device list.
 */
  
int fmn_inmap_devicev_search(const struct fmn_inmap *inmap,int devid) {
  int lo=0,hi=inmap->devicec;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (devid<inmap->devicev[ck]->devid) hi=ck;
    else if (devid>inmap->devicev[ck]->devid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

struct fmn_inmap_device *fmn_inmap_devicev_add(struct fmn_inmap *inmap,int p,int devid) {

  if (p<0) {
    p=fmn_inmap_devicev_search(inmap,devid);
    if (p>=0) return 0;
    p=-p-1;
  }
  if ((p<0)||(p>inmap->devicec)) return 0;
  if (p&&(devid<=inmap->devicev[p-1]->devid)) return 0;
  if ((p<inmap->devicec)&&(devid>=inmap->devicev[p]->devid)) return 0;

  if (inmap->devicec>=inmap->devicea) {
    int na=inmap->devicea+8;
    if (na>INT_MAX/sizeof(void*)) return 0;
    void *nv=realloc(inmap->devicev,sizeof(void*)*na);
    if (!nv) return 0;
    inmap->devicev=nv;
    inmap->devicea=na;
  }

  struct fmn_inmap_device *device=fmn_inmap_device_new();
  if (!device) return 0;
  device->devid=devid;

  memmove(inmap->devicev+p+1,inmap->devicev+p,sizeof(void*)*(inmap->devicec-p));
  inmap->devicec++;
  inmap->devicev[p]=device;

  return device;
}

void fmn_inmap_devicev_remove(struct fmn_inmap *inmap,int p) {
  if ((p<0)||(p>=inmap->devicec)) return;
  fmn_inmap_device_del(inmap->devicev[p]);
  inmap->devicec--;
  memmove(inmap->devicev+p,inmap->devicev+p+1,sizeof(void*)*(inmap->devicec-p));
}

/* Find rules for a device id.
 */
 
struct fmn_inmap_rules *fmn_inmap_find_rules(struct fmn_inmap *inmap,int vid,int pid,const char *name) {
  struct fmn_inmap_rules **p=inmap->rulesv;
  int i=inmap->rulesc;
  for (;i-->0;p++) {
    struct fmn_inmap_rules *rules=*p;
    if (rules->vid&&(rules->vid!=vid)) continue;
    if (rules->pid&&(rules->pid!=pid)) continue;
    if (rules->name&&(!name||strcmp(rules->name,name))) continue;
    if (fmn_inmap_rules_ref(rules)<0) return 0;
    return rules;
  }
  return 0;
}

/* Handshake button list primitives.
 */

int fmn_inmap_handshake_buttonv_search(const struct fmn_inmap *inmap,int srcbtnid) {
  int lo=0,hi=inmap->handshake_buttonc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (srcbtnid<inmap->handshake_buttonv[ck].srcbtnid) hi=ck;
    else if (srcbtnid>inmap->handshake_buttonv[ck].srcbtnid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

struct fmn_inmap_button *fmn_inmap_handshake_buttonv_insert(struct fmn_inmap *inmap,int p,int srcbtnid) {
  if ((p<0)||(p>inmap->handshake_buttonc)) return 0;
  // Duplicate (srcbtnid) is OK but they do need to be sorted.
  if (p&&(srcbtnid<inmap->handshake_buttonv[p-1].srcbtnid)) return 0;
  if ((p<inmap->handshake_buttonc)&&(srcbtnid>inmap->handshake_buttonv[p].srcbtnid)) return 0;

  if (inmap->handshake_buttonc>=inmap->handshake_buttona) {
    int na=inmap->handshake_buttona+16;
    if (na>INT_MAX/sizeof(struct fmn_inmap_button)) return 0;
    void *nv=realloc(inmap->handshake_buttonv,sizeof(struct fmn_inmap_button)*na);
    if (!nv) return 0;
    inmap->handshake_buttonv=nv;
    inmap->handshake_buttona=na;
  }

  struct fmn_inmap_button *button=inmap->handshake_buttonv+p;
  memmove(button+1,button,sizeof(struct fmn_inmap_button)*(inmap->handshake_buttonc-p));
  inmap->handshake_buttonc++;
  memset(button,0,sizeof(struct fmn_inmap_button));
  button->srcbtnid=srcbtnid;
  return button;
}

/* Add a mapping during handshake.
 */
 
int fmn_inmap_add_handshake_button(
  struct fmn_inmap *inmap,
  int srcbtnid,
  uint16_t dstbtnid,
  int cfgsrclo,int cfgsrchi, // range from rules, eg explicit dead zone for sticks -- usually ignore
  int devsrclo,int devsrchi // actual full range from device
) {

  // We split up aggregate buttons HORZ and VERT.
  switch (dstbtnid) {
    case FMN_BUTTON_HORZ: {
        if (devsrclo>devsrchi-2) return 0;
        if (cfgsrclo<devsrclo) cfgsrclo=devsrclo;
        if (cfgsrchi>devsrchi) cfgsrchi=devsrchi;
        if (cfgsrclo>=cfgsrchi-1) {
          cfgsrclo=devsrclo;
          cfgsrchi=devsrchi;
        }
        if (fmn_inmap_add_handshake_button(inmap,srcbtnid,FMN_BUTTON_LEFT,INT_MIN,cfgsrclo,INT_MIN,INT_MAX)<0) return -1;
        if (fmn_inmap_add_handshake_button(inmap,srcbtnid,FMN_BUTTON_RIGHT,cfgsrchi,INT_MAX,INT_MIN,INT_MAX)<0) return -1;
      } return 0;
    case FMN_BUTTON_VERT: {
        if (devsrclo>devsrchi-2) return 0;
        if (cfgsrclo<devsrclo) cfgsrclo=devsrclo;
        if (cfgsrchi>devsrchi) cfgsrchi=devsrchi;
        if (cfgsrclo>=cfgsrchi-1) {
          cfgsrclo=devsrclo;
          cfgsrchi=devsrchi;
        }
        if (fmn_inmap_add_handshake_button(inmap,srcbtnid,FMN_BUTTON_UP,INT_MIN,cfgsrclo,INT_MIN,INT_MAX)<0) return -1;
        if (fmn_inmap_add_handshake_button(inmap,srcbtnid,FMN_BUTTON_DOWN,cfgsrchi,INT_MAX,INT_MIN,INT_MAX)<0) return -1;
      } return 0;
  }

  // Everything else should be a single bit.
  // It's OK if srcbtnid collides, there can be more than one.
  int p=fmn_inmap_handshake_buttonv_search(inmap,srcbtnid);
  if (p<0) p=-p-1;
  struct fmn_inmap_button *button=fmn_inmap_handshake_buttonv_insert(inmap,p,srcbtnid);
  if (!button) return -1;
  button->dstbtnid=dstbtnid;
  button->srcvalue=0;
  button->dstvalue=0;
  if ((cfgsrclo>=devsrclo)&&(cfgsrchi<=devsrchi)) {
    button->srclo=cfgsrclo;
    button->srchi=cfgsrchi;
  } else {
    button->srclo=(devsrclo+devsrchi)>>1;
    if (button->srclo<=devsrclo) button->srclo=devsrclo+1;
    button->srchi=INT_MAX;
  }

  return 0;
}

/* Guessing a map target, provide the least-used so far of (HORZ,VERT) or (A,B).
 */

static int fmn_inmap_handshake_least_mapped_axis(const struct fmn_inmap *inmap) {
  int xc=0,yc=0;
  const struct fmn_inmap_button *button=inmap->handshake_buttonv;
  int i=inmap->handshake_buttonc;
  for (;i-->0;button++) {
    switch (button->dstbtnid) {
      case FMN_BUTTON_LEFT: xc++; break;
      case FMN_BUTTON_RIGHT: xc++; break;
      case FMN_BUTTON_UP: yc++; break;
      case FMN_BUTTON_DOWN: yc++; break;
    }
  }
  // X wins ties.
  return (xc<=yc)?FMN_BUTTON_HORZ:FMN_BUTTON_VERT;
}

static int fmn_inmap_handshake_least_mapped_button(const struct fmn_inmap *inmap) {
  int ac=0,bc=0;
  const struct fmn_inmap_button *button=inmap->handshake_buttonv;
  int i=inmap->handshake_buttonc;
  for (;i-->0;button++) {
    switch (button->dstbtnid) {
      case FMN_BUTTON_A: ac++; break;
      case FMN_BUTTON_B: bc++; break;
    }
  }
  // A wins ties.
  return (ac<=bc)?FMN_BUTTON_A:FMN_BUTTON_B;
}

/* Guess output button from HID usage, range, and existing mapped buttons.
 */
 
int fmn_inmap_guess_dstbtnid(int *srclo,int *srchi,const struct fmn_inmap *inmap,int usage,int devsrclo,int devsrchi) {

  // If it reports fewer than two possible values, we can't use it.
  if (devsrclo>=devsrchi) return 0;

  // Ignore anything in the vendor ranges.
  if (usage>=0xff000000) return 0;

  #define TWOSTATE(tag) { \
    *srclo=(devsrclo+devsrchi)>>1; \
    if (*srclo<=devsrclo) *srclo=devsrclo+1; \
    *srchi=INT_MAX; \
    return FMN_BUTTON_##tag; \
  }
  #define THREEWAY(tag) { \
    if (devsrclo<=devsrchi-2) { \
      int mid=(devsrclo+devsrchi)>>1; \
      *srclo=(devsrclo+mid)>>1; \
      *srchi=(devsrchi+mid)>>1; \
      return FMN_BUTTON_##tag; \
    } \
  }

  // Well-behaved devices should report a sane HID Usage code for each button. Use it if we recognize it.
  switch (usage) {

    case 0x00010030: THREEWAY(HORZ) break; // absolute axes
    case 0x00010031: THREEWAY(VERT) break;
    case 0x00010033: THREEWAY(HORZ) break;
    case 0x00010034: THREEWAY(VERT) break;
    
    case 0x00010090: TWOSTATE(UP) break; // dpad
    case 0x00010091: TWOSTATE(DOWN) break;
    case 0x00010092: TWOSTATE(RIGHT) break;
    case 0x00010093: TWOSTATE(LEFT) break;

    case 0x00070036: TWOSTATE(B) break; // comma
    case 0x00070037: TWOSTATE(A) break; // dot
    case 0x0007004f: TWOSTATE(RIGHT) break; // arrows
    case 0x00070050: TWOSTATE(LEFT) break;
    case 0x00070051: TWOSTATE(DOWN) break;
    case 0x00070052: TWOSTATE(UP) break;

    case 0x00070058: TWOSTATE(B) break; // kp enter
    case 0x0007005a: TWOSTATE(DOWN) break; // kp 2
    case 0x0007005c: TWOSTATE(LEFT) break; // kp 4
    case 0x0007005d: TWOSTATE(DOWN) break; // kp 5
    case 0x0007005e: TWOSTATE(RIGHT) break; // kp 6
    case 0x00070060: TWOSTATE(UP) break; // kp 8
    case 0x00070062: TWOSTATE(A) break; // kp 0
  }

  // Anything else on the keyboard page, ignore it. We don't want 105 mappings...
  if ((usage>=0x00070000)&&(usage<=0x0007ffff)) {
    return 0;
  }

  // Generic buttons, page 9. Even are A and odd are B.
  if ((usage>=0x00090000)&&(usage<=0x0009ffff)) {
    *srclo=(devsrclo+devsrchi)>>1;
    if (*srclo<=devsrclo) *srclo=devsrclo+1;
    *srchi=INT_MAX;
    return (usage&1)?FMN_BUTTON_A:FMN_BUTTON_B;
  }

  // If it has at least 3 states and straddles zero, call it an axis. Whichever axis has the fewest mappings so far.
  if ((devsrclo<0)&&(devsrchi>0)) {
    int mid=(devsrclo+devsrchi)>>1;
    *srclo=(devsrclo+mid)>>1;
    *srchi=(devsrchi+mid)>>1;
    return fmn_inmap_handshake_least_mapped_axis(inmap);
  }

  // If its low end is zero and has 2 or 3 states, call it a button.
  if (!devsrclo&&((devsrchi==1)||(devsrchi==2))) {
    *srclo=1;
    *srchi=devsrchi;
    return fmn_inmap_handshake_least_mapped_button(inmap);
  }

  // Don't try to map buttons with no declared usage and ranges like 0..255 -- we can't tell whether it's a stick or a button.

  #undef TWOSTATE
  #undef THREEWAY
  return 0;
}
