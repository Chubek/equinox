#ifndef REDVISAGE_REWRITE_H
#define REDVISAGE_REWRITE_H

#include <stddef.h>

#include "equinox/rewrite.h"

#ifdef __cplusplus
extern "C" {
#endif

eqx_rewrite_rule_t** redvisage_default_rules(size_t* out_count);
void redvisage_default_rules_destroy(eqx_rewrite_rule_t** rules, size_t rule_count);

#ifdef __cplusplus
}
#endif

#endif
