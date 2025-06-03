/* Cubo Colorido com Transformações e Instâncias
 *
 * Adaptado do projeto “Hello Triangle” de Rossana Baptista Queiroz
 * para desenhar cubos coloridos, controlar translação e escala via teclado,
 * e instanciar múltiplos cubos na cena.
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <vector>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 2000, HEIGHT = 1000;

// Código fonte do Vertex Shader (GLSL)
const GLchar* vertexShaderSource = R"glsl(
    #version 450
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec3 color;
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    out vec4 finalColor;
    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0);
        finalColor = vec4(color, 1.0);
    }
)glsl";

// Código fonte do Fragment Shader (GLSL)
const GLchar* fragmentShaderSource = R"glsl(
    #version 450
    in vec4 finalColor;
    out vec4 color;
    void main()
    {
        color = finalColor;
    }
)glsl";

// Flags de rotação
bool rotateX = false, rotateY = false, rotateZ = false;
// Vetor de translação global
glm::vec3 translation = glm::vec3(0.0f);
// Escala uniforme global
float scaleFactor = 1.0f;

// Posições estáticas dos cubos (para instanciá-los na cena) 
//Onde o cubo está na cena
vector<glm::vec3> cubePositions = {
    glm::vec3(0.0f,  0.0f, 1.0f),
};

int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Configurações de contexto OpenGL (versão 4.6 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Cubo Colorido Instanciado", nullptr, nullptr);
    if (!window) {
        std::cout << "Falha ao criar janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Callback de teclado
    glfwSetKeyCallback(window, key_callback);

    // Inicialização do GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    // Exibe informações de GPU e versão OpenGL
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported: " << version << endl;

    // Ajuste da viewport
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Compila e linka shaders
    GLuint shaderProgram = setupShader();

    // Cria a geometria do cubo (VBO + VAO)
    GLuint cubeVAO = setupGeometry();

    // Configurações de profundidade e modo de desenho
    glEnable(GL_DEPTH_TEST);

    // Configuração das matrizes de view e projection (fixas para esta cena)
    glm::mat4 view       = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.3f, -5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)WIDTH / (float)HEIGHT,
                                            0.1f, 100.0f);

    // Recupera localizações de uniformes
    GLint modelLoc      = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc       = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc       = glGetUniformLocation(shaderProgram, "projection");

    // Passa view e projection uma única vez (não mudam em tempo de execução)
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Loop principal
    while (!glfwWindowShouldClose(window)) {
        // Processa eventos de input
        glfwPollEvents();

        // Limpa buffers de cor e profundidade
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calcula ângulo de rotação baseado no tempo
        float angle = static_cast<float>(glfwGetTime());

        // Para cada posição de cubo, desenha uma instância
        for (size_t i = 0; i < cubePositions.size(); i++) {
            // Matriz de modelo: identidade
            glm::mat4 model = glm::mat4(1.0f);

            // 1) Aplica translação individual + offset global
            model = glm::translate(model, cubePositions[i] + translation);

            // 2) Aplica rotação condicional
            if (rotateX) {
                model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
            }
            else if (rotateY) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else if (rotateZ) {
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
            }

            // 3) Aplica escala uniforme
            model = glm::scale(model, glm::vec3(scaleFactor));

            // Envia matriz de modelo ao shader
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Desenha o cubo (36 vértices → 12 triângulos)
            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 64);
            glBindVertexArray(0);
        }

        // Troca buffers
        glfwSwapBuffers(window);
    }

    // Limpa recursos
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}

// Callback de teclado para controles
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Pressionar ESC fecha a janela
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Rotação nos eixos X, Y, Z
    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        rotateX = true;  rotateY = false; rotateZ = false;
    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        rotateX = false; rotateY = true;  rotateZ = false;
    }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
        rotateX = false; rotateY = false; rotateZ = true;
    }

    // Translação: WASD (X e Z), I (Y+), J (Y-)
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        const float delta = 0.05f;
        switch (key) {
            case GLFW_KEY_W: translation.z -= delta; break;
            case GLFW_KEY_S: translation.z += delta; break;
            case GLFW_KEY_A: translation.x -= delta; break;
            case GLFW_KEY_D: translation.x += delta; break;
            case GLFW_KEY_I: translation.y += delta; break;
            case GLFW_KEY_J: translation.y -= delta; break;
            // Escala uniforme: [ para diminuir, ] para aumentar
            case GLFW_KEY_LEFT_BRACKET:  scaleFactor = std::max(0.1f, scaleFactor - 0.05f); break;
            case GLFW_KEY_RIGHT_BRACKET: scaleFactor += 0.05f; break;
            default: break;
        }
    }
}

// Compila e linka shaders; retorna ID do programa
int setupShader()
{
    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Verifica erros
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Verifica erros
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Programa de Shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Verifica erros de linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER_PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Limpa shaders individuais
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Cria VBO + VAO com a geometria de um cubo colorido (36 vértices)
int setupGeometry()
{
    // Cada face do cubo (2 triângulos, 6 vértices) com uma cor sólida diferente.
    // Ordem dos vértices: x, y, z,    r, g, b
    GLfloat vertices[] = {
        // Face frontal (z = +0.5) – cor vermelha
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        // Face traseira (z = -0.5) – cor verde
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

        // Face esquerda (x = -0.5) – cor azul
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,

        // Face direita (x = +0.5) – cor amarela
          0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,

        // Face de cima (y = +0.5) – cor ciano
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
          0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,

        // Face de baixo (y = -0.5) – cor magenta
         -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,


    };
         






    GLuint VBO, VAO;
    // Gera e vincula VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Envia dados para o buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Gera e vincula VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Atributo posição (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Atributo cor (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Desvincula VAO e VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}
