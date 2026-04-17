from __future__ import annotations
from typing import Any
from .support import AllTypedefs


def member_resolves_to_choice_spec(merged: AllTypedefs, m: dict) -> dict | None:
    if not m:
        return None
    if m.get("type") == "CHOICE":
        return m
    ty = m.get("type")
    if isinstance(ty, str) and ty in merged:
        spec = merged[ty]
        if spec.get("type") == "CHOICE" and "parameters" not in spec:
            return spec
    return None


def flatten_nested_choice_members(
    merged: AllTypedefs,
    members: list[dict],
    hint: str,
    *,
    expanding_typedefs: frozenset[str] | None = None,
) -> list[dict]:
    if expanding_typedefs is None:
        expanding_typedefs = frozenset()
    out: list[dict] = []
    for m in members:
        if m is None:
            continue
        ch = member_resolves_to_choice_spec(merged, m)
        if ch is None:
            out.append(m)
            continue
        ty_ref = m.get("type")
        if (
            isinstance(ty_ref, str)
            and ty_ref in expanding_typedefs
            and merged.get(ty_ref, {}).get("type") == "CHOICE"
        ):
            out.append(m)
            continue
        new_expanding = expanding_typedefs
        if isinstance(ty_ref, str) and merged.get(ty_ref, {}).get("type") == "CHOICE":
            new_expanding = expanding_typedefs | frozenset([ty_ref])
        inners = [x for x in ch.get("members", []) if x is not None]
        nested_hint = f"{hint}_{m.get('name', 'arm')}"
        out.extend(
            flatten_nested_choice_members(
                merged, inners, nested_hint, expanding_typedefs=new_expanding
            )
        )
    return out
