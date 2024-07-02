Math Visual Test

The project will open up in attractMode where the player can choose different game modes based on assigned hotkeys. Math Visual Test currently has 13 modes.
1. IsPointInside
2. RaycastVsDisc
3. Billards2D
4. TestShapes3D
5. Raycast Vs Line
6. Raycast Vs AABB2
7. Pachinko
8. Easing, Splines, Bezier Curves
9. Spin and Friction 2D
10. Raycast Vs AABB 3D
11. Raycast Vs OBB 2D
12. Raycast Vs OBB 3D
13. Convex Scene

Convex Scene performance summary
a.	How many raycasts/ms can you do vs. 100 objects?  1000?  10000?
b.	How does this improve (or worsen!) when you enable your hashing/partitioning scheme?
c.	How do these speeds compare in each build configuration (Debug, DebugInline, FastBreak, Release)?
d.	Any general trends you can observe, i.e. the speed seems to be O(N) or O(N2) with #objects, #rays, etc.
e.	Any data specific to your hashing/partitioning scheme you can observe (e.g. AABB Tree depth)
f.	Anything else interesting you observe about your results
g.	All speed measurements (other than cross-build comparisons) are taken in Release builds

Performance results for a raycast on a screen size (200,100). StartPos(100.0f, 50.0f), endPos(150.0f, 50.0f), fwd(1.0f, 0.0f).
- Computing 1024 raycasts against 8192 convex objects takes approximately 3.0 seconds with no optimizations.
- Computing 1024 raycasts against 8192 convex objects takes approximately 0.7 seconds with narrow phase optimizations.
- Computing 1024 raycasts against 8192 convex objects takes approximately 0.5 seconds with broad phase optimizations.

Overall, the performance gains from BVH was directly correlated with the length of its ray. The longer the ray, the higher computation costs were for rejecting AABB tree nodes. It also required the extra calculations to test for potentail broad phase rejections, only to find out the need to raycast against the majority of the overlapping objects in the first place.



Common controls
- F6 and F7 to switch between modes.
- F8 to reshuffle objects 
- ESDF or WASD to move
- Esc to exit