/**
 * @file DiscreteFunction.h
 * Defines the maxsum::DiscreteFunction class and related utility functions.
 */
#ifndef MAX_SUM_DISCRETE_FUNCTION_H
#define MAX_SUM_DISCRETE_FUNCTION_H

#include "maxsum/EigenWithPlugin.h"
#include <cmath>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "common.h"
#include "register.h"
#include "DomainIterator.h"

namespace maxsum
{
   /**
    * Class representing functions of sets of variables with discrete domains.
    * @tparam ValType the scalar type of value returned by this function.
    * We expect this to by a primitive numeric type, such as double or int.
    */
   class DiscreteFunction
   {
      /**
       * Pretty prints this function.
       * Format is similar to the disp function in Matlab for N-D arrays, except
       * that first dimension appears in rows rather than columns.
       */
      friend std::ostream& operator<<
      (
       std::ostream& out,
       const DiscreteFunction& fun
      );

   private:

      /**
       * Convenience typedef for type used to store variable ids
       */
      typedef std::vector<VarID> VarVec;

      /**
       * Convenience typedef for type used to store variable sizes
       */
      typedef std::vector<ValIndex> SizeVec;

      /**
       * Convenience typedef for type used to store function values.
       */
      typedef Eigen::Array<ValType, Eigen::Dynamic, 1> ValVec;

      /**
       * Set of variables on which this function depends.
       */
      VarVec vars_i;

      /**
       * Cache specifying the domain size for each variable on which this
       * function depends.
       */
      SizeVec size_i;

      /**
       * Array containing the values for this function.
       */
      ValVec values_i;

   public:

      // Eigen new operator (only required if we use fixed size eigen types.
      // Current we only use dynamic sizes.
      // EIGEN_MAKE_ALIGNED_OPERATOR_NEW

      /**
       * Default Constructor creates constant function that depends on no
       * variables.
       * @param[in] val the constant scalar value of this function.
       */
      DiscreteFunction(ValType val=0)
         : vars_i(0), size_i(0), values_i()
      {
         values_i.resize(1);
         values_i(0) = val;
      }

      /**
       * Constructs function depending on specified variables.
       * When passing variable ids, we assume that the list is unique and
       * sorted. Initially, for any input the function returns the value of
       * the val parameter, which defaults to zero.
       * @param[in] begin Iterator to start of variable list.
       * @param[in] end  Iterator to end of variable list.
       * @param[in] val inital scalar value output for function.
       * @throws UnknownVariableException if any variable in the list specified
       * by \c begin and \c end is not registered.
       */
      template<class VarIt> DiscreteFunction
      (
       VarIt begin,
       VarIt end,
       ValType val=0
      )
      : vars_i(begin,end), size_i(end-begin), values_i()
      {
         //*********************************************************************
         // Ensure that the variable ids are sorted.
         //*********************************************************************
         std::sort(vars_i.begin(),vars_i.end());

         //*********************************************************************
         // Cache the size of each variable, and from this, calculate
         // the total capacity required for the data array.
         //*********************************************************************
         ValIndex totalSize = 1;
         for(int k=0; k<vars_i.size(); k++)
         {
            size_i[k] = getDomainSize(vars_i[k]);
            totalSize *= size_i[k];
         }

         //*********************************************************************
         // Initialise the data array
         //*********************************************************************
         values_i.resize(totalSize);
         Eigen::Matrix<ValType,1,1> valWrapper;
         valWrapper(0) = val;
         values_i.matrix().rowwise() = valWrapper;

      } // DiscreteFunction constructor

      /**
       * Constructs a function that depends on only one variable.
       * @param[in] var the variable to put in this function's domain.
       * @param[in] val scalar value used to initialise function across domain.
       * @post \f$ \forall k \; f(k)=val\f$
       * @throws UnknownVariableException if \c var is not registered.
       */
      DiscreteFunction(VarID var, ValType val)
         : vars_i(1,var), size_i(1,getDomainSize(var)), values_i()
      {
         values_i.resize(getDomainSize(var));
         Eigen::Matrix<ValType,1,1> valWrapper;
         valWrapper(0) = val;
         values_i.matrix().rowwise() = valWrapper;
      }

      /**
       * Copy Constructor performs deep copy.
       * @param[in] val the object to copy.
       */
      DiscreteFunction(const DiscreteFunction& val)
         : vars_i(val.vars_i), size_i(val.size_i), values_i(val.values_i) {}

      /**
       * Accessor method for the total size this function's domain.
       */
      ValIndex domainSize() const
      {
         return values_i.size();
      }
      
      /**
       * Returns true if this function depends on the specified variable.
       * @param[in] var The id of the variable to search for in this function's
       * domain.
       * @returns true if <code>var</code> is in this function's domain.
       */
      bool dependsOn(VarID var) const;

      /**
       * Type of iterator returned by DiscreteFunction::varBegin() and
       * DiscreteFunction::varEnd() functions.
       */
      typedef std::vector<VarID>::const_iterator VarIterator;

      /**
       * Returns an iterator to the start of this function's domain variable
       * list. The variable list cannot be modified through this iterator.
       */
      VarIterator varBegin() const
      {
         return vars_i.begin();
      }

      /**
       * Returns an iterator to the end of this function's domain variable
       * list. This variable list cannot be modified through this iterator.
       */
      VarIterator varEnd() const
      {
         return vars_i.end();
      }

      /**
       * Type of iterator returned by DiscreteFunction::sizeBegin() and
       * DiscreteFunction::sizeEnd() functions.
       */
      typedef std::vector<ValIndex>::const_iterator SizeIterator;

      /**
       * Returns an iterator to the start of this function's domain variable
       * size list. This list cannot be modified through this iterator. The
       * values in this list are defined such that
       * size[k] = maxsum::getDomainSize(var[k]);
       * However, this method is generally faster than using
       * maxsum::getDomainSize() directly.
       * @see maxsum::getDomainSize()
       * @see DiscreteFunction::sizeEnd()
       */
      SizeIterator sizeBegin() const
      {
         return size_i.begin();
      }

      /**
       * Returns an iterator to the end of this function's domain variable
       * size list. This list cannot be modified through this iterator. The
       * values in this list are defined such that
       * size[k] = maxsum::getDomainSize(var[k]);
       * However, this method is generally faster than using
       * maxsum::getDomainSize() directly.
       * @see maxsum::getDomainSize()
       * @see DiscreteFunction::sizeBegin()
       */
      SizeIterator sizeEnd() const
      {
         return size_i.end();
      }


      /**
       * Returns the number of variables on which this function depends
       */
      int noVars() const
      {
         return vars_i.size();
      }

      /**
       * Sets this function to a constant scalar value.
       * @param[in] val the value to assign to this function.
       * @post this function will no longer depend on any variables.
       */
      DiscreteFunction& operator=(ValType val);

      /**
       * Sets this function to a constant scalar value, but preserves
       * domain.
       * @param[in] val the value to assign to this function.
       * @post the domain of this function will be unchanged, but all values
       * will be set to the specified constant value.
       */
      DiscreteFunction& assignKeepDomain(ValType val)
      {
         values_i.setConstant(values_i.size(), val);
         return *this;

      } // assignKeepDomain

      /**
       * Sets this function to be equal to another.
       * @param[in] val the value to assign to this function.
       */
      DiscreteFunction& operator=(const DiscreteFunction& val);

      /**
       * Adds a scalar value to this function.
       */
      DiscreteFunction& operator+=(ValType val)
      {
         values_i += val;
         return *this;
      }

      /**
       * Subtracts a scalar value from this function.
       */
      DiscreteFunction& operator-=(ValType val)
      {
         values_i -= val;
         return *this;
      }

      /**
       * Multiplies this function by a scalar.
       */
      DiscreteFunction& operator*=(ValType val)
      {
         values_i *= val;
         return *this;
      }

      /**
       * Divides this function by a scalar.
       */
      DiscreteFunction& operator/=(ValType val)
      {
         values_i /= val;
         return *this;
      }

      /**
       * Multiply function by -1
       */
      DiscreteFunction operator-() const
      {
         return DiscreteFunction(*this) *= -1;
      }

      /**
       * Identity function.
       * @returns a reference to this function.
       */
      const DiscreteFunction& operator+() const
      {
         return *this;
      }

      /**
       * Identity function.
       * @returns a reference to this function.
       */
      DiscreteFunction& operator+() 
      {
         return *this;
      }

      /**
       * Adds a function to this one, expanding the domain if necessary.
       */
      DiscreteFunction& operator+=(const DiscreteFunction& rhs);

      /**
       * Subtracts a function from this one, expanding domain if necessary.
       */
      DiscreteFunction& operator-=(const DiscreteFunction& rhs);

      /**
       * Multiplies this function by another, expanding domain if necessary.
       */
      DiscreteFunction& operator*=(const DiscreteFunction& rhs);

      /**
       * Divides this function by another, expanding domain if necessary.
       */
      DiscreteFunction& operator/=(const DiscreteFunction& rhs);

      /**
       * Subtract function or scalar.
       */
      template<class T> DiscreteFunction operator-(T rhs) const
      {
         return DiscreteFunction(*this) -= rhs;
      }

      /**
       * Add function or scalar.
       */
      template<class T> DiscreteFunction operator+(T rhs) const
      {
         return DiscreteFunction(*this) += rhs;
      }

      /**
       * Multiply function or scalar.
       */
      template<class T> DiscreteFunction operator*(T rhs) const
      {
         return DiscreteFunction(*this) *= rhs;
      }

      /**
       * Divide function by function or scalar.
       */
      template<class T> DiscreteFunction operator/(T rhs) const
      {
         return DiscreteFunction(*this) /= rhs;
      }

      /**
       * Adds a list of Functions to this one, expanding the domain if
       * necessary. 
       * The result is \f$this + \sum_k funcs[k]\f$
       * @todo Make this more efficient by expanding domain in advance.
       * @returns reference to this function.
       */
      template<class VecIt> DiscreteFunction& add(VecIt begin, VecIt end)
      {
         for(VecIt it=begin; it!=end; ++it)
         {
            (*this) += *it;
         }
         return *this;
      }

      /**
       * Access coefficient using linear index
       */
      ValType& operator()(ValIndex ind)
      {
         return values_i[ind];
      }

      /**
       * Access coefficient using linear index
       */
      const ValType& operator()(ValIndex ind) const
      {
         return values_i[ind];
      }

      /**
       * Access coefficient using subindices specified in argument list.
       */
      ValType& operator()(ValIndex ind1, ValIndex ind2, ...);

      /**
       * Access coefficient using subindices specified in argument list.
       */
      const ValType& operator()(ValIndex ind1, ValIndex ind2, ...) const;

      /**
       * Access coefficient using scalar index.
       * This is equivalent to DiscreteFunction::operator()(maxsum::ValIndex ind)
       * but is more convenient when for use with pointers e.g.
       * <pre>
       * x->(k); // illegal
       * x->at(k); // ok
       * </pre>
       * @see DiscreteFunction::operator()(maxsum::ValIndex ind)
       */
      ValType& at(ValIndex ind)
      {
         return (*this)(ind);
      }

      /**
       * Access coefficient using scalar index.
       * This is equivalent to DiscreteFunction::operator()(maxsum::ValIndex ind) const
       * but is more convenient when for use with pointers e.g.
       * <pre>
       * x->(k); // illegal
       * x->at(k); // ok
       * </pre>
       * @see DiscreteFunction::operator()(maxsum::ValIndex ind) const
       */
      const ValType& at(ValIndex ind) const
      {
         // throw away const to use the non-const implementation
         DiscreteFunction* me = const_cast<DiscreteFunction*>(this);
         return (*me)(ind);
      }

      /**
       * Access coefficient using subindices specified as argumments
       * This is equivalent to DiscreteFunction::operator()
       * but is more convenient when for use with pointers e.g.
       * <pre>
       * x->(k); // illegal
       * x->at(k); // ok
       * </pre>
       * @see DiscreteFunction::operator()
       */
      ValType& at(ValIndex ind1, ValIndex ind2, ...);

      /**
       * Access coefficient using subindices specified as argumments
       * This is equivalent to DiscreteFunction::operator()
       * but is more convenient when for use with pointers e.g.
       * <pre>
       * x->(k); // illegal
       * x->at(k); // ok
       * </pre>
       * @see DiscreteFunction::operator()
       */
      const ValType& at(ValIndex ind1, ValIndex ind2, ...) const;

      /**
       * Access coefficient by subindices
       */
      template<class IndIt> ValType& operator()(IndIt begin, IndIt end)
      {
         ValIndex ind = sub2ind(size_i.begin(),size_i.end(),begin,end);
         return values_i[ind];
      }

      /**
       * Access coefficient by subindices
       */
      template<class IndIt>
         const ValType& operator()(IndIt begin, IndIt end) const
      {
         // throw away const to use the non-const implementation
         DiscreteFunction* me = const_cast<DiscreteFunction*>(this);
         return (*me)(begin,end);
      }
      
      /**
       * Access coefficient by subindices for specified variables.
       * Specified variables must be superset of variables on which this
       * function depends.
       */
      template<class VarIt, class IndIt> ValType& operator()
         (VarIt varBegin, VarIt varEnd, IndIt subBegin, IndIt subEnd)
      {
         //*********************************************************************
         // This function turns out to be crucial for efficiency, and therefore
         // has to be optimised as much as possible. The original implementation
         // first buffered required indices in a dynamically allocated vector,
         // and then called sub2ind. To avoid the substantial overhead of
         // this memory allocation, we now instead perform the sub2ind
         // operations inline, except that we now need to ignore any indices
         // that this function does not actually depend on.
         //
         // To begin, we start by setting up the result variable, and the
         // skipSize - which is the amount we need to increment by given the
         // size and position of the current subindex.
         //*********************************************************************
         ValIndex skipSize = 1;
         ValIndex index = 0;
         
         //*********************************************************************
         // Validate list sizes as far as possible
         //*********************************************************************
         assert( (varEnd-varBegin) == (subEnd-subBegin) );
         assert( vars_i.size() == size_i.size() );
         assert( (varEnd-varBegin) >= vars_i.size() );
         
         //*********************************************************************
         // Now we need iterators for the input variables and indices,
         // and this variables own variables and their domain sizes.
         //*********************************************************************
         VarIt inV = varBegin;
         IndIt sub = subBegin;
         VarVec::const_iterator myV = vars_i.begin();
         SizeVec::const_iterator siz = size_i.begin();
         
         //*********************************************************************
         // Now we iterate through the input variables and indices and perform
         // exactly the same operations as the sub2ind function. The difference
         // here is that we need to ignore any variables that our not in this
         // function's domain.
         //
         // Notice this code assumes that all lists are sorted, allowing us to
         // to check for variable equality by iterating through each list in
         // strict order
         //*********************************************************************         
         while( (myV!=vars_i.end()) && (inV!=varEnd) )
         {
            //******************************************************************
            // Skip indices for variables that are not in the domain of this
            // function.
            //******************************************************************
            if(*myV!=*inV)
            {
               ++sub;
               ++inV;
               continue;
            }
            
            //******************************************************************
            // Ensure that index is in range
            //******************************************************************
            assert((0<=*sub) && (*siz>*sub));

            //******************************************************************
            // Increment the result by the amount required by the current
            // subindex
            //******************************************************************
            index += (*sub) * skipSize;
            skipSize *= *siz;
            
            //******************************************************************
            // Move iterators on to the next variable in the list
            //******************************************************************
            ++siz;
            ++sub;
            ++inV;
            ++myV;
            
         } // while loop
         
         //*********************************************************************
         // Sanity check that we found all the variables in this functions
         // domain, and return the resulting linear index.
         //*********************************************************************
         assert(myV==vars_i.end());
         assert(siz==size_i.end());
         assert(index<values_i.size());
         return values_i[index];
         
      } // operator()

      /**
       * Access coefficient by subindices for specified variables.
       * Specified variables must be superset of variables on which this
       * function depends.
       */
      template<class VarIt, class IndIt> const ValType& operator() 
         (VarIt varBegin, VarIt varEnd, IndIt indBegin, IndIt indEnd) const
      {
         // throw away const to use the non-const implementation
         DiscreteFunction* me = const_cast<DiscreteFunction*>(this);
         return (*me)(varBegin,varEnd,indBegin,indEnd);
      }

      /**
       * Access coefficient by subindices for specified variables.
       * Specified variables must be superset of variables on which this
       * function depends.
       */
      template<class VarVec, class IndVec> ValType& operator()
         (const VarVec& var, const IndVec& ind)
      {
         return (*this)(var.begin(),var.end(),ind.begin(),ind.end());
      }

      /**
       * Access coefficient by subindices for specified variables.
       * Specified variables must be superset of variables on which this
       * function depends.
       */
      template<class VarVec, class IndVec> const ValType& operator()
         (const VarVec& var, const IndVec& ind) const
      {
         // throw away const to use the non-const implementation
         DiscreteFunction* me = const_cast<DiscreteFunction*>(this);
         return (*me)(var, ind);
      }

      /**
       * Get corresponding linear index for subindices specified in map.
       * Specified variables must be superset of variables on which this
       * function depends. The specified map must be sorted.
       * @param vals map of VarID (variables) to ValIndex (values)
       * @pre iterators over vals must return pairs in ascending key order.
       */
      template<class VarMap> ValIndex getIndex(const VarMap& vals) const
      {
         //*********************************************************************
         // Deal with the special case that this function is just a constant
         // with a single value - not depending on any variables
         //*********************************************************************
         if(0==noVars())
         {
             return 0;
         }
         
         //*********************************************************************
         // This function turns out to be crucial for efficiency, and therefore
         // has to be optimised as much as possible. The original implementation
         // first buffered required indices in a dynamically allocated vector,
         // and then called sub2ind. To avoid the substantial overhead of
         // this memory allocation, we now instead perform the sub2ind
         // operations inline, except that we now need to ignore any indices
         // that this function does not actually depend on.
         //
         // To begin, we start by setting up the result variable, and the
         // skipSize - which is the amount we need to increment by given the
         // size and position of the current subindex.
         //*********************************************************************
         ValIndex skipSize = 1;
         ValIndex index = 0;
         
         //*********************************************************************
         // Validate list sizes as far as possible
         //*********************************************************************
         assert( vars_i.size() == size_i.size() );
         assert( vals.size() >= vars_i.size() );
         
         //*********************************************************************
         // Now we need iterators for the input variables and indices,
         // and this variables own variables and their domain sizes.
         //*********************************************************************
         typename VarMap::const_iterator inV = vals.begin();
         VarVec::const_iterator myV = vars_i.begin();
         SizeVec::const_iterator siz = size_i.begin();
         
         //*********************************************************************
         // Now we iterate through the input variables and indices and perform
         // exactly the same operations as the sub2ind function. The difference
         // here is that we need to ignore any variables that our not in this
         // function's domain.
         //
         // Notice this code assumes that all lists are sorted, allowing us to
         // to check for variable equality by iterating through each list in
         // strict order
         //*********************************************************************
         while( (myV!=vars_i.end()) && (inV!=vals.end()) )
         {
            //******************************************************************
            // Skip indices for variables that are not in the domain of this
            // function.
            //******************************************************************
            if(*myV!=inV->first)
            {
               ++inV;
               continue;
            }
            
            //******************************************************************
            // Ensure that index is in range
            //******************************************************************
            assert((0<=inV->second) && (*siz>inV->second));
            
            //******************************************************************
            // Increment the result by the amount required by the current
            // subindex
            //******************************************************************
            index += (inV->second) * skipSize;
            skipSize *= *siz;
            
            //******************************************************************
            // Move iterators on to the next variable in the list
            //******************************************************************
            ++siz;
            ++inV;
            ++myV;
            
         } // while loop
         
         //*********************************************************************
         // Sanity check that we found all the variables in this functions
         // domain, and return the resulting linear index.
         //*********************************************************************
         assert(myV==vars_i.end());
         assert(siz==size_i.end());
         assert(index<values_i.size());
         return index;

      } // getIndex()
      
      /**
       * Access coefficient by subindices specified in variable map.
       * Specified variables must be superset of variables on which this
       * function depends. The specified map must be sorted.
       * @param vals map of VarID (variables) to ValIndex (values)
       * @pre iterators over vals must return pairs in ascending key order.
       */
      template<class VarMap> ValType& operator()(const VarMap& vals)
      {
         return values_i[getIndex(vals)];
      }

      /**
       * Access coefficient by subindices specified in variable map.
       * Specified variables must be superset of variables on which this
       * function depends. The specified map must be sorted.
       * @param vals map of VarID (variables) to ValIndex (values)
       * @pre iterators over vals must return pairs in ascending key order.
       */
      template<class VarMap> const ValType& operator()(const VarMap& vals) const
      {
         // throw away const to use the non-const implementation
         DiscreteFunction* me = const_cast<DiscreteFunction*>(this);
         return (*me)(vals);
      }

      /**
       * Access coefficient by subindices specified by the current indices
       * specified by a maxsum::DomainIterator. Note: this works by accessing
       * maxsum::DomainIterator::getSubInd() array. If the domain of the
       * maxsum::DomainIterator exactly matches the domain of this function,
       * then it is more efficient to index using the corresponding linear
       * index instead. That is
       * <pre>
       * maxsum::DiscreteFunction f(...); // some function
       * maxsum::DomainIterator it(f); // iterator over domain of f
       * maxsum::ValType x = f(it); // this works
       * maxsum::ValType x = f(it.getInd()); // more efficient in this case.
       * </pre>
       * @param[in] it Iterator used to index this function.
       * @see maxsum::DomainIterator::getSubInd()
       * @see maxsum::DomainIterator::getInd()
       */
      ValType& operator()(const DomainIterator& it);

      /**
       * Access coefficient by subindices specified by the current indices
       * specified by a maxsum::DomainIterator. Note: this works by accessing
       * maxsum::DomainIterator::getSubInd() array. If the domain of the
       * maxsum::DomainIterator exactly matches the domain of this function,
       * then it is more efficient to index using the corresponding linear
       * index instead. That is
       * <pre>
       * maxsum::DiscreteFunction f(...); // some function
       * maxsum::DomainIterator it(f); // iterator over domain of f
       * maxsum::ValType x = f(it); // this works
       * maxsum::ValType x = f(it.getInd()); // more efficient in this case.
       * </pre>
       * @param[in] it Iterator used to index this function.
       * @see maxsum::DomainIterator::getSubInd()
       * @see maxsum::DomainIterator::getInd()
       */
      const ValType& operator()(const DomainIterator& it) const;

      /**
       * Swaps the value and domain of this function with another.
       * After the call to this member function, the elements in this function
       * are those which were in fun before the call, and the elements of fun
       * are those which were in this. 
       * @param fun Another DiscreteFunction whose value and domain is swapped
       * with that of this one.
       */
      void swap(DiscreteFunction& fun);

      /**
       * Make this function depend on additional variables.
       * If necessary, the domain of this function is expanded to include the
       * specified list of variables.
       * @param[in] begin iterator to first variable to add
       * @param[in] end iterator to end of list of variables.
       */
      template<class VarInd> void expand(VarInd begin, VarInd end)
      {
         //*********************************************************************
         // Construct the union of the specified variables with this
         // functions current domain
         //*********************************************************************
         std::vector<VarID> newVar = vars_i;
         int maxSize = (end - begin) + vars_i.size();
         newVar.reserve(maxSize);
         newVar.insert(newVar.end(),begin,end);
         std::sort(newVar.begin(),newVar.end());

         std::vector<VarID>::iterator newEnd =
            std::unique(newVar.begin(),newVar.end());

         newVar.resize(newEnd-newVar.begin());

         //*********************************************************************
         // If the specified list is a subset of the current domain, then
         // we're done.
         //*********************************************************************
         if(newVar.size() <= vars_i.size())
         {
            return;
         }

         //*********************************************************************
         // Otherwise, create a temporary function to hold this functions new
         // value.
         //*********************************************************************
         DiscreteFunction result(newVar.begin(),newVar.end());

         //*********************************************************************
         // Copy old values to new values across the expanded domain
         //*********************************************************************
         for(DomainIterator k(result); k.hasNext(); ++k)
         { 
            result(k.getInd()) = (*this)(newVar,k.getSubInd());
         }

         //*********************************************************************
         // Assign the new values to this one.
         //*********************************************************************
         result.swap(*this);

      } // expand method

      /**
       * Expand the domain of this function to include a named variable.
       * @param[in] var the id of the variable to add to this function's domain.
       * @post The domain of this function is the union of its previous domain,
       * and the additional specified variable.
       */
      void expand(const VarID var);

      /**
       * Make the domain of this function include the domain of another.
       * If necessary, the domain of this function is expanded to include the
       * domain of the parameter fun.
       * @param[in] fun function whose domain we want to expand to.
       * @todo Make this more efficient by using fun.size_i cache.
       * @post domain of this is union of its previous domain, with that of fun.
       */
      void expand(const DiscreteFunction& fun);

      /**
       * Condition function on specified variable values.
       * Changes this function so that it does not depend on any of the
       * variables in the list specified by varBegin and varEnd, by
       * conditioning this variables on a corresponding list of values.
       * @param[in] vBegin iterator to start of variable list.
       * @param[in] vEnd iterator to end of variable list.
       * @param[in] iBegin iterator to start of value list.
       * @param[in] iEnd iterator to end of value list.
       * @pre parameters must be iterators over \em sorted lists.
       * @post After calling this method, this DiscreteFunction will not
       * depend on any of the variables in the list specified by varBegin
       * and varEnd.
       */
      template<class VarIt, class IndIt> void condition
      (
       VarIt vBegin,
       VarIt vEnd,
       IndIt iBegin,
       IndIt iEnd
      )
      {
         //*********************************************************************
         // Construct an iterator for this function's domain, and condition
         // on the specified variables.
         //*********************************************************************
         DomainIterator it(*this);
         it.condition(vBegin,vEnd,iBegin,iEnd);

         //*********************************************************************
         // If there are no variables to condition on (i.e. the intersection
         // of the input variables with this variables domain is empty) then
         // there is nothing left to do.
         //*********************************************************************
         if(0==it.fixedCount())
         {
            return;
         }

         //*********************************************************************
         // Otherwise construct the reduced domain of free variables.
         //*********************************************************************
         std::vector<VarID> freeVars;
         freeVars.reserve(vars_i.size());
         for(VarIterator varIt=varBegin(); varIt != varEnd(); ++varIt)
         {
            if(!it.isFixed(*varIt))
            {
               freeVars.push_back(*varIt);
            }
         }

         //*********************************************************************
         // Create a temporary function to hold the result, and copy in the
         // conditioned values.
         //*********************************************************************
         DiscreteFunction result(freeVars.begin(),freeVars.end());
         while(it.hasNext())
         {
            result(it.getVars(),it.getSubInd()) = this->at(it.getInd());
            ++it;
         }

         //*********************************************************************
         // Finally, swap the result values into this function
         //*********************************************************************
         result.swap(*this);

      } // condition method
      
      /**
       * Returns the element-wise maximum of this function and a specified
       * scalar. That is, if M = N.max(s) then M(k)=max(N(k),s).
       * @param[in] s the scalar value to compare
       * @param[out] result the result of the operation.
       */
      void max(const ValType s, DiscreteFunction& result);

      /**
       * Returns the minimum scalar value of the function across entire domain.
       */
      ValType min() const;

      /**
       * Returns the maximum scalar value for function across entire domain.
       */
      ValType max() const; 

      /**
       * Returns the linear index of the maximum value across entire domain.
       */
      ValIndex argmax() const;

      /**
       * Returns the linear index of the 2nd largest value.
       * Example usage:
       * <p>
       * <code>
       * ValIndex mx1 = fun.argmax();
       * ValIndex mx2 = fun.argmax2(mx1);
       * </code>
       * </p>
       * @param mxInd the value returned by DiscreteFunction::argmax()
       * This is the maximum of the set of values, excluding the largest one,
       * returned by DiscreteFunction::argmax
       */
      ValIndex argmax2(ValIndex mxInd) const;

      /**
       * Returns the maxnorm for this function.
       * The maxnorm is defined as the maximum absolute value of the function:
       * \f[
       * \|f(k)\|_{\infty} = \max_k |f(k)|
       * \f]
       */
      ValType maxnorm() const;

      /**
       * Returns the mean scalar value for function across entire domain.
       */
      ValType mean() const;

   }; // class DiscreteFunction

   /**
    * Pretty prints a maxsum::DiscreteFunction
    * Format is similar to the disp function in Matlab for N-D arrays, except
    * that first dimension appears in rows rather than columns.
    */
   std::ostream& operator<<(std::ostream& out, const DiscreteFunction& fun);

   /**
    * Check that two maxsum::DiscreteFunction objects have the same domain.
    * Two functions have the same domain, if they depend on the same set of
    * variables.
    * @param[in] f1 First function to compare
    * @param[in] f2 Second functions to compare
    * @returns true if both function have the same domain.
    */
   bool sameDomain(const DiscreteFunction& f1, const DiscreteFunction& f2);

   /**
    * Check that two maxsum::DiscreteFunction objects are equal within a
    * specified tolerance. This function returns true if and only if, for all
    * <code>k</code>:
    * <p>
    * <code>
    * -tol < 1-f1(k)/f2(k) < tol
    * </code>
    * </p>
    * @param[in] f1 First function to compare
    * @param[in] f2 Second function to compare
    * @param[in] tol tolerance used for comparing values
    * @see maxsum::DEFAULT_VALUE_TOLERANCE
    */
   bool equalWithinTolerance
   (
    const DiscreteFunction& f1, 
    const DiscreteFunction& f2, 
    ValType tol=DEFAULT_VALUE_TOLERANCE
   );

   /**
    * Check that two maxsum::DiscreteFunction objects are equal with a
    * specified tolerance, and have exactly the same domain.
    * This function returns true if and only if:
    * <p>
    * <code>
    * true == sameDomain(f1,f2) && equalWithinTolerance(f1,f2,tol)
    * </code>
    * </p>
    * @param[in] f1 First function to compare
    * @param[in] f2 Second function to compare
    * @param[in] tol tolerance used for comparing values
    * @see maxsum::DEFAULT_VALUE_TOLERANCE
    */
   bool strictlyEqualWithinTolerance
   (
    const DiscreteFunction& f1, 
    const DiscreteFunction& f2, 
    ValType tol=DEFAULT_VALUE_TOLERANCE
   );

   /**
    * Return true if functions are equal.
    * Two functions are equal if they have the same value over the cartesian
    * product of their domains. 
    */
   inline bool operator==
   (
    const DiscreteFunction& f1, 
    const DiscreteFunction& f2 
   )
   {
      return equalWithinTolerance(f1,f2,0.0);
   }

   /**
    * Return true if functions are equal.
    * Two functions are equal if they have the same value over the cartesian
    * product of their domains.
    */
   inline bool operator!=
   (
    const DiscreteFunction& f1, 
    const DiscreteFunction& f2 
   )
   {
      return !equalWithinTolerance(f1,f2,0.0);
   }
   
   /**
    * Returns true iff function is less than specified value
    * across its entire domain.
    */
   inline bool operator<(const DiscreteFunction& f, const ValType v)
   {
      for(int k=0; k<f.domainSize(); ++k)
      {
         if(f(k)>=v)
         {
            return false;
         }
      }
      return true;
   }
   
   /**
    * Returns true iff function is less than specified value
    * across its entire domain.
    */
   inline bool operator>=(const ValType v, const DiscreteFunction& f)
   {
      return f < v;
   }
   
   /**
    * Returns true iff function is less than or equal to specified value
    * across its entire domain.
    */
   inline bool operator<=(const DiscreteFunction& f, const ValType v)
   {
      for(int k=0; k<f.domainSize(); ++k)
      {
         if(f(k)>v)
         {
            return false;
         }
      }
      return true;
   }
   
   /**
    * Returns true iff function is less than or equal to specified value
    * across its entire domain.
    */
   inline bool operator>(const ValType v, const DiscreteFunction& f)
   {
      return f <= v;
   }
   
   /**
    * Returns true iff function is greater than specified value
    * across its entire domain.
    */
   inline bool operator>(const DiscreteFunction& f, const ValType v)
   {
      for(int k=0; k<f.domainSize(); ++k)
      {
         if(f(k)<=v)
         {
            return false;
         }
      }
      return true;
   }
   
   /**
    * Returns true iff function is greater than specified value
    * across its entire domain.
    */
   inline bool operator<=(const ValType v, const DiscreteFunction& f)
   {
      return f > v;
   }
   
   /**
    * Returns true iff function is greater than or equal to specified value
    * across its entire domain.
    */
   inline bool operator>=(const DiscreteFunction& f, const ValType v)
   {
      for(int k=0; k<f.domainSize(); ++k)
      {
         if(f(k)<v)
         {
            return false;
         }
      }
      return true;
   }
   
   /**
    * Returns true iff function is greater than or equal to specified value
    * across its entire domain.
    */
   inline bool operator<(const ValType v, const DiscreteFunction& f)
   {
      return f >= v;
   }

   /**
    * Peforms element-wise division of a scalar by a function.
    */
   inline DiscreteFunction operator/
   (
    const ValType f1, 
    const DiscreteFunction& f2 
   )
   {
      return DiscreteFunction(f1) / f2;
   }

   /**
    * Peforms element-wise multiplication of a scalar by a function.
    */
   inline DiscreteFunction operator*
   (
    const ValType f1, 
    const DiscreteFunction& f2 
   )
   {
      return f2 * f1;
   }

   /**
    * Peforms element-wise addition of a scalar by a function.
    */
   inline DiscreteFunction operator+
   (
    const ValType f1, 
    const DiscreteFunction& f2 
   )
   {
      return f2 + f1;
   }

   /**
    * Peforms element-wise subtraction of a function from a scalar.
    */
   inline DiscreteFunction operator-
   (
    const ValType f1, 
    const DiscreteFunction& f2 
   )
   {
      return DiscreteFunction(f1) - f2;
   }

   /**
    * Condition function on specified variable values.
    * Changes a function so that it does not depend on any of the
    * variables in the list specified by varBegin and varEnd, by
    * conditioning these variables on a corresponding list of values.
    * @param[in] inFun the function to condition
    * @param[out] outFun function in which to store result.
    * @param[in] vBegin iterator to start of variable list.
    * @param[in] vEnd iterator to end of variable list.
    * @param[in] iBegin iterator to start of value list.
    * @param[in] iEnd iterator to end of value list.
    * @pre parameters must be iterators over \em sorted lists.
    * @post Previous value of condition will be overwritten, and replaced
    * with the conditioned values from inFun.
    */
   template<class VarIt, class IndIt> void condition
   (
    const DiscreteFunction& inFun,
    DiscreteFunction& outFun,
    VarIt vBegin,
    VarIt vEnd,
    IndIt iBegin,
    IndIt iEnd
   )
   {
      //*********************************************************************
      // Construct an iterator over inFun's domain, and condition
      // on the specified variables.
      //*********************************************************************
      DomainIterator it(inFun);
      it.condition(vBegin,vEnd,iBegin,iEnd);

      //*********************************************************************
      // If there are no variables to condition on (i.e. the intersection
      // of the input variables with this function domain is empty) then
      // simply copy the input function to the output
      //*********************************************************************
      if(0==it.fixedCount())
      {
         outFun = inFun;
         return;
      }

      //*********************************************************************
      // Otherwise construct the reduced domain of free variables.
      //*********************************************************************
      std::vector<VarID> freeVars;
      freeVars.reserve(inFun.noVars());
      for(DiscreteFunction::VarIterator varIt=inFun.varBegin();
            varIt != inFun.varEnd(); ++varIt)
      {
         if(!it.isFixed(*varIt))
         {
            freeVars.push_back(*varIt);
         }
      }

      //*********************************************************************
      // Create a temporary function to hold the result, and copy in the
      // conditioned values.
      //*********************************************************************
      DiscreteFunction result(freeVars.begin(),freeVars.end());
      while(it.hasNext())
      {
         result(it.getVars(),it.getSubInd()) = inFun(it.getInd());
         ++it;
      }

      //*********************************************************************
      // Finally, swap the result values into the output function
      //*********************************************************************
      result.swap(outFun);

   } // condition 

   /**
    * Condition function on specified variable values.
    * Changes a function so that it does not depend on any of the
    * variables in the key set of the vars argument, by
    * conditioning these variables on the values specified in vars.
    * @param[in] inFun the function to condition
    * @param[out] outFun function in which to store result.
    * @param[in] vars maps conditioned vars to their assigned values.
    * @post Previous value of condition will be overwritten, and replaced
    * with the conditioned values from inFun.
    */
   template<class VarMap> void condition
   (
    const DiscreteFunction& inFun,
    DiscreteFunction& outFun,
    VarMap vars
   )
   {
      //*********************************************************************
      // Construct an iterator over inFun's domain, and condition
      // on the specified variables.
      //*********************************************************************
      DomainIterator it(inFun);
      it.condition(vars);

      //*********************************************************************
      // If there are no variables to condition on (i.e. the intersection
      // of the input variables with this function domain is empty) then
      // simply copy the input function to the output
      //*********************************************************************
      if(0==it.fixedCount())
      {
         outFun = inFun;
         return;
      }

      //*********************************************************************
      // Otherwise construct the reduced domain of free variables.
      //*********************************************************************
      std::vector<VarID> freeVars;
      freeVars.reserve(inFun.noVars());
      for(DiscreteFunction::VarIterator varIt=inFun.varBegin();
            varIt != inFun.varEnd(); ++varIt)
      {
         if(!it.isFixed(*varIt))
         {
            freeVars.push_back(*varIt);
         }
      }

      //*********************************************************************
      // Create a temporary function to hold the result, and copy in the
      // conditioned values.
      //*********************************************************************
      DiscreteFunction result(freeVars.begin(),freeVars.end());
      while(it.hasNext())
      {
         result(it.getVars(),it.getSubInd()) = inFun(it.getInd());
         ++it;
      }

      //*********************************************************************
      // Finally, swap the result values into the output function
      //*********************************************************************
      result.swap(outFun);

   } // condition 

   /**
    * Marginalise a maxsum::DiscreteFunction using a specified aggregation
    * function. This function reduces the domain of <code>inFun</code> to that
    * of <code>outFun</code> and stores the result in <code>outFun</code>.
    * This is done by aggregating over all variables that are in the domain of
    * <code>inFun</code>, but not in the domain of <code>outFun</code>.
    * <p>Aggregation is performed by a functor or function pointer,
    * <code>aggregate</code>, which is passed as a template parameter.
    * <code>aggregate</code> should be defined such that, in the code below,
    * <code>z</code> is the aggregation of the function values <code>x</code>
    * and <code>y</code>.</p>
    * <pre>
    * ValType x = inFun(k);
    * ValType y = inFun(t);
    * ValType z = aggregate(x,y); // result = aggregate(prevResult,nextVal)
    * </pre>
    * <p>Note that aggregate may be a function pointer, or any other object for
    * which the above syntax is valid, such as a class instance that overloads
    * the () operator, which may depend some internal state. The first argument
    * to aggregate should always be the previous estimate, while the second
    * is always the next value to be aggregated.</p>
    * <p>Various specialisations of this function are provided by
    * maxsum::maxMarginal(), maxsum::minMarginal(),
    * maxsum::meanMarginal().</p>
    * <p>In the special case where the domain of outFun is equal to the
    * domain of inFun, outFun becomes a copy of inFun</p>
    * @pre aggregate is a functor with signature
    * <code>ValType aggregate(ValType prevResult, ValType nextVal)</code>
    * @pre variables in domain of outFun are a subset of variables in inFun.
    * @post previous content of outFun is overwritten.
    * @post The domains of outFun and inFun remain unchanged.
    * @tparam F type of parameter aggregate.
    * @param[in] inFun function to marginalise
    * @param[in] aggregate functor or function pointer used to aggregate
    * results.
    * @param[out] outFun maxsum::DiscreteFunction in which to store result.
    * @throws maxsum::BadDomainException is the domain of outFun is not a
    * subset of inFun.
    * @see maxsum::maxMarginal()
    * @see maxsum::minMarginal()
    * @see maxsum::meanMarginal()
    */
   template<typename F> void marginal
   (
    const DiscreteFunction& inFun,
    F aggregate,
    DiscreteFunction& outFun
   )
   {
      //************************************************************************
      // Ensure that the domain of outFun is a subset of inFun
      // We throw an exception if this is not the case.
      //************************************************************************
      if(!std::includes(inFun.varBegin(),inFun.varEnd(),
               outFun.varBegin(),outFun.varEnd()))
      {
         throw BadDomainException("marginal(DiscreteFunction,DiscreteFunction)",
               "Out domain is not subset of in domain.");
      }

      //************************************************************************
      // Calculate and store the result for each value in the output function.
      // Notice, we need to get the first element outside the loop, so that we
      // have at least two values with which to call the aggregate function.
      // If there is only one value, then that becomes its own aggregate.
      //************************************************************************
      for(DomainIterator outIt(outFun); outIt.hasNext(); ++outIt)
      {
         //*********************************************************************
         // Initialise the result to the first corresponding value in the input
         // function. This ensures that things like max and min will work
         // properly as aggregate functions, because they already have
         // something to work with.
         //*********************************************************************
         DomainIterator inIt(inFun);
         inIt.condition(outIt);
         ValType result = inFun(inIt.getInd());
         ++inIt;

         //*********************************************************************
         // Update the result by aggregating with any remaining values in the
         // free part of the input function domain.
         //*********************************************************************
         while(inIt.hasNext())
         {
            result = aggregate(result,inFun(inIt.getInd()));
            ++inIt;
         }

         //*********************************************************************
         // Store the current aggregate value in the current position of the
         // output function.
         //*********************************************************************
         outFun(outIt.getInd()) = result;

      } // for loop

   } // function marginal

   /**
    * Marginalise a maxsum::DiscreteFunction by maximisation.
    * This function reduces the domain of inFun to that of outFun by
    * maximisation, and stores the result in outFun. This behaviour is
    * equivalent to maxsum::marginal(inFun,std::max<ValType>,outFun).
    * @pre variables in domain of outFun are a subset of variables in inFun.
    * @post previous content of outFun is overwritten.
    * @post The domains of outFun and inFun remain unchanged.
    * @param[in] inFun function to marginalise
    * @param[out] outFun maxsum::DiscreteFunction in which to store result.
    * @throws maxsum::BadDomainException is the domain of outFun is not a
    * subset of inFun.
    * @see maxsum::marginal()
    * @see maxsum::minMarginal()
    * @see maxsum::meanMarginal()
    */
   void maxMarginal(const DiscreteFunction& inFun, DiscreteFunction& outFun);

   /**
    * Marginalise a maxsum::DiscreteFunction by minimisation.
    * This function reduces the domain of inFun to that of outFun by
    * minimisation, and stores the result in outFun. This behaviour is
    * equivalent to maxsum::marginal(inFun,std::min<ValType>,outFun).
    * @pre variables in domain of outFun are a subset of variables in inFun.
    * @post previous content of outFun is overwritten.
    * @post The domains of outFun and inFun remain unchanged.
    * @param[in] inFun function to marginalise
    * @param[out] outFun maxsum::DiscreteFunction in which to store result.
    * @throws maxsum::BadDomainException is the domain of outFun is not a
    * subset of inFun.
    * @see maxsum::marginal()
    * @see maxsum::maxMarginal()
    * @see maxsum::meanMarginal()
    */
   void minMarginal(const DiscreteFunction& inFun, DiscreteFunction& outFun);

   /**
    * Marginalise a maxsum::DiscreteFunction by averaging.
    * This function reduces the domain of inFun to that of outFun by
    * averaging, and stores the result in outFun. 
    * @pre variables in domain of outFun are a subset of variables in inFun.
    * @post previous content of outFun is overwritten.
    * @post The domains of outFun and inFun remain unchanged.
    * @param[in] inFun function to marginalise
    * @param[out] outFun maxsum::DiscreteFunction in which to store result.
    * @throws maxsum::BadDomainException is the domain of outFun is not a
    * subset of inFun.
    * @see maxsum::marginal()
    * @see maxsum::minMarginal()
    * @see maxsum::maxMarginal()
    */
   void meanMarginal(const DiscreteFunction& inFun, DiscreteFunction& outFun);

   /**
    * Function type for operations applied to function values.
    */
   typedef ValType(*UnaryScalarOp)(const ValType);

   /**
    * Function type for operations applied to two functions.
    */
   typedef ValType(*DualScalarOp)(const ValType, const ValType);

   /**
    * Template used to generate operations that apply some function to each
    * of a DiscreteFunction's values.
    */
   template<UnaryScalarOp OP> DiscreteFunction elementWiseOp
   (
    const DiscreteFunction& inFcn
   )
   {
      //************************************************************************
      // Create a function to hold the result
      //************************************************************************
      DiscreteFunction result(inFcn.varBegin(),inFcn.varEnd());

      //************************************************************************
      // Apply the specified operation elementwise to the input function
      //************************************************************************
      for(int k=0; k<inFcn.domainSize(); ++k)
      {
         result(k) = OP(inFcn(k));
      }

      return result;

   } // elementWiseOp

   /**
    * Template used to generate operations that apply some operation to a pair
    * of DiscreteFunctions.
    */
   template<DualScalarOp OP> DiscreteFunction elementWiseOp
   (
    const DiscreteFunction& inFcn1,
    const DiscreteFunction& inFcn2
   )
   {
      //************************************************************************
      // Create a function to hold the result
      //************************************************************************
      DiscreteFunction result(inFcn1.varBegin(),inFcn1.varEnd());
      result.expand(inFcn2);

      //************************************************************************
      // Apply the specified operation elementwise to the input function
      //************************************************************************
      for(DomainIterator it(result); it.hasNext(); ++it)
      {
         result(it.getInd()) = OP(inFcn1(it),inFcn2(it));
      }

      return result;

   } // elementWiseOp

} // namespace maxsum

namespace std
{

   /**
    * Defines numeric limits for maxsum::DiscreteFunction class.
    * The numeric limits for DiscreteFunctions are inherited from their
    * underlying value type.
    */
   template<> class numeric_limits<maxsum::DiscreteFunction>
      : public numeric_limits<maxsum::ValType> {};

   /**
    * Returns the elementwise log of a function.
    */
   inline maxsum::DiscreteFunction log(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<log>(fcn);
   }

   /**
    * Returns the elementwise cosine value of a function.
    */
   inline maxsum::DiscreteFunction cos(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<cos>(fcn);
   }

   /**
    * Returns the elementwise sine value of a function.
    */
   inline maxsum::DiscreteFunction sin(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<sin>(fcn);
   }

   /**
    * Returns the elementwise tangent value of a function.
    */
   inline maxsum::DiscreteFunction tan(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<tan>(fcn);
   }

   /**
    * Returns the elementwise absolute value of a function.
    */
   inline maxsum::DiscreteFunction fabs(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<fabs>(fcn);
   }

   /**
    * Returns the elementwise absolute value of a function.
    */
   inline maxsum::DiscreteFunction abs(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<abs>(fcn);
   }

   /**
    * Returns the elementwise exponent of a function.
    */
   inline maxsum::DiscreteFunction exp(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<exp>(fcn);
   }

   /**
    * Returns the elementwise square root of a function.
    */
   inline maxsum::DiscreteFunction sqrt(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<sqrt>(fcn);
   }

   /**
    * Returns the elementwise ceil value of a function.
    */
   inline maxsum::DiscreteFunction ceil(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<ceil>(fcn);
   }

   /**
    * Returns the elementwise floor value of a function.
    */
   inline maxsum::DiscreteFunction floor(const maxsum::DiscreteFunction& fcn)
   {
      return maxsum::elementWiseOp<floor>(fcn);
   }

   /**
    * Takes one function to the power of another.
    * The result is <code>base</code> raised to the power of <code>exp</code>.
    */
   inline maxsum::DiscreteFunction pow
   (
    const maxsum::DiscreteFunction& base,
    const maxsum::DiscreteFunction& exp 
   )
   {
      return maxsum::elementWiseOp<pow>(base,exp);
   }

} // namespace std


#endif // MAX_SUM_DISCRETE_FUNCTION_H
