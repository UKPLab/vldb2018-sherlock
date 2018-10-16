# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This example is the same than the one implemented in rcpsp_multi_mode.py except that
input data files are represented with JSON format, simpler to read and modify.

The MMRCPSP (Multi-Mode Resource-Constrained Project Scheduling Problem) is a
generalization of the Resource-Constrained Project Scheduling problem
(see rcpsp.py).

In the MMRCPSP, each activity can be performed in one out of several modes.
Each mode of an activity represents an alternative way of combining different levels
of resource requirements with a related duration.
Renewable and non-renewable resources are distinguished.
While renewable resources have a limited instantaneous availability such as
manpower and machines, non renewable resources are limited for the entire project,
allowing to model, e.g., a budget for the project.

The objective is to find a mode and a start time for each activity such that the
schedule is makespan minimal and feasible with regard to the precedence
and resource constraints.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, CpoStepFunction, INTERVAL_MIN, INTERVAL_MAX
import docplex.cp.utils_visu as visu
import os
import json


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# Load input data from json file
filename = os.path.dirname(os.path.abspath(__file__)) + "/data/rcpspmm_default.json"
with open(filename, 'r') as f:
    jstr = f.read()
JSON_DATA = json.loads(jstr)


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Get renewable capacities
CAPACITIES_RENEWABLE = JSON_DATA['capacityRenewable']
NB_RENEWABLE = len(CAPACITIES_RENEWABLE)

# Get non-renewable capacities
CAPACITIES_NON_RENEWABLE = JSON_DATA['capacityNonRenewable']
NB_NON_RENEWABLE = len(CAPACITIES_NON_RENEWABLE)

# Get list of tasks
TASKS = JSON_DATA['tasks']
NB_TASKS = len(TASKS)

# Create a unique id for each mode (to retrieve results)
MODES = []
for t in TASKS:
   for i, m in enumerate(t['modes']):
       m['id'] = "T{}-M{}".format(t['id'], i + 1)
       MODES.append(m)


#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Create one interval variable per task
tasks = {t['id']: mdl.interval_var(name="T{}".format(t['id'])) for t in TASKS}

# Add precedence constraints
for t in TASKS:
    for s in t['successors']:
        mdl.add(mdl.end_before_start(tasks[t['id']], tasks[s]))

# Create one optional interval variable per task mode and add alternatives for tasks
modes = {}  # Map of all modes
for t in TASKS:
    tmds = [mdl.interval_var(name=m['id'], optional=True, size=m['duration']) for m in t['modes']]
    mdl.add(mdl.alternative(tasks[t['id']], tmds))
    for m in tmds:
        modes[m.name] = m

# Initialize pulse functions for renewable and non renewable resources
renewables = [mdl.pulse((0, 0), 0) for j in range(NB_RENEWABLE)]
non_renewables = [0 for j in range(NB_NON_RENEWABLE)]
for m in MODES:
    dren = m['demandRenewable']
    dnren = m['demandNonRenewable']
    for j in range(NB_RENEWABLE):
        dem = m['demandRenewable'][j]
        if dem > 0:
            renewables[j] += mdl.pulse(modes[m['id']], dem)
    for j in range(NB_NON_RENEWABLE):
        dem = m['demandNonRenewable'][j]
        if dem > 0:
            non_renewables[j] += dem * mdl.presence_of(modes[m['id']])

# Constrain renewable resources capacity
for j in range(NB_RENEWABLE):
    mdl.add(mdl.always_in(renewables[j], (INTERVAL_MIN, INTERVAL_MAX), 0, CAPACITIES_RENEWABLE[j]))

# Constrain non-renewable resources capacity
for j in range(NB_NON_RENEWABLE):
    mdl.add(non_renewables[j] <= CAPACITIES_NON_RENEWABLE[j])

# Minimize overall schedule end date
mdl.add(mdl.minimize(mdl.max([mdl.end_of(t) for t in tasks.values()])))


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

# Solve model
print("Solving model....")
msol = mdl.solve(FailLimit=30000, TimeLimit=10)
print("Solution: ")
msol.print_solution()

if msol and visu.is_visu_enabled():
    load = [CpoStepFunction() for j in range(NB_RENEWABLE)]
    for m in MODES:
        itv = msol.get_var_solution(modes[m['id']])
        if itv.is_present():
            for j in range(NB_RENEWABLE):
                dem = m['demandRenewable'][j]
                if dem > 0:
                    load[j].add_value(itv.get_start(), itv.get_end(), dem)

    visu.timeline("Solution for RCPSPMM " + filename)
    visu.panel("Tasks")
    for t in TASKS:
        tid = t['id']
        visu.interval(msol.get_var_solution(tasks[tid]), tid, str(tid))
    for j in range(NB_RENEWABLE):
        visu.panel("R " + str(j + 1))
        visu.function(segments=[(INTERVAL_MIN, INTERVAL_MAX, CAPACITIES_RENEWABLE[j])], style='area', color='lightgrey')
        visu.function(segments=load[j], style='area', color=j)
    visu.show()
