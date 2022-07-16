#include "cli.h"
#include <string.h>
#include <stdio.h>

/* Cleanup.
 */

void cli_cleanup(struct cli *cli) {
}

/* --help
 */
 
static void cli_print_help(struct cli *cli) {
  fprintf(stderr,"\nUsage: %s [OPTIONS]\n",cli->exename);
  fprintf(stderr,
    "\nOPTIONS:\n"
    "  --help             Print this message.\n"
  );
}

/* Init.
 */

int cli_init(struct cli *cli,const struct cli_config *config,int argc,char **argv) {
  int argp=0;
  
  if (argp<argc) {
    if (argv[argp]&&argv[argp][0]) cli->exename=argv[argp];
    else if (!cli->exename) cli->exename="(unknown)";
    argp++;
  } else if (!cli->exename) {
    cli->exename="(unknown)";
  }
  
  while (argp<argc) {
    const char *arg=argv[argp++];
    
    // Null or empty is always illegal.
    if (!arg||!arg[0]) goto _illegal_;
    
    // No dash, or just a single or double dash: positional.
    if (
      (arg[0]!='-')||
      !arg[1]||
      ((arg[1]=='-')&&!arg[2])
    ) {
      if (!config||!config->arg_positional) goto _illegal_;
      int err=config->arg_positional(cli,arg,strlen(arg));
      if (err>=0) continue;
      if (err==-2) return -1; // logged by client
      goto _illegal_;
    }
    
    // "--help" is mandatory; we implement it.
    if (!strcmp(arg,"--help")) {
      cli_print_help(cli);
      if (config&&config->help_extra) config->help_extra(cli);
      return -1;
    }
    
    // Single dash: short option.
    if (arg[1]!='-') {
      if (!config||!config->arg_option) goto _illegal_;
      const char *v=0;
      if (arg[2]) v=arg+2;
      else if ((argp<argc)&&argv[argp]&&(argv[argp][0]!='-')) v=argv[argp++];
      int vc=0; if (v) { while (v[vc]) vc++; }
      int err=config->arg_option(cli,arg+1,1,v,vc);
      if (err>=0) continue;
      if (err==-2) return -1; // logged by client
      goto _illegal_;
    }
    
    // Double dash: long option.
    if (!config||!config->arg_option) goto _illegal_;
    const char *k=arg+2;
    int kc=0; while (k[kc]&&(k[kc]!='=')) kc++;
    const char *v=0;
    if (k[kc]=='=') v=k+kc+1;
    else if ((argp<argc)&&argv[argp]&&(argv[argp][0]!='-')) v=argv[argp++];
    else if ((kc>=3)&&!memcmp(k,"no-",3)) { k+=3; kc-=3; v="0"; }
    else v="1";
    int vc=0; while (v[vc]) vc++;
    int err=config->arg_option(cli,k,kc,v,vc);
    if (err>=0) continue;
    if (err==-2) return -1; // logged by client
    // pass
    
   _illegal_:;
    fprintf(stderr,"%s: Unexpected argument '%s'.\n",cli->exename,arg);
    return -1;
  }
  
  return 0;
}
