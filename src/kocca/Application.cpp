#include "Application.h"

#include "Exceptions.h"
#include "datalib/KinectCalibrationFile.h"
#include "widgets/CalibrationTypesChoiceDialog.h"
#include "utils.h"
#include "operations/SequenceReading.h"
#include "operations/Monitoring.h"
#include "operations/SequenceRecording.h"
#include "operations/Calibration.h"
#include "datalib/SequenceFile.h"
#include "datalib/TaskProgress.h"

#include <gtkmm/filefilter.h>

#include <glib/gstdio.h>

#include <thread>
#include <WinBase.h>
#include <Psapi.h>

namespace kocca {
	bool Application::singletonInstanciated = false;
	Gtk::Main* Application::gtkApplication = NULL;
	widgets::MainWindow* Application::mainWindow = NULL;
	std::atomic<bool> Application::defaultKinectIsAvailable = false; // @todo check if this is still used => delete if it isn't, or move to KinectV2Sensor class if it is.
	KinectV2Sensor Application::kinect;
	std::atomic<kinectStreamType> Application::monitoredKinectStream = KINECT_STREAM_TYPE_RGB;
	datalib::IntrinsicCalibrationParametersSet* Application::intrinsicIRCalibRes;
	datalib::IntrinsicCalibrationParametersSet* Application::intrinsicRGBCalibRes;
	datalib::ExtrinsicCalibrationParametersSet* Application::extrinsicIRCalibRes;
	datalib::ExtrinsicCalibrationParametersSet* Application::extrinsicRGBCalibRes;
	operations::Operation* Application::currentOperation = NULL;
	std::mutex Application::currentOperation_mutex;
	NatNetClient* Application::natNetClient = NULL;
	std::vector<std::string> Application::natNetMarkerNames;
	std::vector<cv::Point3d>* Application::latestsMocapFramePoints = NULL;
	datalib::Sequence* Application::currentLoadedSequence = NULL;
	Gtk::Label* Application::loadingSequenceDataMsgLabel = NULL;
	Gtk::Label* Application::preparingRecordMsgLabel = NULL;
	Gtk::Label* Application::writingSequenceDataMsgLabel = NULL;
	Gtk::Label* Application::exportingSequenceFileMsgLabel = NULL;
	Gtk::Label* Application::unzippingSequenceFileMsgLabel = NULL;
	std::string Application::openFileAtStartup = "";
	bool Application::hasUnsavedCalibration = false;
	bool Application::hasUnsavedSequence = false;
	boost::filesystem::path Application::baseTempFolder = get_temp_folder_path();

	Application::Application(int argc, char* argv[]) {
		if(singletonInstanciated != true) {
			singletonInstanciated = true;
			mainWindow = NULL;
			currentOperation = NULL;
			natNetClient = NULL;
			latestsMocapFramePoints = NULL;
			currentLoadedSequence = NULL;

			loadingSequenceDataMsgLabel = NULL;
			preparingRecordMsgLabel = NULL;
			writingSequenceDataMsgLabel = NULL;
			exportingSequenceFileMsgLabel = NULL;
			unzippingSequenceFileMsgLabel = NULL;
			hasUnsavedCalibration = false;
			hasUnsavedSequence = false;

			if(argc > 1) {
				 std::string argString(argv[argc-1]);

				 if(boost::filesystem::exists(argString) && boost::filesystem::is_regular_file(argString))
					openFileAtStartup = argString;
			}

			// find gtk schema dir withing app's directory
			boost::filesystem::path glibSchemasDirPath = kocca::get_application_base_path() / "share" / "glib-2.0" / "schemas";
			bool res = g_setenv("GSETTINGS_SCHEMA_DIR", glibSchemasDirPath.string().c_str(), true);
			
			// create Application instance
			gtkApplication = new Gtk::Main(argc, argv);
			
			// find GTK standard icons withing app's directory
			boost::filesystem::path gtkIconsPath = kocca::get_application_base_path() / "share" / "icons";
			Gtk::IconTheme::get_default()->append_search_path(gtkIconsPath.string().c_str());

			// find and load the glade UI file
			boost::filesystem::path gladeFilePath = kocca::get_application_base_path() / "kocca.ui";
			Glib::RefPtr<Gtk::Builder> gtkBuilder = Gtk::Builder::create_from_file(gladeFilePath.string());
			gtkBuilder->get_widget_derived("mainWindow", mainWindow);

			// add kocca icon to main window
			boost::filesystem::path mainWindowIconFilePath = kocca::get_application_base_path() / "kocca.ico";
			
			if (boost::filesystem::exists(mainWindowIconFilePath))
				mainWindow->set_icon_from_file(mainWindowIconFilePath.string());

			mainWindow->show();

			mainWindow->kinectRGBStreamThumbnail->signal_selected_event().connect(sigc::ptr_fun(Application::onSelectKinectRGBStreamThumbnail));
			mainWindow->kinectInfraredStreamThumbnail->signal_selected_event().connect(sigc::ptr_fun(Application::onSelectKinectInfraredStreamThumbnail));
			mainWindow->kinectDepthStreamThumbnail->signal_selected_event().connect(sigc::ptr_fun(Application::onSelectKinectDepthStreamThumbnail));
			mainWindow->natNetConnectButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickNatNetConnectButton));
			mainWindow->loadCalibrationFileButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickLoadCalibrationFileButton));
			mainWindow->saveCalibrationToFileButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickSaveCalibrationToFileButton));
			mainWindow->clearCalibrationDataButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickClearCalibrationDataButton));
			mainWindow->startCalibrationButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickStartCalibrationButton));
			mainWindow->pauseCalibrationButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickPauseCalibrationButton));
			mainWindow->abortCalibrationButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickAbortCalibrationButton));
			mainWindow->countdownDurationSpinButton->signal_changed().connect(sigc::ptr_fun(Application::onChangeCountdownDurationSpinButton));
			mainWindow->snapshotFreezeDurationSpinButton->signal_changed().connect(sigc::ptr_fun(Application::onChangeSnapshotFreezeDurationSpinButton));
			mainWindow->stopButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickStopButton));
			mainWindow->recordButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickRecordButton));
			mainWindow->startPlaybackButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickStartPlaybackButton));
			mainWindow->goToBeginingButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickGoToBeginingButton));
			mainWindow->goToEndButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickGoToEndButton));
			mainWindow->previousButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickPreviousButton));
			mainWindow->nextButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickNextButton));
			mainWindow->videoTimeSliderScale->signal_change_value().connect(sigc::ptr_fun(Application::onMoveVideoTimeSliderScale));
			mainWindow->openSequenceButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickOpenSequenceButton));
			mainWindow->closeSequenceButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickCloseSequenceButton));
			mainWindow->exportSequenceButton->signal_clicked().connect(sigc::ptr_fun(Application::onClickExportSequenceButton));
			mainWindow->signal_drag_data_received().connect(sigc::ptr_fun(Application::onMainWindowDragDataReceived));
			mainWindow->signal_shortcuts_Space_event().connect(sigc::ptr_fun(Application::onMainWindowSpaceShortcut));
			mainWindow->signal_shortcuts_CtrlS_event().connect(sigc::ptr_fun(Application::onClickExportSequenceButton));
			mainWindow->signal_shortcuts_CtrlO_event().connect(sigc::ptr_fun(Application::onClickOpenSequenceButton));
			mainWindow->signal_shortcuts_CtrlQ_event().connect(sigc::ptr_fun(Application::onMainWindowSpaceShortcut));
			mainWindow->signal_shortcuts_CtrlW_event().connect(sigc::ptr_fun(Application::onClickCloseSequenceButton));
			mainWindow->signal_shortcuts_Left_event().connect(sigc::ptr_fun(Application::onClickPreviousButton));
			mainWindow->signal_shortcuts_Right_event().connect(sigc::ptr_fun(Application::onClickNextButton));
			mainWindow->signal_shortcuts_PageUp_event().connect(sigc::ptr_fun(Application::onClickGoToBeginingButton));
			mainWindow->signal_shortcuts_PageDown_event().connect(sigc::ptr_fun(Application::onClickGoToEndButton));
			mainWindow->signal_map_event().connect(sigc::ptr_fun(Application::onShowMainWindow), true);
			mainWindow->mocapMarkersListView->onSelectedMarkersChanged = Application::on_mocapMarkersListView_selected_markers_changed;
			mainWindow->monitor->onSelectedMarkersChanged = Application::on_monitor_selected_markers_changed;
			mainWindow->signal_delete_event().connect(sigc::ptr_fun(Application::onCloseMainWindow));

			setMonitoredKinectStream(KINECT_STREAM_TYPE_RGB);
		}
		else
			throw SingletonMultipleInstanciationException("Singleton class Application already instanciated, only one instance allowed");
	}
	
	void Application::run() {
		gtkApplication->run(*mainWindow);
	}

	Application::~Application() {
		delete gtkApplication;
		kinect.stop();
		singletonInstanciated = false;
	}
	
	/**
	 * @throws KinectCallbackAssignmentException
	 */
	void Application::disableCurrentOperationDisplay() {
		mainWindow->kinectRGBStreamThumbnail->invalidateLastFrame();
		mainWindow->kinectInfraredStreamThumbnail->invalidateLastFrame();
		mainWindow->kinectDepthStreamThumbnail->invalidateLastFrame();
		mainWindow->monitor->invalidateLastFrame();
	}

	gboolean Application::displayWaitMessage(gpointer data) {
		waitMessageParameters* pParams = (waitMessageParameters*)data;
		*(pParams->ppLabel) = mainWindow->waitMsgPopup->addOperationMessage(*(pParams->pMessage));
		delete data;
		return false;
	}

	void Application::addWaitMessage(std::string messageString, Gtk::Label** ppLabel) {
		waitMessageParameters* pParams = new waitMessageParameters;
		pParams->pMessage = new std::string(messageString);
		pParams->ppLabel = ppLabel;
		gdk_threads_add_idle(Application::displayWaitMessage, (gpointer)pParams);
	}
	
	gboolean Application::unDisplayWaitMessage(gpointer data) {
		waitMessageParameters* pParams = (waitMessageParameters*)data;
		mainWindow->waitMsgPopup->removeOperationMessage(pParams->ppLabel);
		delete data;
		return false;
	}

	void Application::removeWaitMessage(Gtk::Label** ppLabel) {
		waitMessageParameters* pParams = new waitMessageParameters;
		pParams->ppLabel = ppLabel;
		gdk_threads_add_idle(Application::unDisplayWaitMessage, (gpointer)pParams);
	}

	gboolean Application::displayErrorMessage(gpointer data) {
		messageBoxParameters* params = (messageBoxParameters*)data;
		std::string* message = params->message;
		Gtk::MessageDialog(*mainWindow, *message, false, Gtk::MESSAGE_ERROR).run();
		delete message;
		delete data;
		return false;
	}

	gboolean Application::displayInfoMessage(gpointer data) {
		messageBoxParameters* params = (messageBoxParameters*)data;
		std::string* message = params->message;
		Gtk::MessageDialog dialog(*mainWindow, *message, false, Gtk::MESSAGE_INFO);

		if(params->callBackFunction != NULL)
			dialog.signal_response().connect(sigc::ptr_fun(params->callBackFunction));

		dialog.run();
		delete message;
		delete data;
		return false;
	}

	void Application::errorMessageBox(std::string messageString) {
		messageBoxParameters* params = new messageBoxParameters;
		params->message = new std::string(messageString);
		gdk_threads_add_idle(Application::displayErrorMessage, (gpointer)params);
	}

	void Application::errorMessageBox(const char* messageCStr) {
		messageBoxParameters* params = new messageBoxParameters;
		params->message = new std::string(messageCStr);
		gdk_threads_add_idle(Application::displayErrorMessage, (gpointer)params);
	}

	void Application::infoMessageBox(std::string messageString, void (*callBackFunction)(int response_id)) {
		messageBoxParameters* params = new messageBoxParameters;
		params->message = new std::string(messageString);
		params->callBackFunction = callBackFunction;
		gdk_threads_add_idle(Application::displayInfoMessage, (gpointer)params);
	}

	void Application::infoMessageBox(const char* messageCStr, void (*callBackFunction)(int response_id)) {
		messageBoxParameters* params = new messageBoxParameters;
		params->message = new std::string(messageCStr);
		params->callBackFunction = callBackFunction;
		gdk_threads_add_idle(Application::displayInfoMessage, (gpointer)params);
	}

	void Application::checkKinectThreadImp() {
		BOOLEAN isAvailable = FALSE;

		for (int i = 0; (i < 50) && (isAvailable == FALSE); i++) {
			if (i != 0)
				Sleep(50);

			isAvailable = kinect.isAvailable();
		}

		if (isAvailable == FALSE)
			errorMessageBox("No available Kinect sensor was found");
		else
			defaultKinectIsAvailable = true;
	}

	void Application::setCurrentOperation(operations::Operation* newOperation) {
		bool previousUsesKinect, newUsesKinect;

		currentOperation_mutex.lock();

		if(currentOperation != NULL) {
			operations::Operation* previousOperation = currentOperation;
			currentOperation = newOperation;
			currentOperation_mutex.unlock();

			if(previousOperation->type == operations::KOCCA_RECORDING_OPERATION) {
				delete (operations::SequenceRecording*)previousOperation;
				previousUsesKinect = true;
			}
			else if(previousOperation->type == operations::KOCCA_READING_OPERATION) {
				delete (operations::SequenceReading*)previousOperation;
				previousUsesKinect = false;
			}
			else if(previousOperation->type == operations::KOCCA_MONITORING_OPERATION) {
				delete (operations::Monitoring*)previousOperation;
				previousUsesKinect = true;
			}
			else if(previousOperation->type == operations::KOCCA_CALIBRATION_OPERATION) {
				delete (operations::Calibration*)previousOperation;
				previousUsesKinect = true;
			}
		}
		else {
			currentOperation = newOperation;
			currentOperation_mutex.unlock();
			previousUsesKinect = false;
		}

		if(newOperation->type == operations::KOCCA_RECORDING_OPERATION) {
			newUsesKinect = true;
		}
		else if(newOperation->type == operations::KOCCA_READING_OPERATION) {
			newUsesKinect = false;
		}
		else if(newOperation->type == operations::KOCCA_MONITORING_OPERATION) {
			newUsesKinect = true;
		}
		else if(newOperation->type == operations::KOCCA_CALIBRATION_OPERATION) {
			newUsesKinect = true;
		}

		if(previousUsesKinect && !newUsesKinect)
			kinect.stop();
		else if(!previousUsesKinect && newUsesKinect)
			kinect.setUp();
	}

	void Application::onCurrentOperationColorImageFrameOutputThread(datalib::TimeCodedFrame* tcFrame) {
		try {
			mainWindow->kinectRGBStreamThumbnail->setNextFrame((*tcFrame).frame);

			if(monitoredKinectStream == KINECT_STREAM_TYPE_RGB) {
				mainWindow->monitor->setNextFrame((*tcFrame).frame);
			
				if (currentOperation->type == operations::KOCCA_READING_OPERATION)
					mainWindow->setMonitorFrameTime((*tcFrame).time);
				else
					if (currentOperation->type == operations::KOCCA_RECORDING_OPERATION)
						mainWindow->monitor->setRecordingTime((*tcFrame).time);
			}
		}
		catch(EmptyFrameException& e) {
			std::cerr << "onCurrentOperationImageFrameOutput() error: " << e.what() << ". (frame time :" << (*tcFrame).time << ")" << std::endl;
		}

		delete tcFrame;
	}

	void Application::onCurrentOperationColorImageFrameOutput(datalib::TimeCodedFrame tcFrame) {
		new std::thread(&Application::onCurrentOperationColorImageFrameOutputThread, new datalib::TimeCodedFrame(tcFrame));
	}

	void Application::onCurrentOperationIRImageFrameOutputThread(datalib::TimeCodedFrame* tcFrame) {
		try {
			mainWindow->kinectInfraredStreamThumbnail->setNextFrame((*tcFrame).frame);

			if (monitoredKinectStream == KINECT_STREAM_TYPE_INFRARED) {
				mainWindow->monitor->setNextFrame((*tcFrame).frame);

				if (currentOperation->type == operations::KOCCA_READING_OPERATION)
					mainWindow->setMonitorFrameTime((*tcFrame).time);
			}
		}
		catch (EmptyFrameException& e) {
			std::cerr << "onCurrentOperationImageFrameOutput() error: " << e.what() << ". (frame time :" << (*tcFrame).time << ")" << std::endl;
		}

		delete tcFrame;
	}

	void Application::onCurrentOperationIRImageFrameOutput(datalib::TimeCodedFrame tcFrame) {
		new std::thread(&Application::onCurrentOperationIRImageFrameOutputThread, new datalib::TimeCodedFrame(tcFrame));
	}

	void Application::onCurrentOperationDepthFrameOutputThread(datalib::TimeCodedFrame* tcFrame) {
		try {
			cv::normalize((*tcFrame).frame, (*tcFrame).frame, 65535, 0, cv::NORM_MINMAX);
			mainWindow->kinectDepthStreamThumbnail->setNextFrame((*tcFrame).frame);

			if (monitoredKinectStream == KINECT_STREAM_TYPE_DEPTH) {
				mainWindow->monitor->setNextFrame((*tcFrame).frame);
			
				if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION))
					mainWindow->setMonitorFrameTime((*tcFrame).time);
			}
		}
		catch(EmptyFrameException& e) {
			std::cerr << "onCurrentOperationDepthFrameOutput() error: " << e.what() << ". (frame time :" << (*tcFrame).time << ")" << std::endl;
		}

		delete tcFrame;
	}

	void Application::onCurrentOperationDepthFrameOutput(datalib::TimeCodedFrame tcFrame) {
		new std::thread(&Application::onCurrentOperationDepthFrameOutputThread, new datalib::TimeCodedFrame(tcFrame));
	}

	void Application::onCurrentOperationMarkerFrameOutputThread(datalib::MocapMarkerFrame* frame) {
		mainWindow->mocapMarkersListView->setMarkerFrame(*frame);
		mainWindow->monitor->setNextMocapMarkerFrame(*frame);

		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION))
			mainWindow->setMocapFrameTime((*frame).time);

		currentOperation_mutex.unlock();

		delete frame;
	}

	void Application::onCurrentOperationMarkerFrameOutput(datalib::MocapMarkerFrame frame) {
		new std::thread(&Application::onCurrentOperationMarkerFrameOutputThread, new datalib::MocapMarkerFrame(frame));
	}

	void Application::updateSaveButton() {
		mainWindow->enableSaveCalibrationButton(
				(intrinsicIRCalibRes != NULL)
			||	(intrinsicRGBCalibRes != NULL)
			|| 	(extrinsicIRCalibRes != NULL)
			|| 	(extrinsicRGBCalibRes != NULL)
		);

		mainWindow->enableClearCalibrationButton(
				(intrinsicIRCalibRes != NULL)
			||	(intrinsicRGBCalibRes != NULL)
			|| 	(extrinsicIRCalibRes != NULL)
			|| 	(extrinsicRGBCalibRes != NULL)
		);

		if(
				(intrinsicIRCalibRes == NULL)
			&&	(intrinsicRGBCalibRes == NULL)
			&& 	(extrinsicIRCalibRes == NULL)
			&& 	(extrinsicRGBCalibRes == NULL)
		)
			hasUnsavedCalibration = false;
	}

	void Application::setIntrinsicIRCalibrationParameters(datalib::IntrinsicCalibrationParametersSet newParameters) {
		if(intrinsicIRCalibRes != NULL)
			delete intrinsicIRCalibRes;

		intrinsicIRCalibRes = new datalib::IntrinsicCalibrationParametersSet(newParameters);
		mainWindow->intrinsicIRResultCell->setResult(intrinsicIRCalibRes);
		updateSaveButton();

		if (monitoredKinectStream == KINECT_STREAM_TYPE_INFRARED || monitoredKinectStream == KINECT_STREAM_TYPE_DEPTH)
			mainWindow->monitor->setIntrinsicCalibrationParams(intrinsicIRCalibRes);
	}

	void Application::setIntrinsicRGBCalibrationParameters(datalib::IntrinsicCalibrationParametersSet newParameters) {
		if(intrinsicRGBCalibRes != NULL)
			delete intrinsicRGBCalibRes;

		intrinsicRGBCalibRes = new datalib::IntrinsicCalibrationParametersSet(newParameters);
		mainWindow->intrinsicRGBResultCell->setResult(intrinsicRGBCalibRes);
		updateSaveButton();

		if (monitoredKinectStream == KINECT_STREAM_TYPE_RGB)
			mainWindow->monitor->setIntrinsicCalibrationParams(intrinsicRGBCalibRes);
	}

	void Application::setExtrinsicIRCalibrationParameters(datalib::ExtrinsicCalibrationParametersSet newParameters) {
		if(extrinsicIRCalibRes != NULL)
			delete extrinsicIRCalibRes;

		extrinsicIRCalibRes = new datalib::ExtrinsicCalibrationParametersSet(newParameters);
		mainWindow->extrinsicIRResultCell->setResult(extrinsicIRCalibRes);
		updateSaveButton();

		if (monitoredKinectStream == KINECT_STREAM_TYPE_INFRARED || monitoredKinectStream == KINECT_STREAM_TYPE_DEPTH)
			mainWindow->monitor->setExtrinsicCalibrationParams(extrinsicIRCalibRes);
	}

	void Application::setExtrinsicRGBCalibrationParameters(datalib::ExtrinsicCalibrationParametersSet newParameters) {
		if(extrinsicRGBCalibRes != NULL)
			delete extrinsicRGBCalibRes;

		extrinsicRGBCalibRes = new datalib::ExtrinsicCalibrationParametersSet(newParameters);
		mainWindow->extrinsicRGBResultCell->setResult(extrinsicRGBCalibRes);
		updateSaveButton();

		if (monitoredKinectStream == KINECT_STREAM_TYPE_RGB)
			mainWindow->monitor->setExtrinsicCalibrationParams(extrinsicRGBCalibRes);
	}
	
	/**
	 * Checks if a cv::Mat contains only zeros
	 */
	bool Application::rgbMatIsZeroFilled(cv::Mat rgbMat) {
		cv::Mat channels[3];
		cv::split(rgbMat,channels);
		return ((countNonZero(channels[0]) < 1) && (countNonZero(channels[1]) < 1) && (countNonZero(channels[2]) < 1));
	}

	void Application::updateNatNetMarkersNames() {
		sDataDescriptions* pNatNetDataDescr = NULL;
		int nbDescriptions = natNetClient->GetDataDescriptions(&pNatNetDataDescr);

		for(int i = 0; ((i < nbDescriptions) && (natNetMarkerNames.empty())); i++) {
			sDataDescription& dataDescr = pNatNetDataDescr->arrDataDescriptions[i];
			std::string markerSetName(dataDescr.Data.MarkerSetDescription->szName);

			if(markerSetName == "all") {
				natNetMarkerNames.clear();
				int markerNamesSize = dataDescr.Data.MarkerSetDescription->nMarkers;

				for(int j = 0; j < markerNamesSize; j++)
					natNetMarkerNames.push_back(std::string(dataDescr.Data.MarkerSetDescription->szMarkerNames[j]));
			}
		}
	}

	// callback function called when data is received from the NatNet Server (Motive)
	void __cdecl Application::onNatNetData(sFrameOfMocapData* data, void* pUserData) {
		unsigned long long sysTime = getMSTime();
		currentOperation_mutex.lock();

		if ((natNetClient != NULL) && (data != NULL) && (currentOperation != NULL) && (currentOperation->type != kocca::operations::KOCCA_READING_OPERATION) && (currentOperation->type != kocca::operations::KOCCA_CALIBRATION_OPERATION)) {
			currentOperation_mutex.unlock();

			if (natNetMarkerNames.empty())
				updateNatNetMarkersNames();

			// @TODO : utiliser plutot le temps de la frame : data->fTimestamp
			// -> multiplier par 1000 pour convertir en ms
			// -> puis appliquer correction de latence mesuree lorsque cette mesure sera implementee
			datalib::MocapMarkerFrame markerFrame(sysTime);

			bool allFound = false;

			for (int i = 0; (i < data->nMarkerSets) && (!allFound); i++) {
				sMarkerSetData markerSet = data->MocapData[i];

				if (std::string(markerSet.szName) == "all") {
					allFound = true;

					for (int j = 0; (j < markerSet.nMarkers); j++) {
						double x = markerSet.Markers[j][0] * 1000;
						double y = markerSet.Markers[j][2] * 1000;
						double z = (-markerSet.Markers[j][1]) * 1000;

						std::string name;

						if (j < natNetMarkerNames.size())
							name = natNetMarkerNames.at(j);
						else {
							std::ostringstream nameSS;
							nameSS << "Unidentified_" << j;
							name = nameSS.str();
						}

						try {
							markerFrame.add_marker(datalib::MocapMarker(x, y, z, name));
						}
						catch (DuplicateMarkerNameException& e) {
							errorMessageBox(e.what());
						}
					}
				}
			}

			currentOperation_mutex.lock();

			if ((natNetClient != NULL) && (currentOperation->type != operations::KOCCA_READING_OPERATION) && (currentOperation->type != operations::KOCCA_CALIBRATION_OPERATION)) {
				try {
					currentOperation->processMarkersFrame(markerFrame);
				}
				catch (std::runtime_error& re) {
					std::string errorMessageStr;

					if (currentOperation->type == operations::KOCCA_RECORDING_OPERATION) {
						onClickStopButton();
						std::ostringstream errorMessage;
						errorMessage << "Recording failed !\rAn error occured during recording operation : " << std::endl << re.what() << std::endl;
						errorMessageStr = errorMessage.str();
					}
					else
						errorMessageStr = re.what();

					errorMessageBox(errorMessageStr);
				}
			}

			currentOperation_mutex.unlock();
		}
		else
			currentOperation_mutex.unlock();
	}

	void Application::disconnectNatNetClient() {
		if(natNetClient != NULL)
		{
			natNetClient->Uninitialize();
			delete natNetClient;
			natNetClient = NULL;
		}

		natNetMarkerNames.clear();
		mainWindow->set_natnet_is_connected(false);
	}

	void Application::connectNatNetClient(bool displayMessageOnFailure) {
		if(natNetClient != NULL)
			disconnectNatNetClient();

		// initialise connection to NatNet Server 
		natNetClient = new NatNetClient(ConnectionType_Multicast);

		char myIP[128] = "";
		sprintf(myIP, "%s", mainWindow->natNetClientAddressEntry->get_text().c_str());

		char serverIP[128] = "";
		sprintf(serverIP, "%s", mainWindow->natNetServerAddressEntry->get_text().c_str());
	
		int connectionResultCode = natNetClient->Initialize(myIP, serverIP);

		if((connectionResultCode == ErrorCode_OK))
		{
			sServerDescription natNetServerDescription;
			memset(&natNetServerDescription, 0, sizeof(natNetServerDescription));
			natNetClient->GetServerDescription(&natNetServerDescription);

			if(natNetServerDescription.HostPresent)
			{
				natNetClient->SetDataCallback(onNatNetData);
				mainWindow->set_natnet_is_connected(true);
				updateNatNetMarkersNames();
				mainWindow->mocapMarkersListView->setInitMarkersNames(natNetMarkerNames);
			}
			else
			{
				disconnectNatNetClient();

				if(displayMessageOnFailure)
					errorMessageBox("Failed to get NatNet server description.");
			}
		}
		else
		{
			disconnectNatNetClient();

			if(displayMessageOnFailure)
				errorMessageBox("Failed to initialise NatNet client.");
		}
	}

	void Application::activateMonitoring() {
		connectNatNetClient(false);
 		operations::Monitoring* newOperation = new operations::Monitoring();
		newOperation->onColorImageFrameOutput = onCurrentOperationColorImageFrameOutput;
		newOperation->onIRImageFrameOutput = onCurrentOperationIRImageFrameOutput;
		newOperation->onDepthFrameOutput = onCurrentOperationDepthFrameOutput;
		newOperation->onMarkersFrameOutput = onCurrentOperationMarkerFrameOutput;

		try {
			setCurrentOperation(newOperation);
			mainWindow->enableMonitoringWidgets();
			updateSaveButton();
		}
		catch (KinectException& ke) {
			std::cout << "KinectException" << std::endl;
			errorMessageBox(ke.what());
		}
	}

	void Application::onInfoBeforeExtrinsicResponse(int response_id) {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION)) {
			((operations::Calibration*)currentOperation)->goToNextStep();
		}

		currentOperation_mutex.unlock();
	}

	void Application::onCalibrationOperationBeforeExtrinsic() {
		infoMessageBox("KOCCA will now process extrinsic calibration.\rMake sure the chessboard isn't moved during this step\rClick OK when you're ready.", &onInfoBeforeExtrinsicResponse);
	}

	void Application::onCalibrationOperationAllDone() {
		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION)) {
			infoMessageBox("Calibration done.");

			try {
				if(((operations::Calibration*)currentOperation)->getCalibrateIntrinsicIR())
					setIntrinsicIRCalibrationParameters(((operations::Calibration*)currentOperation)->getIntrinsicIRCalibRes());

				if(((operations::Calibration*)currentOperation)->getCalibrateIntrinsicRGB())
					setIntrinsicRGBCalibrationParameters(((operations::Calibration*)currentOperation)->getIntrinsicRGBCalibRes());

				if(((operations::Calibration*)currentOperation)->getCalibrateExtrinsic()) {
					setExtrinsicIRCalibrationParameters(((operations::Calibration*)currentOperation)->getExtrinsicIRCalibRes());
					setExtrinsicRGBCalibrationParameters(((operations::Calibration*)currentOperation)->getExtrinsicRGBCalibRes());
				}
			}
			catch (std::exception& e) {
				errorMessageBox(e.what());
			}

			currentOperation_mutex.unlock();

			hasUnsavedCalibration = true;

			updateSaveButton();
			activateMonitoring();
		}
		else
			currentOperation_mutex.unlock();
	}

	void Application::onCurrentOperationCalibrateRGB() {
		new std::thread(&Application::setMonitoredKinectStream, KINECT_STREAM_TYPE_RGB);
	}

	void Application::onCurrentOperationCalibrateInfrared() {
		new std::thread(&Application::setMonitoredKinectStream, KINECT_STREAM_TYPE_INFRARED);
	}

	void Application::onClickStartCalibrationButton() {
		if (kinect.isAvailable()) {
			widgets::CalibrationTypesChoiceDialog calTypesDialog(
				widgets::CALTYPECHOICE_DIALOG_CALIBRATE,
				true,
				true,
				true,
				true,
				(intrinsicIRCalibRes == NULL),
				(intrinsicRGBCalibRes == NULL),
				(extrinsicIRCalibRes == NULL),
				(extrinsicRGBCalibRes == NULL));

			if(calTypesDialog.run() == Gtk::RESPONSE_OK)
			{
				mainWindow->enterCalibrationMode();

				bool calibrateIntIR = calTypesDialog.inIR_checked();
				bool calibrateInRGB = calTypesDialog.inRGB_checked();
				bool calibrateEx = (calTypesDialog.exIR_checked() && calTypesDialog.exRGB_checked());
				int countdownDuration = mainWindow->countdownDurationSpinButton->get_value() * 1000;
				int frameCount = mainWindow->snapshotsCountSpinButton->get_value();
				int chessBoardWidth = mainWindow->chessBoardWidthSpinButton->get_value();
				int chessBoardHeight = mainWindow->chessBoardHeightSpinButton->get_value();
				float squareSize = mainWindow->chessBoardSquareSizeSpinButton->get_value();
				float freezeDuration = mainWindow->snapshotFreezeDurationSpinButton->get_value();

				operations::Calibration* calibrationOperation = new operations::Calibration(calibrateIntIR, calibrateInRGB, calibrateEx, countdownDuration, frameCount, chessBoardWidth, chessBoardHeight, squareSize, freezeDuration);
				calibrationOperation->onColorImageFrameOutput = onCurrentOperationColorImageFrameOutput;
				calibrationOperation->onIRImageFrameOutput = onCurrentOperationIRImageFrameOutput;
				calibrationOperation->onDepthFrameOutput = onCurrentOperationDepthFrameOutput;
				calibrationOperation->onMarkersFrameOutput = onCurrentOperationMarkerFrameOutput;
				calibrationOperation->onCalibrateRGB = onCurrentOperationCalibrateRGB;
				calibrationOperation->onCalibrateInfrared = onCurrentOperationCalibrateInfrared;
				calibrationOperation->onBeforeExtrinsicCalibration = onCalibrationOperationBeforeExtrinsic;
				calibrationOperation->onAllCalibrationsDone = onCalibrationOperationAllDone;

				if(calibrateEx) {
					if(!calibrateIntIR)
						calibrationOperation->setIntrinsicIRCalibRes(*intrinsicIRCalibRes);

					if(!calibrateInRGB)
						calibrationOperation->setIntrinsicRGBCalibRes(*intrinsicRGBCalibRes);
				}

				setCurrentOperation(calibrationOperation);
				calibrationOperation->goToNextStep();
			}
		}
	}

	void Application::onClickPauseCalibrationButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION)) {
			if(!((operations::Calibration*)currentOperation)->isPaused()) {
				((operations::Calibration*)currentOperation)->pause();
				mainWindow->pauseCalibrationButton->remove();
				mainWindow->pauseCalibrationButton->add_pixlabel(get_img_path("pause.png"), "Resume");
			}
			else {
				((operations::Calibration*)currentOperation)->resume();
				mainWindow->pauseCalibrationButton->remove();
				mainWindow->pauseCalibrationButton->add_pixlabel(get_img_path("pause.png"), "Pause     ");
			}
		}

		currentOperation_mutex.unlock();
	}

	void Application::onClickAbortCalibrationButton() {
		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION)) {
			currentOperation_mutex.unlock();
			activateMonitoring();
		}
		else
			currentOperation_mutex.unlock();
	}
	
	/**
	 * @throws CalibrationDataNotAvailableException
	 * @throws InvalidKinectCalibrationFileException
	 */
	void Application::loadCalibrationFile(const char* calibrationFilePath, bool forceLoadAll) {
		datalib::KinectCalibrationFile kcfFile(calibrationFilePath);

		widgets::CalibrationTypesChoiceDialog calTypesDialog(
				widgets::CALTYPECHOICE_DIALOG_IMPORT,
				kcfFile.hasIntrinsicIRParameters(),
				kcfFile.hasIntrinsicRGBParameters(),
				kcfFile.hasExtrinsicIRParameters(),
				kcfFile.hasExtrinsicRGBParameters(),
				(intrinsicIRCalibRes == NULL),
				(intrinsicRGBCalibRes == NULL),
				(extrinsicIRCalibRes == NULL),
				(extrinsicRGBCalibRes == NULL));

		if(forceLoadAll || (calTypesDialog.run() == Gtk::RESPONSE_OK)) {
			if((forceLoadAll || calTypesDialog.inIR_checked()) && kcfFile.hasIntrinsicIRParameters()) {
				setIntrinsicIRCalibrationParameters(kcfFile.getIntrinsicIRParameters());

				if(currentLoadedSequence != NULL)
					currentLoadedSequence->setIntrinsicIRCalibrationParameters(kcfFile.getIntrinsicIRParameters());
			}

			if((forceLoadAll || calTypesDialog.inRGB_checked()) && kcfFile.hasIntrinsicRGBParameters()) {
				setIntrinsicRGBCalibrationParameters(kcfFile.getIntrinsicRGBParameters());

				if(currentLoadedSequence != NULL)
					currentLoadedSequence->setIntrinsicRGBCalibrationParameters(kcfFile.getIntrinsicRGBParameters());
			}

			if((forceLoadAll || calTypesDialog.exIR_checked()) && kcfFile.hasExtrinsicIRParameters()) {
				setExtrinsicIRCalibrationParameters(kcfFile.getExtrinsicIRParameters());

				if(currentLoadedSequence != NULL)
					currentLoadedSequence->setExtrinsicIRCalibrationParameters(kcfFile.getExtrinsicIRParameters());
			}

			if((forceLoadAll || calTypesDialog.exRGB_checked()) && kcfFile.hasExtrinsicRGBParameters()) {
				setExtrinsicRGBCalibrationParameters(kcfFile.getExtrinsicRGBParameters());

				if(currentLoadedSequence != NULL)
					currentLoadedSequence->setExtrinsicRGBCalibrationParameters(kcfFile.getExtrinsicRGBParameters());
			}
		}
	}

	void Application::onClickLoadCalibrationFileButton() {
		bool appHasCalibData = ((intrinsicIRCalibRes != NULL) || (extrinsicIRCalibRes != NULL) || (intrinsicRGBCalibRes != NULL) || (extrinsicRGBCalibRes != NULL));
		bool seqHasCalibData = ((currentLoadedSequence != NULL) && currentLoadedSequence->hasCalibrationData());

		Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : there already is some calibration data currently loaded.\nIf you import some new, current data wil be overwriten and will not be recoverable (unless you already saved it). ", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
		confirmPopup.set_secondary_text("Are you sure you want to continue ?");

		if((!appHasCalibData && !seqHasCalibData) || (confirmPopup.run() == Gtk::RESPONSE_OK)) {
			confirmPopup.hide();

			Gtk::FileChooserDialog fileDialog("Please choose a kinect calibration file to open", Gtk::FILE_CHOOSER_ACTION_OPEN);
			fileDialog.set_transient_for(*mainWindow);

			//Add response buttons to the dialog:
			fileDialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
			fileDialog.add_button("Select", Gtk::RESPONSE_OK);

			Glib::RefPtr<Gtk::FileFilter> filter_kcf = Gtk::FileFilter::create();
			filter_kcf->set_name("Kinect calibration files");
			filter_kcf->add_pattern("*.kcf");
			fileDialog.add_filter(filter_kcf);

			if(fileDialog.run() == Gtk::RESPONSE_OK)
			{
				std::string kcfFileName = fileDialog.get_filename();
				fileDialog.hide();

				try {
					loadCalibrationFile(kcfFileName.c_str());
				}
				catch(std::exception& e) {
					errorMessageBox(e.what());
				}
			}
		}
	}

	void Application::onClickSaveCalibrationToFileButton() {
		Gtk::FileChooserDialog dialog("Please indicate where to save kinect calibration file", Gtk::FILE_CHOOSER_ACTION_SAVE);
		dialog.set_transient_for(*mainWindow);

		//Add response buttons the the dialog:
		dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
		dialog.add_button("Select", Gtk::RESPONSE_OK);

		Glib::RefPtr<Gtk::FileFilter> filter_kcf = Gtk::FileFilter::create();
		filter_kcf->set_name("Kinect calibration files");
		filter_kcf->add_pattern("*.kcf");
		dialog.add_filter(filter_kcf);

		widgets::CalibrationTypesChoiceDialog calTypesDialog(
			widgets::CALTYPECHOICE_DIALOG_EXPORT,
			(intrinsicIRCalibRes != NULL),
			(intrinsicRGBCalibRes != NULL),
			(extrinsicIRCalibRes != NULL),
			(extrinsicRGBCalibRes != NULL),
			true,
			true,
			true,
			true);

		if((calTypesDialog.run() == Gtk::RESPONSE_OK) && (dialog.run() == Gtk::RESPONSE_OK))
		{
			try {
				datalib::KinectCalibrationFile kcfFile;

				if(calTypesDialog.inIR_checked())
					kcfFile.setIntrinsicIRParameters(*intrinsicIRCalibRes);

				if(calTypesDialog.inRGB_checked())
					kcfFile.setIntrinsicRGBParameters(*intrinsicRGBCalibRes);

				if(calTypesDialog.exIR_checked())
					kcfFile.setExtrinsicIRParameters(*extrinsicIRCalibRes);

				if(calTypesDialog.exRGB_checked())
					kcfFile.setExtrinsicRGBParameters(*extrinsicRGBCalibRes);

				kcfFile.saveTo(dialog.get_filename().c_str());
				hasUnsavedCalibration = false;
			}
			catch(std::exception& e) {
				errorMessageBox(e.what());
			}
		}
	}

	void Application::onChangeCountdownDurationSpinButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION))
			((operations::Calibration*)currentOperation)->setCountDownDuration(mainWindow->countdownDurationSpinButton->get_value() * 1000);

		currentOperation_mutex.unlock();
	}

	void Application::onChangeSnapshotFreezeDurationSpinButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION))
			((operations::Calibration*)currentOperation)->setFreezeDuration(mainWindow->snapshotFreezeDurationSpinButton->get_value());

		currentOperation_mutex.unlock();
	}

	void Application::onClickClearCalibrationDataButton() {
		Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : your calibration results have not been saved yet !\n If you clear it now, all calibration data will be lost", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
		confirmPopup.set_secondary_text("Are you sure you want to clear and lose unsaved data ?");

		if(!hasUnsavedCalibration || (confirmPopup.run() == Gtk::RESPONSE_OK)) {
			widgets::CalibrationTypesChoiceDialog calTypesDialog(
				widgets::CALTYPECHOICE_DIALOG_CLEAR,
				(intrinsicIRCalibRes != NULL),
				(intrinsicRGBCalibRes != NULL),
				(extrinsicIRCalibRes != NULL),
				(extrinsicRGBCalibRes != NULL),
				(intrinsicIRCalibRes != NULL),
				(intrinsicRGBCalibRes != NULL),
				(extrinsicIRCalibRes != NULL),
				(extrinsicRGBCalibRes != NULL));

			if(calTypesDialog.run() == Gtk::RESPONSE_OK)
			{
				if(calTypesDialog.inIR_checked())
				{
					if(intrinsicIRCalibRes != NULL)
					{
						delete intrinsicIRCalibRes;
						intrinsicIRCalibRes = NULL;
					}

					mainWindow->intrinsicIRResultCell->removeResult();

					if(currentLoadedSequence != NULL)
						currentLoadedSequence->removeIntrinsicIRCalibrationParameters();
				}

				if(calTypesDialog.inRGB_checked())
				{
					if(intrinsicRGBCalibRes != NULL)
					{
						delete intrinsicRGBCalibRes;
						intrinsicRGBCalibRes = NULL;
					}

					mainWindow->intrinsicRGBResultCell->removeResult();

					if(currentLoadedSequence != NULL)
						currentLoadedSequence->removeIntrinsicRGBCalibrationParameters();
				}

				if(calTypesDialog.exIR_checked())
				{
					if(extrinsicIRCalibRes != NULL)
					{
						delete extrinsicIRCalibRes;
						extrinsicIRCalibRes = NULL;
					}

					mainWindow->extrinsicIRResultCell->removeResult();

					if(currentLoadedSequence != NULL)
						currentLoadedSequence->removeExtrinsicIRCalibrationParameters();
				}

				if(calTypesDialog.exRGB_checked())
				{
					if(extrinsicRGBCalibRes != NULL)
					{
						delete extrinsicRGBCalibRes;
						extrinsicRGBCalibRes = NULL;
					}

					mainWindow->extrinsicRGBResultCell->removeResult();

					if(currentLoadedSequence != NULL)
						currentLoadedSequence->removeExtrinsicRGBCalibrationParameters();
				}

				updateSaveButton();
			}
		}
	}

	void Application::onClickNatNetConnectButton() {
		if(natNetClient != NULL) {
			disconnectNatNetClient();
			mainWindow->mocapMarkersListView->clear_items();
		}
		else {
			mainWindow->mocapMarkersListView->clear_items();
			connectNatNetClient();
		}
	}

	void Application::startRecording() {
		addWaitMessage("Preparing temp folder for recording ...", &preparingRecordMsgLabel);
	
		try {
			if(currentLoadedSequence != NULL)
				delete currentLoadedSequence;

			currentLoadedSequence = new datalib::Sequence();
			boost::filesystem::path sequenceTempFolder = getNewSequenceTempFolder();
			currentLoadedSequence->setRootDirectory(sequenceTempFolder.string().c_str());

			if(intrinsicIRCalibRes != NULL)
				currentLoadedSequence->setIntrinsicIRCalibrationParameters(*intrinsicIRCalibRes);

			if(intrinsicRGBCalibRes != NULL)
				currentLoadedSequence->setIntrinsicRGBCalibrationParameters(*intrinsicRGBCalibRes);

			if(extrinsicIRCalibRes != NULL)
				currentLoadedSequence->setExtrinsicIRCalibrationParameters(*extrinsicIRCalibRes);

			if(extrinsicRGBCalibRes != NULL)
				currentLoadedSequence->setExtrinsicRGBCalibrationParameters(*extrinsicRGBCalibRes);

			operations::SequenceRecording* newRecordingOperation = new operations::SequenceRecording(currentLoadedSequence);
			newRecordingOperation->onColorImageFrameOutput = onCurrentOperationColorImageFrameOutput;
			newRecordingOperation->onIRImageFrameOutput = onCurrentOperationIRImageFrameOutput;
			newRecordingOperation->onDepthFrameOutput = onCurrentOperationDepthFrameOutput;
			newRecordingOperation->onMarkersFrameOutput = onCurrentOperationMarkerFrameOutput;
			setCurrentOperation(newRecordingOperation);
			newRecordingOperation->startRecording();
			mainWindow->monitor->enterRecordingMode();
			mainWindow->stopButton->set_sensitive(true); // @TODO: deplacer dans MainWindow avec dispatcher
		}
		catch(std::exception& e) {
			errorMessageBox(e.what());
		}

		removeWaitMessage(&preparingRecordMsgLabel);
	}

	void Application::onClickRecordButton() {
		mainWindow->recordButton->set_sensitive(false); // @TODO: deplacer dans MainWindow avec dispatcher
		mainWindow->openSequenceButton->set_sensitive(false); // @TODO: deplacer dans MainWindow avec dispatcher
		std::thread startRecordingThread(Application::startRecording);
		startRecordingThread.detach();
	}

	void Application::onClickStartPlaybackButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) {
			mainWindow->set_sequence_is_playing(true);
			((operations::SequenceReading*)currentOperation)->startPlaying();
		}

		currentOperation_mutex.unlock();
	}

	void Application::onCurrentOperationChangePlayheadPosition(unsigned long long position) {
		mainWindow->set_playhead_position(position);
	}

	void Application::onCurrentOperationUpdateBufferEndingPoint(unsigned long long endingPoint) {
		mainWindow->setBufferEndingPoint(endingPoint);
	}

	void Application::onCurrentOperationStopAtTheEnd() {
		mainWindow->set_sequence_is_playing(false);
	}
	
	/**
	 * @throws EmptySequenceException
	 */
	void Application::readCurrentLoadedSequence() {
		operations::SequenceReading* newReadingOperation = new operations::SequenceReading(currentLoadedSequence);
		disconnectNatNetClient();
		mainWindow->disableAllWidgets();
		setCurrentOperation(newReadingOperation);
		newReadingOperation->onColorImageFrameOutput = onCurrentOperationColorImageFrameOutput;
		newReadingOperation->onIRImageFrameOutput = onCurrentOperationIRImageFrameOutput;
		newReadingOperation->onDepthFrameOutput = onCurrentOperationDepthFrameOutput;
		newReadingOperation->onMarkersFrameOutput = onCurrentOperationMarkerFrameOutput;
		newReadingOperation->onChangePlayheadPosition = onCurrentOperationChangePlayheadPosition;
		newReadingOperation->onUpdateBufferEndingPoint = onCurrentOperationUpdateBufferEndingPoint;
		newReadingOperation->onStopAtTheEnd = onCurrentOperationStopAtTheEnd;
		mainWindow->enableSequenceReadingWidgets(currentLoadedSequence->getDuration());
		newReadingOperation->setPlayHeadPosition(0);
		updateSaveButton();
		setMonitoredKinectStream(KINECT_STREAM_TYPE_RGB);
	}

	void Application::onSequenceLoaded(datalib::Sequence* sequence) {
		if(currentLoadedSequence != NULL)
			delete currentLoadedSequence;

		currentLoadedSequence = sequence;

		datalib::IntrinsicCalibrationParametersSet* intrIRParams = sequence->getIntrinsicIRCalibrationParameters();

		if(intrIRParams != NULL)
			setIntrinsicIRCalibrationParameters(*intrIRParams);
		else {
			if(intrinsicIRCalibRes != NULL) {
				delete intrinsicIRCalibRes;
				intrinsicIRCalibRes = NULL;
			}

			mainWindow->intrinsicIRResultCell->removeResult();
		}

		datalib::IntrinsicCalibrationParametersSet* intrRGBParams = sequence->getIntrinsicRGBCalibrationParameters();

		if(intrRGBParams != NULL)
			setIntrinsicRGBCalibrationParameters(*intrRGBParams);
		else {
			if(intrinsicRGBCalibRes != NULL) {
				delete intrinsicRGBCalibRes;
				intrinsicRGBCalibRes = NULL;
			}

			mainWindow->intrinsicRGBResultCell->removeResult();
		}

		datalib::ExtrinsicCalibrationParametersSet* extrIRParams = sequence->getExtrinsicIRCalibrationParameters();

		if(extrIRParams != NULL)
			setExtrinsicIRCalibrationParameters(*extrIRParams);
		else {
			if(extrinsicIRCalibRes != NULL) {
				delete extrinsicIRCalibRes;
				extrinsicIRCalibRes = NULL;
			}

			mainWindow->extrinsicIRResultCell->removeResult();
		}

		datalib::ExtrinsicCalibrationParametersSet* extrRGBParams = sequence->getExtrinsicRGBCalibrationParameters();

		if(extrRGBParams != NULL)
			setExtrinsicRGBCalibrationParameters(*extrRGBParams);
		else {
			if(extrinsicRGBCalibRes != NULL) {
				delete extrinsicRGBCalibRes;
				extrinsicRGBCalibRes = NULL;
			}

			mainWindow->extrinsicRGBResultCell->removeResult();
		}

		updateSaveButton();
		readCurrentLoadedSequence();
	}

	void Application::onUpdateProgress(float progress) {
		mainWindow->waitMsgPopup->setProgress(progress);
	}

	void Application::openUnzippedSequenceFromDirectory(boost::filesystem::path sequenceRootDirectoryPath) {
		addWaitMessage("Loading sequence data ...", &loadingSequenceDataMsgLabel);
		datalib::TaskProgress* loadProgress = new datalib::TaskProgress();
		loadProgress->onSetProgress = onUpdateProgress;

		try {
			datalib::Sequence* newSequence = new datalib::Sequence();
			newSequence->setRootDirectory(sequenceRootDirectoryPath.string().c_str());
			newSequence->readDataFromRootDirectory(loadProgress);
			mainWindow->waitMsgPopup->hideProgress();
			onSequenceLoaded(newSequence);
		}
		catch(std::exception& e) {
			mainWindow->waitMsgPopup->hideProgress();
			std::ostringstream errorMessage;
			errorMessage << "Error trying to open sequence : " << e.what() << std::endl;
			errorMessageBox(errorMessage.str());
			activateMonitoring();
		}

		delete loadProgress;
 		removeWaitMessage(&loadingSequenceDataMsgLabel);
	}

	void Application::unZipSequence(std::string ksaFileName) {
		addWaitMessage("Unarchiving sequence files to temp folder ...", &unzippingSequenceFileMsgLabel);
		
		try {
			boost::filesystem::path sequenceTempFolder = getNewSequenceTempFolder();

			disconnectNatNetClient();
			mainWindow->mocapMarkersListView->clear_items();

			try {
				disableCurrentOperationDisplay();
			}
			catch (KinectCallbackAssignmentException& e) {
				errorMessageBox(e.what());
			}

			try {
				datalib::SequenceFile sequenceFile(ksaFileName.c_str());
				datalib::TaskProgress* unzipProgress = new datalib::TaskProgress();
				unzipProgress->onSetProgress = onUpdateProgress;

				int unzipThreadsCount = 4; // @TODO : definir comme option globale (parametrable) plutot qu'en dur

				if (sequenceFile.unZipSequenceToTempPath(sequenceTempFolder.string().c_str(), unzipProgress, unzipThreadsCount)) {
					mainWindow->waitMsgPopup->hideProgress();
					removeWaitMessage(&unzippingSequenceFileMsgLabel);
					openUnzippedSequenceFromDirectory(sequenceTempFolder);
				}
				else {
					mainWindow->waitMsgPopup->hideProgress();
					removeWaitMessage(&unzippingSequenceFileMsgLabel);
					errorMessageBox("Error trying to unarchive sequence file to temp folder");
				}

				delete unzipProgress;
			}
			catch (std::exception& e) {
				removeWaitMessage(&unzippingSequenceFileMsgLabel);
				errorMessageBox(e.what());
			}
		}
		catch (boost::filesystem::filesystem_error& bfse) {
			removeWaitMessage(&unzippingSequenceFileMsgLabel);
			errorMessageBox(bfse.what());
		}
	}

	void Application::onClickOpenSequenceButton() {
		Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : you have unsaved data (calibration and/or sequence) !\n If you open a sequence file now, all unsaved data will be lost", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
		confirmPopup.set_secondary_text("Are you sure you want to open a sequence and override unsaved data ?");

		if(!(hasUnsavedCalibration || hasUnsavedSequence) || (confirmPopup.run() == Gtk::RESPONSE_OK)) {
			Gtk::FileChooserDialog fileDialog("Please choose a kinect sequence archive to open", Gtk::FILE_CHOOSER_ACTION_OPEN);
			fileDialog.set_transient_for(*mainWindow);

			//Add response buttons to the dialog:
			fileDialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
			fileDialog.add_button("Select", Gtk::RESPONSE_OK);

			Glib::RefPtr<Gtk::FileFilter> filter_ksa = Gtk::FileFilter::create();
			filter_ksa->set_name("Kinect sequence archive");
			filter_ksa->add_pattern("*.ksa");
			fileDialog.add_filter(filter_ksa);

			if(fileDialog.run() == Gtk::RESPONSE_OK)
			{
				std::string ksaFileName = fileDialog.get_filename();
				fileDialog.hide();
				kinect.stop();
				disableCurrentOperationDisplay();
				std::thread unZipSequenceThread(Application::unZipSequence, ksaFileName);
				unZipSequenceThread.detach();
			}
		}
	}

	void Application::onClickCloseSequenceButton() {
		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) {
			Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : current sequence has not been saved yet !\n If you close it now, all it's data will be lost", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
			confirmPopup.set_secondary_text("Are you sure you want to close and lose unsaved data ?");

			if (!hasUnsavedSequence || (confirmPopup.run() == Gtk::RESPONSE_OK)) {
				((operations::SequenceReading*)currentOperation)->stop();
				delete (operations::SequenceReading*)currentOperation;
				currentOperation = NULL;
				currentOperation_mutex.unlock();

				mainWindow->disableAllWidgets();
				disableCurrentOperationDisplay();
				mainWindow->mocapMarkersListView->clear_items();

				if (currentLoadedSequence != NULL) {
					boost::filesystem::path sequenceTempFolder = currentLoadedSequence->getRootDirectory();

					delete currentLoadedSequence;
					currentLoadedSequence = NULL;

					try {
						boost::filesystem::remove_all(sequenceTempFolder);
					}
					catch (std::exception& e) {
						errorMessageBox(e.what());
					}
				}

				hasUnsavedSequence = false;
				activateMonitoring();
			}
			else
				currentOperation_mutex.unlock();
		}
		else
			currentOperation_mutex.unlock();
	}

	void Application::stopRecording() {
		addWaitMessage("Finishing writing sequence data to disk ...", &writingSequenceDataMsgLabel);
		currentOperation_mutex.lock();

		try {
			((operations::SequenceRecording*)currentOperation)->stop();
		}
		catch(std::exception& e) {
			errorMessageBox(e.what());
		}

		mainWindow->monitor->exitRecordingMode();
		hasUnsavedSequence = true;
		currentOperation_mutex.unlock();
		mainWindow->stopButton->set_sensitive(false);// @TODO : deplacer dans mainwindow avec dispatcher

		try {
			disableCurrentOperationDisplay();
		}
		catch(std::exception& e) {
			errorMessageBox(e.what());
		}

		std::thread readCurrentLoadedSequenceThread(Application::readCurrentLoadedSequence);
		readCurrentLoadedSequenceThread.detach();
		removeWaitMessage(&writingSequenceDataMsgLabel);
	}

	void Application::onClickStopButton() {
		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) {
			((operations::SequenceReading*)currentOperation)->stop();
			currentOperation_mutex.unlock();
			mainWindow->set_sequence_is_playing(false);
		}
		else if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_RECORDING_OPERATION)) {
			currentOperation_mutex.unlock();
			std::thread stopRecordingThread(Application::stopRecording);
			stopRecordingThread.detach();
		}
		else
			currentOperation_mutex.unlock();
	}

	void Application::onClickGoToBeginingButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION) && !((operations::SequenceReading*)currentOperation)->isPlaying())
			((operations::SequenceReading*)currentOperation)->goToBegining();

		currentOperation_mutex.unlock();
	}

	void Application::onClickGoToEndButton() {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION) && !((operations::SequenceReading*)currentOperation)->isPlaying())
			((operations::SequenceReading*)currentOperation)->goToEnd();

		currentOperation_mutex.unlock();
	}

	bool Application::onMoveVideoTimeSliderScale(Gtk::ScrollType scroll, double new_value) {
		currentOperation_mutex.lock();

		if((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION))
			((operations::SequenceReading*)currentOperation)->setPlayHeadPosition(new_value);
		
		currentOperation_mutex.unlock();
		return true;
	}

	void Application::onClickPreviousButton() {
		currentOperation_mutex.lock();

		if(((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) && !((operations::SequenceReading*)currentOperation)->isPlaying()) {
			try {
				((operations::SequenceReading*)currentOperation)->goToPreviousFrame();
			}
			catch(NoPreviousEventException& e) {
				// we don't do anything, we simply don't go to previous frame as ther is none
			}
		}

		currentOperation_mutex.unlock();
	}

	void Application::onClickNextButton() {
		currentOperation_mutex.lock();

		if(((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) && !((operations::SequenceReading*)currentOperation)->isPlaying()) {
			try {
				((operations::SequenceReading*)currentOperation)->goToNextFrame();
			}
			catch(NoNextEventException& e) {
				// we don't do anything, we simply don't go to next frame as there is none
			}
		}

		currentOperation_mutex.unlock();
	}

	bool Application::openKoccaFile(const char* filePath) {
		boost::filesystem::path openFilePath = filePath;
		std::string fileExt = openFilePath.extension().string();
		std::transform(fileExt.begin(),fileExt.end(),fileExt.begin(), ::toupper);
		
		if(fileExt == ".KCF") {
			try {
				loadCalibrationFile((filePath), true);
			}
			catch(std::exception& e) {
				errorMessageBox(e.what());
			}

			return false;
		}
		else if(fileExt == ".KSA") {
			std::thread unZipSequenceThread(Application::unZipSequence, std::string(filePath));
			unZipSequenceThread.detach();

			// switch tabs to the "capture" page
			mainWindow->tabs->set_current_page(1);
			return true;
		}
		else {
			errorMessageBox("Error: unknown file type.");
			return false;
		}
	}

	bool Application::folderContainsSequenceData(boost::filesystem::path folderPath) {
		if(boost::filesystem::exists(folderPath) && boost::filesystem::is_directory(folderPath)) {
			boost::filesystem::path calibrationFilePath = folderPath / "kinect_calibration_parameters.kcf";

			if(boost::filesystem::exists(calibrationFilePath) && boost::filesystem::is_regular_file(calibrationFilePath))
				return true;

			// read markers data
			boost::filesystem::path markersDataFilePath = folderPath / "markersData.csv";

			if(boost::filesystem::exists(markersDataFilePath) && boost::filesystem::is_regular_file(markersDataFilePath))
				return true;
			// ---

			boost::filesystem::path imageFramesDirectory = folderPath / "image";
			boost::filesystem::path depthFramesDirectory = folderPath / "depth";
			boost::filesystem::directory_iterator end;
		
			// browse "image" directory and index it's files
			if(boost::filesystem::exists(imageFramesDirectory) && boost::filesystem::is_directory(imageFramesDirectory))
				for(boost::filesystem::directory_iterator i(imageFramesDirectory); i != end; ++i)
					if(boost::filesystem::is_regular_file(i->path()))
						return true;

			// browse "depth" directory and index it's files
			if(boost::filesystem::exists(depthFramesDirectory) && boost::filesystem::is_directory(depthFramesDirectory))
				for(boost::filesystem::directory_iterator i(depthFramesDirectory); i != end; ++i)
					if(boost::filesystem::is_regular_file(i->path()))
						return true;
		}
		else
			return false;
	}

	boost::filesystem::path Application::getFirstTempFolderWithRecoverableSequenceData() {
		if(boost::filesystem::exists(baseTempFolder) && boost::filesystem::is_directory(baseTempFolder)) {
			boost::filesystem::directory_iterator end;
			std::vector<boost::filesystem::path> tempChilds;

			for(boost::filesystem::directory_iterator i(baseTempFolder); i != end; ++i) {
				if(!pidIsARunningKoccaInstance(getPIDFromTempFolderPath(i->path())) && folderContainsSequenceData(i->path()))
					return i->path();
			}
		}

		throw TempFolderNotAvailableException("No sequence temp folder found in");
	}

	std::vector<unsigned long> Application::getAllRunningKoccaPIDs() {
		std::vector<unsigned long> koccaPIDs;
		unsigned long allPIDs[1024];
		unsigned long allPIDsSize;

		if(EnumProcesses(allPIDs, sizeof(allPIDs), &allPIDsSize)) {
			unsigned long allPIDsCount = allPIDsSize / sizeof(unsigned long);

			for(int i = 0; i < allPIDsCount; i++) {
				unsigned long pid = allPIDs[i];

				if(pid != 0) {
					void* processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, pid);

					if(processHandle != NULL) {
						HMODULE processModules;
						unsigned long processModulesSize;

						if(EnumProcessModules(processHandle, &processModules, sizeof(processModules), &processModulesSize)) {
							char processName[MAX_PATH];

							GetModuleBaseName(processHandle, processModules, processName, sizeof(processName)/sizeof(char));

							if(std::string(processName) == "kocca.exe")
								koccaPIDs.push_back(pid);
						}
					}
				}
			}
		}

		return koccaPIDs;
	}

	bool Application::pidIsARunningKoccaInstance(unsigned long pid) {
		bool pidFound = false;
		std::vector<unsigned long> koccaPids = getAllRunningKoccaPIDs();

		for(int i = 0; (i < koccaPids.size()) && !pidFound; i++)
			pidFound = (koccaPids.at(i) == pid);

		return pidFound;
	}

	unsigned long Application::getPIDFromTempFolderPath(boost::filesystem::path folderPath) {
		unsigned long pid = 0;

		std::string folderName = folderPath.stem().string();
		std::istringstream folderNameSS(folderName);

		std::string PidStr;
		std::getline(folderNameSS, PidStr, '_');

		std::istringstream PidSS(PidStr);
		PidSS >> pid;

		return pid;
	}

	bool Application::baseTempFolderContainsRecoverableSequenceData() {
		if(boost::filesystem::exists(baseTempFolder) && boost::filesystem::is_directory(baseTempFolder)) {
			boost::filesystem::directory_iterator end;
			std::vector<boost::filesystem::path> tempChilds;

			for(boost::filesystem::directory_iterator i(baseTempFolder); i != end; ++i) {
				if(!pidIsARunningKoccaInstance(getPIDFromTempFolderPath(i->path())) && folderContainsSequenceData(i->path()))
					return true;
			}
		}
		
		return false;
	}

	void Application::clearTempFolder() {
		if(boost::filesystem::exists(baseTempFolder)) {
			if(boost::filesystem::is_directory(baseTempFolder)) {
				boost::filesystem::directory_iterator end;
				std::vector<boost::filesystem::path> tempChilds;

				for(boost::filesystem::directory_iterator i(baseTempFolder); i != end; ++i)
					boost::filesystem::remove_all(i->path());
			}
			else
				throw TempFolderNotAvailableException("path is not a valid directory");
		}
		else
			throw TempFolderNotAvailableException("path does not exists");
	}

	void Application::onKinectColorImageThread(cv::Mat* pFrame) {
		unsigned long long time = getMSTime();

		// adjust image brightness and contrats
		double brightnessScaleValue = mainWindow->rgbBrightnessScale->get_value();
		double brightnessAdjustment = brightnessScaleValue * 2.55;

		double contrastScaleValue = mainWindow->rgbContrastScale->get_value();
		double contrastAdjustment = ((contrastScaleValue + 100.0) / 100.0) * ((contrastScaleValue + 100.0) / 100.0);

		if ((brightnessAdjustment != 0.0) || (contrastAdjustment != 1.0))
			pFrame->convertTo(*pFrame, -1, contrastAdjustment, brightnessAdjustment);
		// ---

		datalib::TimeCodedFrame tcFrame;
		tcFrame.frame = *pFrame;
		tcFrame.time = time;

		try {
			if (currentOperation != NULL);
				currentOperation->processColorImageFrame(tcFrame);
		}
		catch (std::runtime_error& re) {
			std::string errorMessageStr;

			if (currentOperation->type == kocca::operations::KOCCA_RECORDING_OPERATION) {
				onClickStopButton();
				std::ostringstream errorMessage;
				errorMessage << "Recording failed !\rAn error occured during recording operation : " << std::endl << re.what() << std::endl;
				errorMessageStr = errorMessage.str();
			}
			else
				errorMessageStr = re.what();

			errorMessageBox(errorMessageStr);
		}

		pFrame->release();
		delete pFrame;
	}

	void Application::onKinectIRImageThread(cv::Mat* pFrame) {
		unsigned long long time = getMSTime();

		// adjust image brightness and contrast
		double brightnessScaleValue = mainWindow->irBrightnessScale->get_value();
		double brightnessAdjustment = brightnessScaleValue * 655.35;

		double contrastScaleValue = mainWindow->irContrastScale->get_value();
		double contrastAdjustment = ((contrastScaleValue + 100.0) / 100.0) * ((contrastScaleValue + 100.0) / 100.0);

		if ((brightnessAdjustment != 0.0) || (contrastAdjustment != 1.0))
			pFrame->convertTo(*pFrame, -1, contrastAdjustment, brightnessAdjustment);
		// ---

		datalib::TimeCodedFrame tcFrame;
		tcFrame.frame = *pFrame;
		tcFrame.time = time;

		try {
			if (currentOperation != NULL);
				currentOperation->processIRImageFrame(tcFrame);
		}
		catch (std::runtime_error& re) {
			std::string errorMessageStr;

			if (currentOperation->type == kocca::operations::KOCCA_RECORDING_OPERATION) {
				onClickStopButton();
				std::ostringstream errorMessage;
				errorMessage << "Recording failed !\rAn error occured during recording operation : " << std::endl << re.what() << std::endl;
				errorMessageStr = errorMessage.str();
			}
			else
				errorMessageStr = re.what();

			errorMessageBox(errorMessageStr);
		}

		pFrame->release();
		delete pFrame;
	}

	void Application::onKinectDepthImageThread(cv::Mat* pFrame) {
		unsigned long long time = getMSTime();

		datalib::TimeCodedFrame tcFrame;
		tcFrame.frame = *pFrame;
		tcFrame.time = time;

		try {
			if (currentOperation != NULL);
				currentOperation->processDepthFrame(tcFrame);
		}
		catch (std::runtime_error& re) {
			std::string errorMessageStr;

			if (currentOperation->type == kocca::operations::KOCCA_RECORDING_OPERATION) {
				onClickStopButton();
				std::ostringstream errorMessage;
				errorMessage << "Recording failed !\rAn error occured during recording operation : " << std::endl << re.what() << std::endl;
				errorMessageStr = errorMessage.str();
			}
			else
				errorMessageStr = re.what();

			errorMessageBox(errorMessageStr);
		}

		pFrame->release();
		delete pFrame;
	}

	void  Application::onKinectColorImage(cv::Mat frame) {
		new std::thread(&Application::onKinectColorImageThread, new cv::Mat(frame));
	}

	void Application::onKinectIRImage(cv::Mat frame) {
		new std::thread(&Application::onKinectIRImageThread, new cv::Mat(frame));
	}

	void  Application::onKinectDepthImage(cv::Mat frame) {
		new std::thread(&Application::onKinectDepthImageThread, new cv::Mat(frame));
	}

	bool Application::onShowMainWindow(_GdkEventAny* event) {
		try {
			if(baseTempFolderContainsRecoverableSequenceData()) {
				Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING: Your temp folder contains some recoverable sequence data !", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
				confirmPopup.set_secondary_text("KOCCA may have crashed last time it was used before this sequenced was saved.\nDo you want to recover sequence now ?\n(If you cancel recovering, this data will be erased)");

				if(confirmPopup.run() == Gtk::RESPONSE_OK) {
					confirmPopup.hide();
					boost::filesystem::path oldSequenceTempFolderPath = getFirstTempFolderWithRecoverableSequenceData();

					// rename sequence's temp folder to attach it to current process
					std::ostringstream newFolderName;
					newFolderName << getProcessID() << "_" << getMSTime();
					boost::filesystem::path newSequenceTempFolderPath = baseTempFolder / newFolderName.str();
					boost::filesystem::rename(oldSequenceTempFolderPath, newSequenceTempFolderPath);

					std::thread openUnzippedSequenceFromDirectoryThread(Application::openUnzippedSequenceFromDirectory, newSequenceTempFolderPath);
					openUnzippedSequenceFromDirectoryThread.detach();

					hasUnsavedSequence = true;
					mainWindow->tabs->set_current_page(1);
					return true;
				}
				else
					clearTempFolder();
			}

			kinect.onRGBFrame = &Application::onKinectColorImage;
			kinect.onIRFrame = &Application::onKinectIRImage;
			kinect.onDepthFrame = &Application::onKinectDepthImage;

			if(!openFileAtStartup.empty()) {
				if(!openKoccaFile(openFileAtStartup.c_str()))
					activateMonitoring();
			}
			else
				activateMonitoring();

			return true;
		}
		catch(std::exception& e) {
			errorMessageBox(e.what());
			return false;
		}
	}

	void Application::on_mocapMarkersListView_selected_markers_changed(std::vector<std::string> selectedMarkersIDs) {
		mainWindow->monitor->setSelectedMarkersIds(selectedMarkersIDs);
	}

	void Application::on_monitor_selected_markers_changed(std::vector<std::string> selectedMarkersIDs) {
		mainWindow->monitor->setSelectedMarkersIds(selectedMarkersIDs);
		mainWindow->mocapMarkersListView->setSelectedMarkersIds(selectedMarkersIDs);
	}

	void Application::exportSequence(std::string destFilePath) {
		addWaitMessage("Exporting KOCCA sequence archive ...", &exportingSequenceFileMsgLabel);

		try {
			datalib::SequenceFile exportFile(destFilePath.c_str());
			exportFile.exportSequence(currentLoadedSequence);
			removeWaitMessage(&exportingSequenceFileMsgLabel);
			infoMessageBox("Sequence succesfully exported.");
			hasUnsavedSequence = false;
			hasUnsavedCalibration = false;
		}
		catch(std::exception& e) {
			removeWaitMessage(&exportingSequenceFileMsgLabel);
			errorMessageBox(e.what());
		}
	}

	void Application::onClickExportSequenceButton() {
		if(currentLoadedSequence != NULL) {
			bool seqHasCalibData = currentLoadedSequence->hasCalibrationData();
			
			Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : your sequence does not have any calibration data.", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
			confirmPopup.set_secondary_text("Are you sure you want to export it anyway ?");

			if(seqHasCalibData || (confirmPopup.run() == Gtk::RESPONSE_OK)) {
				confirmPopup.hide();

				Gtk::FileChooserDialog dialog("Please indicate where to save sequence", Gtk::FILE_CHOOSER_ACTION_SAVE);
				dialog.set_transient_for(*mainWindow);

				//Add response buttons the the dialog:
				dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
				dialog.add_button("Select", Gtk::RESPONSE_OK);

				Glib::RefPtr<Gtk::FileFilter> filter_ksa = Gtk::FileFilter::create();
				filter_ksa->set_name("KOCCA sequence archive");
				filter_ksa->add_pattern("*.ksa");
				dialog.add_filter(filter_ksa);

				if(dialog.run() == Gtk::RESPONSE_OK) {
					std::string destFilePath(dialog.get_filename().c_str());
					std::thread exportSequenceThread(Application::exportSequence, destFilePath);
					exportSequenceThread.detach();
				}
			}
		}
	}

	void Application::onMainWindowDragDataReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time) {
		if((selection_data.get_length() >= 0) && (selection_data.get_format() == 8)) {
			std::vector<Glib::ustring> file_list;
			file_list = selection_data.get_uris();

			if(file_list.size() > 0) {
				if(file_list.size() == 1) {
					Glib::ustring path = Glib::filename_from_uri(file_list[0]);
					openKoccaFile(path.c_str());
				}
				else
					errorMessageBox("Error: cannot open multiple files at the same time.");
			}
		}

		context->drag_finish(false, false, time);
	}

	void Application::onMainWindowSpaceShortcut() {
		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION)) {
			if (((operations::SequenceReading*)currentOperation)->isPlaying()) {
				currentOperation_mutex.unlock();
				onClickStopButton();
			}
			else {
				currentOperation_mutex.unlock();
				onClickStartPlaybackButton();
			}
		}
		else if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_CALIBRATION_OPERATION)) {
			currentOperation_mutex.unlock();
			onClickPauseCalibrationButton();
		}
		else
			currentOperation_mutex.unlock();
	}

	bool Application::onCloseMainWindow(GdkEventAny* event) {
		if(hasUnsavedSequence || hasUnsavedCalibration) {
			Gtk::MessageDialog confirmPopup(*mainWindow, "WARNING : you have unsaved data (calibration and/or sequence) !\n If you quit KOCCA now, all unsaved data will be lost", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
			confirmPopup.set_secondary_text("Are you sure you want to quit and lose unsaved data ?");

			if(confirmPopup.run() == Gtk::RESPONSE_OK) {
				if(hasUnsavedSequence && (currentLoadedSequence != NULL)) {
					try {
						boost::filesystem::remove_all(currentLoadedSequence->getRootDirectory());
					}
					catch(std::exception& e) {
						errorMessageBox(e.what());
						return true;
					}
				}

				return false;
			}
			else
				return true;
		}
		else
			return false;
	}

	unsigned long Application::getProcessID() {
		return GetCurrentProcessId();
	}

	boost::filesystem::path Application::getNewSequenceTempFolder() {
		boost::filesystem::path newFolderPath;
		bool created = false;

		while(!created) {
			std::ostringstream newFolderName;
			newFolderName << getProcessID() << "_" << getMSTime();
			newFolderPath = baseTempFolder / newFolderName.str();

			if(!boost::filesystem::exists(newFolderPath)) {
				if(boost::filesystem::create_directory(newFolderPath))
					created = true;
				else {
					std::ostringstream errorMessage;
					errorMessage << baseTempFolder.string() << " does not seems to be a writeable directory.";
					throw new TempFolderNotAvailableException(errorMessage.str().c_str());
				}
			}
		}

		return newFolderPath;
	}

	void Application::setMonitoredKinectStream(kinectStreamType newMonitoredStream) {
		monitoredKinectStream = newMonitoredStream;

		if (monitoredKinectStream == KINECT_STREAM_TYPE_RGB) {
			mainWindow->kinectRGBStreamThumbnail->set_selected(true);
			mainWindow->kinectInfraredStreamThumbnail->set_selected(false);
			mainWindow->kinectDepthStreamThumbnail->set_selected(false);
			mainWindow->RGBAdjustmentsVBox->set_visible(true);
			mainWindow->infraredAdjustmentsVBox->set_visible(false);
			mainWindow->monitor->setIntrinsicCalibrationParams(intrinsicRGBCalibRes);
			mainWindow->monitor->setExtrinsicCalibrationParams(extrinsicRGBCalibRes);
		}
		else if (monitoredKinectStream == KINECT_STREAM_TYPE_INFRARED) {
			mainWindow->kinectRGBStreamThumbnail->set_selected(false);
			mainWindow->kinectInfraredStreamThumbnail->set_selected(true);
			mainWindow->kinectDepthStreamThumbnail->set_selected(false);
			mainWindow->RGBAdjustmentsVBox->set_visible(false);
			mainWindow->infraredAdjustmentsVBox->set_visible(true);
			mainWindow->monitor->setIntrinsicCalibrationParams(intrinsicIRCalibRes);
			mainWindow->monitor->setExtrinsicCalibrationParams(extrinsicIRCalibRes);
		}
		else if (monitoredKinectStream == KINECT_STREAM_TYPE_DEPTH) {
			mainWindow->kinectRGBStreamThumbnail->set_selected(false);
			mainWindow->kinectInfraredStreamThumbnail->set_selected(false);
			mainWindow->kinectDepthStreamThumbnail->set_selected(true);
			mainWindow->RGBAdjustmentsVBox->set_visible(false);
			mainWindow->infraredAdjustmentsVBox->set_visible(false);
			mainWindow->monitor->setIntrinsicCalibrationParams(intrinsicIRCalibRes);
			mainWindow->monitor->setExtrinsicCalibrationParams(extrinsicIRCalibRes);
		}

		currentOperation_mutex.lock();

		if ((currentOperation != NULL) && (currentOperation->type == operations::KOCCA_READING_OPERATION) && !((operations::SequenceReading*)currentOperation)->isPlaying()) {
			mainWindow->monitor->invalidateLastFrame();

			if (monitoredKinectStream == KINECT_STREAM_TYPE_RGB)
				((operations::SequenceReading*)currentOperation)->outputImageFrame(true);
			else if (monitoredKinectStream == KINECT_STREAM_TYPE_INFRARED)
				((operations::SequenceReading*)currentOperation)->outputIRFrame(true);
			else if (monitoredKinectStream == KINECT_STREAM_TYPE_DEPTH)
				((operations::SequenceReading*)currentOperation)->outputDepthFrame(true);

			((operations::SequenceReading*)currentOperation)->outputMarkersFrame();
		}

		currentOperation_mutex.unlock();
	}

	void Application::onSelectKinectRGBStreamThumbnail() {
		setMonitoredKinectStream(KINECT_STREAM_TYPE_RGB);
	}

	void Application::onSelectKinectInfraredStreamThumbnail() {
		setMonitoredKinectStream(KINECT_STREAM_TYPE_INFRARED);
	}

	void Application::onSelectKinectDepthStreamThumbnail() {
		setMonitoredKinectStream(KINECT_STREAM_TYPE_DEPTH);
	}
} // namespace kocca
