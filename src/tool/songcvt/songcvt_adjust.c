#include "songcvt.h"

/* Add a blank rule.
 */
 
static struct songcvt_rule *songcvt_add_rule(struct songcvt *songcvt) {
  if (songcvt->rulec>=songcvt->rulea) {
    int na=songcvt->rulea+8;
    if (na>INT_MAX/sizeof(struct songcvt_rule)) return 0;
    void *nv=realloc(songcvt->rulev,sizeof(struct songcvt_rule)*na);
    if (!nv) return 0;
    songcvt->rulev=nv;
    songcvt->rulea=na;
  }
  struct songcvt_rule *rule=songcvt->rulev+songcvt->rulec++;
  
  rule->match_MTrk=-1;
  rule->match_chid=-1;
  rule->match_pid=-1;
  rule->set_chid=-1;
  rule->set_pid=-1;
  
  return rule;
}

/* Criterion clause in rule.
 */
 
static int songcvt_adjust_rule_criterion(struct songcvt *songcvt,struct songcvt_rule *rule,const char *src,int srcc,const char *path,int lineno) {
  int kc=0;
  const char *k=src;
  while ((kc<srcc)&&(k[kc]!='=')) kc++;
  if (kc>=srcc) {
    fprintf(stderr,"%s:%d: Expected '=' in rule criterion '%.*s'\n",path,lineno,srcc,src);
    return -2;
  }
  int v;
  if (int_eval(&v,src+kc+1,srcc-kc-1)<1) {
    fprintf(stderr,"%s:%d: Failed to evaluate rule criterion value in '%.*s'\n",path,lineno,srcc,src);
    return -2;
  }

  if ((kc==4)&&!memcmp(k,"MTrk",4)) rule->match_MTrk=v;
  else if ((kc==4)&&!memcmp(k,"chid",4)) rule->match_chid=v;
  else if ((kc==3)&&!memcmp(k,"pid",3)) rule->match_pid=v;

  else {
    fprintf(stderr,"%s:%d: Unknown criterion field '%.*s' (MTrk,chid,pid)\n",path,lineno,kc,k);
    return -2;
  }
  return 0;
}

/* Action clause in rule.
 */
 
static int songcvt_adjust_rule_action(struct songcvt *songcvt,struct songcvt_rule *rule,const char *src,int srcc,const char *path,int lineno) {
  int kc=0;
  const char *k=src;
  while ((kc<srcc)&&(k[kc]!='=')) kc++;
  if (kc>=srcc) {
    fprintf(stderr,"%s:%d: Expected '=' in rule action '%.*s'\n",path,lineno,srcc,src);
    return -2;
  }
  int v;
  if (int_eval(&v,src+kc+1,srcc-kc-1)<1) {
    fprintf(stderr,"%s:%d: Failed to evaluate rule action value in '%.*s'\n",path,lineno,srcc,src);
    return -2;
  }

  if ((kc==4)&&!memcmp(k,"chid",4)) rule->set_chid=v;
  else if ((kc==3)&&!memcmp(k,"pid",3)) rule->set_pid=v;

  else {
    fprintf(stderr,"%s:%d: Unknown action field '%.*s' (chid,pid)\n",path,lineno,kc,k);
    return -2;
  }
  return 0;
}

/* Read and record a "CRITERIA : ACTIONS" line.
 */
 
static int songcvt_adjust_criteria_actions(struct songcvt *songcvt,const char *src,int srcc,const char *path,int lineno) {

  struct songcvt_rule *rule=songcvt_add_rule(songcvt);
  if (!rule) return -1;
  
  int stage=0; // 0=criteria, 1=actions
  int srcp=0;
  while (srcp<srcc) {
    if ((unsigned char)src[srcp]<=0x20) { srcp++; continue; }
    const char *word=src+srcp;
    int wordc=0,err;
    while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; wordc++; }
    
    if ((stage==0)&&(wordc==1)&&(word[0]==':')) {
      stage=1;
      continue;
    }
    
    switch (stage) {
      case 0: if ((err=songcvt_adjust_rule_criterion(songcvt,rule,word,wordc,path,lineno))<0) return err; break;
      case 1: if ((err=songcvt_adjust_rule_action(songcvt,rule,word,wordc,path,lineno))<0) return err; break;
      default: return -1;
    }
  }

  return 0;
}

/* Receive one adjustment command.
 */
 
static int songcvt_adjust_line(struct songcvt *songcvt,const char *src,int srcc,const char *path,int lineno) {

  // Read the first space=delimited word.
  int srcp=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  const char *kw=src+srcp;
  int kwc=0;
  while ((srcp<srcc)&&((unsigned char)src[srcp]>0x20)) { srcp++; kwc++; }
  while ((srcp<srcc)&&((unsigned char)src[srcp]<=0x20)) srcp++;
  
  // "debug", no parameters.
  if ((kwc==5)&&!memcmp(kw,"debug",5)) {
    songcvt->debug=1;
    return 0;
  }
  
  //TODO tempo adjustment?
  //TODO division?
  //TODO directives to strip events eg aftertouch?
  
  // If keyword has '=', it's a "CRITERIA : ACTIONS" line.
  int i=kwc; while (i-->0) if (kw[i]=='=') {
    return songcvt_adjust_criteria_actions(songcvt,src,srcc,path,lineno);
  }
  
  return -1;
}

/* Read adjustments in memory.
 */
 
static int songcvt_adjust(struct songcvt *songcvt,const char *src,int srcc,const char *path) {
  int srcp=0,lineno=0;
  while (srcp<srcc) {
  
    lineno++;
    const char *line=src+srcp;
    int linec=0,comment=0;
    while (srcp<srcc) {
      char ch=src[srcp++];
      if (ch==0x0a) break;
      if (ch=='#') comment=1;
      else if (!comment) linec++;
    }
    while (linec&&((unsigned char)line[linec-1]<=0x20)) linec--;
    while (linec&&((unsigned char)line[0]<=0x20)) { line++; linec--; }
    if (!linec) continue;
    
    int err=songcvt_adjust_line(songcvt,line,linec,path,lineno);
    if (err<0) {
      if (err!=-2) fprintf(stderr,"%s:%d: Unspecified error processing adjustment line:\n  %.*s\n",path,lineno,linec,line);
      return -2;
    }
  }
  return 0;
}

/* Read adjustments, main entry point.
 */
 
int songcvt_read_adjust(struct songcvt *songcvt) {

  if (!songcvt->srcpath) return 0;
  int srcpathc=0;
  while (songcvt->srcpath[srcpathc]) srcpathc++;
  if ((srcpathc>=4)&&!memcmp(songcvt->srcpath+srcpathc-4,".mid",4)) srcpathc-=4;
  char path[1024];
  int pathc=snprintf(path,sizeof(path),"%.*s.adjust",srcpathc,songcvt->srcpath);
  if ((pathc<1)||(pathc>=sizeof(path))) return -1;
  
  char *src=0;
  int srcc=file_read(&src,path);
  if (srcc<0) return 0;
  int err=songcvt_adjust(songcvt,src,srcc,path);
  free(src);
  return err;
}
