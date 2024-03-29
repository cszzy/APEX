cmake_minimum_required(VERSION 3.12)
project(apex)

set(CMAKE_CXX_STANDARD 14)
include(FetchContent)
include(ExternalProject)

set(CMAKE_CXX_FLAGS "-lnuma -lrt -lpthread -ldl -mrtm -msse4.1 -mavx2 -O3 -g -march=native")

add_definitions(-DPMEM)
add_definitions(-DPMDK)
message(STATUS "Persistence support: PMDK")

##################### PMDK ####################
set(PMDK_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/pmdk")
ExternalProject_Add(PMDK
        GIT_REPOSITORY https://github.com/HaoPatrick/pmdk.git
        GIT_TAG addr-patch
	      BUILD_IN_SOURCE 1
        BUILD_COMMAND $(MAKE) EXTRA_CFLAGS=-Wno-error
        PREFIX ${PMDK_PREFIX}
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""
        LOG_DOWNLOAD ON
        LOG_CONFIGURE ON
        LOG_BUILD ON
        )

include_directories(${PMDK_PREFIX}/src/PMDK/src/include)
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  link_directories(${PMDK_PREFIX}/src/PMDK/src/debug)
else ()
  link_directories(${PMDK_PREFIX}/src/PMDK/src/nondebug)
endif ()

##################### Epoch Reclaim ####################
FetchContent_Declare(
        epoch_reclaimer
        GIT_REPOSITORY https://github.com/XiangpengHao/epoch-reclaimer.git
        GIT_TAG master
)
if (NOT epoch_reclaimer_POPULATED)
  FetchContent_Populate(epoch_reclaimer)
  include_directories(${epoch_reclaimer_SOURCE_DIR})
endif ()

include_directories(include)
include_directories(benchmark)
include_directories(include/util)
include_directories(src)
list(APPEND libs_to_link pmemobj pmem tbb papi tcmalloc_minimal numa pthread)

add_executable(nali_multi_bench benchmark/multi_bench.cpp)       
add_dependencies(nali_multi_bench PMDK)
target_link_libraries(nali_multi_bench PRIVATE ${libs_to_link})