# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This is a problem of building five houses in different locations.
The masonry, roofing, painting, etc. must be scheduled.
Some tasks must necessarily take place before others and these requirements are
expressed through precedence constraints.

There are three workers, and each task requires a worker.
There is also a cash budget which starts with a given balance.
Each task costs a given amount of cash per day which must be available at the start of the task.
A cash payment is received periodically.
The objective is to minimize the overall completion date.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, CpoStepFunction, INTERVAL_MAX, INT_MAX
import docplex.cp.utils_visu as visu


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# House building task descriptor
class BuildingTask(object):
    def __init__(self, name, duration):
        self.name = name
        self.duration = duration

# List of tasks to be executed for each house
MASONRY   = BuildingTask('masonry', 35)
CARPENTRY = BuildingTask('carpentry', 15)
PLUMBING  = BuildingTask('plumbing', 40)
CEILING   = BuildingTask('ceiling', 15)
ROOFING   = BuildingTask('roofing', 5)
PAINTING  = BuildingTask('painting', 10)
WINDOWS   = BuildingTask('windows', 5)
FACADE    = BuildingTask('facade', 10)
GARDEN    = BuildingTask('garden', 5)
MOVING    = BuildingTask('moving', 5)

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

# Number of workers
NB_WORKERS = 3

# List of houses to build. Value is the minimum start date
HOUSES = (31, 0, 90, 120, 90)

# Cash parameters
NB_PAYMENTS = 5
PAYMENT_AMOUNT = 30000
PAYMENT_INTERVAL = 60


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

# Initialize model variable sets
all_tasks = []  # Array of all tasks
desc = dict()   # Dictionary task interval var -> task descriptor
house = dict()  # Dictionary task interval var -> id of the corresponding house
workers_usage = mdl.step_at(0, 0)  # Total worker usage

# Initialize cash function
cash = mdl.step_at(0, 0)
for p in range(NB_PAYMENTS):
    cash += mdl.step_at(PAYMENT_INTERVAL * p, PAYMENT_AMOUNT)

# Utility function
def make_house(loc, rd):
    ''' Create model elements corresponding to the building of one house
    loc: Identification (index) of the house to build
    rd:  Min start date
    '''

    # Create interval variable for each task for this house
    tasks = [mdl.interval_var(size=t.duration,
                          start=(rd, INTERVAL_MAX),
                          name="H{}-{}".format(loc, t.name)) for t in ALL_TASKS]
    all_tasks.extend(tasks)

    # Add precedence constraints
    for p, s in PRECEDENCES:
        mdl.add(mdl.end_before_start(tasks[p.id], tasks[s.id]))


    global workers_usage
    global cash

    # Allocate tasks to workers
    for t in ALL_TASKS:
        desc[tasks[t.id]] = t
        house[tasks[t.id]] = loc
        workers_usage += mdl.pulse(tasks[t.id], 1)
        cash -= mdl.step_at_start(tasks[t.id], 200 * t.duration)


# Make houses
for i, sd in enumerate(HOUSES):
    make_house(i, sd)

# Number of workers should not be greater than the limit
mdl.add(workers_usage <= NB_WORKERS)

# Cash should not be negative
mdl.add(cash >= 0)

# Minimize overall completion date
mdl.add(mdl.minimize(mdl.max([mdl.end_of(task) for task in all_tasks])))


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

def compact(name):
    # Example: H3-garden -> G3
    #           ^ ^
    loc, task = name[1:].split('-', 1)
    return task[0].upper() + loc

# Solve model
print("Solving model....")
msol = mdl.solve(FailLimit=10000, TimeLimit=10)
print("Solution: ")
msol.print_solution()

# Display result
if msol and visu.is_visu_enabled():
    workersF = CpoStepFunction()
    cashF = CpoStepFunction()
    for p in range(5):
        cashF.add_value(60 * p, INT_MAX, 30000)
    for task in all_tasks:
        itv = msol.get_var_solution(task)
        workersF.add_value(itv.get_start(), itv.get_end(), 1)
        cashF.add_value(itv.start, INT_MAX, -200 * desc[task].duration)

    visu.timeline('Solution SchedCumul')
    visu.panel(name="Schedule")
    for task in all_tasks:
        visu.interval(msol.get_var_solution(task), house[task], compact(task.get_name()))
    visu.panel(name="Workers")
    visu.function(segments=workersF, style='area')
    visu.panel(name="Cash")
    visu.function(segments=cashF, style='area', color='gold')
    visu.show()
