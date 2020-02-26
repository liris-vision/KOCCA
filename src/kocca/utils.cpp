#include "utils.h"
#include <cctype>
#include "whereami.h"
#include <WinBase.h>
#include <mmsystem.h>
#include <time.h>
#include <iomanip>
#include "Exceptions.h"

#include <iostream>

#ifdef _WIN32
	struct MSTime { long long tv_sec; long long tv_nsec; };

	int clock_gettime(int, struct MSTime* spec) {
		__int64 wintime;
		GetSystemTimeAsFileTime((FILETIME*)&wintime);
		spec->tv_sec = wintime / 10000000i64;           //seconds
		spec->tv_nsec = wintime % 10000000i64 *100;      //nano-seconds
		return 0;
	}
#endif

namespace kocca {
	unsigned long long getMSTime() {
		struct MSTime top;
		clock_gettime(0, &top);
		return ((top.tv_sec * 1000) + (top.tv_nsec / 1000000));
	}

	boost::filesystem::path get_executable_path() {
		char* path = NULL;
		int length, dirname_length;

		// wai_getExecutablePath function comes from "whereami.h"
		length = wai_getExecutablePath(NULL, 0, NULL);
		path = (char*) malloc(length + 1);
		wai_getExecutablePath(path, length, &dirname_length);
		path[length] = '\0';
		boost::filesystem::path boostPath(path);
		free(path);
		return boostPath;
	}

	boost::filesystem::path get_application_base_path() {
		boost::filesystem::path current_path(boost::filesystem::current_path());
		
		if(boost::filesystem::is_directory(current_path / "img") && boost::filesystem::is_directory(current_path / "share") && boost::filesystem::is_regular_file(current_path / "kocca.ui"))
			return current_path;
		else {
			boost::filesystem::path executable_path(get_executable_path());
			return (executable_path.parent_path());
		}
	}

	boost::filesystem::path get_img_folder_path() {
		// we use "boost::filesystem" library for cross-platform path manipulations
		boost::filesystem::path application_base_path(get_application_base_path());
		return (application_base_path / "img");
	}

	std::string get_img_path(const char* fileName) {
		// we use "boost::filesystem" library for cross-platform path manipulations
		boost::filesystem::path resource_folder_path = get_img_folder_path();
		boost::filesystem::path resource_path = resource_folder_path / fileName;
		return (resource_path.string());
	}

	boost::filesystem::path get_temp_folder_path() {
		// @todo : créer fichier de conf, stocker chemin temp dedans, utiliser si défini, sinon : "%USERPROFILE%/AppData/Local/KOCCA" par défaut : 
		boost::filesystem::path userprofilePath(getenv("USERPROFILE"));
		boost::filesystem::path userprofileKoccaTempPath = userprofilePath / "AppData" / "Local" / "KOCCA";

		if ((!boost::filesystem::exists(userprofileKoccaTempPath) || !boost::filesystem::is_directory(userprofileKoccaTempPath)) && !boost::filesystem::create_directory(userprofileKoccaTempPath))
			throw (TempFolderNotAvailableException("KOCCA's temp folder not found and could not be created"));

		return userprofileKoccaTempPath;
	}

	std::string get_temp_file_path(const char* fileName) {
		// we use "boost::filesystem" library for cross-platform path manipulations
		boost::filesystem::path temp_folder_path = get_temp_folder_path();
		boost::filesystem::path temp_file_path = temp_folder_path / fileName;
		return (temp_file_path.string());
	}

	bool strisprint(std::string str) {
		bool isprint = true;

		for(unsigned int i = 0; i < str.size() && isprint; i++)
			isprint = (std::isprint(str.at(i)) != 0);

		return isprint;
	}

	std::string format_timestamp(long long int timestamp) {
		int m = std::floor(timestamp / 60000.00);
		int s = std::floor((timestamp - (60000 * m)) / 1000.00);
		int ms = timestamp - ((60000 * m) + (1000 * s));

		std::ostringstream outputStream;
		outputStream << std::setfill('0') << std::setw(3) << m << ":" << std::setw(2) << s << ":" << std::setw(3) << ms;

		return outputStream.str();
	}
} // namespace kocca
