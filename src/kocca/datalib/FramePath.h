#ifndef KOCCA_DATALIB_FRAME_PATH_H
#define KOCCA_DATALIB_FRAME_PATH_H

#include <string>
#include "boost/filesystem.hpp"

namespace kocca {
	namespace datalib {

		/**
		 * Utility class allowing to extract the time from a sequence image file path
		 */
		class FramePath {
		public:

			/**
			 * The file path
			 */
			std::string path;

			/**
			 * The frame time
			 */
			long long time;

			/**
			 * Constructor
			 */
			FramePath();

			/**
			 * Constructor
			 * @param boostPath the file path as a boost::filesystem::path
			 */
			FramePath(boost::filesystem::path boostPath);

			/**
			 * Compares two frame paths by their respective frame time. Usefull for chronologic sorting of files lists.
			 * @param framePathA the first frame path to compare
			 * @param framePathB the first frame path to compare
			 * @return true if the time of framePathA is strictly inferior to the time of framePathB, false otherwise
			 */
			static bool compareFramePaths(FramePath framePathA, FramePath framePathB);
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_FRAME_PATH_H
