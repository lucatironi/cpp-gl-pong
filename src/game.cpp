#include "game.hpp"
#include "resource_manager.hpp"
#include "sprite_renderer.hpp"

SpriteRenderer *Renderer;

Game::Game(GLuint width, GLuint height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Renderer;
}

void Game::Init()
{
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.fs", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // Set render-specific controls
    Shader myShader;
    myShader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(myShader);
}

void Game::Update(GLfloat dt)
{
}

void Game::ProcessInput(GLfloat dt)
{
}

void Game::Render()
{
}