cmake_minimum_required(VERSION 3.10)
project(umi-lerobot)

# 1. Path to ORBSLAM3
set(ORB_SLAM3_DIR "/home/thanh-huynhbao/git/umi-lerobot/submodules/ORB_SLAM3")
# 2. Dependencies
find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
    find_package(OpenCV 3.0 REQUIRED)
endif()
find_package(Eigen3 3.1.0 REQUIRED)
find_package(Pangolin REQUIRED)
# 3. Include ORBSLAM3
include_directories(
    ${ORB_SLAM3_DIR}
    ${ORB_SLAM3_DIR}/include
    ${ORB_SLAM3_DIR}/include/CameraModels
    ${ORB_SLAM3_DIR}/Thirdparty/Sophus
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)
# 4. Find build
set(ORB_SLAM3_LIBS
    ${ORB_SLAM3_DIR}/lib/libORB_SLAM3.so
    ${ORB_SLAM3_DIR}/Thirdparty/DBoW2/lib/libDBoW2.so
    ${ORB_SLAM3_DIR}/Thirdparty/g2o/lib/libg2o.so
    ${OpenCV_LIBS}
    ${EIGEN3_LIBS}
    ${Pangolin_LIBRARIES}
    -lboost_serialization
    -lcrypto
)
# 5. File build
add_executable(run_tum run_tum.cc)
target_link_libraries(run_tum ${ORB_SLAM3_LIBS})

set_target_properties(run_tum PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "/home/thanh-huynhbao/git/umi-lerobot/slam"
)

add_executable(run_tum_vi run_tum_vi.cc)
target_link_libraries(run_tum_vi ${ORB_SLAM3_LIBS})

set_target_properties(run_tum_vi PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "/home/thanh-huynhbao/git/umi-lerobot/slam"
)