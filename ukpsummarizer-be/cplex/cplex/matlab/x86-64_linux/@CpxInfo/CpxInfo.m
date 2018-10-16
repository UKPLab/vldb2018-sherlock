classdef CpxInfo < handle

% CpxInfo - The CpxInfo object will be passed to the CPLEX(R) callback
   %           during MIP optimization. The callback function is called
   %           with such an object as parameter.
   %           Its properties are initialized to represent the current
   %           solution/progress of the solve and can be queried by the
   %           callback function.

% ---------------------------------------------------------------------------
% File: CpxInfo.m
% ---------------------------------------------------------------------------
% Licensed Materials - Property of IBM
% 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
% Copyright IBM Corporation 2008, 2017. All Rights Reserved.
%
% US Government Users Restricted Rights - Use, duplication or
% disclosure restricted by GSA ADP Schedule Contract with
% IBM Corp.
% ---------------------------------------------------------------------------
   properties
      % NumNodes
      %  Total number of nodes solved.
      NumNodes;
      % NumIters
      %  Total number of MIP iterations.
     NumIters;
      % Cutoff
      %  Updated cutoff value.
      Cutoff;
      % BestObj
      %  Objective value of best remaining node.
      BestObj;
      % IncObj
      %  Objective value of best integer solution.
      IncObj;
      % IncX
      %  x value of best integer solution.
      IncX;
      % MipGap
      %  Value of relative gap in a MIP.
      MipGap
      Ncliques;
      % Ncliques
      %  Number of clique cuts added.
      Ncovers;
      % Ncovers
      %  Number of cover cuts added.
      NflowCovers;
      % NflowCovers
      %  Number of flowCover cuts added.
      NflowPaths;
      % NflowPaths
      %  Number of flowPath cuts added.
      NGUBcovers;
      % NGUBcovers
      %  Number of GUBcover cuts added.
      NfractionalCuts;
      % NfractionalCuts
      %  Number of fractional cuts added.
      NdisjunctiveCuts;
      % NdisjunctiveCuts
      %  Number of disjunctive cuts added.
      NMIRs;
      % NMIRs
      %  Number of MIR cuts added.
      NimpliedBounds;
      % NimpliedBounds
      %  Number of impliedBounds cuts added.
      NzeroHalfCuts;
      % NzeroHalfCuts
      %  Number of zeroHalf cuts added.
      NMCFCuts;
      % NMCFCuts
      %  Number of MCF cuts added.
      NliftProjectCuts;
      % NliftProjectCuts
      %  Number of Lift and Project cuts added.
      NuserCuts;
      % NuserCuts
      %  Number of user cuts added.
      NtableCuts;
      % NtableCuts
      %  Number of table cuts added.
      NsolutionPoolCuts;
      % NsolutionPoolCuts
      %  Number of solution pool cuts added.
      DetTime;
      % DetTime
      %  Current deterministic time since start of solve
      Time;
      % Time
      %  Current time since start of solve
      EndDetTime;
      % EndDetTime
      %  Remaining deterministic time till time limit
      EndTime;
      % EndTime
      %  Remaining real time till time limit
   end%end of properties
   methods %methods of CpxInfo class
      function obj = set.DetTime(obj,value)
         % Set DetTime
         % See also CpxInfo.
      end%end of function
      function obj = set.Time(obj,value)
         % Set Time
         % See also CpxInfo.
      end%end of function
      function obj = set.EndDetTime(obj,value)
         % Set EndDetTime
         % See also CpxInfo.
      end%end of function
      function obj = set.EndTime(obj,value)
         % Set EndTime
         % See also CpxInfo.
      end%end of function
      function obj = set.NumNodes(obj,value) %#ok<MCHV2>
         % Set NumNodes
         % See also CpxInfo.
      end%end of function
      function obj = set.NumIters(obj,value)
         % Set NumIters
         % See also CpxInfo.
      end%end of function
      function obj = set.Cutoff(obj,value)
         % Set Cutoff
         % See also CpxInfo.
      end%end of function
      function obj = set.BestObj(obj,value)
         % Set BestObj
         % See also CpxInfo.
      end%end of function
      function obj = set.IncObj(obj,value)
         % Set IncObj
         % See also CpxInfo.
      end%end of function
      function obj = set.IncX(obj,value)
         % Set IncX
         % See also CpxInfo.
      end%end of function
      function obj = set.MipGap(obj,value)
         % Set MipGap
         % See also CpxInfo.
      end%end of function
      function obj = set.Ncliques(obj,value)
         % Set Ncliques
         % See also CpxInfo.
      end%end of function
      function obj = set.Ncovers(obj,value)
         % Set Ncovers
         % See also CpxInfo.
      end%end of function
      function obj = set.NflowCovers(obj,value)
         % Set NflowCovers
         % See also CpxInfo.
      end%end of function
      function obj = set.NflowPaths(obj,value)
         % Set NflowPaths
         % See also CpxInfo.
      end%end of function
      function obj = set.NGUBcovers(obj,value)
         % Set NGUBcovers
         % See also CpxInfo.
      end%end of function
      function obj = set.NfractionalCuts(obj,value)
         % Set NfractionalCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NdisjunctiveCuts(obj,value)
         % Set NdisjunctiveCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NMIRs(obj,value)
         % Set NMIRs
         % See also CpxInfo.
      end%end of function
      function obj = set.NimpliedBounds(obj,value)
         % Set NimpliedBounds
         % See also CpxInfo.
      end%end of function
      function obj = set.NzeroHalfCuts(obj,value)
         % Set NzeroHalfCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NMCFCuts(obj,value)
         % Set NMCFCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NliftProjectCuts(obj,value)
         % Set NliftProjectCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NuserCuts(obj,value)
         % Set NuserCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NtableCuts(obj,value)
         % Set NtableCuts
         % See also CpxInfo.
      end%end of function
      function obj = set.NsolutionPoolCuts(obj,value)
         % Set NsolutionPoolCuts
         % See also CpxInfo.
      end%end of function
      function val = get.EndTime(obj)
         % Get EndTime
         % See also CpxInfo.
      end%end of function
      function val = get.EndDetTime(obj)
         % Get EndDetTime
         % See also CpxInfo.
      end%end of function
      function val = get.Time(obj)
         % Get Time
         % See also CpxInfo.
      end%end of function
      function val = get.DetTime(obj)
         % Get DetTime
         % See also CpxInfo.
      end%end of function
      function val = get.NumNodes(obj)
         % Get NumNodes
         % See also CpxInfo.
      end%end of function
      function val = get.NumIters(obj)
         % Get NumIters
         % See also CpxInfo.
      end%end of function
      function val = get.Cutoff(obj)
         % Get Cutoff
         % See also CpxInfo.
      end%end of function
      function val = get.BestObj(obj)
         % Get BestObj
         % See also CpxInfo.
      end%end of function
      function val = get.IncObj(obj)
         % Get IncObj
         % See also CpxInfo.
      end%end of function
      function val = get.IncX(obj)
         % Get IncX
         % See also CpxInfo.
      end%end of function
      function val = get.MipGap(obj)
         % Get MipGap
         % See also CpxInfo.
      end%end of function
      function val = get.Ncliques(obj)
         % Get Ncliques
         % See also CpxInfo.
      end%end of function
      function val = get.Ncovers(obj)
         % Get Ncovers
         % See also CpxInfo.
      end%end of function
      function val = get.NflowCovers(obj)
         % Get NflowCovers
         % See also CpxInfo.
      end%end of function
      function val = get.NflowPaths(obj)
         % Get NflowPaths
         % See also CpxInfo.
      end%end of function
      function val = get.NGUBcovers(obj)
         % Get NGUBcovers
         % See also CpxInfo.
      end%end of function
      function val = get.NfractionalCuts(obj)
         % Get NfractionalCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NdisjunctiveCuts(obj)
         % Get NdisjunctiveCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NMIRs(obj)
         % Get NMIRs
         % See also CpxInfo.
      end%end of function
      function val = get.NimpliedBounds(obj)
         % Get NimpliedBounds
         % See also CpxInfo.
      end%end of function
      function val = get.NzeroHalfCuts(obj)
         % Get NzeroHalfCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NMCFCuts(obj)
         % Get NMCFCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NliftProjectCuts(obj)
         % Get NliftProjectCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NuserCuts(obj)
         % Get NuserCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NtableCuts(obj)
         % Get NtableCuts
         % See also CpxInfo.
      end%end of function
      function val = get.NsolutionPoolCuts(obj)
         % Get NsolutionPoolCuts
         % See also CpxInfo.
      end%end of function
   end%end of methods
end%end of class
