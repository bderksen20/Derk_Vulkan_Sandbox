cmake_minimum_required(VERSION 3.2)

set(project_name 1-Devices)
project(${project_name})

set(HAVE_LIBC TRUE)
set(src
	Main.cpp
)

set(headers

)

add_executable(${project_name} ${src} ${headers})

find_package(Vulkan)

if(Vulkan_FOUND)
	target_include_directories(${project_name} PRIVATE Vulkan::Vulkan)
	target_link_libraries(${project_name} Vulkan::Vulkan)
endif()