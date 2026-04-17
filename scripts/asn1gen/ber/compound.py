from __future__ import annotations
from typing import Any
from .support import AllTypedefs


def effective_construct(merged: AllTypedefs, name: str, _seen: set[str] | None = None) -> str | None:
    if _seen is None:
        _seen = set()
    if name in _seen or name not in merged:
        return None
    _seen.add(name)
    spec = merged[name]
    if "parameters" in spec:
        return None
    ty = spec.get("type")
    if not isinstance(ty, str):
        return None
    if ty in ("CHOICE", "SEQUENCE", "ENUMERATED", "SEQUENCE OF", "SET OF"):
        return ty
    if ty in merged:
        return effective_construct(merged, ty, _seen)
    return None


def compound_typedef_arm(merged: AllTypedefs, m: dict) -> bool:
    ty = m.get("type")
    if isinstance(ty, str) and ty in merged:
        ec = effective_construct(merged, ty)
        return ec in ("CHOICE", "SEQUENCE")
    inner_d = {k: v for k, v in m.items() if k not in ("name", "optional", "default", "tag")}
    return inner_d.get("type") in ("CHOICE", "SEQUENCE")
