cmake_minimum_required(VERSION 3.5)
project(libProcessSafeQueue)

SET(CMAKE_CXX_FLAGS "-std=c++14 -O3 -Wall ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# find_package(catkin REQUIRED COMPONENTS
#   roscpp
#   dataspeed_can_msgs
# )
# find_package(GTest REQUIRED)

include_directories(
    include
)

add_library(processSafeQueue SHARED
    src/process_safe_queue.c
)

target_link_libraries(processSafeQueue
    rt
)

##################build examples####################
add_executable(enqueue_demo
    examples/enqueue_demo.c
)
target_link_libraries(enqueue_demo
    processSafeQueue
    pthread
)

add_executable(dequeue_demo
    examples/dequeue_demo.c
)
target_link_libraries(dequeue_demo
    processSafeQueue
    pthread
)

add_executable(clearqueue_demo
    examples/clearqueue_demo.c
)
target_link_libraries(clearqueue_demo
    processSafeQueue
    pthread
)

##################build units test####################
add_executable(units_test
  tests/units_test.cpp
)
target_link_libraries(units_test
  gtest
  gtest_main
  processSafeQueue
  rt
  pthread
)

install(TARGETS processSafeQueue LIBRARY DESTINATION lib/)
install(TARGETS units_test DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})
