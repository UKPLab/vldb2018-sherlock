# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
This problem is an extension of the classical Job-Shop Scheduling problem
(see job_shop_basic.py) which allows an operation to be processed by any machine
from a given set.
The operation processing time depends on the allocated machine.
The problem is to assign each operation to a machine and to order the operations
on the machines such that the maximal completion time (makespan) of all
operations is minimized.

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel
import docplex.cp.utils_visu as visu
import os


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# Read the input data file.
# Available files are jobshopflex_default and multiple other jobshopflex_XXXX.
# First line contains the number of jobs, and the number of machines.
# The rest of the file consists of one line per job.
# First integer is the number of job steps, followed by the choices for each step.
# For each step, first integer indicates the number of choices, followed
# by the choices expressed with two integers: machine and duration
filename = os.path.dirname(os.path.abspath(__file__)) + "/data/jobshopflex_default.data"
with open(filename, "r") as file:
    NB_JOBS, NB_MACHINES = [int(v) for v in file.readline().split()]
    list_jobs = [[int(v) for v in file.readline().split()] for i in range(NB_JOBS)]


#-----------------------------------------------------------------------------
# Prepare the data for modeling
#-----------------------------------------------------------------------------

# Build final list of jobs.
# Each job is a list of operations.
# Each operation is a list of choices expressed as tuples (machine, duration)
JOBS = []
for jline in list_jobs:
    nbstps = jline.pop(0)
    job = []
    for stp in range(nbstps):
        nbc = jline.pop(0)
        choices = []
        for c in range(nbc):
            m = jline.pop(0)
            d = jline.pop(0)
            choices.append((m - 1, d))
        job.append(choices)
    JOBS.append(job)


#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Following code creates:
# - creates one interval variable for each possible operation choice
# - creates one interval variable for each operation, as an alternative of all operation choices
# - setup precedence constraints between operations of each job
# - fills machine_operations structure that list all operations that reside on a machine

# Initialize working variables
job_number = {}      # Job_number for each operation choice (for display)
all_operations = []  # List of all operations
machine_operations = [[] for m in range(NB_MACHINES)] # All choices per machine

# Loop on all jobs/operations/choices
for jx, job in enumerate(JOBS):
    op_vars = []
    for ox, op in enumerate(job):
        choice_vars = []
        for cx, (m, d) in enumerate(op):
            cv = mdl.interval_var(name="J{}_O{}_C{}_M{}".format(jx, ox, cx, m), optional=True, size=d)
            job_number[cv.get_name()] = jx
            choice_vars.append(cv)
            machine_operations[m].append(cv)
        # Create alternative
        jv = mdl.interval_var(name="J{}_O{}".format(jx, ox))
        mdl.add(mdl.alternative(jv, choice_vars))
        op_vars.append(jv)
        # Add precedence
        if ox > 0:
            mdl.add(mdl.end_before_start(op_vars[ox - 1], op_vars[ox]))
    all_operations.extend(op_vars)

# Add no_overlap constraint between operations executed on the same machine
for lops in machine_operations:
    mdl.add(mdl.no_overlap(lops))

# Minimize termination date
mdl.add(mdl.minimize(mdl.max([mdl.end_of(op) for op in all_operations])))


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

# Solve model
print("Solving model....")
msol = mdl.solve(FailLimit=100000, TimeLimit=10)
print("Solution: ")
msol.print_solution()

# Draw solution
if msol and visu.is_visu_enabled():
    visu.timeline("Solution for flexible job-shop " + filename)
    visu.panel("Machines")
    for j in range(NB_MACHINES):
        visu.sequence(name='M' + str(j))
        for v in machine_operations[j]:
            itv = msol.get_var_solution(v)
            if itv.is_present():
                jn = job_number[v.get_name()]
                visu.interval(itv, jn, 'J' + str(jn))
    visu.show()
