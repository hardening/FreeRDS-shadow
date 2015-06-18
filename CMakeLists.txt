# FreeRDS - Free remote desktop service stack
#
# Copyright 2015 Thinstuff Technologies GmbH
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

cmake_minimum_required(VERSION 2.8)

project(FreeRDS-shadow C)

set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(CheckCmakeCompat)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckStructHasMember)
include(FindPkgConfig)
include(TestBigEndian)

include(FindFeature)
include(FeatureSummary)
include(GNUInstallDirsWrapper)
include(CMakePackageConfigHelpers)
include(ConfigOptions)

set(SHADOW_VERSION_MAJOR "0")
set(SHADOW_VERSION_MINOR "0")
set(SHADOW_VERSION_REVISION "1")
set(SHADOW_VERSION "${SHADOW_VERSION_MAJOR}.${SHADOW_VERSION_MINOR}")
set(SHADOW_VERSION_FULL "${SHADOW_VERSION}.${SHADOW_VERSION_REVISION}")

include(GetGitRevisionDescription)

git_describe(GIT_REVISION --match "v[0-9]*" --abbrev=4 --tags --always)
message(STATUS "Git Revision ${GIT_REVISION}")

include(FreeRDSCompilerFlags)


set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}:\$ORIGIN/..")

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()


set(WINPR_FEATURE_TYPE "REQUIRED")
set(WINPR_FEATURE_PURPOSE "WINPR libraries and headers")
set(WINPR_FEATURE_DESCRIPTION "WINPR")
find_feature(WinPR ${WINPR_FEATURE_TYPE} ${WINPR_FEATURE_PURPOSE} ${WINPR_FEATURE_DESCRIPTION})
include_directories(${WinPR_INCLUDE_DIR})

set(FREERDP_FEATURE_TYPE "REQUIRED")
set(FREERDP_FEATURE_PURPOSE "FreeRDP libraries and headers")
set(FREERDP_FEATURE_DESCRIPTION "FreeRDP")
find_feature(FreeRDP ${FREERDP_FEATURE_TYPE} ${FREERDP_FEATURE_PURPOSE} ${FREERDP_FEATURE_DESCRIPTION})
include_directories(${FreeRDP_INCLUDE_DIR})

set(FREERDS_FEATURE_TYPE "REQUIRED")
set(FREERDS_FEATURE_PURPOSE "FreeRDS libraries and headers")
set(FREERDS_FEATURE_DESCRIPTION "FreeRDS")
find_feature(FreeRDS ${FREERDS_FEATURE_TYPE} ${FREERDS_FEATURE_PURPOSE} ${FREERDS_FEATURE_DESCRIPTION})
include_directories(${FreeRDS_INCLUDE_DIR})

add_subdirectory(src)

