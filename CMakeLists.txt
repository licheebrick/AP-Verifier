cmake_minimum_required(VERSION 3.6)
project(ap_verifier)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp ap_verifier.cpp ap_verifier.h ap_verifier_utils.h predicate_node.h predicate_node.cpp ap_verifier_util.cpp router.h ap_node.h ap_node.cpp router.cpp ap_nodeS.h ap_nodeS.cpp ap_nodeV.h ap_nodeV.cpp ap_nodeB.h ap_nodeB.cpp)

link_directories(/usr/local/lib)
link_directories(/usr/local/Cellar/log4cxx/0.10.0_1/lib)
include_directories(/usr/local/include)
include_directories(/usr/local/Cellar/log4cxx/0.10.0_1/include)

add_executable(ap_verifier ${SOURCE_FILES})

target_link_libraries(ap_verifier bdd.a jsoncpp log4cxx)