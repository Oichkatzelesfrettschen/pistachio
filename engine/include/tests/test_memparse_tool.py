import os
import subprocess
import tempfile
from pathlib import Path
import unittest

# Root of the repository
ROOT = Path(__file__).resolve().parents[1]
# Source file for the memparse tool
SOURCE = ROOT / "tools/memserver/memparse.cc"


class MemparseToolTest(unittest.TestCase):
    """Build and execute the memparse utility."""

    def test_memparse_decode(self) -> None:
        """Compile memparse and verify the output for an Alloc request."""
        with tempfile.TemporaryDirectory() as td:
            binary = Path(td) / "memparse"
            compiler = os.getenv("CXX", "clang++")
            cmd = [compiler, "-std=c++17", str(SOURCE), "-o", str(binary)]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as exc:
                self.skipTest(f"{compiler} failed: {exc}")
            result = subprocess.check_output(
                [str(binary), "0", "0", "1000", "0"], text=True
            )
            self.assertIn("op: Alloc", result)
            self.assertIn("size: 4096", result)


if __name__ == "__main__":
    unittest.main()
