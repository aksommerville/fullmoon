#include "songcvt.h"

/* Converter context.
 */
 
struct m2m_context {
  struct songcvt *songcvt; // WEAK
  
  // MThd:
  uint16_t track_count;
  uint16_t division;
  uint16_t format;
  
  // MTrk:
  struct m2m_track {
    const uint8_t *v; // WEAK; points into (songcvt->src)
    int c;
    int p;
    uint8_t rstat;
    int term;
    int has_notes;
    int delay; // ticks; <0 if we need to read it
  } *trackv;
  int trackc,tracka;
  
  int tickc_output;
  
  // Record all held notes, during processing.
  struct m2m_note {
    int time; // (tickc_output) at the moment of Note On.
    int binp; // points to the start of a Note On event in (songcvt->bin.v).
    uint8_t chid;
    uint8_t noteid;
  } *notev;
  int notec,notea;
};

static void m2m_context_cleanup(struct m2m_context *ctx) {
  if (ctx->trackv) free(ctx->trackv);
  if (ctx->notev) free(ctx->notev);
}

/* Read VLQ.
 */
 
static int m2m_read_vlq(int *dst,const uint8_t *src,int srcc) {
  if (srcc<1) return -1;
  if (!(src[0]&0x80)) {
    *dst=src[0];
    return 1;
  }
  if (srcc<2) return -1;
  if (!(src[1]&0x80)) {
    *dst=((src[0]&0x7f)<<7)|src[1];
    return 2;
  }
  if (srcc<3) return -1;
  if (!(src[2]&0x80)) {
    *dst=((src[0]&0x7f)<<14)|((src[1]&0x7f)<<7)|src[2];
    return 3;
  }
  if (srcc<4) return -1;
  if (!(src[3]&0x80)) {
    *dst=((src[0]&0x7f)<<21)|((src[1]&0x7f)<<14)|((src[2]&0x7f)<<7)|src[3];
    return 4;
  }
  // MIDI files do not permit VLQ sequences longer than 4 bytes, ie 0x0fffffff
  return -1;
}

/* Read event.
 */
 
#define M2M_OPCODE_SYSEX 0xf0 /* (v,c) */
#define M2M_OPCODE_META  0xff /* (a=type,v,c) */
 
struct m2m_event {
  uint8_t opcode;
  uint8_t chid;
  uint8_t a,b;
  const uint8_t *v;
  int c;
};
 
static int m2m_read_event(struct m2m_event *event,const uint8_t *src,int srcc,uint8_t *rstat) {
  if (srcc<1) return -1;
  int srcp=0;

  uint8_t lead;
  if (!(src[srcp]&0x80)) lead=*rstat;
  else lead=src[srcp++];
  
  // A reasonable first guess:
  *rstat=lead;
  event->opcode=lead&0xf0;
  event->chid=lead&0x0f;
  event->v=0;
  event->c=0;
  
  #define AB { \
    if (srcp>srcc-2) return -1; \
    event->a=src[srcp++]; \
    event->b=src[srcp++]; \
  }
  #define A { \
    if (srcp>srcc-1) return -1; \
    event->a=src[srcp++]; \
    event->b=0; \
  }
  switch (lead&0xf0) {
  
    // Regular Channel events.
    case 0x80: AB break;
    case 0x90: AB if (!event->b) { event->opcode=0x80; event->b=0x40; } break;
    case 0xa0: AB break;
    case 0xb0: AB break;
    case 0xc0: A break;
    case 0xd0: A break;
    case 0xe0: AB break;
    
    // Meta and Sysex.
    case 0xf0: {
        *rstat=0;
        event->chid=0xff;
        event->a=0;
        event->b=0;
        switch (lead) {
          case 0xf0: case 0xf7: {
              event->opcode=M2M_OPCODE_SYSEX;
              int len,err;
              if ((err=m2m_read_vlq(&len,src+srcp,srcc-srcp))<1) return -1;
              srcp+=err;
              if (srcp>srcc-len) return -1;
              event->v=src+srcp;
              event->c=len;
              srcp+=len;
            } break;
          case 0xff: {
              event->opcode=M2M_OPCODE_META;
              if (srcp>=srcc) return -1;
              event->a=src[srcp++];
              int len,err;
              if ((err=m2m_read_vlq(&len,src+srcp,srcc-srcp))<1) return -1;
              srcp+=err;
              if (srcp>srcc-len) return -1;
              event->v=src+srcp;
              event->c=len;
              srcp+=len;
            } break;
          default: return -1;
        }
      } break;
    
    default: return -1;
  }
  #undef AB
  #undef A

  return srcp;
}

/* Find a Meta Set Tempo event, or return the default.
 * Always returns something valid, in us/qnote.
 */
 
static int m2m_find_tempo_in_track(const uint8_t *v,int c) {
  int p=0;
  uint8_t rstat=0;
  while (p<c) {
    int n;
    int err=m2m_read_vlq(&n,v+p,c-p);
    if (err<1) return 0;
    p+=err;
    struct m2m_event event;
    if ((err=m2m_read_event(&event,v+p,c-p,&rstat))<1) return 0;
    if (event.opcode==M2M_OPCODE_META) {
      if (event.a==0x51) {
        if (event.c>=3) {
          return (event.v[0]<<16)|(event.v[1]<<8)|event.v[2];
        }
      }
    }
  }
  return 0;
}
 
static int m2m_find_tempo(const struct m2m_context *ctx) {
  const struct m2m_track *track=ctx->trackv;
  int i=ctx->trackc;
  for (;i-->0;track++) {
    int tempo=m2m_find_tempo_in_track(track->v,track->c);
    if (tempo>0) return tempo;
  }
  return 500000; // 120 bpm, a sensible default.
}

/* Emit delay events.
 */
 
static int m2m_output_delay(struct m2m_context *ctx,int tickc) {
  ctx->tickc_output+=tickc;
  while (tickc>0x7f) {
    if (encode_raw(&ctx->songcvt->bin,"\x7f",1)<0) return -1;
    tickc-=0x7f;
  }
  if (tickc) {
    uint8_t serial={tickc};
    if (encode_raw(&ctx->songcvt->bin,&serial,1)<0) return -1;
  }
  return 0;
}

/* Note Off.
 */
 
static int m2m_process_note_off(struct m2m_context *ctx,struct m2m_track *track,struct m2m_event *event) {

  // Find our record of the note. If it's not there, ignore the event.
  struct m2m_note *note=ctx->notev;
  int notep=0;
  for (;notep<ctx->notec;note++,notep++) {
    if ((note->chid==event->chid)&&(note->noteid==event->a)) break;
  }
  if (notep>=ctx->notec) return 0;
  
  // If the elapsed time less than 1024 ticks (likely!), change the existing note to Fire-Forget.
  int duration=ctx->tickc_output-note->time;
  if (duration<0x400) {
    uint8_t *dst=ctx->songcvt->bin.v+note->binp;
    // 1111 wwwt tttt tttt tnnn nnnn: Fire-forget note (n), wave (w), duration (t) ticks.
    dst[0]=0xf0|((note->chid&7)<<1)|(duration>>9);
    dst[1]=duration>>1;
    dst[2]=(duration<<7)|note->noteid;
    
  // Very long note, no worries, emit a Note Off.
  } else {
    uint8_t tmp[3]={
      0x80|event->chid,
      event->a,
      event->b,
    };
    if (encode_raw(&ctx->songcvt->bin,tmp,sizeof(tmp))<0) return -1;
  }
  
  // ...and either way, drop the note record.
  ctx->notec--;
  memmove(note,note+1,sizeof(struct m2m_note)*(ctx->notec-notep));
  
  return 0;
}

/* Note On.
 */
 
static int m2m_process_note_on(struct m2m_context *ctx,struct m2m_track *track,struct m2m_event *event) {

  if (track) track->has_notes=1;

  // Error if it's already held.
  //TODO Does this ever happen? Maybe the right thing to do would be emit a fake Note Off first.
  //...it does happen, if I stupidly export a multi-track song into all one channel, oops.
  { const struct m2m_note *note=ctx->notev;
    int i=ctx->notec;
    for (;i-->0;note++) {
      if ((note->chid==event->chid)&&(note->noteid==event->a)) {
        /*
        fprintf(stderr,"%s: Double-pressed note 0x%02x on channel %d.\n",ctx->songcvt->srcpath,note->noteid,note->chid);
        return -2;
        */
        if (m2m_process_note_off(ctx,track,event)<0) return -1;
        break;
      }
    }
  }
  
  // Record in (ctx->notev).
  if (ctx->notec>=ctx->notea) {
    int na=ctx->notea+32;
    if (na>INT_MAX/sizeof(struct m2m_note)) return -1;
    void *nv=realloc(ctx->notev,sizeof(struct m2m_note)*na);
    if (!nv) return -1;
    ctx->notev=nv;
    ctx->notea=na;
  }
  struct m2m_note *note=ctx->notev+ctx->notec++;
  note->time=ctx->tickc_output;
  note->binp=ctx->songcvt->bin.c;
  note->chid=event->chid;
  note->noteid=event->a;
  
  // Emit a MIDI Note On event. We might rewrite it later as Minisyni Fire-and-Forget; they're the same size.
  uint8_t tmp[3]={
    0x90|event->chid,
    event->a,
    event->b,
  };
  if (encode_raw(&ctx->songcvt->bin,tmp,sizeof(tmp))<0) return -1;

  return 0;
}

/* Modify an event in place according to the rules from our adjustment file.
 * For now, adjustments are all one-to-one or one-to-zero. Will need to restructure this if there's ever one-to-many.
 */
 
static void m2m_apply_rules_to_event(struct m2m_context *ctx,struct m2m_event *event,int trackid) {
  struct songcvt_rule *rule=ctx->songcvt->rulev;
  int i=ctx->songcvt->rulec;
  for (;i-->0;rule++) {
  
    // Check criteria.
    if ((rule->match_MTrk>=0)&&(rule->match_MTrk!=trackid)) continue;
    if ((rule->match_chid>=0)&&(rule->match_chid!=event->chid)) continue;
    if (rule->match_pid>=0) {
      if (event->opcode!=0x0c) continue;
      if (rule->match_pid!=event->a) continue;
    }
    
    // Apply actions.
    if (rule->set_chid>=0) event->chid=rule->set_chid;
    if (rule->set_pid>=0) {
      if (event->opcode==0x0c) event->a=rule->set_pid;
    }
  
  }
}

/* Read the next event off this track, then emit output and update state accordingly.
 */
 
static int m2m_process_event(struct m2m_context *ctx,struct m2m_track *track) {
  int err;
  struct m2m_event event;
  if ((err=m2m_read_event(&event,track->v+track->p,track->c-track->p,&track->rstat))<0) return err;
  track->p+=err;
  track->delay=-1;
  m2m_apply_rules_to_event(ctx,&event,track-ctx->trackv);
  switch (event.opcode) {
    case 0x80: return m2m_process_note_off(ctx,track,&event);
    case 0x90: return m2m_process_note_on(ctx,track,&event);
    //TODO all of these events are now supported and we ought to encode them:
    case 0xa0: break; // Note Adjust; not interesting to us.
    case 0xb0: break; // Control Change, believe it or not, not interesting.
    case 0xc0: break; // Program Change, ''.
    case 0xd0: break; // Channel Pressure, never interesting.
    case 0xe0: break; // Pitch Wheel. Not currently supported but we might in the future.
    case M2M_OPCODE_META: switch (event.a) {
        case 0x2f: track->term=1; break; // EOT. Usually redundant but whatever, easy to support.
      } break;
    case M2M_OPCODE_SYSEX: break; // Probably never interesting.
  }
  return 0;
}

/* Advance by at least one step, producing output.
 * Output must already contain the header.
 * Returns <0 on errors, 0 on complete, >0 to continue.
 */
 
static int m2m_advance(struct m2m_context *ctx) {

  // Check delay on each track and process an event if there's one at time zero.
  int mindelay=INT_MAX;
  struct m2m_track *track=ctx->trackv;
  int i=ctx->trackc;
  for (;i-->0;track++) {
  
    if (track->term) continue;
    if (track->p>=track->c) {
      track->term=1;
      continue;
    }
    
    if (track->delay<0) {
      int err=m2m_read_vlq(&track->delay,track->v+track->p,track->c-track->p);
      if (err<1) return -1;
      track->p+=err;
    }
    
    if (!track->delay) {
      int err=m2m_process_event(ctx,track);
      if (err<0) return err;
      return 1;
    }
    
    if (track->delay<mindelay) mindelay=track->delay;
  }
  
  // No delays recorded? All tracks are complete, we're done.
  // (a track's delay can't be INT_MAX; it can't be above 0x0fffffff)
  if (mindelay==INT_MAX) return 0;
  
  // Emit a delay, and update all track delays.
  if (m2m_output_delay(ctx,mindelay)<0) return -1;
  for (track=ctx->trackv,i=ctx->trackc;i-->0;track++) {
    if (track->term) continue;
    track->delay-=mindelay;
  }
  
  // Emittng the delay counts as progress.
  return 1;
}

/* With the tracks identified, "play" the song and "record" it as minisyni.
 */
 
static int m2m_play_and_record(struct m2m_context *ctx) {

  /* We are not going to support tempo changes, and we'll use the same tick length as the input.
   * Either of those constraints could be lifted, it's really just for our convenience right here.
   */
  double sperqnote=m2m_find_tempo(ctx)/1000000.0;
  double spertick=sperqnote/ctx->division;
  double uspertickf=spertick*1000000.0;
  uint16_t uspertick;
  if (uspertickf<1.0) uspertick=1;
  else if (uspertickf>65535.0) uspertick=0xffff;
  else uspertick=(uint16_t)uspertickf;
  
  // Emit minisyni header. Loop point is initially the start, but we might learn better later, no worries.
  // Use encoder_replace(), not encode_raw(), as there might be some content already.
  uint8_t hdr[6]={
    uspertick>>8,uspertick,
    0,6, // start point, always 6
    0,6, // loop point. may overwrite later
  };
  if (encoder_replace(&ctx->songcvt->bin,0,0,hdr,sizeof(hdr))<0) return -1;

  // Exhaust all tracks.
  while (1) {
    int err=m2m_advance(ctx);
    if (err<=0) return err;
  }
}

/* Condense delay events.
 * We tend to generate split-up delays because not every input event creates an output event.
 * We also take this opportunity to drop long trailing delays, eg produced by Logic Pro X.
 */
 
static int m2m_flush_delay(uint8_t *dst,int delay) {
  if (!delay) return 0;
  int dstc=0;
  while (delay>=0x7f) {
    dst[dstc++]=0x7f;
    delay-=0x7f;
  }
  if (delay>0) {
    dst[dstc++]=delay;
  }
  return dstc;
}
 
static int m2m_condense_delays(void *dstpp,const uint8_t *src,int srcc,struct m2m_context *ctx) {
  if (srcc<6) return -1;
  uint8_t *dst=malloc(srcc); // (dst) can only be smaller than (src), allocate all we could need.
  if (!dst) return -1;
  
  // Take the 6-byte header verbatim.
  memcpy(dst,src,6);
  int dstc=6,srcp=6;
  int srcloopp=(src[4]<<8)|src[5];
  
  int dstlen=0; // output duration in ticks
  int delay=0;
  while (srcp<srcc) {
  
    // At the input loop point (must be on an event boundary), update the output loop point.
    if (srcp==srcloopp) {
      dst[4]=dstc>>8;
      dst[5]=dstc;
    }
    
    // EOF
    if (!src[srcp]) break;
    
    // Delay.
    if (!(src[srcp]&0x80)) {
      delay+=src[srcp];
      srcp++;
      continue;
    }
    
    // We have a real event, so flush any delay first.
    dstlen+=delay;
    dstc+=m2m_flush_delay(dst+dstc,delay);
    delay=0;
    
    // Copy the real event verbatim, 2 or 3 bytes, knowable from the top 4 bits of the leading byte.
    switch (src[srcp]&0xf0) {
      case 0x80:
      case 0x90:
      case 0xa0:
      case 0xb0:
      case 0xe0:
      case 0xf0: {
          if (srcp>srcc-3) return -1;
          memcpy(dst+dstc,src+srcp,3);
          dstc+=3;
          srcp+=3;
        } break;
      case 0xc0:
      case 0xd0: {
          if (srcp>srcc-2) return -1;
          memcpy(dst+dstc,src+srcp,2);
          dstc+=2;
          srcp+=2;
        } break;
      default: return -1;
    }
  }
  
  // Final delay must not push us beyond the next 4-qnote boundary. (an arbitrary rule that might mess up non-4/4 songs).
  int measurelen=ctx->division*4;
  int measurep1=dstlen/measurelen;
  int measurep2=(dstlen+delay)/measurelen;
  if (measurep2>measurep1) {
    delay=measurelen-dstlen%measurelen;
  }
  dstc+=m2m_flush_delay(dst+dstc,delay);
  *(void**)dstpp=dst;
  return dstc;
}

/* After generating the minisyni song sequentially, any further wrap-up needed.
 */
 
static int m2m_finish(struct m2m_context *ctx) {
  
  // Simulate release of any pending notes.
  struct m2m_note *note=ctx->notev;
  int i=ctx->notec;
  for (;i-->0;note++) {
    struct m2m_event event={
      .opcode=0x80,
      .chid=note->chid,
      .a=note->noteid,
      .b=0x40,
    };
    if (m2m_process_note_off(ctx,0,&event)<0) return -1;
  }
  
  // Emit an EOF event just to keep it clean.
  if (encode_raw(&ctx->songcvt->bin,"\0",1)<0) return -1;
  
  void *condensed=0;
  int condensedc=m2m_condense_delays(&condensed,ctx->songcvt->bin.v,ctx->songcvt->bin.c,ctx);
  if (condensedc<0) {
    fprintf(stderr,"%s:!!! Failed to condense delays. Proceeding anyway.\n",ctx->songcvt->srcpath);
  } else {
    free(ctx->songcvt->bin.v);
    ctx->songcvt->bin.v=condensed;
    ctx->songcvt->bin.c=condensedc;
    ctx->songcvt->bin.a=condensedc;
    condensed=0;
  }
  if (condensed) free(condensed);
  
  return 0;
}

/* Produce any initial output. After dechunking but before recording.
 */
 
static int m2m_produce_lead(struct m2m_context *ctx) {

  // Any rule that matches on only MTrk and emits both chid and pid, emit an initial program change.
  struct songcvt_rule *rule=ctx->songcvt->rulev;
  int i=ctx->songcvt->rulec;
  for (;i-->0;rule++) {
    if (rule->match_MTrk<0) continue;
    if (rule->match_MTrk>=ctx->trackc) continue;
    if (rule->match_chid>=0) continue;
    if (rule->match_pid>=0) continue;
    if (rule->set_chid<0) continue;
    if (rule->set_pid<0) continue;
    uint8_t serial[]={0xc0|rule->set_chid,rule->set_pid};
    if (encode_raw(&ctx->songcvt->bin,serial,sizeof(serial))<0) return -1;
  }

  return 0;
}

/* Receive MThd.
 */
 
static int m2m_read_MThd(struct m2m_context *ctx,const uint8_t *src,int srcc) {
  if (ctx->division) {
    fprintf(stderr,"%s: Multiple MThd\n",ctx->songcvt->srcpath);
    return -2;
  }
  if (srcc<6) {
    fprintf(stderr,"%s: Invalid MThd length %d<6\n",ctx->songcvt->srcpath,srcc);
    return -2;
  }
  
  ctx->format=(src[0]<<8)|src[1];
  ctx->track_count=(src[2]<<8)|src[3];
  ctx->division=(src[4]<<8)|src[5];
  
  if (!ctx->division) {
    fprintf(stderr,"%s: Illegal division 0\n",ctx->songcvt->srcpath);
    return -2;
  }
  if (ctx->division&0x8000) {
    fprintf(stderr,"%s: SMPTE timing not supported\n",ctx->songcvt->srcpath);
    return -2;
  }
  if (ctx->format!=1) {
    fprintf(stderr,"%s: Unsupported MIDI file format %d (expected 1), but we'll try anyway.\n",ctx->songcvt->srcpath,ctx->format);
  }
  
  if (ctx->songcvt->debug) {
    fprintf(stderr,"MThd format=%d trackc=%d division=%d\n",ctx->format,ctx->track_count,ctx->division);
  }
  
  return 0;
}

/* In debug mode, pre-read each track and dump some stats.
 */
 
static void m2m_debug_track(const uint8_t *src,int srcc) {
  #define MALFORMED { \
    fprintf(stderr,"  !!! MTrk malformed !!!\n"); \
    return; \
  }
  int chidv[16]={0};
  int notev[256]={0};
  int pidv[256]={0};
  int srcp=0,err,duration=0,i;
  uint8_t rstat=0;
  while (srcp<srcc) {
  
    int delay;
    if ((err=m2m_read_vlq(&delay,src+srcp,srcc-srcp))<1) MALFORMED
    duration+=delay;
    srcp+=err;
    struct m2m_event event;
    if ((err=m2m_read_event(&event,src+srcp,srcc-srcp,&rstat))<0) MALFORMED
    srcp+=err;
    
    if (event.chid<0x10) chidv[event.chid]++;
    switch (event.opcode) {
      case 0x90: notev[event.a]++; break;
      case 0xc0: pidv[event.a]++; break;
    }
  }
  
  fprintf(stderr,"  duration: %d ticks\n",duration);
  
  fprintf(stderr,"  channels:");
  for (i=0;i<16;i++) if (chidv[i]) fprintf(stderr," %d",i);
  fprintf(stderr,"\n");
  
  fprintf(stderr,"  programs:");
  for (i=0;i<256;i++) if (pidv[i]) fprintf(stderr," %d",i);
  fprintf(stderr,"\n");
  
  fprintf(stderr,"  notes:");
  int lonote=-1,hinote=-1;
  for (i=0;i<256;i++) if (notev[i]) { lonote=i; break; }
  if (lonote>=0) {
    for (i=256;i-->0;) if (notev[i]) { hinote=i; break; }
    fprintf(stderr," 0x%02x..0x%02x\n",lonote,hinote);
  } else {
    fprintf(stderr," none\n");
  }
  
  #undef MALFORMED
}

/* Receive MTrk.
 * Just record it in the context, we'll read it later.
 */
 
static int m2m_read_MTrk(struct m2m_context *ctx,const uint8_t *src,int srcc) {
  
  if (ctx->trackc>=ctx->tracka) {
    int na=ctx->tracka+8;
    if (na>INT_MAX/sizeof(struct m2m_track)) return -1;
    void *nv=realloc(ctx->trackv,sizeof(struct m2m_track)*na);
    if (!nv) return -1;
    ctx->trackv=nv;
    ctx->tracka=na;
  }
  
  struct m2m_track *track=ctx->trackv+ctx->trackc++;
  memset(track,0,sizeof(struct m2m_track));
  track->v=src;
  track->c=srcc;
  track->delay=-1;
  
  if (ctx->songcvt->debug) {
    fprintf(stderr,"MTrk %d c=%d\n",ctx->trackc-1,srcc);
    m2m_debug_track(src,srcc);
  }
  
  return 0;
}

/* Read the input file, parse the MThd chunk and identify MTrk chunks.
 * Fails if MThd invalid or no MTrk.
 */
 
static int m2m_read_chunks(struct m2m_context *ctx) {

  ctx->division=0; // Signal that MThd was read, it can't be zero ultimately.
  ctx->trackc=0;
  
  const uint8_t *src=ctx->songcvt->src;
  int srcc=ctx->songcvt->srcc;
  int srcp=0,err;
  while (srcp<=srcc-8) {
  
    const char *chunkid=(char*)src+srcp;
    int chunklen=(src[srcp+4]<<24)|(src[srcp+5]<<16)|(src[srcp+6]<<8)|src[srcp+7];
    srcp+=8;
    if ((chunklen<0)||(srcp>srcc-chunklen)) {
      fprintf(stderr,"%s: Invalid chunk length %d around %d/%d\n",ctx->songcvt->srcpath,chunklen,srcp-8,srcc);
      return -2;
    }
    const void *chunk=src+srcp;
    srcp+=chunklen;
    
    if (!memcmp(chunkid,"MThd",4)) {
      if ((err=m2m_read_MThd(ctx,chunk,chunklen))<0) {
        if (err!=-2) fprintf(stderr,"%s: Error processing MThd\n",ctx->songcvt->srcpath);
        return -2;
      }
    } else if (!memcmp(chunkid,"MTrk",4)) {
      if ((err=m2m_read_MTrk(ctx,chunk,chunklen))<0) {
        if (err!=-2) fprintf(stderr,"%s: Error processing MTrk\n",ctx->songcvt->srcpath);
        return -2;
      }
    } else {
      // Unknown chunk, whatever.
    }
  }
  
  if (!ctx->division) {
    fprintf(stderr,"%s: No MThd chunk.\n",ctx->songcvt->srcpath);
    return -2;
  }
  if (!ctx->trackc) {
    fprintf(stderr,"%s: No MTrk chunks.\n",ctx->songcvt->srcpath);
    return -2;
  }
  return 0;
}

/* Convert song in memory.
 * Output goes into (songcvt->bin).
 */
 
int songcvt_minisyni_from_midi(struct songcvt *songcvt) {
  songcvt->bin.c=0;
  struct m2m_context ctx={
    .songcvt=songcvt,
  };
  
  int err;
  if (
    ((err=m2m_read_chunks(&ctx))<0)||
    ((err=m2m_produce_lead(&ctx))<0)||
    ((err=m2m_play_and_record(&ctx))<0)||
    ((err=m2m_finish(&ctx))<0)
  ) {
    m2m_context_cleanup(&ctx);
    return err;
  }
  
  m2m_context_cleanup(&ctx);
  return 0;
}
