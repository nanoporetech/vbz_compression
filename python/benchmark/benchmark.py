import h5py
import matplotlib.pyplot as plt
import numpy
import time

import zstd


def time_dataset(data, create_fn, target_time=0.1):
    element_size = data.dtype.itemsize
    total_time = 0
    times = []
    count = 0

    for _ in range(1000):
        
        begin = time.time()
        with h5py.File("test_file.h5", "w") as f:
            create_fn(f, "bar", data, element_size)
        end = time.time()

        count += 1
        this_time = (end - begin)
        times.append(this_time)
        total_time += this_time

        if total_time > target_time:
            break

    return sorted(times)[len(times)//2]

def create_uncompressed(f, name, data, element_size):
    f.create_dataset(name, data=data, chunks=(len(data),),)

def create_vbz_zstd(f, name, data, element_size):
    use_zig_zag = 1
    f.create_dataset("bar", compression=32020, compression_opts=(0, element_size, use_zig_zag, 1), data=data, chunks=(len(data),))

def create_vbz(f, name, data, element_size):
    use_zig_zag = 1
    f.create_dataset("bar", compression=32020, compression_opts=(0, element_size, use_zig_zag, 0), data=data, chunks=(len(data),))

def create_zlib(f, name, data, element_size):
    f.create_dataset("bar", compression="gzip", chunks=(len(data),), data=data)

def create_zstd_raw(f, name, data, element_size):
    zstd_data = numpy.frombuffer(zstd.compress(data), dtype="u1")
    f.create_dataset("bar", chunks=(len(zstd_data),), data=zstd_data)


if __name__ == "__main__":
    plots = []
    plot_names = []

    palette = plt.get_cmap('Set1')

    for m_i, method in enumerate([create_zstd_raw, create_vbz, create_vbz_zstd, create_zlib, create_uncompressed]):
        data_types = ["i1", "i2", "i4"]
        for d_i, data_type in enumerate(data_types):
            size_to_times = []

            for i in range(1,20):
                size = 1000000 * i
                data = numpy.random.randint(low=-50, high=50, size=size//int(data_type[1:]), dtype=data_type)
                size_to_times.append((data.nbytes, time_dataset(data, method)))

            print(size_to_times)
            x = [e[0] for e in size_to_times]
            y = [e[0]/e[1] for e in size_to_times]

            c_index = m_i
            
            plt.subplot(1, len(data_types), 1 + d_i)
            plots.append(plt.plot(x, y, color=palette(c_index))[0])
            plot_names.append("%s %s" % (method.__name__, data_type))

            #ax = plt.axes()
            #ax.grid()
            #ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda v,_: format_size(v, "/s")))
            #ax.xaxis.set_major_formatter(plt.FuncFormatter(lambda v,_: format_size(v)))

            plt.xlabel("block size (bytes)")
            if d_i == 0:
                plt.ylabel("Write speed (bytes/second)")
            plt.legend(plots, plot_names)

        
    def format_size(val, suffix=""):
        units = ['bytes', 'KB', 'MB', 'GB']
        divisor = 1000

        unit_idx = 0
        while val > divisor and unit_idx < len(units):
            val /= divisor
            unit_idx += 1

        return '%.1f %s%s' % (val, units[unit_idx], suffix)

    plt.show()