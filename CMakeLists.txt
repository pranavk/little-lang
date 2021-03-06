cmake_minimum_required(VERSION 2.6)
project(lilang)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g") 

find_package(FLEX REQUIRED)
message(STATUS "Found Flex ${FLEX_VERSION}")
FLEX_TARGET(lilFlex ${CMAKE_SOURCE_DIR}/src/lexer.l ${CMAKE_SOURCE_DIR}/src/scanner.cpp)

find_package(LLVM 7.0.0 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core support irreader)

FILE(GLOB SRCFILES src/*.cpp)
# Set file properties on all files except generated ones
set(SRC_FILE_LIST ${SRCFILES})
list(REMOVE_ITEM SRC_FILE_LIST ${CMAKE_SOURCE_DIR}/src/scanner.cpp)
set_source_files_properties(${SRC_FILE_LIST}
			    PROPERTIES COMPILE_FLAGS "-Wall")
add_library(flex STATIC IMPORTED GLOBAL)

add_executable(lilang ${SRCFILES} ${FLEX_lilFlex_OUTPUTS})
target_link_libraries(lilang ${llvm_libs})

add_custom_command(TARGET lilang POST_BUILD
		   COMMAND clang -c ${CMAKE_SOURCE_DIR}/runtime.c)
add_custom_command(TARGET lilang POST_BUILD
		   COMMAND cp ${CMAKE_SOURCE_DIR}/driver.sh ${CMAKE_CURRENT_BINARY_DIR}/little)

enable_testing()

if ("$ENV{GPGKEY}" STREQUAL "C7025AF7")
	message("This is Pranav's personal computer.")
	FILE(GLOB GOODFILES ${CMAKE_SOURCE_DIR}/../passing-tests/*.lil)
else()
	FILE(GLOB GOODFILES ${CMAKE_SOURCE_DIR}/passing-tests/*.lil)
endif()
foreach(file ${GOODFILES})
	add_test(NAME passingtests_${file}
			COMMAND lilang ${file})
	set_tests_properties(passingtests_${file}
						PROPERTIES
						LABELS "PASSING-TESTS")
endforeach(file ${GOODFILES})

if ("$ENV{GPGKEY}" STREQUAL "C7025AF7")
	FILE(GLOB BADFILES ${CMAKE_SOURCE_DIR}/../failing-tests/*.lil)
else()
	FILE(GLOB BADFILES ${CMAKE_SOURCE_DIR}/failing-tests/*.lil)
endif()
foreach(file ${BADFILES})
	add_test(NAME failingtests_${file}
			COMMAND lilang ${file})
	set_tests_properties(failingtests_${file}
						PROPERTIES
						WILL_FAIL true
						LABELS "FAILING-TESTS")
endforeach(file ${BADFILES})
