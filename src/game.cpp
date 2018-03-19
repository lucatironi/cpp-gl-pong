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
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
    Collision collisionPaddle1 = CheckCollision(*Ball, *Paddle1);
    Collision collisionPaddle2 = CheckCollision(*Ball, *Paddle2);

    GLfloat strength = 2.0f;
    if (std::get<0>(collisionPaddle1))
    {
        GLfloat centerBoard = Paddle1->Position.y + Paddle1->Size.y / 2;
        GLfloat distance = (Ball->Position.y + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Paddle1->Size.y / 2);

        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Velocity.x = -Ball->Velocity.x;
    }
    if (std::get<0>(collisionPaddle2))
    {
        GLfloat centerBoard = Paddle2->Position.y + Paddle2->Size.y / 2;
        GLfloat distance = (Ball->Position.y + Ball->Radius) - centerBoard;
        GLfloat percentage = distance / (Paddle2->Size.y / 2);

        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.y = INITIAL_BALL_VELOCITY.y * percentage * strength;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
        Ball->Velocity.x = -Ball->Velocity.x;
    }
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // Get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // Calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2, two.Size.y / 2);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // Get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // Now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // Now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(GL_TRUE, VectorDirection(difference), difference);
    else
        return std::make_tuple(GL_FALSE, UP, glm::vec2(0, 0));
}

// Calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };
    GLfloat max = 0.0f;
    GLuint best_match = -1;
    for (GLuint i = 0; i < 4; i++)
    {
        GLfloat dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}