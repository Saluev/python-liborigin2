python-liborigin2
---------------------

This code is an (almost) standalone library for reading OriginLab project files.

It is based on the code at
 * http://sourceforge.net/projects/liborigin
 * http://soft.proindependent.com/liborigin2

AUTHORS:  Stefan Gerlach, Ion Vasilief, Alex Kargovsky, Miquel Garriga

PYTHON WRAPPING: Tigran Saluev

Dependencies
---------------------------------------------------------------------------
To compile, liborigin (still) depends on
 * BOOST C++ libraries  http://www.boost.org/
			boost/algorithm/string.hpp, boost/variant.hpp and its dependencies.
 * tree.hh (included) http://tree.phi-sci.com/
 * latest version of Cython http://cython.org/
 * Doxygen (optional, just for documentation) https://doxygen.nl/

Note that the BOOST libraries are not needed at run time, neither are linked in the executable.
For Ubuntu users both BOOST and Doxygen can be installed via package manager:

	sudo apt install build-essential libboost-system-dev libboost-thread-dev libboost-program-options-dev libboost-test-dev doxygen

Compiling
---------------------------------------------------------------------------
liborigin uses CMake for the building process.
CMake is available at http://www.cmake.org/ .

After installing CMake and the BOOST C++ library headers on your system, issue the following commands
to build .a liborigin2 library:

    $ mkdir build
    $ cd build
    $ cmake ../
    $ make
    $ doxygen Doxyfile
    $ cd ..

(You'll surely need Doxygen installed to build documentation.)

To build Python module, just type

    $ python setup.py build_ext --inplace

Python Usage
---------------------------------------------------------------------------

In your script, import the path to the `liborigin.pyx` pacakge using your favorite method (see https://fortierq.github.io/python-import/).

To import the content of an Origin project `.opj`, simply type:

```python
import liborigin
file_contents = liborigin.parseOriginFile("my_awesome_project.opj")
```

(Rename the directory with liborigin.so to liborigin first.)

Features
---------------------------------------------------------------------------
 * supports the import of 3.5 to latest (2017) projects.
 
---------------------------------------------------------------------------
