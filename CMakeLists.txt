#
# MIT License
#
# Copyright(c) 2023-present All contributors of SGL
# Document reference link: docs directory
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.12)
project(sgl VERSION 0.0.1 LANGUAGES C)

include(GNUInstallDirs)
include(CheckIncludeFile)

set(SGL_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(SGL_SOURCE_PATH ${PROJECT_SOURCE_DIR}/source)
set(SGL_HEADER_PATH ${SGL_SOURCE_PATH}/include)
set(SGL_GENERATED_PATH ${PROJECT_BINARY_DIR}/generated)
set(SGL_EXPORT_PATH ${PROJECT_BINARY_DIR}/export)

###################### SGL Configuration ######################

set(CONFIG_SGL_BOOT_LOGO        1)
set(CONFIG_SGL_BOOT_ANIMATION   1)

set(CONFIG_SGL_COLOR16_SWAP     1)
set(CONFIG_SGL_DEBUG            1)
set(SGL_LOG_LEVEL               3)
set(CONFIG_SGL_LOG_COLOR        1)
set(CONFIG_SGL_ANIMATION        1)
set(SGL_PANEL_PIXEL_DEPTH    "16")
set(SGL_EVENT_QUEUE_SIZE       16)
set(SGL_SYSTICK_MS             10)
set(SGL_DIRTY_AREA_THRESHOLD   64)
set(SGL_HEAP_ALGO           other)
set(SGL_HEAP_MEMORY_SIZE        0)


set(CONFIG_SGL_EXTERNAL_PIXMAP  1)
set(CONFIG_SGL_OBJ_USE_NAME     1)
set(CONFIG_SGL_FONT_SONG23      0)
set(CONFIG_SGL_FONT_CONSOLAS14  1)
set(CONFIG_SGL_FONT_CONSOLAS23  0)
set(CONFIG_SGL_FONT_CONSOLAS24  0)
set(CONFIG_SGL_FONT_CONSOLAS32  0)

set(CONFIG_SGL_FONT_OPPOSANS_MEDIUM_24 1)
set(CONFIG_SGL_FONT_OPPOSANS_MEDIUM_16 1)
set(CONFIG_SGL_FONT_HARMONY_16 1)

###############################################################

include(${PROJECT_SOURCE_DIR}/build.cmake)

add_library(sgl_object OBJECT ${SGL_LIBRARY})
add_library(sgl ALIAS sgl_object)
target_include_directories(sgl_object PUBLIC
    ${SGL_INCLUDE_DIRS}
)

target_link_libraries(sgl_object PRIVATE
    freertos_kernel
)
