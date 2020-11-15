#ifndef SECURITY_TSQUEUE_H
#define SECURITY_TSQUEUE_H

#include <api.h>
#include <pprint/pprint.h>
#include <pthread.h>

/* represents an element in the queue */
struct tsqueue_element {
  struct candles *cnds;
  size_t indx;
  analysis_func run;
  struct tsqueue_element *next;
};

/* represents a queue */
struct tsqueue {
  pthread_mutex_t locked;
  struct tsqueue_element *head;
};

/*
 * Adds data to the queue. This adds data in a LIFO maner since the latest
 * analysis is always more important than analysis that may have happened
 * in the past.
 *
 * @param queue the queue to append to.
 * @param cnds the list of candles that this analysis will be applied to
 * @param indx the last candle that has finished. When writing analysis
 * NEVER reference cnds[indx].
 * @param run the analysis to run
 */
void tsqueue_add(struct tsqueue *queue, struct candles *cnds, size_t indx,
    analysis_func run);

/*
 * Pops the first element out of the queue
 *
 * @param queue the queue to pop
 * @return the element in the queue. This element must be freed by the popper
 */
struct tsqueue_element *tsqueue_pop(struct tsqueue *queue);

#endif
