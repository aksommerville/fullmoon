/* fmn_inmap.h
 *
 * Generic input mapper, optimized for use with Full Moon on one end and 'intf' on the other.
 *
 * Some input drivers will provide pre-mapped input, we're not involved in that.
 * IoC units are on their own for managing the system keyboard. (TODO maybe we should participate there?)
 *
 * There's no delegate. After sending a batch of events, poll fmn_inmap_get_state() to see any changes.
 * (did this way because it's simpler and we know that that's how the upper framework wants it).
 */

#ifndef FMN_INMAP_H
#define FMN_INMAP_H

struct fmn_inmap;

void fmn_inmap_del(struct fmn_inmap *inmap);
struct fmn_inmap *fmn_inmap_new();

//TODO config file

// Current input state for reporting to the app. Call any time, it's cheap.
uint16_t fmn_inmap_get_state(const struct fmn_inmap *inmap);

/* Device handshake.
 * Owner should call these in order at connect, 'add_button' as many times as needed.
 * We can shake only one hand at a time; 'connect' drops any pending handshake.
 */
int fmn_inmap_connect(struct fmn_inmap *inmap,int devid);
int fmn_inmap_set_ids(struct fmn_inmap *inmap,int devid,int vid,int pid,const char *name);
int fmn_inmap_add_button(struct fmn_inmap *inmap,int devid,int btnid,int hidusage,int value,int lo,int hi);
int fmn_inmap_device_ready(struct fmn_inmap *inmap,int devid);

// Events straight off the driver.
int fmn_inmap_disconnect(struct fmn_inmap *inmap,int devid);
int fmn_inmap_event(struct fmn_inmap *inmap,int devid,int btnid,int value);

#endif
