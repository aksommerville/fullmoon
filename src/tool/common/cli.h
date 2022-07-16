/* cli.h
 * High-level helper for tools.
 */
 
#ifndef CLI_H
#define CLI_H

struct cli {
  const char *exename;
};

struct cli_config {
  // Return -2 if you log the error (then the wrapper won't).
  int (*arg_positional)(struct cli *cli,const char *src,int srcc);
  int (*arg_option)(struct cli *cli,const char *k,int kc,const char *v,int vc);
  void (*help_extra)(struct cli *cli);
};

void cli_cleanup(struct cli *cli);

/* We log all errors.
 */
int cli_init(struct cli *cli,const struct cli_config *config,int argc,char **argv);

#endif
