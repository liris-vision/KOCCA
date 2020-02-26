#include "SequenceFile.h"
#include <sstream>
#include <fstream>
#include "boost/filesystem.hpp"
#include "../Exceptions.h"

#include <thread>

namespace kocca {
	namespace datalib {
		SequenceFile::SequenceFile(const char* _sequenceFilePath) {
			sequenceFilePath = _sequenceFilePath;
		}

		/**
		 * @throws FileArchivingException
		 * @throws FileReadingException
		 */
		void SequenceFile::exportSequenceImageFrames(Sequence* pSequence, mz_zip_archive* pzip_archive) {
			std::vector<FramePath> imageFramesList = pSequence->getImageFramesList();

			for(int i = 0; i < imageFramesList.size(); i++) {
				FramePath framePath = imageFramesList.at(i);

				std::ostringstream archiveRelativePath;
				archiveRelativePath << "image/" << framePath.time << ".jpeg";

				std::ifstream fileStream(framePath.path, std::ios::binary | std::ios::ate);
				std::streamsize fileStreamSize = fileStream.tellg();
				fileStream.seekg(0, std::ios::beg);
				char* fileContent = new char[fileStreamSize];

				if(fileStream.read(fileContent, fileStreamSize)) {
					if(!mz_zip_writer_add_mem_ex(pzip_archive, archiveRelativePath.str().c_str(), fileContent, fileStreamSize, "", 0, MZ_BEST_SPEED, 0, 0)) {
						std::ostringstream errMsg;
						errMsg << "Error while writing image file content to archive as " << archiveRelativePath.str();
						delete fileContent;
						throw FileArchivingException(errMsg.str().c_str());
					}
				}
				else {
					std::ostringstream errMsg;
					errMsg << "Error while reading image file content of " << framePath.path;
					delete fileContent;
					throw FileReadingException(errMsg.str().c_str());
				}

				delete fileContent;
				fileStream.close();
			}
		}

		/**
		* @throws FileArchivingException
		* @throws FileReadingException
		*/
		void SequenceFile::exportSequenceIRFrames(Sequence* pSequence, mz_zip_archive* pzip_archive) {
			std::vector<FramePath> irFramesList = pSequence->getIRFramesList();

			for (int i = 0; i < irFramesList.size(); i++) {
				FramePath framePath = irFramesList.at(i);

				std::ostringstream archiveRelativePath;
				archiveRelativePath << "infrared/" << framePath.time << ".png";

				std::ifstream fileStream(framePath.path, std::ios::binary | std::ios::ate);
				std::streamsize fileStreamSize = fileStream.tellg();
				fileStream.seekg(0, std::ios::beg);
				char* fileContent = new char[fileStreamSize];

				if (fileStream.read(fileContent, fileStreamSize)) {
					if (!mz_zip_writer_add_mem_ex(pzip_archive, archiveRelativePath.str().c_str(), fileContent, fileStreamSize, "", 0, MZ_BEST_SPEED, 0, 0)) {
						std::ostringstream errMsg;
						errMsg << "Error while writing infrared frame content to archive as " << archiveRelativePath.str();
						delete fileContent;
						throw FileArchivingException(errMsg.str().c_str());
					}
				}
				else {
					std::ostringstream errMsg;
					errMsg << "Error while reading image file content of " << framePath.path;
					delete fileContent;
					throw FileReadingException(errMsg.str().c_str());
				}

				delete fileContent;
				fileStream.close();
			}
		}

		/**
		 * @throws FileArchivingException
		 * @throws FileReadingException
		 */
		void SequenceFile::exportSequenceDepthFrames(Sequence* pSequence, mz_zip_archive* pzip_archive) {
			std::vector<FramePath> depthFramesList = pSequence->getDepthFramesList();

			for(int i = 0; i < depthFramesList.size(); i++) {
				FramePath framePath = depthFramesList.at(i);

				std::ostringstream archiveRelativePath;
				archiveRelativePath << "depth/" << framePath.time << ".png";

				std::ifstream fileStream(framePath.path, std::ios::binary | std::ios::ate);
				std::streamsize fileStreamSize = fileStream.tellg();
				fileStream.seekg(0, std::ios::beg);
				char* fileContent = new char[fileStreamSize];

				if(fileStream.read(fileContent, fileStreamSize)) {
					if(!mz_zip_writer_add_mem_ex(pzip_archive, archiveRelativePath.str().c_str(), fileContent, fileStreamSize, "", 0, MZ_BEST_SPEED, 0, 0)) {
						std::ostringstream errMsg;
						errMsg << "Error while writing depth file content to archive as " << archiveRelativePath.str();
						delete fileContent;
						throw FileArchivingException(errMsg.str().c_str());
					}
				}
				else {
					std::ostringstream errMsg;
					errMsg << "Error while reading depth file content of " << framePath.path;
					delete fileContent;
					throw FileReadingException(errMsg.str().c_str());
				}

				delete fileContent;
				fileStream.close();
			}
		}

		/**
		 * @throws FileArchivingException
		 * @throws KinectCalibrationFileExportException
		 */
		void SequenceFile::exportSequenceCalibrationFile(Sequence* pSequence, mz_zip_archive* pzip_archive) {
			if(pSequence->hasCalibrationData()) {
				KinectCalibrationFile* calibrationFile = pSequence->getCalibrationFile();
				std::string calibrationFileContent(calibrationFile->getFileContent());
	
				if(!calibrationFileContent.empty() && !mz_zip_writer_add_mem_ex(pzip_archive, "kinect_calibration_parameters.kcf", calibrationFileContent.c_str(), calibrationFileContent.length(), "", 0, MZ_BEST_SPEED, 0, 0))
					throw FileArchivingException("Error while writing kinect calibration file content to archive as kinect_calibration_parameters.kcf");
			}
		}

		/**
		 * @throws FileArchivingException
		 */
		void SequenceFile::exportSequenceMarkers(Sequence* pSequence, mz_zip_archive* pzip_archive) {
			if(pSequence->markersSequence.hasData()) {
				std::string markersCSVContent = pSequence->markersSequence.getCSVContent();

				if(!markersCSVContent.empty() && !mz_zip_writer_add_mem_ex(pzip_archive, "markersData.csv", markersCSVContent.c_str(), markersCSVContent.length(), "", 0, MZ_BEST_SPEED, 0, 0))
					throw FileArchivingException("Error while writing kinect markers data CSV content to archive as markersData.csv");
			}
		}

		/**
		 * @throws FileArchivingException
		 * @throws FileReadingException
		 * @throws KinectCalibrationFileExportException
		 */
		void SequenceFile::exportSequence(Sequence* pSequence) {
			mz_zip_archive zip_archive;
			memset(&(zip_archive), 0, sizeof(zip_archive));

			if(mz_zip_writer_init_file(&zip_archive, sequenceFilePath, 0)) {
				exportSequenceImageFrames(pSequence, &zip_archive);
				exportSequenceIRFrames(pSequence, &zip_archive);
				exportSequenceDepthFrames(pSequence, &zip_archive);
				exportSequenceCalibrationFile(pSequence, &zip_archive);
				exportSequenceMarkers(pSequence, &zip_archive);

				bool resFinalize = mz_zip_writer_finalize_archive(&zip_archive);
				bool resEnd = mz_zip_writer_end(&zip_archive);

				if(!resFinalize || !resEnd) {
					std::ostringstream errMsg;
					errMsg << "Error trying to finalize archive " << sequenceFilePath << std::endl << "Archive may be currupted.";
					throw FileArchivingException(errMsg.str().c_str());
				}
			}
			else {
				std::ostringstream errMsg;
				errMsg << "Error trying to create archive " << sequenceFilePath;
				throw FileArchivingException(errMsg.str().c_str());
			}
		}

		void SequenceFile::unzipSingleFileThread(unzipSingleFileThreadParams threadParams) {
			mz_zip_archive_file_stat file_stat;

			if(mz_zip_reader_file_stat(threadParams.pZip_archive, threadParams.index, &file_stat)) {
				std::string archiveRelativeFilePath(file_stat.m_filename);
				boost::filesystem::path filePath = *(threadParams.pTempFolderPath) / archiveRelativeFilePath;

				if(mz_zip_reader_is_file_a_directory(threadParams.pZip_archive, threadParams.index)) {
					if(!boost::filesystem::exists(filePath)) {
						threadParams.pFolderCreate_lock->lock();

						if(!boost::filesystem::create_directory(filePath)) {
							threadParams.pFolderCreate_lock->unlock();
							std::cerr << "Error while attempting to create a directory in temp folder : " << filePath.string() << std::endl;
						}
						else
							threadParams.pFolderCreate_lock->unlock();
					}
					else {
						if(!boost::filesystem::is_directory(filePath))
							std::cerr << "Error : " << filePath.string() << " exists but is not a directory" << std::endl;
					}
				}
				else {
					boost::filesystem::path parentFolderPath = filePath.parent_path();
					threadParams.pFolderCreate_lock->lock();

					if(!boost::filesystem::exists(parentFolderPath)) {
						if(!boost::filesystem::create_directory(parentFolderPath)) {
							threadParams.pFolderCreate_lock->unlock();
							std::cerr << "Error : " << parentFolderPath.string() << " does not exists and couldn't be created" << std::endl;
						}
						else
							threadParams.pFolderCreate_lock->unlock();
					}
					else
						threadParams.pFolderCreate_lock->unlock();

					if((boost::filesystem::exists(parentFolderPath) && boost::filesystem::is_directory(parentFolderPath))) {
						size_t fileSize = file_stat.m_uncomp_size;

						if(fileSize > 0) {
							try {
								threadParams.pMzip_lock->lock();
								void* fileBuf = mz_zip_reader_extract_to_heap(threadParams.pZip_archive, threadParams.index, &fileSize, 0);
								threadParams.pMzip_lock->unlock();

								std::ofstream outputFile;
								outputFile.open(filePath.string(), std::ios_base::out | std::ios_base::binary);
								outputFile.write((const char*)fileBuf, fileSize);
								outputFile.close();

								mz_free(fileBuf);
							}
							catch(std::exception& e) {
								std::cerr << "Error : exception thrown during file extraction : " << e.what() << std::endl;
							}
						}
						else
							std::cerr << "Error : file size = 0 !" << std::endl;
					}
					else
						std::cerr << "Error : " << parentFolderPath.string() << " does not exists, or is not a directory" << std::endl;
				}
			}
			else
				std::cerr << "Error trying to read stat of archived file #" << threadParams.index << std::endl;

			if(threadParams.pTaskProgress != NULL)
				threadParams.pTaskProgress->incrementProgress(100.0 / (float)(threadParams.filesCount));
		}

		/**
		 * @throws TempFolderNotAvailableException
		 * @throws FileReadingException
		 */
		bool SequenceFile::unZipSequenceToTempPath(const char* tempPath, TaskProgress* taskProgress, int maxThreadsCount) {
			if(taskProgress != NULL)
				taskProgress->setProgress(0);

			boost::filesystem::path tempFolderPath = tempPath;
			mz_zip_archive zip_archive;
			memset(&(zip_archive), 0, sizeof(zip_archive));

			if(mz_zip_reader_init_file(&zip_archive, sequenceFilePath, 0)) {
				int filesCount = (int)mz_zip_reader_get_num_files(&zip_archive);
				std::deque<std::thread*> unzipThreads;
				std::mutex folderCreate_lock;
				std::mutex mzip_lock;

				for(int i = 0; i < filesCount; i++) {
					unzipSingleFileThreadParams unzipThreadParams;
					unzipThreadParams.pZip_archive = &zip_archive;
					unzipThreadParams.index = i;
					unzipThreadParams.filesCount = filesCount;
					unzipThreadParams.pTempFolderPath = &tempFolderPath;
					unzipThreadParams.pTaskProgress = taskProgress;
					unzipThreadParams.pFolderCreate_lock = &folderCreate_lock;
					unzipThreadParams.pMzip_lock = &mzip_lock;

					std::thread* pUnzipThread = new std::thread(&SequenceFile::unzipSingleFileThread, unzipThreadParams);
					unzipThreads.push_back(pUnzipThread);

					if(unzipThreads.size() >= maxThreadsCount) {
						unzipThreads.at(0)->join();
						unzipThreads.pop_front();
					}
				}

				// wait for all remaining threads to complete
				for (int i = 0; i < unzipThreads.size(); i++) {
					unzipThreads.at(i)->join();
					delete unzipThreads.at(i);
				}

				mz_zip_reader_end(&zip_archive);
			}
			else{
				std::ostringstream errMsg;
				errMsg << "Error trying to open archive " << sequenceFilePath;
				throw FileReadingException(errMsg.str().c_str());
			}

			if(taskProgress != NULL)
				taskProgress->setProgress(100);

			return true;
		}
	} // namespace datalib
} // namespace kocca