import os
import re
from distutils.core import setup
import sys

required = ['requests',
            'six',
            'docloud>=1.0.257']
if ((sys.version_info[0]) < 3) or \
   ((sys.version_info[0] == 3) and (sys.version_info[1] < 2)):
    required.append('futures')

HERE = os.path.abspath(os.path.dirname(__file__))


def read(*parts):
    try:
        with open(os.path.join(HERE, *parts)) as f:
            return f.read()
    except:
        return None

readme = read('README.rst')
if readme is None:
    readme = 'DOcplex 2.4'

changelog = str(read('CHANGELOG.rst'))
if changelog is None:
    changelog = ''

ss = str(readme) + str(changelog)

setup(
    name='docplex',
    packages=['docplex',
               'docplex.cp',
               'docplex.cp.solver',
               'docplex.mp',
               'docplex.mp.internal',
               'docplex.mp.params',
               'docplex.mp.sktrans',
               'docplex.mp.worker',
               'docplex.util'],
    version = '2.4.61',  # replaced at build time
    description = 'The IBM Decision Optimization CPLEX Modeling for Python',
    author = 'The IBM Decision Optimization on Cloud team',
    author_email = 'dofeedback@wwpdl.vnet.ibm.com',
    long_description='%s\n' % ss,
    url = 'https://onboarding-oaas.docloud.ibmcloud.com/software/analytics/docloud/',
    keywords = ['docloud', 'optimization', 'cplex', 'cpo'],
    license = read('LICENSE.txt'),
    install_requires=required,
    classifiers = ["Development Status :: 5 - Production/Stable",
                   "Intended Audience :: Developers",
                   "Intended Audience :: Information Technology",
                   "Intended Audience :: Science/Research",
                   "Operating System :: Unix",
                   "Operating System :: MacOS",
                   "Operating System :: Microsoft",
                   "Operating System :: OS Independent",
                   "Topic :: Scientific/Engineering",
                   "Topic :: Scientific/Engineering :: Mathematics",
                   "Topic :: Software Development :: Libraries",
                   "Topic :: System",
                   "Topic :: Other/Nonlisted Topic",
                   "License :: OSI Approved :: Apache Software License",
                   "Programming Language :: Python",
                   "Programming Language :: Python :: 2.7",
                   "Programming Language :: Python :: 3.4",
                   "Programming Language :: Python :: 3.5",
                   "Programming Language :: Python :: 3.6"
                   ],
)

print("** The documentation can be found here: https://github.com/IBMDecisionOptimization/docplex-doc")
print("** The examples can be found here: https://github.com/IBMDecisionOptimization/docplex-examples")
