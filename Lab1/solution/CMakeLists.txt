file(GLOB_RECURSE sources CONFIGURE_DEPENDS
        src/*.c
        src/*.h
        include/*.h
        )

add_library(main_sources SHARED ${sources})
target_include_directories(main_sources PUBLIC src include)

add_executable(lab1 ${sources})
target_link_libraries(lab1 PRIVATE main_sources)
#target_include_directories(lab1 PRIVATE src include)
