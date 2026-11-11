#!/usr/bin/env python3
"""Validate every machines/*.json CPU config against machines/cpu_schema.json (structure only;
ptasm checks cross-references when it loads a file).

Exits 77 (ctest's skip code here) when the jsonschema module is missing.
"""
import json
import pathlib
import sys

try:
    import jsonschema
except ImportError:
    print("jsonschema not installed -- skipping cpu.json schema check")
    sys.exit(77)

MACHINES = pathlib.Path(__file__).resolve().parent.parent / "machines"
SCHEMA = MACHINES / "cpu_schema.json"

validator = jsonschema.Draft202012Validator(json.loads(SCHEMA.read_text()))
failed = False
for path in sorted(MACHINES.glob("*.json")):
    if path == SCHEMA:
        continue
    for error in validator.iter_errors(json.loads(path.read_text())):
        failed = True
        location = "/".join(str(part) for part in error.absolute_path) or "(top level)"
        print(f"{path.name}: {location}: {error.message}")
sys.exit(1 if failed else 0)
