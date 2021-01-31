# Copyright 2019 Xiang Gao and collaborators.
# This program is distributed under the MIT license.

from setuptools import setup

with open('README.md', 'r') as fh:
    long_description = fh.read()

setup(
    name='xonsh-tcg',
    use_scm_version={
        'root': '../../',
        'version_scheme': 'post-release',
        # 'local_scheme': 'dirty-tag',
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
