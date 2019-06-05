import h5py
import numpy
import os

import unittest

TEST_DATA = os.path.join(os.path.dirname(__file__), "..", "..", "test_data")

class TestVbzFilter(unittest.TestCase):

    def test_simple_range(self):
        original_data = numpy.array(range(1, 100), dtype="i2")

        with  h5py.File("test_file.h5", "w") as f:
            f.create_dataset("bar", compression=32020, compression_opts=(0, 2, 1, 1), data=original_data)

        with h5py.File("test_file.h5", "r") as f:
            numpy.testing.assert_array_equal(f["bar"][:], original_data)

    def test_compressing_real_data(self):
        zip_file = h5py.File(os.path.join(TEST_DATA, "multi_fast5_zip.fast5"))

        for zip_key in zip_file.keys():
            original_data = zip_file[zip_key]["Raw/Signal"][:]
            
            with  h5py.File("test_file.h5", "w") as f:
                f.create_dataset("bar", compression=32020, compression_opts=(0, 2, 1, 1), data=original_data, chunks=(len(original_data),))

            with h5py.File("test_file.h5", "r") as f:
                numpy.testing.assert_array_equal(f["bar"][:], original_data)

    
    def test_stored_files(self):
        zip_file = h5py.File(os.path.join(TEST_DATA, "multi_fast5_zip.fast5"))
        vbz_file = h5py.File(os.path.join(TEST_DATA, "multi_fast5_vbz.fast5"))

        for zip_key,vbz_key in zip(sorted(zip_file.keys()), sorted(vbz_file.keys())):
            self.assertEqual(zip_key, vbz_key)

            zip_data = zip_file[zip_key]["Raw/Signal"][:]
            vbz_data = vbz_file[vbz_key]["Raw/Signal"][:]
            numpy.testing.assert_array_equal(zip_data, vbz_data)

if __name__ == '__main__':
    unittest.main()