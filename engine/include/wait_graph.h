#ifndef WAIT_GRAPH_H
#define WAIT_GRAPH_H

#include <cstddef>

struct tcb_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register a wait edge from *from* to *to*.
 *
 * @param from waiting thread
 * @param to resource owning thread
 *
 * @return 1 if the edge was added, 0 if it would form a cycle
 */
int wait_graph_add_edge(tcb_t *from, tcb_t *to);

/**
 * Remove a previously registered edge from *from* to *to*.
 */
void wait_graph_remove_edge(tcb_t *from, tcb_t *to);

#ifdef __cplusplus
}
#endif

#endif // WAIT_GRAPH_H
