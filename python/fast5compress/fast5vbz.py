#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Fast5 compression filter for demonstrating vbz compression
"""

import os
import h5py
import argparse
from shutil import copy2

__version__ = "0.1.1"


def compress_fast5(filename, output_suffix, vbz_version=0, decompress=False):
    """
    (De)compress the raw signal in the fast5
    """
    # create a copy of the input file for compressing
    filename = os.path.abspath(filename)
    out_filename = filename + output_suffix
    copy2(filename, out_filename)

    # open the copy and compress the signal datasets only
    with h5py.File(out_filename, 'r+') as fast5:

        if decompress:
            compression_opts = {
                "compression": "gzip", "compression_opts": 1
            }
        else:
            compression_opts = {
                "compression": 32020,
                # 2 byte integers with zig zag + level 1 zstd
                "compression_opts": (vbz_version, 2, 1, 1) 
            }

        # multi read files
        for read in fast5:
            if not read.startswith("read_"):
                continue
            
            # read the signal dataset
            group = '%s/Raw/' % read
            raw = fast5[group]["Signal"][:]
            
            # remove the dataset
            del fast5[group]["Signal"]

            # store the dataset with the new compression
            fast5[group].create_dataset(
                "Signal",
                data=raw,
                **compression_opts,
                chunks=(len(raw),)
            )

    return out_filename


def main(args, print_help):
    for filename in args.files:
        print(compress_fast5(filename, args.output_suffix, args.vbz_version, args.decompress))


if __name__ == "__main__":
    parser = argparse.ArgumentParser("fast5compress")
    parser.add_argument('files', nargs='*', help='input files')
    parser.add_argument("-d", "--decompress", action="store_true", default=False)
    parser.add_argument("-s", "--output-suffix", default=".tmp")
    parser.add_argument('-v', '--version', action='version', version=__version__)
    parser.add_argument('--vbz-version', type=int, default=1)
    main(parser.parse_args(), parser.print_help)
