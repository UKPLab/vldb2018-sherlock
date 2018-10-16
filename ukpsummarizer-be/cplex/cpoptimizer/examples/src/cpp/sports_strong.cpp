// -------------------------------------------------------------- -*- C++ -*-
// File: ./examples/src/cpp/sports_strong.cpp
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

The problem involves finding a schedule for a sports league. The league has 10
teams that play games over a season of 18 weeks. Each team has a home arena and
plays each other team twice during the season, once in its home arena and once in
the opposing team's home arena. For each of these games, the team playing at its
home arena is referred to as the home team; the team playing at the opponent's
arena is called the away team. There are 90 games altogether.

Each of the 18 weeks in the season has five identical slots to which games can be
assigned. Each team plays once a week. For each pair of teams, these two teams are
opponents twice in a season; these two games must be scheduled in different halves
of the season. Moreover, these two games must be scheduled at least six weeks
apart. A team must play at home either the first or last week but not both.

A break is a sequence of consecutive weeks in which a team plays its games either
all at home or all away. No team can have a break of three or more weeks in it. The
objective in this problem is to minimize the total number of breaks the teams play.

------------------------------------------------------------ */

#include <ilcp/cp.h>

//
// This macro holds an expression which calculates a unique
// game identifier knowing the home team (h), the away team (a)
// and the total number of teams (n).  It can be used in a
// context where h and a are integers or integer variables
//
#define Game(h, a, n) ((h) * ((n) - 1) + (a) - ((a) > (h)))

typedef IloArray<IloIntVarArray> IloIntVarArray2;

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    IloModel model(env);
    IloInt n = 10;
    if (argc > 1)
      n = atoi(argv[1]);
    if ((n % 2) == 1)
      n++;
    env.out() << "Finding schedule for " << n << " teams" << std::endl;
    IloInt nbWeeks = 2 * (n - 1);
    IloInt nbGamesPerWeek = n / 2;
    IloInt nbGames = n * (n - 1);

    IloIntVarArray2 games(env, nbWeeks);
    IloIntVarArray2 home(env, nbWeeks);
    IloIntVarArray2 away(env, nbWeeks);
    for (IloInt i = 0; i < nbWeeks; i++) {
      home[i]  = IloIntVarArray(env, nbGamesPerWeek, 0, n - 1);
      away[i]  = IloIntVarArray(env, nbGamesPerWeek, 0, n - 1);
      games[i] = IloIntVarArray(env, nbGamesPerWeek, 0, nbGames - 1);
    }

    //
    // For each play slot, set up correspondance between game id,
    // home team, and away team
    //
    for (IloInt i = 0; i < nbWeeks; i++) {
      for (IloInt j = 0; j < nbGamesPerWeek; j++) {
        // Team cannot play itself
        model.add(home[i][j] != away[i][j]);

        // Calculate the unique game identifier for the home/away combination
        model.add(games[i][j] == Game(home[i][j], away[i][j], n));

        // Make sure the above constraint propagates well
        IloIntVarArray vars(env);
        vars.add(home[i][j]);
        vars.add(away[i][j]);
        vars.add(games[i][j]);
        model.add(IloStrong(env, vars));
      }
    }
    //
    // All teams play each week
    //
    for (IloInt i = 0; i < nbWeeks; i++) {
      IloIntVarArray teamsThisWeek(env);
      teamsThisWeek.add(home[i]);
      teamsThisWeek.add(away[i]);
      model.add(IloAllDiff(env, teamsThisWeek));
    }

    //
    // Dual representation: for each game id, the play slot is maintained
    //
    IloIntVarArray weekOfGame(env, nbGames, 0, nbWeeks - 1);
    IloIntVarArray allGames(env);
    IloIntVarArray allSlots(env, nbGames, 0, nbGames - 1);
    for (IloInt i = 0; i < nbWeeks; i++)
      allGames.add(games[i]);
    model.add(IloInverse(env, allGames, allSlots));
    for (IloInt i = 0; i < nbGames; i++)
      model.add(weekOfGame[i] == IloDiv(allSlots[i], nbGamesPerWeek));

    //
    // Two half schedules.  Cannot play the same pair twice in the same half.
    // Plus, impose a minimum number of weeks between two games involving
    // the same teams (up to six weeks)
    //
    IloInt mid = nbWeeks / 2;
    IloInt overlap = 0;
    if (n >= 6)
      overlap = IloMin(n / 2, 6);
    for (IloInt i = 0; i < n; i++) {
      for (IloInt j = i + 1; j < n; j++) {
        IloInt g1 = Game(i, j, n);
        IloInt g2 = Game(j, i, n);
        model.add((weekOfGame[g1] >= mid) == (weekOfGame[g2] < mid));
        // Six week difference...
        if (overlap != 0)
          model.add(IloAbs(weekOfGame[g1] - weekOfGame[g2]) >= overlap);
      }
    }

    //
    // Can't have three homes or three aways in a row.
    //
    IloIntVarArray2 playHome(env, n);
    for (IloInt i = 0; i < n; i++) {
      playHome[i] = IloIntVarArray(env, nbWeeks, 0, 1);
      for (IloInt j = 0; j < nbWeeks; j++)
        model.add(playHome[i][j] == IloCount(home[j], i));
      for (IloInt j = 0; j < nbWeeks -3; j++) {
        IloIntVarArray window(env);
        for (IloInt k = j; k < j + 3; k++)
          window.add(playHome[i][k]);
        model.add(1 <= IloSum(window) <= 2);
      }
    }
    //
    // If we start the season home, we finish away and vice versa.
    //
    for (IloInt i = 0; i < n; i++)
      model.add(playHome[i][0] != playHome[i][nbWeeks-1]);

    //
    // Objective: minimize the number of `breaks'.  A break is
    //            two consecutive home or away matches for a
    //            particular team
    IloIntVarArray teamBreaks(env, n, 0, nbWeeks / 2);
    for (IloInt i = 0; i < n; i++) {
      IloIntExpr nbreaks(env);
      for (IloInt j = 1; j < nbWeeks; j++)
        nbreaks += (playHome[i][j-1] == playHome[i][j]);
      model.add(teamBreaks[i] == nbreaks);
    }
    IloIntVar breaks(env, n - 2, n * (nbWeeks / 2));
    model.add(breaks == IloSum(teamBreaks));
    model.add(IloMinimize(env, breaks));

    //
    // Surrogate constraints
    //

    // Each team plays home the same number of times as away
    for (IloInt i = 0; i < n; i++)
      model.add(IloSum(playHome[i]) == nbWeeks / 2);

    // Breaks must be even for each team
    for (IloInt i = 0; i < n; i++)
      model.add((teamBreaks[i] % 2) == 0);

    //
    // Symmetry breaking constraints
    //

    // Teams are interchangeable.  Fix first week.
    // Also breaks reflection symmetry of the whole schedule.
    for (IloInt i = 0; i < nbGamesPerWeek; i++) {
      model.add(home[0][i] == i * 2);
      model.add(away[0][i] == i * 2 + 1);
    }

    // Order of games in each week is arbitrary.
    // Break symmetry by forcing an order.
    for (IloInt i = 0; i < nbWeeks; i++)
      for (IloInt j = 1; j < nbGamesPerWeek; j++)
        model.add(games[i][j] > games[i][j-1]);

    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, 20);
    cp.setParameter(IloCP::LogPeriod, 10000);

    IloVarSelectorArray varSel(env);
    varSel.add(IloSelectSmallest(IloVarIndex(env, allGames)));
    IloValueSelectorArray valSel(env);
    valSel.add(IloSelectRandomValue(env));

    IloSearchPhase phase(env, allGames, varSel, valSel);
    cp.setSearchPhases(phase);
    cp.startNewSearch();
    while (cp.next()) {
      cp.out() << std::endl << "Solution at " << cp.getValue(breaks) << std::endl;
      for (IloInt j = 0; j < nbWeeks; j++) {
        cp.out() << "Week " << j << ": ";
        if ( j < 10 ) cp.out() << " ";
        for (IloInt i = 0; i < nbGamesPerWeek; i++) {
          IloInt h = cp.getValue(home[j][i]);
          IloInt a = cp.getValue(away[j][i]);
          if (h >= 10) cp.out() << h;
          else         cp.out() << " " << h;
          cp.out() << "-";
          if (a >= 10) cp.out() << a;
          else         cp.out() << a << " ";
          cp.out() << " ";
        }
        cp.out() << std::endl;
      }
      cp.out() << "Team schedules" << std::endl;
      for (IloInt i = 0; i < n; i++) {
        cp.out() << "T " << i << ":\t";
        IloInt prev = -1;
        IloInt brks = 0;
        for (IloInt j = 0; j < nbWeeks; j++) {
          for (IloInt k = 0; k < nbGamesPerWeek; k++) {
            if (cp.getValue(home[j][k]) == i) {
              IloInt t = cp.getValue(away[j][k]);
              if (t >= 10) cp.out() << t << "H ";
              else         cp.out() << " " << t << "H ";
              brks += (prev == 0);
              prev = 0;
            }
            if (cp.getValue(away[j][k]) == i) {
              IloInt t = cp.getValue(home[j][k]);
              if (t >= 10) cp.out() << t << "A ";
              else         cp.out() << " " << t << "A ";
              brks += (prev == 1);
              prev = 1;
            }
          }
        }
        cp.out() << "   " << brks << " breaks" << std::endl;
      }
      cp.out() << std::endl;
    }
    cp.endSearch();
    cp.end();
  } catch (IloException & ex) {
    env.out() << "Caught " << ex << std::endl;
  }
  env.end();
  return 0;
}

/*
Solution at 40
*/
#undef Game
