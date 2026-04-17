from __future__ import annotations

from typing import TYPE_CHECKING, Any

from ..style import asn_to_snake, cpp_enum_member
from .choice_flatten import flatten_nested_choice_members
from .support import IdentSubstitutionMap, arm_references_self, choice_arm_refs_self_deep
from .tagging import apply_tag, arm_use_explicit_wrap, arm_use_plain_with, wrap_tagged_compound_arm
from . import type_emit

if TYPE_CHECKING:
    from .emitter import BerEmitter


def use_assertion_value_for_arm(emitter: BerEmitter, hint: str, arm: dict) -> bool:
    if not emitter.style.substring_choice_use_assertion_value:
        return False
    if "_substrings" not in hint.lower():
        return False
    if "type" not in arm:
        return False
    ty = arm["type"]
    if ty in ("OCTET STRING", "IA5String", "UTF8String"):
        return True
    if isinstance(ty, str) and ty in emitter.merged:
        if ty in ("LDAPString", "AssertionValue"):
            return True
    return False


def tagged_choice_arm_dotwith(emitter: BerEmitter, arm: dict, en: str, arm_label: str, inner: str) -> str:
    if arm_use_explicit_wrap(emitter.merged, emitter._application_pdu_wrap, arm):
        wrapped = wrap_tagged_compound_arm(emitter.cc, arm, inner)
        return f".with({wrapped})"
    if arm_use_plain_with(emitter._application_pdu_wrap, arm):
        return f".with({inner})"
    return f".with<{en}::{arm_label}>({inner})"


def choice_arm_inner_for_with_enum(
    emitter: BerEmitter,
    arm: dict,
    hint: str,
    *,
    self_subst: IdentSubstitutionMap = None,
) -> str:
    if use_assertion_value_for_arm(emitter, hint, arm):
        return "assertion_value"
    if "type" in arm and isinstance(arm["type"], str) and arm["type"] in emitter.merged:
        ref = arm["type"]
        if "parameters" in emitter.merged[ref]:
            return emitter.cc("octet_string")
        if isinstance(ref, str) and ref in emitter._application_pdu_wrap:
            return emitter.ident_subst(ref, self_subst)
        if type_emit.implicit_application_sequence_arm(emitter.merged, ref, arm):
            sh = asn_to_snake(ref)
            return type_emit.sequence_contents_body(emitter, emitter.merged[ref], sh, self_subst=self_subst)
        return emitter.ident_subst(ref, self_subst)
    inner_d = {k: v for k, v in arm.items() if k not in ("name", "tag")}
    return type_emit.type_expression(
        emitter,
        inner_d,
        f"{hint}_{arm.get('name', 'arm')}",
        tag_override=None,
        self_subst=self_subst,
    )


def choice_arm_for_universal_discrimination(emitter: BerEmitter, arm: dict, hint: str) -> str:
    if "type" in arm and isinstance(arm["type"], str) and arm["type"] in emitter.merged:
        ref = arm["type"]
        inner = emitter.cc("octet_string") if "parameters" in emitter.merged[ref] else emitter.ident(ref)
        return apply_tag(emitter.cc, emitter.style, inner, arm.get("tag"))
    inner_d = {k: v for k, v in arm.items() if k not in ("name", "tag")}
    expr = type_emit.type_expression(
        emitter, inner_d, f"{hint}_{arm.get('name', 'arm')}", tag_override=None
    )
    return apply_tag(emitter.cc, emitter.style, expr, arm.get("tag"))


def is_recursive_choice(spec: dict, self_asn_name: str) -> bool:
    if spec.get("type") != "CHOICE":
        return False
    members = [x for x in spec.get("members", []) if x]
    return any(choice_arm_refs_self_deep(arm, self_asn_name) for arm in members)


def emit_split_recursive_choice(emitter: BerEmitter, asn_name: str, spec: dict) -> None:
    snake = asn_to_snake(asn_name)
    inner_ident = f"__{snake}__" if emitter.style.recursive_choice_inner_dunder_name else f"{snake}_logic"
    enum_name = emitter.style.enum_class_name(asn_name)
    members = [x for x in spec.get("members", []) if x is not None]
    leaves = [arm for arm in members if not choice_arm_refs_self_deep(arm, asn_name)]
    if not leaves:
        raise RuntimeError(
            f"asn1gen: recursive CHOICE {asn_name!r} has no arm free of self-reference; "
            "cannot emit split_recursive_choice"
        )

    emitter.emit_choice_tag_enum(enum_name, members)

    leaf_spec: dict[str, Any] = {"type": "CHOICE", "members": leaves}
    inner_expr = choice_expression(
        emitter, leaf_spec, snake, outer_asn_name=asn_name, emit_tag_enum=False, self_subst=None
    )
    emitter.lines.append(f"constexpr auto {inner_ident} = {inner_expr};")

    subst: dict[str, str] = {asn_name: inner_ident}
    opaque = emitter.style.recursive_choice_opaque_self_payload
    full_parts: list[str] = []
    for arm in members:
        arm_label = cpp_enum_member(str(arm.get("name", "alt")))
        if arm_references_self(arm, asn_name):
            ty = arm.get("type")
            if ty in ("SET OF", "SEQUENCE OF") and isinstance(arm.get("element"), dict):
                elty = arm["element"].get("type")
                if elty == asn_name:
                    payload = emitter.cc("octet_string") if opaque else inner_ident
                    if ty == "SEQUENCE OF":
                        full_parts.append(
                            f".with<{enum_name}::{arm_label}>({emitter.cc(f'sequence_of({payload})')})"
                        )
                    else:
                        full_parts.append(
                            f".with<{enum_name}::{arm_label}>({emitter.cc(f'set_of({payload})')})"
                        )
                    continue
            if ty == asn_name:
                if opaque:
                    inner = apply_tag(emitter.cc, emitter.style, emitter.cc("octet_string"), arm.get("tag"))
                else:
                    inner = (
                        f"{emitter.cc(f'explicit_({inner_ident})')}"
                        if emitter.style.wrap_self_choice_in_explicit
                        else inner_ident
                    )
                full_parts.append(f".with<{enum_name}::{arm_label}>({inner})")
                continue
        inner = choice_arm_inner_for_with_enum(emitter, arm, snake, self_subst=subst)
        full_parts.append(tagged_choice_arm_dotwith(emitter, arm, enum_name, arm_label, inner))

    emitter.lines.append(
        f"constexpr auto {snake} = ({emitter.cc(f'choice<{enum_name}>()')}{''.join(full_parts)});"
    )


def choice_expression(
    emitter: BerEmitter,
    spec: dict,
    hint: str,
    *,
    outer_asn_name: str | None,
    emit_tag_enum: bool = True,
    self_subst: IdentSubstitutionMap = None,
) -> str:
    from . import enum_lines

    members = [x for x in spec.get("members", []) if x is not None]
    if not members:
        return emitter.cc("choice<int>()")

    def has_discriminant_tag(arm: dict) -> bool:
        t = arm.get("tag") or {}
        if t.get("number") is None:
            return False
        cls = t.get("class", "CONTEXT_SPECIFIC")
        return cls in ("CONTEXT_SPECIFIC", "CONTEXT", None, "APPLICATION")

    all_tagged = all(has_discriminant_tag(arm) for arm in members)
    if all_tagged:
        en = enum_lines.choice_enum_name_for_hint(emitter.style, emitter._used_enum_names, hint, outer_asn_name)
        if emit_tag_enum:
            emitter.emit_choice_tag_enum(en, members)
        parts: list[str] = []
        for arm in members:
            arm_label = cpp_enum_member(str(arm.get("name", "alt")))
            inner = choice_arm_inner_for_with_enum(emitter, arm, hint, self_subst=self_subst)
            parts.append(tagged_choice_arm_dotwith(emitter, arm, en, arm_label, inner))
        return f"({emitter.cc(f'choice<{en}>()')}{''.join(parts)})"

    flat_members = flatten_nested_choice_members(emitter.merged, members, hint)
    chain = "".join(
        f".with({choice_arm_for_universal_discrimination(emitter, arm, f'{hint}_c{i}')})"
        for i, arm in enumerate(flat_members)
    )
    return f"({emitter.cc('choice<int>()')}{chain})"
