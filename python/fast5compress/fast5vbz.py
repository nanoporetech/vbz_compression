#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Fast5 compression filter for demonstrating vbz compression
"""

import os
import sys
import select
import argparse
from multiprocessing import Pool

import h5py
import numpy as np


__version__ = "0.1.1"


def compress_fast5(filename, decompress=False):
    """
    (De)compress the raw signal in the fast5
    """
    with h5py.File(filename, 'r+') as fast5:

        compression_opts = {
            "compression": 32020,
            "compression_opts": (0, 2, 1, 0) # 2 byte integers with zig zag + level 1 zstd
        }

        if decompress:
            compression_opts = { "compression": "gzip", "compression_opts": 1 }

        # multi read files
        for read in fast5:
            if not read.startswith("read_"):
                continue


            group = '%s/Raw/' % read
            print(fast5[group].keys())
            raw = fast5[group]["Signal"][:]

            del fast5[group]["Signal"]
            fast5[group].create_dataset(
                "Signal",
                data=raw,
                **compression_opts,
                chunks=(len(raw),)
            )
            
            data = fast5[group]["Signal"][:]
            print(raw[0:10])
            print(data[0:10])

            with open("test.c", "w") as file:
                for r in raw:
                    file.write("%s, " % r)

        return filename


def on_success(filename):
    """
    If the file was compressed print it to stdout for repacking
    """
    if filename:
        print(filename)


def on_error(error):
    """
    Log any errors to stderr
    """
    sys.stderr.write("%s\n" % error)


def main(args, print_help):

    # if we no files are given on stdin print the help message and quit
    if not (select.select([sys.stdin], [], [], 0.0)[0] or args.files):
        print_help()
        exit(1)

    with Pool(args.threads) as pool:
        for filename in args.files:
            if filename.endswith(".fast5") and os.path.isfile(filename):
                pool.apply_async(
                    compress_fast5,
                    args=(filename, args.decompress),
                    callback=on_success,
                    error_callback=on_error,
                )
        pool.close()
        pool.join()


if __name__ == "__main__":
    parser = argparse.ArgumentParser("fast5compress")
    parser.add_argument('files', nargs='*', help='input files')
    parser.add_argument("-d", "--decompress", action="store_true", default=False)
    parser.add_argument('-t', "--threads", metavar="threads", type=int, default=1, required=False)
    parser.add_argument('-v', '--version', action='version', version=__version__)
    main(parser.parse_args(), parser.print_help)
