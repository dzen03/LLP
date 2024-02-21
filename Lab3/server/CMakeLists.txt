file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        *.c
        *.h
        )

add_executable(lab3-server ${sources} ${antlr4-GENERATED_SRC} ${protobuf-c-GENERATED_SRC})

add_dependencies(lab3-server GenerateParser GenerateProtobuf)

target_link_libraries(lab3-server main_sources protobuf-c)
