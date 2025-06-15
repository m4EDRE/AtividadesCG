// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;
glm::vec3 Ka, Kd, Ks;
float shininess = 32.0f;

void loadOBJ(const std::string& path) {
    std::ifstream file(path);
    std::string line;
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 pos;
            iss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        } else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (prefix == "f") {
            unsigned int vi[3], ni[3];
            char slash;
            for (int i = 0; i < 3; i++) {
                iss >> vi[i] >> slash >> slash >> ni[i];
                Vertex v;
                v.position = temp_positions[vi[i] - 1];
                v.normal = temp_normals[ni[i] - 1];
                vertices.push_back(v);
                indices.push_back(indices.size());
            }
        }
    }
    std::cout << "Modelo carregado com " << vertices.size() << " vértices e " << indices.size() << " índices.\n";
}

void loadMTL(const std::string& mtlPath) {
    std::ifstream file(mtlPath);
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "Ka") iss >> Ka.r >> Ka.g >> Ka.b;
        else if (prefix == "Kd") iss >> Kd.r >> Kd.g >> Kd.b;
        else if (prefix == "Ks") iss >> Ks.r >> Ks.g >> Ks.b;
        else if (prefix == "Ns") iss >> shininess;
    }
    std::cout << "Ka: " << Ka.r << ", " << Ka.g << ", " << Ka.b << std::endl;
    std::cout << "Kd: " << Kd.r << ", " << Kd.g << ", " << Kd.b << std::endl;
    std::cout << "Ks: " << Ks.r << ", " << Ks.g << ", " << Ks.b << std::endl;
    std::cout << "Shininess: " << shininess << std::endl;
}

GLuint compileShader(const char* path, GLenum type) {
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    const char* src = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }

    return shader;
}

GLuint linkProgram(const char* vertexPath, const char* fragmentPath) {
    GLuint vertexShader = compileShader(vertexPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentPath, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return success ? program : 0;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Erro ao inicializar GLFW." << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Modelo Phong", NULL, NULL);
    if (!window) {
        std::cerr << "Erro ao criar janela GLFW. Verifique sua GPU e drivers OpenGL." << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Erro ao inicializar GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    loadOBJ("C:\\Users\\mateu\\CGCCHibrido\\assets\\Modelos3D\\SuzanneSubdiv1.obj");
    loadMTL("C:\\Users\\mateu\\CGCCHibrido\\assets\\Modelos3D\\SuzanneSubdiv1.mtl");

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    GLuint shaderProgram = linkProgram("vertex_shader.glsl", "fragment_shader.glsl");
    if (!shaderProgram) {
        std::cerr << "Erro ao compilar shaders. Encerrando." << std::endl;
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glUseProgram(shaderProgram);
    glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glUniform3fv(glGetUniformLocation(shaderProgram, "Ka"), 1, glm::value_ptr(Ka));
        glUniform3fv(glGetUniformLocation(shaderProgram, "Kd"), 1, glm::value_ptr(Kd));
        glUniform3fv(glGetUniformLocation(shaderProgram, "Ks"), 1, glm::value_ptr(Ks));
        glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), shininess);
        glUniform3fv(glGetUniformLocation(shaderProgram, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, 3.0f)));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 model = glm::mat4(1.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
