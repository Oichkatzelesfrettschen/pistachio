import threading
import time
import unittest


class WaitQueue:
    def __init__(self) -> None:
        self._cond = threading.Condition()
        self._flag = False

    def wait(self, timeout: float) -> bool:
        with self._cond:
            if not self._flag:
                self._cond.wait(timeout)
            if self._flag:
                self._flag = False
                return True
            return False

    def wake(self) -> None:
        with self._cond:
            self._flag = True
            self._cond.notify()


class WaitQueueTimeoutTest(unittest.TestCase):
    def test_multiple_queues_timeout(self) -> None:
        wq1 = WaitQueue()
        wq2 = WaitQueue()
        results: dict[str, bool] = {}

        def worker1() -> None:
            start = time.time()
            results["r1"] = wq1.wait(0.2)
            results["t1"] = time.time() - start

        def worker2() -> None:
            start = time.time()
            results["r2"] = wq2.wait(1.0)
            results["t2"] = time.time() - start

        t1 = threading.Thread(target=worker1)
        t2 = threading.Thread(target=worker2)
        t1.start()
        t2.start()
        time.sleep(0.1)
        wq2.wake()
        t1.join()
        t2.join()

        self.assertFalse(results["r1"], "wq1 should timeout")
        self.assertTrue(results["r2"], "wq2 should be woken")
        self.assertGreaterEqual(results["t1"], 0.2)
        self.assertLess(results["t2"], 1.0)


if __name__ == "__main__":
    unittest.main()
