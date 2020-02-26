cmake_minimum_required(VERSION 3.8)
project("KOCCA")
set(ALL_BUILD_IN_MAIN_PROJECT "TRUE")

# source files

SET(EXEC_SOURCES
	../src/main.cpp
	../src/kocca/Application.cpp
	../src/kocca/KinectV2Sensor.cpp
	../src/kocca/utils.cpp
	../src/kocca/datalib/TaskProgress.cpp
	../src/kocca/datalib/ExtrinsicCalibrationParametersSet.cpp
	../src/kocca/datalib/IntrinsicCalibrationParametersSet.cpp
	../src/kocca/datalib/Sequence.cpp
	../src/kocca/datalib/MocapMarker.cpp
	../src/kocca/datalib/MocapMarkerFrame.cpp
	../src/kocca/datalib/MocapMarkersSequence.cpp
	../src/kocca/datalib/FramePath.cpp
	../src/kocca/datalib/KinectCalibrationFile.cpp
	../src/kocca/datalib/SequenceFile.cpp
	../src/kocca/operations/Operation.cpp
	../src/kocca/operations/SequenceReading.cpp
	../src/kocca/operations/Monitoring.cpp
	../src/kocca/operations/SequenceRecording.cpp
	../src/kocca/operations/Calibration.cpp
	../src/kocca/operations/TimeCodedFrameBuffer.cpp
	../src/kocca/widgets/MainWindow.cpp
	../src/kocca/widgets/CalibrationResultCell.cpp
	../src/kocca/widgets/CalibrationTypesChoiceDialog.cpp
	../src/kocca/widgets/CvDrawingArea.cpp
	../src/kocca/widgets/SelectableCvDrawingArea.cpp
	../src/kocca/widgets/KoccaMonitoringWidget.cpp
	../src/kocca/widgets/WaitMessagePopup.cpp
	../src/kocca/widgets/MocapMarkersTreeView.cpp
	../src/external/miniz/miniz.c
	../src/external/whereami/whereami.c
   )

add_executable("KOCCA" ${EXEC_SOURCES} exe_icon.rc)

# find and includes miniz and whereami libraries sources

SET(EXTERNAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/external)
include_directories(${EXTERNAL_INCLUDE_DIR}/miniz)
include_directories(${EXTERNAL_INCLUDE_DIR}/whereami)

# find and includes required libraries installed with VCPKG

SET(VCPKG_DIR "VCPKG_DIR-NOTFOUND" CACHE PATH "The root directory of VCPKG")

if(IS_DIRECTORY ${VCPKG_DIR})
	SET(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Path to the cmake file provided with vcpkg")

	# link opencv libs
	list(APPEND LIBS ${OpenCV_LIBS})

	# link gtk libs
	SET(VCPKG_LIBS_DIR "${VCPKG_DIR}/installed/x86-windows/lib" CACHE PATH "Path to the folder containing libs installed with vcpkg")
	SET(VCPKG_DEBUG_LIBS_DIR "${VCPKG_DIR}/installed/x86-windows/debug/lib" CACHE PATH "Path to the folder containing debug versions of libs installed with vcpkg")
	
	SET(GTK_LIBS
		debug ${VCPKG_DEBUG_LIBS_DIR}/atk-1.0.lib optimized ${VCPKG_LIBS_DIR}/atk-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/atkmm.lib optimized ${VCPKG_LIBS_DIR}/atkmm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/cairo-gobjectd.lib optimized ${VCPKG_LIBS_DIR}/cairo-gobject.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/cairod.lib optimized ${VCPKG_LIBS_DIR}/cairo.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/cairomm-1.0.lib optimized ${VCPKG_LIBS_DIR}/cairomm-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/epoxy.lib optimized ${VCPKG_LIBS_DIR}/epoxy.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/expat.lib optimized ${VCPKG_LIBS_DIR}/expat.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/fontconfig.lib optimized ${VCPKG_LIBS_DIR}/fontconfig.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/freetyped.lib optimized ${VCPKG_LIBS_DIR}/freetype.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gdk-3.0.lib optimized ${VCPKG_LIBS_DIR}/gdk-3.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gdkmm.lib optimized ${VCPKG_LIBS_DIR}/gdkmm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gdk_pixbuf-2.0.lib optimized ${VCPKG_LIBS_DIR}/gdk_pixbuf-2.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gio-2.0.lib optimized ${VCPKG_LIBS_DIR}/gio-2.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/giomm.lib optimized ${VCPKG_LIBS_DIR}/giomm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/glib-2.0.lib optimized ${VCPKG_LIBS_DIR}/glib-2.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/glibmm.lib optimized ${VCPKG_LIBS_DIR}/glibmm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gmodule-2.0.lib optimized ${VCPKG_LIBS_DIR}/gmodule-2.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gobject-2.0.lib optimized ${VCPKG_LIBS_DIR}/gobject-2.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gtk-3.0.lib optimized ${VCPKG_LIBS_DIR}/gtk-3.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/gtkmm.lib optimized ${VCPKG_LIBS_DIR}/gtkmm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/libcharset.lib optimized ${VCPKG_LIBS_DIR}/libcharset.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/libffi.lib optimized ${VCPKG_LIBS_DIR}/libffi.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/libiconv.lib optimized ${VCPKG_LIBS_DIR}/libiconv.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/libintl.lib optimized ${VCPKG_LIBS_DIR}/libintl.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/libpng16d.lib optimized ${VCPKG_LIBS_DIR}/libpng16.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pango-1.0.lib optimized ${VCPKG_LIBS_DIR}/pango-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pangocairo-1.0.lib optimized ${VCPKG_LIBS_DIR}/pangocairo-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pangoft2-1.0.lib optimized ${VCPKG_LIBS_DIR}/pangoft2-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pangomm.lib optimized ${VCPKG_LIBS_DIR}/pangomm.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pangowin32-1.0.lib optimized ${VCPKG_LIBS_DIR}/pangowin32-1.0.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pcred.lib optimized ${VCPKG_LIBS_DIR}/pcre.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/pixman-1d.lib optimized ${VCPKG_LIBS_DIR}/pixman-1.lib
		debug ${VCPKG_DEBUG_LIBS_DIR}/sigc-2.0.lib optimized ${VCPKG_LIBS_DIR}/sigc-2.0.lib)

	list(APPEND LIBS ${GTK_LIBS})
endif()

# find and include Kinect V2 SDK

SET(KINECTVSDK_DIR "C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409" CACHE PATH "Path to the MS Kinect SDK v2")

if(IS_DIRECTORY ${KINECTVSDK_DIR})
	SET(KINECTVSDK_INCLUDE_DIR "${KINECTVSDK_DIR}/inc" CACHE PATH "Path to the MS Kinect SDK v2 include directory")
	include_directories(${KINECTVSDK_INCLUDE_DIR})
	SET(KINECTVSDK_LIB_DIR "${KINECTVSDK_DIR}/Lib/x86" CACHE PATH "Path to the MS Kinect SDK v2 Lib/x86 directory")
	find_library(KINECTV2_LIBRARY NAMES Kinect20 PATHS ${KINECTVSDK_LIB_DIR})
	list(APPEND LIBS ${KINECTV2_LIBRARY})
endif()

#find and include NatNet SDK library

SET(NATNETSDK_DIR "NATNETSDK_DIR-NOTFOUND" CACHE PATH "The root directory of the NatNet SDK")

if(IS_DIRECTORY ${NATNETSDK_DIR})
	SET(NATNETSDK_INCLUDE_DIR "${NATNETSDK_DIR}/include" CACHE PATH "Path to the include directory of the NatNet SDK")
	include_directories(${NATNETSDK_INCLUDE_DIR})
	SET(NATNETSDK_LIB "${NATNETSDK_DIR}/lib/NatNetLib.lib" CACHE PATH "Path to the lib directory of the NatNet SDK")
	list(APPEND LIBS ${NATNETSDK_LIB})

	# after build, copy NatNetLib.dll to the target build directory
	add_custom_command(TARGET KOCCA POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${NATNETSDK_DIR}/lib/NatNetLib.dll" $<TARGET_FILE_DIR:KOCCA>)
endif()

# needed for timer function "timeGetTime"
list(APPEND LIBS "winmm")

# Psapi is used to get processes IDs
find_library(PSAPI_LIB NAMES "Psapi" PATHS "C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86")
list(APPEND LIBS ${PSAPI_LIB})

# set KOCCA's base folder as the working directory for debugger execution, as KOCCA needs it's "kocca.ui" file and "img" and "share" sub-folders 
set_property(TARGET KOCCA PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# set KOCCA as the startup project in visual studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT KOCCA)

target_link_libraries("KOCCA" ${LIBS})
