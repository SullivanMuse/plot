#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mathlib.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef unsigned int uint;

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        printf("User pressed escape key\n");
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/* Done */
vec3 *read_to_vertices(const char *filename)
{
    vec3 *vertices = (vec3 *) calloc(50, sizeof(vec3));
    FILE *file;
    file = fopen(filename, "r");
    for (int i = 0; i < 50; ++i)
    {
        vec3 *curr = &vertices[i];
        fscanf(file, "%20f, %20f, %20f", &curr->x, &curr->y, &curr->z);
    }
    return vertices;
}

/* Done */
void normalize(vec3 *vertices)
{
    float xmin, xmax, ymin, ymax;
    for (int i = 0; i < 50; ++i)
    {
        xmin = min(xmin, vertices[i].x);
        xmax = max(xmax, vertices[i].x);
        ymin = min(ymin, vertices[i].y);
        ymax = max(ymax, vertices[i].y);
    }

    for (int i = 0; i < 50; ++i)
    {
        vertices[i].x = 2 * (vertices[i].x - xmin) / (xmax - xmin) - 1;
        vertices[i].y = 2 * (vertices[i].y - ymin) / (ymax - ymin) - 1;
    }
}

/* Done */
vec3 *spline_vertices(size_t size, vec3 vertices[size], float width)
{
    if (size < 2)
    {
        printf("error: must have at least two points to form a line\n");
        exit(1);
    }

    vec3 *vertices_out = calloc(2 * size, sizeof(vec3));
    for (size_t i = 0; i < size; ++i)
    {
        vec3 forward = sub(vertices[i + 1], vertices[i]);
        vec3 z = {0, 0, 1};
        vec3 dir1 = scalar_mul(width / 2, unit(cross(forward, z)));
        vec3 dir2 = neg(dir1);
        vertices_out[2 * i + 1] = add(dir1, vertices[i]);
        vertices_out[2 * i] = add(dir2, vertices[i]);
    }
    return vertices_out;
}

/* Done */
unsigned int *spline_indices(size_t size, vec3 vertices[size])
{
    if (size < 2)
    {
        printf("error: must have at least two points to form a line\n");
        exit(1);
    }

    unsigned int *indices_out = calloc(6 * (size - 1), sizeof(unsigned int));
    for (size_t i = 0; i < size - 1; ++i)
    {
        /* First Triangle */
        indices_out[6 * i] = 2 * i;
        indices_out[6 * i + 1] = 2 * i + 1;
        indices_out[6 * i + 2] = 2 * i + 2;

        /* Second Triangle */
        indices_out[6 * i + 3] = 2 * i + 3;
        indices_out[6 * i + 4] = 2 * i + 2;
        indices_out[6 * i + 5] = 2 * i;
    }
    return indices_out;
}

float *linspace(float a, float b, size_t n)
{
    if (n < 2)
    {
        exit(1);
    }
    float *out = calloc(n, sizeof(float));
    float dt = (b - a) / (n - 1);
    for (size_t i = 0; i < n; ++i)
    {
        out[i] = a + dt * i;
    }
    return out;
}

vec3 *quad(size_t n)
{

}

typedef struct Renderer
{
    char *vertex_shader_source;
    char *fragment_shader_source;
    vec3 *vertices;
    size_t vertices_size; /* size in bytes, NOT number */
    unsigned int *indices;
    size_t indices_size; /* size in bytes, NOT number */
    unsigned int program, VAO, VBO, EBO;
} Renderer;

unsigned int setup_shader_program(const char *vertex_shader_source, const char *fragment_shader_source)
{
    /*
     * 1. Compile vertex shader
     * 2. Compile fragment shader
     * 3. Compile shader program
     * 4. Delete shaders
     */
    int success;
    char log[512];

    /* Vertex Shader */
    unsigned int vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex_shader, 512, NULL, log);
        printf("error compiling vertex shader: %s\n", log);
        return -1;
    }

    /* Fragment Shader */
    unsigned int fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment_shader, 512, NULL, log);
        printf("error compiling fragment shader: %s\n", log);
        return -1;
    }

    /* Shader Program */
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shader_program, 512, NULL, log);
        printf("error linking shader program: %s\n", log);
        return -1;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void setup(Renderer *rend, bool normalize)
{
    /*
     * 1. Compile program
     * 2. Bind VAO
     * 3. Copy our vertices into a VBO
     * 4. Copy Index array into an EBO
     * 5. Set the VAPs
     * 6. Unbind objects
     */
    rend->program = setup_shader_program(rend->vertex_shader_source, rend->fragment_shader_source);

    glGenVertexArrays(1, &rend->VAO);
    glBindVertexArray(rend->VAO);

    glGenBuffers(1, &rend->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, rend->VBO);
    glBufferData(GL_ARRAY_BUFFER, rend->vertices_size, rend->vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &rend->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rend->indices_size, rend->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, normalize, sizeof(vec3), (void*) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); /* Unbind VBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); /* Unbind EBO */
}

void draw(Renderer *rend)
{
    glUseProgram(rend->program);
    glBindVertexArray(rend->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->EBO);
    glDrawElements(GL_TRIANGLES, rend->indices_size / 3, GL_UNSIGNED_INT, 0);
    glUseProgram(0);
    glBindVertexArray(0);
}

void delete_gl_objects(Renderer *rend)
{
    free(rend->vertex_shader_source);
    free(rend->fragment_shader_source);
    glDeleteProgram(rend->program);
    glDeleteVertexArrays(1, &rend->VAO);
    glDeleteBuffers(2, (uint[]){rend->VBO, rend->EBO});
}

GLFWwindow *init_glfw(GLFWframebuffersizefun framebuffer_size_callback)
{
    /*
     * 1. Initialize GLFW
     * 2. Set configuration
     * 3. Create window
     * 4. Initialize GLAD
     * 5. Setup the viewport and resize callback
     */
    printf("Starting\n");

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        exit(-1);
    }

    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    printf("Done starting\n");

    return window;
}

int main(void)
{
    /* Startup */
    GLFWwindow *window = init_glfw(framebuffer_size_callback);

    /* Common */
    const char vertex_shader_source[] =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n";
    const char fragment_shader_source[] =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n";

    /* Triangle */
    Renderer triangle;
    triangle.vertex_shader_source = strdup(vertex_shader_source);
    triangle.fragment_shader_source = strdup(fragment_shader_source);
    triangle.vertices = (vec3[]){
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        { 0.0f,  0.5f, 0.0f},
    };
    triangle.vertices_size = 3 * sizeof(vec3);
    triangle.indices = (uint[]){0, 1, 2};
    triangle.indices_size = 3 * sizeof(uint);
    setup(&triangle, false);

    /* Rect */
    Renderer rect;
    rect.vertex_shader_source = strdup(vertex_shader_source);
    rect.fragment_shader_source = strdup(fragment_shader_source);
    rect.vertices = (vec3[]){
        // Lower left
        {-1.0f, -1.0f, 0.0f},
        // Upper left
        {-1.0f, -0.9f, 0.0f},
        // Lower right
        {1.0f, -1.0f, 0.0f},
        // Upper right
        {1.0f, -0.9f, 0.0f},
    };
    rect.vertices_size = 4 * sizeof(vec3);
    rect.indices = (uint[]){
        1, 0, 3,
        3, 0, 2,
    };
    rect.indices_size = 6 * sizeof(uint);
    setup(&rect, false);

    /* Axes */
    float width = 0.01f;

    /* x-axis */
    vec3 *vertices = (vec3[]){
        {-1.0f, 0.0f, 0.0f},
        { 1.0f, 0.0f, 0.0f},
    };
    Renderer xaxis;
    xaxis.vertex_shader_source = strdup(vertex_shader_source);
    xaxis.fragment_shader_source = strdup(fragment_shader_source);
    xaxis.vertices = spline_vertices(2, vertices, width);
    xaxis.vertices_size = 4 * sizeof(vec3);
    xaxis.indices = spline_indices(2, vertices);
    xaxis.indices_size = 6 * sizeof(uint);
    setup(&xaxis, false);

    /* Y-axis */
    vertices = (vec3[]){
        {0.0f, -1.0f, 0.0f},
        {0.0f,  1.0f, 0.0f},
    };
    Renderer yaxis;
    yaxis.vertex_shader_source = strdup(vertex_shader_source);
    yaxis.fragment_shader_source = strdup(fragment_shader_source);
    yaxis.vertices = spline_vertices(2, vertices, width);
    yaxis.vertices_size = 4 * sizeof(vec3);
    yaxis.indices = spline_indices(2, vertices);
    yaxis.indices_size = 6 * sizeof(uint);
    setup(&yaxis, false);

    /* Plot */
    size_t n = 1000;
    vertices = (vec3 *) calloc(n, sizeof(vec3));
    FILE *file;
    file = fopen("quad.csv", "r");
    for (int i = 0; i < n; ++i)
    {
        vec3 *curr = &vertices[i];
        fscanf(file, "%20f, %20f, %20f", &curr->x, &curr->y, &curr->z);
    }
    // normalize(vertices);

    Renderer plot;
    plot.vertex_shader_source = strdup(vertex_shader_source);
    plot.fragment_shader_source = strdup(fragment_shader_source);
    plot.vertices = spline_vertices(n, vertices, width);
    plot.vertices_size = 2 * n * sizeof(vec3);
    plot.indices = spline_indices(n, vertices);
    plot.indices_size = 6 * n * sizeof(uint);
    setup(&plot, true);

    while (!glfwWindowShouldClose(window))
    {
        // Processing input
        processInput(window);

        // Rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        // draw(&triangle);
        // draw(&rect);
        // draw(&xaxis);
        // draw(&yaxis);
        draw(&plot);

        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    printf("Closing window\n");

    /* Delete stuff and terminate */
    delete_gl_objects(&triangle);
    delete_gl_objects(&rect);
    delete_gl_objects(&xaxis);
    delete_gl_objects(&yaxis);
    delete_gl_objects(&plot);
    glfwTerminate();

    return 0;
}
