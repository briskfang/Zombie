
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream> 
#include <fstream>
#include <iostream>

#include "Player.h"
#include "ZombieArena.h"
#include "TextureHolder.h"
#include "Zombie.h"
#include "Pickup.h"
#include "Bullet.h"
#include "log.h"




// FloatRect:  left, top, width, height (LTWH)
// Vector2f:   a vector with 2 floats
// Vector2i:   a vector with 2 intergers
// HUD: Heads-up display, info displayed on top of game while gameplay
// wave: a common game design pattern, found in horde or survival mode. Player is presented with progressively
//       more intense groups of enemies to defeat in successive rounds.     
// std::ofstream logFile():     overwrite pre-existing content
// std::ofstream appLogFile():  append to existing content


using namespace sf;

int main()
{

/*
================================================================================================================
Initialize 
1. log
2. window(RenderWindow), views(View), arena(IntRect)
3. mouse, crosshair
4. player, zombies, bullets
5. pickups
6. text, score
7. healthBar
8. sound
================================================================================================================
*/    

    Logger::init("log/zombie.log", LogLevel::INFO);
    //Logger::setLevel(LogLevel::DEBUG);
    LOG_INFO("Game starts");
    //LOG_DEBUG("Game starts, very first log after entering main() ");
    

    TextureHolder holder; // the class instance "holder" is never used.

    enum class State{ PAUSED, LEVELING_UP, GAME_OVER, PLAYING }; // state machine
    
    State state = State::GAME_OVER;
    LOG_INFO("Game Initial State: GAME_OVER");

    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;

    //std::cerr << "resolution: " << resolution.x << " , " << resolution.y << std::endl; // needs to include <iostream>
    
    /*  
    ------ window, view   -----
    RenderWindow| similar to physical world | screen coordinates(pixels)| actions: setView, draw..    
    View        | similar to camera         | world coordinates         | actions: setCenter, zoom, setSize

    btw, 
    <sprite>.setPosition(<world coordinates>);
    <object>.getPosition(window) returns screen coordinates
    to change screen coordinates to world coordinates, call window.mapPixelToCoords(pixelPos, view); 


    ------  mainView vs. hudView ------
    mainView: moves through the world
    hudView: UI elements stay fixed on the screen

    --------  arena vs. view -------
    arena: define the logical boundaries of the gameplay; its type is IntRect
    view: constructed via FloatRect
    */
    
    RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena"); // Style::Fullscreen);
    View mainView(FloatRect(0, 0, resolution.x, resolution.y));
    Clock clock;
    Time  gameTimeTotal; 

    Vector2f mouseWorldPosition;    // world coordinates,  used in view space
    Vector2i mouseScreenPosition;   // screen coordinates, used in window space

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
    Texture textureGameOver = TextureHolder::getTexture("graphics/background.png");
    spriteGameOver.setTexture(textureGameOver);
    spriteGameOver.setPosition(0, 0);

    View hudView(FloatRect(0, 0, resolution.x, resolution.y));

    Sprite spriteAmmoIcon;
    Texture textureAmmoIcon = TextureHolder::getTexture("graphics/ammo_icon.png");
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

    std::ifstream inputFile("gamedata/scores.txt");
    if(inputFile.is_open())
    {
        inputFile >> hiScore;
        inputFile.close();
    }


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

    // time of last update
    Time timeSinceLastUpdate;

    // how often to update the HUD
    int fpsMeasurementFrameInterval = 1000;

    // SoundBuffer to hold the audio, Sound to play the audio
    SoundBuffer hitBuffer;
    hitBuffer.loadFromFile("sound/hit.wav");
    Sound hit;
    hit.setBuffer(hitBuffer);

    SoundBuffer splatBuffer;
    splatBuffer.loadFromFile("sound/splat.wav");
    Sound splat;
    splat.setBuffer(splatBuffer);

    SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("sound/shoot.wav");
    Sound shoot;
    shoot.setBuffer(shootBuffer);

    SoundBuffer reloadBuffer;
    reloadBuffer.loadFromFile("sound/reload.wav");
    Sound reload;
    reload.setBuffer(reloadBuffer);

    SoundBuffer reloadFaildBuffer;
    reloadFaildBuffer.loadFromFile("sound/reload_failed.wav");
    Sound reloadFailed;
    reloadFailed.setBuffer(reloadFaildBuffer);

    SoundBuffer powerupBuffer;
    powerupBuffer.loadFromFile("sound/powerup.wav");
    Sound powerup;
    powerup.setBuffer(powerupBuffer);

    SoundBuffer pickupBuffer;
    pickupBuffer.loadFromFile("sound/pickup.wav");
    Sound pickup;
    pickup.setBuffer(pickupBuffer);





    while(window.isOpen())
    {
        Event event;
        while(window.pollEvent(event))
        {
            if(event.type == Event::KeyPressed)
            {
                LOG_INFO("KeyPressed");

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
                    LOG_INFO("State: GameOver, and Return pressed");
                    state = State::LEVELING_UP;
                    LOG_INFO("Set State: LEVELING_UP");

                    wave = 0;
                    score = 0;

                    currentBullet = 0;
                    bulletsSpare  = 24;
                    bulletsInClip = 6;
                    clipSize      = 6;
                    fireRate      = 1;
                    player.resetPlayerStats();
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
                            reload.play();

                        }
                        else if(bulletsSpare > 0)
                        {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0; 
                            reload.play();
                        }
                        else
                        {
                            reloadFailed.play();
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
                    shoot.play();
                    bulletsInClip--;
                }
            }


        } // end WSAD


        if(state == State::LEVELING_UP)
        {
            LOG_INFO("Enter State: LEVELING_UP");
            if(event.key.code == Keyboard::Num1)
            {
                fireRate++;
                state = State::PLAYING;
                LOG_INFO("Set State: PLAYING");
            }
            if(event.key.code == Keyboard::Num2)
            {
                clipSize += clipSize;
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num3)
            {
                player.upgradeHealth();
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num4)
            {
                healthPickup.upgrade();   
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num5)
            {
                ammoPickup.upgrade();
                state = State::PLAYING;
            }
            if(event.key.code == Keyboard::Num6)
            {
                state = State::PLAYING;
            }
            
            if(state == State::PLAYING)
            {
                LOG_INFO("Enter state: PLAYING, and Create Zombies");
                wave++;
                arena.width  = 500 * wave;
                arena.height = 500 * wave;
                arena.left   = 0;
                arena.top    = 0;
                int tileSize = createBackground(background, arena);

                player.spawn(arena, resolution, tileSize);  // arena is smaller than resolution

                healthPickup.setArena(arena);
                ammoPickup.setArena(arena);


                numZombies = 5 * wave;
                delete[] zombies;
                zombies = createHorde(numZombies, arena);
                numZombiesAlive = numZombies;

                LOG_INFO("Play powerup sound"); 
                powerup.play();

                clock.restart();
            }
        } // end Levelling up




        // update
        if(state == State::PLAYING)
        {
            LOG_INFO("Enter State: PLAYING");
            Time dt = clock.restart();
            gameTimeTotal += dt;
            float dtAsSeconds = dt.asSeconds();

            LOG_INFO("Set Crosshair");

            /*
              ----------- why cannot do spriteCrosshair.setPosition(Mouse::getPosition()); -----------


            */
            mouseScreenPosition = Mouse::getPosition();
            mouseWorldPosition  = window.mapPixelToCoords(Mouse::getPosition(), mainView);
            spriteCrosshair.setPosition(mouseWorldPosition);

            player.update(dtAsSeconds, Mouse::getPosition());
            Vector2f playerPosition(player.getCenter());

            mainView.setCenter(player.getCenter());

            LOG_INFO("Update Zombies");
            for(int i = 0; i < numZombies; i++)
            {
                if(zombies[i].isAlive())
                {
                    zombies[i].update(dt.asSeconds(), playerPosition);
                }
            }

            LOG_INFO("Update bullets");
            //update all the bullets that are in-flight
            for(int i = 0; i < 100; i++)
            {
                if(bullets[i].isInFlight()) //set it to true when shoot
                {
                    bullets[i].update(dtAsSeconds);
                }
            }

            // update the pickups
            LOG_INFO("Update the pickups");
            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);

            // collision detection
            LOG_INFO("Detect Collision");
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
                            LOG_INFO("Bullet hits Zombie");
                            splat.play();
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
                        LOG_INFO("Zombie hit the player");
                        hit.play();
                    }
                    if(player.getHealth() <= 0)
                    {
                        state = State::GAME_OVER;
                        std::ofstream outputFile("gamedata/scores.txt");
                        outputFile << hiScore;
                        outputFile.close();
                    }
                }
            } // end zombie touches the player

            // has the player touched health pickup?
            if(player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
            {
                LOG_INFO("Player touches health pickup");
                player.increaseHealthLevel(healthPickup.gotIt());
                pickup.play();
            }

            // has the player touched ammo pickup?
            if(player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
            {
                LOG_INFO("Player touches ammo pickup");
                bulletsSpare += ammoPickup.gotIt();
                reload.play(); //? or powerup.play();
            }

            healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

            timeSinceLastUpdate += dt;

            framesSinceLastHUDUpdate++;
            if(framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
            {
                // update game HUD(heads-up display) text
                LOG_INFO("Update game Heads-up display");
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssHiScore;
                std::stringstream ssWave;
                std::stringstream ssZombiesAlive;

                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());

                ssScore << "Score: " << score;
                scoreText.setString(ssScore.str());

                ssHiScore << "Hi Score: " << hiScore;
                hiScoreText.setString(ssHiScore.str());

                ssWave << "Wave: " << wave;
                waveNumberText.setString(ssWave.str());

                ssZombiesAlive << "Zombies: " << numZombiesAlive;
                zombiesRemainingText.setString(ssZombiesAlive.str());

                framesSinceLastHUDUpdate = 0;
                timeSinceLastUpdate = Time::Zero; // Time::Zero: time duration of 0
            }


        } // end Playing

        // draw
        if(state == State::PLAYING)
        {
            window.clear();
            // Activate mainView, then draw background, player, zombies, bullets, pickups and crosshair.
            window.setView(mainView);
            LOG_INFO("Draw background");
            window.draw(background, &textureBackground);

            LOG_INFO("Draw Zombies");
            for(int i = 0; i < numZombies; i++)
            {
                window.draw(zombies[i].getSprite());
            }

            LOG_INFO("Draw bullets");
            for(int i = 0; i < 100; i++)
            {
                if(bullets[i].isInFlight())
                {
                    window.draw(bullets[i].getShape());
                }
            }

            LOG_INFO("Draw player");
            window.draw(player.getSprite());

            if(ammoPickup.isSpawned())
            {
                LOG_INFO("Draw ammo pickup");
                window.draw(ammoPickup.getSprite());
            }
            if(healthPickup.isSpawned())
            {
                LOG_INFO("Draw health pickup");
                window.draw(healthPickup.getSprite());
            }

            LOG_INFO("Draw crosshair");
            window.draw(spriteCrosshair);


            // Activate hudView, then draw hudView, draw AmmoIcon, Text, healthBar
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
            LOG_INFO("Level up");
            window.draw(spriteGameOver);
            window.draw(levelUpText);

        }
        if(state == State::PAUSED)
        {
            LOG_INFO("PAUSED");
            window.draw(pausedText);
        }
        if(state == State::GAME_OVER)
        {
            LOG_INFO("GAME OVER");
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            
        }
        window.display();

    } //end game loop

    return 0; 
}