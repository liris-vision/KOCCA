#include "Calibration.h"
#include "../utils.h"
#include "../Exceptions.h"

#include <numeric>

namespace kocca {
	namespace operations {
		Calibration::Calibration(bool _calibrateInIR, bool _calibrateInRGB, bool _calibrateEx, int _countdownDuration, int _frameCount, int _chessBoardWidth, int _chessBoardHeight, float _squareSize, float _freezeDuration) : Operation() {
			type = KOCCA_CALIBRATION_OPERATION;
			calibrateInIR = _calibrateInIR;
			calibrateInRGB = _calibrateInRGB;
			calibrateEx = _calibrateEx;
			countdownDuration = _countdownDuration;
			frameCount = _frameCount;
			/*chessBoardWidth = _chessBoardWidth;
			chessBoardHeight = _chessBoardHeight;*/
			IRImageSize = NULL;
			RGBImageSize = NULL;
			chessBoardSize = cv::Size(_chessBoardWidth - 1, _chessBoardHeight - 1);
			squareSize = _squareSize;
			currentCalibrationStep = 0;
			calibrationPausedTime = 0;
			onBeforeExtrinsicCalibration = NULL;
			onAllCalibrationsDone = NULL;
			intrinsicIRCalibRes = NULL;
			intrinsicRGBCalibRes = NULL;
			extrinsicIRCalibRes = NULL;
			extrinsicRGBCalibRes = NULL;
			//intrinsicCalibrator = NULL;
			//extrinsicCalibrator = NULL;
			freezeDuration = _freezeDuration;

			// display is not frozen by default
			unFreezeTime = 0;
		}

		Calibration::~Calibration() {
			frameProcessing_mutex.lock();

			if(intrinsicIRCalibRes != NULL)
				delete intrinsicIRCalibRes;

			if(intrinsicRGBCalibRes != NULL)
				delete intrinsicRGBCalibRes;

			if(extrinsicIRCalibRes != NULL)
				delete extrinsicIRCalibRes;

			if(extrinsicRGBCalibRes != NULL)
				delete extrinsicRGBCalibRes;

			if (IRImageSize != NULL)
				delete IRImageSize;

			if (RGBImageSize != NULL)
				delete RGBImageSize;

			/*if(intrinsicCalibrator != NULL)
				delete intrinsicCalibrator;

			if(extrinsicCalibrator != NULL)
				delete extrinsicCalibrator;*/

			frameProcessing_mutex.unlock();
		}

		bool Calibration::getCalibrateIntrinsicIR() {
			return calibrateInIR;
		}

		bool Calibration::getCalibrateIntrinsicRGB() {
			return calibrateInRGB;
		}

		bool Calibration::getCalibrateExtrinsic() {
			return calibrateEx;
		}

		void Calibration::setIntrinsicIRCalibRes(kocca::datalib::IntrinsicCalibrationParametersSet _intrinsicIRCalibRes) {
			if(intrinsicIRCalibRes != NULL)
				delete intrinsicIRCalibRes;

			intrinsicIRCalibRes = new kocca::datalib::IntrinsicCalibrationParametersSet(_intrinsicIRCalibRes);
		}

		void Calibration::setIntrinsicRGBCalibRes(kocca::datalib::IntrinsicCalibrationParametersSet _intrinsicRGBCalibRes) {
			if(intrinsicRGBCalibRes != NULL)
				delete intrinsicRGBCalibRes;

			intrinsicRGBCalibRes = new kocca::datalib::IntrinsicCalibrationParametersSet(_intrinsicRGBCalibRes);
		}

		bool Calibration::isPaused() {
			return (calibrationPausedTime != 0);
		}

		void Calibration::pause() {
			calibrationPausedTime = getMSTime();
		}

		void Calibration::resume() {
			lastPicTime = getMSTime() - (calibrationPausedTime - lastPicTime);
			calibrationPausedTime = 0;
		}

		void Calibration::setCountDownDuration(int newCountdownDuration) {
			countdownDuration = newCountdownDuration;
			lastPicTime = getMSTime();

			if(calibrationPausedTime != 0)
				calibrationPausedTime = lastPicTime;
		}

		/**
		 * 
		 */
		bool Calibration::processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			if(onDepthFrameOutput != NULL)
				onDepthFrameOutput(tcFrame);

			return false;
		}

		/**
		 * 
		 */
		bool Calibration::processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			bool processed = false;
			unsigned long long currentTime = getMSTime();

			if (currentTime >= unFreezeTime) {
				if (currentCalibrationStep == 2) {
					if (frameProcessing_mutex.try_lock()) {
						processFrameIntrinsic(tcFrame.frame);
						frameProcessing_mutex.unlock();
						processed = true;

						if (onColorImageFrameOutput != NULL)
							onColorImageFrameOutput(tcFrame);
					}
				}
				else if (currentCalibrationStep == 5) {
					if (frameProcessing_mutex.try_lock()) {
						processFrameExtrinsic(tcFrame.frame);
						frameProcessing_mutex.unlock();
						processed = true;

						if (onColorImageFrameOutput != NULL)
							onColorImageFrameOutput(tcFrame);
					}
				}
				else 
					if (onColorImageFrameOutput != NULL)
						onColorImageFrameOutput(tcFrame);
			}
			
			return processed;
		}

		/**
		*
		*/
		bool Calibration::processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			bool processed = false;
			unsigned long long currentTime = getMSTime();

			if (currentTime >= unFreezeTime) {
				if (currentCalibrationStep == 1) {
					if (frameProcessing_mutex.try_lock()) {
						processFrameIntrinsic(tcFrame.frame);
						frameProcessing_mutex.unlock();
						processed = true;

						if (onIRImageFrameOutput != NULL)
							onIRImageFrameOutput(tcFrame);
					}
				}
				else if (currentCalibrationStep == 4) {
					if (frameProcessing_mutex.try_lock()) {
						processFrameExtrinsic(tcFrame.frame);
						frameProcessing_mutex.unlock();
						processed = true;

						if (onIRImageFrameOutput != NULL)
							onIRImageFrameOutput(tcFrame);
					}
				}
				else
					if (onIRImageFrameOutput != NULL)
						onIRImageFrameOutput(tcFrame);
			}

			return processed;
		}

		/**
		 * 
		 */
		bool Calibration::processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame) {
			if(onMarkersFrameOutput != NULL)
				onMarkersFrameOutput(markerFrame);

			return false;
		}

		void Calibration::formatFrame(cv::Mat& frame) {
			int type = frame.type();
			uchar channels = 1 + (type >> CV_CN_SHIFT) + '0';

			if (type != 8) {
				if (channels == '1') {
					double min, max;
					cv::minMaxLoc(frame, &min, &max);
					if ( (min < 0) || (max > 255) )
					cv::normalize(frame, frame, 255, 0, cv::NORM_MINMAX);
				}

				frame.convertTo(frame, CV_8U);
			}
	
			if (channels != '3')
				cv::cvtColor(frame, frame, CV_GRAY2RGB);
		}

		void Calibration::processFrameIntrinsic(cv::Mat& frame) {
			int currentTime = getMSTime();
			int countownSinceTime = (calibrationPausedTime != 0)?calibrationPausedTime:currentTime;
			int timeSinceLastPic = countownSinceTime - lastPicTime;

			if(picNumber > 0)
				timeSinceLastPic -= (freezeDuration * 1000);

			if (currentCalibrationStep == 1) {
				formatFrame(frame);

				if (IRImageSize == NULL)
					IRImageSize = new cv::Size(frame.cols, frame.rows);
			}
			else
				if(RGBImageSize == NULL)
					RGBImageSize = new cv::Size(frame.cols, frame.rows);

			// we will add a legend on top of the screen to indicate calibration mode (intrinsic) and kinect mode (RGB or IR8)
			std::ostringstream oStringStreamTopLegend;
			oStringStreamTopLegend << "Intrinsic Calibration - Kinect mode : ";

			if(currentCalibrationStep == 1)
				oStringStreamTopLegend << "IR8";
			else if(currentCalibrationStep == 2)
				oStringStreamTopLegend << "RGB";

			// check if countdown has reached 0
			if(timeSinceLastPic >= countdownDuration)
			{
				std::vector<cv::Point2f> pointBuf;
				bool found = cv::findChessboardCorners(frame, chessBoardSize, pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

				if (found) {
					cv::Mat frameGray;
					cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
					cv::cornerSubPix(frameGray, pointBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

					if (currentCalibrationStep == 1)
						IRIntrinsicImagePoints.push_back(pointBuf);
					else
						RGBIntrinsicImagePoints.push_back(pointBuf);

					// we add some text on the image to indicate the progression (1/10, 2/10 etc ...)
					std::ostringstream oStringStreamPicCount;
					oStringStreamPicCount << (picNumber + 1) << "/" << frameCount;
					cv::putText(frame, oStringStreamPicCount.str(), cv::Point(frame.cols - 100, frame.rows - 40), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(234, 42, 250), 2);
					freeze();
					picNumber++;
					lastPicTime = getMSTime();

					// check if current calibration step is finished (= enough frames have been collected)
					if (((currentCalibrationStep == 1) && (IRIntrinsicImagePoints.size() >= frameCount)) || (RGBIntrinsicImagePoints.size() >= frameCount))
						goToNextStep();
				}

				cv::drawChessboardCorners(frame, chessBoardSize, cv::Mat(pointBuf), found);
			}
			else // the countdown hasn't reached 0 yet
			{
				std::ostringstream oStringStreamRemainingTime;

				if(calibrationPausedTime == 0)
				{
					// we add some text to update the countdown
					double remainingTime = (countdownDuration - timeSinceLastPic) / 1000;
					oStringStreamRemainingTime << "Next snapshot in " << (int)remainingTime + 1 << "s";
				}
				else
					oStringStreamRemainingTime << "PAUSED";

				cv::putText(frame, oStringStreamRemainingTime.str(), cv::Point(10, frame.rows - 40),cv::FONT_HERSHEY_SIMPLEX,0.8,cv::Scalar(234,42,250),2);
			}

			cv::putText(frame, oStringStreamTopLegend.str(), cv::Point(10,20),cv::FONT_HERSHEY_SIMPLEX,0.6,cv::Scalar(234,42,250),2);
		}

		void Calibration::processFrameExtrinsic(cv::Mat& frame) {
			if (currentCalibrationStep == 4) {
				formatFrame(frame);

				if (IRImageSize == NULL)
					IRImageSize = new cv::Size(frame.cols, frame.rows);
			}
			else
				if (RGBImageSize == NULL)
					RGBImageSize = new cv::Size(frame.cols, frame.rows);

			std::vector<cv::Point2f> pointBuf;
			bool found = cv::findChessboardCorners(frame, chessBoardSize, pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE);

			if (found) {
				cv::Mat frameGray;
				cv::cvtColor(frame, frameGray, cv::COLOR_BGR2GRAY);
				cv::cornerSubPix(frameGray, pointBuf, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

				if (currentCalibrationStep == 4)
					IRExtrinsicImagePoints = pointBuf;
				else
					RGBExtrinsicImagePoints = pointBuf;

				goToNextStep();
			}

			cv::drawChessboardCorners(frame, chessBoardSize, cv::Mat(pointBuf), found);
		}

		void* notifyAllCalibrationsDoneThreadFunc(void* pData) {
			Calibration* calibOp = (Calibration*)pData;

			if(calibOp->onAllCalibrationsDone != NULL)
				calibOp->onAllCalibrationsDone();

			return(NULL);
		}

		std::vector<cv::Point3f> Calibration::getObjectPointModel() {
			std::vector<cv::Point3f> objectPointModel;

			for (int y = 0; y < chessBoardSize.height; y++)
				for (int x = 0; x < chessBoardSize.width; x++)
					objectPointModel.push_back(cv::Point3f(x * squareSize, y * squareSize, 0));

			return objectPointModel;
		}

		datalib::IntrinsicCalibrationParametersSet Calibration::calculateIntrinsicParameters(std::vector<std::vector<cv::Point2f>> imagePoints, cv::Size imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs) {
			std::vector<cv::Mat> rvecs, tvecs;
			std::vector<float> reprojErrs;
			double totalAvgErr = 0;
			cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
			cameraMatrix.at<double>(0, 0) = 1.0;
			distCoeffs = cv::Mat::zeros(8, 1, CV_64F);
			std::vector<std::vector<cv::Point3f>> objectPoints;
			objectPoints.resize(imagePoints.size(), getObjectPointModel());
			cv::calibrateCamera(objectPoints, imagePoints, imageSize, cameraMatrix, distCoeffs, rvecs, tvecs, CV_CALIB_FIX_PRINCIPAL_POINT | CV_CALIB_ZERO_TANGENT_DIST | CV_CALIB_FIX_ASPECT_RATIO | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5);

			// compute reprojection errors
			std::vector<cv::Point2f> imagePoints2;
			int i, totalPoints = 0;
			double totalErr = 0, err;
			reprojErrs.resize(objectPoints.size());

			for (i = 0; i < objectPoints.size(); i++) {
				cv::projectPoints(cv::Mat(objectPoints[i]), rvecs[i], tvecs[i], cameraMatrix, distCoeffs, imagePoints2);
				err = cv::norm(cv::Mat(imagePoints[i]), cv::Mat(imagePoints2), CV_L2);
				int n = objectPoints[i].size();
				reprojErrs[i] = (float)std::sqrt(err*err / n);
				totalErr += err * err;
				totalPoints += n;
			}

			totalAvgErr = std::sqrt(totalErr / totalPoints);

			datalib::VEC4 K;

			for (int i = 0; i < 4; i++)
				K[i] = distCoeffs.at<double>(i);

			datalib::MAT33 A;

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					A[(i * 3) + j] = cameraMatrix.at<double>(i, j);

			return datalib::IntrinsicCalibrationParametersSet(K, A, totalAvgErr);
		}


		datalib::ExtrinsicCalibrationParametersSet Calibration::calculateExtrinsicParameters(std::vector<cv::Point2f> imagePoints, cv::Size imageSize, cv::Mat cameraMatrix, cv::Mat distCoeffs) {
			cv::Mat rvecs, tvecs;
			std::vector<cv::Point3f> objectPoints = getObjectPointModel();
			cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvecs, tvecs, false);
			cv::Mat rodriguesMatrix = cv::Mat::eye(3, 3, CV_64F);
			cv::Rodrigues(rvecs, rodriguesMatrix);

			datalib::VEC3 T;

			for (int i = 0; i < 3; i++)
				T[i] = tvecs.at<double>(i);
			
			datalib::MAT33 R;

			for (int i = 0; i < 3; i++)
				for (int j = 0; j < 3; j++)
					R[(i * 3) + j] = rodriguesMatrix.at<double>(i, j);

			// calculate reprojection error
			cv::Mat projectedPattern;
			cv::projectPoints(objectPoints, rvecs, tvecs, cameraMatrix, distCoeffs, projectedPattern);
			std::vector<float> errors;

			for (int i = 0; i < imagePoints.size(); ++i) {
				float dx = imagePoints.at(i).x - projectedPattern.at<float>(i, 0);
				float dy = imagePoints.at(i).y - projectedPattern.at<float>(i, 1);
				float err = sqrt(dx*dx + dy * dy);
				errors.push_back(err);
			}

			float errSum = std::accumulate(errors.begin(), errors.end(), 0);
			float errMean = errSum / errors.size();

			return datalib::ExtrinsicCalibrationParametersSet(T, R, errMean);
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		void Calibration::goToNextStep() {
			switch(currentCalibrationStep)
			{
				case 1:
					if(calibrateInIR) {
						if(intrinsicIRCalibRes != NULL)
							delete intrinsicIRCalibRes;

						intrinsicIRCalibRes = new datalib::IntrinsicCalibrationParametersSet(calculateIntrinsicParameters(IRIntrinsicImagePoints, *IRImageSize, IRCameraMatrix, IRDistCoeffs));
					}

					break;

				case 2:
					if(calibrateInRGB) {
						if(intrinsicRGBCalibRes != NULL)
							delete intrinsicRGBCalibRes;

						intrinsicRGBCalibRes = new datalib::IntrinsicCalibrationParametersSet(calculateIntrinsicParameters(RGBIntrinsicImagePoints, *RGBImageSize, RGBCameraMatrix, RGBDistCoeffs));
					}

					break;

				case 4:
					if(calibrateEx) {
						if(extrinsicIRCalibRes != NULL)
							delete extrinsicIRCalibRes;

						extrinsicIRCalibRes = new datalib::ExtrinsicCalibrationParametersSet(calculateExtrinsicParameters(IRExtrinsicImagePoints, *IRImageSize, IRCameraMatrix, IRDistCoeffs));
					}

					break;

				case 5:
					if(calibrateEx) {
						if(extrinsicRGBCalibRes != NULL)
							delete extrinsicRGBCalibRes;

						extrinsicRGBCalibRes = new datalib::ExtrinsicCalibrationParametersSet(calculateExtrinsicParameters(RGBExtrinsicImagePoints, *RGBImageSize, RGBCameraMatrix, RGBDistCoeffs));
					}

					break;
			}

			currentCalibrationStep++;

			if((currentCalibrationStep == 1) && (calibrateInIR == false))
				currentCalibrationStep++;

			if((currentCalibrationStep == 2) && (calibrateInRGB == false))
				currentCalibrationStep++;

			if((currentCalibrationStep == 3) && (calibrateEx == false))
				currentCalibrationStep++;

			if((currentCalibrationStep == 4) && (calibrateEx == false))
				currentCalibrationStep++;

			if((currentCalibrationStep == 5) && (calibrateEx == false))
				currentCalibrationStep++;
	
			switch(currentCalibrationStep)
			{
				case 1: // start intrinsic IR
					if (onCalibrateInfrared != NULL)
						onCalibrateInfrared();

					if (IRImageSize != NULL) {
						delete IRImageSize;
						IRImageSize = NULL;
					}

					IRIntrinsicImagePoints.clear();
					picNumber = 0;
					calibrationPausedTime = 0;
					lastPicTime = getMSTime();
					break;

				case 2: // start intrinsic RGB
					if (onCalibrateRGB != NULL)
						onCalibrateRGB();

					if (RGBImageSize != NULL) {
						delete RGBImageSize;
						RGBImageSize = NULL;
					}

					RGBIntrinsicImagePoints.clear();
					picNumber = 0;
					calibrationPausedTime = 0;
					lastPicTime = getMSTime();
					break;

				case 3: // notify begining of extrinsic calibration
					if(onBeforeExtrinsicCalibration != NULL)
						onBeforeExtrinsicCalibration();

					break;

				case 4: // start extrinsic IR
					if (onCalibrateInfrared != NULL)
						onCalibrateInfrared();

					IRExtrinsicImagePoints.clear();

					//if(extrinsicCalibrator != NULL)
						//delete extrinsicCalibrator;

					//extrinsicCalibrator = new ExtrinsicChessboardCalibrator(chessBoardWidth, chessBoardHeight, squareSize, getIntrinsicIRCalibRes());
					break;

				case 5: // start extrinsic RGB
					if (onCalibrateRGB != NULL)
						onCalibrateRGB();

					RGBExtrinsicImagePoints.clear();

					//if(extrinsicCalibrator != NULL)
						//delete extrinsicCalibrator;

					//extrinsicCalibrator = new ExtrinsicChessboardCalibrator(chessBoardWidth, chessBoardHeight, squareSize, getIntrinsicRGBCalibRes());
					break;

				case 6: // all requested calibrations are finished
					std::thread notifyAllCalibrationsDoneThread(notifyAllCalibrationsDoneThreadFunc, this);
					notifyAllCalibrationsDoneThread.detach();
					break;
			}
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		kocca::datalib::IntrinsicCalibrationParametersSet Calibration::getIntrinsicIRCalibRes() {
			if(intrinsicIRCalibRes != NULL) {
				kocca::datalib::IntrinsicCalibrationParametersSet threadSafeIRCalibRes = *intrinsicIRCalibRes;
				return threadSafeIRCalibRes;
			}
			else
				throw CalibrationDataNotAvailableException("Intrinsic IR Calibration parameters not set");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		kocca::datalib::IntrinsicCalibrationParametersSet Calibration::getIntrinsicRGBCalibRes() {
			if(intrinsicRGBCalibRes != NULL) {
				kocca::datalib::IntrinsicCalibrationParametersSet threadSafeRGBCalibRes = *intrinsicRGBCalibRes;
				return threadSafeRGBCalibRes;
			}
			else
				throw CalibrationDataNotAvailableException("Intrinsic RGB Calibration parameters not set");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		kocca::datalib::ExtrinsicCalibrationParametersSet Calibration::getExtrinsicIRCalibRes() {
			if(extrinsicIRCalibRes != NULL) {
				kocca::datalib::ExtrinsicCalibrationParametersSet threadSafeIRCalibRes = *extrinsicIRCalibRes;
				return threadSafeIRCalibRes;
			}
			else
				throw CalibrationDataNotAvailableException("Extrinsic IR Calibration parameters not set");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		kocca::datalib::ExtrinsicCalibrationParametersSet Calibration::getExtrinsicRGBCalibRes() {
			if(extrinsicRGBCalibRes != NULL) {
				kocca::datalib::ExtrinsicCalibrationParametersSet threadSafeRGBCalibRes = *extrinsicRGBCalibRes;
				return threadSafeRGBCalibRes;
			}
			else
				throw CalibrationDataNotAvailableException("Extrinsic RGB Calibration parameters not set");
		}

		void Calibration::freeze() {
			// display will be unfrozen after <freezeDuration> seconds
			unFreezeTime = getMSTime() + (unsigned long long)(freezeDuration * 1000.0);
		}

		void Calibration::setFreezeDuration(float _newFreezeDuration) {
			freezeDuration = _newFreezeDuration;
		}
	} // namespace operations
} // namespace kocca