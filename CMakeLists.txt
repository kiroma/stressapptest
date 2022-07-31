cmake_minimum_required(VERSION 3.1)
project(StressAppTest CXX C)

add_executable(findmask src/findmask.c)

add_executable(stressapptest
        src/adler32memcpy.cc
        src/disk_blocks.cc
        src/error_diag.cc
        src/finelock_queue.cc
        src/logger.cc
        src/main.cc
        src/os.cc
        src/os_factory.cc
        src/pattern.cc
        src/queue.cc
        src/sat.cc
        src/sat_factory.cc
        src/worker.cc
)

target_compile_features(stressapptest PRIVATE cxx_std_11)
string(TIMESTAMP STRESSAPPTEST_TIMESTAMP)
target_compile_definitions(stressapptest PRIVATE STRESSAPPTEST_TIMESTAMP="${STRESSAPPTEST_TIMESTAMP}")