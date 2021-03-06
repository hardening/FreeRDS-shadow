# FreeRDP: A Remote Desktop Protocol Implementation
# FreeRDP Servers
#
# Copyright 2015 David FORT <contact@hardening-consulting.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(MODULE_NAME "freerds-shadow")
set(MODULE_PREFIX "FREERDP_SERVER_SHADOW")


set(XEXT_FEATURE_TYPE "RECOMMENDED")
set(XEXT_FEATURE_PURPOSE "X11 extension")
set(XEXT_FEATURE_DESCRIPTION "X11 core extensions")

set(XSHM_FEATURE_TYPE "RECOMMENDED")
set(XSHM_FEATURE_PURPOSE "X11 shared memory")
set(XSHM_FEATURE_DESCRIPTION "X11 shared memory extension")

set(XINERAMA_FEATURE_TYPE "RECOMMENDED")
set(XINERAMA_FEATURE_PURPOSE "multi-monitor")
set(XINERAMA_FEATURE_DESCRIPTION "X11 multi-monitor extension")

set(XTEST_FEATURE_TYPE "RECOMMENDED")
set(XTEST_FEATURE_PURPOSE "X11 input event injection")
set(XTEST_FEATURE_DESCRIPTION "X11 input event injection extension")

set(XCURSOR_FEATURE_TYPE "RECOMMENDED")
set(XCURSOR_FEATURE_PURPOSE "cursor")
set(XCURSOR_FEATURE_DESCRIPTION "X11 cursor extension")

set(XFIXES_FEATURE_TYPE "RECOMMENDED")
set(XFIXES_FEATURE_PURPOSE "X11 region")
set(XFIXES_FEATURE_DESCRIPTION "X11 region fix extension")

set(XRANDR_FEATURE_TYPE "RECOMMENDED")
set(XRANDR_FEATURE_PURPOSE "X11 resize, rotate and reflect")
set(XRANDR_FEATURE_DESCRIPTION "X11 resize, rotate and reflect extension")

set(XDAMAGE_FEATURE_TYPE "RECOMMENDED")
set(XDAMAGE_FEATURE_PURPOSE "X11 region damage")
set(XDAMAGE_FEATURE_DESCRIPTION "X11 region damage extension")

find_feature(Xext ${XEXT_FEATURE_TYPE} ${XEXT_FEATURE_PURPOSE} ${XEXT_FEATURE_DESCRIPTION})
find_feature(XShm ${XSHM_FEATURE_TYPE} ${XSHM_FEATURE_PURPOSE} ${XSHM_FEATURE_DESCRIPTION})
find_feature(XTest ${XTEST_FEATURE_TYPE} ${XTEST_FEATURE_PURPOSE} ${XTEST_FEATURE_DESCRIPTION})
find_feature(Xfixes ${XFIXES_FEATURE_TYPE} ${XFIXES_FEATURE_PURPOSE} ${XFIXES_FEATURE_DESCRIPTION})
find_feature(XRandR ${XRANDR_FEATURE_TYPE} ${XRANDR_FEATURE_PURPOSE} ${XRANDR_FEATURE_DESCRIPTION})
find_feature(Xdamage ${XDAMAGE_FEATURE_TYPE} ${XDAMAGE_FEATURE_PURPOSE} ${XDAMAGE_FEATURE_DESCRIPTION})
find_feature(Xcursor ${XCURSOR_FEATURE_TYPE} ${XCURSOR_FEATURE_PURPOSE} ${XCURSOR_FEATURE_DESCRIPTION})
find_feature(Xinerama ${XINERAMA_FEATURE_TYPE} ${XINERAMA_FEATURE_PURPOSE} ${XINERAMA_FEATURE_DESCRIPTION})

include_directories(${X11_INCLUDE_DIRS})
list(APPEND ${MODULE_PREFIX}_X11_LIBS ${X11_LIBRARIES})
list(APPEND ${MODULE_PREFIX}_X11_LIBS X11)

if(WITH_XSHM)
	add_definitions(-DWITH_XSHM)
	include_directories(${XSHM_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XSHM_LIBRARIES})
endif()

if(WITH_XEXT)
	add_definitions(-DWITH_XEXT)
	include_directories(${XEXT_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XEXT_LIBRARIES})
endif()

if(WITH_XINERAMA)
	add_definitions(-DWITH_XINERAMA)
	include_directories(${XINERAMA_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XINERAMA_LIBRARIES})
endif()

if(WITH_XCURSOR)
	add_definitions(-DWITH_XCURSOR)
	include_directories(${XCURSOR_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XCURSOR_LIBRARIES})
endif()

if(WITH_XDAMAGE)
	add_definitions(-DWITH_XDAMAGE)
	include_directories(${XDAMAGE_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XDAMAGE_LIBRARIES})
endif()

if(WITH_XFIXES)
	add_definitions(-DWITH_XFIXES)
	include_directories(${XFIXES_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XFIXES_LIBRARIES})
endif()

if(WITH_XTEST)
	add_definitions(-DWITH_XTEST)
	include_directories(${XTEST_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XTEST_LIBRARIES})
endif()

if(WITH_XRANDR)
	add_definitions(-DWITH_XRANDR)
	include_directories(${XRANDR_INCLUDE_DIRS})
	list(APPEND ${MODULE_PREFIX}_X11_LIBS ${XRANDR_LIBRARIES})
endif()


include_directories(".")

set(${MODULE_PREFIX}_SRCS
	shadow_main.c
	shadow_x11.c
	shadow_x11.h
)

add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})


list(APPEND ${MODULE_PREFIX}_LIBS freerds-backend)
list(APPEND ${MODULE_PREFIX}_LIBS freerdp)
list(APPEND ${MODULE_PREFIX}_LIBS winpr)
list(APPEND ${MODULE_PREFIX}_LIBS ${${MODULE_PREFIX}_X11_LIBS})

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})

install(TARGETS ${MODULE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

