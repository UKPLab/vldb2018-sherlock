// -------------------------------------------------------------- -*- C++ -*-
// File: iloadmipex9.cpp
// Version 12.8.0
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
// Copyright IBM Corporation 2000, 2017. All Rights Reserved.
//
// US Government Users Restricted Rights - Use, duplication or
// disclosure restricted by GSA ADP Schedule Contract with
// IBM Corp.
// --------------------------------------------------------------------------
//
// iloadmipex9.cpp -  Inject heuristic solutions from the generic callback
//                    for optimizing an all binary MIP problem
//
// To run this example, command line arguments are required.
// i.e.,   iloadmipex9   filename
//
// Example:
//     iloadmipex9  example.mps
//

#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
#include <math.h>
#include <map>
#include <algorithm>


static void usage (const char *progname);

// This is the class implementing the heuristic callback.
class HeuristicCallback: public IloCplex::Callback::Function {
   public:
      // Constructor with data
      HeuristicCallback(IloCplex cplex, IloNumVarArray _vars) :
         vars(_vars),
         obj(cplex.getEnv(), vars.getSize())
      {
         // Generate the objective as a double array for easy look up
         IloObjective objexpr = cplex.getObjective();
         std::map<IloInt, double> objmap;

         for (IloExpr::LinearIterator it = IloExpr(objexpr.getExpr()).getLinearIterator(); it.ok(); ++it) {
            objmap[it.getVar().getId()] = it.getCoef();
         }

         // Fill into a double array
         IloInt cols = vars.getSize();
         for (IloInt j = 0; j  <  cols; ++j) {
            std::map<IloInt, double>::iterator it = objmap.find(vars[j].getId());
            if ( it != objmap.end() ) {
               obj[j] = it->second;
            }
         }
      }

      void Rounddown (const IloCplex::Callback::Context& context) {
         IloNumArray x(context.getEnv());
         try {
            context.getRelaxationPoint(vars, x);

            // Heuristic motivated by knapsack constrained problems.
            // Rounding down all fractional values will give an integer
            // solution that is feasible, since all constraints are <=
            // with positive coefficients

            double relobj = context.getRelaxationObjective();

            IloInt cols = vars.getSize();
            for (IloInt j = 0; j < cols; j++) {
               // Set the fractional variable to zero
               // Note that we assume all-binary variables. If there are
               // non-binary variables then the update must of course be
               // different.
               if ( x[j] ) {
                  // Set the fractional variables to zero
                  double integral;
                  double frac = modf(x[j], &integral);
                  frac        = (std::min) (1.0-frac,frac);

                  if ( frac > 1.0e-6 ) {
                     relobj -= obj[j]*x[j];
                     x[j]    = 0.0;
                  }
               }
            }

            // Post the rounded solution
            context.postHeuristicSolution(vars, x, relobj,
                                           IloCplex::Callback::Context::SolutionStrategy::CheckFeasible);
            x.end();
         }
         catch (...) {
            x.end();
            throw;
         }
      }

      // This is the function that we have to implement and that CPLEX will call
      // during the solution process at the places that we asked for.
      virtual void invoke (const IloCplex::Callback::Context& context);
   private:
      // Variables
      IloNumVarArray vars;

      // Objective
      IloNumArray obj;
};

// Implementation of the invokeCallback function
void
HeuristicCallback::invoke (const IloCplex::Callback::Context &context)
{
   if ( context.inRelaxation() ) {
      // Call rounding heuristic
      Rounddown (context);
   }
}

int
main (int argc, char **argv)
{
   IloEnv   env;
   try {
      IloModel model(env);
      IloCplex cplex(env);

      if ( argc != 2 ) {
         usage (argv[0]);
         throw(-1);
      }

      IloObjective   obj;
      IloNumVarArray vars(env);
      IloRangeArray  rng(env);
      cplex.importModel(model, argv[1], obj, vars, rng);

      cplex.extract(model);

      // Now we get to setting up the callback.
      // We instanciate a HeuristicCallback and set the wherefrom parameter.
      HeuristicCallback heurCallback(cplex, vars);
      CPXLONG wherefrom = 0;

      wherefrom |= IloCplex::Callback::Context::Id::Relaxation;

      // We add the callback.
      cplex.use(&heurCallback, wherefrom);

      // Switch off regular heuristics to give the callback a chance
      cplex.setParam(IloCplex::Param::MIP::Strategy::HeuristicFreq, -1);

      if ( !cplex.solve() ) {
         cerr << "No solution found! Status = " << cplex.getStatus() << endl;
         throw(-1);
      }

      IloNumArray vals(env);
      cplex.getValues(vals, vars);
      env.out() << "Solution status = " << cplex.getStatus() << endl;
      env.out() << "Solution value  = " << cplex.getObjValue() << endl;
      env.out() << "Values          = " << vals << endl;
   }
   catch (IloException& e) {
      cerr << "Concert exception caught: " << e << endl;
      env.end();
      throw;
   }
   catch (...) {
      cerr << "Unknown exception caught" << endl;
      env.end();
      throw;
   }

   env.end();
   return 0;
}  // END main


static void usage (const char *progname)
{
   cerr << "Usage: " << progname << " filename" << endl;
   cerr << "   where filename is a file with extension " << endl;
   cerr << "      MPS, SAV, or LP (lower case is allowed)" << endl;
   cerr << " Exiting..." << endl;
} // END usage

