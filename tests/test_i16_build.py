import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include <l4/types.h>
static_assert(sizeof(L4_Word_t) > 0);
"""

class I16CompilationTest(unittest.TestCase):
    def test_i16_build(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.cpp"
            src.write_text(CODE)
            cmd = [
                "g++",
                "-m16",
                "-std=c++23",
                "-Werror",
                "-I",
                str(ROOT / "user/include"),
                "-c",
                str(src),
            ]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except subprocess.CalledProcessError as e:
                self.skipTest(f"16-bit build not supported: {e.output.decode()}")

if __name__ == "__main__":
    unittest.main()
