import subprocess
import tempfile
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
CODE = r"""
#include "posix.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/wait.h>
int main() {
    char buf[16];
    posix_getcwd(buf, sizeof(buf));
    posix_chdir("/");
    posix_link("a","b");
    posix_unlink("a");
    char *argv[] = {"/bin/true", NULL};
    char *env[] = {NULL};
    posix_execve("/bin/true", argv, env);
    posix_wait(NULL);
    posix_waitpid(0, NULL, 0);
    int s = posix_socket(AF_INET, SOCK_STREAM, 0);
    posix_bind(s, NULL, 0);
    posix_listen(s, 1);
    posix_accept(s, NULL, NULL);
    return 0;
}
"""

class PosixBuildTest(unittest.TestCase):
    def test_compile(self) -> None:
        with tempfile.TemporaryDirectory() as td:
            src = Path(td) / "test.c"
            src.write_text(CODE)
            cmd = [
                "gcc",
                "-std=c2x",
                "-Werror",
                "-I",
                str(ROOT / "user/lib/posix"),
                "-c",
                str(src),
            ]
            subprocess.check_output(cmd, stderr=subprocess.STDOUT)

if __name__ == "__main__":
    unittest.main()
