#include "analysis.h"

static struct vtable **tables = NULL;
static size_t num_vtables = 0;

void
analysis_run(struct candle *cnds, size_t indx)
{
  for (size_t i = 0; i < num_vtables; ++i) {
    tables[i]->run(cnds, indx);
  }
}

static void
_analysis_load_so(char path[PATH_MAX])
{
  void *handle = dlopen(path, RTLD_LAZY);
  if (!handle) {
    pprint_error("unable to load %s", path);
    pprint_error("dlopen error: %s", dlerror());
    return;
  }

  struct vtable *libclass = dlsym(handle, "exports");
  if (!libclass) {
    pprint_error("%s", "this library does not export a vtable (aborting)");
    exit(1);
  }

  num_vtables += 1;
  tables = realloc(tables, sizeof(struct vtable *) * num_vtables);
  tables[num_vtables - 1] = libclass;

  pprint_info("loading %s... OK", path);
}

void
analysis_init(char *base_path)
{

  /* the maximum path defined by the linux filesystem limit */
  char path[PATH_MAX];

  /* directory structure for looping */
  struct dirent *dp;
  DIR *dir = opendir(base_path);

  /* make sure we have opened a directory and not a file */
  if (!dir)
    return;

  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {

      /* check if name ends in .so */
      size_t name_len = strlen(dp->d_name);
      if (dp->d_name[name_len - 1] == 'o' && dp->d_name[name_len - 2] == 's' &&
          dp->d_name[name_len - 3] == '.') {
        strcpy(path, base_path);
        strcat(path, dp->d_name);
        _analysis_load_so(path);
      }

      // Construct new path from our base path
      strcpy(path, base_path);
      strcat(path, "/");
      strcat(path, dp->d_name);

      analysis_init(path);
    }
  }
  closedir(dir);

  /*
  void *handle = dlopen("./libs/marubuzu.so", RTLD_NOW);
  if (!handle) {
    pprint_error("%s", "unable to load ./libs/marubuzu.so (aborting)");
    pprint_error("dlopen error: %s", dlerror());
    exit(1);
  }
  pprint_info("%s", "loading libs/marubuzu.so...");

  struct vtable *libclass = dlsym(handle, "exports");
  if (!libclass) {
    pprint_error("%s", "this library does not export a vtable (aborting)");
    exit(1);
  }

  num_vtables += 1;
  tables = realloc(tables, sizeof(struct vtable *) * num_vtables);
  tables[num_vtables - 1] = libclass;

  pprint_info("%s", "loading libs/marubuzu.so... vtable OK");
  */
}
