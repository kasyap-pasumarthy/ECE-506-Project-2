Cache Coherence Simulator

For this project, we were given the code for a working L1 data cache and asked to implement three coherence protocols - MSI, MESI, and DRAGON. We fixed the number of processors at 4 and used the given trace files to run the simulator. The report was made to meet the grading requirements and thus does not go into much detail regarding the working of the simulator.

To run the simulator, type 'make' in console to run the Makefile and compile the code. Once that's done, give command line inputs as follows:

./smp_cache <cache_size>  <assoc> <block_size> <num_processors> <protocol> <trace_file>

* `smp_cache`: Executable of the SMP simulator generated after making. 
* `cache_size`: Size of each cache in the system (all caches are of the same size)
* `assoc`: Associativity of each cache (all caches are of the same associativity)
* `block_size`:  Block size of each cache line (all caches are of the same block size) 
* `num_processors`: Number of processors in the system (represents how many caches should be instantiated) 
* `protocol`: Coherence protocol to be used (0: MSI, 1:MESI, 2:Dragon) 
* `trace_file`: The input file that has the multi threaded workload trace. 

The simulator outputs everything to the console.
