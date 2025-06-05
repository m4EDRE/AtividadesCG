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

// Posições estáticas dos cubos (para instanciá-los na cena)
vector<glm::vec3> cubePositions = {
    glm::vec3(-0.5f, -0.5f, -1.0f),
    glm::vec3(0.5f, 0.5f, 1.0f)
};

// Índice do cubo atualmente selecionado
int selectedCubeIndex = 0;

// Vetores de translação individuais por cubo
vector<glm::vec3> cubeTranslations = {
    glm::vec3(0.0f), // Cubo 0
    glm::vec3(0.0f)  // Cubo 1
};

// Escala uniforme global
float scaleFactor = 1.0f;

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

            // 1) Aplica translação individual do cubo
            model = glm::translate(model, cubePositions[i] + cubeTranslations[i]);

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
            glDrawArrays(GL_TRIANGLES, 0, 36);
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

    // Translação individual do cubo selecionado, escala e troca de cubo
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        const float delta = 0.05f;
        glm::vec3& currentTranslation = cubeTranslations[selectedCubeIndex];
        switch (key) {
            case GLFW_KEY_W: currentTranslation.z -= delta; break;
            case GLFW_KEY_S: currentTranslation.z += delta; break;
            case GLFW_KEY_A: currentTranslation.x -= delta; break;
            case GLFW_KEY_D: currentTranslation.x += delta; break;
            case GLFW_KEY_I: currentTranslation.y += delta; break;
            case GLFW_KEY_J: currentTranslation.y -= delta; break;

            // Escala uniforme global
            case GLFW_KEY_LEFT_BRACKET:  scaleFactor = std::max(0.1f, scaleFactor - 0.05f); break;
            case GLFW_KEY_RIGHT_BRACKET: scaleFactor += 0.05f; break;

            // Trocar cubo selecionado (1 ou 2)
            case GLFW_KEY_1: selectedCubeIndex = 0; break;
            case GLFW_KEY_2: selectedCubeIndex = 1; break;

            default: break;
        }
    }
}

// Cria e compila shaders, linka e retorna programa
int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Checa erros vertex shader
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "Erro compilacao vertex shader: " << infoLog << endl;
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Checa erros fragment shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "Erro compilacao fragment shader: " << infoLog << endl;
    }

    // Programa shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Checa erros linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "Erro linkagem shader: " << infoLog << endl;
    }

    // Limpa shaders (já linkados)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Setup da geometria do cubo (VBO + VAO)
int setupGeometry()
{
    // Vértices do cubo (posição e cor)
    GLfloat vertices[] = {
        // Posições          // Cores
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,

         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
         0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f
    };

    GLuint VBO, VAO;

    // Cria buffers e arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Configura VAO
    glBindVertexArray(VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Posicao
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Cor
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO e VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}
