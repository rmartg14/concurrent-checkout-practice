# Concurrent Checkout Practice – Single Queue Simulation

A university practice project focused on **multithreading** and **concurrent programming**, implemented in **C** using POSIX threads (`pthreads`).

This repository contains the source code for a simulated **checkout system with a single shared queue**, where multiple customer threads and cashier threads interact concurrently. The project was developed as part of a university course on operating systems, concurrency, and parallel programming.

> This is an academic exercise designed to practise thread creation, synchronisation, shared-resource management, and concurrent access control in C.

## Overview

The program simulates a checkout scenario in which:

- Multiple **customer threads** arrive and join a **single shared queue**.
- One or more **cashier threads** serve customers from that queue.
- Access to the queue and related shared state is synchronised to avoid race conditions.
- The simulation runs for a defined period or until a certain number of customers have been processed.

The entire implementation is contained in a single C source file:

```text
Fila_Unica.c
```

This file includes:

- Thread creation and management.
- Queue data structure and operations.
- Synchronisation primitives (mutexes, condition variables, or semaphores, depending on the implementation).
- Logging or console output to visualise the concurrent behaviour.

## Educational Context

This project was developed as a **university practice** for a course on **multithreading and concurrent programming**. The main learning objectives include:

- Understanding the fundamentals of concurrent execution.
- Practising thread creation and lifecycle management with `pthreads`.
- Applying synchronisation mechanisms to protect shared resources.
- Avoiding common concurrency issues such as race conditions, deadlocks, and data inconsistencies.
- Designing a simple concurrent system from scratch in C.
- Observing and debugging non-deterministic behaviour typical of multithreaded programs.

The single-queue checkout scenario provides a clear and intuitive example of how multiple threads must coordinate access to shared data structures in a safe and efficient way.

## Features

- Simulation of a checkout system with a **single shared queue**.
- Multiple **customer threads** that join the queue concurrently.
- One or more **cashier threads** that process customers from the queue.
- Use of **POSIX threads (pthreads)** for concurrency.
- Synchronisation of shared resources using appropriate primitives (e.g., mutexes, condition variables).
- Console output or logging to visualise thread activity and queue state.
- Configurable parameters such as number of customers, cashiers, or simulation duration (depending on the implementation).

## Project Structure

```text
.
└── Fila_Unica.c    # Main (and only) source file with the complete implementation
```

All logic is implemented in `Fila_Unica.c`, including:

- Data structures for the queue and shared state.
- Customer and cashier thread functions.
- Synchronisation logic.
- Main function that initialises the simulation and launches threads.
- Optional logging or diagnostic output.

## Compilation and Execution

### Prerequisites

To compile and run this project, you need:

- A C compiler (e.g., `gcc` or `clang`).
- POSIX threads support (standard on Linux and macOS; available via MinGW or WSL on Windows).
- A Unix-like environment is recommended.

### Compilation

From the repository root, compile with:

```bash
gcc -pthread -o Fila_Unica Fila_Unica.c
```

The `-pthread` flag is required to link against the POSIX threads library.

### Execution

Run the compiled executable:

```bash
./Fila_Unica
```

Depending on the implementation, the program may accept command-line arguments to configure parameters such as:

- Number of customer threads.
- Number of cashier threads.
- Simulation duration or number of customers to process.

If supported, refer to comments in the source code or test different argument combinations to observe how concurrency behaviour changes.

## Concurrency Concepts Practised

This project applies several core concepts from concurrent and parallel programming:

- **Thread creation and management**: Spawning multiple customer and cashier threads that execute concurrently.
- **Shared resources**: A single queue accessed by all threads.
- **Synchronisation**: Protecting shared data with mutexes and/or condition variables to ensure consistency.
- **Coordination**: Ensuring cashiers wait when the queue is empty and customers wait when the queue is full (if bounded).
- **Race condition prevention**: Avoiding inconsistent states when multiple threads modify the queue simultaneously.
- **Non-deterministic execution**: Observing how thread scheduling affects the order of operations and output.

## Development History

This repository reflects an **iterative group development process**:

- Multiple commits from different contributors during a short time span.
- Progressive implementation and refinement of thread logic, queue operations, and synchronisation.
- Debugging and adjustment of concurrent behaviour based on observed execution.

The commit history shows the evolution of the implementation as the team added functionality, fixed concurrency issues, and improved the overall simulation.

## Limitations

- The project is a simplified educational simulation, not a production-ready checkout system.
- All code resides in a single file for simplicity, without modular separation into headers and multiple source files.
- Error handling and input validation are minimal, focused on demonstrating concurrency concepts rather than robustness.
- No automated tests are included; correctness is verified through manual execution and observation.

## License

This repository is an academic project. No explicit licence has been defined.
