#!/usr/bin/env python3
#
#  get-version.py (for meson build)
#
# Extracts versions for build from .h header
import subprocess
import os
import sys
import shutil

if __name__ == '__main__':
    srcroot = os.path.normpath(os.path.join(os.path.dirname(__file__)))

    openaptx_lt_major = None
    openaptx_lt_minor = None
    openaptx_lt_patch = None

    with open(os.path.join(srcroot, 'openaptx.h'), 'r') as f:
        for line in f:
            if line.strip().startswith('#define OPENAPTX_MAJOR '):
                openaptx_lt_major = line[23:].strip()
            elif line.strip().startswith('#define OPENAPTX_MINOR '):
                openaptx_lt_minor = line[23:].strip()
            elif line.strip().startswith('#define OPENAPTX_PATCH '):
                openaptx_lt_patch = line[23:].strip()

    if openaptx_lt_major and openaptx_lt_minor and openaptx_lt_patch:
        print('{}.{}.{}'.format(openaptx_lt_major, openaptx_lt_minor, openaptx_lt_patch))
        sys.exit(0)
    else:
        print('ERROR: Could not extract openaptx version from openaptx.h file in', srcroot, file=sys.stderr)
        sys.exit(-1)
