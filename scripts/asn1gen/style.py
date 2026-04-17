from __future__ import annotations

import re
from dataclasses import dataclass


def asn_to_snake(name: str) -> str:
    if not name:
        return name
    name = name.replace("-", "_")
    name = re.sub(r"_+", "_", name)
    name = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", name)
    name = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", name)
    return name.lower()


def cpp_enum_member(name: str) -> str:
    return asn_to_snake(name).upper()


@dataclass(frozen=True)
class EmitStyle:
    namespace_default: str = "asn1_generated"
    constexpr_prefix: str = ""
    split_recursive_choice: bool = True
    recursive_choice_inner_dunder_name: bool = True
    recursive_choice_opaque_self_payload: bool = False
    wrap_self_choice_in_explicit: bool = False
    substring_choice_use_assertion_value: bool = False
    context_specific_prefer_explicit: bool = False
    application_prefer_explicit: bool = False

    def value_ident(self, asn_name: str) -> str:
        base = asn_to_snake(asn_name)
        if base and base[0].isdigit():
            base = "_" + base
        prefix = self.constexpr_prefix
        return f"{prefix}{base}" if prefix else base

    def enum_class_name(self, asn_type_name: str) -> str:
        return f"{asn_to_snake(asn_type_name)}_enum"
