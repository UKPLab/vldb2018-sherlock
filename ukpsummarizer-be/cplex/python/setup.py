'''This script installs CPLEX and DOcplex.

All command line options are passed to both cplex and docplex setup.py.

The command:

    python setup.py install

invokes the scripts:

    python cplex/setup.py install
    python docplex/setup.py install

The command
    python setup.py install --home=yourPythonPackageshome

invokes the scripts:

    python cplex/setup.py install
    python docplex/setup.py install

'''
import glob
import os
import platform
import re
import shutil
import subprocess
import sys
import tarfile
import tempfile


try:
    # py3 only
    FileNotFoundError
except NameError:
    FileNotFoundError = IOError

try:
    import pip
    installed_packages = pip.get_installed_distributions()
    installed_packages_names = [package.project_name for package in installed_packages]
except:
    installed_packages_names = []

root = os.path.dirname(os.path.abspath(__file__))

# list of dependent packages to be installed
dependencies = ['certifi-2017.7.27.1',
                'chardet-3.0.4',
                'enum34-1.1.6',
                'idna-2.6',
                'requests-2.18.4',
                'six-1.10.0',
                'urllib3-1.22',
                'docloud-1.0.257']

# install futures only if needed
if ((sys.version_info[0]) < 3) or \
   ((sys.version_info[0] == 3) and (sys.version_info[1] < 2)):
    dependencies.append('futures-3.1.1')


def guess_cplex_path():
    python_version = "%s.%s" % (sys.version_info[0], sys.version_info[1])
    # cplex is in root/../cplex
    # the python wrappers are in a subdir python/python_version/port
    # and port/ contains cplex and setup.py
    g = os.path.join(os.path.dirname(root), 'cplex', 'python', python_version)
    cplex_dir = glob.glob(os.path.join(g, '*', 'cplex'))
    print(cplex_dir)
    setup_py = glob.glob(os.path.join(g, '*', 'setup.py'))
    print(setup_py)

    if len(cplex_dir) == 1 and len(setup_py) == 1:
        if os.path.isdir(cplex_dir[0]) and os.path.exists(setup_py[0]):
            return (os.path.dirname(setup_py[0]))
    return None


def guess_docplex_path():
    return os.path.join(root, 'docplex')


def unpack_and_install(package):
    archive = os.path.join('dependencies', 'python', '%s.tar.gz' % package)
    tmpd = tempfile.mkdtemp()
    print('   Unpack: %s in %s' % (archive, tmpd))
    try:
        with tarfile.open(archive, mode='r:gz') as tf:
            tf.extractall(tmpd)
        package_instdir = os.path.join(tmpd, package)
        print('   Installing from %s' % package_instdir)
        command = [python, 'setup.py'] + args
        r = subprocess.call(command, cwd=package_instdir)
        if r != 0:
            print("ERROR: Installation of %s failed, code = %s" % (package, r))
    finally:
        shutil.rmtree(tmpd)

if __name__ == '__main__':
    args = sys.argv[1:]
    python = sys.executable

    cplex_path = guess_cplex_path()
    if cplex_path is None:
        print("WARNING: No CPLEX python wrappers found.")

    install_deps = True
    if '--no-deps' in args:
        install_deps = False
        args.remove('--no-deps')
    if '--no-dependencies' in args:
        install_deps = False
        args.remove('--no-dependencies')

    # install cplex
    if cplex_path:
        command = [python, 'setup.py'] + args
        print("Invoking %s in %s" % (command, cplex_path))
        r = subprocess.call(command, cwd=cplex_path)
        if r != 0:
            print("Installation of CPLEX failed, code = %s" % r)
            exit(r)

    # install deps
    if 'install' in args and install_deps:
        for dep in dependencies:
            p = dep.split('-')[0]
            if p in installed_packages_names:
                print('Skip %s, already installed' % dep)
            else:
                print('Installing %s' % dep)
                unpack_and_install(dep)

    # install docplex
    command = [python, 'setup.py'] + args
    r = subprocess.call(command, cwd=guess_docplex_path())
    if r != 0:
        print("Could not install docplex, code = %s" % r)
        exit(r)
