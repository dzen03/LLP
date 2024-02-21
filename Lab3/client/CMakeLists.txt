file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        *.cpp
        *.hpp
        )

add_executable(lab3-client ${sources} ${antlr4-GENERATED_SRC} ${protobuf-cpp-GENERATED_SRC})

add_dependencies(lab3-client GenerateParser GenerateProtobuf)

target_link_libraries(lab3-client antlr4_shared antlr4-runtime protobuf-cpp protobuf::libprotobuf)

configure_file(test_query ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

# protoc -I=$SRC_DIR --cpp_out=$DST_DIR $SRC_DIR/addressbook.proto
