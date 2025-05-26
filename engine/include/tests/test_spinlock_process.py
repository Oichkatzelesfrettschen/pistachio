import multiprocessing as mp
import unittest


class SpinLock:
    def __init__(self):
        self._lock = mp.Lock()

    def acquire(self) -> None:
        while not self._lock.acquire(block=False):
            pass

    def release(self) -> None:
        self._lock.release()


def _worker(lock: SpinLock, counter: mp.Value, loops: int) -> None:
    for _ in range(loops):
        lock.acquire()
        counter.value += 1
        lock.release()


class SpinLockProcessTest(unittest.TestCase):
    def test_spinlock_stress_processes(self) -> None:
        lock = SpinLock()
        counter = mp.Value("i", 0)
        loops = 5000
        n_procs = 5

        procs = [
            mp.Process(target=_worker, args=(lock, counter, loops))
            for _ in range(n_procs)
        ]
        for p in procs:
            p.start()
        for p in procs:
            p.join()

        self.assertEqual(counter.value, loops * n_procs)


if __name__ == "__main__":
    unittest.main()
