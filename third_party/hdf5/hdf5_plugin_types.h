/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5. The full HDF5 copyright notice, including      *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Purpose:     Header file for writing external HDF5 plugins.
 */

#ifndef _H5PLextern_H
#define _H5PLextern_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum H5PL_type_t {
    H5PL_TYPE_ERROR         = -1,   /* Error                */
    H5PL_TYPE_FILTER        =  0,   /* Filter               */
    H5PL_TYPE_NONE          =  1    /* This must be last!   */
} H5PL_type_t;


#define H5Z_CLASS_T_VERS    (1)
#define H5Z_FLAG_REVERSE	0x0100	/*reverse direction; read	*/

/*
 * Filter identifiers.  Values 0 through 255 are for filters defined by the
 * HDF5 library.  Values 256 through 511 are available for testing new
 * filters. Subsequent values should be obtained from the HDF5 development
 * team at hdf5dev@ncsa.uiuc.edu.  These values will never change because they
 * appear in the HDF5 files.
 */
typedef int H5Z_filter_t;

/*
 * A filter gets definition flags and invocation flags (defined above), the
 * client data array and size defined when the filter was added to the
 * pipeline, the size in bytes of the data on which to operate, and pointers
 * to a buffer and its allocated size.
 *
 * The filter should store the result in the supplied buffer if possible,
 * otherwise it can allocate a new buffer, freeing the original.  The
 * allocated size of the new buffer should be returned through the BUF_SIZE
 * pointer and the new buffer through the BUF pointer.
 *
 * The return value from the filter is the number of bytes in the output
 * buffer. If an error occurs then the function should return zero and leave
 * all pointer arguments unchanged.
 */
typedef size_t (*H5Z_func_t)(unsigned int flags, size_t cd_nelmts,
			     const unsigned int cd_values[], size_t nbytes,
			     size_t *buf_size, void **buf);

/*
 * The filter table maps filter identification numbers to structs that
 * contain a pointers to the filter function and timing statistics.
 */
typedef struct H5Z_class2_t {
    int version;                    /* Version number of the H5Z_class_t struct     */
    H5Z_filter_t id;                /* Filter ID number                             */
    unsigned encoder_present;       /* Does this filter have an encoder?            */
    unsigned decoder_present;       /* Does this filter have a decoder?             */
    const char	*name;              /* Comment for debugging                        */
    void* can_apply;                /* The "can apply" callback for a filter        */
    void* set_local;                /* The "set local" callback for a filter        */
    H5Z_func_t filter;              /* The actual filter function                   */
} H5Z_class2_t;

#ifdef __cplusplus
}
#endif

#endif /* _H5PLextern_H */

