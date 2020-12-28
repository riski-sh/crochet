#include "algae.h"

static char *
_read_file(FILE *file)
{
}

struct algae_program *
algae_compile(char *file)
{

  /* make sure file is not NULL */
  if (!file)
  {
    pprint_error("file location is %s (aborting)", file);
    exit(1);
  }

  /* open the file in read only mode */
  FILE *fp = fopen(file, "r");
  if (!fp)
  {
    pprint_error("unable to load file %s (aborting)", file);
    exit(1);
  }

  fclose(fp);
  fp = NULL;
}
