#ifndef KOCCA_OPERATIONS_CALIBRATION_H
#define KOCCA_OPERATIONS_CALIBRATION_H

#include "Operation.h"

#include "../datalib/IntrinsicCalibrationParametersSet.h"
#include "../datalib/ExtrinsicCalibrationParametersSet.h"

#include "../datalib/TimeCodedFrame.h"

#include <mutex>
#include <atomic>

namespace kocca {
	namespace operations {

		/**
		 * The calibration operation is used for both intrinsic and extrinsic calibration of both Infrared and RGB sensors. The required calibration settings are set in the constructor, then the calibration objects handles incoming data according to these settings. It emmits notification during the calibration and at the end of it. The calibration results are then available through getters functions once the calibration is done.
		 */
		class Calibration: public Operation {
		public:

			/**
			 * Constructor. 
			 * @param _calibrateInIR Wheter or not we wish to perform intrinsic calibration of the infrared sensor.
			 * @param _calibrateInRGB  Wheter or not we wish to perform intrinsic calibration of the RGB sensor.
			 * @param _calibrateEx  Wheter or not we wish to perform extrinsic calibration of both sensors (It wouldn't make sense to do one without the other).
			 * @param _countdownDuration The duration of the countdown between each snapshot, in seconds.
			 * @param _frameCount The number of snapshots with successfull chessboard detection we want to use for intrinsic calibration of each desired sensor (extrinsic only uses one for each sensor).
			 * @param _chessBoardWidth The number of columns in the chessboard calibration target we use.
			 * @param _chessBoardHeight The number of rows in the chessboard calibration target we use.
			 * @param _squareSize The size (in millimeters) of each square in the chessboard calibration target we use.
			 * @param _freezeDuration The freeze duration we want after each successfull snapshot.
			 */
			Calibration(bool _calibrateInIR = true, bool _calibrateInRGB = true, bool _calibrateEx = true, int _countdownDuration = 5, int _frameCount = 10, int _chessBoardWidth = 10, int _chessBoardHeight = 7, float _squareSize = 34.0, float _freezeDuration = 1.0);

			/**
			 * Destructor.
			 */
			~Calibration();

			/**
			 * Processes an image frame of the depth stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the RGB stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the infrared stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes a MoCap markers frame of the MoCap stream, through the operation.
			 * @param markerFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame);

			/**
			 * Suspends the ongoing calibration until we call resume()
			 */
			void pause();

			/**
			 * Resumes a paused calibration
			 */
			void resume();

			/**
			 * Check if the calibration is currently paused.
			 */
			bool isPaused();

			/**
			 * Sets the countdownDuration setting.
			 * @param newCountdownDuration the new value for countdownDuration, in seconds.
			 */
			void setCountDownDuration(int newCountdownDuration);

			/**
			 * Callback function to notify the begining of a calibration of the RGB sensor.
			 */
			void (*onCalibrateRGB)();

			/**
			 * Callback function to notify the begining of a calibration of the infrared sensor.
			 */
			void (*onCalibrateInfrared)();

			/**
			 * Callback function to notify before the begining of the extrinsic calibration.
			 */
			void (*onBeforeExtrinsicCalibration)();

			/**
			 * Callback function to notify the end of all requested calibrations.
			 */
			void (*onAllCalibrationsDone)();

			/**
			 * Checks if the intrinsic calibration of the infrared sensor is planned during the operation.
			 */
			bool getCalibrateIntrinsicIR();

			/**
			 * Checks if the intrinsic calibration of the RGB sensor is planned during the operation
			 */
			bool getCalibrateIntrinsicRGB();

			/**
			 * Checks if the extrinsic calibration of both sensors is planned during the operation
			 */
			bool getCalibrateExtrinsic();

			/**
			 * Gets the result of intrinsic calibration of the infrared sensor.
			 * @throws CalibrationDataNotAvailableException if intrinsic calibration of the infrared sensor hasn't been requested or completed.
			 */
			kocca::datalib::IntrinsicCalibrationParametersSet getIntrinsicIRCalibRes();

			/**
			 * Gets the result of intrinsic calibration of the RGB sensor.
			 * @throws CalibrationDataNotAvailableException if intrinsic calibration of the RGB sensor hasn't been requested or completed.
			 */
			kocca::datalib::IntrinsicCalibrationParametersSet getIntrinsicRGBCalibRes();

			/**
			 * Gets the result of extrinsic calibration of the infrared sensor.
			 * @throws CalibrationDataNotAvailableException if extrinsic calibration of the infrared sensor hasn't been requested or completed.
			 */
			kocca::datalib::ExtrinsicCalibrationParametersSet getExtrinsicIRCalibRes();

			/**
			 * Gets the result of extrinsic calibration of the RGB sensor.
			 * @throws CalibrationDataNotAvailableException if extrinsic calibration of the RGB sensor hasn't been requested or completed.
			 */
			kocca::datalib::ExtrinsicCalibrationParametersSet getExtrinsicRGBCalibRes();

			/**
			 * Sets the intrinsic calibration parameters for the infrared sensor. This is usefull when we want to perform only extrinsic calibration, relying on already existing intrinsic calibration. 
			 */
			void setIntrinsicIRCalibRes(kocca::datalib::IntrinsicCalibrationParametersSet _intrinsicIRCalibRes);

			/**
			 * Sets the intrinsic calibration parameters for the RGB sensor. This is usefull when we want to perform only extrinsic calibration, relying on already existing intrinsic calibration.
			 */
			void setIntrinsicRGBCalibRes(kocca::datalib::IntrinsicCalibrationParametersSet _intrinsicRGBCalibRes);

			/**
			 * Moves to the next step of the calibration operation.
			 */
			void goToNextStep();

			/**
			 * Sets the freezeDuration setting.
			 * @param _newFreezeDuration the new value for freezeDuration, in seconds.
			 */
			void setFreezeDuration(float _newFreezeDuration);

		protected:

			/**
			 * Result of intrinsic calibration of infrared sensor. 
			 */
			kocca::datalib::IntrinsicCalibrationParametersSet* intrinsicIRCalibRes;

			/**
			 * Result of intrinsic calibration of RGB sensor.
			 */
			kocca::datalib::IntrinsicCalibrationParametersSet* intrinsicRGBCalibRes;

			/**
			 * Result of extrinsic calibration of infrared sensor. 
			 */
			kocca::datalib::ExtrinsicCalibrationParametersSet* extrinsicIRCalibRes;

			/**
			 * Result of extrinsic calibration of RGB sensor. 
			 */
			kocca::datalib::ExtrinsicCalibrationParametersSet* extrinsicRGBCalibRes;

			/**
			 * Wheter or not we wish to perform intrinsic calibration of the infrared sensor.
			 */
			bool calibrateInIR;

			/**
			 * Wheter or not we wish to perform intrinsic calibration of the RGB sensor.
			 */
			bool calibrateInRGB;

			/**
			 * Wheter or not we wish to perform extrinsic calibration of both sensors.
			 */
			bool calibrateEx;

			/**
			 * Current step through calibration operation. The values correspond to :
				- 1 : intrinsic infrared calibration
				- 2 : intrinsic RGB calibration
				- 3 : notify begining of extrinsic calibration
				- 4 : extrinsic infrared calibration
				- 5 : extrinsic RGB calibration
				<br><br>This is incremented at the end of each step, by a call to goToNextStep().
			 */
			std::atomic<int> currentCalibrationStep;

			/**
			 * Last time a snapshot with sucessfull detection of the chessboard was taken, in milliseconds of local system time. Snapshots are taken periodically, so we count time between each one to take them at the right moment and to display a countdown.
			 */
			int lastPicTime;

			/**
			 * Time at wich the calibration was paused, in milliseconds of local system time, or 0 if the calibration is not currently paused.
			 */
			int calibrationPausedTime;

			/**
			 * Number of snapshot with successfull chessboard detection transmitted to the calibrator object.
			 */
			unsigned int picNumber;

			/**
			 * A mutex to lock calls to processFrameIntrinsic() and processFrameExtrinsic(), so we make shure to process only one frame at the same time in a multi-threaded context.
			 */
			std::mutex frameProcessing_mutex;

			/**
			 * The duration of the countdown between each snapshot, in seconds.
			 */
			int countdownDuration;

			/**
			 * The number of snapshots with successfull chessboard detection we want to use for intrinsic calibration of each desired sensor (extrinsic only uses one for each sensor).
			 */
			int frameCount;

			/**
			 * The size of the chessboard we use, in number of corners (width and height are both 1 less than the corresponding number of squares)
			 */
			cv::Size chessBoardSize;

			/**
			 * The size (in pixels) of the images provided by the Infra-Red sensor of the Kinect
			 */
			cv::Size* IRImageSize;

			/**
			 * The size (in pixels) of the images provided by the RGB sensor of the Kinect
			 */
			cv::Size* RGBImageSize;

			/**
			 * 2D coordinates of the chessboard corners detected in each image used during intrinsic calibration of the Infra-Red sensor
			 */
			std::vector<std::vector<cv::Point2f>> IRIntrinsicImagePoints;

			/**
			 * 2D coordinates of the chessboard corners detected in each image used during intrinsic calibration of the RGB sensor
			 */
			std::vector<std::vector<cv::Point2f>> RGBIntrinsicImagePoints;

			/**
			 * 2D coordinates of the chessboard corners detected in the one image used during extrinsic calibration of the Infra-Red sensor
			 */
			std::vector<cv::Point2f> IRExtrinsicImagePoints;

			/**
			 * 2D coordinates of the chessboard corners detected in the one image used during extrinsic calibration of the RGB sensor
			 */
			std::vector<cv::Point2f> RGBExtrinsicImagePoints;

			/**
			 * Camera matrix for the IR sensor, calculated from intrinsic calibration
			 */
			cv::Mat IRCameraMatrix;
			
			/**
			 * Distortion coefficients for the IR sensor, calculated from intrinsic calibration 
			 */
			cv::Mat IRDistCoeffs;

			/**
			 * Camera matrix for the RGB sensor, calculated from intrinsic calibration
			 */
			cv::Mat RGBCameraMatrix;

			/**
			 * Distortion coefficients for the RGB sensor, calculated from intrinsic calibration
			 */
			cv::Mat RGBDistCoeffs;

			/**
			 * The size (in millimeters) of each square in the chessboard calibration target we use.
			 */
			float squareSize;

			/**
			 * The freeze duration we want after each successfull snapshot.
			 */
			float freezeDuration;

			/**
			 * The time at wich current freeze will have to stop, in milliseconds of local system time, or 0 if display is not currently frozen
			 */
			std::atomic<unsigned long long> unFreezeTime;

			/**
			 * Attempts to collect a frame for intrinsic calibration, when the time is right.
			 * @param frame a reference to the image frame to collect.
			 */
			void processFrameIntrinsic(cv::Mat& frame);

			/**
			 * Attempts to collect a frame for extrinsic calibration, when the time is right.
			 * @param frame a reference to the image frame to collect.
			 */
			void processFrameExtrinsic(cv::Mat& frame);

			/**
			 * Formats an image frame so it has the characteristics (number of channels and bit-size) required to be accepted by the OpenCV library to use it for calibration.
			 */
			void formatFrame(cv::Mat& frame);

			/**
			 * Freezes the frame processing for freezeDuration time
			 */
			void freeze();

			/**
			 * Calculates intrinsic calibration parameters for a sensor, when we have collected enough chessboard corners detections
			 * @param imagePoints the corners detected for each image
			 * @param the size of images
			 * @param cameraMatrix output result for the camera matrix
			 * @param distCoeffs output result for the camera distortion coefficients
			 * @return the intrinsic calibration parameters
			 */
			datalib::IntrinsicCalibrationParametersSet calculateIntrinsicParameters(std::vector<std::vector<cv::Point2f>> imagePoints, cv::Size imageSize, cv::Mat& cameraMatrix, cv::Mat& distCoeffs);

			/**
			 * Calculates extrinsic calibration parameters, when we have intrinsic calibration parameters and a succesful chessboard corner detection for that camera
			 * @param imagePoints the corners detected for the image
			 * @param imageSize the size of the image
			 * @param cameraMatrix the camera matrix calculated during intrinsic calibration
			 * @param distCoeffs the distortion coefficients calculated during intrinsic calibration
			 * @return the extrinsic calibration parameters
			 */
			datalib::ExtrinsicCalibrationParametersSet calculateExtrinsicParameters(std::vector<cv::Point2f> imagePoints, cv::Size imageSize, cv::Mat cameraMatrix, cv::Mat distCoeffs);

			/**
			 * Creates a model for each "objectPoint" (3D coordinates of each corner), according to the chessboard caracteristics (number of corners and size of squares)
			 */
			std::vector<cv::Point3f> getObjectPointModel();
		};
	} // namespace operations
} // namespace kocca

#endif // KOCCA_OPERATIONS_CALIBRATION_H
