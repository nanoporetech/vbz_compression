import subprocess
import os
import sys

import unittest

TEST_DATA = os.path.join(os.path.dirname(__file__), "..", "..", "test_data")


class TestHDFRepack(unittest.TestCase):

    def test_simple_range(self):
        source_file = os.path.join(TEST_DATA, "multi_fast5_zip.fast5")
        dest_file_vbz = os.path.join(TEST_DATA, "multi_fast5_zip.fast5.repacked_vbz")
        dest_file_none = os.path.join(TEST_DATA, "multi_fast5_zip.fast5.repacked_none")

        result = subprocess.run([ 'h5repack', '-f', 'NONE', source_file, dest_file_none ],
            stdout=sys.stdout,
            stderr=sys.stderr
        )
        self.assertEqual(result.returncode, 0)

        result = subprocess.run([ 'h5repack', '-f', 'UD=32020,5,0,0,2,1,1', source_file, dest_file_vbz ],
            stdout=sys.stdout,
            stderr=sys.stderr
        )
        self.assertEqual(result.returncode, 0)

        uncompressed_bytes = os.stat(dest_file_none).st_size
        source_bytes = os.stat(source_file).st_size
        vbz_bytes = os.stat(dest_file_vbz).st_size
        print("Uncompressed file is %s bytes" 
            % uncompressed_bytes)
        print("Source file is %s bytes (%.0f%% of uncompressed)" 
            % (source_bytes, (source_bytes/uncompressed_bytes*100)))
        print("vbz file is %s bytes (%.0f%% of uncompressed, %.0f%% of source)" 
            % (vbz_bytes, (vbz_bytes/uncompressed_bytes*100), (vbz_bytes/source_bytes*100)))

if __name__ == '__main__':
    unittest.main()