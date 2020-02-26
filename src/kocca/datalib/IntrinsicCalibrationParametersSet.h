#ifndef KOCCA_DATALIB_INTRINSIC_CALIBRATION_PARAMETERS_SET_H
#define KOCCA_DATALIB_INTRINSIC_CALIBRATION_PARAMETERS_SET_H

#include <opencv2/opencv.hpp>
#include "CalibrationParametersSet.h"

namespace kocca {
	namespace datalib {

		/**
		 * Stores data for Intrinsic calibration of a camera
		 */
		class IntrinsicCalibrationParametersSet: public CalibrationParametersSet {
		public:

			/**
			 * Distortion parameter - 1x4 float vector
			 */
			VEC4 K;

			/**
			 * Projection parameter - 3x3 float matrix
			 */
			MAT33 A;

			/**
			 * Constructor
			 * @param K_ distortion - 1x4 float vector
			 * @param A_ projection - 3x3 float matrix
			 * @param error_ Error in parameters computing
			 */
			IntrinsicCalibrationParametersSet(const VEC4 K_, const MAT33 A_,
					const float error_);

			/**
			 * Copy constructor
			 * @param other the IntrinsicCalibrationParametersSet to copy from
			 */
			IntrinsicCalibrationParametersSet(const IntrinsicCalibrationParametersSet& other);

			/**
			 * Destructor
			 */
			~IntrinsicCalibrationParametersSet();

			/**
			 * Renders all intrinsic calibration data as a human-readable string
			 */
			std::string toString();

			/**
			 * Returns the distortion vector
			 */
			cv::Mat getDistortionVector();

			/**
			 * Returns the projection matrix
			 */
			cv::Mat getProjectionMatrix();

		protected:

			/**
			 * The distortion vector
			 */
			cv::Mat* distortionVector;

			/**
			 * The projection matrix
			 */
			cv::Mat* projectionMatrix;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_INTRINSIC_CALIBRATION_PARAMETERS_SET_H
