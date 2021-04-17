# Copyright 2019 Xiang Gao and collaborators.
# This program is distributed under the MIT license.

from setuptools import setup
import os

dirname = os.path.dirname(os.path.abspath(__file__))
readme = os.path.join(dirname, 'README.md')

with open(readme, 'r') as fh:
    long_description = fh.read()

def get_version(version):
    distance = version.distance if version.distance is not None else 0
    v = f'{str(version.tag)}.{distance}'
    return v

setup(
    name='xonsh-tcg',
    use_scm_version={
        'root': '../../',
        'relative_to': __file__,
        'version_scheme': get_version,
        'local_scheme': lambda x:"",
        'fallback_version': 'unknown',
    },
    setup_requires=['setuptools_scm'],
    license='MIT',
    url='https://github.com/zasdfgbnm/tcg',
    description='tcg support for the Xonsh shell',
    long_description=long_description,
    author='Xiang Gao',
    author_email='tcg@emailaddress.biz',
    packages=['xontrib'],
    package_dir={'xontrib': 'xontrib'},
    package_data={'xontrib': ['*.xsh']},
)
