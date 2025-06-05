// main.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



// Função para lidar com resize da janela
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Função para processar input básico
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Shaders (vertex e fragment) com suporte a textura
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
}
)glsl";

const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);
}
)glsl";

int main()
{
    // Inicializa GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "Cubos Texturizados", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Inicializa GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Define dados do cubo (posição, cor, textura)
    float vertices[] = {
        // positions          // colors           // texture coords
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  0.3f, 0.3f, 0.3f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.8f, 0.1f, 0.3f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.9f, 0.9f, 0.1f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.1f, 0.9f, 0.9f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.3f, 0.3f, 0.3f,   1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  0.1f, 0.4f, 0.7f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.9f, 0.2f, 0.2f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.3f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.3f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.7f, 0.3f, 0.7f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.1f, 0.4f, 0.7f,   1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.9f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.6f, 0.1f, 0.8f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.1f, 0.8f, 0.5f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.1f, 0.8f, 0.5f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.5f, 0.5f, 0.2f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.3f, 0.7f, 0.9f,   0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.9f, 0.6f, 0.2f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.3f, 0.8f, 0.7f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.6f, 0.3f, 0.1f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.6f, 0.3f, 0.1f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.9f, 0.9f, 0.9f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.9f, 0.6f, 0.2f,   0.0f, 1.0f
    };

    // Configura VAO, VBO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Pos (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Cor (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Textura (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Compilar vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Verifica erros de compilação
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "Erro vertex shader:\n" << infoLog << std::endl;
    }

    // Compilar fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "Erro fragment shader:\n" << infoLog << std::endl;
    }

    // Linkar shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Erro link shader program:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Carregar textura
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Configura parâmetros da textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carrega imagem usando stb_image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // inverte verticalmente para combinar com coords OpenGL
    unsigned char *data = stbi_load("C:/Users/mateu/CGCCHibrido/assets/tex/pixelWall.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
            format = GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Falha ao carregar textura" << std::endl;
    }
    stbi_image_free(data);

    // Configura OpenGL para usar profundidade
    glEnable(GL_DEPTH_TEST);

    // Variáveis para transformações
    glm::mat4 model, view, projection;

    // Biblioteca glm é comum para matrizes, se não tiver, pode fazer manualmente ou usar outra

    // Inclua:


    // Inicializar view e projection aqui
    view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    // Render loop
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Limpa tela
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Usa shader
        glUseProgram(shaderProgram);

        // Passa view e projection para shader
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        int projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Ativa textura
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);

        glBindVertexArray(VAO);

        // Desenha vários cubos, cada um com posição e rotação diferente
        for(unsigned int i = 0; i < 10; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i % 3 * 1.5f, (i / 3) * 1.5f, 0.0f));
            float angle = 20.0f * i;
            model = glm::rotate(model, (float)glfwGetTime() + glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            int modelLoc = glGetUniformLocation(shaderProgram, "model");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Swap buffers e eventos
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Limpar
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
