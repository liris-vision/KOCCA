#ifndef KOCCA_APPLICATION_H
#define KOCCA_APPLICATION_H

#include <Windows.h>

#include <vector>
#include <mutex>
#include <atomic>
#include <opencv2/opencv.hpp>
#include <gtkmm/main.h>
#include "NatNetClient.h"

#include "KinectV2Sensor.h"
#include "widgets/MainWindow.h"
#include "datalib/IntrinsicCalibrationParametersSet.h"
#include "datalib/ExtrinsicCalibrationParametersSet.h"
#include "datalib/Sequence.h"
#include "operations/Operation.h"
#include "datalib/MocapMarkerFrame.h"

#include "boost/filesystem.hpp"

/**
 * kocca namespace contains all types defined for KOCCA
 */
namespace kocca {

	/**
	 * Set of parameters for \fn static gboolean Application::displayWaitMessage(gpointer data)
	 */
	struct waitMessageParameters {
		std::string* pMessage; /**< The content of the message to display */
		Gtk::Label** ppLabel; /**< After the message has been displayed, this will point to the Gtk::Label widget that has been created to display the message, so we can remove it later  */
	};

	/**
	 * Set of parameters for \fn Application::displayInfoMessage() function.
	 */
	struct messageBoxParameters {
		std::string* message; /**< The content of the message to display */
		void (*callBackFunction)(int response_id); /**< Optional callback function that will be called after the dialog box has been clicked "OK" */
	};

	/**
	 * One of the three kinect streams that we use.
	 */
	enum kinectStreamType {
		KINECT_STREAM_TYPE_RGB, /**< RGB color stream */
		KINECT_STREAM_TYPE_INFRARED, /**< Infrared stream */
		KINECT_STREAM_TYPE_DEPTH /**< Depth stream */
	};

	/**
	 * Application is the main class of KOCCA's control layer.
	 * It instanciates all the required data structures, threads, and user interface
	 * It is a static class (all it's members and methods are static, except constructor and destructor)
	 * It implements the "Singleton" design pattern.
	 */
	class Application {

	public:
		/**
		 * Application constructor.
		 * Arguments are the same as those passed to the program main() function.
		 * @param argc the number of argument passed to the program.
		 * @param argv values of each argument. The number of elements in argv[] sould be equal to argc.
		 */
		Application(int argc, char* argv[]);

		/**
		 * Application destructor
		 */
		~Application();

		/**
		 * Runs the user interface
		 */
		static void run();

		/**
		 * Callback function triggered by the KinectV2Sensor instance in a separate thread, each time a frame is available from the color stream
		 * @param frame the newly available color frame
		 */
		static void onKinectColorImage(cv::Mat frame);

		/**
		 * Callback function triggered by the KinectV2Sensor instance in a separate thread, each time a frame is available from the infrared stream
		 * @param frame the newly available infrared frame
		 */
		static void onKinectIRImage(cv::Mat frame);

		/**
		 * Callback function triggered by the KinectV2Sensor instance in a separate thread, each time a frame is available from the depth stream
		 * @param frame the newly available depth frame
		 */
		static void onKinectDepthImage(cv::Mat frame);

	private:

		/**
		 * This class member is set to TRUE when the first instance of Application is created. Any further attempt to instanciate another Application will raise an exception.
		 */
		static bool singletonInstanciated;

		/**
		 * Instance of GtkApplication that runs the user interface.
		 */
		static Gtk::Main* gtkApplication;

		/**
		 * The user interface main window.
		 */
		static widgets::MainWindow* mainWindow;

		/**
		 * The kinect sensor. Only Kinect V2 sensors are supported. Due to Microsoft Kinect SDK limitation, only one sensor at the time is supported. 
		 */
		static KinectV2Sensor kinect;

		/**
		 * The kinect's stream that is currently beeing displayed in the "monitoring" area of the UI
		 */
		static std::atomic<kinectStreamType> monitoredKinectStream;

		/**
		 * Thread implementation used for each new thread that is created when the kinect object emits a color frame.
		 * @param pFrame a pointer to the cv::Mat object corresponding to the new color frame.
		 */
		static void onKinectColorImageThread(cv::Mat* pFrame);

		/**
		 * Thread implementation used for each new thread that is created when the kinect object emits an infrared frame.
		 * @param pFrame a pointer to the cv::Mat object corresponding to the new infrared frame.
		 */
		static void onKinectIRImageThread(cv::Mat* pFrame);

		/**
		 * Thread implementation used for each new thread that is created when the kinect object emits a depth frame.
		 * @param pFrame a pointer to the cv::Mat object corresponding to the new depth frame.
		 */
		static void onKinectDepthImageThread(cv::Mat* pFrame);

		/**
		 * This will be set to true if a kinect sensor has actually been found and if it is available, false otherwise.
		 * @todo check if this is still used => delete if it isn't, or move to KinectV2Sensor class if it is.
		 */
		static std::atomic<bool> defaultKinectIsAvailable;

		/**
		 * Application's current intrinsic calibration parameters for the infrared stream
		 */
		static datalib::IntrinsicCalibrationParametersSet* intrinsicIRCalibRes;

		/**
		 * Application's current intrinsic calibration parameters for the RGB color stream
		 */
		static datalib::IntrinsicCalibrationParametersSet* intrinsicRGBCalibRes;

		/**
		 * Application's current extrinsic calibration parameters for the infrared stream
		 */
		static datalib::ExtrinsicCalibrationParametersSet* extrinsicIRCalibRes;

		/**
		 * Application's current extrinsic calibration parameters for the RGB color stream
		 */
		static datalib::ExtrinsicCalibrationParametersSet* extrinsicRGBCalibRes;

		/**
		 * The operation that is currently performed by the application.
		 */
		static operations::Operation* currentOperation;

		/**
		 * A mutex to lock and protect Application::currentOperation from simultaneous accesses conflicts in a multithread context
		 */
		static std::mutex currentOperation_mutex;
		
		/**
		 * client connection to the NetNat server (= Optitrack Motive)
		 */
		static NatNetClient* natNetClient;

		/**
		 * NatNet markers names
		 */
		static std::vector<std::string> natNetMarkerNames;
		
		/**
		 * The set of 3D points corresponding to the latest frame received trough natNetClient
		 */
		static std::vector<cv::Point3d>* latestsMocapFramePoints;

		/**
		 * The sequence that is currently loaded in the application
		 */
		static datalib::Sequence* currentLoadedSequence;

		/**
		 * Widget created to display a message while loading a sequence
		 */
		static Gtk::Label* loadingSequenceDataMsgLabel;

		/**
		 * Widget created to display a message while preparing the temp folder to record a sequence
		 */
		static Gtk::Label* preparingRecordMsgLabel;

		/**
		 * Widget created to display a message while finishing to write the new sequence to disk at the end of recording
		 */
		static Gtk::Label* writingSequenceDataMsgLabel;

		/**
		 * Widget created to display a message while exporting a sequence
		 */
		static Gtk::Label* exportingSequenceFileMsgLabel;

		/**
		 * Widget created to display a message while unzipping a sequence archive to temp folder
		 */
		static Gtk::Label* unzippingSequenceFileMsgLabel;

		/**
		 * Path to the file passed as an argument to the program, so we must open it at startup (if any and if it is a valid path)
		 */
		static std::string openFileAtStartup;

		/**
		 * Set to true if the application has some calibration data that has not been saved yet (a calibration a just been performed)
		 */
		static bool hasUnsavedCalibration;

		/**
		 * Set to true if the application has some sequence data that has not been saved yet (a sequence has just been recorded or restored from temp folder, but hasn't been properly exported as a sequence archive yet)
		 */
		static bool hasUnsavedSequence;

		/**
		 * Path to the root of the "temp" folder used for recording and archive unzipping.
		 * @todo default should be "/path/to/applicationroot/temp" but it should be stored in a config file and customizable through a "settings" panel in UI
		 */
		static boost::filesystem::path baseTempFolder;
		
		/**
		 * Invalidate last received frame for each "CvDrawingArea" (and descendant classes) widgets in the UI
		 * @throws KinectCallbackAssignmentException
		 */
		static void disableCurrentOperationDisplay();

		/**
		 * Callback function triggered when the user clicks the "Stop" button
		 */
		static void onClickStopButton();

		/**
		 * Displays a custom message in the "wait" popup. You probably souldn't call this function directly as it is meant to be called by the GTK main loop, after scheduling through gdk_threads_add_idle(). Instead, you should call addWaitMessage().
		 * @param data a pointer to a waitMessageParameters structure
		 * @return always false
		 */
		static gboolean displayWaitMessage(gpointer data);

		/**
		 * Adds a custom message to the "wait" popup and displays this popup if needed. It creates a Gtk::Label widget in the popup, and writes the reference to the created widget in the ppLabel parameter, so we can use it later to remove this specific label from the popup.
		 * @param messageString the content of the message to be displayed
		 * @param ppLabel will contain a reference to ne newly created label widget, after execution
		 */
		static void addWaitMessage(std::string messageString, Gtk::Label** ppLabel);

		/**
		 * Removes a currently displayed message from the "wait" popup. You probably souldn't call this function directly as it is meant to be called by the GTK main loop, after scheduling through gdk_threads_add_idle(). Instead, you should call removeWaitMessage().
		 * @param data a pointer to a waitMessageParameters structure
		 * @return always false
		 */
		static gboolean unDisplayWaitMessage(gpointer data);

		/**
		 * Removes from the "wait" popup a specific message identified by it's label widget reference.
		 * @param ppLabel a pointer to the label widget that should be removed
		 */
		static void removeWaitMessage(Gtk::Label** ppLabel);

		/**
		 * Displays an error message in a GTK dialog popup. You probably souldn't call this function directly as it is meant to be called by the GTK main loop, after scheduling through gdk_threads_add_idle(). Instead, you should call errorMessageBox().
		 * @param data a pointer to a messageBoxParameters structure.
		 * @return always false
		 */
		static gboolean displayErrorMessage(gpointer data);

		/**
		 * Displays an information message in a GTK dialog popup. You probably souldn't call this function directly as it is meant to be called by the GTK main loop, after scheduling through gdk_threads_add_idle(). Instead, you should call infoMessageBox().
		 * @param data a pointer to a messageBoxParameters structure.
		 * @return always false
		 */
		static gboolean displayInfoMessage(gpointer data);

		/**
		 * Displays a custom error message in a dialog popup.
		 * @param messageString the content of the message as a std::string
		 */
		static void errorMessageBox(std::string messageString);

		/**
		 * Displays a custom error message in a dialog popup.
		 * @param messageCStr the content of the message as a C string
		 */
		static void errorMessageBox(const char* messageCStr);

		/**
		 * Displays a custom information message in a dialog popup.
		 * @param messageString the content of the message as a std::string
		 * @param callBackFunction an optional reference to a callback function that will be called after the user closed the dialog. This function must of form "void myfunction(int)"
		 */
		static void infoMessageBox(std::string messageString, void (*callBackFunction)(int response_id) = NULL);

		/**

		 * Displays a custom information message in a dialog popup.
		 * @param messageCStr the content of the message as a C string
		 * @param callBackFunction an optional reference to a callback function that will be called after the user closed the dialog. This function must of form "void myfunction(int)"
		 */
		static void infoMessageBox(const char* messageCStr, void (*callBackFunction)(int response_id) = NULL);

		/**
		 * Checks if the default kinect sensor is available through Kinect SDK.
		 * @todo check if this is still used => delete if it isn't, or move to KinectV2Sensor class if it is.
		 */
		static void checkKinectThreadImp();

		/**
		 * Sets a operations::Operation instance as the current operation for the application.
		 * It also closes and delete the previous current operation, sets the kinect sensor up or down if needed, and sets appropriate kinect callbacks.
		 * @param newOperation the new operation instance to use application-wise
		 */
		static void setCurrentOperation(operations::Operation* newOperation);

		/**
		 * Implementation for the threads that are started by onCurrentOperationColorImageFrameOutput() at each new color image frame emitted by the current operation.
		 * @param tcFrame a pointer to the color image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationColorImageFrameOutputThread(datalib::TimeCodedFrame* tcFrame);

		/**
		 * Callback function triggered each time the current operation instance emits a color image frame. It's only job is to start a new thread running onCurrentOperationColorImageFrameOutputThread(), to process the data.
		 * @param tcFrame the color image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationColorImageFrameOutput(datalib::TimeCodedFrame tcFrame);

		/**
		 * Implementation for the threads that are started by onCurrentOperationIRImageFrameOutput() at each new infrared image frame emitted by the current operation.
		 * @param tcFrame a pointer to the infrared image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationIRImageFrameOutputThread(datalib::TimeCodedFrame* tcFrame);

		/**
		 * Callback function triggered each time the current operation instance emits an infrared image frame. It's only job is to start a new thread running onCurrentOperationIRImageFrameOutputThread(), to process the data.
		 * @param tcFrame the infrared image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationIRImageFrameOutput(datalib::TimeCodedFrame tcFrame);

		/**
		 * Implementation for the threads that are started by onCurrentOperationDepthImageFrameOutput() at each new depth image frame emitted by the current operation.
		 * @param tcFrame a pointer to the depth image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationDepthFrameOutputThread(datalib::TimeCodedFrame* tcFrame);

		/**
		 * Callback function triggered each time the current operation instance emits an depth image frame. It's only job is to start a new thread running onCurrentOperationDepthImageFrameOutputThread(), to process the data.
		 * @param tcFrame the depth image frame emitted, with it's timesamp.
		 */
		static void onCurrentOperationDepthFrameOutput(datalib::TimeCodedFrame tcFrame);

		/**
		 * Implementation for the threads that are started by onCurrentOperationMarkerFrameOutput() at each new marker frame emitted by the current operation.
		 * @param frame a pointer to the MocapMarkerFrame emitted.
		 */
		static void onCurrentOperationMarkerFrameOutputThread(datalib::MocapMarkerFrame* frame);

		/**
		 * Callback function triggered each time the current operation instance emits an marker frame. It's only job is to start a new thread running onCurrentOperationMarkerFrameOutputThread(), to process the data.
		 * @param frame the marker frame emitted.
		 */
		static void onCurrentOperationMarkerFrameOutput(datalib::MocapMarkerFrame frame);

		/**
		 * Callback function triggered when the current operation instance starts calibrating the RGB sensor (happens only in calibration mode).
		 */
		static void onCurrentOperationCalibrateRGB();

		/**
		 * Callback function triggered when the current operation instance starts calibrating the IR sensor (happens only in calibration mode).
		 */
		static void onCurrentOperationCalibrateInfrared();

		/**
		 * Updates the "save" calibration button state (active or inactive).
		 */
		static void updateSaveButton();

		/**
		 * Sets the intrinsic calibration parameters application-wise for the infrared sensor.
		 * @param newParameters the new intrinsic calibration parameters for the infrared sensor.
		 */
		static void setIntrinsicIRCalibrationParameters(datalib::IntrinsicCalibrationParametersSet newParameters);

		/**
		 * Sets the intrinsic calibration parameters application-wise for the RGB sensor.
		 * @param newParameters the new intrinsic calibration parameters for the RGB sensor.
		 */
		static void setIntrinsicRGBCalibrationParameters(datalib::IntrinsicCalibrationParametersSet newParameters);

		/**
		 * Sets the extrinsic calibration parameters application-wise for the infrared sensor.
		 * @param newParameters the new extrinsic calibration parameters for the infrared sensor.
		 */
		static void setExtrinsicIRCalibrationParameters(datalib::ExtrinsicCalibrationParametersSet newParameters);

		/**
		 * Sets the extrinsic calibration parameters application-wise for the RGB sensor.
		 * @param newParameters the new extrinsic calibration parameters for the RGB sensor.
		 */
		static void setExtrinsicRGBCalibrationParameters(datalib::ExtrinsicCalibrationParametersSet newParameters);
		
		/**
		 * Checks if a cv::Mat contains only zeros
		 * @param rgbMat the cv::Mat to check content
		 * @return true if the cv::Mat contains only zeros, false otherwise
		 */
		static bool rgbMatIsZeroFilled(cv::Mat rgbMat);

		/**
		 * Requests the NatNet server for markers names, then update the local list of names.
		 */
		static void updateNatNetMarkersNames();
		
		/**
		 * Callback function triggered each time some data is received from the NatNet Server (Motive)
		 * @see NatNet documentation
		 */
		static void __cdecl onNatNetData(sFrameOfMocapData* data, void* pUserData);

		/**
		 * Disconnects the application from the NatNet server.
		 */
		static void disconnectNatNetClient();

		/**
		 * Connects the application to the NatNet server.
		 * @param displayMessageOnFailure indicate if the application must display an error dialog in case the connection fails.
		 */
		static void connectNatNetClient(bool displayMessageOnFailure = true);

		/**
		 * Switches the application to monitoring mode (creates a new Monitoring operation and sets it as currentOperation).
		 */
		static void activateMonitoring();

		/**
		 * Callback function called after the user clicked the info dialog displayed before extrinsic calibration starts.
		 * @param response_id Not shure ??? Maybe some GTK ID for the info dialog (not used anyway, so you can ignore it)
		 */
		static void onInfoBeforeExtrinsicResponse(int response_id);

		/**
		 * Callback function called before extrinsic calibration starts. It displays an info dialog to the user.
		 */
		static void onCalibrationOperationBeforeExtrinsic();

		/**
		 * Callback function called when all requested calibrations are finished.
		 */
		static void onCalibrationOperationAllDone();

		/**
		 * Callback function called when the user clicks the "Start" calibration button.
		 */
		static void onClickStartCalibrationButton();

		/**
		 * Callback function called when the user clicks the "Pause" calibration button.
		 */
		static void onClickPauseCalibrationButton();

		/**
		 * Callback function called when the user clicks the "Abort" calibration button.
		 */
		static void onClickAbortCalibrationButton();
		
		/**
		 * Loads calibration parameters from a .kcf file, after asking the user what parts of this file should be loaded through a dialog popup.
		 * @param calibrationFilePath the path to the calibration file to load
		 * @param forceLoadAll if set to true, skips the dialog popup and load all the calibration parameters in the file.
		 * @throws CalibrationDataNotAvailableException
		 * @throws InvalidKinectCalibrationFileException
		 */
		static void loadCalibrationFile(const char* calibrationFilePath, bool forceLoadAll = false);

		/**
		 * Callback function called when the user clicks the "Load" calibration file button.
		 */
		static void onClickLoadCalibrationFileButton();

		/**
		 * Callback function called when the user clicks the "save" calibration file button.
		 */
		static void onClickSaveCalibrationToFileButton();

		/**
		 * Callback function called when the user changes the value of the "Countdown duration" spin button.
		 */
		static void onChangeCountdownDurationSpinButton();

		/**
		 * Callback function called when the user changes the value of the "Freeze duration" spin button.
		 */
		static void onChangeSnapshotFreezeDurationSpinButton();

		/**
		 * Callback function called when the user clicks the "Clear" calibration parameters button
		 */
		static void onClickClearCalibrationDataButton();

		/**
		 * Callback function called when the user clicks the "Connect" to NatNet server button.
		 */
		static void onClickNatNetConnectButton();

		/**
		 * Starts recording. It prepares the "temp" folder, creates a new "Sequence" object, creates a new "SequenceRecording" operation object, sets it as the currentOperation and tells it to start recording frames.
		 */
		static void startRecording();

		/**
		 * Callback function called when the user clicks the "Record" button.
		 */
		static void onClickRecordButton();

		/**
		 * Callback function called when the user clicks the "Play" button.
		 */
		static void onClickStartPlaybackButton();

		/**
		 * Callback function triggered during a sequence reading operation, each time the playhead moves.
		 * @param position the new position for the playhead (in milliseconds, within the sequence timemline).
		 */
		static void onCurrentOperationChangePlayheadPosition(unsigned long long position);

		/**
		 * Callback function triggered during a sequence reading operation, each time the reading buffer grows.
		 * @param endingPoint the new end for the buffer (in milliseconds, within the sequence timemline).
		 */
		static void onCurrentOperationUpdateBufferEndingPoint(unsigned long long endingPoint);

		/**
		 * Callback function triggered during a sequence reading operation, when the playing stops because the playhead has reached the sequence's end.
		 */
		static void onCurrentOperationStopAtTheEnd();
		
		/**
		 * Starts the reading of the "currentLoadedSequence" object. It creates a new "SequenceReading" operation object, switches the UI to reading mode and puts the playhead to 0ms position.
		 * @throws EmptySequenceException
		 */
		static void readCurrentLoadedSequence();

		/**
		 * Callback function triggered when a sequence has finished loading and is ready to be read.
		 * @param sequence a pointer to the loaded sequence.
		 */
		static void onSequenceLoaded(datalib::Sequence* sequence);

		/**
		 * Callback function called every time a long operation (i.e. : .ksa file unzipping) makes some progress, so we can update the progress bar in the UI.
		 * @param progress the new value for the operation progress, in percentage.
		 */
		static void onUpdateProgress(float progress);

		/**
		 * Loads data from a sequence "temp" directory (wich is available just after unzipping an existing .ksa file, or just after recording a new sequence, or when recovering from a crash)
		 * @param sequenceRootDirectoryPath the path to the sequence temp directory on the filesystem.
		 */
		static void openUnzippedSequenceFromDirectory(boost::filesystem::path sequenceRootDirectoryPath);

		/**
		 * Prepares a "temp" sub-folder and extracts a .ksa archive file content to it.
		 * @param ksaFileName the .ksa file path on the filesystem.
		 */
		static void unZipSequence(std::string ksaFileName);

		/**
		 * Callback function called when the user clicks the "Open" sequence button.
		 */
		static void onClickOpenSequenceButton();

		/**
		 * Callback function called when the user clicks the "Close" sequence button.
		 */
		static void onClickCloseSequenceButton();

		/**
		 * Stops the current sequence recording, finishes to write recorded data to the disk, then switches to sequence reading mode.
		 */
		static void stopRecording();

		/**
		 * Callback function called when the user clicks the "Go to begining" button.
		 */
		static void onClickGoToBeginingButton();

		/**
		 * Callback function called when the user clicks the "Go to the end" button.
		 */
		static void onClickGoToEndButton();

		/**
		 * Callback function called when the user manually moves the "playhead" slider.
		 * @param scroll ??? Not shure what it is : check the GTK documentation. We don't use it anyway.
		 * @param new_value the new value for the playhead, in milliseconds within the sequence timeline.
		 */
		static bool onMoveVideoTimeSliderScale(Gtk::ScrollType scroll, double new_value);

		/**
		 * Callback function called when the user clicks the "Go to previous frame" button.
		 */
		static void onClickPreviousButton();

		/**
		 * Callback function called when the user clicks the "Go to next frame" button.
		 */
		static void onClickNextButton();

		/**
		 * Opens a file of any of the two types supported by KOCCA : .kcf (Kinect Calibration File) or .ksa (KOCCA Sequence Archive). Used only when the user drags a file to the UI's main window or when a file is opened at startup as an argument for the executable.
		 * @param filePath the path to the file to open, on the filesystem.
		 */
		static bool openKoccaFile(const char* filePath);

		/**
		 * Checks if a given folder complies to the KOCCA temp structure, and if it actually contains any data.
		 * @param folderPath the patrh to the folder to check, on the filesystem.
		 */
		static bool folderContainsSequenceData(boost::filesystem::path folderPath);

		/**
		 * Browses KOCCA's "temp" subfolders until one of them is found with actual readable sequence data in it. Used at startup to check if some un-cleared sequence data has been left in the temp folder, wich may indicates a crash has happened last time KOCCA was executed.
		 */
		static boost::filesystem::path getFirstTempFolderWithRecoverableSequenceData();

		/**
		 * Reads the process ID from a "temp" subfolder name and returns it.
		 * @param folderPath the path to a KOCCA "temp" sub-folder, on the filesystem.
		 * @return a process ID (PID)
		 */
		static unsigned long getPIDFromTempFolderPath(boost::filesystem::path folderPath);

		/**
		 * Checks if a given process ID (PID) correspond to a KOCCA instance process that is currently running on the system.
		 * @param pid the process ID to check
		 * @return true if pid correspond to a running KOCCA process, false otherwise
		 */
		static bool pidIsARunningKoccaInstance(unsigned long pid);

		/**
		 * Lists PIDs of all KOCCA processes currently running on the system.
		 */
		static std::vector<unsigned long> getAllRunningKoccaPIDs();

		/**
		 * Checks if KOCCA's "temp" folder has any sub-folder that contains sequence data and don't belong to another KOCCA running instance.
		 */
		static bool baseTempFolderContainsRecoverableSequenceData();

		/**
		 * Callback function triggered when the UI's main window is shown for the first time (that is, when the application has just started up, finished loading the UI and is ready to roll).
		 * @see Gtk::Window::signal_map_event()
		 */
		static bool onShowMainWindow(_GdkEventAny* event);

		/**
		 * Callback function triggered when the user changes the marker selection in the "Mocap Markers List".
		 * @param selectedMarkersIDs IDs (= names) of the newly selected markers.
		 */
		static void on_mocapMarkersListView_selected_markers_changed(std::vector<std::string> selectedMarkersIDs);

		/**
		 * Callback function triggered when the user changes the marker selection in the "Monitoring" widget.
		 * @param selectedMarkersIDs IDs (= names) of the newly selected markers.
		 */
		static void on_monitor_selected_markers_changed(std::vector<std::string> selectedMarkersIDs);

		/**
		 * Exports the "currentLoadedSequence" object to a new .ksa file.
		 * @param destFilePath path to the new ksa file on the filesystem.
		 */
		static void exportSequence(std::string destFilePath);

		/**
		 * Callback function triggered when the user clicks the "Export" sequence button.
		 */
		static void onClickExportSequenceButton();

		/**
		 * Callback function triggered when the user drag&drops object(s) to the main window.
		 * @see Gtk::Window::signal_drag_data_received()
		 */
		static void onMainWindowDragDataReceived(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

		/**
		 * Callback function triggered when the user hits the "space" key on the keyboard.
		 */
		static void onMainWindowSpaceShortcut();

		/**
		 * Callback function triggered when the user closes the main window.
		 * @see Gtk::Window::signal_delete_event()
		 */
		static bool onCloseMainWindow(GdkEventAny* event);

		/**
		 * Returns the current process PID
		 */
		static unsigned long getProcessID();

		/**
		 * Creates a new sub-folder in the "temp" directory and returns it's path. To avoid duplicates, sub folder name is created with the form PID_TIME where "PID" is the current process PID and "TIME" is a local timestamp in milliseconds.
		 * @return the path to ne newly created sub-folder.
		 */
		static boost::filesystem::path getNewSequenceTempFolder();

		/**
		 * Deletes any file and sub-folder that resides in the "temp" directory.
		 */
		static void clearTempFolder();

		/**
		 * Switches the monitored kinect stream, so the application shows its frames in the "Monitor" widget. Note this is not only available in "Monitoring" mode, but also in "Calibration", "SequenceRecording" and "SequenceReading" modes.
		 * @param newMonitoredStream the kinect stream type to monitor from now
		 */
		static void setMonitoredKinectStream(kinectStreamType newMonitoredStream);

		/**
		 * Callback function triggered when the user clicks the "RGB stream" thumbnail.
		 */
		static void onSelectKinectRGBStreamThumbnail();

		/**
		 * Callback function triggered when the user clicks the "Infrared stream" thumbnail.
		 */
		static void onSelectKinectInfraredStreamThumbnail();

		/**
		 * Callback function triggered when the user clicks the "Depth stream" thumbnail.
		 */
		static void onSelectKinectDepthStreamThumbnail();
	};
} // namespace kocca

#endif //KOCCA_APPLICATION_H
