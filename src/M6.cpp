#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
glm::vec3 position = glm::vec3(0.0f);
float scale = 1.0f;

struct SceneObject {
    glm::vec3 position;
    std::vector<glm::vec3> trajectoryPoints;
    float speed = 0.01f;
    size_t currentTargetPoint = 0;
    bool isMoving = false;
    bool loopTrajectory = true;
};

std::vector<SceneObject> sceneObjects;

int selectedObjectIndex = 0;
bool showTrajectories = true;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

const char* trajectoryVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
}
)";

const char* trajectoryFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 1.0, 0.0, 1.0); // Amarelo para trajetórias
}
)";

float vertices[] = {
    // frente 
    -0.5f, -0.5f,  0.5f, 1, 0, 0,
     0.5f, -0.5f,  0.5f, 1, 0, 0,
     0.5f,  0.5f,  0.5f, 1, 0, 0,
     0.5f,  0.5f,  0.5f, 1, 0, 0,
    -0.5f,  0.5f,  0.5f, 1, 0, 0,
    -0.5f, -0.5f,  0.5f, 1, 0, 0,

    // trás 
    -0.5f, -0.5f, -0.5f, 0, 1, 0,
     0.5f, -0.5f, -0.5f, 0, 1, 0,
     0.5f,  0.5f, -0.5f, 0, 1, 0,
     0.5f,  0.5f, -0.5f, 0, 1, 0,
    -0.5f,  0.5f, -0.5f, 0, 1, 0,
    -0.5f, -0.5f, -0.5f, 0, 1, 0,

    // esquerda 
    -0.5f,  0.5f,  0.5f, 0, 0, 1,
    -0.5f,  0.5f, -0.5f, 0, 0, 1,
    -0.5f, -0.5f, -0.5f, 0, 0, 1,
    -0.5f, -0.5f, -0.5f, 0, 0, 1,
    -0.5f, -0.5f,  0.5f, 0, 0, 1,
    -0.5f,  0.5f,  0.5f, 0, 0, 1,

    // direita 
     0.5f,  0.5f,  0.5f, 1, 1, 0,
     0.5f,  0.5f, -0.5f, 1, 1, 0,
     0.5f, -0.5f, -0.5f, 1, 1, 0,
     0.5f, -0.5f, -0.5f, 1, 1, 0,
     0.5f, -0.5f,  0.5f, 1, 1, 0,
     0.5f,  0.5f,  0.5f, 1, 1, 0,

    // topo 
    -0.5f,  0.5f, -0.5f, 0, 1, 1,
     0.5f,  0.5f, -0.5f, 0, 1, 1,
     0.5f,  0.5f,  0.5f, 0, 1, 1,
     0.5f,  0.5f,  0.5f, 0, 1, 1,
    -0.5f,  0.5f,  0.5f, 0, 1, 1,
    -0.5f,  0.5f, -0.5f, 0, 1, 1,

    // fundo 
    -0.5f, -0.5f, -0.5f, 1, 0, 1,
     0.5f, -0.5f, -0.5f, 1, 0, 1,
     0.5f, -0.5f,  0.5f, 1, 0, 1,
     0.5f, -0.5f,  0.5f, 1, 0, 1,
    -0.5f, -0.5f,  0.5f, 1, 0, 1,
    -0.5f, -0.5f, -0.5f, 1, 0, 1
};

void updateObjects(float deltaTime) {
    for (auto& obj : sceneObjects) {
        if (!obj.isMoving || obj.trajectoryPoints.empty()) continue;

        glm::vec3 target = obj.trajectoryPoints[obj.currentTargetPoint];
        glm::vec3 direction = target - obj.position;
        float distance = glm::length(direction);

        if (distance < obj.speed) {
            obj.position = target;
            obj.currentTargetPoint++;
            
            if (obj.currentTargetPoint >= obj.trajectoryPoints.size()) {
                if (obj.loopTrajectory) {
                    obj.currentTargetPoint = 0;
                } else {
                    obj.isMoving = false;
                }
            }
        } else {
            obj.position += glm::normalize(direction) * obj.speed * deltaTime * 60.0f;
        }
    }
}

void processInput(GLFWwindow* window) {
    static float lastTime = 0.0f;
    float currentTime = glfwGetTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    float moveSpeed = 0.05f;
    float scaleSpeed = 0.02f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.z -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.z += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.x -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.x += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.y += moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) sceneObjects[selectedObjectIndex].position.y -= moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS) scale -= scaleSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS) scale += scaleSpeed;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) rotationX += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) rotationY += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) rotationZ += 1.0f;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        static double lastPressTime = 0;
        double currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 0.2) { 
            sceneObjects[selectedObjectIndex].trajectoryPoints.push_back(sceneObjects[selectedObjectIndex].position);
            std::cout << "Added trajectory point at (" 
                      << sceneObjects[selectedObjectIndex].position.x << ", "
                      << sceneObjects[selectedObjectIndex].position.y << ", "
                      << sceneObjects[selectedObjectIndex].position.z << ")\n";
            lastPressTime = currentTime;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        static double lastPressTime = 0;
        double currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 0.2) {
            sceneObjects[selectedObjectIndex].trajectoryPoints.clear();
            std::cout << "Cleared trajectory points for object " << selectedObjectIndex << "\n";
            lastPressTime = currentTime;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        static double lastPressTime = 0;
        double currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 0.2) {
            sceneObjects[selectedObjectIndex].isMoving = !sceneObjects[selectedObjectIndex].isMoving;
            std::cout << (sceneObjects[selectedObjectIndex].isMoving ? "Started" : "Stopped") 
                      << " movement for object " << selectedObjectIndex << "\n";
            lastPressTime = currentTime;
        }
    }

    for (int i = GLFW_KEY_1; i <= GLFW_KEY_9; ++i) {
        if (glfwGetKey(window, i) == GLFW_PRESS) {
            int newIndex = i - GLFW_KEY_1;
            if (newIndex < sceneObjects.size()) {
                selectedObjectIndex = newIndex;
                std::cout << "Selected object " << selectedObjectIndex << "\n";
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        static double lastPressTime = 0;
        double currentTime = glfwGetTime();
        if (currentTime - lastPressTime > 0.2) {
            showTrajectories = !showTrajectories;
            std::cout << (showTrajectories ? "Showing" : "Hiding") << " trajectories\n";
            lastPressTime = currentTime;
        }
    }

    updateObjects(deltaTime);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tarefa M6", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, nullptr);
    glCompileShader(vs);
    int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fs);
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    int trajectoryVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(trajectoryVS, 1, &trajectoryVertexShaderSource, nullptr);
    glCompileShader(trajectoryVS);
    int trajectoryFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(trajectoryFS, 1, &trajectoryFragmentShaderSource, nullptr);
    glCompileShader(trajectoryFS);
    int trajectoryShaderProgram = glCreateProgram();
    glAttachShader(trajectoryShaderProgram, trajectoryVS);
    glAttachShader(trajectoryShaderProgram, trajectoryFS);
    glLinkProgram(trajectoryShaderProgram);
    glDeleteShader(trajectoryVS);
    glDeleteShader(trajectoryFS);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int trajectoryVAO, trajectoryVBO;
    glGenVertexArrays(1, &trajectoryVAO);
    glGenBuffers(1, &trajectoryVBO);

    sceneObjects = {
        {{0.0f, 0.0f, 0.0f}, {}, 0.02f, 0, false, true},
        {{2.0f, 0.0f, -5.0f}, {}, 0.02f, 0, false, true},
        {{-2.0f, 1.0f, -3.0f}, {}, 0.02f, 0, false, true}
    };

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 100.0f);

        if (showTrajectories) {
            glUseProgram(trajectoryShaderProgram);
            glUniformMatrix4fv(glGetUniformLocation(trajectoryShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(trajectoryShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            for (size_t i = 0; i < sceneObjects.size(); ++i) {
                const auto& obj = sceneObjects[i];
                if (obj.trajectoryPoints.size() < 2) continue;

                std::vector<glm::vec3> trajectoryVertices;
                for (const auto& point : obj.trajectoryPoints) {
                    trajectoryVertices.push_back(point);
                }
                if (obj.loopTrajectory && obj.trajectoryPoints.size() > 2) {
                    trajectoryVertices.push_back(obj.trajectoryPoints[0]);
                }

                glBindVertexArray(trajectoryVAO);
                glBindBuffer(GL_ARRAY_BUFFER, trajectoryVBO);
                glBufferData(GL_ARRAY_BUFFER, trajectoryVertices.size() * sizeof(glm::vec3), trajectoryVertices.data(), GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
                glEnableVertexAttribArray(0);
                glDrawArrays(GL_LINE_STRIP, 0, trajectoryVertices.size());
            }
        }

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (size_t i = 0; i < sceneObjects.size(); ++i) {
            const auto& obj = sceneObjects[i];
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj.position);
            model = glm::rotate(model, glm::radians(rotationX), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(rotationZ), glm::vec3(0, 0, 1));
            model = glm::scale(model, glm::vec3(scale));

            glm::vec3 baseColor = (i == selectedObjectIndex) ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(0.7f, 0.7f, 0.7f);
            glUniform3fv(glGetUniformLocation(shaderProgram, "overrideColor"), 1, glm::value_ptr(baseColor));

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
