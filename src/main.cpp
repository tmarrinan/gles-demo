#include <iostream>
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct GShaderProgram {
    GLuint program;
    GLint projection_uniform;
    GLint view_uniform;
    GLint model_uniform;
    GLint solid_color_uniform;
    GLint shininess_uniform;
} GShaderProgram;

static void Init(GLFWwindow *window, GShaderProgram *shader, GLuint *vao);
static void Render(GLFWwindow *window, GShaderProgram *shader, GLuint vao);
static void GenerateShader(GShaderProgram *shader);
static void GenerateVao(GLuint *vao);
static GLint CompileShader(char *source, uint32_t length, GLint type);
static void CreateShaderProgram(GLint vertex_shader, GLint fragment_shader, GLuint *program);
static void LinkShaderProgram(GLuint program);
static int32_t ReadFile(const char *filename, char **data);


GLuint vertex_position_attrib = 0;
GLuint vertex_normal_attrib = 1;
float rotation;
double frame_time;
double prev_time;
uint32_t frame_count;


int main(int argc, char **argv)
{
    // initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Error initializing GLFW\n");
        exit(1);
    }

    // create a window and it's OpenGL ES context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(1280, 720, "GLES Example", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Error creating window\n");
        exit(1);
    }

    // make window's context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // initialize glad OpenGL extension loader
    if (!gladLoadGLES2Loader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Error initializing Glad\n");
        exit(1);
    }

    // initialize app
    GShaderProgram shader;
    GLuint vao;
    Init(window, &shader, &vao);
    
    // main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        Render(window, &shader, vao);
    }

    // clean up
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void Init(GLFWwindow *window, GShaderProgram *shader, GLuint *vao)
{
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    GenerateShader(shader);
    GenerateVao(vao);

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.3, 0.4, 0.6, 1.0);
    glEnable(GL_DEPTH_TEST);
    
    rotation = 0.0;
    frame_time = glfwGetTime();
    prev_time = frame_time;
    frame_count = 0;

    Render(window, shader, *vao);
}

void Render(GLFWwindow *window, GShaderProgram *shader, GLuint vao)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(shader->program);
    
    glm::mat4 projection = glm::perspective(42.5 * (M_PI / 180.0), 1280.0 / 720.0, 0.1, 100.0);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 8.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(2.5, 2.5, 2.5));
    model = glm::rotate(model, rotation, glm::vec3(0.0, 1.0, 0.0));
    
    glUniformMatrix4fv(shader->projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(shader->view_uniform, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(shader->model_uniform, 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(shader->solid_color_uniform, 0.8, 0.3, 0.1);
    glUniform1f(shader->shininess_uniform, 68.4);
    
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);

    frame_count++;
    double now = glfwGetTime();
    rotation += (M_PI / 4.0) * (now - prev_time);
    if (now - frame_time > 1.0)
    {
        double fps = frame_count / (now - frame_time);
        char title[22];
        snprintf(title, 22, "GLES Example - % 5.1lf", fps);
        glfwSetWindowTitle(window, title);
        frame_time = now;
        frame_count = 0;
    }
    prev_time = now;

    glfwSwapBuffers(window);
}

void GenerateShader(GShaderProgram *shader)
{
    const char *vertex_file = "resrc/shaders/phong_es3.vert";
    char *vertex_src;
    int32_t vertex_src_length = ReadFile(vertex_file, &vertex_src);
    GLint vertex_shader = CompileShader(vertex_src, vertex_src_length, GL_VERTEX_SHADER);
    free(vertex_src);

    const char *fragment_file = "resrc/shaders/phong_es3.frag";
    char *fragment_src;
    int32_t fragment_src_length = ReadFile(fragment_file, &fragment_src);
    GLint fragment_shader = CompileShader(fragment_src, fragment_src_length, GL_FRAGMENT_SHADER);
    free(fragment_src);

    CreateShaderProgram(vertex_shader, fragment_shader, &(shader->program));

    glBindAttribLocation(shader->program, vertex_position_attrib, "aVertexPosition");
    glBindAttribLocation(shader->program, vertex_normal_attrib, "aVertexNormal");
    glBindAttribLocation(shader->program, 0, "FragColor");

    LinkShaderProgram(shader->program);

    shader->projection_uniform = glGetUniformLocation(shader->program, "uProjectionMatrix");
    shader->view_uniform = glGetUniformLocation(shader->program, "uViewMatrix");
    shader->model_uniform = glGetUniformLocation(shader->program, "uModelMatrix");
    shader->solid_color_uniform = glGetUniformLocation(shader->program, "uSolidColor");
    shader->shininess_uniform = glGetUniformLocation(shader->program, "uShininess");
}

void GenerateVao(GLuint *vao)
{
    GLuint buffer;
    
    // vertex array for cube
    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);
    
    // buffer for vertex positions
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat vertices[72] = {
         0.5, -0.5,  0.5,   0.5, -0.5, -0.5,   0.5,  0.5, -0.5,   0.5,  0.5,  0.5, // right
        -0.5, -0.5, -0.5,  -0.5, -0.5,  0.5,  -0.5,  0.5,  0.5,  -0.5,  0.5, -0.5, // left
        -0.5,  0.5,  0.5,   0.5,  0.5,  0.5,   0.5,  0.5, -0.5,  -0.5,  0.5, -0.5, // top
         0.5, -0.5,  0.5,  -0.5, -0.5,  0.5,  -0.5, -0.5, -0.5,   0.5, -0.5, -0.5, // bottom
        -0.5, -0.5,  0.5,   0.5, -0.5,  0.5,   0.5,  0.5,  0.5,  -0.5,  0.5,  0.5, // front
         0.5, -0.5, -0.5,  -0.5, -0.5, -0.5,  -0.5,  0.5, -0.5,   0.5,  0.5, -0.5  // back
    };
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_position_attrib);
    glVertexAttribPointer(vertex_position_attrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    // buffer for vertex normals
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    GLfloat normals[72] = {
         1.0,  0.0,  0.0,   1.0,  0.0,  0.0,   1.0,  0.0,  0.0,   1.0,  0.0,  0.0, // right
        -1.0,  0.0,  0.0,  -1.0,  0.0,  0.0,  -1.0,  0.0,  0.0,  -1.0,  0.0,  0.0, // left
         0.0,  1.0,  0.0,   0.0,  1.0,  0.0,   0.0,  1.0,  0.0,   0.0,  1.0,  0.0, // top
         0.0, -1.0,  0.0,   0.0, -1.0,  0.0,   0.0, -1.0,  0.0,   0.0, -1.0,  0.0, // bottom
         0.0,  0.0,  1.0,   0.0,  0.0,  1.0,   0.0,  0.0,  1.0,   0.0,  0.0,  1.0, // front
         0.0,  0.0, -1.0,   0.0,  0.0, -1.0,   0.0,  0.0, -1.0,   0.0,  0.0, -1.0  // back
    };
    glBufferData(GL_ARRAY_BUFFER, 72 * sizeof(GLfloat), normals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertex_normal_attrib);
    glVertexAttribPointer(vertex_normal_attrib, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    // buffer for face indices
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    GLushort indices[36] = {
         0,  1,  2,   0,  2,  3, // right
         4,  5,  6,   4,  6,  7, // left
         8,  9, 10,   8, 10, 11, // top
        12, 13, 14,  12, 14, 15, // bottom
        16, 17, 18,  16, 18, 19, // front
        20, 21, 22,  20, 22, 23  // back
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLushort), indices, GL_STATIC_DRAW);
    
    // clean up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLint CompileShader(char *source, uint32_t length, GLint type)
{
    GLint status;
    GLint shader = glCreateShader(type);

    const char *src_bytes = const_cast<const char*>(source);
    const GLint len = length;
    glShaderSource(shader, 1, &src_bytes, &len);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        char *info = (char*)malloc(log_length + 1);
        glGetShaderInfoLog(shader, log_length, NULL, info);
        if (type == GL_VERTEX_SHADER)
        {
            fprintf(stderr, "Error: failed to compile vertex shader: %s\n", info);
        }
        else
        {
            fprintf(stderr, "Error: failed to compile fragment shader: %s\n", info);
        }
        free(info);

        return -1;
    }

    return shader;
}

void CreateShaderProgram(GLint vertex_shader, GLint fragment_shader, GLuint *program)
{
    *program = glCreateProgram();
    glAttachShader(*program, vertex_shader);
    glAttachShader(*program, fragment_shader);
}

void LinkShaderProgram(GLuint program)
{
    GLint status;
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == 0)
    {
        fprintf(stderr, "Error: unable to initialize shader program\n");
    }
}

int32_t ReadFile(const char *filename, char **data)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: cannot open %s\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int32_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    *data = (char*)malloc(fsize);
    size_t read = fread(*data, fsize, 1, fp);
    if (read != 1)
    {
        fprintf(stderr, "Error: cannot read %s\n", filename);
        return -1;
    }
    fclose(fp);

    return fsize;
}

