cmake_minimum_required(VERSION 2.6)

add_definitions(-g)

set(SOURCE_FILES
	datatypes.cpp 
	main.cpp
	savefile.cpp)

set(HEADER_FILES
	args.h
	crc.h
	datatypes.h
	netbuf.h
	entry.h
	savefile.h
	readable.h)

add_executable(Savefile
	${SOURCE_FILES}
	${HEADER_FILES})

if(WIN32)
	target_link_libraries(Savefile ws2_32)
endif(WIN32)
