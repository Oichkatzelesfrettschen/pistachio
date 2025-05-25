import ctypes
import os
import signal
import unittest

libc = ctypes.CDLL(None)

PTRACE_TRACEME = 0
PTRACE_CONT = 7


def _child() -> None:
    libc.ptrace(PTRACE_TRACEME, 0, None, None)
    os.kill(os.getpid(), signal.SIGSTOP)
    os._exit(0)


class PtraceConcurrentTest(unittest.TestCase):
    def test_concurrent_ptrace(self) -> None:
        n_procs = 3
        pids = []
        for _ in range(n_procs):
            pid = os.fork()
            if pid == 0:
                _child()
            else:
                pids.append(pid)

        for pid in pids:
            waited, _ = os.waitpid(pid, os.WUNTRACED)
            self.assertEqual(waited, pid)
            libc.ptrace(PTRACE_CONT, pid, None, None)

        for pid in pids:
            os.waitpid(pid, 0)


if __name__ == "__main__":
    unittest.main()
