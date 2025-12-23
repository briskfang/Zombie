#include <cstdlib>
#include <ctime>
#include "Zombie.h"
#include "TextureHolder.h"

using namespace std;

void Zombie::spawn(float startX, float startY, int type, int seed)
{
    switch(type)
    {
        case 0:
            m_Sprite = Sprite(TextureHolder::getTexture("graphics/bloater.png"));
            m_Speed  = 40;
            m_Health = 5;
            break;
        case 1:
            m_Sprite = Sprite(TextureHolder::getTexture("graphics/chaser.png"));
            m_Speed = 70;
            m_Health = 1;
            break;
        case 2:
            m_Sprite = Sprite(TextureHolder::getTexture("graphics/crawler.png"));
            m_Speed = 20;
            m_Health = 3;
            break;
    }

    srand((int)time(0) * seed);
    float modifier = (rand() % MAX_VARRIANCE) + OFFSET; // 30, 71
    modifier /= 100; // 0.7 ~ 1
    m_Speed *= modifier;
    m_Position.x = startX;
    m_Position.y = startY;
    
    m_Sprite.setOrigin(25, 25);  // why?
    m_Sprite.setPosition(m_Position);

}


bool Zombie::hit()
{
    m_Health--;
    if(m_Health < 0)
    {
        m_Alive = false;
        m_Sprite.setTexture(TextureHolder::getTexture("graphics/blood.png"));
       return true;
    }
    return false;
}

bool Zombie::isAlive()
{
    return m_Alive;
}

FloatRect Zombie::getPosition()
{
    return m_Sprite.getGlobalBounds();
}

Sprite Zombie::getSprite()
{
    return m_Sprite;
}

void Zombie::update(float elapsedTime, Vector2f playerLocation)
{
    float playerX = playerLocation.x;
    float playerY = playerLocation.y;

    if(playerX > m_Position.x)
    {
        m_Position.x = m_Position.x + elapsedTime * m_Speed;
    }

    if(playerY > m_Position.y)
    {
        m_Position.y = m_Position.y + elapsedTime * m_Speed;
    }
    
    if(playerX < m_Position.x)
    {
        m_Position.x = m_Position.x - elapsedTime * m_Speed;
    }

    if(playerY < m_Position.y)
    {
        m_Position.y = m_Position.y - elapsedTime * m_Speed;
    }

    m_Sprite.setPosition(m_Position);

    float angle = (atan2(playerY - m_Position.y, playerX - m_Position.x) * 180 ) / 3.141;
    
    m_Sprite.setRotation(angle);

}







