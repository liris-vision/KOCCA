#include "KinectV2Sensor.h"

#include "Exceptions.h"

namespace kocca {
	KinectV2Sensor::KinectV2Sensor() {
		pSensor = NULL;
		colorFrameReader = NULL;
		infraredFrameReader = NULL;
		depthFrameReader = NULL;
		pColorThread = NULL;
		pInfraredThread = NULL;
		pDepthThread = NULL;
		keepThreadsRunning = false;
		onRGBFrame = NULL;
		onIRFrame = NULL;
		onDepthFrame = NULL;
	}

	void KinectV2Sensor::setUp() {
		if (SUCCEEDED(GetDefaultKinectSensor(&pSensor)) && (pSensor != NULL)) {
			if (SUCCEEDED(pSensor->Open())) {
				// set up color stream
				IColorFrameSource* colorFrameSource;

				if (SUCCEEDED(pSensor->get_ColorFrameSource(&colorFrameSource))) {
					if (SUCCEEDED(colorFrameSource->OpenReader(&colorFrameReader))) {
						colorFrameSource->Release();

						if (SUCCEEDED(colorFrameReader->SubscribeFrameArrived(&colorFrameEvent))) {
							keepThreadsRunning = true;
							pColorThread = new std::thread(&KinectV2Sensor::colorThreadFunc, this);
						}
						else
							throw KinectSensorStreamError("Failed to subscribe color frame reader to frame events");
					}
					else
						throw KinectSensorStreamError("Failed to open color frame source reader");
				}
				else
					throw KinectSensorStreamError("Failed to get color frame source");
				// ---

				// set up infrared source
				IInfraredFrameSource* infraredFrameSource;

				if (SUCCEEDED(pSensor->get_InfraredFrameSource(&infraredFrameSource))) {
					if (SUCCEEDED(infraredFrameSource->OpenReader(&infraredFrameReader))) {
						infraredFrameSource->Release();

						if (SUCCEEDED(infraredFrameReader->SubscribeFrameArrived(&infraredFrameEvent))) {
							keepThreadsRunning = true;
							pInfraredThread = new std::thread(&KinectV2Sensor::infraredThreadFunc, this);
						}
						else
							throw KinectSensorStreamError("Failed to subscribe color frame reader to frame events");
					}
					else
						throw KinectSensorStreamError("Failed to open infrared frame source reader");
				}
				else
					throw KinectSensorStreamError("Failed to get infrared frame source");
				// ---


				// set up depth source
				IDepthFrameSource* depthFrameSource;

				if (SUCCEEDED(pSensor->get_DepthFrameSource(&depthFrameSource))) {
					if (SUCCEEDED(depthFrameSource->OpenReader(&depthFrameReader))) {
						depthFrameSource->Release();

						if (SUCCEEDED(depthFrameReader->SubscribeFrameArrived(&depthFrameEvent))) {
							keepThreadsRunning = true;
							pDepthThread = new std::thread(&KinectV2Sensor::depthThreadFunc, this);
						}
						else
							throw KinectSensorStreamError("Failed to subscribe depth frame reader to frame events");
					}
					else
						throw KinectSensorStreamError("Failed to open depth framesource reader");
				}
				else
					throw KinectSensorStreamError("Failed to get depth frame source");
				// ---
			}
			else
				throw OpenKinectSensorFailedException("Failed to open Kinect sensor");
		}
		else
			throw DefaultKinectSensorNotFoundException("Unable to get default Kinect sensor");
	}

	void KinectV2Sensor::colorThreadFunc() {
		while (keepThreadsRunning) {
			if (onRGBFrame != NULL) {
				DWORD waitResult = WaitForSingleObject(reinterpret_cast<HANDLE>(colorFrameEvent), 10);

				if (waitResult == WAIT_OBJECT_0) {
					IColorFrameArrivedEventArgs* frameEventArgs = NULL;

					if (SUCCEEDED(colorFrameReader->GetFrameArrivedEventData(colorFrameEvent, &frameEventArgs))) {
						IColorFrameReference* frameReference = NULL;

						if (SUCCEEDED(frameEventArgs->get_FrameReference(&frameReference))) {
							IColorFrame* frame = NULL;

							if (SUCCEEDED(frameReference->AcquireFrame(&frame))) {
								frameReference->Release();
								frameEventArgs->Release();

								// build cv::Mat from Kinect frame data
								IFrameDescription* pFrameDescription = NULL;
								int frameWidth = 0;
								int frameHeight = 0;
								unsigned int frameBytePerPixel = 4;

								frame->get_FrameDescription(&pFrameDescription);
								pFrameDescription->get_Width(&frameWidth);
								pFrameDescription->get_Height(&frameHeight);
								pFrameDescription->get_BytesPerPixel(&frameBytePerPixel);
								pFrameDescription->Release();

								UINT frameSize = frameWidth * frameHeight * frameBytePerPixel;

								cv::Mat cvFrame(frameHeight, frameWidth, CV_8UC4);
								BYTE* cvFrameDataPtr = (BYTE*)cvFrame.data;

								frame->CopyConvertedFrameDataToArray(frameWidth * frameHeight * 4, cvFrameDataPtr, ColorImageFormat_Bgra);
								frame->Release();
								cv::cvtColor(cvFrame, cvFrame, CV_BGRA2RGB);

								// At this point, Kinect image is mirror-flipped (not sure why ?), so we flip it back
								cv::Mat flippedCVFrame;
								cv::flip(cvFrame, flippedCVFrame, 1);
								// ---

								onRGBFrame(flippedCVFrame);
							}
						}
					}
				}
			}
		}
	}

	void KinectV2Sensor::infraredThreadFunc() {
		while (keepThreadsRunning) {
			if (onIRFrame != NULL) {
				DWORD waitResult = WaitForSingleObject(reinterpret_cast<HANDLE>(infraredFrameEvent), 10);

				if (waitResult == WAIT_OBJECT_0) {
					IInfraredFrameArrivedEventArgs* frameEventArgs = NULL;

					if (SUCCEEDED(infraredFrameReader->GetFrameArrivedEventData(infraredFrameEvent, &frameEventArgs))) {
						IInfraredFrameReference* frameReference = NULL;

						if (SUCCEEDED(frameEventArgs->get_FrameReference(&frameReference))) {
							IInfraredFrame* frame = NULL;

							if (SUCCEEDED(frameReference->AcquireFrame(&frame))) {
								frameReference->Release();
								frameEventArgs->Release();

								IFrameDescription* pFrameDescription = NULL;
								int frameWidth = 0;
								int frameHeight = 0;
								unsigned int frameBytePerPixel = 0;

								frame->get_FrameDescription(&pFrameDescription);
								pFrameDescription->get_Width(&frameWidth);
								pFrameDescription->get_Height(&frameHeight);
								pFrameDescription->Release();

								cv::Mat cvFrame(frameHeight, frameWidth, CV_16UC1);
								UINT16* frameDataPtr = (UINT16*)cvFrame.data;

								frame->CopyFrameDataToArray(frameWidth * frameHeight, frameDataPtr);
								frame->Release();

								// At this point, Kinect image is mirror-flipped (not sure why ?), so we flip it back
								cv::Mat flippedCVFrame;
								cv::flip(cvFrame, flippedCVFrame, 1);

								onIRFrame(flippedCVFrame);
							}
						}
					}
				}
			}
		}
	}

	void KinectV2Sensor::depthThreadFunc() {
		while (keepThreadsRunning) {
			if (onDepthFrame != NULL) {
				DWORD waitResult = WaitForSingleObject(reinterpret_cast<HANDLE>(depthFrameEvent), 10);

				if (waitResult == WAIT_OBJECT_0) {
					IDepthFrameArrivedEventArgs* frameEventArgs = NULL;

					if (SUCCEEDED(depthFrameReader->GetFrameArrivedEventData(depthFrameEvent, &frameEventArgs))) {
						IDepthFrameReference* frameReference = NULL;

						if (SUCCEEDED(frameEventArgs->get_FrameReference(&frameReference))) {
							IDepthFrame* frame = NULL;

							if (SUCCEEDED(frameReference->AcquireFrame(&frame))) {
								frameReference->Release();
								frameEventArgs->Release();
								
								IFrameDescription* pFrameDescription = NULL;
								int frameWidth = 0;
								int frameHeight = 0;
								unsigned int frameBytePerPixel = 0;

								frame->get_FrameDescription(&pFrameDescription);
								pFrameDescription->get_Width(&frameWidth);
								pFrameDescription->get_Height(&frameHeight);
								pFrameDescription->Release();

								cv::Mat cvFrame(frameHeight, frameWidth, CV_16UC1);
								UINT16* frameDataPtr = (UINT16*)cvFrame.data;

								frame->CopyFrameDataToArray(frameWidth * frameHeight, frameDataPtr);
								frame->Release();

								// At this point, Kinect image is mirror-flipped (not sure why ?), so we flip it back
								cv::Mat flippedCVFrame;
								cv::flip(cvFrame, flippedCVFrame, 1);

								onDepthFrame(flippedCVFrame);
							}
						}
					}
				}
			}
		}
	}

	bool KinectV2Sensor::isAvailable() {
		if (pSensor != NULL) {
			BOOLEAN isAvailable = FALSE;

			if (SUCCEEDED(pSensor->get_IsAvailable(&isAvailable)))
				return isAvailable;
			else
				return false;
		}
		else
			return false;
	}

	bool KinectV2Sensor::isOpen() {
		if (isAvailable()) {
			BOOLEAN isOpen;

			if (SUCCEEDED(pSensor->get_IsOpen(&isOpen)))
				return isOpen;
			else
				return false;
		}
		else
			return false;
	}

	void KinectV2Sensor::stop() {
		keepThreadsRunning = false;

		if (colorFrameReader != NULL) {
			colorFrameReader->UnsubscribeFrameArrived(colorFrameEvent);
			colorFrameReader->Release();
			colorFrameReader = NULL;
		}

		if (infraredFrameReader != NULL) {
			infraredFrameReader->UnsubscribeFrameArrived(infraredFrameEvent);
			infraredFrameReader->Release();
			infraredFrameReader = NULL;
		}

		if (depthFrameReader != NULL) {
			depthFrameReader->UnsubscribeFrameArrived(depthFrameEvent);
			depthFrameReader->Release();
			depthFrameReader = NULL;
		}

		if ((pColorThread != NULL) && (pColorThread->joinable())) {
			pColorThread->join();
			delete pColorThread;
			pColorThread = NULL;
		}

		if ((pInfraredThread != NULL) && (pInfraredThread->joinable())) {
			pInfraredThread->join();
			delete pInfraredThread;
			pInfraredThread = NULL;
		}

		if ((pDepthThread != NULL) && (pDepthThread->joinable())) {
			pDepthThread->join();
			delete pDepthThread;
			pDepthThread = NULL;
		}

		if(isOpen())
			pSensor->Close();
	}

	KinectV2Sensor::~KinectV2Sensor() {
		stop();
	}

} // namespace kocca