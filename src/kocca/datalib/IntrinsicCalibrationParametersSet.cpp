#include "IntrinsicCalibrationParametersSet.h"

#include <sstream>

namespace kocca {
	namespace datalib {
		IntrinsicCalibrationParametersSet::IntrinsicCalibrationParametersSet(
				const VEC4 K_, const MAT33 A_, const float error_) {
			// copy K_ parameter array to protected property K
			for (int i = 0; i < 4; i++)
				K[i] = K_[i];

			// copy A_ parameter array to protected property A
			for (int i = 0; i < 9; i++)
				A[i] = A_[i];

			error = error_;
			distortionVector = NULL;
			projectionMatrix = NULL;
		}

		IntrinsicCalibrationParametersSet::IntrinsicCalibrationParametersSet(const IntrinsicCalibrationParametersSet& other) {
			// copy K_ parameter array to protected property K
			for (int i = 0; i < 4; i++)
				K[i] = other.K[i];

			// copy A_ parameter array to protected property A
			for (int i = 0; i < 9; i++)
				A[i] = other.A[i];

			error = other.error;
			distortionVector = NULL;
			projectionMatrix = NULL;
		}

		IntrinsicCalibrationParametersSet::~IntrinsicCalibrationParametersSet()
		{
			if(distortionVector != NULL)
				delete distortionVector;

			if(projectionMatrix != NULL)
				delete projectionMatrix;
		}

		std::string IntrinsicCalibrationParametersSet::toString() {
			std::ostringstream stringStream;
			stringStream << "Distortion:" << std::endl << K[0] << ", " << K[1] << ", "
					<< K[2] << ", " << K[3] << std::endl << std::endl << "Projection:"
					<< std::endl << A[0] << ", " << A[1] << ", " << A[2] << std::endl
					<< A[3] << ", " << A[4] << ", " << A[5] << std::endl << A[6] << ", "
					<< A[7] << ", " << A[8] << std::endl << std::endl << "Error:"
					<< std::endl << error;

			return (stringStream.str());
		}

		cv::Mat IntrinsicCalibrationParametersSet::getDistortionVector()
		{
			if(distortionVector == NULL)
			{
				distortionVector = new cv::Mat(4,1,cv::DataType<double>::type);

				for(int i = 0; i < 4; i++)
					distortionVector->at<double>(i) = K[i];
			}

			return(*distortionVector);
		}

		cv::Mat IntrinsicCalibrationParametersSet::getProjectionMatrix()
		{
			if(projectionMatrix == NULL)
			{
				projectionMatrix = new cv::Mat(3,3,cv::DataType<double>::type);

				for(int i = 0; i < 3; i++)
					for(int j = 0; j < 3; j++)
						projectionMatrix->at<double>(i,j) = A[3*i+j];
			}

			return(*projectionMatrix);
		}
	} // namespace datalib
} // namespace kocca