# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This is a problem of building a house. The masonry, roofing, painting,
etc. must be scheduled.  Some tasks must necessarily take place before
others and these requirements are expressed through precedence
constraints.

Moreover, there are earliness and tardiness costs associated with some tasks.
The objective is to minimize these costs.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, CpoSegmentedFunction
import docplex.cp.utils_visu as visu

#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# House building task descriptor
class BuildingTask(object):
    def __init__(self, name, duration, release_date=None, due_date=None, earliness_cost=None, tardiness_cost=None):
        self.name = name
        self.duration = duration
        self.release_date = release_date
        self.due_date = due_date
        self.earliness_cost = earliness_cost
        self.tardiness_cost = tardiness_cost

# List of tasks to be executed for the house
MASONRY   = BuildingTask('masonry',   35, release_date=25, earliness_cost=200.0)
CARPENTRY = BuildingTask('carpentry', 15, release_date=75, earliness_cost=300.0)
PLUMBING  = BuildingTask('plumbing',  40)
CEILING   = BuildingTask('ceiling',   15, release_date=75, earliness_cost=100.0)
ROOFING   = BuildingTask('roofing',    5)
PAINTING  = BuildingTask('painting',  10)
WINDOWS   = BuildingTask('windows',    5)
FACADE    = BuildingTask('facade',    10)
GARDEN    = BuildingTask('garden',     5)
MOVING    = BuildingTask('moving',     5, due_date=100, tardiness_cost=400.0)

# Tasks precedence constraints (each tuple (X, Y) means X ends before start of Y)
PRECEDENCES = ( (MASONRY, CARPENTRY),
                (MASONRY, PLUMBING),
                (MASONRY, CEILING),
                (CARPENTRY, ROOFING),
                (CEILING, PAINTING),
                (ROOFING, WINDOWS),
                (ROOFING, FACADE),
                (PLUMBING, FACADE),
                (ROOFING, GARDEN),
                (PLUMBING, GARDEN),
                (WINDOWS, MOVING),
                (FACADE, MOVING),
                (GARDEN, MOVING),
                (PAINTING, MOVING),
            )


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Assign an index to tasks
ALL_TASKS = (MASONRY, CARPENTRY, PLUMBING, CEILING, ROOFING, PAINTING, WINDOWS, FACADE, GARDEN, MOVING)
for i in range(len(ALL_TASKS)):
    ALL_TASKS[i].id = i


#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Create interval variable for each building task
tasks = [mdl.interval_var(size=t.duration, name=t.name) for t in ALL_TASKS]

# Add precedence constraints
for p, s in PRECEDENCES:
    mdl.add(mdl.end_before_start(tasks[p.id], tasks[s.id]))

# Cost function
cost = []             # List of individual costs
fearliness = dict()   # Task earliness cost function (for display)
ftardiness = dict()   # Task tardiness cost function (for display)

for t in ALL_TASKS:
    task = tasks[t.id]
    if t.release_date is not None:
        f = CpoSegmentedFunction((-t.earliness_cost, 0), [(t.release_date, 0, 0)])
        cost.append(mdl.start_eval(task, f))
        fearliness[t] = f
    if t.due_date is not None:
        f = CpoSegmentedFunction((0, 0), [(t.due_date, 0, t.tardiness_cost)])
        cost.append(mdl.end_eval(task, f))
        ftardiness[t] = f

# Minimize cost
mdl.add(mdl.minimize(mdl.sum(cost)))


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

print("Solving model....")
msol = mdl.solve(TimeLimit=10)
print("Solution: ")
msol.print_solution()

if msol and visu.is_visu_enabled():
    visu.timeline("Solution SchedTime", origin=10, horizon=120)
    visu.panel("Schedule")
    for t in ALL_TASKS:
        visu.interval(msol.get_var_solution(tasks[t.id]), t.id, t.name)
    for t in ALL_TASKS:
        if t.release_date is not None:
            visu.panel('Earliness')
            itvsol = msol.get_var_solution(tasks[t.id])
            cost = fearliness[t].get_value(itvsol.get_start())
            visu.function(segments=[(itvsol, cost, t.name)], color=t.id, style='interval')
            visu.function(segments=fearliness[t], color=t.id)
        if t.due_date is not None:
            visu.panel('Tardiness')
            itvsol = msol.get_var_solution(tasks[t.id])
            cost = ftardiness[t].get_value(itvsol.get_end())
            visu.function(segments=[(itvsol, cost, t.name)], color=t.id, style='interval')
            visu.function(segments=ftardiness[t], color=t.id)
    visu.show()
