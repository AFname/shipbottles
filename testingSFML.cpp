#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

class Menu
{
public:
    Menu(sf::RenderWindow& window) : window(window)
    {
        font.loadFromFile("ArialRegular.otf");

        title.setFont(font);
        title.setString("SpaceShip");
        title.setCharacterSize(150);
        title.setFillColor(sf::Color::White);
        title.setPosition(200.f, 200.f);

        startButton.setFont(font);
        startButton.setString("Start Game");
        startButton.setCharacterSize(100);
        startButton.setFillColor(sf::Color::White);
        startButton.setPosition(200.f, 400.f);

        settingsButton.setFont(font);
        settingsButton.setString("Settings");
        settingsButton.setCharacterSize(100);
        settingsButton.setFillColor(sf::Color::White);
        settingsButton.setPosition(200.f, 600.f);

        aboutButton.setFont(font);
        aboutButton.setString("About");
        aboutButton.setCharacterSize(100);
        aboutButton.setFillColor(sf::Color::White);
        aboutButton.setPosition(200.f, 800.f);

        text1.setFont(font);
        text1.setString("Svitkovskiy");
        text1.setCharacterSize(100);
        text1.setFillColor(sf::Color::White);
        text1.setPosition(200.f, 200.f);

        returnButton.setFont(font);
        returnButton.setString("return");
        returnButton.setCharacterSize(100);
        returnButton.setFillColor(sf::Color::White);
        returnButton.setPosition(200.f, 1200.f);

        if (!backgroundTextureMenu.loadFromFile("backgroundMenu.jpg"))
        {
            exit(-1);
        }
        spriteBackgroundMenu.setTexture(backgroundTextureMenu);
        spriteBackgroundMenu.setTextureRect(sf::IntRect(0, 0, 1024, 1024));
        spriteBackgroundMenu.setScale(sf::Vector2f(1.42, 1.42));
    }

    void draw()
    {
        window.draw(spriteBackgroundMenu);
        window.draw(title);
        window.draw(startButton);
        window.draw(aboutButton);
        window.draw(settingsButton);
    }

    void drawAbout()
    {
        window.draw(spriteBackgroundMenu);
        window.draw(text1);

        window.draw(returnButton);
    }

    bool isStartButtonPressed(sf::Vector2f mousePos)
    {
        return startButton.getGlobalBounds().contains(mousePos);
    }
    bool isSettingsButtonPressed(sf::Vector2f mousePos)
    {
        return settingsButton.getGlobalBounds().contains(mousePos);
    }
    bool isAboutButtonPressed(sf::Vector2f mousePos)
    {
        return aboutButton.getGlobalBounds().contains(mousePos);
    }
    bool isReturnButtonPressed(sf::Vector2f mousePos)
    {
        return returnButton.getGlobalBounds().contains(mousePos);
    }

private:
    sf::Texture backgroundTextureMenu;
    sf::RenderWindow& window;
    sf::Sprite spriteBackgroundMenu;
    sf::Font font;
    sf::Text title;
    sf::Text startButton;
    sf::Text settingsButton;
    sf::Text aboutButton;
    sf::Text returnButton;
    sf::Text text1;
};

sf::Vector2f normalize(sf::Vector2f move)
{
    float modul = sqrt(move.x * move.x + move.y * move.y);
    return (modul != 0) ? move / modul : sf::Vector2f(0, 0);
}

sf::Vector2f calculateDirection(float angleDegrees) {
    float angleRadians = angleDegrees * (atan(1) * 4) / 180.0f;

    float x = cos(angleRadians);
    float y = sin(angleRadians);

    return sf::Vector2f(x, y);
}

float CalculateAngle(sf::Vector2f spritePos, sf::Vector2f mousePos) {
    float dx = mousePos.x - spritePos.x;
    float dy = mousePos.y - spritePos.y;

    return atan2(dy, dx) * 180 / (atan(1) * 4);
}

bool checkCollision(sf::Vector2f a, float aSize, sf::Vector2f b, float bSize)
{
    float halfSizeA = aSize / 2.0f;
    float halfSizeB = bSize / 2.0f;

    sf::Vector2f aMin = a - sf::Vector2f(halfSizeA, halfSizeA);
    sf::Vector2f aMax = a + sf::Vector2f(halfSizeA, halfSizeA);
    sf::Vector2f bMin = b - sf::Vector2f(halfSizeB, halfSizeB);
    sf::Vector2f bMax = b + sf::Vector2f(halfSizeB, halfSizeB);

    if (aMax.x <= bMin.x || aMin.x >= bMax.x)
        return false;
    if (aMax.y <= bMin.y || aMin.y >= bMax.y)
        return false;

    return true; //stuck -> minus Health
}

struct Bullet {
    bool whose = 0; // 0 - enemy, 1 - player
    sf::Vector2f position = {};
    sf::Vector2f direction = {};
    float angle = 1;
    float bulletSpeed = 500.f;
};

struct Enemy {
    float health = 100;
    int typeOfTexture = 0;
    sf::Vector2f position = {};
    sf::Vector2f viewDirection = sf::Vector2f(1,0);
    float speed = 0;
    float turnSpeed = 0;
    float enemyBulletSpeed = 300.f;
};

struct GameplayData {
    sf::Vector2f playerPos = { 2400, 2400 };
    float health = 100;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    int killCounter = 0;
};

int main()
{
#pragma region default
    sf::RenderWindow window(sf::VideoMode(1440, 1440), "SpaceShip");
    sf::Event event;
    Menu menu(window);
    bool inMenu = true;
    bool inSettings = false;
    bool inAbout = false;
    bool inPause = false;
    bool keyKostul = false;
#pragma endregion default

#pragma region spaceshipTexture
    sf::Texture spaceshipTexture;
    if (!spaceshipTexture.loadFromFile("spaceships2.png"))
    {
        exit(-1);
    }
    sf::Sprite spriteSpaceship;
    spriteSpaceship.setTexture(spaceshipTexture);
    spriteSpaceship.setTextureRect(sf::IntRect(0, 0, 48, 48));
    spriteSpaceship.setOrigin(sf::Vector2f(24, 24));
    spriteSpaceship.setScale(sf::Vector2f(2, 2));
#pragma endregion spaceshipTexture

#pragma region enemies
    sf::Texture spaceshipEnemyTexture[3];
    sf::Sprite spriteSpaceshipEnemy[3];
    for (int i = 0; i < 3; i++) {
        if (!spaceshipEnemyTexture[i].loadFromFile("spaceships2.png"))
        {
            exit(-1);
        }
        spriteSpaceshipEnemy[i].setTexture(spaceshipEnemyTexture[i]);
        spriteSpaceshipEnemy[i].setOrigin(sf::Vector2f(24, 24));
        spriteSpaceshipEnemy[i].setScale(sf::Vector2f(2, 2));
    }
    spriteSpaceshipEnemy[0].setTextureRect(sf::IntRect(48, 0, 48, 48));
    spriteSpaceshipEnemy[1].setTextureRect(sf::IntRect(0, 48, 48, 48));
    spriteSpaceshipEnemy[2].setTextureRect(sf::IntRect(48, 48, 48, 48));



#pragma endregion enemies

#pragma region bulletMy
    sf::Texture bulletTextureMy;
    if (!bulletTextureMy.loadFromFile("bulletMy.png"))
    {
        exit(-1);
    }
    sf::Sprite spriteBulletMy;
    spriteBulletMy.setTexture(bulletTextureMy);
    spriteBulletMy.setTextureRect(sf::IntRect(0, 0, 48, 48));
    spriteBulletMy.setScale(sf::Vector2f(0.5, 0.5));
    spriteBulletMy.setOrigin(sf::Vector2f(24, 24));
#pragma endregion bulletMy

#pragma region bulletEnemy
    sf::Texture bulletTextureEnemy;
    if (!bulletTextureEnemy.loadFromFile("bulletEnemy.png"))
    {
        exit(-1);
    }
    sf::Sprite spriteBulletEnemy;
    spriteBulletEnemy.setTexture(bulletTextureEnemy);
    spriteBulletEnemy.setTextureRect(sf::IntRect(0, 0, 48, 48));
    spriteBulletEnemy.setScale(sf::Vector2f(0.5, 0.5));
    spriteBulletEnemy.setOrigin(sf::Vector2f(24, 24));
#pragma endregion bulletEnemy

#pragma region background
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background2.png"))
    {
        exit(-1);
    }
    sf::Sprite spriteBackground;
    spriteBackground.setTexture(backgroundTexture);
    spriteBackground.setTextureRect(sf::IntRect(0, 0, 1024, 1024));
    spriteBackground.setScale(sf::Vector2f(4, 4));
    GameplayData data;
#pragma endregion background

#pragma region mouse
    sf::Mouse mouse;
#pragma endregion mouse

#pragma region Text
    sf::Font font;
    if (!font.loadFromFile("ArialRegular.otf"))
    {
        std::cout << "Error loading font\n";
    }
    sf::Text healthNumber;
    healthNumber.setFont(font);
    healthNumber.setCharacterSize(100);
    healthNumber.setFillColor(sf::Color::Red);
    sf::Text killCounterNumber;
    killCounterNumber.setFont(font);
    killCounterNumber.setCharacterSize(100);
    killCounterNumber.setFillColor(sf::Color::Yellow);
#pragma endregion Text

#pragma region otherStuff
    sf::Vector2f move = { 0, 0 };

    sf::Clock clock;
    sf::Clock bulletClock;
    sf::Clock enemiesClock;
    sf::Clock enemiesBulletsClock;
    sf::Clock healtAddTime;
    healtAddTime.restart();
    bulletClock.restart();
    enemiesClock.restart();
    enemiesBulletsClock.restart();
    sf::View view;
#pragma endregion otherStuff


#pragma region WINDOW
    while (window.isOpen()) {

        sf::Time dt = clock.restart();

#pragma region close
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (inMenu && event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                if (menu.isStartButtonPressed(mousePos))
                {
                    inMenu = false;
                    inSettings = false;
                    inAbout = false;
                    std::cout << "1";
                }
                if (menu.isSettingsButtonPressed(mousePos))
                {
                    inMenu = true;
                    inSettings = true;
                    inAbout = false;
                    std::cout << "2";
                }
                if (menu.isAboutButtonPressed(mousePos))
                {
                    inMenu = true;
                    inSettings = false;
                    inAbout = true;
                    std::cout << "3";
                }
                if (menu.isReturnButtonPressed(mousePos))
                {
                    inMenu = true;
                    inSettings = false;
                    inAbout = false;
                    std::cout << "4";
                }
            }
            
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
            inMenu = true;
        }
#pragma endregion close
        if (!inMenu)
        {
            
#pragma region movement

            move = { 0, 0 };

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
                move.y -= 1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                move.y += 1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
                move.x -= 1;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                move.x += 1;
            }

            if (data.playerPos.x < (24) and move.x == -1) {
                move.x = 0;
            }
            if (data.playerPos.x > (4800 - 24) and move.x == 1) {
                move.x = 0;
            }
            if (data.playerPos.y < (24) and move.y == -1) {
                move.y = 0;
            }
            if (data.playerPos.y > (4800 - 24) and move.y == 1) {
                move.y = 0;
            }

            if (move.x != 0 || move.y != 0) {
                move = normalize(move);
                data.playerPos += move * (700.0f * dt.asSeconds());  
            }
            spriteSpaceship.setPosition(data.playerPos);
#pragma endregion movement

#pragma region mousePosition
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mousePosView = window.mapPixelToCoords(mousePos);
            float angle = CalculateAngle(spriteSpaceship.getPosition(), mousePosView);
            spriteSpaceship.setRotation(angle + 90);
#pragma endregion mousePosition

#pragma region bulletPosition
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) and bulletClock.getElapsedTime().asMilliseconds() > 100) {
                Bullet b;
                b.whose = 1;
                b.position = data.playerPos;
                b.direction = calculateDirection(angle);
                b.angle = angle;
                b.bulletSpeed = 1200.f;
                data.bullets.push_back(b);
                bulletClock.restart();
            }
#pragma endregion bulletPosition

#pragma region enemyBullet
            if (enemiesBulletsClock.getElapsedTime().asMilliseconds() > 200) {
                for (int i = 0; i < data.enemies.size(); ++i) {
                    Bullet b;
                    b.whose = 0;
                    b.position = data.enemies[i].position;
                    b.direction = calculateDirection(CalculateAngle(data.enemies[i].position, data.playerPos));
                    b.angle = CalculateAngle(data.enemies[i].position, data.playerPos);
                    b.bulletSpeed = data.enemies[i].enemyBulletSpeed;
                    data.bullets.push_back(b);
                }
                enemiesBulletsClock.restart();
            }

#pragma endregion enemyBullet

#pragma region bulletUpdate
            for (int i = 0; i < data.bullets.size(); ++i) {
                if (data.bullets[i].whose) {
                    spriteBulletMy.setPosition(data.bullets[i].position += data.bullets[i].direction * dt.asSeconds() * data.bullets[i].bulletSpeed);
                    if (std::abs(std::hypot(data.bullets[i].position.x - data.playerPos.x, data.bullets[i].position.y - data.playerPos.y)) > 1000) {
                        data.bullets.erase(data.bullets.begin() + i);
                        i--;
                    }
                }
                else {
                    spriteBulletEnemy.setPosition(data.bullets[i].position += data.bullets[i].direction * dt.asSeconds() * data.bullets[i].bulletSpeed);
                    if (std::abs(std::hypot(data.bullets[i].position.x - data.playerPos.x, data.bullets[i].position.y - data.playerPos.y)) > 1000) {
                        data.bullets.erase(data.bullets.begin() + i);
                        i--;
                    }
                }
            }
#pragma endregion bulletUpdate

#pragma region enemiesSpawn
            if (enemiesClock.getElapsedTime().asMilliseconds() > 750 and data.enemies.size() < 30) {
                Enemy b;
                b.typeOfTexture = rand() % 3;
                b.position = data.playerPos + sf::Vector2f((std::abs(rand() % 1200 - 600) > 50 ? rand() % 1200 - 600 : 51), (std::abs(rand() % 1200 - 600) > 50 ? rand() % 1200 - 600 : 51));
                b.viewDirection = normalize(data.playerPos - b.position);
                b.speed = 150.f + rand() % 200;
                b.turnSpeed = 2.f + (rand() % 1000) / 500.f;
                b.enemyBulletSpeed = 200 + rand() % 400;
                data.enemies.push_back(b);
                enemiesClock.restart();
            }

#pragma endregion enemiesSpawn

#pragma region enemiesUpdate

            for (int i = 0; i < data.enemies.size(); ++i) {
                if (CalculateAngle(data.enemies[i].viewDirection, normalize(data.playerPos - data.enemies[i].position)) > 0.1) {
                    data.enemies[i].viewDirection += dt.asSeconds() * data.enemies[i].turnSpeed * normalize(data.playerPos - data.enemies[i].position);
                }
                else {
                    data.enemies[i].viewDirection = normalize(data.playerPos - data.enemies[i].position);
                }

                data.enemies[i].position += dt.asSeconds() * data.enemies[i].viewDirection * data.enemies[i].speed;
            }

#pragma endregion enemiesUpdate

#pragma region bulletCollision

            for (int i = 0; i < data.bullets.size(); ++i) {
                if (data.bullets[i].whose) { //my
                    keyKostul = true;
                    for (int j = 0; j < data.enemies.size(); ++j) {
                        if(keyKostul) {
                            if (checkCollision(data.bullets[i].position, 48.f, data.enemies[j].position, 48)) {
                                data.enemies[j].health -= 50;
                                if (data.enemies[j].health <= 0) {
                                    data.enemies.erase(data.enemies.begin() + j);
                                    data.killCounter++;
                                }
                                data.bullets.erase(data.bullets.begin() + i);
                                keyKostul = false;
                            }
                        }
                    }
                }
                else { //enemy
                    if (checkCollision(data.bullets[i].position, 48.f, data.playerPos, 48)) {
                        data.health -= 7;
                        if (data.health <= 0) {
                            data = {};
                            break;
                        }
                        data.bullets.erase(data.bullets.begin() + i);
                        
                    }
                }
            }

#pragma endregion bulletCollision

#pragma region healthAdd

            if (data.health < 100 and healtAddTime.getElapsedTime().asSeconds() > 1) {
                data.health += 50;
                if (data.health > 100) {
                    data.health = 100;
                }
                healtAddTime.restart();
            }
        }
#pragma endregion healthAdd
        window.clear(sf::Color::Black);
#pragma region drawings
        if (inMenu)
        {   
            if (inAbout) {
                menu.drawAbout();
            }
            else
            {
                menu.draw();
            }

        }
        else
        {


#pragma region backgroundDraw
            //clear

            view.setCenter(data.playerPos + sf::Vector2f(24, 24));
            view.setSize(sf::Vector2f(1800, 1800));
            window.setView(view);
            window.draw(spriteBackground);
#pragma endregion backgroundDraw

        //enemies
            for (int i = 0; i < data.enemies.size(); ++i) {
                spriteSpaceshipEnemy[data.enemies[i].typeOfTexture].setPosition(data.enemies[i].position);
                spriteSpaceshipEnemy[data.enemies[i].typeOfTexture].setRotation(CalculateAngle(data.playerPos, data.enemies[i].position) - 90);
                window.draw(spriteSpaceshipEnemy[data.enemies[i].typeOfTexture]);
            }
            //bullet
            for (int i = 0; i < data.bullets.size(); ++i) {
                if (data.bullets[i].whose) {
                    spriteBulletMy.setPosition(data.bullets[i].position);
                    spriteBulletMy.setRotation(data.bullets[i].angle);
                    window.draw(spriteBulletMy);
                }
                else {
                    spriteBulletEnemy.setPosition(data.bullets[i].position);
                    spriteBulletEnemy.setRotation(data.bullets[i].angle);
                    window.draw(spriteBulletEnemy);
                }
            }
            //spaceship
            window.draw(spriteSpaceship);
            //text 
            healthNumber.setString(std::to_string((int)data.health));
            healthNumber.setPosition(sf::Vector2f((view.getCenter() - sf::Vector2f(-600, 800))));
            window.draw(healthNumber);
            killCounterNumber.setString(std::to_string(data.killCounter));
            killCounterNumber.setPosition(sf::Vector2f((view.getCenter() - sf::Vector2f(-600, 700))));
            window.draw(killCounterNumber);
            //window
        }
        window.display();
#pragma endregion drawings
    }
#pragma endregion WINDOW

    return 0;
}
