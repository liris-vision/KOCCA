# KOCCA - Kinect & Optitrack Calibration & Capture Application

KOCCA allows to simultaneously acquire data from both an Optitrack system and a Microsoft Kinect V2, in order to be able to project the Optitrack makers coordinates to Kinect images.

For more details about it's features and how to use them, check KOCCA's « User Manual ».

This guide will help you through setting up a developpement environement, allowing you to build KOCCA from source.

## Development environnement setup

### Required Components :
You need a x86-compatible PC, running Microsoft Windows 10.

Make sure the following software are installed on it :
- Microsoft Visual Studio 2017 (https://visualstudio.microsoft.com/fr/downloads/), with packages :
	- Visual C++ 2017
	- English language pack
- Microsoft Windows 8.1 SDK (https://developer.microsoft.com/fr-fr/windows/downloads/sdk-archive)
- Microsoft Kinect SDK v2  (https://www.microsoft.com/en-us/download/details.aspx?id=44561)
- CMake  ver. 3.8+ (https://cmake.org/download/)
- Git  (https://git-scm.com/download/win)
- SVN  (https://sourceforge.net/projects/win32svn/)

Optional components :
- Doxygen : if you want to generate the API documentation (see ) ()
- InnoSetup : if you want to generate an installer for the built application (see ) ()


#### VCPKG :
Use Git to clone VCPKG from url : https://github.com/Microsoft/vcpkg

	> git clone https://github.com/Microsoft/vcpkg

From a command line, go to the root folder of VCPKG, then run :

	> bootstrap-vcpkg.bat

then :

	> vcpkg integrate install

VCPKG is now ready to downlaod and build the libraries we need. Run :

	> vcpkg install opencv gtkmm boost-filesystem tinyxml2 libusb

(Warning : this might take a while !)

As of february 1st, 2019, the project was successfully built with the following versions of vcpkg and vcpkg-installed libraries :

	> vcpkg version
	Vcpkg package management program version 2018.11.23-nohash

	> vcpkg list
	atk:x86-windows							2.24.0-2
	atkmm:x86-windows						2.24.2
	boost-array:x86-windows					1.68.0
	boost-assert:x86-windows				1.68.0
	boost-bind:x86-windows					1.68.0
	boost-build:x86-windows					1.68.0 
	boost-compatibility:x86-windows			1.68.0
	boost-concept-check:x86-windows			1.68.0
	boost-config:x86-windows				1.68.0
	boost-container-hash:x86-windows		1.68.0
	boost-conversion:x86-windows			1.68.0
	boost-core:x86-windows					1.68.0
	boost-detail:x86-windows				1.68.0
	boost-filesystem:x86-windows			1.68.0
	boost-function-types:x86-windows		1.68.0
	boost-functional:x86-windows			1.68.0
	boost-fusion:x86-windows				1.68.0
	boost-integer:x86-windows				1.68.0
	boost-io:x86-windows					1.68.0
	boost-iterator:x86-windows				1.68.0
	boost-modular-build-helper:x86-windows	2018-10-19
	boost-move:x86-windows					1.68.0
	boost-mpl:x86-windows					1.68.0
	boost-optional:x86-windows				1.68.0
	boost-predef:x86-windows				1.68.0
	boost-preprocessor:x86-windows			1.68.0
	boost-range:x86-windows					1.68.0
	boost-regex:x86-windows					1.68.0
	boost-smart-ptr:x86-windows				1.68.0
	boost-static-assert:x86-windows			1.68.0
	boost-system:x86-windows				1.68.0
	boost-throw-exception:x86-windows		1.68.0
	boost-tuple:x86-windows					1.68.0
	boost-type-traits:x86-windows			1.68.0
	boost-typeof:x86-windows				1.68.0
	boost-utility:x86-windows				1.68.0
	boost-vcpkg-helpers:x86-windows			4
	boost-winapi:x86-windows				1.68.0
	bzip2:x86-windows						1.0.6-3
	cairo:x86-windows						1.15.8-4
	cairomm:x86-windows						1.15.3-2
	dirent:x86-windows						1.23.1
	eigen3:x86-windows						3.3.7
	expat:x86-windows						2.2.6 
	fontconfig:x86-windows					2.12.4-7
	freetype:x86-windows					2.8.1-3
	gdk-pixbuf:x86-windows					2.36.9-1
	gettext:x86-windows						0.19-5
	glib:x86-windows						2.52.3-11
	glibmm:x86-windows						2.52.1-7
	gtk:x86-windows							3.22.19-2 
	gtkmm:x86-windows						3.22.2 
	harfbuzz:x86-windows					1.8.4-3
	harfbuzz[glib]:x86-windows
	harfbuzz[ucdn]:x86-windows
	libepoxy:x86-windows					1.4.3-1
	libffi:x86-windows						3.1-1 
	libiconv:x86-windows					1.15-5
	libjpeg-turbo:x86-windows				1.5.3-1
	liblzma:x86-windows						5.2.4
	libpng:x86-windows						1.6.36
	libsigcpp:x86-windows					2.10-1
	libusb:x86-windows						1.0.22-1
	opencv:x86-windows						3.4.3-5
	opencv[eigen]:x86-windows
	opencv[flann]:x86-windows
	opencv[jpeg]:x86-windows
	opencv[opengl]:x86-windows
	opencv[png]:x86-windows
	opencv[tiff]:x86-windows
	opengl:x86-windows						0.0-5
	pango:x86-windows						1.40.11-3
	pangomm:x86-windows						2.40.1
	pcre:x86-windows						8.41-1
	pixman:x86-windows						0.34.0-5
	ragel:x86-windows						6.10-1
	tiff:x86-windows						4.0.10-1
	tinyxml2:x86-windows					7.0.1
	zlib:x86-windows						1.2.11-3


#### NatNet SDK :
Download NatNet SDK from https://optitrack.com/downloads/developer-tools.html#natnet-sdk and unzip it anywhere you like on your file system.


#### KOCCA source :
Use Git to clone KOCCA's sources from https://github.com/liris-vision/KOCCA

	> git clone https://github.com/liris-vision/KOCCA.git

## Building KOCCA from source

### CMake :

Use CMake to generate the Visual Studio solution.
You will need to set the following options :
- VCPKG_DIR : the path to the root folder of vcpkg.
- NATNETSDK_DIR : the path to the folder of your unzipped NatNet SDK.

The following options should be set automatically if you have let Kinect SDK and Windows 8.1 SDK get installed to their default folders. If you haven't, you my have to set them manually :

- KINECTVSDK_DIR : the path to the folder of your Kinect SDK v2 (default : C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409)
- PSAPI_LIB : the path to your « Psapi.lib » file (default : C:/Program Files (x86)/Windows Kits/8.1/Lib/winv6.3/um/x86/Psapi.lib)

Once you set the correct paths in these option, you should be able to generate the Visual Studio  solution.


### Build from Visual studio :

Open <your_cmake_build_path>/KOCCA.sln with Visual Studio 2017
Choose « Debug » or « Release » build mode.
Right-click on the « KOCCA » project, then « Generate »
