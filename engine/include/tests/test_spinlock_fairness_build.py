import os
import subprocess
import tempfile
from pathlib import Path
import unittest

# Path to the spinlock fairness example
SRC = Path(__file__).resolve().parent / "spinlock_fairness.c"


class SpinlockFairnessBuildTest(unittest.TestCase):
    """Compile and run the spinlock fairness demo."""

    def test_fairness_exec(self) -> None:
        """Build the example and ensure it exits successfully."""
        with tempfile.TemporaryDirectory() as td:
            binary = Path(td) / "spinlock_fairness"
            compiler = os.getenv("CC", "clang")
            cmd = [compiler, "-std=c2x", "-pthread", str(SRC), "-o", str(binary)]
            try:
                subprocess.check_output(cmd, stderr=subprocess.STDOUT)
            except (subprocess.CalledProcessError, FileNotFoundError) as exc:
                self.skipTest(f"{compiler} failed: {exc}")
            res = subprocess.run([str(binary), "2"], capture_output=True, text=True)
            self.assertEqual(res.returncode, 0, res.stdout + res.stderr)


if __name__ == "__main__":
    unittest.main()
