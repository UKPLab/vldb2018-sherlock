// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/alloc.cpp
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2015. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Frequency assignment problem
----------------------------

Problem Description

The problem is given here in the form of discrete data; that is,
each frequency is represented by a number that can be called its
channel number.  For practical purposes, the network is divided
into cells (this problem is an actual cellular phone problem).
In each cell, there is a transmitter which uses different
channels.  The shape of the cells have been determined, as well
as the precise location where the transmitters will be
installed.  For each of these cells, traffic requires a number
of frequencies.

Between two cells, the distance between frequencies is given in
the matrix on the next page.

The problem of frequency assignment is to avoid interference.
As a consequence, the distance between the frequencies within a
cell must be greater than 16.  To avoid inter-cell interference,
the distance must vary because of the geography.

------------------------------------------------------------ */
#include <ilcp/cp.h>

ILOSTLBEGIN

// ----------------------------------------------------------------------------
const int nbCell               = 25;
const int nbAvailFreq          = 256;
const int nbChannel[nbCell] =
  { 8,6,6,1,4,4,8,8,8,8,4,9,8,4,4,10,8,9,8,4,5,4,8,1,1 };
const int dist[nbCell][nbCell] = {
  { 16,1,1,0,0,0,0,0,1,1,1,1,1,2,2,1,1,0,0,0,2,2,1,1,1 },
  { 1,16,2,0,0,0,0,0,2,2,1,1,1,2,2,1,1,0,0,0,0,0,0,0,0 },
  { 1,2,16,0,0,0,0,0,2,2,1,1,1,2,2,1,1,0,0,0,0,0,0,0,0 },
  { 0,0,0,16,2,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
  { 0,0,0,2,16,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
  { 0,0,0,2,2,16,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,1,1 },
  { 0,0,0,0,0,0,16,2,0,0,1,1,1,0,0,1,1,1,1,2,0,0,0,1,1 },
  { 0,0,0,0,0,0,2,16,0,0,1,1,1,0,0,1,1,1,1,2,0,0,0,1,1 },
  { 1,2,2,0,0,0,0,0,16,2,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1 },
  { 1,2,2,0,0,0,0,0,2,16,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1 },
  { 1,1,1,0,0,0,1,1,2,2,16,2,2,2,2,2,2,1,1,2,1,1,0,1,1 },
  { 1,1,1,0,0,0,1,1,2,2,2,16,2,2,2,2,2,1,1,2,1,1,0,1,1 },
  { 1,1,1,0,0,0,1,1,2,2,2,2,16,2,2,2,2,1,1,2,1,1,0,1,1 },
  { 2,2,2,0,0,0,0,0,2,2,2,2,2,16,2,1,1,1,1,1,1,1,1,1,1 },
  { 2,2,2,0,0,0,0,0,2,2,2,2,2,2,16,1,1,1,1,1,1,1,1,1,1 },
  { 1,1,1,0,0,0,1,1,1,1,2,2,2,1,1,16,2,2,2,1,2,2,1,2,2 },
  { 1,1,1,0,0,0,1,1,1,1,2,2,2,1,1,2,16,2,2,1,2,2,1,2,2 },
  { 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,16,2,2,1,1,0,2,2 },
  { 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,16,2,1,1,0,2,2 },
  { 0,0,0,1,1,1,2,2,1,1,2,2,2,1,1,1,1,2,2,16,1,1,0,1,1 },
  { 2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,1,1,1,16,2,1,2,2 },
  { 2,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,1,1,1,2,16,1,2,2 },
  { 1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,16,1,1 },
  { 1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,2,2,1,16,2 },
  { 1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,1,2,2,1,2,16 }
};
// ----------------------------------------------------------------------------

IloInt GetTransmitterIndex(IloInt cell, IloInt channel) {
  IloInt idx = 0;
  IloInt c = 0;
  while (c < cell)
    idx += nbChannel[c++];
  return (idx+channel);
}

int main(int, const char * []){
  IloEnv env;
  try {
    IloModel model(env);
    IloInt nbTransmitters = GetTransmitterIndex(nbCell, 0);
    IloIntVarArray freq(env, nbTransmitters, 0, nbAvailFreq - 1);
    freq.setNames("freq");
    for (IloInt cell = 0; cell < nbCell; cell++)
      for (IloInt channel1 = 0; channel1 < nbChannel[cell]; channel1++)
        for (IloInt channel2= channel1+1; channel2 < nbChannel[cell]; channel2++)
          model.add(IloAbs(  freq[GetTransmitterIndex(cell, channel1)]
                             - freq[GetTransmitterIndex(cell, channel2)] )
                    >= 16);
    for (IloInt cell1 = 0; cell1 < nbCell; cell1++)
      for (IloInt cell2 = cell1+1; cell2 < nbCell; cell2++)
        if (dist[cell1][cell2] > 0)
          for (IloInt channel1 = 0; channel1 < nbChannel[cell1]; channel1++)
            for (IloInt channel2 = 0; channel2 < nbChannel[cell2]; channel2++)
              model.add(IloAbs(  freq[GetTransmitterIndex(cell1, channel1)]
                                 - freq[GetTransmitterIndex(cell2, channel2)] )
                        >= dist[cell1][cell2]);
    
    // Minimizing the total number of frequencies
    IloIntExpr nbFreq = IloCountDifferent(freq); 
    model.add(IloMinimize(env, nbFreq));
    
    IloCP cp(model);
    cp.setParameter(IloCP::CountDifferentInferenceLevel, IloCP::Extended);
    cp.setParameter(IloCP::FailLimit, 40000);
    cp.setParameter(IloCP::LogPeriod, 100000);

    if (cp.solve()) {
      for (IloInt cell = 0; cell < nbCell; cell++) {
        for (IloInt channel = 0; channel < nbChannel[cell]; channel++)
          cp.out() << cp.getValue(freq[GetTransmitterIndex(cell, channel)])
                   << "  " ;
        cp.out() << std::endl;
      }
      cp.out() << "Total # of sites       " << nbTransmitters << std::endl;
      cp.out() << "Total # of frequencies " << cp.getValue(nbFreq) << std::endl;
    } else
      cp.out() << "No solution found."  << std::endl;
    cp.end();
  } catch (IloException & ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}

