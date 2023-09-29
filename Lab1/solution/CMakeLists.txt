file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        src/*.c
        src/*.h
        include/*.h
        )

add_executable(lab1 ${sources})
target_include_directories(lab1 PRIVATE src include)
