# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
In the classical Job-Shop Scheduling problem a finite set of jobs is processed
on a finite set of machines.
Each job is characterized by a fixed order of operations, each of which is to
be processed on a specific machine for a specified duration.
All machines are used by each job.
Each machine can process at most one operation at a time and once an operation
initiates processing on a given machine it must complete processing uninterrupted.

The objective of the problem is to find a schedule that minimizes the makespan (end date) of the schedule.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel
import docplex.cp.utils_visu as visu
import os


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# Read the input data file.
# Available files are jobshop_ft06, jobshop_ft10 and jobshop_ft20
# First line contains the number of jobs, and the number of machines.
# The rest of the file consists of one line per job.
# Each line contains list of operations, each one given by 2 numbers: machine and duration
filename = os.path.dirname(os.path.abspath(__file__)) + "/data/jobshop_ft06.data"
with open(filename, "r") as file:
    NB_JOBS, NB_MACHINES = [int(v) for v in file.readline().split()]
    JOBS = [[int(v) for v in file.readline().split()] for i in range(NB_JOBS)]


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Build list of machines. MACHINES[j][s] = id of the machine for the operation s of the job j
MACHINES = [[JOBS[j][2 * s] for s in range(NB_MACHINES)] for j in range(NB_JOBS)]

# Build list of durations. DURATION[j][s] = duration of the operation s of the job j
DURATION = [[JOBS[j][2 * s + 1] for s in range(NB_MACHINES)] for j in range(NB_JOBS)]


#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Create one interval variable per job operation
job_operations = [[mdl.interval_var(size=DURATION[j][m], name="O{}-{}".format(j, m)) for m in range(NB_MACHINES)] for j in range(NB_JOBS)]

# Each operation must start after the end of the previous
for j in range(NB_JOBS):
    for s in range(1, NB_MACHINES):
        mdl.add(mdl.end_before_start(job_operations[j][s - 1], job_operations[j][s]))

# Force no overlap for operations executed on a same machine
machine_operations = [[] for m in range(NB_MACHINES)]
for j in range(NB_JOBS):
    for s in range(0, NB_MACHINES):
        machine_operations[MACHINES[j][s]].append(job_operations[j][s])
for lops in machine_operations:
    mdl.add(mdl.no_overlap(lops))

# Minimize termination date
mdl.add(mdl.minimize(mdl.max([mdl.end_of(job_operations[i][NB_MACHINES - 1]) for i in range(NB_JOBS)])))


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

# Solve model
print("Solving model....")
msol = mdl.solve(TimeLimit=10)
print("Solution: ")
msol.print_solution()

# Draw solution
if msol and visu.is_visu_enabled():
    visu.timeline("Solution for job-shop " + filename)
    visu.panel("Jobs")
    for i in range(NB_JOBS):
        visu.sequence(name='J' + str(i),
                      intervals=[(msol.get_var_solution(job_operations[i][j]), MACHINES[i][j], 'M' + str(MACHINES[i][j])) for j in
                                 range(NB_MACHINES)])
    visu.panel("Machines")
    for k in range(NB_MACHINES):
        visu.sequence(name='M' + str(k),
                      intervals=[(msol.get_var_solution(machine_operations[k][i]), k, 'J' + str(i)) for i in range(NB_JOBS)])
    visu.show()
