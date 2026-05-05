# Marbles

Small game featuring a marble going down along a path.

This concept is loosely based around the word "transform", as in transforming gravity into forward motion. 

## How to start the game

Using Visual Studio, the game should be playable right away by pressing F5 in both Debug (featuring a visualization of collisions) and Release mode.

## How to play

There are only three commands:

- Left and right arrows, to go left and right
- Space, to return to the last checkpoint (useful when the marble has gone off the path)

These commands are also displayed in the upper left corner of the UI.

## Resources & Attributions

From "Game Programming in C++" by Sanjay Madhav, the code for the following classes/namespaces:

- Quaternion
- Matrix4 (CreateLookAt, CreatePerspectiveFOV, CreateOrtho)
- Shader
- Mesh
- Texture
- FollowCamera

3D rendering:

- Phong: [LearnOpenGL](https://learnopengl.com/Lighting/Basic-Lighting)
- Anti-Aliasing: [LearnOpenGL](https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing)
- Skybox, including the skybox image: [LearnOpenGL](https://learnopengl.com/Advanced-OpenGL/Cubemaps)
- HUD: [LearnOpenGL](https://learnopengl.com/In-Practice/Text-Rendering), [lackeyccg.com](https://lackeyccg.com/glfont.c)

Collision detection: ”Real-Time Collision Detection" by Christer Ericson

- ClosestPtvec3Triangle (5.1.5)
- IntersectsTriangle (5.2.7)

Impulse physics: "Game Physics Engine Development" by Ian Millington (section 7.2)

Physics loop with a fixed timestep：[Gaffer On Games](https://www.gafferongames.com/post/fix_your_timestep/)

Making ramps and gutters in Blender3D:

- making a curved road/ramp: [BlenderVitals](https://www.youtube.com/watch?v=-v-_vubDXog)
- making a gutter: [PIXXO 3D](https://youtu.be/i0hK-sqxWS0)
