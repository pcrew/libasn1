from __future__ import annotations

import re
from typing import Any

from ..style import cpp_enum_member


def append_enum_class_int(lines: list[str], enum_name: str, values: list) -> None:
    lines.append(f"enum class {enum_name} : int {{")
    for v in values:
        if v is None:
            continue
        if isinstance(v, tuple):
            nm, num = v[0], v[1]
        else:
            nm, num = v, 0
        lab = cpp_enum_member(str(nm))
        lines.append(f"    {lab} = {int(num)},")
    lines.append("};")


def append_choice_tag_enum(lines: list[str], enum_name: str, members: list[dict]) -> None:
    lines.append(f"enum class {enum_name} : int {{")
    for arm in members:
        tag = arm.get("tag") or {}
        n = tag.get("number")
        if n is None:
            continue
        lab = cpp_enum_member(str(arm.get("name", "alt")))
        lines.append(f"    {lab} = {int(n)},")
    lines.append("};")


def choice_enum_name_for_hint(
    style: Any,
    used_enum_names: set[str],
    hint: str,
    outer_asn_name: str | None,
) -> str:
    if outer_asn_name:
        return style.enum_class_name(outer_asn_name)
    if hint.endswith("_el"):
        return re.sub(r"_el$", "", hint) + "_enum"
    base = f"choice_{hint}_enum"
    cand = base
    n = 2
    while cand in used_enum_names:
        cand = f"{base}_{n}"
        n += 1
    used_enum_names.add(cand)
    return cand
