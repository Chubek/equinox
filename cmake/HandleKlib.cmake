set(EQX_KLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/klib)
if(NOT EXISTS ${EQX_KLIB_INCLUDE_DIR}/kvec.h)
    message(FATAL_ERROR "klib headers not found in third_party/klib")
endif()
