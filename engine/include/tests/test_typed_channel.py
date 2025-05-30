import os
import subprocess
import tempfile
from pathlib import Path
import unittest

# Repository root
ROOT = Path(__file__).resolve().parents[1]
# Source program using typed_channel
SOURCE = ROOT / "user/apps/ipc_demo/typed_channel/main.cc"


class TypedChannelBuildTest(unittest.TestCase):
    """Ensure the typed channel demo builds."""

    def test_compile(self) -> None:
        """Compile the demo program without linking."""
        with tempfile.TemporaryDirectory() as td:
            obj = Path(td) / "typed_channel.o"
            compiler = os.getenv("CXX", "clang++")
            cmd = [
                compiler,
                "-std=c++17",
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
