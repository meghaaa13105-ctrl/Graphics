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

// --------------------------------------------------
// Clipping Window
// --------------------------------------------------

const float XMIN = -0.6f;
const float XMAX =  0.6f;
const float YMIN = -0.4f;
const float YMAX =  0.4f;

// --------------------------------------------------
// Region Codes
// --------------------------------------------------

const int INSIDE = 0; // 0000
const int LEFT   = 1; // 0001
const int RIGHT  = 2; // 0010
const int BOTTOM = 4; // 0100
const int TOP    = 8; // 1000

// --------------------------------------------------
// Get Region Code
// --------------------------------------------------

int computeCode(Point p)
{
    int code = INSIDE;

    if (p.x < XMIN)
        code |= LEFT;

    else if (p.x > XMAX)
        code |= RIGHT;

    if (p.y < YMIN)
        code |= BOTTOM;

    else if (p.y > YMAX)
        code |= TOP;

    return code;
}

// --------------------------------------------------
// Cohen-Sutherland Line Clipping
// --------------------------------------------------

bool cohenSutherlandClip(Point &p1, Point &p2)
{
    int code1 = computeCode(p1);
    int code2 = computeCode(p2);

    bool accept = false;

    while (true)
    {
        // Both points inside
        if ((code1 | code2) == 0)
        {
            accept = true;
            break;
        }

        // Both points share an outside region
        else if (code1 & code2)
        {
            break;
        }

        // One point is outside
        else
        {
            float x, y;

            int outsideCode;

            if (code1 != 0)
                outsideCode = code1;
            else
                outsideCode = code2;

            // TOP
            if (outsideCode & TOP)
            {
                x = p1.x +
                    (p2.x - p1.x) *
                    (YMAX - p1.y) /
                    (p2.y - p1.y);

                y = YMAX;
            }

            // BOTTOM
            else if (outsideCode & BOTTOM)
            {
                x = p1.x +
                    (p2.x - p1.x) *
                    (YMIN - p1.y) /
                    (p2.y - p1.y);

                y = YMIN;
            }

            // RIGHT
            else if (outsideCode & RIGHT)
            {
                y = p1.y +
                    (p2.y - p1.y) *
                    (XMAX - p1.x) /
                    (p2.x - p1.x);

                x = XMAX;
            }

            // LEFT
            else
            {
                y = p1.y +
                    (p2.y - p1.y) *
                    (XMIN - p1.x) /
                    (p2.x - p1.x);

                x = XMIN;
            }

            // Replace outside point
            if (outsideCode == code1)
            {
                p1.x = x;
                p1.y = y;

                code1 = computeCode(p1);
            }
            else
            {
                p2.x = x;
                p2.y = y;

                code2 = computeCode(p2);
            }
        }
    }

    return accept;
}

// --------------------------------------------------
// Add Line to Vector
// --------------------------------------------------

vector<Point> points;

void addLine(Point p1, Point p2)
{
    points.push_back(p1);
    points.push_back(p2);
}

// --------------------------------------------------
// Window Resize
// --------------------------------------------------

void framebuffer_size_callback(
    GLFWwindow* window,
    int width,
    int height)
{
    glViewport(0, 0, width, height);
}

// --------------------------------------------------
// ESC to Close
// --------------------------------------------------

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(
        window,
        GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(
            window,
            true);
    }
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

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    // Initialize GLFW
    glfwInit();

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3);

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3);

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__

    glfwWindowHint(
        GLFW_OPENGL_FORWARD_COMPAT,
        GL_TRUE);

#endif

    // Create Window
    GLFWwindow* window =
        glfwCreateWindow(
            800,
            600,
            "Cohen-Sutherland Line Clipping",
            NULL,
            NULL);

    if (window == NULL)
    {
        cout << "Failed to create GLFW Window\n";

        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(
        window,
        framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader(
        (GLADloadproc)glfwGetProcAddress))
    {
        cout << "Failed to initialize GLAD\n";

        return -1;
    }

    // --------------------------------------------------
    // Original Line
    // --------------------------------------------------

    Point p1 = {-0.9f, -0.7f};
    Point p2 = { 0.9f,  0.7f};

    // Save original points
    Point originalP1 = p1;
    Point originalP2 = p2;

    // --------------------------------------------------
    // Print Region Codes
    // --------------------------------------------------

    int code1 = computeCode(p1);
    int code2 = computeCode(p2);

    cout << "\n========================================\n";
    cout << " Cohen-Sutherland Line Clipping\n";
    cout << "========================================\n";

    cout << "\nClipping Window:\n";

    cout << "XMIN = " << XMIN << endl;
    cout << "XMAX = " << XMAX << endl;
    cout << "YMIN = " << YMIN << endl;
    cout << "YMAX = " << YMAX << endl;

    cout << "\nOriginal Line:\n";

    cout << "P1 = ("
         << p1.x << ", "
         << p1.y << ")\n";

    cout << "P2 = ("
         << p2.x << ", "
         << p2.y << ")\n";

    cout << "\nRegion Codes:\n";

    cout << "P1 Code = "
         << code1 << endl;

    cout << "P2 Code = "
         << code2 << endl;

    // --------------------------------------------------
    // Perform Clipping
    // --------------------------------------------------

    bool accepted =
        cohenSutherlandClip(p1, p2);

    if (accepted)
    {
        cout << "\nLine ACCEPTED\n";

        cout << "\nClipped Line:\n";

        cout << "P1 = ("
             << p1.x << ", "
             << p1.y << ")\n";

        cout << "P2 = ("
             << p2.x << ", "
             << p2.y << ")\n";

        addLine(p1, p2);
    }
    else
    {
        cout << "\nLine REJECTED\n";
    }

    cout << "\n========================================\n";

    // --------------------------------------------------
    // Create Clipping Window Border
    // --------------------------------------------------

    vector<Point> border;


Point b1;
b1.x = XMIN;
b1.y = YMIN;
border.push_back(b1);

Point b2;
b2.x = XMAX;
b2.y = YMIN;
border.push_back(b2);

Point b3;
b3.x = XMAX;
b3.y = YMAX;
border.push_back(b3);

Point b4;
b4.x = XMIN;
b4.y = YMAX;
border.push_back(b4);

border.push_back(b1);

    // --------------------------------------------------
    // Shader Compilation
    // --------------------------------------------------

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        NULL);

    glCompileShader(vertexShader);

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        NULL);

    glCompileShader(fragmentShader);

    // --------------------------------------------------
    // Shader Program
    // --------------------------------------------------

    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(
        shaderProgram,
        vertexShader);

    glAttachShader(
        shaderProgram,
        fragmentShader);

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --------------------------------------------------
    // VAO + VBO for Original Line
    // --------------------------------------------------

    unsigned int originalVAO;
    unsigned int originalVBO;

    glGenVertexArrays(1, &originalVAO);
    glGenBuffers(1, &originalVBO);

    glBindVertexArray(originalVAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        originalVBO);

    Point originalPoints[2] =
    {
        originalP1,
        originalP2
    };

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(originalPoints),
        originalPoints,
        GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (void*)0);

    glEnableVertexAttribArray(0);

    // --------------------------------------------------
    // VAO + VBO for Clipped Line
    // --------------------------------------------------

    unsigned int clippedVAO;
    unsigned int clippedVBO;

    glGenVertexArrays(1, &clippedVAO);
    glGenBuffers(1, &clippedVBO);

    glBindVertexArray(clippedVAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        clippedVBO);

    if (accepted)
    {
        glBufferData(
            GL_ARRAY_BUFFER,
            points.size() * sizeof(Point),
            points.data(),
            GL_STATIC_DRAW);
    }

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (void*)0);

    glEnableVertexAttribArray(0);

    // --------------------------------------------------
    // VAO + VBO for Clipping Window
    // --------------------------------------------------

    unsigned int borderVAO;
    unsigned int borderVBO;

    glGenVertexArrays(1, &borderVAO);
    glGenBuffers(1, &borderVBO);

    glBindVertexArray(borderVAO);

    glBindBuffer(
        GL_ARRAY_BUFFER,
        borderVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        border.size() * sizeof(Point),
        border.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Point),
        (void*)0);

    glEnableVertexAttribArray(0);

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
            1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        int colorLocation =
            glGetUniformLocation(
                shaderProgram,
                "color");

        // --------------------------------------------------
        // Draw Clipping Window
        // --------------------------------------------------

        glUniform3f(
            colorLocation,
            1.0f,
            1.0f,
            1.0f);

        glBindVertexArray(borderVAO);

        glLineWidth(2.0f);

        glDrawArrays(
            GL_LINES,
            0,
            8);

        // --------------------------------------------------
        // Draw Original Line
        // --------------------------------------------------

        glUniform3f(
            colorLocation,
            1.0f,
            1.0f,
            0.0f);

        glBindVertexArray(originalVAO);

        glLineWidth(1.0f);

        glDrawArrays(
            GL_LINES,
            0,
            2);

        // --------------------------------------------------
        // Draw Clipped Line
        // --------------------------------------------------

        if (accepted)
        {
            glUniform3f(
                colorLocation,
                0.0f,
                1.0f,
                0.0f);

            glBindVertexArray(clippedVAO);

            glLineWidth(4.0f);

            glDrawArrays(
                GL_LINES,
                0,
                2);
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // --------------------------------------------------
    // Cleanup
    // --------------------------------------------------

    glDeleteVertexArrays(
        1,
        &originalVAO);

    glDeleteBuffers(
        1,
        &originalVBO);

    glDeleteVertexArrays(
        1,
        &clippedVAO);

    glDeleteBuffers(
        1,
        &clippedVBO);

    glDeleteVertexArrays(
        1,
        &borderVAO);

    glDeleteBuffers(
        1,
        &borderVBO);

    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}