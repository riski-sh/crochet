#include "tsqueue.h"

void
tsqueue_add(struct tsqueue *queue, struct candles *cnds, size_t indx,
            analysis_func run)
{
  /* create an element first */
  struct tsqueue_element *element = NULL;
  element = malloc(sizeof(struct tsqueue_element) * 1);

  if (!element)
  {
    pprint_error("unable to allocate %lu bytes for analysis (aborting)",
                 sizeof(struct tsqueue_element));
    exit(1);
  }

  element->cnds = cnds;
  element->indx = indx;
  element->run = run;

  pthread_mutex_lock(&(queue->locked));
  element->next = queue->head;
  queue->head = element;
  pthread_mutex_unlock(&(queue->locked));
}

struct tsqueue_element *
tsqueue_pop(struct tsqueue *queue)
{
  struct tsqueue_element *element = NULL;
  pthread_mutex_lock(&(queue->locked));
  element = queue->head;
  queue->head = element->next;
  pthread_mutex_unlock(&(queue->locked));
  return element;
}
