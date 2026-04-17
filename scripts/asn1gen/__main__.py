from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .ber.emitter import emit_ber_header, emit_der_header
from .model import load_modules, merge_modules, topo_sort
from .profile import load_emit_profile, resolve_backend


def generate(
    asn1_files: list[Path],
    output: Path,
    *,
    profile_file: Path | None,
    namespace: str | None,
    backend_override: str | None,
) -> tuple[str, int]:
    extras, style = load_emit_profile(profile_file)
    backend = resolve_backend(backend_override, extras)
    ns = namespace if namespace is not None else style.namespace_default

    parsed = load_modules(asn1_files)
    merged = merge_modules(parsed)
    order = topo_sort(merged)
    if backend == "der":
        text = emit_der_header(merged, order, ns, style)
    else:
        text = emit_ber_header(merged, order, ns, style)
    return text, len(order)


def main() -> int:
    p = argparse.ArgumentParser(description="Generate libasn C++ protocol headers from ASN.1.")
    p.add_argument("asn1_files", nargs="+", type=Path)
    p.add_argument("-o", "--output", type=Path, required=True)
    p.add_argument(
        "--profile-file",
        type=Path,
        default=None,
        metavar="PATH",
        help="Optional asn1gen.json (EmitStyle fields + optional 'backend'); else EmitStyle defaults.",
    )
    p.add_argument("-n", "--namespace", default=None)
    p.add_argument(
        "--backend",
        choices=["ber", "der"],
        default=None,
        help="Force codec; if omitted, use profile file 'backend' or ber.",
    )
    args = p.parse_args()

    for f in args.asn1_files:
        if not f.exists():
            print(f"Missing file: {f}", file=sys.stderr)
            return 1

    if args.profile_file is not None and not args.profile_file.is_file():
        print(f"Missing profile file: {args.profile_file}", file=sys.stderr)
        return 1

    text, n = generate(
        args.asn1_files,
        args.output,
        profile_file=args.profile_file,
        namespace=args.namespace,
        backend_override=args.backend,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(text, encoding="utf-8")
    print("wrote", args.output, n, "types")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
