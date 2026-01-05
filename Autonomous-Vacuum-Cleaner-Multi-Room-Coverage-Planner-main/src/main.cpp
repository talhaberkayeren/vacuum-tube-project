#include "core/GridParser.hpp"
#include "core/GridModel.hpp"
#include "core/PathFinder.hpp"
#include "core/BatteryManager.hpp"
#include "core/RoomDecomposer.hpp"
#include "core/RoomStrategy.hpp"
#include "core/CoveragePlanner.hpp"
#include "frontend/TUI.hpp"
#include "frontend/DisplayMode.hpp"
#include "data_structures/Stack.h"
#include "data_structures/Queues.h"
#include "data_structures/HashSet.h"
#include "data_structures/LinkedList.h"
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>

// Global state for hooks (function pointers need static/global context)
static GridModel* g_grid = nullptr;
static PathFinder* g_pathFinder = nullptr;
static BatteryManager* g_batteryManager = nullptr;
static bool g_requestReturnToDock = false;

// Helper: BFS path builder (application-level helper, not core algorithm modification)
static bool buildPathBFS(const Position& from, const Position& to, ds::Stack<Position>& outPath);

// Helper: Distance function wrapper for room ordering
static int distanceToDockHook(const Position& pos);

// Helper: Request return to dock callback
static void requestReturnToDockHook();

int main(int argc, char* argv[]) {
    // Determine scenario file
    const std::string scenario = (argc >= 2) ? argv[1] : "data/small_room.json";
    
    // Print which scenario is being loaded
    std::cout << "Loading scenario: " << scenario << std::endl;
    
    // Parse arguments for display mode
    DisplayMode mode = DisplayMode::FAST;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--step") {
            mode = DisplayMode::STEP;
        } else if (arg == "--fast") {
            mode = DisplayMode::FAST;
        }
    }
    
    try {
        // Parse grid using existing GridParser
        GridModel grid = GridParser::parse(scenario);
        
        // Validate grid was loaded correctly - must have valid dimensions
        if (grid.R() == 0 || grid.C() == 0) {
            std::cerr << "Error: Grid is empty or invalid (rows=" << grid.R() << ", cols=" << grid.C() << ")" << std::endl;
            std::cerr << "The JSON file '" << scenario << "' appears to contain an empty grid." << std::endl;
            return 1;
        }
        
        g_grid = &grid;
        
        // Initialize components
        PathFinder pathFinder(grid);
        g_pathFinder = &pathFinder;
        BatteryManager batteryMgr(grid.initialBattery, pathFinder);
        g_batteryManager = &batteryMgr;
        RoomDecomposer decomposer;
        RoomStrategy strategy;
        
        // Decompose rooms
        ds::LinkedList<Room> rooms;
        decomposer.assignRooms(grid, rooms);
        
        // Build room order using RoomStrategy
        ds::LinkedList<int> roomOrder = strategy.buildRoomOrder(rooms, distanceToDockHook);
        
        // Initialize CoveragePlanner with hooks
        CoveragePlanner planner;
        CoveragePlanner::Hooks hooks;
        hooks.buildPath = buildPathBFS;
        hooks.distToDock = distanceToDockHook;
        hooks.requestReturnToDock = requestReturnToDockHook;
        hooks.safetyMargin = 2;
        planner.init(&grid, &rooms, &decomposer, hooks);
        planner.setRoomOrder(roomOrder);
        
        // Initialize TUI
        TUI tui;
        tui.setMode(mode);
        
        // Simulation state
        Position currentPos = grid.dockPosition;
        int totalSteps = 0;
        int initialBattery = grid.initialBattery;
        
        // Main simulation loop
        while (!planner.allFloorsCleaned()) {
            // Render current state
            tui.render(grid, currentPos, batteryMgr.getBattery());
            tui.waitForNext();
            
            // Check if need to return to dock
            if (g_requestReturnToDock || batteryMgr.needsReturn(currentPos)) {
                g_requestReturnToDock = false;
                
                // Build path to dock
                ds::Stack<Position> dockPath;
                buildPathBFS(currentPos, grid.dockPosition, dockPath);
                
                // Execute path to dock (path is stored as stack, need to reverse for execution)
                // Since Stack pops in reverse order of push, we need to collect first
                ds::LinkedList<Position> execList;
                while (!dockPath.isEmpty()) {
                    execList.pushFront(dockPath.pop());
                }
                
                // Execute path to dock
                for (int i = 0; i < (int)execList.size(); i++) {
                    Position next = execList.getAt(i);
                    if (next.r == currentPos.r && next.c == currentPos.c) continue;
                    
                    currentPos = next;
                    batteryMgr.consume(1);
                    totalSteps++;
                    
                    tui.render(grid, currentPos, batteryMgr.getBattery());
                    tui.waitForNext();
                }
                
                // Recharge at dock
                batteryMgr.recharge();
                continue;
            }
            
            // Plan next path
            ds::Stack<Position> path;
            bool hasPath = planner.planNextPath(currentPos, batteryMgr.getBattery(), path);
            
            if (!hasPath || path.isEmpty()) {
                // No path available, try to return to dock
                g_requestReturnToDock = true;
                continue;
            }
            
            // Execute path (path from planNextPath needs to be reversed for execution)
            // Collect path into list first
            ds::LinkedList<Position> execList;
            while (!path.isEmpty()) {
                execList.pushFront(path.pop());
            }
            
            // Execute path step by step
            for (int i = 0; i < (int)execList.size(); i++) {
                Position next = execList.getAt(i);
                if (i == 0 && next.r == currentPos.r && next.c == currentPos.c) {
                    continue;  // Skip first if same as current
                }
                
                currentPos = next;
                batteryMgr.consume(1);
                totalSteps++;
                
                // Mark as cleaned if it's a floor cell
                if (grid.isFloor(currentPos.r, currentPos.c)) {
                    grid.markCleaned(currentPos.r, currentPos.c);
                }
                
                tui.render(grid, currentPos, batteryMgr.getBattery());
                tui.waitForNext();
            }
        }
        
        // Print final summary
        int totalBatteryUsed = initialBattery - batteryMgr.getBattery();
        tui.printSummary(grid, totalSteps, totalBatteryUsed);
        
        std::cout << "Simulation complete!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to load scenario '" << scenario << "': " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown error occurred while loading scenario '" << scenario << "'" << std::endl;
        return 1;
    }
}

// BFS implementation for path building
static bool buildPathBFS(const Position& from, const Position& to, ds::Stack<Position>& outPath) {
    if (g_grid == nullptr) return false;
    
    // Clear output path
    while (!outPath.isEmpty()) {
        outPath.pop();
    }
    
    // Simple case: already at destination
    if (from.r == to.r && from.c == to.c) {
        outPath.push(Position(from.r, from.c));
        return true;
    }
    
    // BFS to find path
    ds::Queue<Position> queue;
    ds::HashSet<Position, PositionHash> visited;
    ds::LinkedList<Position> parentList;  // Store positions in order
    ds::LinkedList<int> parentIndex;      // Store parent indices
    
    queue.enqueue(from);
    visited.insert(from);
    parentList.pushBack(from);
    parentIndex.pushBack(-1);  // -1 means no parent (root)
    
    int startIndex = 0;
    bool found = false;
    int targetIndex = -1;
    
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};
    
    while (!queue.isEmpty()) {
        Position current = queue.dequeue();
        int currentIndex = startIndex++;
        
        if (current.r == to.r && current.c == to.c) {
            found = true;
            targetIndex = currentIndex;
            break;
        }
        
        for (int k = 0; k < 4; k++) {
            int nr = current.r + dr[k];
            int nc = current.c + dc[k];
            
            if (!g_grid->inBounds(nr, nc)) continue;
            if (g_grid->cells[nr][nc] == CellType::WALL) continue;
            
            Position next(nr, nc);
            if (visited.contains(next)) continue;
            
            visited.insert(next);
            queue.enqueue(next);
            parentList.pushBack(next);
            parentIndex.pushBack(currentIndex);
        }
    }
    
    if (!found) return false;
    
    // Reconstruct path backwards from target to start
    ds::LinkedList<Position> reversedPath;
    int idx = targetIndex;
    while (idx >= 0) {
        reversedPath.pushFront(parentList.getAt(idx));
        idx = parentIndex.getAt(idx);
    }
    
    // Push path onto stack (from -> to order)
    for (int i = 0; i < (int)reversedPath.size(); i++) {
        outPath.push(Position(reversedPath.getAt(i).r, reversedPath.getAt(i).c));
    }
    
    return true;
}

// Distance to dock hook
static int distanceToDockHook(const Position& pos) {
    if (g_pathFinder == nullptr) return 0;
    return g_pathFinder->distanceToDock(pos);
}

// Request return to dock hook
static void requestReturnToDockHook() {
    g_requestReturnToDock = true;
}

