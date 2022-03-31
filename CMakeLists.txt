cmake_minimum_required(VERSION 3.21)
project(FFmpegVdpauTest VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)

# external project
include(ExternalProject)

# ffmpeg
ExternalProject_Add(ffmpeg_project
        PREFIX ${CMAKE_SOURCE_DIR}/third-party/ffmpeg
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/third-party/ffmpeg
        BINARY_DIR ${CMAKE_SOURCE_DIR}/third-party/ffmpeg/build
        INSTALL_DIR ${CMAKE_SOURCE_DIR}/third-party/ffmpeg/install
        CONFIGURE_COMMAND ../configure --prefix=${CMAKE_SOURCE_DIR}/third-party/ffmpeg/install
        BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
        INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
        )

ExternalProject_Get_Property(ffmpeg_project INSTALL_DIR)
set(FFMPEG_INCLUDE_DIR ${INSTALL_DIR}/include)
set(FFMPEG_LIBRARY_DIR ${INSTALL_DIR}/lib)

add_library(avdevice STATIC IMPORTED)
set_property(TARGET avdevice PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libavdevice.a)
add_dependencies(avdevice ffmpeg_project)

add_library(avcodec STATIC IMPORTED)
set_property(TARGET avcodec PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libavcodec.a)
add_dependencies(avcodec ffmpeg_project)

add_library(avfilter STATIC IMPORTED)
set_property(TARGET avfilter PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libavfilter.a)
add_dependencies(avfilter ffmpeg_project)

add_library(avformat STATIC IMPORTED)
set_property(TARGET avformat PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libavformat.a)
add_dependencies(avformat ffmpeg_project)

add_library(avutil STATIC IMPORTED)
set_property(TARGET avutil PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libavutil.a)
add_dependencies(avutil ffmpeg_project)

add_library(swresample STATIC IMPORTED)
set_property(TARGET swresample PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libswresample.a)
add_dependencies(swresample ffmpeg_project)

add_library(swscale STATIC IMPORTED)
set_property(TARGET swscale PROPERTY IMPORTED_LOCATION ${FFMPEG_LIBRARY_DIR}/libswscale.a)
add_dependencies(swscale ffmpeg_project)

# vdpau
find_path(VDPAU_INCLUDE_DIR
        vdpau/vdpau.h vdpau/vdpau_x11.h
        /usr/include /usr/local/include
        )
find_library(VDPAU_LIBRARIES
        NAMES "vdpau"
        PATHS /usr/lib /usr/local/lib
        )

# x11
find_package(X11)
if (NOT ${X11_FOUND})
    message(FATAL_ERROR "X11 not found")
endif()

# va & va-drm & va-x11
find_path(VAAPI_INCLUDE_DIR
        va/va.h va/va_x11.h
        /usr/include /usr/local/include
        )
find_library(VA_LIBRARIES
        NAMES "va"
        PATHS /usr/lib /usr/local/lib
        )
find_library(VA-DRM_LIBRARIES
        NAMES "va-drm"
        PATHS /usr/lib /usr/local/lib
        )
find_library(VA-X11_LIBRARIES
        NAMES "va-x11"
        PATHS /usr/lib /usr/local/lib
        )

# zlib
find_package(ZLIB)
if (NOT ${ZLIB_FOUND})
    message(FATAL_ERROR "ZLIB not found")
endif()

# lzma
find_package(LibLZMA)
if (NOT ${LIBLZMA_FOUND})
    message(FATAL_ERROR "LibLZMA not found")
endif()

# bz
find_package(BZip2)
if (NOT ${BZIP2_FOUND})
    message(FATAL_ERROR "BZIP2 not found")
endif()

# opencv
ExternalProject_Add(opencv_project
        PREFIX ${CMAKE_SOURCE_DIR}/third-party/opencv
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/third-party/opencv
        BINARY_DIR ${CMAKE_SOURCE_DIR}/third-party/opencv/build
        INSTALL_DIR ${CMAKE_SOURCE_DIR}/third-party/opencv/install
        CMAKE_ARGS
            -DBUILD_DOCS:BOOL=OFF
            -DBUILD_EXAMPLES:BOOL=OFF
            -DBUILD_PACKAGE:BOOL=OFF
            -DBUILD_SHARED_LIBS:BOOL=OFF
            -DBUILD_TESTS:BOOL=OFF
            -DWITH_FFMPEG:BOOL=OFF
            -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_SOURCE_DIR}/third-party/opencv/install
        )

ExternalProject_Get_Property(opencv_project INSTALL_DIR)
set(OPENCV_INCLUDE_DIR ${INSTALL_DIR}/include/opencv4)
set(OPENCV_LIBRARY_DIR ${INSTALL_DIR}/lib)

add_library(opencv_core STATIC IMPORTED)
set_property(TARGET opencv_core PROPERTY IMPORTED_LOCATION ${OPENCV_LIBRARY_DIR}/libopencv_core.a)
add_dependencies(opencv_core opencv_project)

add_library(opencv_imgcodecs STATIC IMPORTED)
set_property(TARGET opencv_imgcodecs PROPERTY IMPORTED_LOCATION ${OPENCV_LIBRARY_DIR}/libopencv_imgcodecs.a)
add_dependencies(opencv_imgcodecs opencv_project)

add_library(opencv_imgproc STATIC IMPORTED)
set_property(TARGET opencv_imgproc PROPERTY IMPORTED_LOCATION ${OPENCV_LIBRARY_DIR}/libopencv_imgproc.a)
add_dependencies(opencv_imgproc opencv_project)

# jpeg
find_package(JPEG)
if (NOT ${JPEG_FOUND})
    message(FATAL_ERROR "JPEG not found")
endif()

# png
find_package(PNG)
if (NOT ${PNG_FOUND})
    message(FATAL_ERROR "PNG not found")
endif()

# tiff
find_package(TIFF)
if (NOT ${TIFF_FOUND})
    message(FATAL_ERROR "TIFF not found")
endif()

# openjp2
find_package(OpenJPEG)
if (NOT ${OpenJPEG_FOUND})
    message(FATAL_ERROR "OpenJPEG not found")
endif()

# webp
find_path(WEBP_INCLUDE_DIR
        webp/decode.h
        /usr/include /usr/local/include
        )
find_library(WEBP_LIBRARIES
        NAMES "webp"
        PATHS /usr/lib /usr/local/lib
        )

add_executable(FFmpegVdpauTest main.cpp)
target_include_directories(FFmpegVdpauTest PRIVATE ${FFMPEG_INCLUDE_DIR} ${OPENCV_INCLUDE_DIR})
target_link_libraries(FFmpegVdpauTest
        avformat
        avcodec
        avutil
        swresample
        ${VA_LIBRARIES}
        ${VA-DRM_LIBRARIES}
        ${VA-X11_LIBRARIES}
        ${VDPAU_LIBRARIES}
        ${X11_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${LIBLZMA_LIBRARIES}
        ${BZIP2_LIBRARIES}
        opencv_imgcodecs
        opencv_imgproc
        opencv_core
        ${OPENCV_LIBRARY_DIR}/opencv4/3rdparty/libippiw.a
        ${OPENCV_LIBRARY_DIR}/opencv4/3rdparty/libippicv.a
        ${OPENCV_LIBRARY_DIR}/opencv4/3rdparty/libittnotify.a
        ${OPENCV_LIBRARY_DIR}/opencv4/3rdparty/libIlmImf.a
        ${JPEG_LIBRARIES}
        ${PNG_LIBRARIES}
        ${TIFF_LIBRARIES}
        ${OPENJPEG_LIBRARIES}
        ${WEBP_LIBRARIES}
        )
