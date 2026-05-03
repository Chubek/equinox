#!/usr/bin/env bash
set -euo pipefail

echo "[+] Fixing Equinox compilation issues..."

########################################
# 1. Fix UTHash includes (UT_string etc)
########################################

echo "[+] Patching libutils.h for UTHash..."

LIBUTILS="include/equinox/libutils.h"

# Ensure uthash + utstring are included
grep -q "utstring.h" "$LIBUTILS" || sed -i '1i #include "utstring.h"' "$LIBUTILS"
grep -q "uthash.h" "$LIBUTILS" || sed -i '1i #include "uthash.h"' "$LIBUTILS"

########################################
# 2. Ensure include aggregator exists
########################################

INCLUDES_HDR="include/equinox/libutil-includes.h"

if [ ! -f "$INCLUDES_HDR" ]; then
  echo "[+] Creating libutil-includes.h..."
  cat > "$INCLUDES_HDR" <<'EOF'
#pragma once

/* UTHash */
#include "uthash.h"
#include "utstring.h"
#include "utarray.h"

/* TLSF */
#include "tlsf.h"

/* libdag-ethash */
#include "dag.h"
#include "mdag.h"
#include "common.h"
#include "mine.h"
EOF
fi

########################################
# 3. Fix libutils.h to use aggregator
########################################

grep -q "libutil-includes.h" "$LIBUTILS" || \
  sed -i '1i #include "equinox/libutil-includes.h"' "$LIBUTILS"

########################################
# 4. Fix Equinox type visibility
########################################

echo "[+] Patching enode.c includes..."

ENODE_SRC="src/enode.c"

sed -i '1i #include <equinox/eclass.h>' "$ENODE_SRC"
sed -i '1i #include <equinox/egraph.h>' "$ENODE_SRC"

########################################
# 5. Ensure EQX_ECLASS_ID_INVALID exists
########################################

ECLASS_HDR="include/equinox/eclass.h"

grep -q "EQX_ECLASS_ID_INVALID" "$ECLASS_HDR" || cat >> "$ECLASS_HDR" <<'EOF'

#ifndef EQX_ECLASS_ID_INVALID
#define EQX_ECLASS_ID_INVALID ((eqx_eclass_id_t)-1)
#endif
EOF

########################################
# 6. Fix ethash type visibility
########################################

echo "[+] Patching ethash includes..."

# Find likely header using ethash_light_t
grep -rl "ethash_light_t" include src | while read -r file; do
  grep -q "dag.h" "$file" || sed -i '1i #include "dag.h"' "$file"
done

########################################
# 7. Fix include paths in CMake
########################################

echo "[+] Patching CMakeLists.txt..."

CMAKE_FILE="CMakeLists.txt"

# Add include dirs if missing
grep -q "third_party/uthash/src" "$CMAKE_FILE" || \
sed -i '/include_directories/a \
    ${CMAKE_SOURCE_DIR}/third_party/uthash/src \
    ${CMAKE_SOURCE_DIR}/third_party/tlsf \
    ${CMAKE_SOURCE_DIR}/third_party/libdag-ethash \
' "$CMAKE_FILE"

########################################
# 8. Force correct include order (important)
########################################

echo "[+] Enforcing include order in libutils.h..."

sed -i '1i \
/* Order matters: third-party first */\
#include "uthash.h"\
#include "utstring.h"\
#include "utarray.h"\
' "$LIBUTILS"

########################################
# 9. Clean + rebuild
########################################

echo "[+] Cleaning build..."
rm -rf build
mkdir build
cd build

echo "[+] Reconfiguring..."
cmake ..

echo "[+] Building..."
make -j$(nproc)

echo "[+] Done. Build should now succeed."