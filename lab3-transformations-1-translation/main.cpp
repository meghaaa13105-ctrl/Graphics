#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

using namespace std;

struct Point
{
    float x;
    float y;
};

vector<Point> points;

// --------------------------------------------------
// Window
// --------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// --------------------------------------------------
// 3x3 Homogeneous Transformation Matrix
// --------------------------------------------------

struct Matrix3
{
    float m[3][3];
};

// Translation matrix
Matrix3 translationMatrix(float tx, float ty)
{
    Matrix3 T = {{
        {1.0f, 0.0f, tx},
        {0.0f, 1.0f, ty},
        {0.0f, 0.0f, 1.0f}
    }};

    return T;
}

// --------------------------------------------------
// Apply transformation matrix to a point
// --------------------------------------------------

Point transformPoint(Matrix3 T, Point p)
{
    Point result;

    // Homogeneous coordinates:
    // [x']   [1  0  tx] [x]
    // [y'] = [0  1  ty] [y]
    // [1 ]   [0  0  1 ] [1]

    result.x =
        T.m[0][0] * p.x +
        T.m[0][1] * p.y +
        T.m[0][2] * 1.0f;

    result.y =
        T.m[1][0] * p.x +
        T.m[1][1] * p.y +
        T.m[1][2] * 1.0f;

    return result;
}

// --------------------------------------------------
// Add triangle points
// --------------------------------------------------

void addTriangle(Point p1, Point p2, Point p3)
{
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
}

// --------------------------------------------------
// Shaders
// --------------------------------------------------

const char* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}";

const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(color, 1.0);\n"
"}";

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    // Initialize GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

#ifdef __APPLE__
    glfwWindowHint(
        GLFW_OPENGL_FORWARD_COMPAT,
        GL_TRUE
    );
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "2D Translation using Homogeneous Coordinates",
        NULL,
        NULL
    );

    if (window == NULL)
    {
        cout << "Failed to create GLFW Window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback
    );

    // Initialize GLAD
    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // --------------------------------------------------
    // Original Triangle
    // --------------------------------------------------

    Point p1 = {-0.6f, -0.3f};
    Point p2 = {-0.2f, -0.3f};
    Point p3 = {-0.4f,  0.2f};

    // Add original triangle
    addTriangle(p1, p2, p3);

    // --------------------------------------------------
    // Translation
    // --------------------------------------------------

    float tx = 0.8f;
    float ty = 0.0f;

    Matrix3 T = translationMatrix(tx, ty);

    // Transform triangle vertices
    Point tp1 = transformPoint(T, p1);
    Point tp2 = transformPoint(T, p2);
    Point tp3 = transformPoint(T, p3);

    // Add translated triangle
    addTriangle(tp1, tp2, tp3);

    // --------------------------------------------------
    // Compile Vertex Shader
    // --------------------------------------------------

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    // --------------------------------------------------
    // Compile Fragment Shader
    // --------------------------------------------------

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    // --------------------------------------------------
    // Shader Program
    // --------------------------------------------------

    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(
        shaderProgram,
        vertexShader
    );

    glAttachShader(
        shaderProgram,
        fragmentShader
    );

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --------------------------------------------------
    // VAO and VBO
    // --------------------------------------------------

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        VBO
    );

    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(Point),
        points.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // --------------------------------------------------
    // Main Loop
    // --------------------------------------------------

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(
            0.0f,
            0.0f,
            0.0f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(VAO);

        int colorLocation =
            glGetUniformLocation(
                shaderProgram,
                "color"
            );

        // Original triangle - white
        glUniform3f(
            colorLocation,
            1.0f,
            1.0f,
            1.0f
        );

        glDrawArrays(
            GL_LINE_LOOP,
            0,
            3
        );

        // Translated triangle - green
        glUniform3f(
            colorLocation,
            0.0f,
            1.0f,
            0.0f
        );

        glDrawArrays(
            GL_LINE_LOOP,
            3,
            3
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --------------------------------------------------
    // Cleanup
    // --------------------------------------------------

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}