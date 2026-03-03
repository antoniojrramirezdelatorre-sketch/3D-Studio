# 3D Studio - Computer Graphics Project

**3D Studio** is a C++ / OpenGL rendering engine developed as part of the Computer Graphics course at Umeå University. This project implements a modern programmable graphics pipeline, featuring custom shader support, dynamic lighting, texture mapping, and an interactive camera system.

![Project Status](https://img.shields.io/badge/Status-Completed-success)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![Platform](https://img.shields.io/badge/Platform-Linux-lightgrey)

## Features

### Core Rendering
* **OBJ Loader:** Custom parser for loading 3D geometry from `.obj` files.
* **Programmable Pipeline:** Vertex and Fragment shaders implemented in GLSL.
* **Transformations:** Full support for Affine transformations (Translation, Rotation, Scaling).

### Visuals & Shading (Part 3)
* **Lighting Models:** Per-pixel shading implementing **Phong/Blinn-Phong** reflection models.
* **Texture Mapping:** Support for 2D textures using two-part mapping.
* **Material System:** Adjustable material properties (ambient, diffuse, specular) via GUI.

### Camera & Projection (Part 2)
* **Interactive Camera:** Free-moving camera with look-at functionality.
* **Projections:** Support for Perspective, Orthographic, and Oblique projections.
* **Viewport:** Dynamic aspect ratio handling.

### User Interface
* **GUI:** Integrated **Dear ImGui** interface for real-time parameter adjustment.
* **Scene Control:** Load models, toggle textures, and change light positions at runtime.

---

## Installation & Setup

### Project Structure
This project is distributed across three archives (`porject1.zip`, `porject2.zip`, `porject3.zip`) representing different development stages. **Use the content of `porject3` for the final version**, as it supersedes the previous parts.

### Dependencies

Ensure the following libraries are installed on your system or placed in a local `./lib` folder:
* **GLFW3** (Windowing and Input)
* **GLEW** (OpenGL Extension Wrangler)
* **Dear ImGui** (UI Library - source files expected in build path)
* **stb_image** (Image loading)

### Build Instructions
1.  **Merge Files:** Extract the contents of `porject3.zip`.
2.  **Restore Libraries:** Create a `lib/` directory and place the required binaries/headers there.
3.  **Compile:** Use the provided Makefile.
    ```bash
    cd 3DStudio
    make
    ./3dstudio
    ```

---

## Controls

### Camera Movement
| Key | Action |
| :--- | :--- |
| **W / S** | Move Forward / Backward (Z-axis) |
| **A / D** | Move Left / Right (X-axis) |
| **Q / E** | Move Down / Up (Y-axis) |
| **Mouse + Click** | Rotate Camera (Orbit) |

### Object Manipulation
| Key | Action |
| :--- | :--- |
| **Arrow Keys** | Rotate Object around X/Y axes |
| **I / K** | Translate Object vertically |
| **J / L** | Translate Object horizontally |

---

## File Manifest
* `main.cpp`: Entry point and main loop.
* `openglwindow.cpp`: Window context and event handling.
* `geometryrender.cpp`: Handles VAO/VBO setup and drawing calls.
* `vshader.glsl` / `fshader.glsl`: Vertex and Fragment shaders.
* `sphere.obj`: Sample 3D model.

---

## Credits
**Author:** [Antonio Ramírez]  
**Course:** Computer Graphics (5DV179) - Umeå University  
**Instructor:** Stefan Johansson  

*Based on project specifications from 2025.*