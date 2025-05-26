import threading
import unittest


class SpinLock:
    def __init__(self):
        self._lock = threading.Lock()

    def acquire(self) -> None:
        while not self._lock.acquire(blocking=False):
            pass

    def release(self) -> None:
        self._lock.release()


class SpinLockThreadTest(unittest.TestCase):
    def test_spinlock_stress_threads(self) -> None:
        lock = SpinLock()
        counter = 0
        loops = 10000
        n_threads = 10

        def worker() -> None:
            nonlocal counter
            for _ in range(loops):
                lock.acquire()
                counter += 1
                lock.release()

        threads = [threading.Thread(target=worker) for _ in range(n_threads)]
        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(counter, loops * n_threads)


if __name__ == "__main__":
    unittest.main()
