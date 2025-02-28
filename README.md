# dsa-queue-simulator

This repository contains the source code for COMP202 Assignment 1, a traffic intersection simulator implemented using priority queues and SDL2.

## Overview

This project simulates traffic flow at an intersection by using priority queues to manage vehicles and traffic signals. The simulation visualizes vehicles moving through the intersection, stopping at red lights, and changing directions based on predefined rules.

## Features

- **Vehicle Spawning**: Vehicles are generated at random intervals and assigned to one of four roads (A, B, C, D).
- **Traffic Light Control**: Traffic lights are managed using a priority queue, with higher priority given to lanes with more waiting vehicles.
- **Vehicle Movement**: Vehicles move along their assigned lanes and stop at red lights. They can also be redirected at intersections based on predefined rules.
- **Rendering**: The simulation is rendered using SDL2, with vehicles, traffic lights, and road markings displayed in real-time.

## Project Structure

```
.
├── car1.png             # Vehicle texture for rendering
├── car.png              # Alternate vehicle texture
├── laneA.txt            # Log file for vehicles on Road A
├── laneB.txt            # Log file for vehicles on Road B
├── laneC.txt            # Log file for vehicles on Road C
├── laneD.txt            # Log file for vehicles on Road D
├── queue.c              # Implementation of priority queue and vehicle management
├── queue.h              # Header file for queue and vehicle structures
├── queue.o              # Compiled object file for queue.c
├── README.md            # This file
├── simulator            # Compiled executable
├── simulator.c          # Main simulation logic and rendering
├── traffic_generator.c  # Vehicle generation and spawning logic
└── traffic_generator.o  # Compiled object file for traffic_generator.c
```

## Dependencies

- **SDL2**: For rendering the simulation.
- **SDL2_image**: For loading and rendering vehicle textures.

## Building the Project

To build the project, ensure you have SDL2 and SDL2_image installed, then run:

```bash
g++ simulator.c queue.c traffic_generator.c -o simulator $(sdl2-config --cflags --libs) -lSDL2_image
```

## Running the Simulation

After building, run the simulation using:

```bash
./simulator
```

## Logging

Vehicle data is logged to `laneA.txt`, `laneB.txt`, `laneC.txt`, and `laneD.txt` for each respective road. Each log entry includes the vehicle ID, timestamp, lane, and direction (straight or left).

## Customization

- **Vehicle Spawn Rate**: Adjust `VEHICLE_SPAWN_INTERVAL` in `traffic_generator.c`.
- **Traffic Light Duration**: Modify `currentGreenDuration` in `simulator.c`.
- **Priority Thresholds**: Tune `PRIORITY_THRESHOLD` and `NORMAL_THRESHOLD` in `queue.c`.

## Resources

### SDL2 Resources
- [Official SDL2 Website](https://www.libsdl.org/)
- [SDL2 Wiki](https://wiki.libsdl.org/)
- [Lazy Foo' Productions SDL2 Tutorials](https://lazyfoo.net/tutorials/SDL/)
- [SDL2 API Documentation](https://wiki.libsdl.org/SDL2/CategoryAPI)
- [SDL2_image Documentation](https://www.libsdl.org/projects/SDL_image/docs/index.html)

### Priority Queue Implementation
- [Introduction to Priority Queues](https://www.geeksforgeeks.org/priority-queue-set-1-introduction/)
- [Binary Heaps for Priority Queues](https://www.cs.princeton.edu/~wayne/kleinberg-tardos/pdf/HeapSort.pdf)

### Traffic Simulation References
- [SUMO (Simulation of Urban MObility)](https://www.eclipse.org/sumo/) - Open source traffic simulation package
- [Introduction to Traffic Flow Theory](https://www.researchgate.net/publication/290490588_Introduction_to_Traffic_Flow_Theory)

## License

This project is open-source and available under the MIT License. Feel free to modify and distribute it as needed.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request with your changes.
