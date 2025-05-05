<pre>
Requires C++ Boost Library to be installed. <a href="https://www.boost.org">https://www.boost.org</a>
  
[alexander@alexander-home Point_Search_CPP2]$ ./generate_bloom
[00:01:48] P_table generated
[00:01:48] Range Start: 51 bits
[00:01:48] Range End  : 52 bits
[00:01:48] Block Width: 2^25
[00:01:48] Search Pub : 033195139de0331d7a5cab602c4471f728f2e3fb97ed82f593d49ed30ec3c0ba85
[00:01:48] Settings written to file
[00:01:48] Creating BloomFile2
[00:01:48] Creating BloomFile1
[00:05:43] Writing BloomFile2 to bloom2.bf
[00:05:43] Writing BloomFile1 to bloom1.bf
[00:05:43] Elapsed time: (0)hours (3)minutes (55)seconds

[alexander@alexander-home Point_Search_CPP2]$ ./point_search
[23:56:39] S_table generated
[23:56:39] Range Start: 51 bits
[23:56:39] Range End  : 52 bits
[23:56:39] Block Width: 2^25
[23:56:39] Search Pub : 033195139de0331d7a5cab602c4471f728f2e3fb97ed82f593d49ed30ec3c0ba85
[23:56:39] Loading Bloomfilter bloom1.bf
[23:56:40] Loading Bloomfilter bloom2.bf
[23:56:40] Search in progress...
[23:57:12] BloomFilter Hit bloom2.bf (Odd Point) [Lower Range Half]
[23:57:12] Privatekey: 000000000000000000000000000000000000000000000000000ad89e2c8e65c3
[23:57:12] Elapsed time: (0)hours (0)minutes (32)seconds]

</pre>
