cmake_minimum_required(VERSION 3.19)
project(cvgl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")

set(CUE_DIR "${PROJECT_SOURCE_DIR}/src/cues/animism")

# setup BlackMagic
#set(DECKLINK_SDK "${PROJECT_SOURCE_DIR}/../Blackmagic DeckLink SDK 11.7")
set(DECKLINK_SDK "${PROJECT_SOURCE_DIR}/../Blackmagic DeckLink SDK 12.1")
set(DECKLINK_PLATFORM_INCLUDE "${DECKLINK_SDK}/Examples/Linux" )
set(DECKLINK_SDK_INCLUDE_DIR "${DECKLINK_SDK}/Linux/include" )

#set(PLATFORM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Linux)

set(BLACKMAGIC_PLATFORM_SOURCES
            ${DECKLINK_SDK_INCLUDE_DIR}/DeckLinkAPIDispatch.cpp
            ${DECKLINK_PLATFORM_INCLUDE}/platform.cpp
)

#${CMAKE_CURRENT_SOURCE_DIR}/Linux/platform.h
#${CMAKE_CURRENT_SOURCE_DIR}/Linux/platform.cpp

list(APPEND LIB_LIST dl pthread) #${CMAKE_DL_LIBS}


# setup OpenGL / GLFW / GLAD / OPENCV
set(GLAD_DIR "${PROJECT_SOURCE_DIR}/../glad")

list(APPEND LIB_LIST ${CMAKE_DL_LIBS} )

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

list(APPEND LIB_LIST glfw OpenGL::GL ${OpenCV_LIBS} )

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_package(Eigen3 REQUIRED)
list(APPEND LIB_LIST Eigen3::Eigen)

message(STATUS "    eigen include path: ${Eigen3_INCLUDE_DIRS}")

add_library(libcvgl STATIC
    src/mixer/cvglMixer.cpp
    src/camera/cvglCameraBase.cpp
    src/camera/cvglCVCamera.cpp
    src/camera/cvglDeckLinkCamera.cpp
    src/gl/cvglCallbacks.cpp
    src/gl/cvglContext.cpp
    src/gl/cvglObject.cpp
    src/gl/cvglTexture.cpp
    src/gl/cvglVAO.cpp
    src/gl/cvglVertex.cpp
    src/gl/delabella.cpp
    src/cv/cvglCV.cpp
    src/cv/cvglConvexHull.cpp
    src/cv/cvglAnalysisData.cpp
    src/cv/cvglGaussian.cpp
    src/osc/MapOSC.cpp
    src/udp/cvglUDPServer.cpp
    src/utils/cvglThreadPool.cpp
    src/utils/cvglConversions.cpp
    src/utils/cvglHelperFunctions.cpp
    src/utils/cvglRandom.cpp
    src/utils/cvglLookupFigure.cpp
    src/cues/cvglCues.cpp

    ${GLAD_DIR}/src/glad.c
    ${BLACKMAGIC_PLATFORM_SOURCES}
)

target_include_directories(libcvgl PUBLIC ${GLAD_DIR}/include )
target_include_directories(libcvgl PUBLIC ${DECKLINK_SDK_INCLUDE_DIR} PUBLIC ${DECKLINK_PLATFORM_INCLUDE})
target_include_directories(libcvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_link_libraries(libcvgl ${LIB_LIST}  )

list(APPEND LIB_LIST libcvgl)

file(GLOB RES_FILES src/*.fs src/*.vs )
file(GLOB CUE_FILES ${CUE_DIR}/*.cpp)

add_executable(cvgl
    ${RES_FILES}
    ${CUE_FILES}
    src/cvgl_main.cpp
    src/cvglMainProcess.cpp
)

target_include_directories(cvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include )
target_include_directories(cvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )
target_include_directories(cvgl PUBLIC ${CUE_DIR})

#target_include_directories(cvgl PRIVATE ${GLAD_DIR}/include )
#target_include_directories(cvgl PRIVATE ${DECKLINK_SDK_INCLUDE_DIR} PRIVATE ${DECKLINK_PLATFORM_INCLUDE})

# this is a funny idea, maybe useful someday, but for now too much work
#target_compile_definitions(cvgl PRIVATE "CUE_FILE_NAMES=\"yo\"")
#target_compile_definitions(main PRIVATE "MY_FILE_PATH=\"${MY_FILE_PATH}\"")

target_link_libraries(cvgl libcvgl)

message("CMAKE_C_FLAGS_DEBUG is ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_C_FLAGS_RELEASE is ${CMAKE_C_FLAGS_RELEASE}")
message("Shders found ${RES_FILES}")
message("Cues found ${CUE_FILES}")

