from setuptools import setup, find_packages

setup(
    name='pyvbz',
    version='1.0.1',
    url='https://nanoporetech.com',
    author='Oxford Nanopore Technologies, Limited',
    author_email="support@nanoporetech.com",
    packages=find_packages(),
    description="Python bindings to libvbz",
    install_requires=['numpy', 'cffi'],
    setup_requires=['cffi', 'wheel'],
    cffi_modules=['vbz/build.py:ffibuilder'],
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: Mozilla Public License 2.0',
        'Natural Language :: English',
        'Operating System :: POSIX :: Linux',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 3',
        'Topic :: Scientific/Engineering :: Bio-Informatics',
    ]
)
