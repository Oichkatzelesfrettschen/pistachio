import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include <l4/exo_ipc.h>
#include <l4/ipc.h>
#include <l4/memory.h>

int main() {
    exo_ipc_status st = exo_call(L4_nilthread);
    (void)st;
    L4_ThreadId_t ms = memory_server();
    (void)memory_alloc(ms, 4096);
    return 0;
}
"""

class IpcHelpersBuildTest(unittest.TestCase):
    def test_compile(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.cpp"
            src.write_text(CODE)
            cmd = [
                "g++",
                "-std=c++23",
                "-I",
                str(ROOT / "user/include"),
                "-c",
                str(src),
            ]
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)

if __name__ == "__main__":
    unittest.main()
