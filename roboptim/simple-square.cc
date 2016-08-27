// Copyright (C) 2016 by Benjamin Chrétien, CNRS-AIST JRL.
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

#include <roboptim/core/numeric-quadratic-function.hh>

namespace roboptim
{
  namespace simple_square
  {
    template <typename T>
    struct F : public GenericNumericQuadraticFunction<T>
    {
      ROBOPTIM_DIFFERENTIABLE_FUNCTION_FWD_TYPEDEFS_ (
          GenericNumericQuadraticFunction<T>);

      explicit F ()
        : GenericNumericQuadraticFunction<T> (
              matrix_t (1, 1), vector_t::Zero (1), vector_t::Zero (1))
      {
        initialize ();
      }

      void initialize ();

      ~F () {}
    };

    template <>
    void F<EigenMatrixSparse>::initialize ()
    {
      // Fill matrix A.
      Eigen::MatrixXd denseA (1, 1);
      denseA << 4.;
      denseA *= 0.5;
      this->A () = denseA.sparseView ();

      // Fill vector b.
      this->b () << -3.;

      // Fill c.
      this->c () << 5.;
    }

    template <typename T>
    void F<T>::initialize ()
    {
      // Fill matrix A.
      this->A () << 4.;
      this->A () *= 0.5;

      // Fill vector b.
      this->b () << -3.;

      // Fill c.
      this->c () << 5.;
    }
  } // end of namespace unconstrained
} // end of namespace roboptim

BOOST_FIXTURE_TEST_SUITE (roboptim, TestSuiteConfiguration)

BOOST_AUTO_TEST_CASE (roboptim_simple_square)
{
  using namespace roboptim;
  using namespace roboptim::simple_square;

  // Tolerances for Boost checks.
  double f0_tol = 1e-6;
  double x_tol = 1e-5;
  double f_tol = 1e-4;

  ExpectedResult expectedResult;
  expectedResult.f0 = 257.;
  expectedResult.x = (ExpectedResult::argument_t (1) << 0.75).finished ();
  expectedResult.fx = 3.875;

  // Build cost function.
  boost::shared_ptr<F<functionType_t> > f (new F<functionType_t> ());

  // Build problem.
  solver_t::problem_t problem (f);

  // Load starting point
  F<functionType_t>::argument_t x (1);
  x << 12.;
  problem.startingPoint () = x;

  BOOST_CHECK_SMALL_OR_CLOSE ((*f) (x)[0], expectedResult.f0, f0_tol);

  // Initialize solver.
  SolverFactory<solver_t> factory (SOLVER_NAME, problem);
  solver_t& solver = factory ();

  // Set optimization logger
  SET_OPTIMIZATION_LOGGER (solver, "roboptim/simple-square");

  // Set optional log file for debugging
  SET_LOG_FILE (solver);

  // Compute the minimum and retrieve the result.
  solver_t::result_t res = solver.minimum ();

  // Display solver information.
  std::cout << solver << std::endl;

  // Process the result
  PROCESS_RESULT_UNCONSTRAINED ();
}

BOOST_AUTO_TEST_SUITE_END ()
