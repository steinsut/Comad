add_executable(Tests Tests.cpp)

target_link_libraries(Tests PRIVATE Comad)

set_target_properties(Tests PROPERTIES 
                        CXX_STANDARD 20
                        CXX_STANDARD_REQUIRED ON
                        CXX_EXTENSIONS OFF
                        INTERPROCEDURAL_OPTIMIZATION $<IF:$<CONFIG:Debug>, ON, OFF>)