#include <iostream>
#include <stdexcept>
#include "kocca/Application.h"

#ifdef WIN32
	// if the program is built in release mode (not debug)
	#ifndef _DEBUG
		// hide windows console output in release build (but let it show up in DEBUG)
		#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
	#endif
#endif

/**
 * As usual, the main() function is the program's entry point.
 * @param argc the number of argument passed to the program
 * @param argv values of each argument. The number of elements in argv[] sould be equal to argc.
 * @return 0 if the program ran without error (success), 1 if an exception has been raised during it's execution (failure)
 */
int main(int argc, char* argv[]) {

	// returnCode is the program status code that will be returned at the end of it's execution. By default it's 0 (success).
	int returnCode = 0;

	try {
		// we instantiate the Application class
		kocca::Application koccaApplication(argc, argv);

		// then we run it
		koccaApplication.run();
	}

	// if an exception is raised (something went wrong during Application instantiation or during run() execution)
	catch(std::exception& e) {
		// we write the exception's description to the error output
		std::cerr << "Unhandled exception : " << e.what() << std::endl;

		// the program return code will be 1 (failure)
		returnCode = 1;
	}

	// finally, we return the program's success or failure code
	return(returnCode);
}