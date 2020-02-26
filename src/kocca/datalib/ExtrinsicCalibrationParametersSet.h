#ifndef KOCCA_DATALIB_EXTRINSIC_CALIBRATION_PARAMETERS_SET_H
#define KOCCA_DATALIB_EXTRINSIC_CALIBRATION_PARAMETERS_SET_H

#include <opencv2/opencv.hpp>

#include "CalibrationParametersSet.h"

namespace kocca {
	namespace datalib {

		/**
		 * Stores extrinsic calibration data of a camera
		 */
		class ExtrinsicCalibrationParametersSet: public CalibrationParametersSet {
		public:

			/**
			 *  extrinsic translation - 1x3 float vector
			 */
			VEC3 T;

			/**
			 * extrinsic rotation - 3x3 float matrix
			 */
			MAT33 R;

			/**
			 * Constructor
			 * @param  T_  extrinsic translation - 1x3 float vector
			 * @param R_ extrinsic rotation - 3x3 float matrix
			 * @param error_ Error in parameters computing
			 */
			ExtrinsicCalibrationParametersSet(const VEC3 T_, const MAT33 R_,
					const float error_);

			/**
			 * Copy constructor
			 * @param other The ExtrinsicCalibrationParametersSet to copy from
			 */
			ExtrinsicCalibrationParametersSet(const ExtrinsicCalibrationParametersSet& other);

			/**
			 * Destructor
			 */
			~ExtrinsicCalibrationParametersSet();

			/**
			 * Renders all calibration data as a human-readable string
			 */
			std::string toString();

			/**
			 * Get the rotation vector as a cv::Mat
			 * @return cv::Mat the rotation vector
			 */
			cv::Mat getRotationVector();

			/**
			 * Get the translation vector as a cv::Mat
			 * @return cv::Mat the translation vector
			 */
			cv::Mat getTranslationVector();

		protected:

			/**
			 * The rotation vector
			 */
			cv::Mat* rotationVector;

			/**
			 * The translation vector
			 */
			cv::Mat* translationVector;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_EXTRINSIC_CALIBRATION_PARAMETERS_SET_H
