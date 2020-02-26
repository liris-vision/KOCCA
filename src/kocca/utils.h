#ifndef KOCCA_UTILS_H
#define KOCCA_UTILS_H

#include <string>
#include "boost/filesystem.hpp"
#include <gtkmm/widget.h>

#ifdef _WIN32
	#include <Windows.h>
	struct MStime;
	int clock_gettime(int, struct MSTime* spec);
	#define usleep(n_microseconds)   Sleep(n_microseconds)
	#define NOMINMAX
#else
	#include <time.h>
	#include <unistd.h>
#endif // _WIN32

namespace kocca {

	/**
	 * Gets a local system timestamp in MilliSeconds
	 */
	unsigned long long getMSTime();

	/**
	 * Gets the full path on the file system of the main executable (cross-platform)
	 */
	boost::filesystem::path get_executable_path();

	/**
	 * Gets the path on the file system of the base folder that contains the main executable (cross-platform)
	 */
	boost::filesystem::path get_application_base_path();

	/**
	 * Gets the path on the file system of the "img" folder that contains images (cross-platform)
	 */
	boost::filesystem::path get_img_folder_path();

	/**
	 * Gets the full path on the file system of an image file contained in the "img" folder
	 * @param fileName the image file name, as a C string
	 */
	std::string get_img_path(const char* fileName);

	/**
	 * Gets the path on the file system of the "temp" folder that contains temporary files during recording (cross-platform)
	 */
	boost::filesystem::path get_temp_folder_path();

	/**
	 * Gets the full path on the file system of a temporary file contained in the "temp" folder
	 * @param fileName the file name, as a C string
	 * @todo check if this is still used, and how
	 */
	std::string get_temp_file_path(const char* fileName);

	/**
	 * Checks if a string contains only printable characters
	 * @param str the string to check
	 */
	bool strisprint(std::string str);

	/**
	 * Formats a timestamp in a string of format "mmm:ss:xxx" (where mmm is for minutes, ss is for seconds, and xxx is for milliseconds)
	 */
	std::string format_timestamp(long long int timestamp);

} // namespace kocca

#endif // KOCCA_UTILS_H
