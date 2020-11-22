#include "analysis.h"

static struct vtable **tables = NULL;
static void **libraries_handles = NULL;
static size_t num_vtables = 0;

void
analysis_run(struct candle *cnds, size_t indx)
{
  for (size_t i = 0; i < num_vtables; ++i)
  {
    tables[i]->run(cnds, indx);
  }
}

static void
_analysis_load_so(char path[PATH_MAX])
{
  void *handle = dlopen(path, RTLD_LAZY);
  if (!handle)
  {
    pprint_error("unable to load %s", path);
    pprint_error("dlopen error: %s", dlerror());
    return;
  }

  struct vtable *libclass = dlsym(handle, "exports");
  if (!libclass)
  {
    pprint_error("%s", "this library does not export a vtable (aborting)");
    exit(1);
  }

  num_vtables += 1;
  tables = realloc(tables, sizeof(struct vtable *) * num_vtables);
  tables[num_vtables - 1] = libclass;

  libraries_handles = realloc(libraries_handles, sizeof(void *) * num_vtables);
  libraries_handles[num_vtables - 1] = handle;

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

  /* loop recursivly through the directory of the libs */
  while ((dp = readdir(dir)) != NULL)
  {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
    {

      /* check if name ends in .so */
      size_t name_len = strlen(dp->d_name);
      if (dp->d_name[name_len - 1] == 'o' && dp->d_name[name_len - 2] == 's' &&
          dp->d_name[name_len - 3] == '.')
      {

        /* create the full path of the .so to load */
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
}

void
analysis_clear(void)
{
  for (size_t i = 0; i < num_vtables; ++i)
  {
    dlclose(libraries_handles[i]);
  }
  free(tables);
  free(libraries_handles);
}
