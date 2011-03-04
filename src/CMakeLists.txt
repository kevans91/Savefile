cmake_minimum_required(VERSION 2.6)

add_definitions(-g)

set(SOURCE_FILES 
	main.cpp)

set(HEADER_FILES
	args.h
	netbuf.h
	savefile.h)

add_executable(Savefile
	${SOURCE_FILES}
	${HEADER_FILES})
