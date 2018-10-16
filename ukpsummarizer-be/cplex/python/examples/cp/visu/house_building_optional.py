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

There are three workers, and each worker has a given non-negative
skill level for each task.  Each task requires one worker that will
have to be selected among the ones who have a non null skill level for
that task.  A worker can be assigned to only one task at a time.  Each
house has a deadline. The objective is to maximize the skill levels of
the workers assigned to the tasks while respecting the deadlines.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel, INTERVAL_MIN
import docplex.cp.utils_visu as visu


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

NB_HOUSES = 5
DEADLINE = 318
WORKER_NAMES = ['Joe', 'Jack', 'Jim']
NB_WORKERS = len(WORKER_NAMES)

# House building task descriptor
class BuildingTask(object):
    def __init__(self, name, duration, skills):
        self.name = name
        self.duration = duration  # Task duration
        self.skills = skills      # Skills of each worker for this task

# List of tasks to be executed for each house
MASONRY   = BuildingTask('masonry',   35, [9, 5, 0])
CARPENTRY = BuildingTask('carpentry', 15, [7, 0, 5])
PLUMBING  = BuildingTask('plumbing',  40, [0, 7, 0])
CEILING   = BuildingTask('ceiling',   15, [5, 8, 0])
ROOFING   = BuildingTask('roofing',    5, [6, 7, 0])
PAINTING  = BuildingTask('painting',  10, [0, 9, 6])
WINDOWS   = BuildingTask('windows',    5, [8, 0, 5])
FACADE    = BuildingTask('facade',    10, [5, 5, 0])
GARDEN    = BuildingTask('garden',     5, [5, 5, 9])
MOVING    = BuildingTask('moving',     5, [6, 0, 8])

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

# Initialize model variable sets
total_skill = 0                                 # Expression computing total of skills
worker_tasks = [[] for w in range(NB_WORKERS)]  # Tasks (interval variables) assigned to a each worker
desc = dict()                                   # Map retrieving task from interval variable

# Utility function
def make_house(loc, deadline):
    ''' Create model elements corresponding to the building of a house
    loc      Identification of house location
    deadline Deadline for finishing the house
    '''

    # Create interval variable for each task for this house
    tasks = [mdl.interval_var(size=t.duration,
                          end=(INTERVAL_MIN, deadline),
                          name='H' + str(loc) + '-' + t.name) for t in ALL_TASKS]

    # Add precedence constraints
    for p, s in PRECEDENCES:
        mdl.add(mdl.end_before_start(tasks[p.id], tasks[s.id]))

    # Allocate tasks to workers
    global total_skill
    for t in ALL_TASKS:
        allocs = []
        for w in range(NB_WORKERS):
            if t.skills[w] > 0:
                wt = mdl.interval_var(optional=True, name="H{}-{}({})".format(loc, t.name, WORKER_NAMES[w]))
                worker_tasks[w].append(wt)
                allocs.append(wt)
                total_skill += (t.skills[w] * mdl.presence_of(wt))
                desc[wt] = t
        mdl.add(mdl.alternative(tasks[t.id], allocs))


# Make houses
for h in range(NB_HOUSES):
    make_house(h, DEADLINE)

# Avoid overlapping between tasks of each worker
for w in range(NB_WORKERS):
    mdl.add(mdl.no_overlap(worker_tasks[w]))

# Maximize total of skills
mdl.add(mdl.maximize(total_skill))


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

# Draw solution
if msol and visu.is_visu_enabled():
    visu.timeline('Solution SchedOptional', 0, DEADLINE)
    for w in range(NB_WORKERS):
        visu.sequence(name=WORKER_NAMES[w])
        for t in worker_tasks[w]:
            wt = msol.get_var_solution(t)
            if wt.is_present():
                if desc[t].skills[w] == max(desc[t].skills):
                    # Green-like color when task is using the most skilled worker
                    color = 'lightgreen'
                else:
                    # Red-like color when task does not use the most skilled worker
                    color = 'salmon'
                visu.interval(wt, color, compact(wt.get_name()))
    visu.show()
