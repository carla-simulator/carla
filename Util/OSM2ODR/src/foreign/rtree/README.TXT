
TITLE

	R-TREES: A DYNAMIC INDEX STRUCTURE FOR SPATIAL SEARCHING

DESCRIPTION

	A C++ templated version of the RTree algorithm.
	For more information please read the comments in RTree.h

AUTHORS

	* 1983 Original algorithm and test code by Antonin Guttman and Michael Stonebraker, UC Berkely
	* 1994 ANCI C ported from original test code by Melinda Green - melinda@superliminal.com
	* 1995 Sphere volume fix for degeneracy problem submitted by Paul Brook
	* 2004 Templated C++ port by Greg Douglas

LICENSE:

	Entirely free for all uses. Enjoy!

FILES
	* RTree.h            The C++ templated RTree implementation. Well commented.
	* Test.cpp           A simple test program, ported from the original C version.
	* MemoryTest.cpp     A more rigourous test to validate memory use.
	* README.TXT         This file.

TO BUILD

	To build a test, compile only one of the test files with RTree.h.  
	Both test files contain a main() function.

