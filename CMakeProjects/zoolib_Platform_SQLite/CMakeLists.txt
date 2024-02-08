cmake_minimum_required(VERSION 3.10.2)

set(ZOOLIB_CXX ../..)

set(SourceDir ${ZOOLIB_CXX}/Platform/zoolib/SQLite)

set (SourceFiles	
	${SourceDir}/SQLite.cpp
	${SourceDir}/SQLite.h
	)

source_group("" FILES ${SourceFiles})

include_directories(${ZOOLIB_CXX} ${ZOOLIB_CXX}/Core ${ZOOLIB_CXX}/Portable ${ZOOLIB_CXX}/Platform)

add_library(ZooLib_Platform_SQLite STATIC

	${SourceFiles}
	)
