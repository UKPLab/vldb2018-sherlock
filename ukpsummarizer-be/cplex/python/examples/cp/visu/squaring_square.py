# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

"""
The aim of the square example is to place a set of small squares of
different sizes into a large square.

See https://en.wikipedia.org/wiki/Squaring_the_square for details on this classical problem.

This version is extended and uses matplotlib to draw the result at the end.
Requires installation of numpy (installer) and following python packages:
    "pip install matplotlib python-dateutil pyparsing"

Please refer to documentation for appropriate setup of solving configuration.
"""

from docplex.cp.model import CpoModel
import docplex.cp.utils_visu as visu


#-----------------------------------------------------------------------------
# Initialize the problem data
#-----------------------------------------------------------------------------

# Size of the englobing square
SIZE_SQUARE = 112

# Sizes of the sub-squares
SIZE_SUBSQUARE = [50, 42, 37, 35, 33, 29, 27, 25, 24, 19, 18, 17, 16, 15, 11, 9, 8, 7, 6, 4, 2]
NB_SUBSQUARE = len(SIZE_SUBSQUARE)

#-----------------------------------------------------------------------------
# Build the model
#-----------------------------------------------------------------------------

# Create model
mdl = CpoModel()

# Create array of variables for subsquares
vx = [mdl.interval_var(size=SIZE_SUBSQUARE[i], name="X" + str(i), end=(0, SIZE_SQUARE)) for i in range(NB_SUBSQUARE)]
vy = [mdl.interval_var(size=SIZE_SUBSQUARE[i], name="Y" + str(i), end=(0, SIZE_SQUARE)) for i in range(NB_SUBSQUARE)]

# Create dependencies between variables
for i in range(len(SIZE_SUBSQUARE)):
    for j in range(i):
        mdl.add(  (mdl.end_of(vx[i]) <= mdl.start_of(vx[j])) | (mdl.end_of(vx[j]) <= mdl.start_of(vx[i]))
                | (mdl.end_of(vy[i]) <= mdl.start_of(vy[j])) | (mdl.end_of(vy[j]) <= mdl.start_of(vy[i])))

# To speed-up the search, create cumulative expressions on each dimension
rx = mdl.sum([mdl.pulse(vx[i], SIZE_SUBSQUARE[i]) for i in range(NB_SUBSQUARE)])
mdl.add(mdl.always_in(rx, (0, SIZE_SQUARE), SIZE_SQUARE, SIZE_SQUARE))

ry = mdl.sum([mdl.pulse(vy[i], SIZE_SUBSQUARE[i]) for i in range(NB_SUBSQUARE)])
mdl.add(mdl.always_in(ry, (0, SIZE_SQUARE), SIZE_SQUARE, SIZE_SQUARE))

# Define search phases, also to speed-up the search
mdl.set_search_phases([mdl.search_phase(vx), mdl.search_phase(vy)])


#-----------------------------------------------------------------------------
# Solve the model and display the result
#-----------------------------------------------------------------------------

# Solve model
print("Solving model....")
msol = mdl.solve(TimeLimit=20, LogPeriod=50000)
print("Solution: ")
msol.print_solution()

if msol and visu.is_visu_enabled():
    import matplotlib.pyplot as plt
    import matplotlib.cm as cm
    from matplotlib.patches import Polygon

    # Plot external square
    print("Plotting squares....")
    fig, ax = plt.subplots()
    plt.plot((0, 0), (0, SIZE_SQUARE), (SIZE_SQUARE, SIZE_SQUARE), (SIZE_SQUARE, 0))
    for i in range(len(SIZE_SUBSQUARE)):
        # Display square i
        sx, sy = msol.get_var_solution(vx[i]), msol.get_var_solution(vy[i])
        (sx1, sx2, sy1, sy2) = (sx.get_start(), sx.get_end(), sy.get_start(), sy.get_end())
        poly = Polygon([(sx1, sy1), (sx1, sy2), (sx2, sy2), (sx2, sy1)], fc=cm.Set2(float(i) / len(SIZE_SUBSQUARE)))
        ax.add_patch(poly)
        # Display identifier of square i at its center
        ax.text(float(sx1 + sx2) / 2, float(sy1 + sy2) / 2, str(SIZE_SUBSQUARE[i]), ha='center', va='center')
    plt.margins(0)
    plt.show()
