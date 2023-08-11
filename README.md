## Obtaining GLFW on Ubuntu

`sudo apt install libglfw3 libglfw3-dev`

## Compiling

`gcc -pthread -o test test.c glad/src/glad.c -lglfw -lGLU -lGL -lXrandr -lXxf86vm -lXi -Iglad/include`
