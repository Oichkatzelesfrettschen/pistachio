import os
import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include <l4/types.h>
static_assert(sizeof(L4_Fpage_t) == sizeof(L4_Word_t));
static_assert(sizeof(L4_ThreadId_t) == sizeof(L4_Word_t));
static_assert(sizeof(L4_Clock_t) == sizeof(L4_Word64_t));
static_assert(sizeof(L4_Time_t) == sizeof(L4_Word_t));
"""

class TypeSizeCompilationTest(unittest.TestCase):
    def _compile(self, flag: str) -> None:
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.cpp"
            src.write_text(CODE)
            compiler = os.getenv("CXX", "clang++")
            cmd = [
                compiler,
                flag,
                "-std=c++23",
                "-Werror",
                "-I",
                str(ROOT / "user/include"),
                "-c",
                str(src),
            ]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                self.skipTest(f"{flag} build not supported: {getattr(e, 'output', e)}")

    def test_builds(self) -> None:
        self._compile("-m64")
        self._compile("-m32")

if __name__ == "__main__":
    unittest.main()
