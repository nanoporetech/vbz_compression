import h5py
import numpy
import os

import unittest

TEST_DATA = os.path.join(os.path.dirname(__file__), "..", "..", "test_data")

class TestVbzFilter(unittest.TestCase):
    VBZ_VERISON = 0

    def test_simple_range(self):
        original_data = numpy.arange(1, 100, dtype="i2")

        with  h5py.File("test_file.h5", "w") as f:
            f.create_dataset("bar", compression=32020, compression_opts=(self.VBZ_VERISON, 2, 1, 1), data=original_data)

        with h5py.File("test_file.h5", "r") as f:
            numpy.testing.assert_array_equal(f["bar"][:], original_data)

    def test_compressing_real_data(self):
        zip_file = h5py.File(os.path.join(TEST_DATA, "multi_fast5_zip.fast5"))

        for zip_key in zip_file.keys():
            original_data = zip_file[zip_key]["Raw/Signal"][:]

            if (zip_key == "read_008468c3-e477-46c4-a6e2-7d021a4ebf0b"):
                continue

            with h5py.File("test_file.h5", "w") as f:
                f.create_dataset("bar", compression=32020, compression_opts=(self.VBZ_VERISON, 2, 1, 1), data=original_data, chunks=(len(original_data),))

            with h5py.File("test_file.h5", "r") as f:
                numpy.testing.assert_array_equal(f["bar"][:], original_data)

class TestVbzFilterV0(TestVbzFilter):
    VBZ_VERISON = 0
    
class TestVbzFilterV1(TestVbzFilter):
    VBZ_VERISON = 1
    
class TestVbzStoredFiles(unittest.TestCase):
    def test_stored_files(self):
        files = [
            h5py.File(os.path.join(TEST_DATA, "multi_fast5_zip.fast5")),
            h5py.File(os.path.join(TEST_DATA, "multi_fast5_vbz.fast5")),
            h5py.File(os.path.join(TEST_DATA, "multi_fast5_vbz_v1.fast5")),
        ]

        for sorted_keys in zip(*[sorted(file.keys()) for file in files ]):
            for e in sorted_keys[1:]:
                self.assertEqual(sorted_keys[0], e)

            key = sorted_keys[0]
            first_data = files[0][key]["Raw/Signal"][:]
            for file in files[1:]:
                data = file[key]["Raw/Signal"][:]
                numpy.testing.assert_array_equal(first_data, data)


if __name__ == '__main__':
    unittest.main()