from __future__ import annotations

import heapq
import re
from pathlib import Path
from typing import Any

BUILTIN_TYPES = frozenset(
    {
        "INTEGER",
        "ENUMERATED",
        "SEQUENCE",
        "CHOICE",
        "SEQUENCE OF",
        "SET OF",
        "BIT STRING",
        "OCTET STRING",
        "BOOLEAN",
        "NULL",
        "REAL",
        "OBJECT IDENTIFIER",
        "IA5String",
        "PrintableString",
        "UTF8String",
        "GeneralString",
        "BMPString",
        "VisibleString",
        "GraphicString",
        "UTCTime",
        "GeneralizedTime",
    }
)

CPP_RESERVED = frozenset(
    {
        "alignas",
        "alignof",
        "and",
        "and_eq",
        "asm",
        "auto",
        "bitand",
        "bitor",
        "bool",
        "break",
        "case",
        "catch",
        "char",
        "char8_t",
        "char16_t",
        "char32_t",
        "class",
        "compl",
        "concept",
        "const",
        "consteval",
        "constexpr",
        "constinit",
        "const_cast",
        "continue",
        "co_await",
        "co_return",
        "co_yield",
        "decltype",
        "default",
        "delete",
        "do",
        "double",
        "dynamic_cast",
        "else",
        "enum",
        "explicit",
        "export",
        "extern",
        "false",
        "float",
        "for",
        "friend",
        "goto",
        "if",
        "inline",
        "int",
        "long",
        "mutable",
        "namespace",
        "new",
        "noexcept",
        "not",
        "not_eq",
        "nullptr",
        "operator",
        "or",
        "or_eq",
        "private",
        "protected",
        "public",
        "register",
        "reinterpret_cast",
        "requires",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "static_assert",
        "static_cast",
        "struct",
        "switch",
        "template",
        "this",
        "thread_local",
        "throw",
        "true",
        "try",
        "typedef",
        "typeid",
        "typename",
        "union",
        "unsigned",
        "using",
        "virtual",
        "void",
        "volatile",
        "wchar_t",
        "while",
        "xor",
        "xor_eq",
    }
)


def cpp_name(name: str) -> str:
    safe = re.sub(r"[^A-Za-z0-9_]", "_", name)
    if safe and safe[0].isdigit():
        safe = "_" + safe
    return safe


_asn1tools_convert_value_before_patch = None


def _integer_named_number_or_literal(tokens, named_numbers):
    head = tokens[0]
    if named_numbers is not None and isinstance(head, str) and head in named_numbers:
        return named_numbers[head]
    return int(head)


def _install_asn1tools_parser_fixes():
    global _asn1tools_convert_value_before_patch
    import asn1tools.parser as parser

    if _asn1tools_convert_value_before_patch is not None:
        return

    _asn1tools_convert_value_before_patch = parser.convert_value

    def convert_value_fixed(tokens, type_=None, named_numbers=None):
        if type_ == "INTEGER":
            return _integer_named_number_or_literal(tokens, named_numbers)
        return _asn1tools_convert_value_before_patch(tokens, type_)

    def convert_members_fixed(member_list):
        out = []
        for row in member_list:
            if row in [["..."], "..."]:
                out.append(parser.EXTENSION_MARKER)
                continue
            if row[0] == "COMPONENTS OF":
                typedef = row[1][0]["type"]
                out.append({"components-of": typedef})
                continue
            if row[0] == "[[":
                out.append(convert_members_fixed(row[1]))
                continue
            if len(row) == 2:
                row, qualifiers = row
                qualifiers = qualifiers.asList()
            else:
                qualifiers = []
            field = parser.convert_type(row[2], [])
            field["name"] = row[0]
            if "OPTIONAL" in qualifiers:
                field["optional"] = True
            if "DEFAULT" in qualifiers:
                if len(qualifiers[1]) == 0:
                    field["default"] = []
                else:
                    field["default"] = convert_value_fixed(
                        qualifiers[1],
                        field["type"],
                        field.get("named-numbers"),
                    )
            tag_info = parser.convert_tag(row[1])
            if tag_info:
                field["tag"] = tag_info
            out.append(field)
        return out

    parser.convert_value = convert_value_fixed
    parser.convert_members = convert_members_fixed


def load_modules(paths: list[Path], encoding: str = "utf-8") -> dict[str, Any]:
    _install_asn1tools_parser_fixes()
    import asn1tools

    return asn1tools.parse_files([str(p) for p in paths], encoding=encoding)


def merge_modules(parsed: dict[str, Any]) -> dict[str, dict]:
    typedefs: dict[str, dict] = {}
    for module_body in parsed.values():
        for name, spec in module_body.get("types", {}).items():
            typedefs[name] = spec
    return typedefs


def collect_refs(syntax: object, merged: dict[str, object], refs: set[str]) -> None:
    if isinstance(syntax, dict):
        components_typedef = syntax.get("components-of")
        if isinstance(components_typedef, str) and components_typedef in merged:
            refs.add(components_typedef)
        actual_parameters = syntax.get("actual-parameters")
        if isinstance(actual_parameters, list):
            for item in actual_parameters:
                if isinstance(item, str) and item in merged:
                    refs.add(item)
        ref_name = syntax.get("type")
        if isinstance(ref_name, str) and ref_name not in BUILTIN_TYPES and ref_name in merged:
            refs.add(ref_name)
        for child in syntax.values():
            collect_refs(child, merged, refs)
    elif isinstance(syntax, list):
        for item in syntax:
            collect_refs(item, merged, refs)


def application_pdu_wrap_tags(merged: dict[str, dict]) -> dict[str, int]:
    application_tag_by_typedef: dict[str, int] = {}
    for choice_spec in merged.values():
        if choice_spec.get("type") != "CHOICE":
            continue
        for arm in choice_spec.get("members") or []:
            if not arm:
                continue
            tag = arm.get("tag") or {}
            if tag.get("class") != "APPLICATION":
                continue
            child_typedef = arm.get("type")
            if not isinstance(child_typedef, str) or child_typedef not in merged:
                continue
            tag_number = int(tag["number"])
            if child_typedef in application_tag_by_typedef:
                if application_tag_by_typedef[child_typedef] != tag_number:
                    application_tag_by_typedef[child_typedef] = -1
            else:
                application_tag_by_typedef[child_typedef] = tag_number

    def choice_arm_is_application_parent(parent_name: str, parent_spec: dict, child_typedef: str) -> bool:
        if parent_spec.get("type") != "CHOICE":
            return False
        for arm in parent_spec.get("members") or []:
            if not arm or arm.get("type") != child_typedef:
                continue
            arm_tag = arm.get("tag") or {}
            if arm_tag.get("class") == "APPLICATION":
                return True
        return False

    wrap_tag: dict[str, int] = {}
    for child_typedef, tag_number in application_tag_by_typedef.items():
        if tag_number < 0:
            continue
        unique_parent_ok = True
        for parent_name, parent_spec in merged.items():
            if parent_name == child_typedef:
                continue
            referenced = set()
            collect_refs(parent_spec, merged, referenced)
            if child_typedef not in referenced:
                continue
            if not choice_arm_is_application_parent(parent_name, parent_spec, child_typedef):
                unique_parent_ok = False
                break
        if unique_parent_ok:
            wrap_tag[child_typedef] = tag_number
    return wrap_tag


def references_from_each_typedef(merged: dict[str, dict]) -> dict[str, set[str]]:
    out: dict[str, set[str]] = {}
    for typedef_name in merged:
        refs: set[str] = set()
        collect_refs(merged[typedef_name], merged, refs)
        refs.discard(typedef_name)
        out[typedef_name] = {r for r in refs if r in merged}
    return out


def level_bundles_mutually_recursive_typedefs(
    ordered_typedef_names: list[str],
    references_from_typedef: dict[str, set[str]],
) -> list[list[str]]:
    reverse_adj: dict[str, list[str]] = {n: [] for n in ordered_typedef_names}
    for src in ordered_typedef_names:
        for dst in references_from_typedef[src]:
            reverse_adj[dst].append(src)
    visited: set[str] = set()
    finish_stack: list[str] = []

    def walk_reverse(vertex: str) -> None:
        visited.add(vertex)
        for nxt in reverse_adj[vertex]:
            if nxt not in visited:
                walk_reverse(nxt)
        finish_stack.append(vertex)

    for vertex in ordered_typedef_names:
        if vertex not in visited:
            walk_reverse(vertex)

    forward_adj: dict[str, list[str]] = {n: [] for n in ordered_typedef_names}
    for src in ordered_typedef_names:
        for dst in references_from_typedef[src]:
            forward_adj[src].append(dst)

    visited.clear()
    bundles: list[list[str]] = []

    def walk_forward_collect(vertex: str, bundle: list[str]) -> None:
        visited.add(vertex)
        bundle.append(vertex)
        for nxt in forward_adj[vertex]:
            if nxt not in visited:
                walk_forward_collect(nxt, bundle)

    for vertex in reversed(finish_stack):
        if vertex not in visited:
            bundle: list[str] = []
            walk_forward_collect(vertex, bundle)
            bundles.append(bundle)
    return bundles


def typedef_to_level_bundle_index(level_bundles: list[list[str]]) -> dict[str, int]:
    out: dict[str, int] = {}
    for level_id, names_in_level in enumerate(level_bundles):
        for name in names_in_level:
            out[name] = level_id
    return out


def prerequisite_edges_between_level_bundles(
    level_bundle_count: int,
    ordered_typedef_names: list[str],
    references_from_typedef: dict[str, set[str]],
    typedef_to_level_bundle: dict[str, int],
) -> tuple[list[set[int]], list[int]]:
    outgoing = [set() for _ in range(level_bundle_count)]
    indegree = [0] * level_bundle_count
    seen = set()
    for src in ordered_typedef_names:
        for dst in references_from_typedef[src]:
            prerequisite_level = typedef_to_level_bundle[dst]
            dependent_level = typedef_to_level_bundle[src]
            if prerequisite_level == dependent_level:
                continue
            edge = (prerequisite_level, dependent_level)
            if edge in seen:
                continue
            seen.add(edge)
            outgoing[prerequisite_level].add(dependent_level)
            indegree[dependent_level] += 1
    return outgoing, indegree


def order_level_bundles_stable(
    level_bundle_count: int,
    outgoing: list[set[int]],
    indegree: list[int],
    level_bundles: list[list[str]],
    first_seen_index: dict[str, int],
) -> list[int]:
    def stable_key(level_id: int) -> int:
        return min(first_seen_index[n] for n in level_bundles[level_id])

    remaining_indegree = list(indegree)
    heap: list[tuple[int, int]] = [
        (stable_key(i), i) for i in range(level_bundle_count) if remaining_indegree[i] == 0
    ]
    heapq.heapify(heap)
    ordered_level_ids: list[int] = []
    while heap:
        _, level_id = heapq.heappop(heap)
        ordered_level_ids.append(level_id)
        for nxt in outgoing[level_id]:
            remaining_indegree[nxt] -= 1
            if remaining_indegree[nxt] == 0:
                heapq.heappush(heap, (stable_key(nxt), nxt))
    if len(ordered_level_ids) != level_bundle_count:
        leftover = [i for i in range(level_bundle_count) if i not in ordered_level_ids]
        leftover.sort(key=stable_key)
        ordered_level_ids.extend(leftover)
    return ordered_level_ids


def typedef_names_in_level_bundle_order(
    ordered_level_ids: list[int],
    level_bundles: list[list[str]],
    first_seen_index: dict[str, int],
) -> list[str]:
    out: list[str] = []
    for level_id in ordered_level_ids:
        out.extend(sorted(level_bundles[level_id], key=lambda n: first_seen_index[n]))
    return out


def topo_sort(merged: dict[str, dict]) -> list[str]:
    first_seen_index = {name: index for index, name in enumerate(merged.keys())}
    references_from_typedef = references_from_each_typedef(merged)
    ordered_names = list(merged.keys())
    level_bundles = level_bundles_mutually_recursive_typedefs(ordered_names, references_from_typedef)
    typedef_to_level_bundle = typedef_to_level_bundle_index(level_bundles)
    outgoing, indegree = prerequisite_edges_between_level_bundles(
        len(level_bundles), ordered_names, references_from_typedef, typedef_to_level_bundle
    )
    ordered_level_ids = order_level_bundles_stable(
        len(level_bundles), outgoing, indegree, level_bundles, first_seen_index
    )
    return typedef_names_in_level_bundle_order(ordered_level_ids, level_bundles, first_seen_index)
