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

// 3x3 Matrix
struct Matrix3
{
    float m[3][3];
};

// --------------------------------------------------
// Print Matrix
// --------------------------------------------------

void printMatrix(Matrix3 M, string name)
{
    cout << "\n" << name << ":\n";

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            cout << M.m[i][j] << "\t";
        }

        cout << endl;
    }
}

// --------------------------------------------------
// Matrix multiplication
// --------------------------------------------------

Matrix3 multiply(Matrix3 A, Matrix3 B)
{
    Matrix3 result = {};

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                result.m[i][j] +=
                    A.m[i][k] * B.m[k][j];
            }
        }
    }

    return result;
}

// --------------------------------------------------
// Scaling Matrix
// --------------------------------------------------

Matrix3 scalingMatrix(float sx, float sy)
{
    Matrix3 S = {{
        {sx, 0.0f, 0.0f},
        {0.0f, sy, 0.0f},
        {0.0f, 0.0f, 1.0f}
    }};

    return S;
}

// --------------------------------------------------
// Rotation Matrix
// --------------------------------------------------

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

// --------------------------------------------------
// Apply matrix to point
// --------------------------------------------------

Point transformPoint(Matrix3 M, Point p)
{
    Point result;

    result.x =
        M.m[0][0] * p.x +
        M.m[0][1] * p.y +
        M.m[0][2];

    result.y =
        M.m[1][0] * p.x +
        M.m[1][1] * p.y +
        M.m[1][2];

    return result;
}

// --------------------------------------------------
// Add triangle
// --------------------------------------------------

void addTriangle(Point p1, Point p2, Point p3)
{
    points.push_back(p1);
    points.push_back(p2);
    points.push_back(p3);
}

// --------------------------------------------------
// Vertex Shader
// --------------------------------------------------

const char* vertexShaderSource =
"#version 330 core\n"
"layout(location = 0) in vec2 aPos;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(aPos, 0.0, 1.0);\n"
"}";

// --------------------------------------------------
// Fragment Shader
// --------------------------------------------------

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

    // Create Window
    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Composite Rotation using Matrix Representation",
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

    Point p1 = {-0.5f, -0.3f};
    Point p2 = {-0.1f, -0.3f};
    Point p3 = {-0.3f,  0.2f};

    addTriangle(p1, p2, p3);

    // ---------------------------------------------
    // Composite Transformation
    // ---------------------------------------------

    float sx = 1.5f;
    float sy = 1.5f;

    float angle = 45.0f;

    Matrix3 S = scalingMatrix(sx, sy);

    Matrix3 R = rotationMatrix(angle);

    // Scaling first, Rotation second
    Matrix3 M = multiply(R, S);

    // ---------------------------------------------
    // PRINT MATRICES
    // ---------------------------------------------

    printMatrix(S, "Scaling Matrix (S)");
    printMatrix(R, "Rotation Matrix (R)");
    printMatrix(M, "Composite Matrix (M = R x S)");

    // ---------------------------------------------
    // Apply composite matrix
    // ---------------------------------------------

    Point cp1 = transformPoint(M, p1);
    Point cp2 = transformPoint(M, p2);
    Point cp3 = transformPoint(M, p3);

    addTriangle(cp1, cp2, cp3);

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

        // Original Triangle - White
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

        // Composite Triangle - Green
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