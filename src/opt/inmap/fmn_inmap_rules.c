#include "fmn_inmap_internal.h"

/* Cleanup.
 */

void fmn_inmap_rules_del(struct fmn_inmap_rules *rules) {
  if (!rules) return;
  if (rules->refc-->1) return;

  if (rules->name) free(rules->name);
  if (rules->buttonv) free(rules->buttonv);

  free(rules);
}

/* Retain.
 */
 
int fmn_inmap_rules_ref(struct fmn_inmap_rules *rules) {
  if (!rules) return -1;
  if (rules->refc<1) return -1;
  if (rules->refc==INT_MAX) return -1;
  rules->refc++;
  return 0;
}

/* New.
 */
 
struct fmn_inmap_rules *fmn_inmap_rules_new() {
  struct fmn_inmap_rules *rules=calloc(1,sizeof(struct fmn_inmap_rules));
  if (!rules) return 0;
  rules->refc=1;
  return rules;
}

/* Search buttons.
 */

int fmn_inmap_rules_buttonv_search(const struct fmn_inmap_rules *rules,int srcbtnid) {
  int lo=0,hi=rules->buttonc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
         if (srcbtnid<rules->buttonv[ck].srcbtnid) hi=ck;
    else if (srcbtnid>rules->buttonv[ck].srcbtnid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}
