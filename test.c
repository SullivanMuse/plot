#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mathlib.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef uint uint;

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

typedef struct Mesh
{
    size_t num_vertices;
    size_t num_indices;
    vec3 *vertices;
    uint *indices;
} Mesh;

/* Done */
Mesh line(size_t n, vec3 vertices[n], float width)
{
    if (n < 2)
    {
        printf("error: must have at least two points to form a line\n");
        exit(1);
    }

    float dx, dy, norm;
    Mesh out;

    // Allocate mesh
    out.num_vertices = 2 * n;
    out.num_indices = 6 * (n - 1);
    out.vertices = calloc(out.num_vertices, sizeof(vec3));
    out.indices = calloc(out.num_indices, sizeof(uint));

    // Left boundary
    dx = vertices[1].x - vertices[0].x;
    dy = vertices[1].y - vertices[0].y;
    norm = sqrt(dx * dx + dy * dy);
    dx = width * dx / norm;
    dy = width * dy / norm;
    out.vertices[0].x = vertices[0].x - dy;
    out.vertices[0].y = vertices[0].y + dx;
    out.vertices[1].x = vertices[0].x + dy;
    out.vertices[1].y = vertices[0].y - dx;

    // Right boundary
    dx = vertices[n - 1].x - vertices[n - 2].x;
    dy = vertices[n - 1].y - vertices[n - 2].y;
    norm = sqrt(dx * dx + dy * dy);
    dx = width * dx / norm;
    dy = width * dy / norm;
    out.vertices[2 * n - 2].x = vertices[n - 1].x - dy;
    out.vertices[2 * n - 2].y = vertices[n - 1].y + dx;
    out.vertices[2 * n - 1].x = vertices[n - 1].x + dy;
    out.vertices[2 * n - 1].y = vertices[n - 1].y - dx;

    // Interior
    for (size_t i = 1; i < n - 1; ++i)
    {
        dx = vertices[i + 1].x + vertices[i].x - vertices[i - 1].x;
        dy = vertices[i + 1].y + vertices[i].y - vertices[i - 1].y;
        norm = sqrt(dx * dx + dy * dy);
        dx = width * dx / norm;
        dy = width * dy / norm;
        out.vertices[2 * i].x = vertices[i].x - dy;
        out.vertices[2 * i].y = vertices[i].y + dx;
        out.vertices[2 * i + 1].x = vertices[i].x + dy;
        out.vertices[2 * i + 1].y = vertices[i].y - dx;
    }

    // Indices
    for (size_t i = 0; i < n - 1; ++i)
    {
        out.indices[6 * i] = 2 * i;
        out.indices[6 * i + 1] = 2 * i + 1;
        out.indices[6 * i + 2] = 2 * i + 2;
        out.indices[6 * i + 3] = 2 * i + 3;
        out.indices[6 * i + 4] = 2 * i + 2;
        out.indices[6 * i + 5] = 2 * i + 1;
    }

    return out;
}

typedef struct GameObject
{
    uint program, VAO, VBO, EBO;
    char *vertex_shader_source;
    char *fragment_shader_source;
    Mesh mesh;
} GameObject;

uint setup_shader_program(const char *vertex_shader_source, const char *fragment_shader_source)
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

void setup(GameObject *rend)
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
    printf("\x1b[34mHere\x1b[0m\n");

    glGenVertexArrays(1, &rend->VAO);
    glBindVertexArray(rend->VAO);

    glGenBuffers(1, &rend->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, rend->VBO);
    glBufferData(GL_ARRAY_BUFFER, rend->mesh.num_vertices * sizeof(vec3), rend->mesh.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &rend->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rend->mesh.num_indices * sizeof(uint), rend->mesh.indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, sizeof(vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); /* Unbind VBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); /* Unbind EBO */
}

void draw(GameObject *rend)
{
    glUseProgram(rend->program);
    glBindVertexArray(rend->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->EBO);

    glDrawElements(GL_TRIANGLES, rend->mesh.num_indices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void delete_Mesh(Mesh *mesh)
{


}

void delete_GameObject(GameObject *rend)
{


    delete_Mesh(&rend->mesh);
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

Mesh line_naive(size_t n, vec3 vertices[n], float width)
{
    Mesh out;

    // Allocate
    out.num_vertices = 2 * n;
    out.num_indices = 6 * (n - 1);
    out.vertices = calloc(out.num_vertices, sizeof(vec3));
    out.indices = calloc(out.num_indices, sizeof(uint));

    // Vertices
    for (size_t i = 0; i < n; ++i)
    {
        printf("i: %i; x: %f\n", i, vertices[i].x);
        out.vertices[2 * i].x = vertices[i].x;
        out.vertices[2 * i + 1].x = vertices[i].x;
        out.vertices[2 * i].y = vertices[i].y + width;
        out.vertices[2 * i + 1].y = vertices[i].y - width;
    }

    // Indices
    for (size_t i = 0; i < n - 1; ++i)
    {
        out.indices[6 * i] = 2 * i;
        out.indices[6 * i + 1] = 2 * i + 1;
        out.indices[6 * i + 2] = 2 * (i + 1);
        out.indices[6 * i + 3] = 2 * (i + 1);
        out.indices[6 * i + 4] = 2 * i + 1;
        out.indices[6 * i + 5] = 2 * (i + 1) + 1;
    }

    return out;
}

Mesh diamond(vec3 point, float offset)
{
    Mesh out;
    out.num_vertices = 4;
    out.num_indices = 6;
    out.vertices = calloc(out.num_vertices, sizeof(vec3));
    out.indices = calloc(out.num_indices, sizeof(uint));

    out.vertices[0] = (vec3) {point.x - offset, point.y, point.z};
    out.vertices[1] = (vec3) {point.x, point.y - offset, point.z};
    out.vertices[2] = (vec3) {point.x + offset, point.y, point.z};
    out.vertices[3] = (vec3) {point.x, point.y + offset, point.z};

    *out.indices = (uint) {0, 1, 2, 2, 3, 0};

    return out;
}

int main(void)
{
    /* Startup */
    GLFWwindow *window = init_glfw(framebuffer_size_callback);

    /* Common */
    const char vertex_shader_source[] =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\n\0";
    const char fragment_shader_source[] =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";

    /* Triangle */
    GameObject triangle;
    triangle.vertex_shader_source = strdup(vertex_shader_source);
    triangle.fragment_shader_source = strdup(fragment_shader_source);
    triangle.mesh.vertices = (vec3[]){
        {-0.5, -0.5, 0.0},
        { 0.5, -0.5, 0.0},
        { 0.0,  0.5, 0.0},
        { 1.0,  1.0, 0.0},
    };
    triangle.mesh.num_vertices = 3;
    triangle.mesh.num_indices = 6;
    triangle.mesh.indices = (uint[]){
        0, 1, 2, 1, 2, 3
    };
    setup(&triangle);

    /* Rect */
    GameObject rect;
    rect.vertex_shader_source = strdup(vertex_shader_source);
    rect.fragment_shader_source = strdup(fragment_shader_source);
    rect.mesh.num_vertices = 4;
    rect.mesh.vertices = (vec3[]){
        { 0.5f,  0.5f, 0.0f},  // Top Right
        { 0.5f, -0.5f, 0.0f},  // Bottom Right
        {-0.5f,  0.5f, 0.0f},  // Top Left
        {-0.5f, -0.5f, 0.0f},  // Bottom Left
    };
    rect.mesh.num_indices = 6;
    rect.mesh.indices = (uint[]){
        0, 1, 3,
        0, 3, 2
    };
    setup(&rect);

    /* Axes */
    GameObject xaxis;
    xaxis.vertex_shader_source = strdup(vertex_shader_source);
    xaxis.fragment_shader_source = strdup(fragment_shader_source);
    xaxis.mesh.num_vertices = 4;
    xaxis.mesh.num_indices = 6;
    xaxis.mesh.vertices = (vec3[]){
        {-1.0, -0.01, 0.0},
        {-1.0,  0.01, 0.0},
        { 1.0,  0.01, 0.0},
        { 1.0, -0.01, 0.0},
    };
    xaxis.mesh.indices = (uint[]){
        0, 1, 2, 2, 3, 0,
    };
    setup(&xaxis);

    GameObject yaxis;
    yaxis.vertex_shader_source = strdup(vertex_shader_source);
    yaxis.fragment_shader_source = strdup(fragment_shader_source);
    yaxis.mesh.num_vertices = 4;
    yaxis.mesh.num_indices = 6;
    yaxis.mesh.vertices = (vec3[]){
        {-0.01, -1.0, 0.0},
        { 0.01, -1.0, 0.0},
        { 0.01,  1.0, 0.0},
        {-0.01,  1.0, 0.0},
    };
    yaxis.mesh.indices = (uint[]){
        0, 1, 2, 2, 3, 0,
    };
    setup(&yaxis);

    /* Plot */
    float width = 0.01f;

    size_t n1 = 5;
    vec3 vertices[] = {
        {0.0, 0.0, 0.0},
        {0.25, 0.0625, 0.0},
        {0.50, 0.25, 0.0},
        {0.75, 0.5625, 0.0},
        {1.0, 1.0, 0.0},
    };

    // Create GameObject
    GameObject plot1;
    plot1.vertex_shader_source = strdup(vertex_shader_source);
    plot1.fragment_shader_source = strdup(fragment_shader_source);
    plot1.mesh = line(n1, vertices, width);
    for (size_t i = 0; i < plot1.mesh.num_vertices; ++i)
    {
        print_vec3(&plot1.mesh.vertices[i]);
        printf("\n");
    }
    for (size_t i = 0; i < plot1.mesh.num_indices; ++i)
    {
        printf("i = %d\n", plot1.mesh.indices[i]);
    }
    setup(&plot1);

    /* Plot from file */
    size_t n2 = 100;
    vec3 *vertices2 = calloc(n2, sizeof(vec3));
    FILE *file;
    file = fopen("quad.csv", "r");
    for (int i = 0; i < n2; ++i)
    {
        fscanf(file, "%f, %f, %f", &vertices2[i].x, &vertices2[i].y, &vertices2[i].z);
    }

    GameObject plot2;
    plot2.vertex_shader_source = strdup(vertex_shader_source);
    plot2.fragment_shader_source = strdup(fragment_shader_source);
    plot2.mesh = line_naive(n2, vertices2, width);
    for (size_t i = 0; i < plot2.mesh.num_vertices; ++i)
    {
        printf("i: %i ", i);
        print_vec3(&plot2.mesh.vertices[i]);
        printf("\n");
    }
    // for (size_t i = 0; i < plot2.mesh.num_indices; ++i)
    // {
    //     printf("i = %d\n", plot2.mesh.indices[i]);
    // }
    setup(&plot2);

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
        // draw(&plot1);
        draw(&plot2);

        // Check and call events and swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    printf("Closing window\n");

    /* Delete stuff and terminate */
    delete_GameObject(&triangle);
    // delete_GameObject(&rect);
    // delete_GameObject(&xaxis);
    // delete_GameObject(&yaxis);
    // delete_GameObject(&plot1);
    glfwTerminate();

    return 0;
}
