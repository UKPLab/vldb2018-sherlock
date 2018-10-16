from setuptools import setup, find_packages

requirements = [l.strip() for l in open('requirements.txt').readlines()]

setup(
    name='${python.module.setup.name}',
    url='${python.module.setup.url}',
    version='${python.module.setup.version}',
    author='${python.module.setup.author}',
    author_email='${python.module.setup.author_email}',
    description='${python.module.setup.description}',
    packages=find_packages(),
    install_requires=requirements,
    include_package_data=True,
)
