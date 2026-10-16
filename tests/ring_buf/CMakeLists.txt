cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(ring_buf_unit_test)

# Enable coverage compilation flags
if(CONFIG_COVERAGE)
    target_compile_options(app PRIVATE -fprofile-arcs -ftest-coverage)
    target_link_options(app PRIVATE -fprofile-arcs -ftest-coverage)
endif()

target_sources(app PRIVATE
    src/test_ring_buf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/modules/ring_buf/src/ring_buf.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../app/modules/ring_buf/include
)
