# Eigen consumed the way its "Using Eigen in CMake Projects" page teaches:
# find_package(Eigen3 ... NO_MODULE) and link the Eigen3::Eigen target.
cmake_minimum_required(VERSION 3.16)

project(app VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_executable(app src/main.cpp)
target_link_libraries(app PRIVATE Eigen3::Eigen)

# `ctest` runs the program; it exits non-zero when any check fails.
enable_testing()
add_test(NAME linear_algebra_checks COMMAND app)

install(TARGETS app RUNTIME DESTINATION bin)
