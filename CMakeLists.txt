cmake_minimum_required(VERSION 3.16)
project(vizionsdk_ros2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ament_cmake REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)

find_package(vizionsdk CONFIG QUIET)
if(NOT TARGET vizionsdk::VizionSDK)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(VIZIONSDK REQUIRED IMPORTED_TARGET vizionsdk)
endif()

add_library(vizionsdk_ros2_conversions INTERFACE)
target_include_directories(vizionsdk_ros2_conversions
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
if(TARGET vizionsdk::VizionSDK)
  target_link_libraries(vizionsdk_ros2_conversions INTERFACE vizionsdk::VizionSDK)
else()
  target_link_libraries(vizionsdk_ros2_conversions INTERFACE PkgConfig::VIZIONSDK)
  foreach(include_dir IN LISTS VIZIONSDK_INCLUDE_DIRS)
    target_include_directories(vizionsdk_ros2_conversions INTERFACE "${include_dir}/vizionsdk")
  endforeach()
endif()

add_library(vizionsdk_ros2_node
  src/vizionsdk_camera_node.cpp
)
target_include_directories(vizionsdk_ros2_node
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vizionsdk_ros2_node vizionsdk_ros2_conversions)
ament_target_dependencies(vizionsdk_ros2_node diagnostic_msgs rcl_interfaces rclcpp sensor_msgs)

add_executable(vizionsdk_camera_node
  src/main.cpp
)
target_link_libraries(vizionsdk_camera_node vizionsdk_ros2_node)
ament_target_dependencies(vizionsdk_camera_node rclcpp)

add_executable(list_devices
  src/list_devices.cpp
)
target_link_libraries(list_devices vizionsdk_ros2_conversions)

if(BUILD_TESTING)
  include(CTest)
  add_executable(test_conversions
    test/test_conversions.cpp
  )
  target_link_libraries(test_conversions PRIVATE vizionsdk_ros2_conversions)
  add_test(NAME test_conversions COMMAND test_conversions)

  add_executable(test_list_devices
    test/test_list_devices.cpp
  )
  target_link_libraries(test_list_devices PRIVATE vizionsdk_ros2_conversions)
  add_test(NAME test_list_devices COMMAND test_list_devices)

  add_executable(test_isp_controls
    test/test_isp_controls.cpp
  )
  target_link_libraries(test_isp_controls PRIVATE vizionsdk_ros2_conversions)
  add_test(NAME test_isp_controls COMMAND test_isp_controls)
endif()

install(TARGETS
  vizionsdk_ros2_node
  vizionsdk_camera_node
  list_devices
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

install(DIRECTORY rviz
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
