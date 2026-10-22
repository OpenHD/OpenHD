#!/usr/bin/env python3
"""Bridge between the top-level Kconfig and CMake.

  kconfig.py defconfig <defconfig>   load a configs/*_defconfig into .config
  kconfig.py cmake <out.cmake>       write a CMake initial-cache file (cmake -C)
  kconfig.py get <SYMBOL>            print a symbol's value (used by the Makefile)

Symbols are exported under their own name, so ENABLE_AIR in Kconfig sets the
ENABLE_AIR cache variable in CMake. MAKE_* symbols are Makefile-only.
"""

import argparse
import os
import sys

try:
    import kconfiglib
except ImportError:
    sys.exit("kconfiglib is missing; run 'make setup' or 'pip install -r requirements.txt'")

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BUILD_TYPES = {
    "MAKE_BUILD_TYPE_RELEASE": "Release",
    "MAKE_BUILD_TYPE_RELWITHDEBINFO": "RelWithDebInfo",
    "MAKE_BUILD_TYPE_DEBUG": "Debug",
}


def load(config):
    os.environ.setdefault("srctree", REPO_ROOT)
    kconf = kconfiglib.Kconfig(os.path.join(REPO_ROOT, "Kconfig"), warn_to_stderr=True)
    if os.path.exists(config):
        kconf.load_config(config)
    return kconf


def cmake_value(sym):
    if sym.type in (kconfiglib.BOOL, kconfiglib.TRISTATE):
        return "BOOL", "ON" if sym.str_value == "y" else "OFF"
    if sym.type in (kconfiglib.INT, kconfiglib.HEX):
        return "STRING", sym.str_value
    return "STRING", sym.str_value.replace("\\", "\\\\").replace('"', '\\"')


def write_cmake(kconf, out):
    lines = ["# Generated from .config by scripts/kconfig.py - do not edit.\n"]
    for sym in kconf.unique_defined_syms:
        if sym.name.startswith("MAKE_") or sym.type == kconfiglib.UNKNOWN:
            continue
        kind, value = cmake_value(sym)
        lines.append(f'set({sym.name} "{value}" CACHE {kind} "" FORCE)\n')
    for name, build_type in BUILD_TYPES.items():
        if kconf.syms[name].str_value == "y":
            lines.append(f'set(CMAKE_BUILD_TYPE "{build_type}" CACHE STRING "" FORCE)\n')
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    with open(out, "w") as f:
        f.writelines(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--config", default=os.path.join(REPO_ROOT, ".config"))
    sub = parser.add_subparsers(dest="cmd", required=True)
    sub.add_parser("defconfig").add_argument("defconfig")
    sub.add_parser("cmake").add_argument("out")
    sub.add_parser("get").add_argument("symbol")
    args = parser.parse_args()

    if args.cmd == "defconfig":
        if not os.path.exists(args.defconfig):
            sys.exit(f"{args.defconfig} not found")
        kconf = load(args.defconfig)
        print(kconf.write_config(args.config))
    elif args.cmd == "cmake":
        kconf = load(args.config)
        # Normalise .config so options added to Kconfig later show up in it.
        kconf.write_config(args.config)
        write_cmake(kconf, args.out)
    else:
        kconf = load(args.config)
        if args.symbol not in kconf.syms:
            sys.exit(f"unknown symbol {args.symbol}")
        print(kconf.syms[args.symbol].str_value)


if __name__ == "__main__":
    main()
