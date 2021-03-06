#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.hpp"
#include "game_object.hpp"

struct Particle
{
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    GLfloat Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};

class ParticleGenerator
{
  public:
    ParticleGenerator(Shader shader, GLuint amount);
    ~ParticleGenerator();

    void Update(GLfloat deltaTime, GameObject &object, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
    void Draw();

  private:
    std::vector<Particle> particles;
    GLuint amount;
    
    Shader shader;
    GLuint quadVAO;

    void initRenderData();
    GLuint firstUnusedParticle();
    void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};

#endif