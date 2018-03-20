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
}

void Game::Init()
{
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.fs", nullptr, "sprite");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->Width), static_cast<GLfloat>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // Configure game objects
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
    if (this->State == GAME_ACTIVE)
    {
        // Update objects
        Ball->Move(deltaTime, this->Height);
        // Check for collisions
        this->DoCollisions();
        // Check loss condition
        if (Ball->Position.x <= 0.0f)
        {
            Ball->Reset(glm::vec2(this->Width / 2, this->Height / 2), INITIAL_BALL_VELOCITY);
        } else if (Ball->Position.x + Ball->Size.x >= this->Width) {
            Ball->Reset(glm::vec2(this->Width / 2, this->Height / 2), INITIAL_BALL_VELOCITY);
        }
    }
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

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);

void Game::DoCollisions()
{
    GLfloat strength = 2.0f;
    glm::vec2 oldVelocity = Ball->Velocity;
    if (CheckCollision(*Ball, *Paddle1))
    {
        GLfloat centerBoard = Paddle1->Position.y + Paddle1->Size.y / 2;
        GLfloat distance = (Ball->Position.y + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Paddle1->Size.y / 2);

        Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Velocity.x = -Ball->Velocity.x;
        Ball->Position.x = Paddle1->Position.x + Paddle1->Size.x;
    }
    if (CheckCollision(*Ball, *Paddle2))
    {
        GLfloat centerBoard = Paddle2->Position.y + Paddle2->Size.y / 2;
        GLfloat distance = (Ball->Position.y + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Paddle2->Size.y / 2);

        Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Velocity.x = -Ball->Velocity.x;
        Ball->Position.x = Paddle2->Position.x - Ball->Size.x;
    }
}

GLboolean CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // Collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    // Collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    // Collision only if on both axes
    return collisionX && collisionY;
}