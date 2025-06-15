#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();
GLuint loadTexture(string filePath);
GLuint loadSuzanneModel(const string& objPath, int &nVertices);

void drawModel(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color = vec3(1.0, 0.0, 0.0), vec3 axis = vec3(0.0, 0.0, 1.0));

const GLuint WIDTH = 800, HEIGHT = 800;

const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 vColor;

void main()
{
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(model))) * normal;  
    TexCoord = texCoord;
    vColor = color;
    gl_Position = projection * view * model * vec4(position, 1.0);
})";

const GLchar *fragmentShaderSource = R"(
#version 400
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 vColor;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float shininess;

out vec4 FragColor;

void main()
{
    vec3 ambient = ka * vec3(1.0, 1.0, 1.0);
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = kd * diff * vec3(1.0, 1.0, 1.0);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = ks * spec * vec3(1.0, 1.0, 1.0);
    vec4 texColor = texture(texture_diffuse1, TexCoord);
    vec3 result = (ambient + diffuse + specular) * vColor * texColor.rgb;
    FragColor = vec4(result, 1.0);
})";

int main()
{
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "M4 Tarefa", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();
    int nVertices;
    GLuint VAO = loadSuzanneModel("../assets/Modelos3D/Suzanne.obj", nVertices);
    GLuint textureID = loadTexture("../assets/Modelos3D/Suzanne.png");

    float ka = 0.1f;
    float kd = 0.7f;
    float ks = 0.5f;
    float shininess = 32.0f;
    vec3 lightPos = vec3(2.0f, 2.0f, 2.0f);
    vec3 viewPos = vec3(0.0f, 0.0f, 3.0f);

    glUseProgram(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, "texture_diffuse1"), 0);
    glUniform3f(glGetUniformLocation(shaderID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shaderID, "viewPos"), viewPos.x, viewPos.y, viewPos.z);
    glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
    glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
    glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
    glUniform1f(glGetUniformLocation(shaderID, "shininess"), shininess);

    mat4 projection = perspective(radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));
    mat4 view = lookAt(viewPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, value_ptr(view));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static float angle = 0.0f;
        angle += 0.5f;
        drawModel(shaderID, VAO, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 1.0f, 1.0f), angle, nVertices, vec3(1.0f, 1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f));

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint loadTexture(string filePath)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filePath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoord;
};

GLuint loadSuzanneModel(const string& objPath, int &nVertices) {
    vector<vec3> temp_positions;
    vector<vec3> temp_normals;
    vector<vec2> temp_texcoords;
    vector<Vertex> vertices;

    ifstream file(objPath);
    if (!file.is_open()) {
        cerr << "Failed to open OBJ file: " << objPath << endl;
        return 0;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string type;
        iss >> type;

        if (type == "v") {
            vec3 position;
            iss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (type == "vn") {
            vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "vt") {
            vec2 texcoord;
            iss >> texcoord.x >> texcoord.y;
            texcoord.y = 1.0f - texcoord.y;
            temp_texcoords.push_back(texcoord);
        }
        else if (type == "f") {
            string vertexData;
            while (iss >> vertexData) {
                Vertex vertex;
                istringstream viss(vertexData);
                string indexStr;
                unsigned int indices[3] = {0, 0, 0};
                int i = 0;

                while (getline(viss, indexStr, '/')) {
                    if (!indexStr.empty()) {
                        indices[i] = stoul(indexStr) - 1;
                    }
                    i++;
                    if (i >= 3) break;
                }

                if (indices[0] < temp_positions.size()) {
                    vertex.position = temp_positions[indices[0]];
                }
                if (indices[1] < temp_texcoords.size()) {
                    vertex.texCoord = temp_texcoords[indices[1]];
                }
                if (indices[2] < temp_normals.size()) {
                    vertex.normal = temp_normals[indices[2]];
                }
                vertices.push_back(vertex);
            }
        }
    }

    file.close();
    nVertices = vertices.size();

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    return VAO;
}

void drawModel(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color, vec3 axis)
{
    mat4 model = mat4(1.0f);
    model = translate(model, position);
    model = rotate(model, radians(angle), axis);
    model = scale(model, dimensions);
    
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
    glUniform3f(glGetUniformLocation(shaderID, "vColor"), color.r, color.g, color.b);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glBindVertexArray(0);
}
