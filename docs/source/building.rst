.. _building:
Building GIECS
==============

Requirements
------------
- C++11 compiler
- boost >= 1.60

To use Boost.Build the environment-variable BOOST_ROOT is required:
::

	export BOOST_ROOT=<your boost installation>


Run Unittests
-------------
::

	b2 test

Build Interpreters
------------------
::

	b2 languages/brainfuck
	b2 languages/forth
	b2 languages/lisp

