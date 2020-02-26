#ifndef TIME_CODED_FRAME_H
#define TIME_CODED_FRAME_H

#include <opencv2/opencv.hpp>

namespace kocca {
	namespace datalib {

		/**
		 * This structure associates a cv::Mat frame of a video stream with it's appearance time (relative to the video recording)
		 */
		struct TimeCodedFrame {

			/**
			 * The time of the frame
			 */
			unsigned long long time;

			/**
			 * The content of the frame
			 */
			cv::Mat frame;
		};
	} // namespace datalib
} // namespace kocca

#endif //TIME_CODED_FRAME_H
