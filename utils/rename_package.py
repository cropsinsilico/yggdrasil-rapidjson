import os
import glob
import argparse


def process_file(fname, sequence):
    with open(fname, 'r') as fd:
        contents = fd.read()
    for a, b in sequence:
        contents = contents.replace(a, b)
    with open(fname, 'w') as fd:
        fd.write(contents)


def process_directory(fdir, ext=None, new='yggdrasil_rapidjson',
                      old='rapidjson',
                      reverse_overall_macro='RAPIDJSON_YGGDRASIL'):
    if ext is None:
        ext = '.*'
    files = sorted(glob.glob(os.path.join(fdir, f'*{ext}')))
    new_standin = new.replace(old, 'rj')
    sequence = []
    if reverse_overall_macro:
        sequence += [
            (f'#ifdef {reverse_overall_macro}',
             f'#ifndef DISABLE_{new.upper()}'),
            (f'#ifndef {reverse_overall_macro}',
             f'#ifdef DISABLE_{new.upper()}'),
            (reverse_overall_macro, f'DISABLE_{new.upper()}'),
        ]
    sequence += [
        # Protect existing uses of new packages
        (new, new_standin),
        (new.upper(), new_standin.upper()),
        # Macros
        (old.upper(), new_standin.upper()),
        # Namespace
        (f'using namespace {old}', f'using namespace {new_standin}'),
        (f'{old}::', f'{new_standin}::'),
        # Includes
        (f'#include "{old}', f'#include "{new}'),
        (f'#include <{old}', f'#include <{new}'),
        # Restore existing uses of new packages
        (new_standin, new),
        (new_standin.upper(), new.upper()),
    ]
    for fname in files:
        print(f"PROCESSING \"{fname}\"...")
        process_file(fname, sequence)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        "Change the package name in macros & namespace")
    parser.add_argument(
        "directory",
        help="Path to directory containing files to change.")
    parser.add_argument(
        "--ext", help="Extension of files to process")
    parser.add_argument(
        "--old", default='rapidjson',
        help='Old package name')
    parser.add_argument(
        "--new", default='yggdrasil_rapidjson',
        help='New package name')
    args = parser.parse_args()
    process_directory(args.directory, ext=args.ext,
                      new=args.new, old=args.old)
    # include/rapidjson/ --ext=.h
    # include/rapidjson/internal --ext=.h
    # include/rapidjson/error --ext=.h
    # test/unittest/ --ext=.h
    # test/unittest/ --ext=.cpp
    # test/perftest/ --ext=.h
    # test/perftest/ --ext=.cpp
