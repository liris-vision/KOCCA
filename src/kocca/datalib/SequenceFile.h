#ifndef KOCCA_DATALIB_SEQUENCE_FILE_H
#define KOCCA_DATALIB_SEQUENCE_FILE_H

#include "Sequence.h"
#include "TaskProgress.h"

#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#include <mutex>

namespace kocca {
	namespace datalib {

		/**
		 * A structure for unzipSingleFileThread parameters
		 */
		struct unzipSingleFileThreadParams {

			/**
			 * The zip archive object to unzip our file from
			 */
			mz_zip_archive* pZip_archive;

			/**
			 * The index of the file to unzip 
			 */
			int index;

			/**
			 * The total number of files in the zip archive
			 */
			int filesCount;

			/**
			 * The temp destination folder, where to unzip our file
			 */
			boost::filesystem::path* pTempFolderPath;

			/**
			 * A pointer to a TaskProgress object, that we will update in order to be able to keep track of the current task's progression
			 */
			TaskProgress* pTaskProgress;

			/**
			 * A mutex to prevent simultaneous creation attempts of the temp folder in a multi-threaded context
			 */
			std::mutex* pFolderCreate_lock;

			/**
			 * A mutex to prevent simultaneous accesses to the zip object in a multi-threaded context
			 */
			std::mutex* pMzip_lock;
		};

		/**
		 * This utility class helps with the writing of kocca sequence archive (.ksa) files, or their unarchiving to a temp folder
		 */
		class SequenceFile {
		protected:

			/**
			 * The path of the sequence file.
			 */
			const char* sequenceFilePath;

			/**
			 * Exports all the frames of the color image stream of a sequence into a zip archive.
			 * @param pSequence the sequence to export color image frames from
			 * @param pzip_archive the zip archive to export the frames into
			 * @throws FileReadingError if an error happens during the reading of a frame image file
			 * @throws FileArchivingError if an error happens during the writing in the zip archive file
			 */
			void exportSequenceImageFrames(Sequence* pSequence, mz_zip_archive* pzip_archive);

			/**
			 * Exports all the frames of the infrared stream of a sequence into a zip archive.
			 * @param pSequence the sequence to export infrared frames from
			 * @param pzip_archive the zip archive to export the frames into
			 * @throws FileReadingError if an error happens during the reading of a frame image file
			 * @throws FileArchivingError if an error happens during the writing in the zip archive file
			 */
			void exportSequenceIRFrames(Sequence* pSequence, mz_zip_archive* pzip_archive);

			/**
			 * Exports all the frames of the depth stream of a sequence into a zip archive.
			 * @param pSequence the sequence to export depth frames from
			 * @param pzip_archive the zip archive to export the frames into
			 * @throws FileReadingError if an error happens during the reading of a frame image file
			 * @throws FileArchivingError if an error happens during the writing in the zip archive file
			 */
			void exportSequenceDepthFrames(Sequence* pSequence, mz_zip_archive* pzip_archive);

			/**
			 * Exports the calibration file of a sequence into a zip archive.
			 * @param pSequence the sequence to export the calibration file from
			 * @param pzip_archive the zip archive to export the calibration file into
			 * @throws FileArchivingException if an error happens during the writing in the zip archive file
			 */
			void exportSequenceCalibrationFile(Sequence* pSequence, mz_zip_archive* pzip_archive);

			/**
			 * Exports the MoCap markers data of a sequence into a zip archive.
			 * @param pSequence the sequence to export the MoCap markers data from
			 * @param pzip_archive the zip archive to export the MoCap markers data into
			 * @throws FileArchivingException if an error happens during the writing in the zip archive file
			 */
			void exportSequenceMarkers(Sequence* pSequence, mz_zip_archive* pzip_archive);

			
		public:

			/**
			 * Constructor
			 * @param _sequenceFilePath the path to the .ksa sequence archive file, as a C string.
			 */
			SequenceFile(const char* _sequenceFilePath);

			/**
			 * Exports the whole sequence into a .ksa sequence file
			 * @param pSequence the sequence to export
			 * @throws FileArchivingException if an error happens when writing/finalizing the .ksa archive
			 */
			void exportSequence(Sequence* pSequence);

			/**
			 * Unachives a .ksa sequence file to a temp folder
			 * @param tempPath the path of the destination temp folder where data will be written
			 * @param taskProgress a TaskProgress pointer, allowing to track the progress of this operation from other objects
			 * @param maxThreadsCount the maximum number of parallel threads that will run at the same time for unarchiving
			 * @throws FileReadingException if an error happens when reading/uncompressing the data in the .ksa archive
			 */
			bool unZipSequenceToTempPath(const char* tempPath, TaskProgress* taskProgress = NULL, int maxThreadsCount = 4);

			/**
			 * Implementation of a thread unzipping one file from the archive
			 * @param threadParams the parameters for the thread, as there are too many of them to pass them individually
			 */
			static void unzipSingleFileThread(unzipSingleFileThreadParams threadParams);
		};
	} // namespace datalib
} // namespace kocca

#endif //KOCCA_DATALIB_SEQUENCE_FILE_H
