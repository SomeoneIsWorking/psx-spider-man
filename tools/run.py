#!/usr/bin/env python3
"""Build and launch the default Spider-Man native PC port target."""

from __future__ import annotations

import hashlib
import os
import runpy
import subprocess
import sys
import tempfile
from collections.abc import Mapping, Sequence
from pathlib import Path

from disc_path import DiscPathError, resolve_disc, resolve_unidentified_disc
from launcher_dependencies import (
    DependencyError,
    Requirement,
    install_instructions,
    platform_family,
    require_native_dependencies,
)
from provision import ProvisionError, provision_executable
from title_catalog import Title, TitleCatalogError, load_catalog

ROOT = Path(__file__).resolve().parents[1]
# The one product: both titles, picked in its window.
PRODUCT_TARGET = "spiderman_port"
FRAMEWORK_FETCH = ["uv", "run", "--no-project", "https://raw.githubusercontent.com/SomeoneIsWorking/psxport/main/tools/psxport_fetch.py", "--auto"]
CYAN = "\033[1;36m"
RED = "\033[1;31m"
RESET = "\033[0m"
USAGE = """Usage: ./run.sh [--prepare-only] [disc.chd]

Build and launch the Spider-Man product after provisioning the title on a user-supplied disc.

Options:
  --prepare-only  Provision and build without launching.
  -h, --help      Show this help and exit.
"""


class LauncherError(RuntimeError):
    """A user-actionable launcher refusal."""


def say(message: str) -> None:
    print(f"{CYAN}[run]{RESET} {message}", flush=True)


def refuse(message: str) -> int:
    print(f"{RED}[run] error:{RESET} {message}", file=sys.stderr, flush=True)
    return 1


def run(
    command: Sequence[str],
    *,
    cwd: Path = ROOT,
    env: Mapping[str, str] | None = None,
    quiet: bool = False,
) -> None:
    result = subprocess.run(
        list(command),
        cwd=cwd,
        env=dict(env) if env is not None else None,
        stdout=subprocess.DEVNULL if quiet else None,
        check=False,
    )
    if result.returncode:
        raise LauncherError("")


def run_or_refuse(
    command: Sequence[str],
    failure: str,
    *,
    cwd: Path = ROOT,
    env: Mapping[str, str] | None = None,
    quiet: bool = False,
) -> None:
    try:
        run(command, cwd=cwd, env=env, quiet=quiet)
    except LauncherError as exc:
        raise LauncherError(failure) from exc


def command_output(command: Sequence[str]) -> str | None:
    try:
        result = subprocess.run(
            list(command), cwd=ROOT, text=True, capture_output=True, check=False
        )
    except OSError:
        return None
    return result.stdout.strip() if result.returncode == 0 else None


def cpu_count() -> int:
    return os.cpu_count() or 4


def framework_revision(framework: Path) -> tuple[str, bool]:
    revision = command_output(
        ["git", "-C", str(framework), "rev-parse", "--short", "HEAD"]
    )
    status = command_output(["git", "-C", str(framework), "status", "--porcelain"])
    return revision or "?", bool(status)


def announce_framework(framework_text: str, framework: Path) -> None:
    revision, dirty = framework_revision(framework)
    suffix = " +dirty" if dirty else ""
    if framework_text == "external/psxport":
        say(
            f"framework: external/psxport -> {framework.resolve()} @ {revision}{suffix}"
        )
        return
    say(
        f"framework: *** {framework_text} *** (DEV CLONE {revision}{suffix}) — NOT the recorded pin"
    )


def submodule_sync_invocation(framework: Path, python: str) -> tuple[list[str], Path]:
    """Return psxport's poison-gitlink-aware dependency sync command."""
    framework_root = framework.resolve()
    return [python, str(framework_root / "scripts" / "sync_submodules.py")], framework_root


def sync_submodules(framework: Path, python: str) -> None:
    command, framework_root = submodule_sync_invocation(framework, python)
    synchronizer = Path(command[1])
    if not synchronizer.is_file():
        raise LauncherError(f"psxport dependency synchronizer is missing: {synchronizer}")
    run_or_refuse(command, "submodule sync failed", cwd=framework_root)


def player_build_root(cc: str, cxx: str) -> Path:
    """Return an isolated build root without classifying compiler identity."""
    # Build-dir key; unchanged so existing player trees stay valid.
    contract = f"{cc}\0{cxx}\0BUILD_TESTING=OFF\0PSXPORT_BUILD_TESTS=OFF"
    key = hashlib.sha256(contract.encode()).hexdigest()[:12]
    return Path("build/player") / key


def discdump_commands(
    framework: Path,
    build: Path,
    cc: str,
    cxx: str,
    python: str,
    jobs: int,
) -> list[list[str]]:
    return [
        [
            "cmake",
            "-S",
            str(framework),
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DCMAKE_C_COMPILER={cc}",
            f"-DCMAKE_CXX_COMPILER={cxx}",
            f"-DPython3_EXECUTABLE={python}",
        ],
        ["cmake", "--build", str(build), "-j", str(jobs), "--target", "discdump"],
    ]


def port_commands(
    framework: Path,
    build: Path,
    cc: str,
    cxx: str,
    python: str,
    jobs: int,
) -> list[list[str]]:
    return [
        [
            "cmake",
            "-S",
            ".",
            "-B",
            str(build),
            "-DCMAKE_BUILD_TYPE=Release",
            f"-DPSXPORT_DIR={framework.resolve()}",
            f"-DCMAKE_C_COMPILER={cc}",
            f"-DCMAKE_CXX_COMPILER={cxx}",
            f"-DPython3_EXECUTABLE={python}",
        ],
        ["cmake", "--build", str(build), "-j", str(jobs), "--target", PRODUCT_TARGET],
    ]


def launch_environment(
    environ: Mapping[str, str], framework_text: str, disc: str, title: Title
) -> dict[str, str]:
    framework = Path(framework_text)
    if not framework.is_absolute():
        framework = ROOT / framework
    policy = runpy.run_path(str(framework / "tools/port/launch_environment.py"))
    result = policy["player_environment"](environ, product=PRODUCT_TARGET)
    if not result.get("PSXPORT_ASSET_DIR"):
        result["PSXPORT_ASSET_DIR"] = framework_text
    result[title.disc_env] = disc
    return result


def detect_title(discdump: Path, disc: str) -> Title:
    scratch = ROOT / "scratch"
    scratch.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(
        prefix="launcher-title-", dir=scratch
    ) as directory:
        result = subprocess.run(
            [str(discdump), "get", "SYSTEM.CNF", disc, directory],
            cwd=ROOT,
            text=True,
            capture_output=True,
            check=False,
        )
        cnf = Path(directory) / "SYSTEM.CNF"
        if result.returncode or not cnf.is_file():
            raise LauncherError(
                f"could not inspect SYSTEM.CNF: {result.stderr.strip()}"
            )
        try:
            return load_catalog(ROOT).from_system_cnf(
                cnf.read_text(encoding="ascii", errors="replace")
            )
        except TitleCatalogError as exc:
            raise LauncherError(str(exc)) from exc




def parse_launch_arguments(argv: Sequence[str]) -> tuple[bool, str | None]:
    arguments = list(argv)
    prepare_only = False
    if "--prepare-only" in arguments:
        prepare_only = True
        arguments.remove("--prepare-only")
    if len(arguments) > 1 or "--prepare-only" in arguments:
        raise LauncherError("usage: ./run.sh [--prepare-only] [disc.chd]")
    if arguments and arguments[0].startswith("-"):
        raise LauncherError(f"unknown launcher option: {arguments[0]}")
    return prepare_only, arguments[0] if arguments else None


def launch(argv: Sequence[str]) -> int:
    os.chdir(ROOT)
    prepare_only, argument = parse_launch_arguments(argv)
    try:
        native = require_native_dependencies()
    except DependencyError as exc:
        raise LauncherError(str(exc)) from exc
    python = sys.executable
    cc = native.cc
    cxx = native.cxx
    jobs = cpu_count()
    build_root = player_build_root(cc, cxx)

    framework_text = os.environ.get("PSXPORT_DIR") or "external/psxport"
    if not os.environ.get("PSXPORT_DIR") and not Path("external/psxport/cmake/psxport.cmake").is_file():
        run_or_refuse(FRAMEWORK_FETCH, "could not resolve external/psxport")
    framework = Path(framework_text)
    if not (framework / "cmake/psxport.cmake").is_file():
        raise LauncherError(f"PSXPORT_DIR={framework_text} is not a psxport checkout")
    announce_framework(framework_text, framework)
    sync_submodules(framework, python)

    catalog = load_catalog(ROOT)
    try:
        disc = resolve_unidentified_disc(
            ROOT,
            argument,
            os.environ,
            [catalog.by_id(title_id).disc_env for title_id in catalog.ids()],
        )
    except DiscPathError as exc:
        raise LauncherError(str(exc)) from exc
    if not disc or not Path(disc).is_file():
        raise LauncherError(
            "no disc image — pass it as ./run.sh <disc.chd>, set a title-specific disc "
            "environment variable, or drop one *.chd here"
        )
    say(f"disc: {disc}")

    say("building libchdr + discdump…")
    framework_build = build_root / "framework"
    configure_discdump, build_discdump = discdump_commands(
        framework, framework_build, cc, cxx, python, jobs
    )
    run_or_refuse(configure_discdump, "psxport cmake configure failed", quiet=True)
    run_or_refuse(build_discdump, "discdump build failed", quiet=True)
    discdump = framework_build / "tools/discdump"
    if not os.access(discdump, os.X_OK):
        discdump = framework_build / "tools/discdump.exe"
    if not os.access(discdump, os.X_OK):
        raise LauncherError("discdump build failed")

    title = detect_title(discdump, disc)
    say(f"title: {title.label} ({title.serial})")

    try:
        provision_executable(ROOT, discdump, Path(disc), title)
    except ProvisionError as exc:
        raise LauncherError(str(exc)) from exc

    say(f"building the native port (CMake -j{jobs})…")
    port_build = build_root / "product"
    configure_port, build_port = port_commands(
        framework, port_build, cc, cxx, python, jobs
    )
    run_or_refuse(configure_port, "cmake configure failed", quiet=True)
    run_or_refuse(build_port, "port build failed")

    port_executable = port_build / "bin" / PRODUCT_TARGET
    if prepare_only:
        say(f"{title.label} is provisioned; the product is built at {port_executable}")
        return 0

    say("launching Spider-Man (native PC port)…")
    launch_env = launch_environment(os.environ, framework_text, disc, title)
    # No argument: the product picks among the provisioned titles in its window.
    os.execvpe(str(port_executable), [str(port_executable)], launch_env)
    return 0


def main(argv: Sequence[str] | None = None) -> int:
    arguments = list(sys.argv[1:] if argv is None else argv)
    if arguments in (["-h"], ["--help"]):
        print(USAGE, end="")
        return 0
    try:
        return launch(arguments)
    except LauncherError as exc:
        return refuse(str(exc))
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
