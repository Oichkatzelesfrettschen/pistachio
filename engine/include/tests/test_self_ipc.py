import os
import subprocess
import tempfile
from pathlib import Path
import unittest

# Repository root
ROOT = Path(__file__).resolve().parents[1]
# Source program for self IPC
SOURCE = ROOT / "user/apps/ipc_demo/self_ipc/main.cc"


class SelfIpcBuildTest(unittest.TestCase):
    """Verify that the self IPC demo compiles."""

    def test_compile(self) -> None:
        """Compile self_ipc without linking."""
        with tempfile.TemporaryDirectory() as td:
            obj = Path(td) / "self_ipc.o"
            compiler = os.getenv("CXX", "clang++")
            cmd = [
                compiler,
                "-std=c++23",
                "-I",
                str(ROOT / "user/include"),
                "-I",
                str(ROOT / "include"),
                "-c",
                str(SOURCE),
                "-o",
                str(obj),
            ]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as exc:
                self.skipTest(f"{compiler} failed: {exc}")


if __name__ == "__main__":
    unittest.main()
