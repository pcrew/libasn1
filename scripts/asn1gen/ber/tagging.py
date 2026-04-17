from __future__ import annotations

from typing import Any, Callable

from .compound import compound_typedef_arm
from .support import AllTypedefs


def apply_tag(
    cc: Callable[[str], str],
    style: Any,
    expr: str,
    tag: dict[str, Any] | None,
    *,
    inner_is_choice: bool = False,
) -> str:
    if not tag:
        return expr
    
    n = tag["number"]
    kind = tag.get("kind", "IMPLICIT")
    cls = tag.get("class", "CONTEXT_SPECIFIC")
    
    if cls in (None, "CONTEXT"):
        cls = "CONTEXT_SPECIFIC"

    if cls == "CONTEXT_SPECIFIC":
        if kind == "EXPLICIT" or inner_is_choice or style.context_specific_prefer_explicit:
            return cc(f"explicit_context_specific<{n}>({expr})")
        return f"({expr}).template context_specific<{n}>()"
   
    if cls == "APPLICATION":
        if kind == "EXPLICIT" or inner_is_choice or style.application_prefer_explicit:
            return cc(f"explicit_application<{n}>({expr})")
        return f"({expr}).template application<{n}>()"
    
    return f"({expr})"


def arm_use_explicit_wrap(
    merged: AllTypedefs,
    application_pdu_wrap: dict[str, int],
    m: dict,
) -> bool:
    if not compound_typedef_arm(merged, m):
        return False
    
    tg = m.get("tag") or {}
    if tg.get("number") is None:
        return False
    
    ref = m.get("type")
    if (
        isinstance(ref, str)
        and ref in application_pdu_wrap
        and tg.get("class") == "APPLICATION"
        and int(tg["number"]) == application_pdu_wrap[ref]
    ):
        return False
    return True


def arm_use_plain_with(application_pdu_wrap: dict[str, int], m: dict) -> bool:
    ref = m.get("type")
    return isinstance(ref, str) and ref in application_pdu_wrap


def wrap_tagged_compound_arm(cc: Callable[[str], str], m: dict, inner: str) -> str:
    tg = m.get("tag") or {}
    n = int(tg["number"])
    cls = tg.get("class", "CONTEXT_SPECIFIC")
    if cls == "APPLICATION":
        return cc(f"explicit_application<{n}>({inner})")
    return cc(f"explicit_context_specific<{n}>({inner})")
