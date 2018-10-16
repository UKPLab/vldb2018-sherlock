// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/cpofileformat.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2014. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

// This example illustrate importModel/dumpModel functionality on a simple map
// coloring problem. Function createModel builds a model using Concert C++ API
// and then, instead of solving it, it dumps the model into text file (the
// default file name is cpofileformat.cpo). After that, function solveModel
// imports the model from the file into another instance of IloCP, modifies
// domain of one of the variables (France must be blue), solves the model and
// prints the solution. 

#include <ilcp/cp.h>


// Problem Description
// -------------------
// The problem involves choosing colors for the countries on a map in such a way
// that at most four colors are used and no neighboring countries are the same
// color. In this exercise, you will find a solution for a map coloring problem
// with six countries: Belgium, Denmark, France, Germany, Luxembourg, and the
// Netherlands.

const char* Names[] = {"blue", "white", "yellow", "green"};

void createModel(const char* filename) {
  IloEnv env;
  try {
    IloModel model(env);
    // Macros ILOSETLOCATION and ILOADD store current source file location.
    // Thanks to that locations will be included in the generated file.
    IloIntVar Belgium(env, 0, 3, "Belgium");         ILOSETLOCATION(Belgium);
    IloIntVar Denmark(env, 0, 3, "Denmark");         ILOSETLOCATION(Denmark);
    IloIntVar France(env, 0, 3, "France");           ILOSETLOCATION(France);
    IloIntVar Germany(env, 0, 3, "Germany");         ILOSETLOCATION(Germany);
    IloIntVar Luxembourg(env, 0, 3, "Luxembourg");   ILOSETLOCATION(Luxembourg);
    IloIntVar Netherlands(env, 0, 3, "Netherlands"); ILOSETLOCATION(Netherlands);
    ILOADD(model, Belgium != France);
    ILOADD(model, Belgium != Germany);
    ILOADD(model, Belgium != Netherlands);
    ILOADD(model, Belgium != Luxembourg);
    ILOADD(model, Denmark != Germany );
    ILOADD(model, France != Germany);
    ILOADD(model, France != Luxembourg);
    ILOADD(model, Germany != Luxembourg);
    ILOADD(model, Germany != Netherlands);
    IloCP cp(model);
    cp.dumpModel(filename);
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}

void solveModel(const char* filename) {
  IloEnv env;
  try {
    IloCP cp(env);
    cp.importModel(filename);
    // Force blue color (zero) for France:
    cp.getIloIntVar("France").setBounds(0, 0);
    if (cp.solve()) {
      cp.out() << std::endl << "Solution:" << std::endl;
      IloIntVarArray vars = cp.getAllIloIntVars();
      for (IloInt i = 0; i < vars.getSize(); i++)
        cp.out() << vars[i].getName() << ": " << Names[cp.getValue(vars[i])] << std::endl;
    }
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}

int main(int argc, const char** argv) {
  const char* filename = (argc > 1 ? argv[1] : "cpofileformat.cpo");
  createModel(filename);
  solveModel(filename);
  return 0;
}
