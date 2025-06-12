#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// Callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Vertex Shader
const GLchar* vertexShaderSource = "#version 450 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

// Fragment Shader
const GLchar* fragmentShaderSource = "#version 450 core\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

// Controle
float translateX = 0.0f, translateY = 0.0f, translateZ = -3.0f;
float scaleFactor = 1.0f;
bool rotateX = false, rotateY = false, rotateZ = false;

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo 3D - Mateus Biscardi", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();
    GLuint VAO = setupGeometry();
    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, 0.1f, 100.0f);

    // Instâncias de cubos com posições diferentes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 0.0f, 0.0f),
        glm::vec3(-2.0f, 0.0f, 0.0f),
 
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = glfwGetTime();
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(translateX, translateY, translateZ));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 4; i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            model = glm::scale(model, glm::vec3(scaleFactor));
            if (rotateX)
                model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
            else if (rotateY)
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            else if (rotateZ)
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if (key == GLFW_KEY_X) rotateX = true, rotateY = false, rotateZ = false;
        if (key == GLFW_KEY_Y) rotateX = false, rotateY = true, rotateZ = false;
        if (key == GLFW_KEY_Z) rotateX = false, rotateY = false, rotateZ = true;

        if (key == GLFW_KEY_W) translateZ += 0.1f;
        if (key == GLFW_KEY_S) translateZ -= 0.1f;
        if (key == GLFW_KEY_A) translateX -= 0.1f;
        if (key == GLFW_KEY_D) translateX += 0.1f;
        if (key == GLFW_KEY_I) translateY += 0.1f;
        if (key == GLFW_KEY_J) translateY -= 0.1f;

        if (key == GLFW_KEY_LEFT_BRACKET) scaleFactor -= 0.05f;
        if (key == GLFW_KEY_RIGHT_BRACKET) scaleFactor += 0.05f;
    }
}

int setupShader()
{
    GLint success;
    GLchar infoLog[512];

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "Erro Vertex Shader: " << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "Erro Fragment Shader: " << infoLog << endl;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        cout << "Erro Shader Program: " << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

int setupGeometry()
{
    GLfloat vertices[] = {
        // Face frontal (vermelho)
        -0.5f, -0.5f,  0.5f,   1, 0, 0,
         0.5f, -0.5f,  0.5f,   1, 0, 0,
         0.5f,  0.5f,  0.5f,   1, 0, 0,
         0.5f,  0.5f,  0.5f,   1, 0, 0,
        -0.5f,  0.5f,  0.5f,   1, 0, 0,
        -0.5f, -0.5f,  0.5f,   1, 0, 0,

        // Face traseira (verde)
        -0.5f, -0.5f, -0.5f,   0, 1, 0,
        -0.5f,  0.5f, -0.5f,   0, 1, 0,
         0.5f,  0.5f, -0.5f,   0, 1, 0,
         0.5f,  0.5f, -0.5f,   0, 1, 0,
         0.5f, -0.5f, -0.5f,   0, 1, 0,
        -0.5f, -0.5f, -0.5f,   0, 1, 0,

        // Face esquerda (azul)
        -0.5f,  0.5f,  0.5f,   0, 0, 1,
        -0.5f,  0.5f, -0.5f,   0, 0, 1,
        -0.5f, -0.5f, -0.5f,   0, 0, 1,
        -0.5f, -0.5f, -0.5f,   0, 0, 1,
        -0.5f, -0.5f,  0.5f,   0, 0, 1,
        -0.5f,  0.5f,  0.5f,   0, 0, 1,

        // Face direita (amarelo)
         0.5f,  0.5f,  0.5f,   1, 1, 0,
         0.5f, -0.5f,  0.5f,   1, 1, 0,
         0.5f, -0.5f, -0.5f,   1, 1, 0,
         0.5f, -0.5f, -0.5f,   1, 1, 0,
         0.5f,  0.5f, -0.5f,   1, 1, 0,
         0.5f,  0.5f,  0.5f,   1, 1, 0,

        // Face inferior (ciano)
        -0.5f, -0.5f, -0.5f,   0, 1, 1,
         0.5f, -0.5f, -0.5f,   0, 1, 1,
         0.5f, -0.5f,  0.5f,   0, 1, 1,
         0.5f, -0.5f,  0.5f,   0, 1, 1,
        -0.5f, -0.5f,  0.5f,   0, 1, 1,
        -0.5f, -0.5f, -0.5f,   0, 1, 1,

        // Face superior (magenta)
        -0.5f,  0.5f, -0.5f,   1, 0, 1,
        -0.5f,  0.5f,  0.5f,   1, 0, 1,
         0.5f,  0.5f,  0.5f,   1, 0, 1,
         0.5f,  0.5f,  0.5f,   1, 0, 1,
         0.5f,  0.5f, -0.5f,   1, 0, 1,
        -0.5f,  0.5f, -0.5f,   1, 0, 1
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    return VAO;
}
