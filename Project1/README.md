Emre Karataş - 22001641
İpek Öztaş - 22003250

Part A (50 pts): In this project you will develop an application that will find
the K most frequently occurring words, i.e., top-K words, in a given input
data set (K is an positive integer). In part A, the application will use multiple
child processes to process the data set. In part B, it will use multiple threads.
Hence, you will develop two programs, that will essentially do the same
thing. The data set will contain N input files. Child processes will use shared
memory to pass information to the parent. The shared memory segment will
be created by the parent. Each child process will access and use a portion of
the shared memory.

Part B (25 pts): In this part, do the same project by using threads, instead of
child processes. The name of the program will be threadtopk in this case.
The program will have the following parameters. Their meaning is the same
with proctopk.

Part C – Experiments (25 pts):
Do some measurement experiments, and record, plot and interpret your data.
Measure the time it takes to run the programs A and B for various values of N
(fix K to a value). Measure the time it takes to run the programs A and B for
various K values (fix N to a value). Compare and try interpret the results.
