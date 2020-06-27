# Tower Defence Demo (OpenGL)

<p align="center">
	<img src="/examples/maze.png" width="60%" />
	<br>
	<font size="2">
		A tower defence maze with dynamic pathfinding 
	</font>
</p>

This demo was created to learn about OpenGL and programming techniques related to object pooling and rendering. As of now, it is not a fully featured game, but instead serves as a smaller demo.

## Features

### Dynamic pathfinding

<p align="center">
	<img src="/examples/dynamic_0.png" width="30%" />
	<img src="/examples/dynamic_1.png" width="30%" />
	<br>
	<font size="2">
		When a tower is placed, a new path to goal is calculated
	</font>
</p>
	
This demo uses Flow Field Pathfinding to ensure fast updates and allowing for massive object pools. The flow field is recalculated when towers are placed or removed, and at all other times pathfinding calculations for enemies are equivalent to vector lookups. 

### Object pooling

To minimize latency issues and cache misses, all game objects are allocated at the start of the game, and kept in a contiguous array. By utilizing object pooling techniques, objects can appear to be created and destroyed, while in reality they are just moving within the object pool array, which also causes them to move in and out of the game loop. 

### Pooled rendering

Somewhat of a new feature, but to render enemies more efficiently, they are pooled into a single mesh, which is then drawn by a single draw call. This significantly improved performance. 


## Compile and run

To compile and run, just use

	make
	bin/tower_defence

