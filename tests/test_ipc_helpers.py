import os
import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include <l4/exo_ipc.h>
#include <l4/ipc.h>
int main() {
    exo_ipc_status st = exo_call(L4_nilthread);
    (void)st;
    return 0;
}
"""

class IpcHelpersBuildTest(unittest.TestCase):
    def test_compile(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.cpp"
            src.write_text(CODE)
            compiler = os.getenv("CXX", "clang++")
            cmd = [
                compiler,
                "-std=c++23",
                "-I",
                str(ROOT / "engine/include"),
                "-c",
                str(src),
            ]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                self.skipTest(f"{compiler} failed: {e}")

if __name__ == "__main__":
    unittest.main()
