cmake_minimum_required(VERSION 3.25)
project(Lab3)

set(CMAKE_CXX_STANDARD 20)

set ( Protobuf_USE_STATIC_LIBS ON )
set ( PROTOBUF_ROOT /opt/homebrew/Cellar/protobuf/25.3)
set ( ABSL_ROOT /opt/homebrew/Cellar/abseil/20230802.1)
set ( PROTOBUF_C_ROOT /opt/homebrew/Cellar/protobuf-c/1.5.0_2)

include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-config.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-module.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-options.cmake)
include(${PROTOBUF_ROOT}/lib/cmake/protobuf/protobuf-targets.cmake)

include(${ABSL_ROOT}/lib/cmake/absl/abslConfig.cmake)
include(${ABSL_ROOT}/lib/cmake/absl/abslTargets.cmake)

find_package( Absl REQUIRED HINTS ${ABSL_ROOT}lib/cmake/absl )
if ( Absl_FOUND )
    message( STATUS "Absl found")
    include_directories(${ABSL_ROOT}/include)
else()
    message( WARNING "Protobuf package not found -> specify search path via PROTOBUF_ROOT variable")
endif()

include_directories(${ABSL_ROOT}/include)

find_package( Protobuf REQUIRED HINTS ${PROTOBUF_ROOT}/lib/cmake/protobuf )
if ( Protobuf_FOUND )
    message( STATUS "Protobuf version : ${Protobuf_VERSION}" )
    message( STATUS "Protobuf include path : ${Protobuf_INCLUDE_DIRS}" )
    message( STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}" )
    message( STATUS "Protobuf compiler libraries : ${Protobuf_PROTOC_LIBRARIES}")
    message( STATUS "Protobuf lite libraries : ${Protobuf_LITE_LIBRARIES}")
else()
    message( WARNING "Protobuf package not found -> specify search path via PROTOBUF_ROOT variable")
endif()


#include_directories(${Protobuf_INCLUDE_DIRS})

include_directories(${PROTOBUF_C_ROOT}/include)



add_subdirectory(client)
add_subdirectory(server)

set(protobuf-cpp-GENERATED_SRC ${PROJECT_SOURCE_DIR}/generated_cpp/query.pb.cc)
set(protobuf-c-GENERATED_SRC ${PROJECT_SOURCE_DIR}/generated_c/query.pb-c.c)

add_custom_target(GenerateProtobuf DEPENDS ${protobuf-cpp-GENERATED_SRC} ${protobuf-c-GENERATED_SRC})
add_custom_command(OUTPUT ${protobuf-cpp-GENERATED_SRC} ${protobuf-c-GENERATED_SRC}
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/generated_cpp/
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/generated_c/
        COMMAND
        protoc  --cpp_out=${PROJECT_SOURCE_DIR}/generated_cpp/ --c_out=${PROJECT_SOURCE_DIR}/generated_c/
            -I=${PROJECT_SOURCE_DIR}/protobufs ${PROJECT_SOURCE_DIR}/protobufs/query.proto
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        DEPENDS ${PROJECT_SOURCE_DIR}/protobufs/query.proto)

file(GLOB_RECURSE protobuf-cpp CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/generated_cpp/*.h
        ${PROJECT_SOURCE_DIR}/generated_cpp/*.cc
        )

file(GLOB_RECURSE protobuf-c CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/generated_c/*.h
        ${PROJECT_SOURCE_DIR}/generated_c/*.c
        )

add_library(protobuf-cpp SHARED ${protobuf-cpp})  # TODO fix cold start
target_include_directories(protobuf-cpp PUBLIC
        ${PROJECT_SOURCE_DIR}/generated_cpp)
target_link_libraries(protobuf-cpp
        protobuf::libprotobuf
        absl::log_internal_message
        absl::log_internal_check_op)
add_dependencies(protobuf-cpp GenerateProtobuf)

add_library(protobuf-c SHARED ${protobuf-c})
target_include_directories(protobuf-c PUBLIC
        ${PROJECT_SOURCE_DIR}/generated_c)
target_link_libraries(protobuf-c
        ${PROTOBUF_C_ROOT}/lib/libprotobuf-c.dylib)  # dark magic used here TODO REWRITE!
add_dependencies(protobuf-c GenerateProtobuf)

find_package(Python COMPONENTS Interpreter REQUIRED)

add_custom_target(lab3-test
        COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test.py
        DEPENDS lab3-client lab3-server ${CMAKE_CURRENT_SOURCE_DIR}/test/test.py
        )

configure_file(test/records.json ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
