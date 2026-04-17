from __future__ import annotations

from typing import Any

AllTypedefs = dict[str, dict[str, Any]]
IdentSubstitutionMap = dict[str, str] | None


def namespace_segments(namespace_cpp: str) -> list[str]:
    parts = [p for p in namespace_cpp.split("::") if p]
    out: list[str] = []
    for part in parts:
        if part and part[0].isdigit():
            part = "_" + part
        out.append(part)
    return out


def allocate_unique_idents(style: Any, order: list[str]) -> dict[str, str]:
    used: set[str] = set()
    out: dict[str, str] = {}
    for asn_name in order:
        base = style.value_ident(asn_name)
        candidate = base
        suffix = 2
        while candidate in used:
            candidate = f"{base}_{suffix}"
            suffix += 1
        out[asn_name] = candidate
        used.add(candidate)
    return out


def codec_builtin_literal(codec: str, ty: str) -> str | None:
    pattern = f"{codec}::{{}}"
    table = {
        "INTEGER": pattern.format("integer"),
        "BOOLEAN": pattern.format("boolean"),
        "BIT STRING": pattern.format("bit_string"),
        "OCTET STRING": pattern.format("octet_string"),
        "NULL": pattern.format("null"),
        "REAL": pattern.format("real"),
        "OBJECT IDENTIFIER": pattern.format("object_identifier"),
        "IA5String": pattern.format("ia5_string"),
        "PrintableString": pattern.format("printable_string"),
        "UTF8String": pattern.format("utf_string"),
        "GeneralString": pattern.format("general_string"),
        "BMPString": pattern.format("utf_string"),
        "VisibleString": pattern.format("visible_string"),
        "GraphicString": pattern.format("graphic_string"),
        "UTCTime": pattern.format("utc_time"),
        "GeneralizedTime": pattern.format("generalized_time"),
    }
    return table.get(ty)


def tree_refs_type_name(syntax: object, target: str) -> bool:
    if isinstance(syntax, dict):
        if syntax.get("type") == target:
            return True
        for child in syntax.values():
            if tree_refs_type_name(child, target):
                return True
    elif isinstance(syntax, list):
        for item in syntax:
            if tree_refs_type_name(item, target):
                return True
    return False


def choice_arm_refs_self_deep(arm: dict, self_asn_name: str) -> bool:
    return tree_refs_type_name(arm, self_asn_name)


def arm_references_self(arm: dict, self_asn_name: str) -> bool:
    ty = arm.get("type")
    if ty == self_asn_name:
        return True
    if ty in ("SEQUENCE OF", "SET OF"):
        element = arm.get("element")
        return isinstance(element, dict) and element.get("type") == self_asn_name
    return False
