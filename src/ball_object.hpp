#ifndef BALLOBJECT_H
#define BALLOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "sprite_renderer.hpp"
#include "game_object.hpp"

class BallObject : public GameObject
{
  public:
    GLfloat Radius;
    
    BallObject();
    BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity);

    glm::vec2 Move(GLfloat deltaTime, GLuint window_width);
    void Reset(glm::vec2 position, glm::vec2 velocity);
};

#endif