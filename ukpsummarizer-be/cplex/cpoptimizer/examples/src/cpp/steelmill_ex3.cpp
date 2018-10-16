// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/steelmill_ex3.cpp
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

/* ------------------------------------------------------------

Problem Description
-------------------

The problem is to build steel coils from slabs that are available in a
work-in-process inventory of semi-finished products. There is no limitation
in the number of slabs that can be requested, but only a finite number of slab
sizes is available (sizes 11, 13, 16, 17, 19, 20, 23, 24, 25, 26, 27, 28, 29,
30, 33, 34, 40, 43, 45). The problem is to select a number of slabs to
build the coil orders, and to satisfy the following constraints:

    * A coil order can be built from only one slab.
    * Each coil order requires a specific process to build it from a
      slab. This process is encoded by a color.
    * Several coil orders can be built from the same slab. But a slab can
      be used to produce at most two different "colors" of coils.
    * The sum of the sizes of each coil order built from a slab must not
      exceed the slab size.

Finally, the production plan should minimize the unused capacity of the
selected slabs.

This problem is based on "prob038: Steel mill slab design problem" from
CSPLib (www.csplib.org).

------------------------------------------------------------ */

#include <ilcp/cp.h>

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt m, o, c, q;

    IloInt         nbOrders   = 12;
    IloInt         nbSlabs    = 12;
    IloInt         nbColors   = 8;

    IloIntArray    capacities(env, 20, 0, 11, 13, 16, 17, 19, 20,
                                       23, 24, 25, 26, 27, 28, 29,
                                       30, 33, 34, 40, 43, 45);
    IloIntArray    sizes(env, nbOrders, 22, 9, 9, 8, 8, 6, 5, 3, 3, 3, 2, 2);
    IloIntArray    colors(env, nbOrders,  5, 3, 4, 5, 7, 3, 6, 0, 2, 3, 1, 5);
    IloIntVarArray where(env, nbOrders, 0, nbSlabs-1);
    IloIntVarArray load(env, nbSlabs, 0, IloSum(sizes));
  // Pack constraint
    model.add(IloPack(env, load, where, sizes));
  // Color constraints
    for(m = 0; m < nbSlabs; m++) {
      IloExprArray colorExpArray(env);
      for(c = 0; c < nbColors; c++) {
        IloOr orExp(env);
        for(o = 0; o < nbOrders; o++){
          if (colors[o] == c){
            orExp.add(where[o] == m);
          }
        }
        colorExpArray.add(orExp);
      }
      model.add(IloSum(colorExpArray) <= 2);
    }

  // Objective function
    IloIntArray lossValues(env);
    lossValues.add(0);
    for(q = 1; q < capacities.getSize(); q++){
      for(IloInt p = capacities[q-1] + 1; p <= capacities[q]; p++){
        lossValues.add(capacities[q] - p);
      }
    }
    IloExpr obj(env);
    for(m = 0; m < nbSlabs; m++){
      obj += lossValues[load[m]];
    }
    IloInt fixedCost=1;
    IloIntVarArray used(env,nbSlabs,0,1);
    for (m=0; m < nbSlabs; m++)
       model.add((load[m] > 0) == used[m]);
    obj += fixedCost*IloSum(used);
    model.add(IloMinimize(env, obj));
    for(m = 1; m < nbSlabs; m++){
      model.add(load[m-1] >= load[m]);
    }
    IloCP cp(model);
    cp.setSearchPhases(IloSearchPhase(env, where));
    if (cp.solve()){
      cp.out() << "Optimal value: " << cp.getValue(obj) << std::endl;
      for (m = 0; m < nbSlabs; m++) {
        IloInt p = 0;
        for (o = 0; o < nbOrders; o++)
          p += cp.getValue(where[o]) == m;
        if (p == 0) continue;
        cp.out() << "Slab " << m << " is used for order";
        if (p > 1) cp.out() << "s";
        cp.out() << " :";
        for (o = 0; o < nbOrders; o++) {
          if (cp.getValue(where[o]) == m)
            cp.out() << " " << o;
        }
        cp.out() << std::endl;
      }
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}

/*
Optimal value: 5
Slab 0 is used for orders : 0 1 9
Slab 1 is used for orders : 3 7 11
Slab 2 is used for orders : 2 10
Slab 3 is used for orders : 4 8
Slab 4 is used for orders : 5 6
*/
