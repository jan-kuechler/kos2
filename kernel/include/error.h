#ifndef ERROR_H
#define ERROR_H

/**
 * Symbolic names for all possible error codes.
 */
enum error_code
{
	OK = 0,       /**< No error, everything is ok */
	E_ALIGN,      /**< Address is not page aligned */
	E_NO_MEM,     /**< Not enough memory */
	E_INVALID,    /**< Invalid parameter */
	E_IMPL,       /**< Not implemented */
	E_PRESENT,    /**< Allready present */
	E_UNKNOWN,    /**< Unknown error */
	E_NUM_ERRORS, /**< Invalid error code */
};

/**
 * Holds the last error that was set with seterr.
 */
extern int last_error; // in kmain.c

/**
 * Returns the last error.
 * @return Error code
 */
inline int geterr(void)
{
	return last_error;
}

/**
 * Sets the error code for the last error.
 * If the code is lower than 0 or bigger then E_NUM_ERRORS
 * it is changed to E_UNKNOWN.
 * @param err Error code
 */
inline void seterr(int err)
{
	err = err < 0 ? -err : err;
	if (err >= E_NUM_ERRORS)
		err = E_UNKNOWN;
	last_error = err;
}

#endif /*ERROR_H*/
