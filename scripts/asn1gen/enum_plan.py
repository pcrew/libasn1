from __future__ import annotations
from collections import defaultdict
from .style import asn_to_snake


def _strip(m: dict, junk: dict) -> dict:
    return {k: v for k, v in m.items() if k not in junk}


def _enum_signature(values: list) -> tuple[tuple[str, int], ...]:
    out: list[tuple[str, int]] = []
    for v in values:
        if v is None:
            continue
        if isinstance(v, tuple):
            nm, num = v[0], int(v[1])
        else:
            nm, num = v, 0
        out.append((str(nm), int(num)))
    return tuple(out)


def _is_ref(inner: dict, merged: dict[str, dict]) -> bool:
    t = inner.get("type")
    return isinstance(t, str) and t in merged


def _path_to_short_stem(path: list[str]) -> str:
    parts: list[str] = []
    for p in path:
        if p == "el":
            parts.append("element")
        else:
            parts.append(asn_to_snake(p))
    return "_".join(parts) if parts else "unnamed"


def collect_inline_enum_hints(
    spec: dict,
    merged: dict[str, dict],
    *,
    type_hint: str,
    path: list[str],
) -> list[tuple[str, list, list[str]]]:
    ty = spec.get("type")
    out: list[tuple[str, list, list[str]]] = []

    if ty == "ENUMERATED":
        key = type_hint + ("_" + "_".join(path) if path else "")
        out.append((key, spec.get("values", []), list(path)))
        return out

    if ty == "SEQUENCE":
        for m in spec.get("members") or []:
            if m is None:
                continue
            nm = str(m.get("name", "f"))
            inner = _strip(m, ("name", "optional", "default", "tag"))
            if _is_ref(inner, merged):
                continue
            out.extend(collect_inline_enum_hints(inner, merged, type_hint=type_hint, path=path + [nm]))
        return out

    if ty == "CHOICE":
        for m in spec.get("members") or []:
            if m is None:
                continue
            nm = str(m.get("name", "arm"))
            inner = _strip(m, ("name", "tag"))
            if _is_ref(inner, merged):
                continue
            out.extend(collect_inline_enum_hints(inner, merged, type_hint=type_hint, path=path + [nm]))
        return out

    if ty in ("SEQUENCE OF", "SET OF"):
        el = spec.get("element")
        if isinstance(el, dict) and not _is_ref(el, merged):
            out.extend(collect_inline_enum_hints(el, merged, type_hint=type_hint, path=path + ["el"]))
        return out

    return out


def plan_inline_enum_names(merged: dict[str, dict]) -> dict[str, str]:
    entries: list[tuple[str, tuple[tuple[str, int], ...], str, str]] = []
    seen: set[tuple[str, tuple]] = set()

    for asn_name, spec in merged.items():
        if "parameters" in spec:
            continue
        if spec.get("type") == "ENUMERATED":
            continue
        th = asn_to_snake(asn_name)
        for key, values, path in collect_inline_enum_hints(spec, merged, type_hint=th, path=[]):
            sig = _enum_signature(values)
            k = (key, sig)
            if k in seen:
                continue
            seen.add(k)
            stem = _path_to_short_stem(path)
            entries.append((key, sig, th, stem))

    by_stem: dict[str, list[tuple[str, tuple[tuple[str, int], ...], str]]] = defaultdict(list)
    for key, sig, th, stem in entries:
        by_stem[stem].append((key, sig, th))

    result: dict[str, str] = {}
    used_cpp_names: set[str] = set()

    for stem in sorted(by_stem.keys()):
        items = by_stem[stem]
        sigs = {x[1] for x in items}

        if len(sigs) == 1:
            base = f"{stem}_enum"
            name = base
            n = 2
            while name in used_cpp_names:
                name = f"{base}_{n}"
                n += 1
            used_cpp_names.add(name)
            for key, _sig, _th in items:
                result[key] = name
            continue

        for key, _sig, th in items:
            base = f"{th}_{stem}_enum"
            name = base
            n = 2
            while name in used_cpp_names:
                name = f"{base}_{n}"
                n += 1
            used_cpp_names.add(name)
            result[key] = name

    return result
