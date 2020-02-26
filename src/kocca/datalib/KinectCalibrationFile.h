#ifndef KOCCA_DATALIB_KINECT_CALIBRATION_FILE_H
#define KOCCA_DATALIB_KINECT_CALIBRATION_FILE_H

#include <tinyxml2.h>
#include <sstream>

#include "IntrinsicCalibrationParametersSet.h"
#include "ExtrinsicCalibrationParametersSet.h"

#ifdef WIN32
	#define snprintf	_snprintf
#endif

namespace kocca {
	namespace datalib {
		/**
		 * Utility class to read and write calibration parameters to/from .kcf XML files
		 */
		class KinectCalibrationFile {
		public:
			/**
			 * Constructs a new empty representation of a KinectCalibrationFile.
			 */
			KinectCalibrationFile();

			/**
			 * Constructs and reads calibration data from an existing XML file
			 *
			 * @param _filePath path to an existing and valid XML .kcf calibration file
			 * @throws InvalidKinectCalibrationFileException
			 */
			KinectCalibrationFile(const char* _filePath);

			/**
			 * Destructor
			 */
			~KinectCalibrationFile();

			/**
			 * Reads calibration data from an existing XML file
			 *
			 * @param _filePath path to an existing and valid XML .kcf calibration file
			 * @throws InvalidKinectCalibrationFileException
			 */
			void loadFromFile(const char* _filePath);

			/**
			 * Returns path to the file where the data has been read from, or NULL if it hasn't been written yet
			 */
			const char* getFilePath();

			/**
			 * Export file content data
			 */
			const char* getFileContent();

			/**
			 * Exports calibration data to a file
			 *
			 * @param _newFilePath path to save data to
			 */
			void saveTo(const char* _newFilePath);

			/**
			 * Check if file contains Intrinsic calibration parameters for IR mode
			 */
			bool hasIntrinsicIRParameters();

			/**
			 * Check if file contains Intrinsic calibration parameters for RGB mode
			 */
			bool hasIntrinsicRGBParameters();

			/**
			 * Check if file contains Extrinsic calibration parameters for IR mode
			 */
			bool hasExtrinsicIRParameters();

			/**
			 * Check if file contains Extrinsic calibration parameters for RGB mode
			 */
			bool hasExtrinsicRGBParameters();

			/**
			 * Get Intrinsic calibration parameters for IR mode
			 * @throws CalibrationDataNotAvailableException
			 */
			IntrinsicCalibrationParametersSet getIntrinsicIRParameters();

			/**
			 * Get Intrinsic calibration parameters for RGB mode
			 * @throws CalibrationDataNotAvailableException
			 */
			IntrinsicCalibrationParametersSet getIntrinsicRGBParameters();

			/**
			 * Get Extrinsic calibration parameters for IR mode
			 * @throws CalibrationDataNotAvailableException
			 */
			ExtrinsicCalibrationParametersSet getExtrinsicIRParameters();

			/**
			 * Get Extrinsic calibration parameters for RGB mode
			 * @throws CalibrationDataNotAvailableException
			 */
			ExtrinsicCalibrationParametersSet getExtrinsicRGBParameters();

			/**
			 * Set Intrinsic calibration parameters for IR mode
			 */
			void setIntrinsicIRParameters(IntrinsicCalibrationParametersSet parameters);

			/**
			 * Set Intrinsic calibration parameters for RGB mode
			 */
			void setIntrinsicRGBParameters(IntrinsicCalibrationParametersSet parameters);

			/**
			 * Set Extrinsic calibration parameters for IR mode
			 */
			void setExtrinsicIRParameters(ExtrinsicCalibrationParametersSet parameters);

			/**
			 * Set Extrinsic calibration parameters for RGB mode
			 */
			void setExtrinsicRGBParameters(ExtrinsicCalibrationParametersSet parameters);

		protected:

			/**
			 * Path to the file on the local filesystem
			 */
			const char* filePath;

			/**
			 * Pointer to a IntrinsicCalibrationParametersSet object, corresponding to Intrinsic IR parameters, if the file contains any
			 */
			IntrinsicCalibrationParametersSet* intrinsicIRParameters;

			/**
			 * Pointer to a IntrinsicCalibrationParametersSet object, corresponding to Intrinsic RGB parameters, if the file contains any
			 */
			IntrinsicCalibrationParametersSet* intrinsicRGBParameters;

			/**
			 * Pointer to a IntrinsicCalibrationParametersSet object, corresponding to Extrinsic IR parameters, if the file contains any
			 */
			ExtrinsicCalibrationParametersSet* extrinsicIRParameters;

			/**
			 * Pointer to a IntrinsicCalibrationParametersSet object, corresponding to Extrinsic RGB parameters, if the file contains any
			 */
			ExtrinsicCalibrationParametersSet* extrinsicRGBParameters;

			/**
			 * Parses an XML node corresponding to a calibration parameters set, then automatically builds a CalibrationParametersSet object of the right type and assign it to the right member of the current object
			 *
			 * @param xmlNode the XML node to parse
			 * @throws InvalidKinectCalibrationFileException
			 */
			void autoAssignParameters(tinyxml2::XMLNode* xmlNode);

			/**
			 * Parses an XML node corresponding to an intrinsic calibration parameters set, and builds an IntrinsicCalibrationParametersSet object containing it's data
			 *
			 * @param xmlNode the XML node to parse
			 */
			static IntrinsicCalibrationParametersSet* loadIntrinsicParameters(
					tinyxml2::XMLNode* xmlNode);

			/**
			 * Parses an XML node corresponding to an extrinsic calibration parameters set, and builds an IntrinsicCalibrationParametersSet object containing it's data
			 * @param xmlNode the XML node to parse
			 */
			static ExtrinsicCalibrationParametersSet* loadExtrinsicParameters(
					tinyxml2::XMLNode* xmlNode);

			/**
			 * Builds an XML node with the data of an IntrinsicCalibrationParametersSet object, and appends it to an XML Document
			 * @param xmlDocument the XML Document to append the new built node to
			 * @param parameters a pointer to the IntrinsicCalibrationParametersSet containing the data to build the XML node with
			 * @throws KinectCalibrationFileExportError
			 */
			static tinyxml2::XMLNode* exportIntrinsicParameters(
					tinyxml2::XMLDocument& xmlDocument,
					IntrinsicCalibrationParametersSet* parameters);

			/**
			 * Builds an XML node with the data of an ExtrinsicCalibrationParametersSet object, and appends it to an XML Document
			 * @param xmlDocument the XML Document to append the new built node to
			 * @param parameters a pointer to the ExtrinsicCalibrationParametersSet containing the data to build the XML node with
			 * @throws KinectCalibrationFileExportError
			 */
			static tinyxml2::XMLNode* exportExtrinsicParameters(
					tinyxml2::XMLDocument& xmlDocument,
					ExtrinsicCalibrationParametersSet* parameters);

			/**
			 * Reads an child XML Node that contains numeric data, and returns this data as a float
			 * @param parentNode the parent XML Node that contains the child to read data from
			 * @param childName the tag name of the child XML node to read data from
			 * @throws InvalidKinectCalibrationFileException
			 */
			static float readChildFloatValue(tinyxml2::XMLNode* parentNode,
					const char* childName);

			/**
			 * Creates an XML node containing numeric data, and appends it to an XML Document
			 * @param xmlDocument the XML Document to append the newly created child XML Node to
			 * @param childName the tag name of the child XML Node to create
			 * @param childValue the numeric value that will be contained by the child XML Node
			 * @throws KinectCalibrationFileExportError
			 */
			static tinyxml2::XMLElement* createChildWithFloatValue(
					tinyxml2::XMLDocument& xmlDocument, const char* childName,
					float childValue);

			/**
			 * Builds and returns a new tinyxml2::XMLDocument* containing all the XML data.
			 */
			tinyxml2::XMLDocument* buildNewXMLDocument();
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_KINECT_CALIBRATION_FILE_H
