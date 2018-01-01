cmake_minimum_required(VERSION 3.6)
project(ap_verifier)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ap_verifier.cpp ap_verifier.h ap_verifier_utils.h predicate_node.h predicate_node.cpp ap_verifier_util.cpp router.h ap_node.h ap_node.cpp router.cpp)

link_directories(/usr/local/lib)
include_directories(/usr/local/include)

add_executable(ap_verifier ${SOURCE_FILES})

target_link_libraries(ap_verifier bdd.a jsoncpp)