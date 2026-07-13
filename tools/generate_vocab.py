#!/usr/bin/env python3
"""Generate src/c/vocab.h from resources/vocab_db.json.

The JSON file is the single source of truth for the vocabulary database.
Run this script (or build.sh, which calls it) after editing the JSON.

Entry format: {"t": term, "m": meaning, "x": example, "e": etymology,
               "p": phonetic, "d": difficulty 1-3, "c": category}
"""
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "resources" / "vocab_db.json"
DST = ROOT / "src" / "c" / "vocab.h"

REQUIRED = ("t", "m", "x", "e", "p", "d", "c")

# The original Pebble (aplite) has a 24KB app region; the full word list's
# const data does not fit. Aplite gets the first N words (which include the
# complete v1 list, preserving migration), all other platforms get everything.
APLITE_WORD_LIMIT = 50


def c_escape(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def main() -> int:
    entries = json.loads(SRC.read_text(encoding="utf-8"))
    if not isinstance(entries, list) or not entries:
        print("error: vocab_db.json must be a non-empty array", file=sys.stderr)
        return 1

    seen = set()
    for i, e in enumerate(entries):
        for key in REQUIRED:
            if key not in e:
                print(f"error: entry {i} missing key '{key}'", file=sys.stderr)
                return 1
        if e["d"] not in (1, 2, 3):
            print(f"error: entry {i} ('{e['t']}') difficulty must be 1-3", file=sys.stderr)
            return 1
        term = e["t"].lower()
        if term in seen and term not in ("eloquence",):  # 'Eloquent'/'Eloquence' kept from v1
            print(f"error: duplicate term '{e['t']}' at entry {i}", file=sys.stderr)
            return 1
        seen.add(term)
        for key in ("m", "x"):
            if len(e[key]) > 90:
                print(f"warning: entry {i} ('{e['t']}') '{key}' is long "
                      f"({len(e[key])} chars); may clip on small screens", file=sys.stderr)

    lines = [
        "#ifndef VOCAB_H",
        "#define VOCAB_H",
        "",
        "// AUTO-GENERATED FILE - DO NOT EDIT BY HAND.",
        "// Source of truth: resources/vocab_db.json",
        "// Regenerate with: python3 tools/generate_vocab.py",
        "",
        "typedef struct {",
        "  const char *term;",
        "  const char *meaning;",
        "  const char *example;",
        "  const char *etymology;",
        "  const char *phonetic;",
        "  int difficulty;  // 1=basic, 2=intermediate, 3=advanced",
        "  const char *category;",
        "} VocabEntry;",
        "",
        "static const VocabEntry vocab_list[] = {",
    ]
    for i, e in enumerate(entries):
        if i == APLITE_WORD_LIMIT:
            lines.append("#ifndef PBL_PLATFORM_APLITE  // trimmed list for aplite's 24KB app region")
        lines.append(
            '  {"%s", "%s", "%s", "%s", "%s", %d, "%s"},'
            % (c_escape(e["t"]), c_escape(e["m"]), c_escape(e["x"]),
               c_escape(e["e"]), c_escape(e["p"]), e["d"], c_escape(e["c"]))
        )
    if len(entries) > APLITE_WORD_LIMIT:
        lines.append("#endif")
    lines += [
        "};",
        "",
        "#define VOCAB_COUNT (sizeof(vocab_list) / sizeof(vocab_list[0]))",
        "",
        "#endif",
        "",
    ]
    DST.write_text("\n".join(lines), encoding="utf-8")
    print(f"Generated {DST.relative_to(ROOT)} with {len(entries)} entries.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
