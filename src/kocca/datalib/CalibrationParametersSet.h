#ifndef KOCCA_DATALIB_CALIBRATION_PARAMETERS_SET_H
#define KOCCA_DATALIB_CALIBRATION_PARAMETERS_SET_H

#include <string>

namespace kocca {
	namespace datalib {

		/**
		 * a 4 floats vector
		 */
		typedef float VEC[4];

		/**
		 * a 2 floats vector
		 */
		typedef float VEC2[2];

		/**
		 * a 3 floats vector
		 */
		typedef float VEC3[3];

		/**
		 * Alias for VEC (4 floats vector) 
		 */
		#define VEC4 VEC

		/**
		 * 4x4 matrix
		 */
		typedef float MAT44[16];

		/**
		 * 3x3 matrix
		 */
		typedef float MAT33[9];

		/**
		 * Abstract class, defines a common interface for descendant classes : IntrinsicCalibrationParametersSet and ExtrinsicCalibrationParametersSet
 		 */
		class CalibrationParametersSet {
		public:
			
			/**
			 * Because CalibrationParametersSet is an abstract class with a purely vitrual method, we have to declare a virtual destructor
			 */
			virtual ~CalibrationParametersSet() {};
			
			/**
			 * Error in parameters computing
			 */
			float error;
			
			/**
			 * Purely vitrual method to represent CalibrationParametersSet as string
			 */
			virtual std::string toString() = 0;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_CALIBRATION_PARAMETERS_SET_H
