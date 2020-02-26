#include "ExtrinsicCalibrationParametersSet.h"

#include <sstream>

namespace kocca {
	namespace datalib {
		ExtrinsicCalibrationParametersSet::ExtrinsicCalibrationParametersSet(
				const VEC3 T_, const MAT33 R_, const float error_) {
			// copy T_ parameter array to protected property T
			for (int i = 0; i < 3; i++)
				T[i] = T_[i];

			// copy R_ parameter array to protected property R
			for (int i = 0; i < 9; i++)
				R[i] = R_[i];

			error = error_;

			rotationVector = NULL;
			translationVector = NULL;
		}

		ExtrinsicCalibrationParametersSet::ExtrinsicCalibrationParametersSet(const ExtrinsicCalibrationParametersSet& other) {
			// copy T_ parameter array to protected property T
			for (int i = 0; i < 3; i++)
				T[i] = other.T[i];

			// copy R_ parameter array to protected property R
			for (int i = 0; i < 9; i++)
				R[i] = other.R[i];

			error = other.error;

			rotationVector = NULL;
			translationVector = NULL;
		}

		ExtrinsicCalibrationParametersSet::~ExtrinsicCalibrationParametersSet()
		{
			if(rotationVector != NULL)
				delete rotationVector;

			if(translationVector != NULL)
				delete translationVector;
		}

		std::string ExtrinsicCalibrationParametersSet::toString() {
			std::ostringstream stringStream;
			stringStream << "Translation:" << std::endl << T[0] << ", " << T[1] << ", "
					<< T[2] << std::endl << std::endl << "Rotation:" << std::endl
					<< R[0] << ", " << R[1] << ", " << R[2] << std::endl << R[3] << ", "
					<< R[4] << ", " << R[5] << std::endl << R[6] << ", " << R[7] << ", "
					<< R[8] << std::endl << std::endl << "Error:" << std::endl << error;

			return (stringStream.str());
		}

		cv::Mat ExtrinsicCalibrationParametersSet::getRotationVector()
		{
			if(rotationVector == NULL)
			{
				cv::Mat rotationMatrix(3,3,cv::DataType<double>::type);

				for(int i = 0; i < 3; i++)
					for(int j = 0; j < 3; j++)
						rotationMatrix.at<double>(i,j) = R[3*i+j];

				rotationVector = new cv::Mat(3,1,cv::DataType<double>::type);
				cv::Rodrigues(rotationMatrix, *rotationVector);
			}

			return(*rotationVector);
		}

		cv::Mat ExtrinsicCalibrationParametersSet::getTranslationVector()
		{
			if(translationVector == NULL)
			{
				translationVector = new cv::Mat(3,1,cv::DataType<double>::type);

				for(int i = 0; i < 3; i++)
					translationVector->at<double>(i) = T[i];
			}

			return(*translationVector);
		}
	} // namespace datalib
} // namespace kocca
