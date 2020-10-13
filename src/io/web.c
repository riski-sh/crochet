#include "web.h"

static int fd1[2];

void
web_read_packet()
{
  int msgLen = 0;
  int ret = read(fd1[0], &msgLen, sizeof(int));

  if (ret != sizeof(int)) {
    pprint_info("%s", "error: unable to read packet length");
    exit(1);
  }
}

void
web_init()
{
  int ret = pipe(fd1);

  if (ret == -1) {
    pprint_error("%s", "unable to create server.pipe for pipe communication with "
                 "golang web server");
    pprint_error("%s", strerror(errno));
    exit(1);
  }

  pprint_info("fd1[0] = %d", fd1[0]);
  pprint_info("fd1[1] = %d", fd1[1]);

  FILE *pidfile = fopen("crochet.pid", "w");
  char pidDataFmt[] = "{\"pid\": %d, \"read\": %d, \"write\": %d}";

  printf("%s\n", pidDataFmt);
  int fmtlen = snprintf(NULL, 0, pidDataFmt, getpid(), fd1[0], fd1[1]);
  char *fdata = malloc(fmtlen + 1);
  snprintf(fdata, fmtlen+1, pidDataFmt, getpid(), fd1[1], fd1[0]);

  fwrite(fdata, fmtlen, 1, pidfile);
  fclose(pidfile);

  pprint_info("%s", "wrote %s to crochet.pid");
  web_read_packet();
}
