#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

struct Point
{
    float x;
    float y;
};

vector<Point> points;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// 3x3 Homogeneous Transformation Matrix
struct Matrix3
{
    float m[3][3];
};

// Rotation matrix
Matrix3 rotationMatrix(float angle)
{
    float rad = angle * 3.14159265f / 180.0f;

    Matrix3 R = {{
        {cos(rad), -sin(rad), 0.0f},
        {sin(rad),  cos(rad), 0.0f},
        {0.0f,      0.0f,     1.0f}
    }};

    return R;
}

// Apply rotation matrix to a point
Point transformPoint(Matrix3 R, Point p)
{
    Point result;

    result.x =
        R.m[0][0] * p.x +
        R.m[0][1] * p.y +
        R.m[0][2] * 1.0f;

    result.y =
        R.m[1][0] * p.x +
        R.m[1][1] * p.y +
        R.m[1][2] * 1.0f;

    return result;
}

// Add triangle to points
void addTriangle(Point p1, Point p2, Point p3)
{
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
}

// Vertex Shader
const char* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}";

// Fragment Shader
const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(color, 1.0);\n"
"}";

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
        "2D Rotation using Homogeneous Coordinates",
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

    // ---------------------------------------------
    // Original Triangle
    // ---------------------------------------------

    Point p1 = {0.2f, -0.3f};
    Point p2 = {0.6f, -0.3f};
    Point p3 = {0.4f,  0.2f};

    addTriangle(p1, p2, p3);

    // ---------------------------------------------
    // Rotation
    // ---------------------------------------------

    float angle = 45.0f;

    Matrix3 R = rotationMatrix(angle);

    Point rp1 = transformPoint(R, p1);
    Point rp2 = transformPoint(R, p2);
    Point rp3 = transformPoint(R, p3);

    addTriangle(rp1, rp2, rp3);

    // ---------------------------------------------
    // Vertex Shader
    // ---------------------------------------------

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL
    );

    glCompileShader(vertexShader);

    // ---------------------------------------------
    // Fragment Shader
    // ---------------------------------------------

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL
    );

    glCompileShader(fragmentShader);

    // ---------------------------------------------
    // Shader Program
    // ---------------------------------------------

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

    // ---------------------------------------------
    // VAO and VBO
    // ---------------------------------------------

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

    // ---------------------------------------------
    // Main Loop
    // ---------------------------------------------

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

        // Rotated triangle - green
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

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}