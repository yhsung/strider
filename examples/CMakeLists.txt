# Example programs
message(STATUS "Building example programs")

# CPU feature detection example
add_executable(cpu_info cpu_info.c)
target_link_libraries(cpu_info PRIVATE strider)
