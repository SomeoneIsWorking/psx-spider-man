#!/usr/bin/env python3
"""Resolve the shared scanner and apply Spider's source-boundary manifest."""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = Path(__file__).with_name("source_policy.json")
sys.path.insert(0, str(ROOT / "external" / "psxport" / "tools"))
from policy import harness  # noqa: E402  (the framework's re-harness locator)


if __name__ == "__main__":
    raise SystemExit(harness.require("source_boundary").main(ROOT, MANIFEST))
