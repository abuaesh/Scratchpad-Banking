# Scratchpad-Banking
This project exhibits a novel dynamic programming algorithm to solve the scratchpad partitioning problem, generally tuned toward data-intensive applications.

## There are 10 files in this repository:

1. _Area_costs.h, energy_costs.h and performance_costs.h:_ These contain data gathered from the memory simulator CACTI(See [CACTI github repo](https://github.com/abuaesh/CACTI-Batch-Generator) for more info). The data in these header files contain the needed information for the cost function that will evaluate the cost of the bankings in the algorithm.
2. _Trace.h_: The array in this file contains the memory access trace to the main signal in the embedded program after running it. This array is used to determine the cost of a banking too, depending on how many times the element is to be accessed.
3. _Dyn_ours_area.cpp, dyn_ours_energy.cpp and dyn_ours_perf.cpp:_ Each of these files is the implementation of the algorithm with a different cost objective, to either optimize the chip area, the energy consumption or the performance, respectively. So the only difference between these three files is the cost function.
4. _Area_optimization_results.cpp, energy_optimization_results.cpp and performance_optimization_results.cpp in the results folder:_ These files contain the results of running the program with area, energy and performance optimization, respectively.

## Running the program:
To run the algorithm, download the repo to your machine then compile run any of the .cpp programs to get the resulting files with the banking positions and the cost of each banking configuration.To find the solution banking for any given embedded signal, replace the trace array _A_ in _Trace.h_ with the trace of memory accesses to the given signal.
