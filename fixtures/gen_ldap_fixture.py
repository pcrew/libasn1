#!/usr/bin/env python3

from __future__ import annotations

import os
import sys


def ber_len(n: int) -> bytes:
    if n < 0x80:
        return bytes([n])
    bl = n.to_bytes((n.bit_length() + 7) // 8, "big")
    return bytes([0x80 | len(bl)]) + bl


def ber_octet_string(data: bytes) -> bytes:
    return bytes([0x04]) + ber_len(len(data)) + data


def ber_integer(value: int) -> bytes:
    if -128 <= value < 128:
        return bytes([0x02, 0x01, value & 0xFF])
    bl = value.to_bytes((value.bit_length() + 7) // 8, "big", signed=True)
    if bl[0] & 0x80:
        bl = b"\x00" + bl
    return bytes([0x02, len(bl)]) + bl


def ber_sequence(content: bytes) -> bytes:
    return bytes([0x30]) + ber_len(len(content)) + content


def ber_set(content: bytes) -> bytes:
    return bytes([0x31]) + ber_len(len(content)) + content


def ber_application_4(content: bytes) -> bytes:
    return bytes([0x64]) + ber_len(len(content)) + content


def partial_attribute(name: bytes, value: bytes) -> bytes:
    vals = ber_set(ber_octet_string(value))
    return ber_sequence(ber_octet_string(name) + vals)


def search_result_entry(dn: bytes, n_attrs: int, value_size: int) -> bytes:
    val = b"x" * value_size
    parts = [partial_attribute(f"attr{i:05d}".encode(), val) for i in range(n_attrs)]
    attrs = ber_sequence(b"".join(parts))
    return ber_application_4(ber_octet_string(dn) + attrs)


def ldap_message(message_id: int, protocol_op: bytes) -> bytes:
    return ber_sequence(ber_integer(message_id) + protocol_op)


def main() -> None:
    n_attrs = 4000
    value_size = 256
    out = sys.argv[1]

    body = search_result_entry(b"cn=big,dc=example,dc=com", n_attrs, value_size)
    msg = ldap_message(1, body)

    with open(out, "wb") as f:
        f.write(msg)
    print(f"Wrote {len(msg)} bytes to {out}", file=sys.stderr)


if __name__ == "__main__":
    main()
