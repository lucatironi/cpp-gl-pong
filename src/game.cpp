#include "game.hpp"
#include "resource_manager.hpp"
#include "sprite_renderer.hpp"
#include "game_object.hpp"
#include "ball_object.hpp"

SpriteRenderer *Renderer;
GameObject *Paddle1, *Paddle2;
BallObject *Ball;

Game::Game(GLuint width, GLuint height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Renderer;
    delete Paddle1;
    delete Paddle2;
    delete Ball;
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

    glm::vec2 paddle1Position = glm::vec2(
        10.0f,
        this->Height / 2 - PADDLE_SIZE.y / 2);
    Paddle1 = new GameObject(paddle1Position, PADDLE_SIZE);

    glm::vec2 paddle2Position = glm::vec2(
        this->Width - PADDLE_SIZE.x - 10.0f,
        this->Height / 2 - PADDLE_SIZE.y / 2);
    Paddle2 = new GameObject(paddle2Position, PADDLE_SIZE);

    glm::vec2 ballPosition = glm::vec2(this->Width / 2, this->Height / 2);
    Ball = new BallObject(ballPosition, BALL_RADIUS, INITIAL_BALL_VELOCITY);
}

void Game::Update(GLfloat deltaTime)
{
}

void Game::ProcessInput(GLfloat deltaTime)
{
    if (this->State == GAME_ACTIVE)
    {
        GLfloat deltaSpace = PADDLE_VELOCITY * deltaTime;
        // Move paddle one
        if (this->Keys[GLFW_KEY_W])
        {
            if (Paddle1->Position.y >= 0)
                Paddle1->Position.y -= deltaSpace;
        }
        if (this->Keys[GLFW_KEY_S])
        {
            if (Paddle1->Position.y <= this->Height - Paddle1->Size.y)
                Paddle1->Position.y += deltaSpace;
        }
        // Move paddle two
        if (this->Keys[GLFW_KEY_UP])
        {
            if (Paddle2->Position.y >= 0)
                Paddle2->Position.y -= deltaSpace;
        }
        if (this->Keys[GLFW_KEY_DOWN])
        {
            if (Paddle2->Position.y <= this->Height - Paddle2->Size.y)
                Paddle2->Position.y += deltaSpace;
        }
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE)
    {
        Paddle1->Draw(*Renderer);
        Paddle2->Draw(*Renderer);
        Ball->Draw(*Renderer);
    }
}
}