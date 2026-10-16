
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>

// Vertex shader
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPosition = model * vec4(aPos, 1.0);

    FragPos = worldPosition.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projection * view * worldPosition;
}
)";

// Fragment shader: Phong illumination model
const char* fragmentShaderSource = R"(
#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main()
{
    // 1. Ambient reflection
    float ambientStrength = 0.15;
    vec3 ambient = ambientStrength * objectColor;

    // Normalize surface normal
    vec3 norm = normalize(Normal);

    // Direction from surface to light
    vec3 lightDir = normalize(lightPos - FragPos);

    // 2. Diffuse reflection
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * objectColor;

    // Direction from surface to camera
    vec3 viewDir = normalize(viewPos - FragPos);

    // Reflection direction of incoming light
    vec3 reflectDir = reflect(-lightDir, norm);

    // 3. Specular reflection
    float spec = pow(
        max(dot(viewDir, reflectDir), 0.0),
        32.0
    );

    float specularStrength = 0.65;
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Combine all three lighting components
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}
)";

// Compile an individual shader
unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed:\n"
                  << infoLog << '\n';
    }

    return shader;
}

// Create and link the shader program
unsigned int createShaderProgram()
{
    unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking failed:\n"
                  << infoLog << '\n';
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Generate one sphere vertex with its normal
void addVertex(std::vector<float>& vertices,
               float phi, float theta, float radius)
{
    float nx = sin(phi) * cos(theta);
    float ny = cos(phi);
    float nz = sin(phi) * sin(theta);

    vertices.push_back(radius * nx);
    vertices.push_back(radius * ny);
    vertices.push_back(radius * nz);

    vertices.push_back(nx);
    vertices.push_back(ny);
    vertices.push_back(nz);
}

// Generate sphere triangles
std::vector<float> createSphere()
{
    std::vector<float> vertices;

    const float PI = 3.14159265358979323846f;
    const float radius = 0.85f;

    const int sectors = 48;
    const int stacks = 32;

    for (int i = 0; i < stacks; i++)
    {
        float phi1 = PI * i / stacks;
        float phi2 = PI * (i + 1) / stacks;

        for (int j = 0; j < sectors; j++)
        {
            float theta1 = 2.0f * PI * j / sectors;
            float theta2 = 2.0f * PI * (j + 1) / sectors;

            // Two triangles form each section of the sphere
            addVertex(vertices, phi1, theta1, radius);
            addVertex(vertices, phi2, theta1, radius);
            addVertex(vertices, phi2, theta2, radius);

            addVertex(vertices, phi1, theta1, radius);
            addVertex(vertices, phi2, theta2, radius);
            addVertex(vertices, phi1, theta2, radius);
        }
    }

    return vertices;
}

// Create a perspective projection matrix
void createPerspective(float* m, float fov, float aspect,
                       float nearPlane, float farPlane)
{
    for (int i = 0; i < 16; i++)
        m[i] = 0.0f;

    float f = 1.0f / tan(fov / 2.0f);

    m[0] = f / aspect;
    m[5] = f;
    m[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
    m[11] = -1.0f;
    m[14] = (2.0f * farPlane * nearPlane) /
            (nearPlane - farPlane);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    GLFWwindow* window = glfwCreateWindow(
        800, 600,
        "Lab 7 - Phong Illumination Model",
        nullptr, nullptr
    );

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, 800, 600);

    // Enable depth testing for the 3D object
    glEnable(GL_DEPTH_TEST);

    unsigned int shaderProgram = createShaderProgram();

    // Generate sphere geometry
    std::vector<float> vertices = createSphere();

    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // Position attribute
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float), (void*)0
    );
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Model matrix: identity
    float model[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // View matrix: move the camera back
    float view[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, -3, 1
    };

    float projection[16];

    const float PI = 3.14159265358979323846f;

    createPerspective(
        projection,
        45.0f * PI / 180.0f,
        800.0f / 600.0f,
        0.1f,
        100.0f
    );

    glUseProgram(shaderProgram);

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "model"),
        1, GL_FALSE, model
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "view"),
        1, GL_FALSE, view
    );

    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1, GL_FALSE, projection
    );

    // Light and camera positions in world coordinates
    glUniform3f(
        glGetUniformLocation(shaderProgram, "lightPos"),
        2.0f, 2.0f, 2.0f
    );

    glUniform3f(
        glGetUniformLocation(shaderProgram, "viewPos"),
        0.0f, 0.0f, 3.0f
    );

    // Blue sphere
    glUniform3f(
        glGetUniformLocation(shaderProgram, "objectColor"),
        0.2f, 0.45f, 0.95f
    );

    std::cout << "Phong Illumination Model\n";
    std::cout << "Ambient, diffuse and specular reflection enabled.\n";

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.04f, 0.04f, 0.06f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glDrawArrays(
            GL_TRIANGLES, 0,
            static_cast<GLsizei>(vertices.size() / 6)
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
