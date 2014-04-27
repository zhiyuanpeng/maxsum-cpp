/** 
 * @file maxsum_c.h
 * Provides C compatible interface to maxsum C++ library
 * 
 */
#ifndef MAX_SUM_C_H
#define MAX_SUM_C_H

#include "maxsum/types.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
using namespace maxsum;
#endif

/**
 * Returns true if the specified variable is registered.
 * @param var id of the variable to search for.
 * @returns true if the specified variable is registered.
 */ 
int isRegistered_ms(VarID var);

/**
 * Returns the registered domain size for a specified variable.
 * @param var id of the variable to search for.
 * @returns domain size of var
 * @throws UnknownVariableException if the variable is not registered.
 */
ValIndex getDomainSize_ms(VarID var);

/**
 * Returns the number of currently registered variables.
 * @returns the number of currently registered variables.
 */
int getNumOfRegisteredVariables_ms();

/**
 * Registers a variable with a specified domain size.
 * Puts the specified variable in a global register, and stores its domain
 * size. Variables can be registered multiple times, but their domain size
 * must never change.
 * @returns -1 if this variable is already registered, but with a different
 * domain size.
 * @param var the unique id of this variable
 * @param siz the domain size of this variable
 * @see maxsum::registerVariables
 */
int registerVariable_ms(VarID var, ValIndex siz);

/**
 * C implementation of matlab ind2sub function.
 * @param idx the scalar linear index
 * @param nDims length of dimension size vector
 * @param pSiz input vector containing dimension sizes in order
 * @param pSub output vector in which to store sub indices
 * @return -1 on error, 0 otherwise.
 */
int ind2sub_ms
(
 const int idx,
 const int nDims,
 const int* const pSiz,
 int* const pSub
);

/**
 * C implementation of the matlab sub2ind function.
 * @param nDims length of dimension size vector
 * @param pSiz input vector containing dimension sizes in order
 * @param pSub input vector in which to store sub indices
 * @return -1 on error, or scalar linear index otherwise.
 */
int sub2ind_ms(const int nDims, const int* const pSiz, int* const pSub);

/**
 * Create a new maxsum controller.
 */
void* newMaxSumController_ms
(
 int maxIterations,
 ValType maxnorm
);

/**
 * Delete maxsum controller.
 */
void deleteMaxSumController_ms
(
 void* pController
);

/**
 * Set/Add factor to factor graph
 * @returns -1 on error
 */
int setFactor_ms
(
 void* pController,
 const FactorID id,
 const int nVars,
 const VarID* const pDomain,
 const ValType* const pData
);

/**
 * Remove factor from factor graph.
 */
void removeFactor_ms(void* pController, const FactorID id);

/**
 * Remove all factors.
 */
void clearAll_ms(void* pController);

/**
 * Report number of factors in factor graph.
 */
int noFactors_ms(void* pController);

/**
 * Report number of variables in factor graph.
 */
int noVars_ms(void* pController);

/**
 * Run the max-sum algorithm on factor graph.
 * @returns number of iterations performed.
 */
int optimise_ms(void* pController);

/**
 * After calling optimise_ms, returns the optimal value for each
 * variable in the factor graph.
 * @param[in] pController pointer to the max-sum controller
 * @param[out] pVars ordered list of all variable ids in graph
 * @param[out] pVals correspond optimal values for each variable in list
 * @returns number of values written
 */
int getValues_ms
(
 void* pController,
 VarID* pVars,
 ValIndex* pVals
);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif // MAX_SUM_C_H