#include "common/erl.h"

#include "redvisage/opcodes.h"
#include "redvisage/rewrite.h"

#include <stdio.h>
#include <string.h>

int redvisage_erl_load_files(size_t file_count, const char** files, redvisage_erl_ruleset_t* out_ruleset) {
    (void)files;
    if (!out_ruleset) return -1;
    out_ruleset->rules = NULL;
    out_ruleset->count = 0;

    if (file_count == 0) {
        out_ruleset->rules = redvisage_default_rules(&out_ruleset->count);
        return out_ruleset->rules ? 0 : -1;
    }

    for (size_t i = 0; i < file_count; ++i) {
        FILE* f = fopen(files[i], "r");
        if (!f) return -1;
        fclose(f);
    }

    /* Placeholder parser: we validate files exist and fall back to built-in rules. */
    out_ruleset->rules = redvisage_default_rules(&out_ruleset->count);
    return out_ruleset->rules ? 0 : -1;
}

void redvisage_erl_ruleset_destroy(redvisage_erl_ruleset_t* ruleset) {
    if (!ruleset) return;
    redvisage_default_rules_destroy(ruleset->rules, ruleset->count);
    ruleset->rules = NULL;
    ruleset->count = 0;
}
