#!/usr/bin/env python3
"""Generate the binary vocabulary resource from resources/vocab_db.json.

The JSON file is the single source of truth for the vocabulary database.
Run this script (or build.sh, which calls it) after editing the JSON.

Since v2.1 the words are packed into a raw resource (vocab_db.bin) that the
watch app reads on demand with resource_load_byte_range(). Resources live in
the 256KB resource bank and do NOT count against the app's RAM region, which
is what allows 1,500+ words (the v2.0 approach compiled words into the
binary, which is copied into RAM and capped the list at a few hundred).

Binary layout (all little-endian):
  header:  magic "VDB2" (4 bytes) | count uint16 | max_record uint16
  offsets: (count+1) x uint32   -- record start offsets relative to data
                                   section; entry [count] is the total size
  diffs:   count x uint8        -- difficulty per word (1-3)
  data:    per record, 6 NUL-terminated strings:
           term, meaning, example, etymology, phonetic, category

Entry format in JSON: {"t": term, "m": meaning, "x": example,
  "e": etymology, "p": phonetic, "d": difficulty 1-3, "c": category}
"""
import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "resources" / "vocab_db.json"
DST = ROOT / "resources" / "data" / "vocab_db.bin"
DST_APLITE = ROOT / "resources" / "data" / "vocab_db~aplite.bin"

REQUIRED = ("t", "m", "x", "e", "p", "d", "c")
MAGIC = b"VDB2"
MAX_RECORD = 500  # must stay below the C-side read buffer (512)

# The original Pebble (aplite) has a smaller resource bank; it gets the
# first N words (which include the complete v1 list, preserving migration).
# All other platforms get the full list. Must not exceed MAX_VOCAB in
# src/c/vocab.h for the respective platform.
APLITE_WORD_LIMIT = 400


def validate(entries):
    if not isinstance(entries, list) or not entries:
        sys.exit("error: vocab_db.json must be a non-empty array")
    seen = set()
    for i, e in enumerate(entries):
        for key in REQUIRED:
            if key not in e:
                sys.exit(f"error: entry {i} missing key '{key}'")
        if e["d"] not in (1, 2, 3):
            sys.exit(f"error: entry {i} ('{e['t']}') difficulty must be 1-3")
        term = e["t"].lower()
        if term in seen and term not in ("eloquence",):  # kept from v1
            sys.exit(f"error: duplicate term '{e['t']}' at entry {i}")
        seen.add(term)
        for key in ("m", "x"):
            if len(e[key]) > 90:
                print(f"warning: entry {i} ('{e['t']}') '{key}' is long "
                      f"({len(e[key])} chars); may clip on small screens",
                      file=sys.stderr)


def pack(entries) -> bytes:
    records = []
    for i, e in enumerate(entries):
        rec = b"\x00".join(
            e[k].encode("utf-8") for k in ("t", "m", "x", "e", "p", "c")
        ) + b"\x00"
        if len(rec) > MAX_RECORD:
            sys.exit(f"error: entry {i} ('{e['t']}') record is {len(rec)} "
                     f"bytes (max {MAX_RECORD})")
        records.append(rec)

    offsets = []
    pos = 0
    for rec in records:
        offsets.append(pos)
        pos += len(rec)
    offsets.append(pos)  # end sentinel = total data size

    out = bytearray()
    out += MAGIC
    out += struct.pack("<HH", len(entries), max(len(r) for r in records))
    out += struct.pack(f"<{len(offsets)}I", *offsets)
    out += bytes(e["d"] for e in entries)
    for rec in records:
        out += rec
    return bytes(out)


def main() -> int:
    entries = json.loads(SRC.read_text(encoding="utf-8"))
    validate(entries)

    DST.parent.mkdir(parents=True, exist_ok=True)
    blob = pack(entries)
    DST.write_bytes(blob)
    print(f"Generated {DST.relative_to(ROOT)}: {len(entries)} words, "
          f"{len(blob)} bytes (resource bank limit 262144)")
    if len(blob) > 262144:
        sys.exit("error: blob exceeds the 256KB resource bank")

    if len(entries) > APLITE_WORD_LIMIT:
        aplite_blob = pack(entries[:APLITE_WORD_LIMIT])
        DST_APLITE.write_bytes(aplite_blob)
        print(f"Generated {DST_APLITE.relative_to(ROOT)}: "
              f"{APLITE_WORD_LIMIT} words, {len(aplite_blob)} bytes")
    elif DST_APLITE.exists():
        DST_APLITE.unlink()
        print(f"Removed stale {DST_APLITE.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
