#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include <iostream>
#include <vector>
#include <cmath>
#include <stdlib.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_monte.h>
#include <gsl/gsl_monte_plain.h>
#include <gsl/gsl_monte_miser.h>
#include <gsl/gsl_monte_vegas.h>
#include <gsl/gsl_interp.h>

/*! @class Integrator

  @brief Interface class integrator

  This class uses Gnu Scientific Library to integrate a given function.
  Specific integration methods must be implemented inheriting from Integrator.

  The function to be integrated is specified using

    Integrator::setFunction(f);

  where the given function must be of the form:

    void myFun( double *x, size_t dim, void *params );

 */
class Integrator {
public:
   //! main constructor
   inline Integrator();
   //! destructor
   inline virtual ~Integrator();

   /*! @name GSL integrator setup */
   //@{
   //! set function to be integrated
   inline void setFunction( double (* f)(double * x, size_t dim, void * params) );
   //! set function dimension
   inline void setFunctionDim( size_t dim );
   //! set function parameters
   inline void setFunctionParams( void * params );
   //! set integration ranges
   inline void setIntRanges( std::vector<double> & xl, std::vector<double> & xu );
   //! set number of calls
   inline void setNcalls( unsigned int nc );
   //@}
   /*! @name Tabulating */
   //@{
   //! add a parameter to be tabulated
   inline void addTabPar( size_t ind, double xmin, double xmax, double step );
   //! set tabulated parameters given a vector of parameter indecis
   inline void setTabParamsVal( std::vector<int> & tabind );
   //! clear table
   inline void clrTable();
   //! get the tabulated value with the given parameter vector
   inline double getValue( std::vector<double> & tabParams );
   //! initialize table
   inline virtual void initTable();
   //! do the tabulation
   inline virtual void tabulate();
   //@}
   /*! @name Initializing, running and reading results */
   //@{
   //! initialize
   inline virtual void initialize();
   //! integrate using the given parameters
   inline virtual void go( void *params );
   //! get chi2
   inline virtual double chisq()=0;
   //! get result
   inline double result();
   //! get error
   inline double error();
   //@}

protected:
   gsl_rng            *m_gslRange;    /**< GSL range */
   unsigned int        m_ncalls;      /**< number of iterations */
   gsl_monte_function  m_gslMonteFun; /**< structure for GSL MC integration */
   std::vector<double> m_intXL;       /**< lower integration range for each integrand */
   std::vector<double> m_intXU;       /**< idem, upper */

   std::vector<size_t> m_tabIndex;    /**< index of tabulated var */
   std::vector<double> m_tabMin;      /**< minimum of tabulated value */
   std::vector<double> m_tabMax;      /**< maximum */
   std::vector<double> m_tabStep;     /**< step size */
   std::vector<int>    m_tabNsteps;   /**< number of steps */
   std::vector<double> m_tabXvalues;  /**< tabulated X for each parameter */
   std::vector<int>    m_tabXvalInd;  /**< first index in above vector for param [i] */
   std::vector<double> m_tabValues;   /**< the actual table */
   bool                m_tabulated;   /**< true if tabulate() is called successfully */


   double m_result;                   /**< result of integration */
   double m_error;                    /**< error of idem */
};

/*! @class IntegratorVegas

@brief Implements the 'Vegas' algorithm

*/
class IntegratorVegas : public Integrator {
public:
   inline IntegratorVegas();
   inline virtual ~IntegratorVegas();
   inline virtual void go( void *params=0 );
   inline virtual void initialize();
   inline virtual double chisq();
private:
   gsl_monte_vegas_state *m_gslVegasState; /**< GSL vegas state */
};

/*! @class IntegratorPlain

@brief Implements the 'Plain' algorithm

*/
class IntegratorPlain : public Integrator {
public:
   inline IntegratorPlain();
   inline virtual ~IntegratorPlain();
   inline virtual void go( void *params=0 );
   inline virtual void initialize();
   inline virtual double chisq();
private:
   gsl_monte_plain_state *m_gslPlainState; /**< GSL plain state */
};

/*! @class IntegratorMiser

@brief Implements the 'Miser' algorithm

*/
class IntegratorMiser : public Integrator {
public:
   inline IntegratorMiser();
   inline virtual ~IntegratorMiser();
   inline virtual void go( void *params=0 );
   inline virtual void initialize();
   inline virtual double chisq();
private:
   gsl_monte_miser_state *m_gslMiserState; /**< GSL miser state */
};

#include "Integrator.icc"

#endif