#include <algorithm>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <wait_graph.h>

/**\brief Simple directed wait-for graph for detecting IPC deadlocks. */
class WaitGraph {
public:
  /** Obtain the singleton instance. */
  static auto &instance() {
    static WaitGraph g;
    return g;
  }

  /**
   * Attempt to add an edge from @p from to @p to.
   *
   * @return true if the edge was added, false if it would create a cycle.
   */
  [[nodiscard]] bool add_edge(tcb_t *from, tcb_t *to) {
    if (would_cause_cycle(from, to)) {
      return false;
    }
    edges[from].push_back(to);
    return true;
  }

  /** Remove the edge from @p from to @p to if present. */
  void remove_edge(tcb_t *from, tcb_t *to) {
    auto it = edges.find(from);
    if (it == edges.end()) {
      return;
    }
    auto &vec = it->second;
    vec.erase(std::remove(vec.begin(), vec.end(), to), vec.end());
    if (vec.empty()) {
      edges.erase(it);
    }
  }

private:
  using Adj = std::unordered_map<tcb_t *, std::vector<tcb_t *>>;
  Adj edges;

  [[nodiscard]] bool would_cause_cycle(tcb_t *from, tcb_t *to) const {
    std::vector<tcb_t *> stack{to};
    std::unordered_set<tcb_t *> visited;
    while (!stack.empty()) {
      auto *cur = stack.back();
      stack.pop_back();
      if (cur == from) {
        return true;
      }
      if (!visited.insert(cur).second) {
        continue;
      }
      auto it = edges.find(cur);
      if (it != edges.end()) {
        std::copy(it->second.begin(), it->second.end(),
                  std::back_inserter(stack));
      }
    }
    return false;
  }
};

extern "C" {

int wait_graph_add_edge(tcb_t *from, tcb_t *to) {
  return WaitGraph::instance().add_edge(from, to) ? 1 : 0;
}

void wait_graph_remove_edge(tcb_t *from, tcb_t *to) {
  WaitGraph::instance().remove_edge(from, to);
}

} // extern "C"
