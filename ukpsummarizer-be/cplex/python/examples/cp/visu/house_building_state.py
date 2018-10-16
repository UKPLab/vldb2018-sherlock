# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This is a problem of building five houses.
The masonry, roofing, painting, etc. must be scheduled.
Some tasks must necessarily take place before others and these requirements
are expressed through precedence constraints.

A pool of two workers is available for building the houses.
For a given house, some tasks (namely: plumbing, ceiling and painting) require
the house to be clean whereas other tasks (namely: masonry, carpentry, roofing
and windows) put the house in a dirty state.
A transition time of 1 is needed to clean the house so to change from state
'dirty' to state 'clean'.

The objective is to minimize the makespan.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, CpoStepFunction, INTERVAL_MIN, INTERVAL_MAX
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
MASONRY   = BuildingTask('masonry',   35)
CARPENTRY = BuildingTask('carpentry', 15)
PLUMBING  = BuildingTask('plumbing',  40)
CEILING   = BuildingTask('ceiling',   15)
ROOFING   = BuildingTask('roofing',    5)
PAINTING  = BuildingTask('painting',  10)
WINDOWS   = BuildingTask('windows',    5)
FACADE    = BuildingTask('facade',    10)
GARDEN    = BuildingTask('garden',     5)
MOVING    = BuildingTask('moving',     5)

# Tasks precedence constraints (each tuple (X, Y) means X ends before start of Y)
PRECEDENCES = ((MASONRY, CARPENTRY),
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

# List of tasks that requires the house to be clean
CLEAN_TASKS = (PLUMBING, CEILING, PAINTING)

# List of tasks that put the house in a dirty state
DIRTY_TASKS = (MASONRY, CARPENTRY, ROOFING, WINDOWS)

# House cleaning transition time
HOUSE_CLEANING_TIME = 1

# Number of workers
NB_WORKERS = 2

# List of houses to build. Value is the minimum start date
HOUSES = (31, 0, 90, 120, 90)


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Assign an index to tasks
ALL_TASKS = (MASONRY, CARPENTRY, PLUMBING, CEILING, ROOFING, PAINTING, WINDOWS, FACADE, GARDEN, MOVING)
for i in range(len(ALL_TASKS)):
    ALL_TASKS[i].id = i

# Possible states
CLEAN = 0
DIRTY = 1


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
all_state_functions = []

# Transition matrix for cost between house states
TTIME = [[0, 0],[0, 0]]
TTIME[DIRTY][CLEAN] = HOUSE_CLEANING_TIME

# Utility function
def make_house(loc, rd):
    ''' Create model elements corresponding to the building of a house
        loc: Identification of house location
        rd:  Min start date
    '''

    # Create interval variable for each task for this house
    tasks = [mdl.interval_var(size=t.duration,
                              start=(rd, INTERVAL_MAX),
                              name="H" + str(loc) + "-" + t.name) for t in ALL_TASKS]
    all_tasks.extend(tasks)

    global workers_usage

    # Add precedence constraints
    for p, s in PRECEDENCES:
        mdl.add(mdl.end_before_start(tasks[p.id], tasks[s.id]))

    # Create house state function
    house_state = mdl.state_function(TTIME, name="H" + str(loc))
    for t in CLEAN_TASKS:
        mdl.add(mdl.always_equal(house_state, tasks[t.id], CLEAN))
    for t in DIRTY_TASKS:
        mdl.add(mdl.always_equal(house_state, tasks[t.id], DIRTY))
    all_state_functions.append(house_state)

    # Allocate tasks to workers
    for t in ALL_TASKS:
        desc[tasks[t.id]] = t
        house[tasks[t.id]] = loc
        workers_usage += mdl.pulse(tasks[t.id], 1)


# Make houses
for i, sd in enumerate(HOUSES):
    make_house(i, sd)

# Number of workers should not be greater than the limit
mdl.add(mdl.always_in(workers_usage, (INTERVAL_MIN, INTERVAL_MAX), 0, NB_WORKERS))

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
msol = mdl.solve(TimeLimit=10, FailLimit=10000)
print("Solution: ")
msol.print_solution()

if msol and visu.is_visu_enabled():
    workers_function = CpoStepFunction()
    for v in all_tasks:
        itv = msol.get_var_solution(v)
        workers_function.add_value(itv.get_start(), itv.get_end(), 1)

    visu.timeline('Solution SchedState')
    visu.panel(name="Schedule")
    for v in all_tasks:
        visu.interval(msol.get_var_solution(v), house[v], compact(v.get_name()))
    visu.panel(name="Houses state")
    for f in all_state_functions:
        visu.sequence(name=f.get_name(), segments=msol.get_var_solution(f))
    visu.panel(name="Nb of workers")
    visu.function(segments=workers_function, style='line')
    visu.show()
