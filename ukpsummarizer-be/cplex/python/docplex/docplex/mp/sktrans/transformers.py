# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2017
# --------------------------------------------------------------------------



from sklearn.base import TransformerMixin, BaseEstimator
from docplex.mp.constants import ObjectiveSense
from docplex.mp.advmodel import AdvModel
from docplex.mp.utils import *

import numpy as np
from pandas import DataFrame


class CplexTransformerBase(BaseEstimator, TransformerMixin):
    """ Root class for CPLEX transformers
    """

    def __init__(self, sense="min", keep_zeros=False):
        self.sense = ObjectiveSense.parse(sense)  # fail if error
        self.keep_zeros = keep_zeros

    def fit(self, *_):
        return self

    def transform(self, X, y=None, **transform_params):
        """ Main method to solve Linear Programming problemss.

        :param X: the matrix describing the  constraints of the problem.
            Accepts numpy matrices, pandas dataframes, or sciPy sparse matrices
        :param y: an optional sequence of scalars descrining the cost vector
        :param transform_params: optional keyword arguments to pass additional parameters.

        :return: a pandas dataframe with two columns: name and value containing the values
        of the columns.
        """
        # look for upper, lower bound columns in keyword args
        var_lbs = transform_params.get("lbs", None)
        var_ubs = transform_params.get("ubs", None)
        if is_pandas_dataframe(X):
            return self._transform_from_pandas(X, y, var_lbs, var_ubs, **transform_params)
        elif is_numpy_matrix(X):
            return self._transform_from_numpy(X, y, var_lbs, var_ubs, **transform_params)
        elif is_scipy_sparse(X):
            return self._transform_from_scsparse(X, y, var_lbs, var_ubs, **transform_params)
        elif isinstance(X, list):
            return self._transform_from_sequence(X, y, var_lbs, var_ubs, **transform_params)
        else:
            raise ValueError(
                'transformer expects pandas dataframe, numpy matrix or python list, {0} was passed'.format(X))

    def _transform_from_pandas(self, X, y, var_lbs, var_ubs, **transform_params):
        raise NotImplemented

    def _transform_from_numpy(self, X, y, var_lbs, var_ubs, **transform_params):
        raise NotImplemented

    def _transform_from_scsparse(self, X, y, var_lbs, var_ubs, **transform_params):
        raise NotImplemented

    def _transform_from_sequence(self, X, y, var_lbs, var_ubs, **transform_params):
        # by default, convert X to a numpy matrix
        return self._transform_from_numpy(np.matrix(X), y, var_lbs, var_ubs, **transform_params)

    def _solve_model(self, mdl, cols, colnames, costs, **params):
        if costs is not None:
            mdl.set_objective(sense=self.sense, expr=mdl.scal_prod_vars_all_different(cols, costs))

        # --- lp export
        lp_export = params.pop('lp_export', False)
        lp_base = params.pop('lp_basename', None)
        lp_path = params.pop('lp_path', None)
        if lp_export:
            mdl.export_as_lp(basename=lp_base, path=lp_path)
        # ---

        s = mdl.solve()
        if s:
            dd = {'value': s.get_values(cols)}
            if colnames is not None:
                dd['name'] = colnames
            ret = DataFrame(dd)
            if not self.keep_zeros:
                ret = ret[ret['value'] != 0]
                ret = ret.reset_index(drop=True)

            return ret

        else:
            return self.new_empty_dataframe()

    @classmethod
    def new_empty_dataframe(cls):
        return DataFrame([])


class LPTransformer(CplexTransformerBase):
    """ A Scikit-learn transformer class to solve linear problems.

    This transformer class solves LP problems of
    type
            Ax <= B


    """

    def __init__(self, sense="min"):
        """
        Creates an instance of LPTransformer to solve linear problems.

        :param sense: defines the objective sense. Accepts 'min" or "max" (not case-sensitive),
            or an instance of docplex.mp.ObjectiveSense

        Note:
            The matrix X is supposed to have shape (M,N+1) where M is the number of rows
            and N the number of variables. The last column contains the right hand sides of the problem
            (the B in Ax <= B)
            The optional vector Y contains the N cost coefficients for each column variables.

        Example:
            Passing X = [[1,2,3], [4,5,6]], Y= [11,12,13] means solving the linear problem:

                minimize 11x + 12y + 13z
                s.t.
                        1x + 2y <= 3
                        4x + 5y <= 6
        """
        super(LPTransformer, self).__init__(sense)

    def _transform_from_pandas(self, X, y, var_lbs, var_ubs, **transform_params):
        assert is_pandas_dataframe(X)

        X_new = X.copy()
        # save min, max per nutrients in lists, drop them
        rhs = X["rhs"].tolist()
        X_new.drop(labels=["rhs"], inplace=True, axis=1)

        with AdvModel(name='lp_transformer') as mdl:
            x_rows, x_cols = X.shape
            nb_vars = x_cols - 1

            varlist = mdl.continuous_var_list(nb_vars, lb=var_lbs, ub=var_ubs)
            senses = transform_params.get('sense', 'le')
            mdl.add(mdl.matrix_constraints(X_new, varlist, rhs, sense=senses))
            return self._solve_model(mdl, varlist, colnames=X_new.columns, costs=y, **transform_params)

    def _transform_from_numpy(self, X, y, var_lbs, var_ubs, **transform_params):
        # matrix is nrows x (ncols + 2)
        # last two columns are lbs, ubs in that order
        assert is_numpy_matrix(X)

        colnames = transform_params.get("colnames", None)
        mshape = X.shape
        xr, xc = mshape
        assert xc >= 2
        nb_vars = xc - 1
        X_cts = X[:, :-1]
        rhs = X[:, -1].A1
        with AdvModel(name='lp_transformer') as mdl:
            varlist = mdl.continuous_var_list(nb_vars, lb=var_lbs, ub=var_ubs, name=colnames)
            senses = transform_params.get('sense', 'le')
            mdl.add(mdl.matrix_constraints(X_cts, varlist, rhs, sense=senses))
            return self._solve_model(mdl, varlist, colnames, costs=y, **transform_params)

    def _transform_from_scsparse(self, X, y, var_lbs, var_ubs, **transform_params):
        assert is_scipy_sparse(X)

        colnames = transform_params.get("colnames", None)
        mshape = X.shape
        nr, nc = mshape
        assert nc == nr + 1
        nb_vars = nc - 1
        with AdvModel(name='lp_transformer') as mdl:
            varlist = mdl.continuous_var_list(nb_vars, lb=var_lbs, ub=var_ubs)
            lfactory = mdl._lfactory
            r_rows = range(nr)
            exprs = [lfactory.linear_expr() for _ in r_rows]
            rhss = [0] * nr
            #  convert to coo before iterate()
            x_coo = X.tocoo()
            for coef, row, col in izip(x_coo.data, x_coo.row, x_coo.col):
                if col < nr:
                    exprs[row]._add_term(varlist[col], coef)
                else:
                    rhss[row] = coef
            senses = transform_params.get('sense', 'le')
            cts = [lfactory.new_binary_constraint(exprs[r], rhs=rhss[r], sense=senses) for r in r_rows]
            lfactory._post_constraint_block(cts)
            return self._solve_model(mdl, varlist, colnames, costs=y, **transform_params)


class LPRangeTransformer(CplexTransformerBase):
    def __init__(self, sense="min"):
        """
        Creates an instance of LPRangeTransformer to solve range-based linear problems.

        :param sense: defines the objective sense. Accepts 'min" or "max" (not case-sensitive),
            or an instance of docplex.mp.ObjectiveSense

        Note:
            The matrix X is supposed to have shape (M,N+2) where M is the number of rows
            and N the number of variables.
            The last two columns are assumed to contain the minimum (resp.maximum) values for the
            row ranges, that m and M in:
                    m <= Ax <= M

            The optional vector Y contains the N cost coefficients for each column variables.

        Example:
            Passing X = [[1,2,3,30], [4,5,6,60]], Y= [11,12,13] means solving the linear problem:

                minimize 11x + 12y + 13z
                s.t.
                        3 <= 1x + 2y <= 30
                        6 <= 4x + 5y <= 60
        """
        super(LPRangeTransformer, self).__init__(sense)

    def _transform_from_pandas(self, X, y, var_lbs, var_ubs, **transform_params):
        assert is_pandas_dataframe(X)

        x_rows, x_cols = X.shape
        X_new = X.copy()
        # extract columns with name 'min' and 'max' as series then drop
        row_mins = X["min"].tolist()
        row_maxs = X["max"].tolist()
        X_new.drop(labels=["min", "max"], inplace=True, axis=1)

        with AdvModel(name='lp_range_trasnformer') as mdl:
            nb_vars = x_cols - 2
            varlist = mdl.continuous_var_list(nb_vars, lb=var_lbs, ub=var_ubs)
            mdl.add(mdl.matrix_ranges(X_new, varlist, row_mins, row_maxs))
            return self._solve_model(mdl, varlist, colnames=X_new.columns, costs=y, **transform_params)

    def _transform_from_numpy(self, X, y, var_lbs, var_ubs, **transform_params):
        # matrix is nrows x (ncols + 2)
        # last two columns are lbs, ubs in that order
        assert is_numpy_matrix(X)

        colnames = transform_params.pop("colnames", None)
        mshape = X.shape
        xr, xc = mshape
        assert xc >= 3
        nb_vars = xc - 2
        X_cts = X[:, :-2]
        row_mins = X[:, -2]
        row_maxs = X[:, -1]
        with AdvModel(name='lp_range_transformer') as mdl:
            varlist = mdl.continuous_var_list(nb_vars, lb=var_lbs, ub=var_ubs, name=colnames)
            mdl.add(mdl.matrix_ranges(X_cts, varlist, row_mins, row_maxs))
            return self._solve_model(mdl, varlist, colnames, costs=y, **transform_params)
