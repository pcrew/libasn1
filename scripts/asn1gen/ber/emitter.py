from __future__ import annotations

from typing import Any

from ..enum_plan import plan_inline_enum_names
from ..model import application_pdu_wrap_tags, cpp_name
from ..profile import emit_style_default
from ..style import EmitStyle, asn_to_snake
from . import choice_emit, type_emit
from .enum_lines import append_choice_tag_enum, append_enum_class_int
from .support import IdentSubstitutionMap, allocate_unique_idents, namespace_segments
from .tagging import apply_tag


class BerEmitter:
    def __init__(
        self,
        merged: dict[str, dict],
        namespace: str,
        style: EmitStyle | None = None,
        *,
        codec: str = "ber",
    ) -> None:
        if codec not in ("ber", "der"):
            raise ValueError(f"unsupported codec {codec!r} (expected 'ber' or 'der')")
        self._codec = codec
        self.merged = merged
        self.namespace = namespace
        self.style = style or emit_style_default()
        self.lines: list[str] = []
        self._anon = 0
        self.inline_enum_names: dict[str, str] = plan_inline_enum_names(merged)
        self._used_enum_names: set[str] = set(self.inline_enum_names.values())
        self._emitted_enum_classes: set[str] = set()
        for typedef_name, typedef_spec in merged.items():
            if "parameters" in typedef_spec:
                continue
            kind = typedef_spec.get("type")
            if kind == "ENUMERATED" or kind == "CHOICE":
                self._used_enum_names.add(self.style.enum_class_name(typedef_name))
        self._application_pdu_wrap: dict[str, int] = application_pdu_wrap_tags(merged)
        self._ident_map: dict[str, str] | None = None

    def cc(self, tail: str) -> str:
        return f"{self._codec}::{tail}"

    def fresh(self, hint: str) -> str:
        self._anon += 1
        return f"anon_{cpp_name(hint)}_{self._anon}"

    def ident(self, asn_name: str) -> str:
        if self._ident_map is not None:
            return self._ident_map[asn_name]
        return self.style.value_ident(asn_name)

    def ident_subst(self, asn_name: str, self_subst: IdentSubstitutionMap) -> str:
        if self_subst and asn_name in self_subst:
            return self_subst[asn_name]
        return self.ident(asn_name)

    def emit_enum_values(self, enum_name: str, values: list) -> None:
        if enum_name in self._emitted_enum_classes:
            return
        self._emitted_enum_classes.add(enum_name)
        append_enum_class_int(self.lines, enum_name, values)

    def emit_choice_tag_enum(self, enum_name: str, members: list[dict]) -> None:
        append_choice_tag_enum(self.lines, enum_name, members)

    def apply_tag(
        self,
        expr: str,
        tag: dict[str, Any] | None,
        *,
        inner_is_choice: bool = False,
    ) -> str:
        return apply_tag(self.cc, self.style, expr, tag, inner_is_choice=inner_is_choice)

    def type_expression(
        self,
        spec: dict,
        hint: str,
        tag_override: dict | None = None,
        *,
        self_subst: IdentSubstitutionMap = None,
    ) -> str:
        return type_emit.type_expression(self, spec, hint, tag_override, self_subst=self_subst)

    def sequence_contents_body(self, spec: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
        return type_emit.sequence_contents_body(self, spec, hint, self_subst=self_subst)

    def sequence_member_expr(self, member: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
        return type_emit.sequence_member_expr(self, member, hint, self_subst=self_subst)

    def member_type_expr(self, member: dict, hint: str, *, self_subst: IdentSubstitutionMap = None) -> str:
        return type_emit.member_type_expr(self, member, hint, self_subst=self_subst)

    def implicit_application_sequence_arm(self, ref: str, arm: dict) -> bool:
        return type_emit.implicit_application_sequence_arm(self.merged, ref, arm)

    def implicit_implicit_sequence_typedef(self, asn_name: str) -> bool:
        return type_emit.implicit_implicit_sequence_typedef(self.merged, asn_name)

    def choice_expression(
        self,
        spec: dict,
        hint: str,
        *,
        outer_asn_name: str | None,
        emit_tag_enum: bool = True,
        self_subst: IdentSubstitutionMap = None,
    ) -> str:
        return choice_emit.choice_expression(
            self, spec, hint, outer_asn_name=outer_asn_name, emit_tag_enum=emit_tag_enum, self_subst=self_subst
        )

    def emit_split_recursive_choice(self, asn_name: str, spec: dict) -> None:
        choice_emit.emit_split_recursive_choice(self, asn_name, spec)

    def choice_arm_inner_for_with_enum(
        self,
        arm: dict,
        hint: str,
        *,
        self_subst: IdentSubstitutionMap = None,
    ) -> str:
        return choice_emit.choice_arm_inner_for_with_enum(self, arm, hint, self_subst=self_subst)

    def choice_arm_for_universal_discrimination(self, arm: dict, hint: str) -> str:
        return choice_emit.choice_arm_for_universal_discrimination(self, arm, hint)

    def tagged_choice_arm_dotwith(self, arm: dict, en: str, arm_label: str, inner: str) -> str:
        return choice_emit.tagged_choice_arm_dotwith(self, arm, en, arm_label, inner)

    def use_assertion_value_for_arm(self, hint: str, arm: dict) -> bool:
        return choice_emit.use_assertion_value_for_arm(self, hint, arm)

    def emit_type(self, name: str, spec: dict) -> None:
        if "parameters" in spec:
            return

        ty = spec.get("type")
        hint = asn_to_snake(name)

        if ty == "CHOICE" and self.style.split_recursive_choice and choice_emit.is_recursive_choice(spec, name):
            self.emit_split_recursive_choice(name, spec)
            return

        if ty == "ENUMERATED":
            en = self.style.enum_class_name(name)
            self.emit_enum_values(en, spec.get("values", []))
            wrapped = self.apply_tag(self.cc(f"enumerated<{en}>()"), spec.get("tag"))
            self.lines.append(f"constexpr auto {self.ident(name)} = {wrapped};")
            return

        if ty == "CHOICE":
            expr = self.apply_tag(
                self.choice_expression(spec, hint, outer_asn_name=name),
                spec.get("tag"),
                inner_is_choice=True,
            )
            self.lines.append(f"constexpr auto {self.ident(name)} = {expr};")
            return

        expr = self.type_expression(spec, hint, tag_override=spec.get("tag"))
        if ty in ("SEQUENCE", "CHOICE") and name in self._application_pdu_wrap:
            n = self._application_pdu_wrap[name]
            if ty == "SEQUENCE" and self.implicit_implicit_sequence_typedef(name):
                inner = self.sequence_contents_body(spec, hint)
            else:
                inner = expr
            expr = self.cc(f"explicit_application<{n}>({inner})")
        self.lines.append(f"constexpr auto {self.ident(name)} = {expr};")

    def emit_all(self, order: list[str]) -> str:
        self._ident_map = allocate_unique_idents(self.style, order)
        hdr = "ber.h" if self._codec == "ber" else "der.h"
        ns_parts = namespace_segments(self.namespace)
        self.lines = [
            "#pragma once",
            "",
            f"#include <libasn/{hdr}>",
            "",
            "namespace libasn {",
        ]
        for seg in ns_parts:
            self.lines.append(f"namespace {seg} {{")
        self.lines.append("")
        if self.style.substring_choice_use_assertion_value:
            self.lines.extend(
                [
                    f"constexpr auto assertion_value = {self.cc('octet_string')};",
                    "",
                ]
            )
        for name in order:
            if (
                self.style.substring_choice_use_assertion_value
                and name == "AssertionValue"
                and self.merged[name].get("type") == "OCTET STRING"
                and not self.merged[name].get("tag")
            ):
                continue
            self.emit_type(name, self.merged[name])
        self.lines.append("")
        for _ in range(len(ns_parts) + 1):
            self.lines.append("}")
        return "\n".join(self.lines)


def emit_ber_header(
    merged: dict[str, dict],
    order: list[str],
    namespace: str,
    style: EmitStyle | None = None,
) -> str:
    return BerEmitter(merged, namespace, style, codec="ber").emit_all(order)


class DerEmitter(BerEmitter):
    def __init__(
        self,
        merged: dict[str, dict],
        namespace: str,
        style: EmitStyle | None = None,
    ) -> None:
        super().__init__(merged, namespace, style, codec="der")


def emit_der_header(
    merged: dict[str, dict],
    order: list[str],
    namespace: str,
    style: EmitStyle | None = None,
) -> str:
    return DerEmitter(merged, namespace, style).emit_all(order)
