#ifndef KOCCA_KINECT_V2_SENSOR_H
#define KOCCA_KINECT_V2_SENSOR_H

#include <atomic>
#include <thread>

#include <Kinect.h>

#include <opencv2/opencv.hpp>

namespace kocca {

	/**
	 * This class contains the code that interacts with the Kinect V2 sensor. It encapsulates Microsoft Kinect SDK V2 client code and outputs cv::Mat frames asynchronously, through callback methods. It is meant to be easy to use and to provide an API that is independant from MS Kinect SDK, making it easier to modify/replace/reimplement with another library in the future.
	 */
	class KinectV2Sensor {

	public:

		/**
		 * Constructor.
		 */
		KinectV2Sensor();

		/**
		 * Starts the Kinect device, subscribes to it's required streams (RGB, IR and depth) and starts the threads that will ewtract frames and output them as cv::Mat.
		 */
		void setUp();

		/**
		 * Stops the Kinect device and the threads that extract data.
		 */
		void stop();

		/**
		 * Checks if the MS Kinect service is available.
		 */
		bool isAvailable();

		/**
		 * Checks if the Kinect device is actually plugged, started and opened.
		 */
		bool isOpen();

		/**
		 * Callback function that will be asynchronously called to notify other objects of the availability of a new frame from the RGB stream.
		 * @param cv::Mat the new RGB frame.
		 */
		void(*onRGBFrame)(cv::Mat);

		/**
		 * Callback function that will be asynchronously called to notify other objects of the availability of a new frame from the InfraRed stream.
		 * @param cv::Mat the new InfraRed frame.
		 */
		void(*onIRFrame)(cv::Mat);

		/**
		 * Callback function that will be asynchronously called to notify other objects of the availability of a new frame from the depth stream.
		 * @param cv::Mat the new depth frame.
		 */
		void(*onDepthFrame)(cv::Mat);

		/**
		 * Destructor. It calls stop() to stop the kinect device and terminate the threads that extract images.
		 */
		~KinectV2Sensor();

	protected:

		/**
		 * A pointer to the object that reprensents the sensor in MS Kinect SDK.
		 */
		IKinectSensor* pSensor;

		/**
		 * A pointer to the object that reads frames from the RGB stream, in MS Kinect SDK.
		 */
		IColorFrameReader* colorFrameReader;


		/**
		 * A pointer to the object that reads frame from the InfraRed stream, in MS Kinect SDK.
		 */
		IInfraredFrameReader* infraredFrameReader;

		/**
		 * A pointer to the object that reads frame from the depth stream, in MS Kinect SDK.
		 */
		IDepthFrameReader* depthFrameReader;

		/**
		 * A handle to wait for the color frames arrival events, in MS Kinect SDK.
		 */
		WAITABLE_HANDLE colorFrameEvent;

		/**
		 * A handle to wait for the infrared frames arrival events, in MS Kinect SDK.
		 */
		WAITABLE_HANDLE infraredFrameEvent;

		/**
		 * A handle to wait for the depth frames arrival events, in MS Kinect SDK.
		 */
		WAITABLE_HANDLE depthFrameEvent;

		/**
		 * The thread that extracts the frames from the Kinect's RGB stream, convert their data to cv::Mat, and notifies of the new frames through onRGBFrame() callback function.
		 */
		std::thread* pColorThread;

		/**
		 * The thread that extracts the frames from the Kinect's infrared stream, convert their data to cv::Mat, and notifies of the new frames through onIRFrame() callback function.
		 */
		std::thread* pInfraredThread;

		/**
		 * The thread that extracts the frames from the Kinect's depth stream, convert their data to cv::Mat, and notifies of the new frames through onDepthFrame() callback function.
		 */
		std::thread* pDepthThread;

		/**
		 * Wheter or not we want to keep the threads running. When we set this to false, the threads will stop at their respective next loop.
		 */
		std::atomic<bool> keepThreadsRunning;

		/**
		 * Implementation for the pColorThread thread.
		 */
		void colorThreadFunc();

		/**
		 * Implementation for the pInfraredThread thread.
		 */
		void infraredThreadFunc();

		/**
		 * Implementation for the pDepthThread thread.
		 */
		void depthThreadFunc();
	};
} // namespace kocca

#endif //KOCCA_KINECT_V2_SENSOR_H
