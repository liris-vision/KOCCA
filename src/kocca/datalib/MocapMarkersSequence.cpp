#include "MocapMarkersSequence.h"
#include <fstream>
#include <iostream>
#include "../Exceptions.h"

namespace kocca {
	namespace datalib {
		void MocapMarkersSequence::addFrame(MocapMarkerFrame frame) {
			markersData.push_back(frame);
		}

		std::vector<std::string> MocapMarkersSequence::getAllMarkerNames() {
			std::vector<std::string> allNames;

			for(int i = 0; i < markersData.size(); i++) {
				MocapMarkerFrame frame = markersData.at(i);
				std::vector<std::string> frameNames = frame.getMarkerNames();

				for(int j = 0; j < frameNames.size(); j++) {
					std::string markerName = frameNames.at(j);
					bool nameAlreadyIn = false;

					for(int k = 0; (k < allNames.size()) && (!nameAlreadyIn); k++)
						nameAlreadyIn = (allNames.at(k) == markerName);
			
					if(!nameAlreadyIn)
						allNames.push_back(markerName);
				}
			}

			return allNames;
		}

		int MocapMarkersSequence::getFrameRankAtTime(unsigned long long time) {
			int rank = -1;

			if(markersData.size() > 1) {
				for(int i = 1; (rank == -1) && (i < markersData.size()); i++)
					if(markersData.at(i).time > time)
						rank = i-1;
			}
			else
				if((markersData.size() == 1) && (markersData.at(0).time <= time))
					rank = 0;

			return rank;
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		int MocapMarkersSequence::getNextFrameTime(unsigned long long time) {
			if(!markersData.empty()) {
				int i = 0;

				while((i < markersData.size()) && (markersData.at(i).time <= time))
					i++;

				return(markersData.at(i).time);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		int MocapMarkersSequence::getPreviousFrameTime(unsigned long long time) {
			if(!markersData.empty()) 	{
				int i = 0;

				while((i < markersData.size() - 1) && (markersData.at(i+1).time < time))
					i++;

				return(markersData.at(i).time);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws OutOfSequenceException
		 */
		MocapMarkerFrame MocapMarkersSequence::getFrameAtTime(unsigned long long time) {
			int rank = getFrameRankAtTime(time);

			if(rank != -1)
				return(markersData.at(rank));
			else
				throw OutOfSequenceException("No frame available in mocap data at requested time");
		}

		/**
		 * @throws std::out_of_range
		 */
		MocapMarkerFrame MocapMarkersSequence::getFrameAtRank(int rank) {
			return markersData.at(rank);
		}

		/**
		 * @throws FileReadingException
		 * @throws InvalidMocapDataFileException
		 * @throws DuplicateMarkerNameException
		 */
		void MocapMarkersSequence::readFromFile(const char* filePath, TaskProgress* taskProgress, float progressIncrement) {
			std::ifstream inputFile(filePath);

			int lineCount;
		
			if(taskProgress != NULL) {
				lineCount = std::count(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>(), '\n');
				inputFile.seekg(0, std::ios::beg);
			}

			if(inputFile.good()) {
				std::string headerLine;
				std::getline(inputFile, headerLine);

				if(!headerLine.empty()) {
					std::istringstream headerSS(headerLine);
					std::vector<std::string> markersNames;
					std::string markerName;

					while(!headerSS.eof() && std::getline(headerSS, markerName, ';'))
						markersNames.push_back(markerName);

					if(taskProgress != NULL)
						taskProgress->incrementProgress(progressIncrement / lineCount);

					if(markersNames.size() > 0) {
						std::string dataLine;

						// browse each frame
						while(!inputFile.eof() && std::getline(inputFile, dataLine)) {
							std::istringstream dataLineSS(dataLine);

							// read the first value, which should be the frame's time
							std::string stringValue;
							long long frameTime;
							std::getline(dataLineSS, stringValue, ';');
							std::stringstream(stringValue) >> frameTime;

							// now parse all markers coords for this frame
							std::vector<double> coordsValues;
				
							while(!dataLineSS.eof() && std::getline(dataLineSS, stringValue, ';')) {
								double doubleValue;
								std::stringstream(stringValue) >> doubleValue;
								coordsValues.push_back(doubleValue);
							}

							if(coordsValues.size() == (3*markersNames.size())) {
								MocapMarkerFrame frame(frameTime);

								for(int i = 0; i < markersNames.size(); i++) {
									MocapMarker marker(
											coordsValues.at(i*3),
											coordsValues.at(i*3+1),
											coordsValues.at(i*3+2),
											markersNames.at(i).c_str()
										);

									frame.add_marker(marker);
								}

								markersData.push_back(frame);
							}
							else
								throw InvalidMocapDataFileException("the number or numeric values found doesn't match the number of marker names in the header (must be a multiple of 3)");

						if(taskProgress != NULL)
							taskProgress->incrementProgress(progressIncrement / lineCount);
						}
					}
					else
						throw InvalidMocapDataFileException("no marker name found in header");
				}
				else
					throw InvalidMocapDataFileException("no header found");
			}
			else
				throw FileReadingException("can't read data file");
		}

		std::string MocapMarkersSequence::getCSVContent() {
			std::ostringstream stream;
			std::vector<std::string> markersNames = getAllMarkerNames();

			for(int i = 0; i < markersNames.size(); i++) {
				stream << markersNames.at(i);

				if(i < (markersNames.size() - 1))
					stream << ";";
			}

			stream << std::endl;

			for(int i = 0; i < markersData.size(); i++) {
				MocapMarkerFrame frame = markersData.at(i);

				if(frame.getMarkersCount() > 0) {
					stream << frame.time << ";";

					for(int j = 0; j < markersNames.size(); j++) {
						std::string markerName = markersNames.at(j);
			
						MocapMarker* marker = frame.getMarkerByName(markerName);

						if((marker != NULL))
							stream << marker->coords.x << ";" << marker->coords.y << ";" << marker->coords.z;
						else
							stream << ";;";

						if(j < (markersNames.size() - 1))
							stream << ";";
					}

					stream << std::endl;
				}
			}

			return stream.str();
		}

		void MocapMarkersSequence::writeToFile(const char* filePath) {
			std::ofstream outputFile;
			outputFile.open(filePath);
			outputFile << getCSVContent();
			outputFile.close();
		}

		bool MocapMarkersSequence::hasData() {
			bool dataFound = false;

			if(markersData.size() > 0) {
				int i = 0;

				while(!dataFound && (i < markersData.size()))
					dataFound = markersData.at(i).getMarkersCount() > 0;
			}

			return(dataFound);
		}
	} // namespace datalib
} // namespace kocca