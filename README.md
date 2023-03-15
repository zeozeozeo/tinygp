# **WIP**

# tinygp.h (Tiny Graphics Painter)

This is a small and fast 2D graphics library written in C99 (WIP).

# Features

- Antialiasing
    ![Not antialiased](https://cdn.discordapp.com/attachments/783966433641365504/1085637476074401933/image.png)

    ![Antialiased](https://media.discordapp.net/attachments/783966433641365504/1085637475814346792/image.png)

- 2D transformations (rotation, translation, projection)
- Ability to provide your own userdata for every draw command (the library does not provide shader support or image loading, but it can be implemented by using this feature)
- Does not rely on a graphics API, the library only generates draw commands (there is a backend for OpenGL and OpenGLES)
- Automatic batching: draw commands are automatically merged
- Batch optimization: rearranges draw commands to merge more of them
- Single header library

