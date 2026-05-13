#ifndef EQX_COMMON_ERL_H
#define EQX_COMMON_ERL_H

#include <stddef.h>

#include "equinox/rewrite.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct redvisage_erl_ruleset {
    eqx_rewrite_rule_t** rules;
    size_t count;
} redvisage_erl_ruleset_t;

int redvisage_erl_load_files(size_t file_count, const char** files, redvisage_erl_ruleset_t* out_ruleset);
void redvisage_erl_ruleset_destroy(redvisage_erl_ruleset_t* ruleset);

#ifdef __cplusplus
}
#endif

#endif
