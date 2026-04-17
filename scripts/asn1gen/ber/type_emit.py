from __future__ import annotations

from typing import TYPE_CHECKING, Any

from .compound import compound_typedef_arm
from .support import IdentSubstitutionMap, codec_builtin_literal
from .tagging import apply_tag

if TYPE_CHECKING:
    from .emitter import BerEmitter


def implicit_application_sequence_arm(merged: dict[str, dict], ref: str, arm: dict) -> bool:
    tag = arm.get("tag") or {}
    if tag.get("class") != "APPLICATION" or tag.get("kind") != "IMPLICIT":
        return False
    spec = merged.get(ref)
    return isinstance(spec, dict) and spec.get("type") == "SEQUENCE"


def implicit_implicit_sequence_typedef(merged: dict[str, dict], asn_name: str) -> bool:
    for spec in merged.values():
        if spec.get("type") != "CHOICE":
            continue
        for arm in spec.get("members") or []:
            if not arm or arm.get("type") != asn_name:
                continue
            if implicit_application_sequence_arm(merged, asn_name, arm):
                return True
    return False


def type_expression(
    emitter: BerEmitter,
    spec: dict[str, Any],
    hint: str,
    tag_override: dict[str, Any] | None = None,
    *,
    self_subst: IdentSubstitutionMap = None,
) -> str:
    tag = tag_override if tag_override is not None else spec.get("tag")
    ty = spec.get("type")

    if ty == "INTEGER":
        return apply_tag(emitter.cc, emitter.style, emitter.cc("integer"), tag)
    if ty == "BOOLEAN":
        return apply_tag(emitter.cc, emitter.style, emitter.cc("boolean"), tag)
    if ty == "ENUMERATED":
        en = emitter.inline_enum_names.get(hint) if emitter.inline_enum_names else None
        if not en:
            en = emitter.fresh(hint + "_enum")
        emitter.emit_enum_values(en, spec.get("values", []))
        return apply_tag(emitter.cc, emitter.style, emitter.cc(f"enumerated<{en}>()"), tag)
    if ty == "SEQUENCE":
        parts = _sequence_parts(emitter, spec, hint, self_subst=self_subst)
        body = emitter.cc(f"sequence({', '.join(parts)})") if parts else emitter.cc("sequence()")
        return apply_tag(emitter.cc, emitter.style, body, tag)
    if ty == "SEQUENCE OF":
        el = spec.get("element")
        inner = (
            type_expression(emitter, el, f"{hint}_el", self_subst=self_subst)
            if isinstance(el, dict)
            else emitter.cc("octet_string")
        )
        return apply_tag(emitter.cc, emitter.style, emitter.cc(f"sequence_of({inner})"), tag)
    if ty == "SET OF":
        el = spec.get("element")
        inner = (
            type_expression(emitter, el, f"{hint}_el", self_subst=self_subst)
            if isinstance(el, dict)
            else emitter.cc("octet_string")
        )
        return apply_tag(emitter.cc, emitter.style, emitter.cc(f"set_of({inner})"), tag)
    if ty == "CHOICE":
        from . import choice_emit

        return apply_tag(
            emitter.cc,
            emitter.style,
            choice_emit.choice_expression(
                emitter, spec, hint, outer_asn_name=None, emit_tag_enum=True, self_subst=self_subst
            ),
            tag,
            inner_is_choice=True,
        )
    if isinstance(ty, str) and (lit := codec_builtin_literal(emitter._codec, ty)):
        return apply_tag(emitter.cc, emitter.style, lit, tag)
    if isinstance(ty, str) and ty in emitter.merged:
        if "parameters" in emitter.merged[ty]:
            return apply_tag(emitter.cc, emitter.style, emitter.cc("octet_string"), tag)
        is_ch = emitter.merged[ty].get("type") == "CHOICE"
        idn = emitter.ident_subst(ty, self_subst)
        return apply_tag(emitter.cc, emitter.style, idn, tag, inner_is_choice=is_ch)

    return apply_tag(emitter.cc, emitter.style, emitter.cc("octet_string"), tag)


def _sequence_parts(emitter: BerEmitter, spec: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> list[str]:
    parts: list[str] = []
    for member in spec.get("members", []):
        if member is None:
            continue
        parts.append(sequence_member_expr(emitter, member, hint, self_subst=self_subst))
    return parts


def sequence_contents_body(emitter: BerEmitter, spec: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
    ps = _sequence_parts(emitter, spec, hint, self_subst=self_subst)
    if not ps:
        return emitter.cc("sequence_contents()")
    return emitter.cc(f"sequence_contents({', '.join(ps)})")


def sequence_member_expr(emitter: BerEmitter, member: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
    opt = member.get("optional")
    expr = member_type_expr(emitter, member, hint, self_subst=self_subst)
    if opt:
        return emitter.cc(f"optional({expr})")
    return expr


def member_type_expr(emitter: BerEmitter, member: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
    tag = member.get("tag")
    subhint = f"{hint}_{member.get('name', 'f')}"
    if "type" in member and isinstance(member["type"], str) and member["type"] in emitter.merged:
        ref = member["type"]
        if "parameters" in emitter.merged[ref]:
            inner = emitter.cc("octet_string")
        else:
            inner = emitter.ident_subst(ref, self_subst)
        is_ch = emitter.merged[ref].get("type") == "CHOICE"
        return apply_tag(emitter.cc, emitter.style, inner, tag, inner_is_choice=is_ch)

    inner_d = {k: v for k, v in member.items() if k not in ("name", "optional", "default", "tag")}
    expr = type_expression(emitter, inner_d, subhint, tag_override=None, self_subst=self_subst)
    inner_is_choice = inner_d.get("type") == "CHOICE"
    return apply_tag(emitter.cc, emitter.style, expr, tag, inner_is_choice=inner_is_choice)
