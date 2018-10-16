# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------

# Source: http://blog.yhathq.com/posts/how-yhat-does-cloud-balancing.html

from collections import namedtuple
import json

from docplex.util.environment import get_environment
from docplex.mp.absmodel import AbstractModel


# ----------------------------------------------------------------------------
# Initialize the problem data
# ----------------------------------------------------------------------------
class TUser(namedtuple("TUser", ["id", "running", "sleeping", "current_server"])):
    def __str__(self):
        return self.id

SERVERS = ["server002", "server003", "server001", "server006", "server007", "server004", "server005"]

USERS = [("user013", 2, 1, "server002"),
         ("user014", 0, 2, "server002"),
         ("user015", 0, 4, "server002"),
         ("user016", 1, 4, "server002"),
         ("user017", 0, 3, "server002"),
         ("user018", 0, 2, "server002"),
         ("user019", 0, 2, "server002"),
         ("user020", 0, 1, "server002"),
         ("user021", 4, 4, "server002"),
         ("user022", 0, 1, "server002"),
         ("user023", 0, 3, "server002"),
         ("user024", 1, 2, "server002"),
         ("user025", 0, 1, "server003"),
         ("user026", 0, 1, "server003"),
         ("user027", 1, 1, "server003"),
         ("user028", 0, 1, "server003"),
         ("user029", 2, 1, "server003"),
         ("user030", 0, 5, "server003"),
         ("user031", 0, 2, "server003"),
         ("user032", 0, 3, "server003"),
         ("user033", 1, 1, "server003"),
         ("user034", 0, 1, "server003"),
         ("user035", 0, 1, "server003"),
         ("user036", 4, 1, "server003"),
         ("user037", 7, 1, "server003"),
         ("user038", 2, 1, "server003"),
         ("user039", 0, 3, "server003"),
         ("user040", 1, 2, "server003"),
         ("user001", 0, 2, "server001"),
         ("user002", 0, 3, "server001"),
         ("user003", 5, 4, "server001"),
         ("user004", 0, 1, "server001"),
         ("user005", 0, 1, "server001"),
         ("user006", 0, 2, "server001"),
         ("user007", 0, 4, "server001"),
         ("user008", 0, 1, "server001"),
         ("user009", 5, 1, "server001"),
         ("user010", 7, 1, "server001"),
         ("user011", 4, 5, "server001"),
         ("user012", 0, 4, "server001"),
         ("user062", 0, 1, "server006"),
         ("user063", 3, 5, "server006"),
         ("user064", 0, 1, "server006"),
         ("user065", 0, 3, "server006"),
         ("user066", 3, 1, "server006"),
         ("user067", 0, 1, "server006"),
         ("user068", 0, 1, "server006"),
         ("user069", 0, 2, "server006"),
         ("user070", 3, 2, "server006"),
         ("user071", 0, 1, "server006"),
         ("user072", 5, 3, "server006"),
         ("user073", 0, 1, "server006"),
         ("user074", 0, 1, "server006"),
         ("user075", 0, 2, "server007"),
         ("user076", 1, 1, "server007"),
         ("user077", 1, 1, "server007"),
         ("user078", 0, 1, "server007"),
         ("user079", 0, 3, "server007"),
         ("user080", 0, 1, "server007"),
         ("user081", 4, 1, "server007"),
         ("user082", 1, 1, "server007"),
         ("user041", 0, 1, "server004"),
         ("user042", 2, 1, "server004"),
         ("user043", 5, 2, "server004"),
         ("user044", 5, 2, "server004"),
         ("user045", 0, 2, "server004"),
         ("user046", 1, 5, "server004"),
         ("user047", 0, 1, "server004"),
         ("user048", 0, 3, "server004"),
         ("user049", 5, 1, "server004"),
         ("user050", 0, 2, "server004"),
         ("user051", 0, 3, "server004"),
         ("user052", 0, 3, "server004"),
         ("user053", 0, 1, "server004"),
         ("user054", 0, 2, "server004"),
         ("user055", 0, 3, "server005"),
         ("user056", 3, 1, "server005"),
         ("user057", 0, 3, "server005"),
         ("user058", 0, 2, "server005"),
         ("user059", 0, 1, "server005"),
         ("user060", 0, 5, "server005"),
         ("user061", 0, 2, "server005")
         ]

# ----------------------------------------------------------------------------
# Prepare the data for modeling
# ----------------------------------------------------------------------------
DEFAULT_MAX_PROCESSES_PER_SERVER = 50


# ----------------------------------------------------------------------------
# Build the model
# ----------------------------------------------------------------------------
class LoadBalancingModel(AbstractModel):
    def __init__(self, **kwargs):
        AbstractModel.__init__(self, 'load_balancing', **kwargs)
        # raw data
        self.max_processes_per_server = DEFAULT_MAX_PROCESSES_PER_SERVER
        self.servers = []
        self.users = []
        # decision objects
        self.active_var_by_server = {}
        self.assign_user_to_server_vars = {}
        self.number_of_active_servers = None
        self.number_of_migrations = None
        self.max_sleeping_workload = None

    def load_data(self, servers, users, max_process_per_server=DEFAULT_MAX_PROCESSES_PER_SERVER):
        self.servers = servers
        self.users = [TUser(*user_row) for user_row in users]
        self.max_processes_per_server = max_process_per_server

    def clear(self):
        AbstractModel.clear(self)
        self.active_var_by_server = {}
        self.assign_user_to_server_vars = {}
        self.number_of_active_servers = None

    def setup_variables(self):
        all_servers = self.servers
        all_users = self.users

        self.active_var_by_server = self.binary_var_dict(all_servers, name='isActive')

        def user_server_pair_namer(u_s):
            u, s = u_s
            return '%s_to_%s' % (u.id, s)

        self.assign_user_to_server_vars = self.binary_var_matrix(all_users, all_servers, user_server_pair_namer)

    @staticmethod
    def _is_migration(user, server):
        """ Returns True if server is not the user's current
            Used in setup of constraints.
        """
        return server != user.current_server

    def setup_constraints(self):
        mdl = self
        all_servers = self.servers
        all_users = self.users

        max_proc_per_server = self.max_processes_per_server

        mdl.add_constraints(
            mdl.sum(self.assign_user_to_server_vars[u, s] * u.running for u in all_users) <= max_proc_per_server
            for s in all_servers)

        # each assignment var <u, s>  is <= active_server(s)
        for s in all_servers:
            for u in all_users:
                ct_name = 'ct_assign_to_active_{0!s}_{1!s}'.format(u, s)
                mdl.add_constraint(self.assign_user_to_server_vars[u, s] <= self.active_var_by_server[s], ct_name)

        # sum of assignment vars for (u, all s in servers) == 1
        for u in all_users:
            ct_name = 'ct_unique_server_%s' % (u[0])
            mdl.add_constraint(mdl.sum((self.assign_user_to_server_vars[u, s] for s in all_servers)) == 1.0, ct_name)

    def setup_objective(self):
        mdl = self
        self.number_of_active_servers = mdl.sum((self.active_var_by_server[svr] for svr in self.servers))
        self.add_kpi(self.number_of_active_servers, "Number of active servers")

        self.number_of_migrations = mdl.sum(
            self.assign_user_to_server_vars[u, s] for u in self.users for s in self.servers if
            self._is_migration(u, s))
        mdl.add_kpi(self.number_of_migrations, "Total number of migrations")

        max_sleeping_workload = mdl.integer_var(name="max_sleeping_processes")
        for s in self.servers:
            ct_name = 'ct_define_max_sleeping_%s' % s
            mdl.add_constraint(
                mdl.sum(
                    self.assign_user_to_server_vars[u, s] * u.sleeping for u in self.users) <= max_sleeping_workload,
                ct_name)
        mdl.add_kpi(max_sleeping_workload, "Max sleeping workload")
        self.max_sleeping_workload = max_sleeping_workload
        # Set objective function
        mdl.minimize(self.number_of_active_servers)

    def run(self, **kwargs):
        mdl = self
        mdl.ensure_setup()
        mdl.print_information()
        # build an ordered sequence of goals
        ordered_kpi_keywords = ["servers", "migrations", "sleeping"]
        ordered_goals = [mdl.kpi_by_name(k) for k in ordered_kpi_keywords]

        return mdl.solve_lexicographic(ordered_goals, **kwargs)

    def report(self):
        mdl = self
        active_servers = sorted([s for s in mdl.servers if mdl.active_var_by_server[s].solution_value == 1])
        print("Active Servers: {}".format(active_servers))
        print("*** User assignment ***")
        for (u, s) in sorted(mdl.assign_user_to_server_vars):
            if mdl.assign_user_to_server_vars[(u, s)].solution_value == 1:
                print("{} uses {}, migration: {}".format(u, s, "yes" if mdl._is_migration(u, s) else "no"))
        print("*** Servers sleeping processes ***")
        for s in active_servers:
            sleeping = sum(self.assign_user_to_server_vars[u, s].solution_value * u.sleeping for u in self.users)
            print("Server: {} #sleeping={}".format(s, sleeping))

    def save_solution_as_json(self, json_file):
        """Saves the solution for this model as JSON.

        Note that this is not a CPLEX Solution file, as this is the result of post-processing a CPLEX solution
        """
        mdl = self
        solution_dict = {}
        # active server
        active_servers = sorted([s for s in mdl.servers if mdl.active_var_by_server[s].solution_value == 1])
        solution_dict["active servers"] = active_servers

        # sleeping processes by server
        sleeping_processes = {}
        for s in active_servers:
            sleeping = sum(self.assign_user_to_server_vars[u, s].solution_value * u.sleeping for u in self.users)
            sleeping_processes[s] = sleeping
        solution_dict["sleeping processes by server"] = sleeping_processes

        # user assignment
        user_assignment = []
        for (u, s) in sorted(mdl.assign_user_to_server_vars):
            if mdl.assign_user_to_server_vars[(u, s)].solution_value == 1:
                n = {
                    'user': u.id,
                    'server': s,
                    'migration': "yes" if mdl._is_migration(u, s) else "no"
                }
                user_assignment.append(n)
        solution_dict['user assignment'] = user_assignment
        json_file.write(json.dumps(solution_dict, indent=3).encode('utf-8'))


class DefaultLoadBalancingModel(LoadBalancingModel):
    def __init__(self, context=None, **kwargs):
        LoadBalancingModel.__init__(self, context=context, **kwargs)
        self.load_data(SERVERS, USERS)


# ----------------------------------------------------------------------------
# Solve the model and display the result
# ----------------------------------------------------------------------------
if __name__ == '__main__':
    """DOcplexcloud credentials can be specified with url and api_key in the code block below.

    Alternatively, Context.make_default_context() searches the PYTHONPATH for
    the following files:

        * cplex_config.py
        * cplex_config_<hostname>.py
        * docloud_config.py (must only contain context.solver.docloud configuration)

    These files contain the credentials and other properties. For example,
    something similar to::

       context.solver.docloud.url = "https://docloud.service.com/job_manager/rest/v1"
       context.solver.docloud.key = "example api_key"
    """
    url = None  # put your url here
    key = None  # put your api key here

    lbm = DefaultLoadBalancingModel()

    # Run the model. If a key has been specified above, the model will run on
    # IBM Decision Optimization on cloud.
    if lbm.run(url=url, key=key):
        lbm.report()
        with get_environment().get_output_stream("solution.json") as fp:
            lbm.save_solution_as_json(fp)
