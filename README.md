# ECE486/586 Final Project: 5 Stage MIPS lite Pipeline Simulator

Team 3 Members: Meshal Almutairi, Abdulaziz Alateeqi, Yousef Alothman

## Project Overview

### Description
This project is a MIPS lite pipeline simulator that simulates the execution of MIPS instructions in a 5-stage pipeline. The simulator supports three modes of operation: non-pipelined, pipelined without forwarding, and pipelined with forwarding.


## Getting Started

### Compiling the Program
- **Default**: Use `make` to compile the program with the standard configuration.
- **Debug**: Use `make debug` to compile the program with additional debugging information

### Running the Program
To run the program, use the following command:
```
./mips_sim [-f filename] [-m mode]
```

Where:
- `filename` is the name of the input file containing the memory image.
- `mode` is the mode of the simulator (0: Non-pipelined, 1: Pipelined without forwarding, 2: Pipelined with forwarding).

#### Example
```
./mips_sim -f memory_image.txt -m 1
```

