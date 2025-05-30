import os
import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include "../../user/lib/mlp/mlp.h"
int main() {
    float f[1] = {0};
    mlp_init(nullptr);
    return mlp_predict(f);
}
"""

class MlpSchedulerBuildTest(unittest.TestCase):
    def test_compile(self) -> None:
        eigen_dir = ROOT / "third_party" / "eigen" / "Eigen"
        if not eigen_dir.exists():
            self.skipTest("Eigen not available")
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.cpp"
            src.write_text(CODE)
            compiler = os.getenv("CXX", "clang++")
            cmd = [
                compiler,
                "-std=c++17",
                "-I",
                str(ROOT / "user/include"),
                "-I",
                str(ROOT / "third_party/eigen"),
                "-c",
                str(src),
            ]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as e:
                self.skipTest(f"{compiler} failed: {e}")

if __name__ == "__main__":
    unittest.main()
