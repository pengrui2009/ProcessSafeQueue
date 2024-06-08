cmake_minimum_required(VERSION 3.5)
project(libProcessSafeQueue)

# SET(CMAKE_CXX_FLAGS "-std=c11 -O3 -Wall ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# find_package(catkin REQUIRED COMPONENTS
#   roscpp
#   dataspeed_can_msgs
# )

include_directories(
    include
)

add_library(processSafeQueue SHARED
    src/process_safe_queue.c
)

target_link_libraries(processSafeQueue
    rt
)

# add_executable(kvaser_can_bridge
#   src/kvaser_can_bridge.cpp
# )
# target_link_libraries(kvaser_can_bridge
#   ros_${PROJECT_NAME}
#   ${catkin_LIBRARIES}
# )


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

install(TARGETS processSafeQueue LIBRARY DESTINATION lib/)
install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME})

# install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})