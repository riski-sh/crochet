#ifndef STATUS_H
#define STATUS_H

typedef enum {
  STATUS_OK,
  STATUS_EXPECTED_NULL,
  STATUS_ALLOC_ERR,
  STATUS_UNKNOWN_ERROR,
  NUM_STATUS_CODES
} status_t;

/*
static const char *((statusstr)[NUM_STATUS_CODES]) = {
  "OK",
  "NOT_NULL",
  "RET_NULL"
};
*/

#endif
