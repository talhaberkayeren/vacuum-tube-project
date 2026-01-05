# Autonomous Vacuum Cleaner – Multi-Room Coverage Planner
CSE 211 - Data Structures Term Project

## Project Overview
This project aims to design and implement **a coverage path planning system** for an autonomous vacuum cleaner operating in a multi-room indoor environment.

The environment is represented as a grid-based layout, where each cell corresponds to a specific type of space such as floor, wall, obstacle, doorway or charging dock. The vacuum cleaner must navigate this environment and clean all reachable floor cells at least once, while respecting battery constraints and ensuring it can always return safely to the charging dock. 

The project focuses on data structures and algorithmic reasoning, rather than UI or external frameworks. All core components are implemented using plain C++ with custom pointer-based data structures.

## Core Objectives
1. Represent a multi-room home environment using a grid model.
2. Track visited and unvisited (cleaned / uncleaned) cells.
3. Ensure complete coverage of all floor cells.
4. Enforce battery constraints and safe return to the charging dock.
5. Support returning to charge and resuming cleaning afterward.

## How to Run the Simulation (TUI)

To compile and run the vacuum cleaner simulation with the text-based user interface:

    make run

This command builds the application executable in the `build/` directory and immediately runs it. The simulation uses the default scenario file `data/small_room.json` if no argument is provided.

You can also specify a different scenario file:

    make run data/obstacle_heavy.json

The simulation will display the grid state, vacuum position, battery level, and progress as it executes. The simulation runs until all floor cells have been cleaned.

## Input Format

The grid is specified in JSON format with the following structure:

```json
{
  "battery": 100,
  "grid": [
    "WWWWW",
    "WSCDW",
    "WWWWW"
  ]
}
```

The grid uses a character-based encoding where each character represents a cell type:

- **W** = Wall (impassable boundary)
- **D** = Dock (charging station, exactly one required)
- **S** = Floor (clean floor cell, counted for coverage planning)
- **C** = Dirty cell (non-floor cell, not counted for coverage)

The grid must be rectangular (all rows have the same length) and must contain exactly one dock (D). Any invalid characters will cause the parser to raise an error during initialization.

## Example Output

After the simulation completes, a summary is displayed:

```
=== Simulation Complete ===

Total Steps: 15
Total Battery Used: 15
Floors Cleaned: 2 / 2
Coverage: 100%

Final Grid State:
WWWWW
W..DW
WWWWW
```

The metrics indicate:
- **Total Steps**: Number of moves the vacuum made
- **Total Battery Used**: Battery consumed during the simulation
- **Floors Cleaned**: Number of floor cells cleaned versus total floor cells
- **Coverage**: Percentage of floor cells successfully cleaned
- **Final Grid State**: Visual representation showing cleaned cells (.) and remaining elements

## Build and Run Test Files

RUN TEST: 
    
    make test
    
    make test_all
    
RUN INDIVIDUAL TESTS:

    make test_linkedlist
    
    make test_queue
    
    make test_hashset
    
    make test_stack
    
    make test_parser
    
    make test_battery
    
    make test_pathfinder

CLEAR:
    
    make clean
    
    make clear
        
## Project Structure
    CSE211-TermProject/
    ├── Makefile
    ├── README.md
    ├── build
    │   ├── data_tests_all
    │   ├── test_battery
    │   ├── test_hashset
    │   ├── test_ll
    │   ├── test_parser
    │   ├── test_pathfinder
    │   ├── test_queues
    │   └── test_stack
    ├── data
    │   ├── invalid_grid.json
    │   ├── obstacle_heavy.json
    │   └── small_room.json
    ├── include
    │   ├── core
    │   │   ├── BatteryManager.hpp
    │   │   ├── GridModel.hpp
    │   │   ├── GridParser.hpp
    │   │   └── PathFinder.hpp
    │   ├── data_structures
    │   │   ├── HashSet.h
    │   │   ├── LinkedList.h
    │   │   ├── Queues.h
    │   │   └── Stack.h
    │   └── libraries
    │       └── json.hpp
    ├── src
    │   ├── core
    │   │   ├── BatteryManager.cpp
    │   │   └── PathFinder.cpp
    │   └── data_structures
    └── tests
        ├── test_main.cpp
        └── unit
            ├── TestUtil.hpp
            ├── main
            │   ├── test_main_battery.cpp
            │   ├── test_main_hashset.cpp
            │   ├── test_main_linkedlist.cpp
            │   ├── test_main_parser.cpp
            │   ├── test_main_pathfinder.cpp
            │   ├── test_main_queue.cpp
            │   └── test_main_stack.cpp
            ├── test_battery.cpp
            ├── test_hashset.cpp
            ├── test_linkedlist.cpp
            ├── test_parser.cpp
            ├── test_pathfinder.cpp
            ├── test_queues.cpp
            └── test_stack.cpp

        
## Testing and Validation

The implementation has been validated through comprehensive testing:

- **Unit tests** for custom data structures (LinkedList, Stack, Queue, HashSet)
- **Parser validation tests** to ensure correct JSON input handling
- **Battery management tests** for energy consumption and safety checks
- **Pathfinding tests** for shortest path and distance calculations
- **Coverage planner integration tests** for end-to-end scenario execution

All tests can be run using:

    make test_all

This executes the complete test suite and verifies correctness of all components.

## Notes

This project is complete and has been tested. All core functionality for multi-room coverage planning, battery management, pathfinding, and the text-based user interface has been implemented and validated.

## Contributors
**CSE 211 Project 8 Group**
- Ege Huriel
- Pelin Görer
- Emirhan Eren Elibol
- Eylül Pirinçal
- Selin Suna Kaya

