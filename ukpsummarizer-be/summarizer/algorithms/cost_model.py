import numpy as np
import pandas as pd
from scipy.optimize import curve_fit, minimize, minimize_scalar


class CostModel(object):
    """"""
    def __init__(self, csv_folder):
        self.train_data = {}
        self.set_train_data(csv_folder)
        self.coefficients = {}
        self.estimate_t_constraint_functions()

    def __call__(self, p0=1, t_total=1):
        self.t_total = t_total
        # self.find_maximum_scalar()
        # self.solve_one_dimensional(p0)

        popt = self.estimate_quality_function(p0, neg=True)
        k = self.find_maximum(popt)
        return k

        # a* i * k
        # 1. minimizing: substitute i with t / stuff -> did not work
        # 2. constraint: i <= t_total / self.t(self.k_to_constraints(k))

    def estimate_quality_function(self, p0, neg=False):
        i = self.train_data['i']
        k = self.train_data['k']
        y = self.train_data['y']

        if neg:
            y = [-yi for yi in y]

        popt, pcov = curve_fit(self.quality, (k, i), y, p0=p0)
        return popt

    def find_maximum(self, popt):
        x0 = ([100, 10]) # initial guesses for k and i
        con1 = {'type': 'ineq', 'fun': self.budget_constraint}
        con2 = {'type': 'ineq', 'fun': self.i_constraint}
        con3 = {'type': 'ineq', 'fun': self.number_iterations_constraint}
        constraints = [con1, con2, con3]

        result = minimize(self.quality, x0, args=popt[0], method='COBYLA', constraints=[con1, con2, con3])
        # result = minimize(self.quality, x0, args=popt[0], method='COBYLA', options={'rhobeg': 0.1}, constraints=[con1, con2, con3])

        print(result)
        x = tuple([int(i) for i in result.x])
        self.check_constraints(x, constraints)
        return x[0]

    def quality(self, x, *args):
        '''Objective function'''
        k, i = x
        a = args[0]
        return a * i * k

    def budget_constraint(self, x):
        '''Constraint for time budget'''
        k, i = x
        return (self.t_total / self.t(self.k_to_constraints(k))) - i

    def i_constraint(self, x):
        '''i >= 0 constraint'''
        k, i = x
        return i

    def number_iterations_constraint(self, x):
        '''i should not be bigger than there are a possible number of iterations for a given k'''
        k, i = x
        return self.number_of_iterations(k) - i

    def set_train_data(self, csv_folder):
        df = pd.read_csv(csv_folder + 'iterations.csv')
        self.train_data['i'] = df['iteration']
        self.train_data['k'] = df['k']
        self.train_data['y'] = df['r2']
        self.train_data['t'] = df['t']
        self.train_data['constraints'] = df['constraints']

        maxit = pd.read_csv(csv_folder + 'maxit.csv')
        self.train_data['#i_k'] = maxit['k']
        self.train_data['#i_t'] = maxit['t']

    def t(self, constraints):
        a, b, c = self.coefficients['constraints_to_t']
        return self.d2poly(constraints, a, b, c)

    def constraints(self, t):
        a, b = self.coefficients['t_to_constraints']
        return self.d2root(t, a, b)
        # a, b, c = self.coefficients['t_to_constraints']
        # return self.d2poly(t, a, b, c)

    def number_of_iterations(self, k):
        a, b, c = self.coefficients['number_of_iterations']
        return self.d2poly(k, a, b, c)

    def k_to_constraints(self, k):
        # Stub
        pass

    def k_to_t(self, k):
        a, b, c = self.coefficients['k_to_t']
        return self.d2poly(k, a, b, c)

    def d2poly(self, x, a, b, c):
        return a * x**2 + b * x + c

    def d2root(self, x, a, b):
        return a * x**0.5 + b

    def estimate_t_constraint_functions(self):
        t, constraints, k = self.train_data['t'], self.train_data['constraints'], self.train_data['k']
        t_to_c = self.fit_t_c_function(t, constraints)
        # t_to_c = np.polyfit(t, constraints, 2)
        c_to_t = np.polyfit(constraints, t, 2)
        k_to_t = np.polyfit(k, t, 2)
        self.coefficients['t_to_constraints'] = t_to_c
        self.coefficients['constraints_to_t'] = c_to_t
        self.coefficients['k_to_t'] = k_to_t

        k = self.train_data['#i_k']
        t = self.train_data['#i_t']
        maxit = np.polyfit(k, t, 2)
        self.coefficients['number_of_iterations'] = maxit

    def set_time_budget(self, t_total):
        self.t_total = t_total

    def fit_t_c_function(self, t, c):
        x = t
        y = c
        popt, pcov = curve_fit(self.d2root, x, y, p0=(1, 0))
        return popt

    # One dimensional minimizing #############################
    def estimate_quality_budget_function(self, p0, neg=False):
        k = self.train_data['k'].astype(int)
        y = self.train_data['y']

        if neg:
            y = [-yi for yi in y]

        popt, pcov = curve_fit(self.quality_given_time_budget, k, y, p0=p0)
        return popt

    def find_maximum_scalar(self):
        res = minimize_scalar(self.quality_budget_scalar)
        print(res.x)

    def check_constraints(self, x, constraints):
        satisfy_dict = {True: "satisfied", False: "failed"}
        for con in constraints:
            name = con['fun'].__name__
            satisfied = False
            if con['type'] is 'ineq':
                satisfied = con['fun'](x) >= 0
            elif con['type'] is 'eq':
                satisfied = con['fun'](x) == 0
            print(name, ":", satisfy_dict[satisfied], con['fun'](x))

    def quality_given_time_budget(self, k, a):
        # IDEA: define time budget as a constraint
        return a * (self.t_total / self.t(self.k_to_constraints(k))) * k

    def quality_budget_scalar(self, k):
        return self.alpha * k * self.t_total / self.t(self.k_to_constraints(k))

    def find_maximum_one_dimensional(self, popt):
        print(popt[0])
        x0 = 100
        result = minimize(self.quality_given_time_budget, x0, args=popt[0])
        print(result)

    def solve_one_dimensional(self, p0):
        popt = self.estimate_quality_budget_function(p0, neg=True)
        self.find_maximum_one_dimensional(popt)

# csv_folder = ''
# cost_model = CostModel(csv_folder)
# cost_model(t_total=10)
