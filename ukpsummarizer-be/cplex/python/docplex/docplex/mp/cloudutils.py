# --------------------------------------------------------------------------
# Source file provided under Apache License, Version 2.0, January 2004,
# http://www.apache.org/licenses/
# (c) Copyright IBM Corp. 2015, 2016
# --------------------------------------------------------------------------


# gendoc: ignore

import six
import warnings

from docplex.mp.context import check_credentials


def is_in_docplex_worker():
    try:
        import docplex.worker.solvehook as worker_env
        hook = worker_env.get_solve_hook()
        if hook:
            return True
    except ImportError:
        pass
    return False


def context_must_use_docloud(__context, **kwargs):
    # NOTE: the argument CANNOT be named 'context' here as kwargs may well
    # contain a 'context' key
    #
    # returns True if context + kwargs require an execution on cloud
    # this happens in the following cases:
    # (i)  kwargs contains a "docloud_context" key (compat??)
    # (ii) both an explicit url and api_key appear in kwargs
    # (iv) the context's "solver.agent" is "docloud"
    # (v)  kwargs override agent to be "docloud"
    #
    # Always return false when in docplex worker to ignore url/keys/docloud
    # agent override in the worker
    if is_in_docplex_worker():
        return False
    docloud_agent_name = "docloud"  # this might change
    have_docloud_context = kwargs.get('docloud_context') is not None
    # TODO: remove have_api_key = get_key_in_kwargs(__context, kwargs)
    # TODO: remove have_url = get_url_in_kwargs(__context, kwargs)

    has_url_key_in_kwargs = False
    if 'url' in kwargs and 'key' in kwargs:
        has_url_key_in_kwargs = is_url_valid(kwargs['url'])

    context_agent_is_docloud = __context.solver.get('agent') == docloud_agent_name
    kwargs_agent_is_docloud = kwargs.get('agent') == docloud_agent_name
    return have_docloud_context \
           or has_url_key_in_kwargs \
           or context_agent_is_docloud \
           or kwargs_agent_is_docloud


def is_url_valid(url):
    return url is not None and isinstance(url, six.string_types) and \
        url.strip().lower().startswith('http')


def context_has_docloud_credentials(context, do_warn=True):
    have_credentials = False
    if context.solver.docloud:
        have_credentials, error_message = check_credentials(context.solver.docloud)
        if error_message is not None and do_warn:
            warnings.warn(error_message, stacklevel=2)
    return have_credentials
