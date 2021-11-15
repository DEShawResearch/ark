# Ark

Ark is a json-like file format widely used in DESRES. This module provides C++ and python interfaces for working with arks.  Ark currently supports Python 3.7+.

Quick build
-----------

To build Ark, you will need the following on a Unix-family operating system:

 * A recent gcc compiler supporting C++11; we have built with gcc 8.1.0.

 * python 3.7 or greater (https://www.python.org).

 * Scons (https://scons.org), a build tool, available through `pip install`.

 * pybind11 (https://pybind11.readthedocs.io/en/stable/).

 * Sphinx (https://www.sphinx-doc.org/) along with the `nbsphinx` and 'sphinx_argparse' extensions (available via `pip install nbsphinx sphinx_argparse`), to build the documentation.

 * Doxygen (https://www.doxygen.nl/).

Make sure that g++, scons, doxygen, and python are in your path.  Set PYBIND11PATH to the absolute path to root directory of your pybind11 install (directly above the pybind11 include/ subdirectory) and install Ark to a target directory by running

    ./install.sh <absolute path to install directory>

Running the tests
-----------------

Run the tests via

    PYTHONPATH=<install directory>/lib/python:$PYTHONPATH python -m pytest ./tests/test_* 