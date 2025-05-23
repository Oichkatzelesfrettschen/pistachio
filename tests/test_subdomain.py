import threading
import unittest

class PrioQueue:
    def __init__(self):
        self.refcnt = 0
        self.domain_tcb = object()
        self.cpu_head = self
        self.cpu_link = self
        self.destroyed = False

class SchedState:
    def __init__(self, is_domain=False):
        self.prio_queue = None
        self.is_domain = is_domain
        self.domain_queue = PrioQueue()

    def get_domain_prio_queue(self):
        return self.domain_queue

    def set_prio_queue(self, q):
        old = self.prio_queue
        if q:
            q.refcnt += 1
        self.prio_queue = q
        if old:
            old.refcnt -= 1
            if old.refcnt == 0:
                old.destroyed = True
        if self.is_domain and self.prio_queue:
            pass  # depth update stub

class DestroyTest(unittest.TestCase):
    def test_destroy_domain(self):
        pq_parent = PrioQueue()
        pq_sub = PrioQueue()
        th_state = SchedState()
        th_state.set_prio_queue(pq_sub)
        self.assertEqual(pq_sub.refcnt,1)
        th_state.set_prio_queue(pq_parent)
        self.assertEqual(pq_sub.refcnt,0)
        self.assertTrue(pq_sub.destroyed)

    def test_concurrent_destroy(self):
        pq_parent = PrioQueue()
        pq_sub = PrioQueue()
        states = [SchedState() for _ in range(2)]
        for s in states:
            s.set_prio_queue(pq_sub)
        self.assertEqual(pq_sub.refcnt,2)
        def migrate(s):
            s.set_prio_queue(pq_parent)
        threads=[threading.Thread(target=migrate,args=(s,)) for s in states]
        for t in threads: t.start()
        for t in threads: t.join()
        self.assertEqual(pq_sub.refcnt,0)
        self.assertTrue(pq_sub.destroyed)

    def test_clear_queue(self):
        pq = PrioQueue()
        state = SchedState()
        state.set_prio_queue(pq)
        self.assertEqual(pq.refcnt, 1)

        state.set_prio_queue(None)
        self.assertIsNone(state.prio_queue)
        self.assertEqual(pq.refcnt, 0)
        self.assertTrue(pq.destroyed)

    def test_set_queue_none_when_none(self):
        pq = PrioQueue()
        state = SchedState()
        state.set_prio_queue(pq)
        self.assertEqual(pq.refcnt, 1)

        state.set_prio_queue(None)
        self.assertIsNone(state.prio_queue)
        self.assertEqual(pq.refcnt, 0)
        self.assertTrue(pq.destroyed)

        # setting None again should be a no-op
        state.set_prio_queue(None)
        self.assertIsNone(state.prio_queue)
        self.assertEqual(pq.refcnt, 0)
        self.assertTrue(pq.destroyed)

    def test_many_concurrent_migrations(self):
        pq_parent = PrioQueue()
        pq_sub = PrioQueue()
        states = [SchedState() for _ in range(10)]
        for st in states:
            st.set_prio_queue(pq_sub)
        self.assertEqual(pq_sub.refcnt, 10)

        def migrate(st):
            st.set_prio_queue(pq_parent)

        threads = [threading.Thread(target=migrate, args=(st,)) for st in states]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(pq_sub.refcnt, 0)
        self.assertTrue(pq_sub.destroyed)
        for st in states:
            self.assertIs(st.prio_queue, pq_parent)

    def test_concurrent_migrations_more_than_cpus(self):
        import os
        pq_parent = PrioQueue()
        pq_sub = PrioQueue()
        n_threads = (os.cpu_count() or 1) + 2
        states = [SchedState() for _ in range(n_threads)]

        for st in states:
            st.set_prio_queue(pq_sub)
        self.assertEqual(pq_sub.refcnt, n_threads)

        def migrate(st):
            st.set_prio_queue(pq_parent)

        threads = [threading.Thread(target=migrate, args=(st,)) for st in states]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(pq_sub.refcnt, 0)
        self.assertTrue(pq_sub.destroyed)
        for st in states:
            self.assertIs(st.prio_queue, pq_parent)
        self.assertEqual(pq_parent.refcnt, n_threads)
        self.assertFalse(pq_parent.destroyed)

    def test_race_domain_destroy_and_migrate(self):
        """Domain queue destroyed while both domain and thread migrate."""
        pq_old = PrioQueue()
        pq_new = PrioQueue()

        domain_state = SchedState(is_domain=True)
        domain_state.set_prio_queue(pq_old)

        th_state = SchedState()
        th_state.set_prio_queue(pq_old)

        self.assertEqual(pq_old.refcnt, 2)
        self.assertFalse(pq_old.destroyed)

        def migrate_domain():
            domain_state.set_prio_queue(pq_new)

        def migrate_thread():
            th_state.set_prio_queue(pq_new)

        t1 = threading.Thread(target=migrate_domain)
        t2 = threading.Thread(target=migrate_thread)
        t1.start(); t2.start()
        t1.join(); t2.join()

        self.assertEqual(pq_old.refcnt, 0)
        self.assertTrue(pq_old.destroyed)
        self.assertEqual(pq_new.refcnt, 2)
        self.assertFalse(pq_new.destroyed)
        self.assertIs(domain_state.prio_queue, pq_new)
        self.assertIs(th_state.prio_queue, pq_new)

    def test_race_domain_destroy_with_many_threads(self):
        """Multiple threads and the domain migrate simultaneously."""
        pq_old = PrioQueue()
        pq_new = PrioQueue()

        domain_state = SchedState(is_domain=True)
        domain_state.set_prio_queue(pq_old)

        states = [SchedState() for _ in range(5)]
        for st in states:
            st.set_prio_queue(pq_old)

        self.assertEqual(pq_old.refcnt, 1 + len(states))
        self.assertFalse(pq_old.destroyed)

        def migrate(st):
            st.set_prio_queue(pq_new)

        threads = [threading.Thread(target=migrate, args=(st,)) for st in states]
        threads.append(threading.Thread(target=migrate, args=(domain_state,)))

        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(pq_old.refcnt, 0)
        self.assertTrue(pq_old.destroyed)
        self.assertEqual(pq_new.refcnt, 1 + len(states))
        self.assertFalse(pq_new.destroyed)
        self.assertIs(domain_state.prio_queue, pq_new)
        for st in states:
            self.assertIs(st.prio_queue, pq_new)

if __name__ == '__main__':
    unittest.main()
