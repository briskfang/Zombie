
#include <SFML/Graphics.hpp>
#include <sstream> 

#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Zombie.h"
#include "Pickup.h"
#include "Bullet.h"



// FloatRect:  left, top, width, height (LTWH)
// Vector2f:   a vector with 2 floats
// Vector2i:   a vector with 2 intergers


using namespace sf;


int main()
{
    TextureHolder holder;

    enum class State{ PAUSED, LEVELING_UP, GAME_OVER, PLAYING };
    State state = State::GAME_OVER;

    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;

    RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena"); // Style::Fullscreen);
    View mainView(FloatRect(0, 0, resolution.x, resolution.y));
    Clock clock;
    Time  gameTimeTotal;
    Vector2f mouseWorldPosition;
    Vector2i mouseScreenPosition;

    Player player;
    IntRect arena;

    VertexArray background;
    Texture textureBackground = TextureHolder::getTexture("graphics/background_sheet.png");

    int numZombies;
    int numZombiesAlive;
    Zombie* zombies = NULL;


    Bullet bullets[100];
    int currentBullet = 0;
    int bulletsSpare = 24;
    int bulletsInClip = 6;
    int clipSize = 6;
    float fireRate = 1;  
    Time lastPressed; // fire button last pressed

    window.setMouseCursorVisible(true);
    Sprite spriteCrosshair;
    Texture textureCrosshair = TextureHolder::getTexture("graphics/crosshair.png");
    spriteCrosshair.setTexture(textureCrosshair);
    spriteCrosshair.setOrigin(25, 25);

    Pickup healthPickup(1); 
    Pickup ammoPickup(2);

    int score = 0;
    int hiScore = 0;

    Sprite spriteGameOver;
    Texture textureGameOver = TextureHolder::getTexture("graphics/background.jpg");
    spriteGameOver.setTexture(textureGameOver);
    spriteGameOver.setPosition(0, 0);

    View hudView(FloatRect(0, 0, resolution.x, resolution.y));

    Sprite spriteAmmoIcon;
    Texture textureAmmoIcon = TextureHolder::getTexture("graphics/ammo_incon.jpg");
    spriteAmmoIcon.setTexture(textureAmmoIcon);
    spriteAmmoIcon.setTexture(textureAmmoIcon);
    spriteAmmoIcon.setPosition(20, 800);

    Font font;
    font.loadFromFile("fonts/KOMIKAP_.ttf");

    // Paused
    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(50);
    pausedText.setFillColor(Color::White);
    pausedText.setPosition(400, 400);
    pausedText.setString("Press Enter \n to continue");

    // GameOver
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(250, 800);
    gameOverText.setString("Press Enter to play");


    // Level up
    Text levelUpText;
    levelUpText.setFont(font);
    levelUpText.setCharacterSize(40);
    levelUpText.setFillColor(Color::White);
    levelUpText.setPosition(100, 250);
    std::stringstream levelUpStream;
    levelUpStream <<
            "1 - Increased rate of fire\n" <<
            "2 - Increased clip size(next reload)\n" <<
            "3 - Increased max health\n" <<
            "4 - Increased run speed\n"  <<
            "5 - More and better health pickups\n" <<
            "6 - More and better ammo pickups";
    levelUpText.setString(levelUpStream.str());

    // Ammo
    Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(30);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(200, 800);
    
    // Score
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(20, 0);

    // Hi Score
    Text hiScoreText;
    hiScoreText.setFont(font);
    hiScoreText.setCharacterSize(30);
    hiScoreText.setFillColor(Color::White);
    hiScoreText.setPosition(1200, 0);
    std::stringstream s;
    s << "Hi Score: " << hiScore;
    hiScoreText.setString(s.str());

    // Zombie remaining
    Text zombiesRemainingText;
    zombiesRemainingText.setFont(font);
    zombiesRemainingText.setCharacterSize(30);
    zombiesRemainingText.setFillColor(Color::White);
    zombiesRemainingText.setPosition(1200, 800);
    zombiesRemainingText.setString("Zombies: 100");

    // Wave number
    int wave = 0;
    Text waveNumberText;
    waveNumberText.setFont(font);
    waveNumberText.setCharacterSize(30);
    waveNumberText.setFillColor(Color::White);
    waveNumberText.setPosition(1000, 700);
    waveNumberText.setString("Wave: 0");

    // Health bar
    RectangleShape healthBar;
    healthBar.setFillColor(Color::Red);
    healthBar.setPosition(450, 800);

    // when did we last update the HUD
    int framesSinceLastHUDUpdate = 0;

    // how often to update the HUD
    int fpsMeasurementFrameInterval = 1000;


    while(window.isOpen())
    {
        Event event;
        while(window.pollEvent(event))
        {
            if(event.type == Event::KeyPressed)
            {
                if(event.key.code == Keyboard::Return && state == State::PLAYING )
                {
                    state = State::PAUSED;
                }
                else if(event.key.code == Keyboard::Return && state == State::PAUSED)
                {
                    state = State::PLAYING;
                    clock.restart();
                }
                else if(event.key.code == Keyboard::Return && state == State::GAME_OVER)
                {
                    state = State::LEVELING_UP;
                }
                if(state == State::PLAYING)
                {
                    //load the gun
                    if(event.key.code == Keyboard::R)
                    {
                        if(bulletsSpare >= clipSize)
                        {
                            bulletsInClip = clipSize;
                            bulletsSpare -= clipSize;
                        }
                        else if(bulletsSpare > 0)
                        {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0; 
                        }
                        else
                        {

                        }
                    }

                }

            }
        } // end event polling

        if(Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }

        if(state == State::PLAYING)
        {
            if(Keyboard::isKeyPressed(Keyboard::W))
            {
                player.moveUp();
            }
            else
            {
                player.stopUp();
            }

            if(Keyboard::isKeyPressed(Keyboard::S))
            {
                player.moveDown();
            }
            else
            {
                player.stopDown();
            }

            if(Keyboard::isKeyPressed(Keyboard::A))
            {
                player.moveLeft();
            }
            else
            {
                player.stopLeft();
            }

            if(Keyboard::isKeyPressed(Keyboard::D))
            {
                player.moveRight();
            }
            else
            {
                player.stopRight();
            }

            // fire a bullet: left mouse click
            if(Mouse::isButtonPressed(Mouse::Left))
            {
                if(gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate 
                   && bulletsInClip > 0) //?
                {
                    bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y,
                                                 mouseWorldPosition.x, mouseWorldPosition.y);
                    currentBullet++;
                    if(currentBullet > 99)
                    {
                        currentBullet = 0;
                    }
                    lastPressed = gameTimeTotal;
                    bulletsInClip--;
                }
            }


        } // end WSAD


        if(state == State::LEVELING_UP)
        {
            if(event.key.code == Keyboard::Num1)
            {
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num2)
            {
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num3)
            {
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num4)
            {
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num5)
            {
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num6)
            {
                state = State::PLAYING;
            }
            
            if(state == State::PLAYING)
            {
                arena.width  = 500;
                arena.height = 500;
                arena.left   = 0;
                arena.top    = 0;
                int tileSize = createBackground(background, arena);

                player.spawn(arena, resolution, tileSize);  // arena is smaller than resolution

                healthPickup.setArena(arena);
                ammoPickup.setArena(arena);

                numZombies = 10;
                delete[] zombies;
                zombies = createHorde(numZombies, arena);
                numZombiesAlive = numZombies;

                clock.restart();
            }
        } // end Levelling up

        if(state == State::PLAYING)
        {
            Time dt = clock.restart();
            gameTimeTotal += dt;
            float dtAsSeconds = dt.asSeconds();
            mouseScreenPosition = Mouse::getPosition();
            mouseWorldPosition  = window.mapPixelToCoords(Mouse::getPosition(), mainView);

            spriteCrosshair.setPosition(mouseWorldPosition);

            player.update(dtAsSeconds, Mouse::getPosition());
            Vector2f playerPosition(player.getCenter());

            mainView.setCenter(player.getCenter());

            for(int i = 0; i < numZombies; i++)
            {
                if(zombies[i].isAlive())
                {
                    zombies[i].update(dt.asSeconds(), playerPosition);
                }
            }

            //update all the bullets that are in-flight
            for(int i = 0; i < 100; i++)
            {
                if(bullets[i].isInFlight()) //set it to true when shoot
                {
                    bullets[i].update(dtAsSeconds);
                }
            }

            // update the pickups
            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);

            // collision detection
            for(int i = 0; i < 100; i++ )
            {
                for(int j = 0; j < numZombies; j++)
                {
                    if(bullets[i].isInFlight() && zombies[j].isAlive())
                    {
                        if(bullets[i].getPosition().intersects(zombies[j].getPosition()))
                        {
                            bullets[i].stop();
                            if(zombies[j].hit())
                            {
                                score += 10;
                                if(score >= hiScore)
                                {
                                    hiScore = score;
                                }
                                numZombiesAlive--;
                                // kill all the Zombies, upgrade the level
                                if(numZombiesAlive == 0)
                                {
                                    state = State::LEVELING_UP;
                                }
                            }                                     
                        }
                    }
                }
            } // end colision detection

            // have zombie touches the player?
            for(int i = 0; i < numZombies; i++)
            {
                if(player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
                {
                    if(player.hit(gameTimeTotal))
                    {
                        // more to come
                    }
                    if(player.getHealth() <= 0)
                    {
                        state = State::GAME_OVER;
                    }
                }
            } // end zombie touches the player

            // has the player touched health pickup?
            if(player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
            {
                player.increaseHealthLevel(healthPickup.gotIt());
            }

            // has the player touched ammo pickup?
            if(player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
            {
                bulletsSpare += ammoPickup.gotIt();
            }

            healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

            framesSinceLastHUDUpdate++;
            if(framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
            {
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssHiScore;
                std::stringstream ssWave;
                std::stringstream ssZombieAlive;

                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());

                ssScore << "Score: " << score;
                scoreText.setString(ssScore.str());

                ssHiScore << "Hi Score: " << hiScore;
                hiScoreText.setString(ssHiScore.str());

                ssWave << "Wave: " << wave;
                waveNumberText.setString(ssWave.str());

                ssZombieAlive << "Zombies: " << numZombiesAlive;
                zombiesRemainingText.setString(ssZombieAlive.str());

                framesSinceLastHUDUpdate = 0;
            }


        } // end Playing

        if(state == State::PLAYING)
        {
            window.clear();
            window.setView(mainView);
            window.draw(background, &textureBackground);

            for(int i = 0; i < numZombies; i++)
            {
                window.draw(zombies[i].getSprite());
            }

            for(int i = 0; i < 100; i++)
            {
                if(bullets[i].isInFlight())
                {
                    window.draw(bullets[i].getShape());
                }
            }

            window.draw(player.getSprite());

            if(ammoPickup.isSpawned())
            {
                window.draw(ammoPickup.getSprite());
            }
            if(healthPickup.isSpawned())
            {
                window.draw(healthPickup.getSprite());
            }

            window.draw(spriteCrosshair);

            window.setView(hudView);
            window.draw(spriteAmmoIcon);
            window.draw(ammoText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            window.draw(healthBar);
            window.draw(waveNumberText);
            window.draw(zombiesRemainingText);


        }

        if(state == State::LEVELING_UP)
        {
            window.draw(spriteGameOver);
            window.draw(levelUpText);

        }
        if(state == State::PAUSED)
        {
            window.draw(pausedText);
        }
        if(state == State::GAME_OVER)
        {
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            
        }
        window.display();

    } //end game loop

    return 0; 
}