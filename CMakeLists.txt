cmake_minimum_required(VERSION 3.15)
project(parallel_convolution)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -g -fopenmp")

include_directories(include deps)
set(ALL_PHONY_TARGETS conv bench test)

# Поиск cmocka
find_package(PkgConfig REQUIRED)
pkg_check_modules(CMOCKA REQUIRED cmocka)

set(COMMON_SOURCES
	src/cli_utils.c
	src/core_builder.c
	src/proc_image_utils.c
	src/proc_image.c
	src/queue.c
)

# ============ CLI приложение ============
add_executable(conv_bin src/conv.c ${COMMON_SOURCES})
set_target_properties(conv_bin PROPERTIES OUTPUT_NAME conv)
add_custom_target(build DEPENDS conv_bin)

# ============ Бенчмарк task2 (single_img.c) ============

if(NOT DEFINED MY_BENCH_SIZE)
	set(MY_BENCH_SIZE "3")
endif()

if(NOT DEFINED MY_BENCH_FILTER)
	set(MY_BENCH_FILTER "blur")
endif()

add_executable(bench_task2_bin benchmarks/src/single_img.c ${COMMON_SOURCES})
set_target_properties(bench_task2_bin PROPERTIES OUTPUT_NAME bench_task2_exe)

if(DEFINED MY_BENCH_INPUT)
	add_custom_target(bench_task2
		COMMAND ./bench_task2_exe --input=${MY_BENCH_INPUT} --size=${MY_BENCH_SIZE}
			--filter=${MY_BENCH_FILTER}
		DEPENDS bench_task2_bin
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
	unset(MY_BENCH_INPUT CACHE)
else()
	add_custom_target(bench_task2
		COMMAND ./bench_task2_exe --size=${MY_BENCH_SIZE} --filter=${MY_BENCH_FILTER}
		DEPENDS bench_task2_bin
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()

# ============ Бенчмарк task3 (queue_img.c) ============

add_executable(bench_task3_bin benchmarks/src/queue_img.c ${COMMON_SOURCES})
set_target_properties(bench_task3_bin PROPERTIES OUTPUT_NAME bench_task3_exe)

add_custom_target(bench_task3
	COMMAND ./bench_task3_exe
	DEPENDS bench_task3_bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ============ Бенчмарк setup.c ============

add_executable(bench_setup_bin benchmarks/src/setup.c ${COMMON_SOURCES})
set_target_properties(bench_setup_bin PROPERTIES OUTPUT_NAME bench_setup_exe)

add_custom_target(bench_setup
	COMMAND ./bench_setup_exe
	DEPENDS bench_setup_bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# ================ Tесты =====================

add_executable(test_bin tests/test_parallel_methods.c ${COMMON_SOURCES})
target_include_directories(test_bin PRIVATE ${CMOCKA_INCLUDE_DIRS})
target_link_libraries(test_bin PRIVATE ${CMOCKA_LIBRARIES})
set_target_properties(test_bin PROPERTIES OUTPUT_NAME test_exe)

# Создаем 'make test', который собирает и запускает тесты
add_custom_target(test
	COMMAND ./test_exe
	DEPENDS test_bin
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(conv_bin PRIVATE m pthread)
target_link_libraries(bench_task2_bin PRIVATE m pthread)
target_link_libraries(bench_task3_bin PRIVATE m pthread)
target_link_libraries(bench_setup_bin PRIVATE m pthread)
target_link_libraries(test_bin PRIVATE m pthread)

unset(MY_BENCH_SIZE CACHE)
unset(MY_BENCH_FILTER CACHE)