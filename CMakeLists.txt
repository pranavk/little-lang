cmake_minimum_required(VERSION 2.6)
project(lilang)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g") 

find_package(FLEX REQUIRED)
message(STATUS "Found Flex ${FLEX_VERSION}")
FLEX_TARGET(lilFlex ${CMAKE_SOURCE_DIR}/lexer.l ${CMAKE_SOURCE_DIR}/scanner.cpp)

find_package(LLVM 6.0.1 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core support irreader)

FILE(GLOB SRCFILES *.cpp)
add_library(flex STATIC IMPORTED GLOBAL)

add_executable(lilang ${SRCFILES} ${FLEX_lilFlex_OUTPUTS})
target_link_libraries(lilang ${llvm_libs})

enable_testing()

FILE(GLOB GOODFILES ${CMAKE_SOURCE_DIR}/../passing-tests/*.lil)
foreach(file ${GOODFILES})
	add_test(NAME passingtests_${file}
			COMMAND lilang ${file})
	set_tests_properties(passingtests_${file}
						PROPERTIES
						LABELS "PASSING-TESTS")
endforeach(file ${GOODFILES})

FILE(GLOB BADFILES ${CMAKE_SOURCE_DIR}/../failing-tests/*.lil)
foreach(file ${BADFILES})
	add_test(NAME failingtests_${file}
			COMMAND lilang ${file})
	set_tests_properties(failingtests_${file}
						PROPERTIES
						WILL_FAIL true
						LABELS "FAILING-TESTS")
endforeach(file ${BADFILES})
