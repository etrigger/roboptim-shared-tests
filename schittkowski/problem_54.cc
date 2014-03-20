// Copyright (C) 2014 by Thomas Moulard, AIST, CNRS.
//
// This file is part of the roboptim.
//
// roboptim is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// roboptim is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with roboptim.  If not, see <http://www.gnu.org/licenses/>.

#include "common.hh"

#include <roboptim/core/finite-difference-gradient.hh>

namespace roboptim
{
  namespace schittkowski
  {
    namespace problem54
    {
      struct ExpectedResult
      {
	static const double f0;
	static const double x[];
	static const double fx;
      };
      const double ExpectedResult::f0 = -0.7651;
      const double ExpectedResult::x[] = {
	91600. / 7.,
	79. / 70.,
	2E6,
	10.,
	1E-3,
	1E8
      };
      const double ExpectedResult::fx = -std::exp (-27./280.);

      template <typename T>
      class F : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit F () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const throw ();
      };

      template <typename T>
      F<T>::F () throw ()
	: GenericDifferentiableFunction<T>
	  (6, 1, "-exp(-h(x)/2)")
      {}

      template <typename T>
      void
      F<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	value_type h =
	  (std::pow (x[0] - 1.E6, 2.) / 6.4E7
	   + (x[0] - 1E4) * (x[1] - 1.) / 2.E4
	   + std::pow (x[1] - 1, 2.))
	  * std::pow (x[2] - 2.E6, 2.) / (.96 * 4.9E13)
	  + std::pow (x[3] - 10., 2.) / 2.5E3
	  + std::pow (x[4] - 1.E-3, 2) / 2.5E3
	  + std::pow (x[5] - 1.E8, 2) / 2.5E17;
	result[0] = -std::exp (-h/2.);
      }

      template <typename T>
      void
      F<T>::impl_gradient (gradient_t& grad, const argument_t& x, size_type i)
	const throw ()
      {
	//FIXME:
	GenericFiniteDifferenceGradient<T> fd (*this);
	fd.gradient (grad, x, i);
      }

      template <typename T>
      class G : public GenericDifferentiableFunction<T>
      {
      public:
	ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_
	(GenericDifferentiableFunction<T>);

	explicit G () throw ();
	void
	impl_compute (result_t& result, const argument_t& x) const throw ();
	void
	impl_gradient (gradient_t& grad, const argument_t& x, size_type)
	  const throw ();
      };

      template <typename T>
      G<T>::G () throw ()
	: GenericDifferentiableFunction<T>
	  (6, 1, "x₀ + 4.E3 x₁ - 1.76E4 x₃")
      {}

      template <typename T>
      void
      G<T>::impl_compute (result_t& result, const argument_t& x)
	const throw ()
      {
	result[0] = x[0] + 4.E3 * x[1] - 1.76E4 * x[3];
      }

      template <>
      void
      G<EigenMatrixSparse>::impl_gradient
      (gradient_t& grad, const argument_t&, size_type)
	const throw ()
      {
	grad.insert (0) = 1.;
	grad.insert (1) = 4.E3;
	grad.insert (2) = 0.;
	grad.insert (3) = -1.76E4;
	grad.insert (4) = 0.;
	grad.insert (5) = 0.;
      }

      template <typename T>
      void
      G<T>::impl_gradient (gradient_t& grad, const argument_t&, size_type)
	const throw ()
      {
	grad[0] = 1.;
	grad[1] = 4.E3;
	grad[2] = 0.;
	grad[3] = -1.76E4;
	grad[4] = 0.;
	grad[5] = 0.;
      }
    } // end of namespace problem54.
  } // end of namespace schittkowski.
} // end of namespace roboptim.

BOOST_FIXTURE_TEST_SUITE (schittkowski, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (schittkowski_problem54)
{
  using namespace roboptim;
  using namespace roboptim::schittkowski::problem54;

  // Tolerances for Boost checks.
  double f0_tol = 1e-4;
  double x_tol = 1e-4;
  double f_tol = 1e-4;

  // Build problem.
  F<functionType_t> f;
  solver_t::problem_t problem (f);

  problem.argumentBounds ()[0] = F<functionType_t>::makeInterval (0., 2.E4);
  problem.argumentBounds ()[1] = F<functionType_t>::makeInterval (-10., 10.);
  problem.argumentBounds ()[2] = F<functionType_t>::makeInterval (0., 1.E7);
  problem.argumentBounds ()[3] = F<functionType_t>::makeInterval (0., 20.);
  problem.argumentBounds ()[4] = F<functionType_t>::makeInterval (-1., 1.);
  problem.argumentBounds ()[5] = F<functionType_t>::makeInterval (0., 2.E8);

  boost::shared_ptr<G<functionType_t> > g =
    boost::make_shared<G<functionType_t> > ();
  problem.addConstraint (g, G<functionType_t>::makeInterval (0., 0.));

  F<functionType_t>::argument_t x (6);
  x << 6.E3, 1.5, 4E6, 2., 3E-3, 5E7;
  problem.startingPoint () = x;

  BOOST_CHECK_SMALL_OR_CLOSE (f (x)[0], ExpectedResult::f0, f0_tol);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();
  OptimizationLogger<solver_t> logger
    (solver,
     "/tmp/roboptim-shared-tests/" SOLVER_NAME "/schittkowski/problem-54");

  // Set optional log file for debugging
  SET_LOG_FILE(solver);

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Compute the minimum and retrieve the result.
  solver_t::result_t res = solver.minimum ();

  std::cout << f.inputSize () << std::endl;
  std::cout << problem.function ().inputSize () << std::endl;

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  PROCESS_RESULT();
}

BOOST_AUTO_TEST_SUITE_END ()