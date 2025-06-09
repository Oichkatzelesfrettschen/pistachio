import subprocess
from typing import Sequence
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def run(cmd: Sequence[str], input: str | None = None) -> str:
    """Return the trimmed stdout of executing *cmd*.

    Parameters
    ----------
    cmd : Sequence[str]
        Command and arguments to run via :func:`subprocess.run`.
    input : str | None, optional
        Text passed to the process on stdin.

    Returns
    -------
    str
        Captured standard output with trailing whitespace removed.
    """
    res = subprocess.run(cmd, input=input, text=True, capture_output=True, check=True)
    return res.stdout.strip()


def test_bitfield_gen(tmp_path):
    spec = tmp_path / "spec.yml"
    spec.write_text("FIELD_A: [0, 1]\nFIELD_B: [2, 3]\n")
    out = run([str(ROOT / "tools/bitfield_gen.py"), str(spec)])
    assert "FIELD_A_SHIFT" in out
    assert "FIELD_B_MASK" in out


def test_condition_to_cpp(tmp_path):
    xml = tmp_path / "cond.xml"
    xml.write_text("<and><config var='A'/><not><config var='B'/></not></and>")
    out = run([str(ROOT / "tools/condition.py"), str(xml)])
    assert out == "(defined(A) && !defined(B))"


def test_syscall_header_gen(tmp_path):
    out_file = tmp_path / "sys.h"
    run([str(ROOT / "tools/syscall_header_gen.py"), str(out_file)])
    data = out_file.read_text()
    assert "SYSCALL_CALL" in data


def test_invocation_header_gen(tmp_path):
    out_file = tmp_path / "inv.h"
    run([str(ROOT / "tools/invocation_header_gen.py"), str(out_file)])
    data = out_file.read_text()
    assert "seL4_Call" in data


def test_changed_sh():
    out = run(["bash", str(ROOT / "tools/changed.sh"), "HEAD"])
    assert isinstance(out, str)


def test_xmllint_sh(tmp_path):
    xml = tmp_path / "a.xml"
    xml.write_text("<root/>")
    out = run(["bash", str(ROOT / "tools/xmllint.sh"), str(xml)])
    assert out == ""
