###########
## CMake ##
###########
cmake_minimum_required(VERSION 3.9)
message(STATUS "CMake version:\t" ${CMAKE_VERSION})
message(STATUS "CMAKE_ROOT:\t" ${CMAKE_ROOT})
cmake_policy(VERSION 3.9)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}")
set(CMAKE_WARN_DEPRECATED FALSE)
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE)

#############
## Project ##
#############
project(pointcloud_processing CXX)
set(PROJECT_BINARY ${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.hpp ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.h)
file(GLOB_RECURSE CU_SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cu)
file(GLOB_RECURSE CU_HEADER_FILES ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/*.cuh)
#message(STATUS "SOURCE_FILES:\t" ${SOURCE_FILES})
#message(STATUS "HEADER_FILES:\t" ${HEADER_FILES})
#message(STATUS "CU_SOURCE_FILES:\t" ${CU_SOURCE_FILES})
#message(STATUS "CU_HEADER_FILES:\t" ${CU_HEADER_FILES})

add_library(${PROJECT_BINARY} ${SOURCE_FILES} ${HEADER_FILES} ${CU_SOURCE_FILES} ${CU_HEADER_FILES})

include_directories(include/${PROJECT_NAME})

##############
## Compiler ##
##############
# Default to C99
if(NOT CMAKE_C_STANDARD)
	set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
	set(CMAKE_CXX_STANDARD 14)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_target_properties(${PROJECT_BINARY}
	PROPERTIES
	  CXX_STANDARD 14
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
)

# Filesystem
target_link_libraries(${PROJECT_BINARY} stdc++fs)

##################
## Dependencies ##
##################
#### CUDA ####
message(STATUS "==========[ CUDA ]===========")
# Set CUDACXX if not in environment
if (NOT $ENV{CUDACXX})
	# Search for env variable CUDADIR or CUDA_HOME
	if ($ENV{CUDADIR})
		set(CUDACXX $ENV{CUDADIR}/bin/nvcc)
	elseif($ENV{CUDA_HOME})
		set(CUDACXX $ENV{CUDA_HOME}/bin/nvcc)
	endif()
endif()
# Check for CUDA
include(CheckLanguage)
check_language(CUDA)
if (CMAKE_CUDA_COMPILER)
	# CUDA Status
	enable_language(CUDA)
	message(STATUS "|  NVCC version:\t\t" ${CMAKE_CUDA_COMPILER_VERSION})
	message(STATUS "|  NVCC path:\t\t" ${CMAKE_CUDA_COMPILER})
	set(CUDA_NVCC_EXECUTABLE ${CMAKE_CUDA_COMPILER})
	# Check for and set GPU architecture
	include("cmake/CudaDetect.cmake")
	detect_installed_gpus(CUDA_NVCC_ARCHS)
	foreach(NVCC_ARCH IN LISTS CUDA_NVCC_ARCHS)
		list(APPEND CUDA_ARCH_BIN "${NVCC_ARCH} ")
	endforeach(NVCC_ARCH)
	set_property(TARGET ${PROJECT_BINARY} PROPERTY CUDA_ARCHITECTURES OFF)
	message(STATUS "|  CUDA architecture:\t\t" ${CUDA_ARCH_BIN})
	# Set CUDA language standard
	if(NOT DEFINED CMAKE_CUDA_STANDARD)
		set(CMAKE_CUDA_STANDARD 14)
		set(CMAKE_CUDA_STANDARD_REQUIRED ON)
	endif()
	# Include CUDA libraries
	target_include_directories(${PROJECT_BINARY} PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
	# Link CUDA libraries
	find_library(CUDADRIVER_LIBRARY cuda ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES}) # only for cuMemsetD32Async
	find_library(CUDART_LIBRARY cudart ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(CUBLAS_LIBRARY cublas ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(CUBLASLT_LIBRARY cublasLt ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	message(STATUS "|  CUDA cudadriver library:\t" ${CUDADRIVER_LIBRARY})
	message(STATUS "|  CUDA cudart library:\t" ${CUDART_LIBRARY})
	message(STATUS "|  CUDA cublas library:\t" ${CUBLAS_LIBRARY})
	message(STATUS "|  CUDA cublasLt library:\t" ${CUBLASLT_LIBRARY})
	# Link CUDA NPP libraries
	find_library(NPPC_LIBRARY nppc ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(NPPIF_LIBRARY nppif ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(NPPITC_LIBRARY nppitc ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(NPPIM_LIBRARY nppim ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	find_library(NPPIST_LIBRARY nppist ${CMAKE_CUDA_IMPLICIT_LINK_DIRECTORIES})
	message(STATUS "|  CUDA NPP core library:\t" ${NPPC_LIBRARY})
	message(STATUS "|  CUDA NPP filter library:\t" ${NPPIF_LIBRARY})
	message(STATUS "|  CUDA NPP threshold library:\t" ${NPPITC_LIBRARY})
	message(STATUS "|  CUDA NPP morphol. library:\t" ${NPPIM_LIBRARY})
	message(STATUS "|  CUDA NPP statistical library:\t" ${NPPIST_LIBRARY})
	target_link_libraries(${PROJECT_BINARY}
		${CUDADRIVER_LIBRARY}
		${CUDART_LIBRARY}
		${CUBLAS_LIBRARY}
		${CUBLASLT_LIBRARY}
		${NPPC_LIBRARY}
		${NPPIF_LIBRARY}
		${NPPITC_LIBRARY}
		${NPPIM_LIBRARY}
		${NPPIST_LIBRARY}
		)
else()
	message(STATUS "CUDA compiler not found! Perhaps set CUDACXX environment variable")
endif()
message(STATUS "+============================")

#### PCL ####
find_package(PCL 1.12.0 REQUIRED QUIET)
message(STATUS "+==========[ PCL ]===========")
if (PCL_FOUND)
	message(STATUS "|  PCL version:\t" ${PCL_VERSION})
	#message(STATUS "|  PCL includes: ${PCL_INCLUDE_DIRS}")
	if (CMAKE_CUDA_COMPILER AND TARGET Qt5::Core)
		# Fix -fPIC option send to nvcc compiler not to host compiler by Qt5
		# convert the fpic option in Qt5::Core over to INTERFACE_POSITION_INDEPENDENT_CODE
		get_property(core_options TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_OPTIONS)
		string(REPLACE "-fPIC" "" new_core_options ${core_options})
		set_property(TARGET Qt5::Core PROPERTY INTERFACE_COMPILE_OPTIONS ${new_core_options})
		set_property(TARGET Qt5::Core PROPERTY INTERFACE_POSITION_INDEPENDENT_CODE "ON")
	endif (CMAKE_CUDA_COMPILER AND TARGET Qt5::Core)
	include_directories("${PCL_INCLUDE_DIRS}")
	target_link_libraries(${PROJECT_BINARY} ${PCL_LIBRARIES})
else (PCL_FOUND)
	message(STATUS "|  PCL not found!")
endif (PCL_FOUND)
message(STATUS "+============================")

#### ROS2 ####
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(message_filters REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_eigen REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(image_transport REQUIRED)

# Preprocessor define for ros2 distribution eloquent elusor
if ($ENV{ROS_DISTRO} STREQUAL "eloquent")
	target_compile_definitions(${PROJECT_BINARY} PRIVATE ROS_ELOQUENT)
endif ()
message(STATUS "+==========[ ROS2 ]===========")
message(STATUS "|  Distribution:\t" $ENV{ROS_DISTRO})
message(STATUS "+===================================")

###########
## Build ##
###########
target_include_directories(
	${PROJECT_BINARY} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

ament_target_dependencies(
	${PROJECT_BINARY}
	"rclcpp"
	"sensor_msgs"
	"message_filters"
	"tf2_ros"
	"tf2_eigen"
	"ament_index_cpp"
	"tf2_geometry_msgs"
	"pcl_conversions"
	"image_transport"
)

ament_export_dependencies(PCL)

#############
## Install ##
#############
# Install library
if ($ENV{ROS_DISTRO} STREQUAL "eloquent")
	ament_export_interfaces(${PROJECT_BINARY} HAS_LIBRARY_TARGET) # Eloquent
else()
	ament_export_targets(${PROJECT_BINARY} HAS_LIBRARY_TARGET) # Foxy
endif ()
install(
	DIRECTORY include/${PROJECT_NAME}
	DESTINATION include
)
install(
	TARGETS ${PROJECT_BINARY}
	EXPORT ${PROJECT_BINARY}
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)

###################
## Documentation ##
###################
find_package(Doxygen)
if(DOXYGEN_FOUND)
	message(STATUS "Doxygen found")
	# Prepare doxygen configuration file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
	# Add doc as target
	add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
endif(DOXYGEN_FOUND)

ament_package()
