# FreeRDS - Free remote desktop service stack
#
# Copyright 2015 Thinstuff Technologies GmbH
# Copyright 2015 Bernhard Miklautz <bernhard.miklautz@thincast.com>
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

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

macro (checkCFlag FLAG)
	CHECK_C_COMPILER_FLAG("${FLAG}" CFLAG${FLAG})
	if(CFLAG${FLAG})
		  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
	endif()
endmacro()

macro (checkCXXFlag FLAG)
	CHECK_CXX_COMPILER_FLAG("${FLAG}" CXXFLAG${FLAG})
	if(CXXFLAG${FLAG})
		  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
	endif()
endmacro()

set(WARNING_FLAGS "-Waddress -Warray-bounds -Wchar-subscripts -Wformat -Wreturn-type -Wunused -Wunused-function  -Wunused-label -Wunused-parameter -Wunused-value  -Wunused-variable -Wunreachable-code -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-braces -Wnonnull -Wvolatile-register-var -Wstrict-aliasing -Wclobbered -Wempty-body -Wignored-qualifiers -Wmissing-parameter-type -Woverride-init -Wsign-compare -Wtype-limits -Wuninitialized -Wcast-qual -Wformat-security -Wimplicit-function-declaration -Wimplicit-int -Wmissing-noreturn -Wredundant-decls -Winline")
#disabled
#-Wvariadic-macros - to much noise because of wlog ;(
#-Wshadow thrift/protobuf generated files generate a log of noise
#-Wunknown-pragmas - use clang pragmas where required to remove warnings where
#                    where gcc creates warnings
string(REPLACE " " ";" WARNING_FLAGS ${WARNING_FLAGS})

foreach(FLAG ${WARNING_FLAGS})
	CheckCFlag(${FLAG})
	CheckCXXFlag(${FLAG})
endforeach()

message("Using CFLAGS ${CMAKE_C_FLAGS}")
message("Using CXXFLAGS ${CMAKE_CXX_FLAGS}")

if (ENABLE_WARNING_ERROR)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
endif()
