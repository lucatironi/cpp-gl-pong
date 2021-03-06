#include <sstream>

#include <irrKlang.h>
using namespace irrklang;

#include "game.hpp"
#include "resource_manager.hpp"
#include "sprite_renderer.hpp"
#include "game_object.hpp"
#include "ball_object.hpp"
#include "particle_generator.hpp"
#include "post_processor.hpp"
#include "text_renderer.hpp"

SpriteRenderer    *Renderer;
ParticleGenerator *Particles;
PostProcessor     *Effects;
GameObject        *Paddle1, *Paddle2;
BallObject        *Ball;
ISoundEngine      *SoundEngine = createIrrKlangDevice();
TextRenderer      *Text;

GLfloat ShakeTime = 0.0f;
int MaxScore = 10;
int Paddle1Score = 0;
int Paddle2Score = 0;

Game::Game(GLuint windowWidth, GLuint windowHeight, GLuint framebufferWidth, GLuint framebufferHeight)
    : State(GAME_MENU), Keys(),
      WindowWidth(windowWidth), WindowHeight(windowHeight),
      FramebufferWidth(framebufferWidth), FramebufferHeight(framebufferHeight)
{
}

Game::~Game()
{
    delete Renderer;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();
}

void Game::Init()
{
    // Load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.fs", nullptr, "sprite");
    ResourceManager::LoadShader("../src/shaders/particle.vs", "../src/shaders/particle.fs", nullptr, "particle");
    ResourceManager::LoadShader("../src/shaders/post_processing.vs", "../src/shaders/post_processing.fs", nullptr, "postprocessing");
    // Configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(this->WindowWidth), static_cast<GLfloat>(this->WindowHeight), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetMatrix4("projection", projection);
    // Set render-specific controls
    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), 500);
    Effects = new PostProcessor(ResourceManager::GetShader("postprocessing"), this->FramebufferWidth, this->FramebufferHeight);
    Text = new TextRenderer(this->WindowWidth, this->WindowHeight);
    Text->Load("../assets/PressStart2P-Regular.ttf", 32);

    // Configure game objects
    glm::vec2 paddle1Position = glm::vec2(
        10.0f,
        this->WindowHeight / 2 - PADDLE_SIZE.y / 2);
    Paddle1 = new GameObject(paddle1Position, PADDLE_SIZE);

    glm::vec2 paddle2Position = glm::vec2(
        this->WindowWidth - PADDLE_SIZE.x - 10.0f,
        this->WindowHeight / 2 - PADDLE_SIZE.y / 2);
    Paddle2 = new GameObject(paddle2Position, PADDLE_SIZE);

    glm::vec2 ballPosition = glm::vec2(this->WindowWidth / 2, this->WindowHeight / 2);
    Ball = new BallObject(ballPosition, BALL_RADIUS, INITIAL_BALL_VELOCITY);
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
            if (Paddle1->Position.y <= this->WindowHeight - Paddle1->Size.y)
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
            if (Paddle2->Position.y <= this->WindowHeight - Paddle2->Size.y)
                Paddle2->Position.y += deltaSpace;
        }
    }
    if (this->State == GAME_MENU || this->State == GAME_WIN)
    {
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->Reset();
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = GL_TRUE;
        }
    }
}

void Game::Update(GLfloat deltaTime)
{
    if (this->State == GAME_ACTIVE)
    {
        // Update objects
        Ball->Move(deltaTime, this->WindowHeight);
        // Check for collisions
        this->DoCollisions();
        // Update particles
        Particles->Update(deltaTime, *Ball, 2, glm::vec2(Ball->Radius / 2));
        // Reduce shake time
        if (ShakeTime > 0.0f)
        {
            ShakeTime -= deltaTime;
            if (ShakeTime <= 0.0f)
                Effects->Shake = false;
        }
        // Check loss condition
        if (Ball->Position.x <= 0.0f)
        {
            Paddle2Score++;
            SoundEngine->play2D("../assets/out.wav", GL_FALSE);
            Ball->Reset(glm::vec2(this->WindowWidth / 2, this->WindowHeight / 2), INITIAL_BALL_VELOCITY);
        }
        else if (Ball->Position.x + Ball->Size.x >= this->WindowWidth)
        {
            Paddle1Score++;
            SoundEngine->play2D("../assets/out.wav", GL_FALSE);
            Ball->Reset(glm::vec2(this->WindowWidth / 2, this->WindowHeight / 2), INITIAL_BALL_VELOCITY);
        }

        if (Paddle1Score >= MaxScore || Paddle2Score >= MaxScore)
            this->State = GAME_WIN;
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
    {
        Effects->BeginRender();
            Paddle1->Draw(*Renderer);
            Paddle2->Draw(*Renderer);
            Particles->Draw();
            Ball->Draw(*Renderer);
        Effects->EndRender();
        Effects->Render(glfwGetTime());

        std::stringstream ss;
        ss << Paddle1Score << ":" << Paddle2Score;
        Text->RenderText(ss.str(), this->WindowWidth / 2 - 45.0f, 5.0f, 1.0f);
    }
    if (this->State == GAME_MENU || this->State == GAME_WIN)
        Text->RenderText("Press ENTER to start", 260.0f, this->WindowHeight / 2 - 25.0f, 0.5f);
    if (this->State == GAME_WIN) {
        std::string winText;
        if (Paddle1Score > Paddle2Score)
            winText = "Player 1 Won!";
        else
            winText = "Player 2 Won!";

        Text->RenderText(winText, 270.0f, this->WindowHeight / 2 + 25.0f, 0.75f);
    }
}

void Game::Reset()
{
    Paddle1Score = 0;
    Paddle2Score = 0;
    Paddle1->Position = glm::vec2(10.0f, this->WindowHeight / 2 - PADDLE_SIZE.y / 2);
    Paddle2->Position = glm::vec2(this->WindowWidth - PADDLE_SIZE.x - 10.0f, this->WindowHeight / 2 - PADDLE_SIZE.y / 2);
    Ball->Reset(glm::vec2(this->WindowWidth / 2, this->WindowHeight / 2), INITIAL_BALL_VELOCITY);
}

// Collision detection
GLboolean CheckCollision(GameObject &one, GameObject &two);

void Game::DoCollisions()
{
    GLfloat strength = 2.0f;
    glm::vec2 oldVelocity = Ball->Velocity;
    if (CheckCollision(*Ball, *Paddle1))
    {
        ShakeTime = 0.05f;
        Effects->Shake = true;
        SoundEngine->play2D("../assets/bleep.wav", GL_FALSE);

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
        ShakeTime = 0.05f;
        Effects->Shake = true;
        SoundEngine->play2D("../assets/bleep.wav", GL_FALSE);

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