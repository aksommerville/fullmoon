#include "opt/minisyni/ms_internal.h"

struct minisyni minisyni={0};

/* Init.
 */
 
void minisyni_init(uint16_t rate,uint8_t chanc) {
  minisyni.rate=rate;
  minisyni.chanc=chanc;
  
  minisyni.tmppd=(uint32_t)(((float)0xffffffff*440.0f)/(float)rate);
  minisyni.tmplevel=1000;
}

/* Update.
 */
 
int16_t storage_for_wasi[512];
int32_t tmppdd=100;

int minisyni_update(int16_t *v,int32_t c) {
  if (c<1) return c;
  memset(v,0,c<<1);
  
  int32_t i=c;
  for (;i-->0;v++) {
    *v=minisyni.tmplevel*((minisyni.tmpp&0x80000000)?-1:1);
    minisyni.tmpp+=minisyni.tmppd;
    minisyni.tmppd+=tmppdd;
    if ((minisyni.tmppd>400)&&(tmppdd>0)) tmppdd=-tmppdd;
    else if ((minisyni.tmppd<100)&&(tmppdd<0)) tmppdd=-tmppdd;
  }
  return minisyni.tmplevel;
}
