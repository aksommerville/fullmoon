#include "fmn_inmap_internal.h"

/* Cleanup.
 */
 
void fmn_inmap_device_del(struct fmn_inmap_device *device) {
  if (!device) return;

  if (device->buttonv) free(device->buttonv);

  free(device);
}

/* New.
 */
 
struct fmn_inmap_device *fmn_inmap_device_new() {
  struct fmn_inmap_device *device=calloc(1,sizeof(struct fmn_inmap_device));
  if (!device) return 0;
  return device;
}

/* Receive event.
 */

int fmn_inmap_device_event(uint16_t *mask,uint16_t *bits,struct fmn_inmap_device *device,int btnid,int value) {

  int lo=0,hi=device->buttonc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (btnid<device->buttonv[ck].srcbtnid) hi=ck;
    else if (btnid>device->buttonv[ck].srcbtnid) lo=ck+1;
    else { lo=ck; break; }
  }
  while ((lo>0)&&(device->buttonv[lo-1].srcbtnid==btnid)) lo--;

  int result=0;
  *mask=*bits=0;
  struct fmn_inmap_button *button=device->buttonv+lo;
  int i=hi-lo;
  for (;i-->0;button++) {
    if (button->srcbtnid!=btnid) break;
    if (value==button->srcvalue) continue;
    button->srcvalue=value;
    int dstvalue=((value>=button->srclo)&&(value<=button->srchi))?1:0;
    if (dstvalue==button->dstvalue) continue;
    button->dstvalue=dstvalue;
    (*mask)|=button->dstbtnid;
    if (dstvalue) (*bits)|=button->dstbtnid;
    result=1;
  }
  
  return result;
}
