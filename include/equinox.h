#ifndef EQUINOX_H
#define EQUINOX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY (x)

#define EQUINOX_VERSION_MAJOR 0
#define EQUINOX_VERSION_MINOR 1
#define EQUINOX_VERSION_PATCH 0

#define EQUINOX_VERSION_INT                                                   \
  ((EQUINOX_VERSION_MAJOR * 10000) + (EQUINOX_VERSION_MINOR * 100)            \
   + EQUINOX_VERSION_PATCH)

#define EQUINOX_VERSION_STRING                                                \
  TOSTRING (EQUINOX_VERSION_MAJOR)                                            \
  "." TOSTRING (EQUINOX_VERSION_MINOR) "." TOSTRING (EQUINOX_VERSION_PATCH)

#ifdef __cplusplus
extern "C"
{
#endif

#include "equinox/congruence.h"
#include "equinox/cost.h"
#include "equinox/eclass.h"
#include "equinox/egraph.h"
#include "equinox/enode.h"
#include "equinox/hashcons.h"
#include "equinox/io.h"
#include "equinox/pattern.h"
#include "equinox/rewrite.h"
#include "equinox/sexp.h"
#include "equinox/simplify.h"
#include "equinox/types.h"
#include "equinox/unionfind.h"
#include "equinox/memory.h"

  typedef enum
  {
    EQUINOX_Ok,
    EQUINOX_ERR_IO,
    EQUINOX_ERR_CLI,
    EQUINOX_ERR_MEMORY,
    EQUINOX_ERR_DIVERGENCE,
  } equinox_stat_t;

  equinox_stat_t equinox_init (void);

  void equinox_shutdown (void);

  const char *equinox_get_err (equinox_stat_t);

  typedef void (*equinox_err_callback_t) (const char *msg);
  void equinox_set_err_callback (equinox_stat_t stat,
                                 equinox_err_callback_t callback);

  void equinox_set_debug_mode (bool enable);

  static inline const char *
  equinox_get_version (void)
  {
    return EQUINOX_VERSION_STRING;
  }

#ifdef __cplusplus
}
#endif
