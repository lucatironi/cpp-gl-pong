#include "ball_object.hpp"

BallObject::BallObject()
    : GameObject(), Radius(10.0f) {}

BallObject::BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity)
    : GameObject(pos, glm::vec2(radius * 2, radius * 2), glm::vec3(1.0f), velocity), Radius(radius) {}

glm::vec2 BallObject::Move(GLfloat deltaTime, GLuint windowHeight)
{
    this->Position += this->Velocity * deltaTime;
    if (this->Position.y <= 0.0f)
    {
        this->Velocity.y = -this->Velocity.y;
        this->Position.y = 0.0f;
    }
    else if (this->Position.y + this->Size.y >= windowHeight)
    {
        this->Velocity.y = -this->Velocity.y;
        this->Position.y = windowHeight - this->Size.x;
    }

    return this->Position;
}

void BallObject::Reset(glm::vec2 position, glm::vec2 velocity)
{
    this->Position = position;
    this->Velocity = velocity;
}