Building GIECS
==============

GIECS is a C++ header-only template library. In order to use it, you only have to set your includepath to the root of the giecs-directory.

Requirements
------------
- C++11 compiler
- boost >= 1.60

Using Boost.Build
-----------------
The testcases and examples are built using Boost.Build.
To include GIECS using Boost.Build, the following option in your jamfile is enough::

	<library>/giecs


Overall, Boost.Build requires the environment-variable BOOST_ROOT to be set::

	export BOOST_ROOT=<your boost installation>


Run Unittests
^^^^^^^^^^^^^
::

	b2 test

Build Interpreters
^^^^^^^^^^^^^^^^^^
::

	b2 languages/brainfuck
	b2 languages/forth
	b2 languages/lisp



