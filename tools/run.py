#!/usr/bin/env python3
"""Provision the Spider-Man title on a user-supplied disc, build spiderman_port, and launch it."""

from __future__ import annotations

import os
import subprocess
import tempfile
from collections.abc import Sequence
from pathlib import Path

from automation.process import ToolError
from disc_path import DiscPathError, resolve_unidentified_disc
from port import launcher
from port.launch_environment import player_environment
from provision import ProvisionError, provision_executable
from title_catalog import Title, TitleCatalogError, load_catalog

ROOT = Path(__file__).resolve().parents[1]
# The one product: both titles, picked in its window.
PRODUCT_TARGET = "spiderman_port"


def detect_title(discdump: Path, disc: str) -> Title:
    """The catalog title whose SYSTEM.CNF the disc carries."""
    scratch = ROOT / "scratch"
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="launcher-title-", dir=scratch) as directory:
        result = subprocess.run([str(discdump), "get", "SYSTEM.CNF", disc, directory], cwd=ROOT, text=True,
                                capture_output=True, check=False)
        cnf = Path(directory) / "SYSTEM.CNF"
        if result.returncode or not cnf.is_file():
            raise ToolError(f"could not inspect SYSTEM.CNF: {result.stderr.strip()}")
        try:
            return load_catalog(ROOT).from_system_cnf(cnf.read_text(encoding="ascii", errors="replace"))
        except TitleCatalogError as error:
            raise ToolError(str(error)) from error


def launch(argument: str | None, prepare_only: bool) -> None:
    launcher.preflight()
    catalog = load_catalog(ROOT)
    try:
        disc = resolve_unidentified_disc(ROOT, argument, os.environ,
                                         [catalog.by_id(title_id).disc_env for title_id in catalog.ids()])
    except DiscPathError as error:
        raise ToolError(str(error)) from error
    if not disc or not Path(disc).is_file():
        raise ToolError("no disc image: pass it as ./run.sh <disc.chd>, set a title-specific disc "
                        "environment variable, or drop one *.chd here")
    launcher.say(f"disc: {disc}")
    discdump = launcher.build_discdump(ROOT, os.environ)
    title = detect_title(discdump, disc)
    launcher.say(f"title: {title.label} ({title.serial})")
    try:
        provision_executable(ROOT, discdump, Path(disc), title)
    except ProvisionError as error:
        raise ToolError(str(error)) from error
    build = launcher.build_product(ROOT, PRODUCT_TARGET, os.environ)
    if prepare_only:
        launcher.say(f"{title.label} is provisioned and {PRODUCT_TARGET} is built")
        return
    env = player_environment(os.environ, product=PRODUCT_TARGET)
    env[title.disc_env] = disc
    # No argument: the product picks among the provisioned titles in its window.
    launcher.exec_product(build / "bin" / PRODUCT_TARGET, [], env, ROOT)


def main(argv: Sequence[str] | None = None) -> int:
    args = launcher.argument_parser(__doc__).parse_args(argv)
    return launcher.main(lambda: launch(args.disc, args.prepare_only))


if __name__ == "__main__":
    raise SystemExit(main())
