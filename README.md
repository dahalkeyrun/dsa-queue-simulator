# dsa-queue-simulator
This is a repository that contains the source code for the assignment 1 of COMP202


```markdown
# Traffic Simulator using Priority Queues

## Overview
This project simulates a traffic intersection using a priority queue to manage vehicle movement and traffic light control. The simulation is built using SDL2 for rendering and includes features such as vehicle spawning, traffic light management, and vehicle redirection at intersections.

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

## License
This project is open-source and available under the MIT License. Feel free to modify and distribute it as needed.

## Contributing
Contributions are welcome! Please fork the repository and submit a pull request with your changes.
```