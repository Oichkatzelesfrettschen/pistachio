import os
import subprocess
import tempfile
import textwrap
import unittest

class CompilerAttributeTest(unittest.TestCase):
    def test_function_pointer_attributes(self) -> None:
        code = textwrap.dedent(
            """
            #include <l4/compiler.h>

            void L4_CDECL cdecl_fn(int) {}
            void L4_FASTCALL fast_fn(int, int) {}

            using p1 = L4_CDECL void (*)(int);
            using p2 = L4_FASTCALL void (*)(int, int);

            int main() {
                p1 a = cdecl_fn;
                p2 b = fast_fn;
                (void)a; (void)b;
                return 0;
            }
            """
        )
        with tempfile.NamedTemporaryFile('w', suffix='.cc', delete=False) as f:
            f.write(code)
            name = f.name
        try:
            subprocess.run([
                os.getenv('CXX', 'g++'),
                '-std=c++23',
                '-Iuser/include',
                '-c',
                name,
            ], check=True)
        finally:
            os.unlink(name)

if __name__ == '__main__':
    unittest.main()
