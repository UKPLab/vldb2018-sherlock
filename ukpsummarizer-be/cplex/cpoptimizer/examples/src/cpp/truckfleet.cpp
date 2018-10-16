// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/truckfleet.cpp
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
The problem is to deliver some orders to several clients with a single truck. 
Each order consists of a given quantity of a product of a certain type (called
its color).
The truck must be configured in order to handle one, two or three different colors of products.
The cost for configuring the truck from a configuration A to a configuration B depends on A and B.
The configuration of the truck determines its capacity and its loading cost. 
A truck can only be loaded with orders for the same customer.
Both the cost (for configuring and loading the truck) and the number of travels needed to deliver all the 
orders must be minimized, the cost being the most important criterion. 

------------------------------------------------------------ */

#include <ilcp/cp.h>

int main(int, const char * []) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt nbTruckConfigs = 7; // number of possible configurations for the truck 

    IloInt nbOrders       = 21;
    IloInt nbCustomers    = 3; 
    IloInt nbTrucks       = 15; //max number of travels of the truck
   
    IloIntArray    maxTruckConfigLoad(env, nbTruckConfigs, //Capacity of the truck depends on its config 
      11, 11, 11, 11,10,10,10); 
    IloInt         maxLoad        = IloMax(maxTruckConfigLoad) ; 
    IloIntArray    customerOfOrder(env,  nbOrders,
      0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 
      2);
    IloIntArray    volumes(env,  nbOrders, 
      3, 4, 3, 2, 5, 4, 11, 4, 5, 2,
      4, 7, 3, 5, 2, 5, 6, 11, 1, 6, 
      3);
    IloIntArray    colors(env, nbOrders, 
      1, 2, 0, 1, 1, 1, 0, 0, 0, 0, 
      2, 2, 2 ,0, 2, 1, 0, 2, 0, 0, 
      0);
    IloIntArray    truckCost(env, nbTruckConfigs, //cost for loading a truck of a given config
      2, 2, 2, 3, 3, 3, 4); 

    //Decision variables
    IloIntVarArray truckConfigs(env, nbTrucks,  0, nbTruckConfigs-1); //configuration of the truck
    IloIntVarArray where(env, nbOrders, 0, nbTrucks - 1); //In which truck is an order
    IloIntVarArray load(env, nbTrucks, 0, maxLoad); //load of a truck
    IloIntVar numUsed(env, 0, nbTrucks); // number of trucks used
    IloIntVarArray customerOfTruck(env, nbTrucks, 0, nbCustomers);

    // transition costs between trucks
    IloIntTupleSet costTuples(env,3);    
    costTuples.add(IloIntArray(env,3,0,0,0));
    costTuples.add(IloIntArray(env,3,0,1,0));
    costTuples.add(IloIntArray(env,3,0,2,0));
    costTuples.add(IloIntArray(env,3,0,3,10));
    costTuples.add(IloIntArray(env,3,0,4,10));
    costTuples.add(IloIntArray(env,3,0,5,10));
    costTuples.add(IloIntArray(env,3,0,6,15));
    costTuples.add(IloIntArray(env,3,1,0,0));
    costTuples.add(IloIntArray(env,3,1,1,0));
    costTuples.add(IloIntArray(env,3,1,2,0));
    costTuples.add(IloIntArray(env,3,1,3,10));
    costTuples.add(IloIntArray(env,3,1,4,10));
    costTuples.add(IloIntArray(env,3,1,5,10));
    costTuples.add(IloIntArray(env,3,1,6,15));
    costTuples.add(IloIntArray(env,3,2,0,0));
    costTuples.add(IloIntArray(env,3,2,1,0));
    costTuples.add(IloIntArray(env,3,2,2,0));
    costTuples.add(IloIntArray(env,3,2,3,10));
    costTuples.add(IloIntArray(env,3,2,4,10));
    costTuples.add(IloIntArray(env,3,2,5,10));
    costTuples.add(IloIntArray(env,3,2,6,15));
    costTuples.add(IloIntArray(env,3,3,0,3));
    costTuples.add(IloIntArray(env,3,3,1,3));
    costTuples.add(IloIntArray(env,3,3,2,3));
    costTuples.add(IloIntArray(env,3,3,3,0));
    costTuples.add(IloIntArray(env,3,3,4,10));
    costTuples.add(IloIntArray(env,3,3,5,10));
    costTuples.add(IloIntArray(env,3,3,6,15));
    costTuples.add(IloIntArray(env,3,4,0,3));
    costTuples.add(IloIntArray(env,3,4,1,3));
    costTuples.add(IloIntArray(env,3,4,2,3));
    costTuples.add(IloIntArray(env,3,4,3,10));
    costTuples.add(IloIntArray(env,3,4,4,0));
    costTuples.add(IloIntArray(env,3,4,5,10));
    costTuples.add(IloIntArray(env,3,4,6,15));
    costTuples.add(IloIntArray(env,3,5,0,3));
    costTuples.add(IloIntArray(env,3,5,1,3));
    costTuples.add(IloIntArray(env,3,5,2,3));
    costTuples.add(IloIntArray(env,3,5,3,10));
    costTuples.add(IloIntArray(env,3,5,4,10));
    costTuples.add(IloIntArray(env,3,5,5,0));
    costTuples.add(IloIntArray(env,3,5,6,15));
    costTuples.add(IloIntArray(env,3,6,0,3));
    costTuples.add(IloIntArray(env,3,6,1,3));
    costTuples.add(IloIntArray(env,3,6,2,3));
    costTuples.add(IloIntArray(env,3,6,3,10));
    costTuples.add(IloIntArray(env,3,6,4,10));
    costTuples.add(IloIntArray(env,3,6,5,10));
    costTuples.add(IloIntArray(env,3,6,6,0));

    IloIntVarArray transitionCost(env, nbTrucks-1, 0, 1000);
    for (IloInt i = 1; i < nbTrucks; i++) {
      model.add(IloAllowedAssignments(env, truckConfigs[i-1], truckConfigs[i], transitionCost[i-1],costTuples));
    }

    // constrain the volume of the orders in each truck 
    model.add(IloPack(env, load, where, volumes, numUsed));
    for (IloInt i = 0; i < nbTrucks; i++) {
      model.add(load[i] <= maxTruckConfigLoad[truckConfigs[i]]);
    }

    // compatibility between the colors of an order and the configuration of its truck 
    IloArray<IloIntArray> allowedContainerConfigs(env, 3);
    allowedContainerConfigs[0] = IloIntArray(env, 4, 0, 3, 4, 6);
    allowedContainerConfigs[1] = IloIntArray(env, 4, 1, 3, 5, 6);
    allowedContainerConfigs[2] = IloIntArray(env, 4, 2, 4, 5, 6);
    for (IloInt j = 0; j < nbOrders; j++) {
      IloIntVar configOfContainer(env, allowedContainerConfigs[colors[j]]);
      model.add(configOfContainer == IloElement(truckConfigs,where[j]));
    }

    // only one customer per truck 
    for (IloInt j = 0; j < nbOrders; j++) {
      model.add(customerOfTruck[where[j]]==customerOfOrder[j]);
    }

    // non used trucks are at the end
    for (IloInt j = 1; j < nbTrucks; j++) {
      model.add(load[j-1] > 0 || load[j] == 0);
    }

    // Dominance: the non used truck keep the last used configuration
    model.add(load[0]>0);
    for (IloInt i = 1; i < nbTrucks; i++) {
      model.add(load[i] > 0 || truckConfigs[i] == truckConfigs[i-1]);
    }

    //Dominance:  regroup deliveries with same configuration
    for (IloInt i = nbTrucks-2; i >0; i--) {
      IloConstraint Ct = IloTrueConstraint(env);
      for (IloInt p = i+1; p < nbTrucks; p++) 
        Ct = (truckConfigs[p]!=truckConfigs[i-1]) && Ct;
      model.add( (truckConfigs[i] == truckConfigs[i-1]) || Ct);
    }

    // Objective: first criterion for minimizing the cost for configuring and loading trucks 
    //            second criterion for minimizing the number of trucks
    
    IloExpr obj1(env);
    for(IloInt i = 0; i < nbTrucks; i++){
      obj1 += truckCost[truckConfigs[i]]*(load[i]!=0);
    }
 
    obj1 += IloSum(transitionCost);

    IloIntExpr obj2 = numUsed;
    IloNumExprArray objArray(env);
    objArray.add(obj1);
    objArray.add(obj2);

    // Multicriteria lexicographic optimization
    model.add(IloMinimize(env, IloStaticLex(env, objArray)));
    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, 20);
    cp.setParameter(IloCP::LogPeriod, 50000);
    cp.solve();

    cp.out() << "Configuration cost: " << cp.getValue(obj1)
             << " Number of Trucks: " << cp.getValue(obj2) <<std::endl;
    for(IloInt i = 0; i < nbTrucks; i++){  
      if (cp.getValue(load[i]==0))
        continue;
      cp.out() << "Truck " << i
               << ": Config=" << cp.getValue(truckConfigs[i])
               << " Items= ";
      for (IloInt j = 0; j < nbOrders; j++) {
        if (cp.getValue(where[j]) == i) {
          cp.out() << "<" << j << "," << colors[j] << "," << volumes[j] << "> ";
        }
      }
      cp.out() << std::endl;
    }
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}

