#include "KinectCalibrationFile.h"
#include <limits>
#include "../Exceptions.h"

namespace kocca {
	namespace datalib {
		KinectCalibrationFile::KinectCalibrationFile() {
			intrinsicIRParameters = NULL;
			intrinsicRGBParameters = NULL;
			extrinsicIRParameters = NULL;
			extrinsicRGBParameters = NULL;
			filePath = NULL;
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		KinectCalibrationFile::KinectCalibrationFile(const char* _filePath) {
			intrinsicIRParameters = NULL;
			intrinsicRGBParameters = NULL;
			extrinsicIRParameters = NULL;
			extrinsicRGBParameters = NULL;
			loadFromFile(_filePath); // reads data from the existing file
		}

		const char* KinectCalibrationFile::getFilePath() {
			return(filePath);
		}

		KinectCalibrationFile::~KinectCalibrationFile() {
			if (intrinsicIRParameters != NULL)
				delete intrinsicIRParameters;

			if (intrinsicRGBParameters != NULL)
				delete intrinsicRGBParameters;

			if (extrinsicIRParameters != NULL)
				delete extrinsicIRParameters;

			if (extrinsicRGBParameters != NULL)
				delete extrinsicRGBParameters;
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		void KinectCalibrationFile::autoAssignParameters(tinyxml2::XMLNode* xmlNode) {
			std::string calibrationType(
					xmlNode->ToElement()->Attribute("calibrationType"));
			std::string camera(xmlNode->ToElement()->Attribute("camera"));

			if (calibrationType == "intrinsic") {
				if (camera == "IR")
					intrinsicIRParameters = loadIntrinsicParameters(xmlNode);
				else if (camera == "RGB")
					intrinsicRGBParameters = loadIntrinsicParameters(xmlNode);
				else
					throw InvalidKinectCalibrationFileException("Unknown camera type");
			} else if (calibrationType == "extrinsic") {
				if (camera == "IR")
					extrinsicIRParameters = loadExtrinsicParameters(xmlNode);
				else if (camera == "RGB")
					extrinsicRGBParameters = loadExtrinsicParameters(xmlNode);
				else
					throw InvalidKinectCalibrationFileException("Unknown camera type");
			} else
				throw InvalidKinectCalibrationFileException("Unknown calibration type");
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		void KinectCalibrationFile::loadFromFile(const char* _filePath) {
			filePath = _filePath;
			tinyxml2::XMLDocument xmlDoc;

			// loads the whole XML file into an XMLDocument object
			if (xmlDoc.LoadFile(filePath) == tinyxml2::XML_SUCCESS) {
				tinyxml2::XMLElement* rootElement = xmlDoc.RootElement();

				if (rootElement != NULL) {
					// We would read up to 4 Calibration Parameters Sets, no more (extra would be ignored)
					tinyxml2::XMLNode* firstChild = rootElement->FirstChild();

					if (firstChild != NULL) {
						// for each of the, we parse the corresponding XML Node, build a CalibrationParameterSet object and assign it to the right member of the current object
						autoAssignParameters(firstChild);
						tinyxml2::XMLNode* secondChild = firstChild->NextSibling();

						if (secondChild != NULL) {
							autoAssignParameters(secondChild);
							tinyxml2::XMLNode* thirdChild = secondChild->NextSibling();

							if (thirdChild != NULL) {
								autoAssignParameters(thirdChild);
								tinyxml2::XMLNode* fourthChild =
										thirdChild->NextSibling();

								if (fourthChild != NULL)
									autoAssignParameters(fourthChild);
							}
						}
					} else
						throw InvalidKinectCalibrationFileException("Root element does not have a child");
				} else
					throw InvalidKinectCalibrationFileException("No root element in XML document");
			} else {
				// if the XML file couldn't be read, we try to build an explicit error message
				int errorId = xmlDoc.ErrorID();
				std::ostringstream errorMessage;

				errorMessage << "Error trying to load XML document \"" << filePath
						<< "\"" << std::endl << std::endl << "Error #" << errorId
						<< std::endl;

				throw InvalidKinectCalibrationFileException(errorMessage.str().c_str());
			}
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		tinyxml2::XMLElement* KinectCalibrationFile::createChildWithFloatValue(
				tinyxml2::XMLDocument& xmlDocument, const char* childName,
				float childValue) {
			setlocale(LC_NUMERIC, "C");
			tinyxml2::XMLElement* childElement = xmlDocument.NewElement(childName);
			tinyxml2::XMLText* childText = xmlDocument.NewText("");
			char stringChildValue[32];

			if (snprintf(stringChildValue, 32, "%g", childValue) > 0) {
				childText->SetValue(stringChildValue);
				childElement->InsertEndChild(childText);
				return (childElement);
			} else
				throw KinectCalibrationFileExportException("Cannot write ChildValue to stringChildValue");
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		tinyxml2::XMLNode* KinectCalibrationFile::exportIntrinsicParameters(
				tinyxml2::XMLDocument& xmlDocument,
				IntrinsicCalibrationParametersSet* parameters) {
			tinyxml2::XMLNode* rootNode = xmlDocument.NewElement(
					"calibrationParameters");
			rootNode->ToElement()->SetAttribute("calibrationType", "intrinsic");

			tinyxml2::XMLNode* distortionNode = xmlDocument.NewElement("distortion");
			distortionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "k1", parameters->K[0]));
			distortionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "k2", parameters->K[1]));
			distortionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "p1", parameters->K[2]));
			distortionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "p2", parameters->K[3]));
			rootNode->InsertEndChild(distortionNode);

			tinyxml2::XMLNode* projectionNode = xmlDocument.NewElement("projection");
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "fx", parameters->A[0]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "s", parameters->A[1]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "x0", parameters->A[2]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "u01", parameters->A[3]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "fy", parameters->A[4]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "y0", parameters->A[5]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "u02", parameters->A[6]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "u03", parameters->A[7]));
			projectionNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "u04", parameters->A[8]));
			rootNode->InsertEndChild(projectionNode);

			rootNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "error", parameters->error));
			return (rootNode);
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		tinyxml2::XMLNode* KinectCalibrationFile::exportExtrinsicParameters(
				tinyxml2::XMLDocument& xmlDocument,
				ExtrinsicCalibrationParametersSet* parameters) {
			tinyxml2::XMLNode* rootNode = xmlDocument.NewElement(
					"calibrationParameters");
			rootNode->ToElement()->SetAttribute("calibrationType", "extrinsic");

			tinyxml2::XMLNode* translationNode = xmlDocument.NewElement("translation");
			translationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "x", parameters->T[0]));
			translationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "y", parameters->T[1]));
			translationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "z", parameters->T[2]));
			rootNode->InsertEndChild(translationNode);

			tinyxml2::XMLNode* rotationNode = xmlDocument.NewElement("rotation");
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "a", parameters->R[0]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "b", parameters->R[1]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "c", parameters->R[2]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "d", parameters->R[3]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "e", parameters->R[4]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "f", parameters->R[5]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "g", parameters->R[6]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "h", parameters->R[7]));
			rotationNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "i", parameters->R[8]));
			rootNode->InsertEndChild(rotationNode);

			rootNode->InsertEndChild(
					createChildWithFloatValue(xmlDocument, "error", parameters->error));
			return (rootNode);
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		tinyxml2::XMLDocument* KinectCalibrationFile::buildNewXMLDocument() {
			tinyxml2::XMLDocument* xmlDoc = new tinyxml2::XMLDocument;
			tinyxml2::XMLDeclaration* declaration = xmlDoc->NewDeclaration();
			xmlDoc->InsertFirstChild(declaration);
			tinyxml2::XMLNode* rootNode = xmlDoc->NewElement("kinectCalibrationFile");
			xmlDoc->InsertEndChild(rootNode);

			if (intrinsicIRParameters != NULL) {
				tinyxml2::XMLNode* intrIRNode = exportIntrinsicParameters(*xmlDoc,
						intrinsicIRParameters);
				intrIRNode->ToElement()->SetAttribute("camera", "IR");
				rootNode->InsertEndChild(intrIRNode);
			}

			if (intrinsicIRParameters != NULL) {
				tinyxml2::XMLNode* intrRGBNode = exportIntrinsicParameters(*xmlDoc,
						intrinsicRGBParameters);
				intrRGBNode->ToElement()->SetAttribute("camera", "RGB");
				rootNode->InsertEndChild(intrRGBNode);
			}

			if (extrinsicIRParameters != NULL) {
				tinyxml2::XMLNode* extrIRNode = exportExtrinsicParameters(*xmlDoc,
						extrinsicIRParameters);
				extrIRNode->ToElement()->SetAttribute("camera", "IR");
				rootNode->InsertEndChild(extrIRNode);
			}

			if (extrinsicRGBParameters != NULL) {
				tinyxml2::XMLNode* extrRGBNode = exportExtrinsicParameters(*xmlDoc,
						extrinsicRGBParameters);
				extrRGBNode->ToElement()->SetAttribute("camera", "RGB");
				rootNode->InsertEndChild(extrRGBNode);
			}

			return(xmlDoc);
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		const char* KinectCalibrationFile::getFileContent() {
			tinyxml2::XMLDocument* xmlDoc = buildNewXMLDocument();
			tinyxml2::XMLPrinter* printer = new tinyxml2::XMLPrinter;
			xmlDoc->Print(printer);
			const char* fileContent = printer->CStr();
			return(fileContent);
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		void KinectCalibrationFile::saveTo(const char* _newFilePath) {
			filePath = _newFilePath;
			tinyxml2::XMLDocument* xmlDoc = buildNewXMLDocument();
			xmlDoc->SaveFile(filePath);
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		float KinectCalibrationFile::readChildFloatValue(tinyxml2::XMLNode* parentNode,
				const char* childName) {
			float returnValue = std::numeric_limits<float>::quiet_NaN();

			if (parentNode != NULL) {
				const tinyxml2::XMLNode* childNode = parentNode->FirstChildElement(
						childName);

				if (childNode != NULL) {
					const tinyxml2::XMLNode* childValueNode = childNode->FirstChild();

					if (childValueNode != NULL) {
						std::string childValueString(childValueNode->Value());

						if (!childValueString.empty())
							std::stringstream(childValueString) >> returnValue;
						else
							throw InvalidKinectCalibrationFileException(
									"Could not read float from XML node with empty content");
					} else
						throw InvalidKinectCalibrationFileException(
								"Could not read float from XML node without content");
				} else
					throw InvalidKinectCalibrationFileException(
							"Could not find child XML node with specified tag name");
			} else
				throw InvalidKinectCalibrationFileException("NULL XML parent node");

			return (returnValue);
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		IntrinsicCalibrationParametersSet* KinectCalibrationFile::loadIntrinsicParameters(
				tinyxml2::XMLNode* xmlNode) {
			if ((xmlNode != NULL)
					&& (std::string("calibrationParameters").compare(xmlNode->Value())
							== 0)) {
				const char* calibrationType = xmlNode->ToElement()->Attribute(
						"calibrationType");

				if (std::string("intrinsic").compare(calibrationType) == 0) {
					tinyxml2::XMLNode* distortionNode = xmlNode->FirstChildElement(
							"distortion");
					tinyxml2::XMLNode* projectionNode = xmlNode->FirstChildElement(
							"projection");

					VEC4 K;
					MAT33 A;
					float error;

					K[0] = readChildFloatValue(distortionNode, "k1");
					K[1] = readChildFloatValue(distortionNode, "k2");
					K[2] = readChildFloatValue(distortionNode, "p1");
					K[3] = readChildFloatValue(distortionNode, "p2");

					A[0] = readChildFloatValue(projectionNode, "fx");
					A[1] = readChildFloatValue(projectionNode, "s");
					A[2] = readChildFloatValue(projectionNode, "x0");
					A[3] = readChildFloatValue(projectionNode, "u01");
					A[4] = readChildFloatValue(projectionNode, "fy");
					A[5] = readChildFloatValue(projectionNode, "y0");
					A[6] = readChildFloatValue(projectionNode, "u02");
					A[7] = readChildFloatValue(projectionNode, "u03");
					A[8] = readChildFloatValue(projectionNode, "u04");

					error = readChildFloatValue(xmlNode, "error");

					return (new IntrinsicCalibrationParametersSet(K, A, error));
				} else
					throw InvalidKinectCalibrationFileException(
							"Calibration parameters of non-intrinsic calibration");
			} else
				throw InvalidKinectCalibrationFileException("Not an XML calibration parameters file");
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 */
		ExtrinsicCalibrationParametersSet* KinectCalibrationFile::loadExtrinsicParameters(
				tinyxml2::XMLNode* xmlNode) {
			if ((xmlNode != NULL)
					&& (std::string("calibrationParameters")
							== std::string(xmlNode->Value()))) {
				const char* calibrationType = xmlNode->ToElement()->Attribute(
						"calibrationType");

				if (std::string("extrinsic").compare(calibrationType) == 0) {
					tinyxml2::XMLNode* translationNode = xmlNode->FirstChildElement(
							"translation");
					tinyxml2::XMLNode* rotationNode = xmlNode->FirstChildElement(
							"rotation");

					VEC3 T;
					MAT33 R;
					float error;

					T[0] = readChildFloatValue(translationNode, "x");
					T[1] = readChildFloatValue(translationNode, "y");
					T[2] = readChildFloatValue(translationNode, "z");

					R[0] = readChildFloatValue(rotationNode, "a");
					R[1] = readChildFloatValue(rotationNode, "b");
					R[2] = readChildFloatValue(rotationNode, "c");
					R[3] = readChildFloatValue(rotationNode, "d");
					R[4] = readChildFloatValue(rotationNode, "e");
					R[5] = readChildFloatValue(rotationNode, "f");
					R[6] = readChildFloatValue(rotationNode, "g");
					R[7] = readChildFloatValue(rotationNode, "h");
					R[8] = readChildFloatValue(rotationNode, "i");

					error = readChildFloatValue(xmlNode, "error");

					return (new ExtrinsicCalibrationParametersSet(T, R, error));
				} else
					throw InvalidKinectCalibrationFileException(
							"Calibration parameters of non-extrinsic calibration");
			} else
				throw InvalidKinectCalibrationFileException("Not an XML calibration parameters file");
		}

		bool KinectCalibrationFile::hasIntrinsicIRParameters() {
			return (intrinsicIRParameters != NULL);
		}

		bool KinectCalibrationFile::hasIntrinsicRGBParameters() {
			return (intrinsicRGBParameters != NULL);
		}

		bool KinectCalibrationFile::hasExtrinsicIRParameters() {
			return (extrinsicIRParameters != NULL);
		}

		bool KinectCalibrationFile::hasExtrinsicRGBParameters() {
			return (extrinsicRGBParameters != NULL);
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		IntrinsicCalibrationParametersSet KinectCalibrationFile::getIntrinsicIRParameters() {
			if (intrinsicIRParameters != NULL)
				return (*intrinsicIRParameters);
			else
				throw CalibrationDataNotAvailableException("intrinsicIRParameters not found");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		IntrinsicCalibrationParametersSet KinectCalibrationFile::getIntrinsicRGBParameters() {
			if (intrinsicRGBParameters != NULL)
				return (*intrinsicRGBParameters);
			else
				throw CalibrationDataNotAvailableException("intrinsicRGBParameters not found");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		ExtrinsicCalibrationParametersSet KinectCalibrationFile::getExtrinsicIRParameters() {
			if (extrinsicIRParameters != NULL)
				return (*extrinsicIRParameters);
			else
				throw CalibrationDataNotAvailableException("extrinsicIRParameters not found");
		}

		/**
		 * @throws CalibrationDataNotAvailableException
		 */
		ExtrinsicCalibrationParametersSet KinectCalibrationFile::getExtrinsicRGBParameters() {
			if (extrinsicRGBParameters != NULL)
				return (*extrinsicRGBParameters);
			else
				throw CalibrationDataNotAvailableException("extrinsicRGBParameters not found");
		}

		void KinectCalibrationFile::setIntrinsicIRParameters(
				IntrinsicCalibrationParametersSet parameters) {
			if (intrinsicIRParameters != NULL)
				delete intrinsicIRParameters;

			intrinsicIRParameters = new IntrinsicCalibrationParametersSet(parameters);
		}

		void KinectCalibrationFile::setIntrinsicRGBParameters(
				IntrinsicCalibrationParametersSet parameters) {
			if (intrinsicRGBParameters != NULL)
				delete intrinsicRGBParameters;

			intrinsicRGBParameters = new IntrinsicCalibrationParametersSet(parameters);
		}

		void KinectCalibrationFile::setExtrinsicIRParameters(
				ExtrinsicCalibrationParametersSet parameters) {
			if (extrinsicIRParameters != NULL)
				delete extrinsicIRParameters;

			extrinsicIRParameters = new ExtrinsicCalibrationParametersSet(parameters);
		}

		void KinectCalibrationFile::setExtrinsicRGBParameters(
				ExtrinsicCalibrationParametersSet parameters) {
			if (extrinsicRGBParameters != NULL)
				delete extrinsicRGBParameters;

			extrinsicRGBParameters = new ExtrinsicCalibrationParametersSet(parameters);
		}
	} // namespace datalib
} // namespace kocca
