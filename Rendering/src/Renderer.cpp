#include "Renderer.h"

#include "GraphicsIncludes.h"
#include "ResourceManager.h"
#include "SpriteRenderer.h"

#include <vector>
#include <filesystem>
#define _USE_MATH_DEFINES
#include "Math.h"

using namespace GUI;

std::string GetPath() {
    std::filesystem::path currPath = std::filesystem::current_path();
    while(currPath.filename() != "ChessEngine") {
        currPath = currPath.parent_path();
        if(currPath == currPath.root_path()) {
            throw std::runtime_error("Could not find root path");
        }
    }

    return currPath.string() + "\\Rendering\\";
}

void DrawPiece(GLFWwindow* window, float xPos, float yPos, float len, const Piece& piece) {
    std::string shaderPath = GetPath() + "shaders\\Sprite";
    ResourceManager::LoadShader((shaderPath + ".vs").c_str(), (shaderPath + ".frag").c_str(), nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, 640.0f, 
        640.0f, 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    SpriteRenderer *Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    // load textures
    std::string texturePath = GetPath() + "textures\\";
    std::string textureName = piece.getPath();
    ResourceManager::LoadTexture((texturePath + textureName).c_str(), true, textureName);

    Renderer->DrawSprite(ResourceManager::GetTexture(textureName), glm::vec2(xPos, yPos), glm::vec2(len, len), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
}

void DrawSquare(bool light, float xPos, float yPos, float len) {
    std::string shaderPath = GetPath() + "shaders\\";
    std::string shaderName = light ? "LightSquare" : "DarkSquare";
    Shader shader = ResourceManager::LoadShader((shaderPath + shaderName + ".vs").c_str(), (shaderPath + shaderName + ".frag").c_str(), nullptr, shaderName);
    glUseProgram(shader.ID);

    float vertices[] = {
        xPos+len, yPos, 0.0f,  // top right
        xPos+len, yPos-len, 0.0f,  // bottom right
        xPos, yPos-len, 0.0f,  // bottom left
        xPos,  yPos, 0.0f   // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void DrawCircle(float xPos, float yPos, float radius, int resolution) {
    std::vector<float> vertices;
    std::vector<int> indices;
    vertices.reserve((resolution+1) * 3);
    indices.reserve((resolution+1) * 3);
    vertices.push_back(xPos), vertices.push_back(yPos), vertices.push_back(0.0f);

    GLfloat twicePi = 2.0f * float(M_PI);
    for(int i = 1; i <= resolution+1; i++) { 
        vertices.push_back(xPos + (radius * cos(i *  twicePi / resolution))); 
        vertices.push_back(yPos + (radius * sin(i * twicePi / resolution)));
        vertices.push_back(0.0f);

        indices.push_back(0), indices.push_back(i), indices.push_back(i+1);
    }

    std::string shaderPath = GetPath() + "shaders\\";
    std::string shaderName = "Highlight";
    Shader shader = ResourceManager::LoadShader((shaderPath + shaderName + ".vs").c_str(), (shaderPath + shaderName + ".frag").c_str(), nullptr, shaderName);
    glUseProgram(shader.ID);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawElements(GL_TRIANGLES, resolution * 3, GL_UNSIGNED_INT, 0);
}

void RenderBoard(GLFWwindow* window, const GameState* state) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    const int squares_per_side = 8;
    const float start_x = GUI::GameDimensions.xPos, start_y = GUI::GameDimensions.yPos, len = GUI::GameDimensions.len, squareLen = len / squares_per_side;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    for(int row = 0; row < squares_per_side; ++row) { //draw squares
        bool evenRow = row % 2 == 0;
        float currY = start_y - row * squareLen;

        for(int col = 0; col < squares_per_side; ++col) {
            bool evenCol = col % 2 == 0;
            bool light = evenRow ? evenCol : !evenCol;
            float currX = start_x + col * squareLen;

            DrawSquare(light, currX, currY, squareLen);
        }
    }

    std::vector<Piece> pieceList = state->getPieceList();
    for(const auto& piece : pieceList) { //draw pieces
        float xPos = (start_x + piece.getCol() * squareLen + 1.0f) * (width/2); 
        float yPos = (1.0f - start_y + (7 - piece.getRow()) * squareLen) * (height/2);
        DrawPiece(window, xPos, yPos, (squareLen * width) / 2, piece);
    }

    for(const auto& move : state->getLegalMoves()) { //draw highlighted squares
        uint16_t from = move.getFrom(), to = move.getTo();
        if(from == state->getSelected()) {
            uint16_t row = GameState::getRow(to), col = GameState::getCol(to);
            float xPos = start_x + col * squareLen, yPos = start_y - row * squareLen, radius = squareLen / 5;
            DrawCircle(xPos + squareLen/2, yPos + squareLen/2, radius, CIRCLE_RES);
        }
    }
}

void Renderer::RenderFrame(Window& window) {
    GLFWwindow* glfw_window = window.getWindow();

    RenderBoard(glfw_window, m_state);

    glfwSwapBuffers(glfw_window);
    glfwPollEvents();
}