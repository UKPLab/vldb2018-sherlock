from collections import defaultdict
from time import clock as timer

TIME = 't'
R1 = 'r1'
R2 = 'r2'
R4 = 'r4'


class Timer(object):

    def __init__(self):
        self.measurements = defaultdict(dict)

    def start_timer(self):
        self.t0 = timer()

    def stop_timer_for(self, i, score):
        self.measurements[i][TIME] = timer() - self.t0
        self.measurements[i][R1] = score[0]
        self.measurements[i][R2] = score[1]
        self.measurements[i][R4] = score[2]


class IterationTimer(Timer):
    """docstring for IterationTimer"""

    def __init__(self):
        super(IterationTimer, self).__init__()
        self.break_condition = 0

    def get_measurements(self):
        x = []
        r1 = []
        r2 = []
        r4 = []

        for i in sorted(self.measurements):
            x.append(self.measurements[i][TIME])
            r1.append(self.measurements[i][R1])
            r2.append(self.measurements[i][R2])
            r4.append(self.measurements[i][R4])

        return x, r1, r2, r4

    def set_break_condition(self, bc):
        self.break_condition = bc
        return

    def get_break_condition(self):
        return self.break_condition


class RunTimer(Timer):

    def __init__(self):
        super(RunTimer, self).__init__()

    def get_measurements(self):
        x = []
        y = []

        for k in sorted(self.measurements):
            x.append(k)
            y.append(self.measurements[k][TIME])

        return x, y

    def get_measurements_for(self, k):
        return self.measurements[k][TIME], self.measurements[k][R1], self.measurements[k][R2], self.measurements[k][R4]
