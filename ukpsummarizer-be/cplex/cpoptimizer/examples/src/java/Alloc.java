// ---------------------------------------------------------------*- Java -*-
// File: ./examples/src/java/Alloc.java
// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corporation 1990, 2017. All Rights Reserved.
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


import ilog.cp.*;
import ilog.concert.*;

public class Alloc {
    static final int nbCell = 25;
    static final int nbAvailFreq = 256;
    static final int[] nbChannel = {
        8,6,6,1,4,4,8,8,8,8,4,9,8,4,4,10,8,9,8,4,5,4,8,1,1
    };
    static final int[][] dist = {
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

    public static int getTransmitterIndex(int cell, int channel) {
        int idx = 0;
        int c = 0;
        while (c < cell)
            idx += nbChannel[c++];
        return (idx+channel);
    }

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            int nbTransmitters = getTransmitterIndex(nbCell, 0);
            IloIntVar[] freq = cp.intVarArray(nbTransmitters,
                                              0, nbAvailFreq-1, "freq");
            for (int cell = 0; cell < nbCell; cell++)
                for (int channel1 = 0; channel1 < nbChannel[cell]; channel1++)
                    for (int channel2= channel1+1; channel2 < nbChannel[cell]; channel2++)
                        cp.add(cp.ge(cp.abs(cp.diff(freq[getTransmitterIndex(cell, channel1)],
                                                    freq[getTransmitterIndex(cell, channel2)])),
                                     16));
            for (int cell1 = 0; cell1 < nbCell; cell1++)
                for (int cell2 = cell1+1; cell2 < nbCell; cell2++)
                    if (dist[cell1][cell2] > 0)
                        for (int channel1 = 0; channel1 < nbChannel[cell1]; channel1++)
                            for (int channel2 = 0; channel2 < nbChannel[cell2]; channel2++)
                                cp.add(cp.ge(cp.abs(cp.diff(freq[getTransmitterIndex(cell1, channel1)],
                                                            freq[getTransmitterIndex(cell2, channel2)])),
                                             dist[cell1][cell2]));

            // Minimizing the total number of frequencies
            IloIntExpr nbFreq = cp.countDifferent(freq);
            cp.add(cp.minimize(nbFreq));

            cp.setParameter(IloCP.IntParam.CountDifferentInferenceLevel,
                            IloCP.ParameterValues.Extended);
            cp.setParameter(IloCP.IntParam.FailLimit, 40000);
            cp.setParameter(IloCP.IntParam.LogPeriod, 100000);

            if (cp.solve()) {
                for (int cell = 0; cell < nbCell; cell++) {
                    for (int channel = 0; channel < nbChannel[cell]; channel++)
                      System.out.print((int)cp.getValue(freq[getTransmitterIndex(cell, channel)])
                                         + "  " );
                    System.out.println();
                }
                System.out.println("Total # of sites       " + nbTransmitters);
                System.out.println("Total # of frequencies " + (int)cp.getValue(nbFreq));

            } else
                System.out.println("No solution");
            cp.end();
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }
}
