set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "-std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/3rdparty/tensorflow ${CMAKE_BINARY_DIR}/flatbuffers/include)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_BINARY_DIR}/3rdparty/tensorflow/tensorflow/lite/libtensorflowlite.a
    ${CMAKE_BINARY_DIR}/_deps/ruy-build/libruy.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_base.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_dynamic_annotations.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_log_severity.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_malloc_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_raw_logging_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_spinlock_wait.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/base/libabsl_throw_delegate.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/debugging/libabsl_debugging_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/debugging/libabsl_demangle_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/debugging/libabsl_stacktrace.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/debugging/libabsl_symbolize.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags_config.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags_marshalling.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags_program_name.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/flags/libabsl_flags_registry.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/hash/libabsl_city.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/hash/libabsl_hash.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/numeric/libabsl_int128.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/status/libabsl_status.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/strings/libabsl_cord.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/strings/libabsl_strings.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/strings/libabsl_strings_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/strings/libabsl_str_format_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/synchronization/libabsl_graphcycles_internal.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/synchronization/libabsl_synchronization.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/time/libabsl_civil_time.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/time/libabsl_time.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/time/libabsl_time_zone.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/types/libabsl_bad_optional_access.a
    ${CMAKE_BINARY_DIR}/_deps/abseil-cpp-build/absl/types/libabsl_bad_variant_access.a
    ${CMAKE_BINARY_DIR}/_deps/farmhash-build/libfarmhash.a
    ${CMAKE_BINARY_DIR}/_deps/fft2d-build/libfft2d_fftsg.a
    ${CMAKE_BINARY_DIR}/_deps/fft2d-build/libfft2d_fftsg2d.a
    ${CMAKE_BINARY_DIR}/_deps/flatbuffers-build/libflatbuffers.a
)
