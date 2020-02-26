#ifndef KOCCA_EXCEPTIONS_H
#define KOCCA_EXCEPTIONS_H

#include <stdexcept>

class KinectException : public std::runtime_error {
public:
	KinectException(const char* _message) : std::runtime_error(_message) {}
};

class DefaultKinectSensorNotFoundException : public KinectException {
public:
	DefaultKinectSensorNotFoundException(const char* _message) : KinectException(_message) {}
};

class OpenKinectSensorFailedException : public KinectException {
public:
	OpenKinectSensorFailedException(const char* _message) : KinectException(_message) {}
};

class KinectSensorStreamError : public KinectException {
public:
	KinectSensorStreamError(const char* _message) : KinectException(_message) {}
};

class KinectCallbackAssignmentException: public std::runtime_error {
public:
	KinectCallbackAssignmentException(const char* _message): std::runtime_error(_message){}
};

class InvalidKinectCalibrationFileException: public std::runtime_error {
public:
	InvalidKinectCalibrationFileException(const char* _message): std::runtime_error(_message){}
};

class InvalidMocapDataFileException: public std::runtime_error {
public:
	InvalidMocapDataFileException(const char* _message): std::runtime_error(_message){}
};

class KinectCalibrationFileExportException: public std::runtime_error {
public:
	KinectCalibrationFileExportException(const char* _message): std::runtime_error(_message){}
};

class CalibrationDataNotAvailableException: public std::runtime_error {
public:
	CalibrationDataNotAvailableException(const char* _message): std::runtime_error(_message){}
};

class TempFolderNotAvailableException: public std::runtime_error {
public:
	TempFolderNotAvailableException(const char* _message): std::runtime_error(_message){}
};

class EmptySequenceException: public std::runtime_error {
public:
	EmptySequenceException(const char* _message): std::runtime_error(_message){}
};

class EmptySequenceStreamException: public std::runtime_error {
public:
	EmptySequenceStreamException(const char* _message): std::runtime_error(_message){}
};

class InvalidSequenceRankException: public std::out_of_range {
public:
	InvalidSequenceRankException(const char* _message): std::out_of_range(_message){}
};

class NoNextEventException: public std::out_of_range {
public:
	NoNextEventException(const char* _message): std::out_of_range(_message){}
};

class NoPreviousEventException: public std::out_of_range {
public:
	NoPreviousEventException(const char* _message): std::out_of_range(_message){}
};

class FileReadingException: public std::runtime_error {
public:
	FileReadingException(const char* _message): std::runtime_error(_message){}
};

class FileArchivingException: public std::runtime_error {
public:
	FileArchivingException(const char* _message): std::runtime_error(_message){}
};

class RecordBufferOverFlowException: public std::bad_alloc {
public:
	RecordBufferOverFlowException(const char* _message): std::bad_alloc(/*_message*/){}
};

class FileWritingException: public std::runtime_error {
public:
	FileWritingException(const char* _message): std::runtime_error(_message){}
};

class DuplicateMarkerNameException: public std::runtime_error {
public:
	DuplicateMarkerNameException(const char* _message): std::runtime_error(_message){}
};

class OutOfSequenceException: public std::out_of_range {
public:
	OutOfSequenceException(const char* _message): std::out_of_range(_message){}
};

class FrameNotInBufferException: public std::out_of_range {
public:
	FrameNotInBufferException(const char* _message): std::out_of_range(_message){}
};

class EmptyFrameException: public std::runtime_error {
public:
	EmptyFrameException(const char* _message): std::runtime_error(_message){}
};

class SingletonMultipleInstanciationException: public std::logic_error {
public:
	SingletonMultipleInstanciationException(const char* _message): std::logic_error(_message){}
};

#endif // KOCCA_EXCEPTIONS_H
