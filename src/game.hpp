#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum GameState
{
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

const glm::vec2 PADDLE_SIZE(20, 100);
const GLfloat PADDLE_VELOCITY(500.0f);
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const GLfloat BALL_RADIUS = 10.0f;

class Game
{
  public:
    GameState State;
    GLboolean Keys[1024];
    GLuint Width, Height;
    
    Game(GLuint width, GLuint height);
    ~Game();
    
    void Init();
    void ProcessInput(GLfloat deltaTime);
    void Update(GLfloat deltaTime);
    void Render();

    void Reset();
};

#endif