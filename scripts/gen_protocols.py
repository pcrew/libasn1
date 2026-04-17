#!/usr/bin/env python3
from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPTS = ROOT / "scripts"

if str(SCRIPTS) not in sys.path:
    sys.path.insert(0, str(SCRIPTS))

from asn1gen.profile import find_profile_file as find_asn1gen_profile  # noqa: E402
ASN1_ROOT = ROOT / "asn1"
OUT_ROOT = ROOT / "include" / "libasn" / "protocols"


def namespace_from_dirname(name: str) -> str:
    if name and name[0].isdigit():
        return "_" + name
    return name


def cpp_namespace_from_relpath(relpath: Path) -> str:
    return "::".join(namespace_from_dirname(p) for p in relpath.parts)


def clang_format_inplace(paths: list[Path], *, root: Path) -> None:
    if not paths:
        return
    exe = os.environ.get("CLANG_FORMAT", "clang-format")
    if shutil.which(exe) is None:
        print(f"gen_protocols: {exe!r} not in PATH, skipping format", file=sys.stderr)
        return
    subprocess.run([exe, "-i", *[str(p) for p in paths]], cwd=str(root), check=True)


def main() -> int:
    if not ASN1_ROOT.is_dir():
        print("gen_protocols: missing directory asn1/", file=sys.stderr)
        return 1

    candidates = [ASN1_ROOT, *ASN1_ROOT.rglob("*")]
    dirs_with_asn = sorted((p for p in candidates if p.is_dir()), key=lambda p: str(p))

    generated: list[Path] = []

    for dir_path in dirs_with_asn:
        asn_files = sorted(dir_path.glob("*.asn"))
        if not asn_files:
            continue
        try:
            relpath = dir_path.relative_to(ASN1_ROOT)
        except ValueError:
            continue
        if relpath == Path("."):
            print("gen_protocols: skipping .asn files directly under asn1/ root", file=sys.stderr)
            continue

        pkg = dir_path.name
        out_dir = OUT_ROOT / relpath
        out_dir.mkdir(parents=True, exist_ok=True)
        out_h = out_dir / f"{pkg}.h"

        namespace = cpp_namespace_from_relpath(relpath)

        profile_path = find_asn1gen_profile(dir_path)

        cmd: list[str] = [
            sys.executable,
            "-m",
            "asn1gen",
            *[str(p) for p in asn_files],
            "-o",
            str(out_h),
            "-n",
            namespace,
        ]
        if profile_path is not None:
            cmd.extend(["--profile-file", str(profile_path)])

        env = os.environ.copy()
        env["PYTHONPATH"] = str(SCRIPTS)

        print("gen_protocols:", relpath.as_posix(), "->", out_h.relative_to(ROOT), f"({len(asn_files)} files)")
        subprocess.run(cmd, cwd=str(ROOT), env=env, check=True)
        generated.append(out_h)

    clang_format_inplace(generated, root=ROOT)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
