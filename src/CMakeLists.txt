cmake_minimum_required(VERSION 2.6)

add_definitions(-g)

set(SOURCE_FILES
	datatypes.cpp 
	main.cpp
	savefile.cpp)

set(HEADER_FILES
	args.h
	datatypes.h
	netbuf.h
	entry.h
	savefile.h
	readable.h)

add_executable(Savefile
	${SOURCE_FILES}
	${HEADER_FILES})
