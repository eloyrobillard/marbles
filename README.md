# Marbles

Small game featuring a marble going down along a path.

This concept is loosely based around the word "transform", as in transforming gravity into forward motion. 

## How to start the game

After opening `marbles.slnx` in Visual Studio, the game should be playable right away by pressing F5 in both Debug and Release modes.

## How to play

There are only three commands:

- Left and right arrows, to go left and right
- Space, to return to the last checkpoint (useful when the marble has gone off the path)

These commands are also displayed in the upper left corner of the UI.

### Debug mode

The Debug build has a few visual debug features:

- a wireframe showing colliders currently being tested against;
- a tint that appears on colliders when a collision just happened;
- the shadow map used to generate shadows

## Resources & Attributions

From "Game Programming in C++" by Sanjay Madhav, the code for the following classes/namespaces:

- Quaternion
- Matrix4 (CreateLookAt, CreatePerspectiveFOV, CreateOrtho)
- Shader
- Mesh
- Texture
- FollowCamera

Script to export from Blender3D as GPMesh: "Game Programming in C++" by Sanjay Madhav ([source](https://github.com/gameprogcpp/code/blob/master/Exporter/Blender/gpmesh_export_v2.py))

3D rendering:

- Phong: [LearnOpenGL](https://learnopengl.com/Lighting/Basic-Lighting)
- Anti-Aliasing: [LearnOpenGL](https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing)
- Skybox: [LearnOpenGL](https://learnopengl.com/Advanced-OpenGL/Cubemaps)
- HUD: [LearnOpenGL](https://learnopengl.com/In-Practice/Text-Rendering), [lackeyccg.com](https://lackeyccg.com/glfont.c)

Collision detection: ”Real-Time Collision Detection" by Christer Ericson

- ClosestPtvec3Triangle (5.1.5)
- IntersectsTriangle (5.2.7)

Impulse physics: "Game Physics Engine Development" by Ian Millington (section 7.2)

Physics loop with a fixed timestep：[Gaffer On Games](https://www.gafferongames.com/post/fix_your_timestep/)

Making ramps and gutters in Blender3D:

- making a curved road/ramp: [BlenderVitals](https://www.youtube.com/watch?v=-v-_vubDXog)
- making a gutter: [PIXXO 3D](https://youtu.be/i0hK-sqxWS0)

Render frames debugging: [RenderDoc](https://renderdoc.org/)

### Libraries/Templates

- [SDL3 3.4.4](https://github.com/libsdl-org/SDL)
- Font processing for HUD: [SDL_ttf 3.2.2](https://github.com/libsdl-org/SDL_ttf)
- JSON parsing: [rapidjson](https://github.com/Tencent/rapidjson/)
- Texture loading: [SOIL](https://github.com/littlstar/soil)
- GPU rendering: OpenGL & [glew 2.3.1](https://github.com/nigels-com/glew?tab=readme-ov-file)
- [Tmpl8](https://www.3dgep.com/cpp-fast-track-2-template/) by Breda University

### Assets

Skybox image: [LearnOpenGL](https://learnopengl.com/Advanced-OpenGL/Cubemaps)
Background track: "Station Six" by Dualistic
