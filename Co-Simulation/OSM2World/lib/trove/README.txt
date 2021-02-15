
   GNU Trove: High performance collections for Java.

Objectives

   The GNU Trove library has two objectives:
    1. Provide "free" (as in "free speech" and "free beer"), fast,
       lightweight implementations of the java.util Collections API.
       These implementations are designed to be pluggable replacements
       for their JDK equivalents.
    2. Whenever possible, provide the same collections support for
       primitive types. This gap in the JDK is often addressed by using
       the "wrapper" classes (java.lang.Integer, java.lang.Float, etc.)
       with Object-based collections. For most applications, however,
       collections which store primitives directly will require less
       space and yield significant performance gains.

Hashtable techniques

   The Trove maps/sets use open addressing instead of the chaining
   approach taken by the JDK hashtables. This eliminates the need to
   create Map.Entry wrappper objects for every item in a table and so
   reduces the O (big-oh) in the performance of the hashtable algorithm.
   The size of the tables used in Trove's maps/sets is always a prime
   number, improving the probability of an optimal distribution of
   entries across the table, and so reducing the likelihood of
   performance-degrading collisions. Trove sets are not backed by maps,
   and so using a THashSet does not result in the allocation of an unused
   "values" array.

Hashing strategies

   Trove's maps/sets support the use of custom hashing strategies,
   allowing you to tune collections based on characteristics of the input
   data. This feature also allows you to define hash functions when it is
   not feasible to override Object.hashCode(). For example, the
   java.lang.String class is final, and its implementation of hashCode()
   takes O(n) time to complete. In some applications, however, it may be
   possible for a custom hashing function to save time by skipping
   portions of the string that are invariant.

   Using java.util.HashMap, it is not possible to use Java language
   arrays as keys. For example, this code:
    char[] foo, bar;
    foo = new char[] {'a','b','c'};
    bar = new char[] {'a','b','c'};
    System.out.println(foo.hashCode() == bar.hashCode() ?
      "equal" : "not equal");
    System.out.println(foo.equals(bar) ? "equal" : "not equal");
    

   produces this output:
    not equal
    not equal
    

   And so an entry stored in a java.util.HashMap with foo as a key could
   not be retrieved with bar, since there is no way to override
   hashCode() or equals() on language array objects.

   In a gnu.trove.map.hash.TCustomHashMap, however, you can implement a
   gnu.trove.strategy.HashingStrategy to enable hashing on arrays:
    class CharArrayStrategy implements HashingStrategy {
        public int computeHashCode(Object o) {
            char[] c = (char[])o;
            // use the shift-add-xor class of string hashing functions
            // cf. Ramakrishna and Zobel,
            //     "Performance in Practice of String Hashing Functions"
            int h = 31; // seed chosen at random
            for (int i = 0; i < c.length; i++) { // could skip invariants
                // L=5, R=2 works well for ASCII input
                h = h ^ ((h << 5) + (h >> 2) + c[i]);
            }
            return h;
        }

        public boolean equals(Object o1, Object o2) {
            char[] c1 = (char[])o1;
            char[] c2 = (char[])o2;
            // could drop this check for fixed-length keys
            if (c1.length != c2.length) {
                return false;
            }
            // could skip invariants
            for (int i = 0, len = c1.length; i < len; i++) {
                if (c1[i] != c2[i]) {
                    return false;
                }
            }
            return true;
        }
    }
    

Iterators in primitive collections

   Trove's primitive mappings include access through Iterators as well
   as procedures and functions. The API documentation on those classes
   contains several examples showing how these can be used effectively
   and explaining why their semantics differ from those of
   java.util.Iterator.

_________________________________________________________________

   Last modified: Sep 9, 2011
