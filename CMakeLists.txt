cmake_minimum_required (VERSION 3.8)

project ("kazama")
add_executable (${CMAKE_PROJECT_NAME} "kazama.cpp")

find_package(Boost COMPONENTS
		thread REQUIRED)

if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(${CMAKE_PROJECT_NAME} 
		Boost::thread)
endif()