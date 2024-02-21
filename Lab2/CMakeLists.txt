cmake_minimum_required(VERSION 3.25)
project(Lab2)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(runtime)

set(antlr4-GENERATED_SRC
        ${PROJECT_SOURCE_DIR}/generated/GraphQLLexer.cpp
        ${PROJECT_SOURCE_DIR}/generated/GraphQLParser.cpp
        ${PROJECT_SOURCE_DIR}/generated/GraphQLBaseListener.cpp
        ${PROJECT_SOURCE_DIR}/generated/GraphQLBaseVisitor.cpp
        ${PROJECT_SOURCE_DIR}/generated/GraphQLListener.cpp
        ${PROJECT_SOURCE_DIR}/generated/GraphQLVisitor.cpp)

add_custom_target(GenerateParser DEPENDS ${antlr4-GENERATED_SRC})
add_custom_command(OUTPUT ${antlr4-GENERATED_SRC}
        COMMAND
        ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/generated/
        COMMAND
        java -jar ${PROJECT_SOURCE_DIR}/antlr-4.13.1-complete.jar -Werror -Dlanguage=Cpp -listener -visitor -o ${PROJECT_SOURCE_DIR}/generated/ -package graphql ${PROJECT_SOURCE_DIR}/GraphQL.g4
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        DEPENDS GraphQL.g4)



foreach(src_file ${antlr4-GENERATED_SRC})
    set_source_files_properties(
            ${src_file}
            PROPERTIES
            GENERATED TRUE
    )
endforeach(src_file ${antlr4-GENERATED_SRC})


file(GLOB_RECURSE antlr4-runtime CONFIGURE_DEPENDS
        ${PROJECT_SOURCE_DIR}/runtime/src/*.h
        ${PROJECT_SOURCE_DIR}/runtime/src/misc/*.h
        ${PROJECT_SOURCE_DIR}/runtime/src/atn/*.h
        ${PROJECT_SOURCE_DIR}/runtime/src/dfa/*.h
        ${PROJECT_SOURCE_DIR}/runtime/src/tree/*.h
        ${PROJECT_SOURCE_DIR}/runtime/src/support/*.h
        ${PROJECT_SOURCE_DIR}/generated/*.h

        ${PROJECT_SOURCE_DIR}/runtime/src/*.cpp
        ${PROJECT_SOURCE_DIR}/runtime/src/misc/*.cpp
        ${PROJECT_SOURCE_DIR}/runtime/src/atn/*.cpp
        ${PROJECT_SOURCE_DIR}/runtime/src/dfa/*.cpp
        ${PROJECT_SOURCE_DIR}/runtime/src/tree/*.cpp
        ${PROJECT_SOURCE_DIR}/runtime/src/support/*.cpp
        ${PROJECT_SOURCE_DIR}/generated/*.cpp
        )

add_library(antlr4-runtime SHARED ${antlr4-runtime})
target_include_directories(antlr4-runtime PUBLIC
        ${PROJECT_SOURCE_DIR}/runtime/src
        ${PROJECT_SOURCE_DIR}/runtime/src/misc
        ${PROJECT_SOURCE_DIR}/runtime/src/atn
        ${PROJECT_SOURCE_DIR}/runtime/src/dfa
        ${PROJECT_SOURCE_DIR}/runtime/src/tree
        ${PROJECT_SOURCE_DIR}/runtime/src/support
        ${PROJECT_SOURCE_DIR}/generated)

set(antlr4_SRC
        ${PROJECT_SOURCE_DIR}/main.cpp
        ${antlr4-GENERATED_SRC})





add_executable(Lab2 ${antlr4_SRC})

add_dependencies(Lab2 GenerateParser)

target_link_libraries(Lab2 antlr4_shared antlr4-runtime)

install(TARGETS Lab2
        DESTINATION "share"
        COMPONENT dev)
