from __future__ import annotations

import json
from dataclasses import fields, replace
from pathlib import Path
from typing import Any

from .style import EmitStyle

PROFILE_NAME = "asn1gen.json"
_STYLE_KEYS = frozenset(f.name for f in fields(EmitStyle))

_default_style_cache: EmitStyle | None = None


def find_profile_file(directory: Path) -> Path | None:
    candidate = directory / PROFILE_NAME
    return candidate if candidate.is_file() else None


def _read_json(path: Path) -> dict[str, Any]:
    if path.suffix.lower() != ".json":
        raise ValueError(f"{path}: expected .json")
    root = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(root, dict):
        raise ValueError(f"{path}: root must be a JSON object")
    return root


def _split_emit_style_fields(raw: dict[str, Any]) -> tuple[dict[str, Any], dict[str, Any]]:
    style_overrides = {key: raw[key] for key in raw if key in _STYLE_KEYS}
    other_keys = {key: value for key, value in raw.items() if key not in _STYLE_KEYS}
    return style_overrides, other_keys


def _merge_json_onto_style(base_style: EmitStyle, raw: dict[str, Any]) -> tuple[dict[str, Any], EmitStyle]:
    style_overrides, other_keys = _split_emit_style_fields(raw)
    updated_style = replace(base_style, **style_overrides) if style_overrides else base_style
    return other_keys, updated_style


def emit_style_default() -> EmitStyle:
    global _default_style_cache
    if _default_style_cache is None:
        _default_style_cache = load_emit_profile(None)[1]
    return _default_style_cache


def load_emit_profile(profile_file: Path | None) -> tuple[dict[str, Any], EmitStyle]:
    if profile_file is None:
        return {}, EmitStyle()
    return _merge_json_onto_style(EmitStyle(), _read_json(profile_file))


def resolve_backend(cli_backend: str | None, extras: dict[str, Any]) -> str:
    if cli_backend is not None:
        return cli_backend
    from_json = extras.get("backend")
    if from_json in ("ber", "der"):
        return from_json
    if from_json is not None:
        raise ValueError(f"profile backend must be 'ber' or 'der', got {from_json!r}")
    return "ber"
