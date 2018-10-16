# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This is a problem of building five houses. The masonry, roofing,
painting, etc. must be scheduled. Some tasks must necessarily take
place before others and these requirements are expressed through
precedence constraints.

There are two workers and each task requires a specific worker.
The worker has a calendar of days off that must be taken into account.
The objective is to minimize the overall completion date.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, CpoStepFunction
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

# Tasks assigned to Joe and Jim
JOE_TASKS = (MASONRY, CARPENTRY, ROOFING, FACADE, GARDEN)
JIM_TASKS = (PLUMBING, CEILING, PAINTING, WINDOWS, MOVING)

# Total number of houses to build
NUMBER_OF_HOUSES = 5

# Max number of calendar years
MAX_YEARS = 2

# Holydays for Joe and Jim as list of tuples (start_day_index, end_day_index)
JOE_HOLYDAYS = ((5, 12), (124, 131), (215, 236), (369, 376), (495, 502), (579, 600))
JIM_HOLYDAYS = ((26, 40), (201, 225), (306, 313), (397, 411), (565, 579))


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Assign an index to tasks
ALL_TASKS = (MASONRY, CARPENTRY, PLUMBING, CEILING, ROOFING, PAINTING, WINDOWS, FACADE, GARDEN, MOVING)
for i in range(len(ALL_TASKS)):
    ALL_TASKS[i].id = i

# Initialize availability calendar for workers
joe_calendar = CpoStepFunction()
jim_calendar = CpoStepFunction()
joe_calendar.set_value(0, MAX_YEARS * 365, 100)
jim_calendar.set_value(0, MAX_YEARS * 365, 100)

# Remove week ends
for w in range(MAX_YEARS * 52):
    joe_calendar.set_value(5 + (7 * w), 7 + (7 * w), 0)
    jim_calendar.set_value(5 + (7 * w), 7 + (7 * w), 0)

# Remove holidays
for b, e in JOE_HOLYDAYS:
   joe_calendar.set_value(b, e, 0)
for b, e in JIM_HOLYDAYS:
   jim_calendar.set_value(b, e, 0)


#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Initialize model variable sets
all_tasks = []  # Array of all tasks
joe_tasks = []  # Tasks assigned to Joe
jim_tasks = []  # Tasks assigned to Jim
house_finish_times = []  # Array of house finishing times
type = dict()

# Utility function
def make_house(loc):
    ''' Create model elements corresponding to the building of one house
    loc: Identification (index) of the house to build
    '''

    # Create interval variable for each task for this house
    tasks = [mdl.interval_var(size=t.duration, name="H" + str(loc) + "-" + t.name) for t in ALL_TASKS]
    for t in ALL_TASKS:
        type[tasks[t.id]] = t.id
    for t in JOE_TASKS:
        tasks[t.id].set_intensity(joe_calendar)
        mdl.add(mdl.forbid_start(tasks[t.id], joe_calendar))
        mdl.add(mdl.forbid_end(tasks[t.id], joe_calendar))
    for t in JIM_TASKS:
        tasks[t.id].set_intensity(jim_calendar)
        mdl.add(mdl.forbid_start(tasks[t.id], jim_calendar))
        mdl.add(mdl.forbid_end(tasks[t.id], jim_calendar))

    # Add precedence constraints
    for p, s in PRECEDENCES:
        mdl.add(mdl.end_before_start(tasks[p.id], tasks[s.id]))

    # Allocate tasks to workers
    for t in JOE_TASKS:
        joe_tasks.append(tasks[t.id])
    for t in JIM_TASKS:
        jim_tasks.append(tasks[t.id])

    # Update cost
    house_finish_times.append(mdl.end_of(tasks[MOVING.id]))


# Make houses
for i in range(NUMBER_OF_HOUSES):
    make_house(i)

# Avoid each worker tasks overlapping
mdl.add(mdl.no_overlap(joe_tasks))
mdl.add(mdl.no_overlap(jim_tasks))

# Add minimization objective
mdl.add(mdl.minimize(mdl.max(house_finish_times)))


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
msol = mdl.solve(TimeLimit=10)
print("Solution: ")
msol.print_solution()

# Display result
if msol and visu.is_visu_enabled():
    visu.timeline('Solution SchedCalendar')
    visu.panel()
    visu.pause(joe_calendar)
    visu.sequence(name='Joe',
                  intervals=[(msol.get_var_solution(t), type[t], compact(t.name)) for t in joe_tasks])
    visu.panel()
    visu.pause(jim_calendar)
    visu.sequence(name='Jim',
                  intervals=[(msol.get_var_solution(t), type[t], compact(t.name)) for t in jim_tasks])
    visu.show()
