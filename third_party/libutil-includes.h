/**
 * @file libutil-includes.h
 * @brief Third-party library includes for libutils
 * 
 * This header aggregates all third-party dependencies used by libutils.h
 */

#ifndef LIBUTIL_INCLUDES_H
#define LIBUTIL_INCLUDES_H

/* TLSF Memory Allocator */
#include "tlsf/tlsf.h"

/* UThash - Hash tables, dynamic arrays, and string buffers */
#include "uthash/uthash.h"
#include "uthash/utarray.h"
#include "uthash/utstring.h"
#include "uthash/utlist.h"

/* libdag-ethash - Ethash DAG generation */
#include "libdag-ethash/dag.h"
#include "libdag-ethash/ethash.h"

#endif /* LIBUTIL_INCLUDES_H */
