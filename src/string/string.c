#include <string.h>
#include <string/string.h>

void
string_new(struct string_t *string)
{
  string->data = NULL;
  string->len = 0;
}

void
string_append(struct string_t *string, char *segment, size_t len)
{
  if (string->data) {
    string->data = realloc(string->data, (string->len + len + 1) * sizeof(char));
  } else {
    string->data = calloc(len+1, sizeof(char));
  }
  memcpy(&(string->data)[string->len], segment, len);
  string->len += len;
  string->data[string->len] = '\x0';
}
