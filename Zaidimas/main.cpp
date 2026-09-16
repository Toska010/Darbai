#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <iostream>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdint>

// =====================================================
// LevelConfig (JSON-driven)
// =====================================================
struct LevelConfig {
    std::string name;

    float waveDuration = 30.f;

    // Spawn interval = base * ramp^(wave-1) * spawnUpgradeMult
    float enemySpawnBaseInterval = 0.80f;
    float enemySpawnRampPerWave  = 1.00f; // easy 0.95, normal 0.93, hard 0.90

    float enemySpeed = 150.f;
    int enemyEscapeDamage = 5;

    int playerMaxHP = 100;
    float playerMoveSpeed = 360.f;
    float playerShotsPerSecond = 1.0f;
    float bulletSpeed = 650.f;

    int killsPerLevel = 10;

    sf::Color backgroundColor{10,10,18};
    std::string sourceFile;
};

// =====================================================
// Manual JSON parsing (no libs)
// =====================================================
static std::string readFileToString(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return {};
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}
static size_t skipSpaces(const std::string& s, size_t i) {
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return i;
}
static bool jsonGetString(const std::string& json, const std::string& key, std::string& outValue) {
    const std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos + pattern.size());
    if (pos == std::string::npos) return false;
    pos = skipSpaces(json, pos + 1);
    if (pos >= json.size() || json[pos] != '"') return false;
    size_t end = json.find('"', pos + 1);
    if (end == std::string::npos) return false;
    outValue = json.substr(pos + 1, end - (pos + 1));
    return true;
}
static bool jsonGetFloat(const std::string& json, const std::string& key, float& outValue) {
    const std::string pattern = "\"" + key + "\"";
    size_t pos = json.find(pattern);
    if (pos == std::string::npos) return false;
    pos = json.find(':', pos + pattern.size());
    if (pos == std::string::npos) return false;
    pos = skipSpaces(json, pos + 1);
    if (pos >= json.size()) return false;

    size_t end = pos;
    while (end < json.size() &&
           (std::isdigit(static_cast<unsigned char>(json[end])) ||
            json[end] == '.' || json[end] == '-' || json[end] == '+' ||
            json[end] == 'e' || json[end] == 'E')) {
        ++end;
    }
    try {
        outValue = std::stof(json.substr(pos, end - pos));
        return true;
    } catch (...) {
        return false;
    }
}
static bool jsonGetInt(const std::string& json, const std::string& key, int& outValue) {
    float tmp{};
    if (!jsonGetFloat(json, key, tmp)) return false;
    outValue = static_cast<int>(tmp);
    return true;
}
// expects: "backgroundColor": [12,22,14]
static bool jsonGetColor3(const std::string& json, const std::string& key, sf::Color& out) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return false;

    int r=0,g=0,b=0;
    if (std::sscanf(json.c_str() + pos, "[%d,%d,%d]", &r, &g, &b) != 3) return false;

    r = std::clamp(r, 0, 255);
    g = std::clamp(g, 0, 255);
    b = std::clamp(b, 0, 255);

    out = sf::Color(
        static_cast<std::uint8_t>(r),
        static_cast<std::uint8_t>(g),
        static_cast<std::uint8_t>(b)
    );
    return true;
}

static bool loadLevelFromJsonFile(const std::string& filename, LevelConfig& out, std::string& err) {
    const std::string json = readFileToString(filename);
    if (json.empty()) { err = "Cannot open: " + filename; return false; }

    if (!jsonGetString(json, "name", out.name)) { err = "Missing name"; return false; }
    if (!jsonGetColor3(json, "backgroundColor", out.backgroundColor)) { err = "Missing backgroundColor"; return false; }

    jsonGetFloat(json, "waveDuration", out.waveDuration);
    jsonGetFloat(json, "enemySpawnBaseInterval", out.enemySpawnBaseInterval);
    jsonGetFloat(json, "enemySpawnRampPerWave", out.enemySpawnRampPerWave);
    jsonGetFloat(json, "enemySpeed", out.enemySpeed);
    jsonGetInt(json, "enemyEscapeDamage", out.enemyEscapeDamage);

    jsonGetInt(json, "playerMaxHP", out.playerMaxHP);
    jsonGetFloat(json, "playerMoveSpeed", out.playerMoveSpeed);
    jsonGetFloat(json, "playerShotsPerSecond", out.playerShotsPerSecond);
    jsonGetFloat(json, "bulletSpeed", out.bulletSpeed);

    jsonGetInt(json, "killsPerLevel", out.killsPerLevel);

    if (out.waveDuration <= 0.f) out.waveDuration = 30.f;
    if (out.enemySpawnBaseInterval <= 0.f) { err = "enemySpawnBaseInterval must be > 0"; return false; }
    if (out.enemySpawnRampPerWave <= 0.f)  { err = "enemySpawnRampPerWave must be > 0"; return false; }
    if (out.enemySpeed <= 0.f)             { err = "enemySpeed must be > 0"; return false; }

    out.enemyEscapeDamage = std::max(0, out.enemyEscapeDamage);

    if (out.playerMaxHP <= 0)              { err = "playerMaxHP must be > 0"; return false; }
    if (out.playerMoveSpeed <= 0.f)        { err = "playerMoveSpeed must be > 0"; return false; }
    if (out.playerShotsPerSecond <= 0.f)   { err = "playerShotsPerSecond must be > 0"; return false; }
    if (out.bulletSpeed <= 0.f)            { err = "bulletSpeed must be > 0"; return false; }
    if (out.killsPerLevel <= 0)            { err = "killsPerLevel must be > 0"; return false; }

    out.sourceFile = filename;
    err.clear();
    return true;
}

// =====================================================
// UI helpers
// =====================================================
static void centerText(sf::Text& t) {
    auto b = t.getLocalBounds();
    t.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
}
static void drawCenteredText(sf::RenderWindow& window,
                            const sf::Font& font,
                            const std::string& str,
                            unsigned size,
                            sf::Vector2f pos,
                            sf::Color color)
{
    sf::Text t(font, str, size);
    t.setFillColor(color);
    centerText(t);
    t.setPosition(pos);
    window.draw(t);
}

// =====================================================
// Menu
// =====================================================
struct MenuButton {
    sf::RectangleShape rect;
    std::string label;
    std::string filename;
};
static void layoutMenu(const sf::Vector2u ws, std::vector<MenuButton>& buttons) {
    const float cx = ws.x * 0.5f;
    float y = ws.y * 0.40f;
    for (auto& b : buttons) {
        b.rect.setOrigin(b.rect.getSize() * 0.5f);
        b.rect.setPosition({cx, y});
        y += 110.f;
    }
}

// =====================================================
// Entities: bullets + enemies
// =====================================================
struct Bullet {
    sf::CircleShape shape;
    sf::Vector2f vel;
    float speed = 650.f;
    bool homing = false;
    int pierce = 1;
};

struct Enemy {
    sf::ConvexShape shape; // red upside-down triangle
    sf::Vector2f vel;
};

static sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = std::sqrt(v.x*v.x + v.y*v.y);
    if (len <= 0.0001f) return {0.f, 0.f};
    return {v.x/len, v.y/len};
}

static Enemy makeEnemy(sf::Vector2f pos, float speed) {
    Enemy e;
    const float W = 56.f; // width stays same as before
    const float H = 56.f;
    e.shape = sf::ConvexShape(3);
    e.shape.setPoint(0, {0.f, 0.f});
    e.shape.setPoint(1, {W, 0.f});
    e.shape.setPoint(2, {W * 0.5f, H}); // tip down
    e.shape.setOrigin({W * 0.5f, H * 0.5f});
    e.shape.setFillColor(sf::Color(240, 80, 80));
    e.shape.setPosition(pos);
    e.vel = {0.f, speed};
    return e;
}

static Bullet makeBullet(sf::Vector2f pos, float speed, bool homing, int pierce, sf::Color color) {
    Bullet b;
    b.shape.setRadius(6.f);
    b.shape.setOrigin({6.f, 6.f});
    b.shape.setFillColor(color);
    b.shape.setPosition(pos);
    b.speed = speed;
    b.homing = homing;
    b.pierce = std::max(1, pierce);
    b.vel = {0.f, -speed};
    return b;
}

// =====================================================
// Helpers
// =====================================================
struct HelperDrone {
    sf::ConvexShape shape;
    float x = 0.f;
    float speedFactor = 1.0f;
    float shootTimer = 0.f;
};
static sf::ConvexShape makeHelperShape() {
    sf::ConvexShape h(3);
    h.setPoint(0, {0.f,  0.f});
    h.setPoint(1, {24.f, 0.f});
    h.setPoint(2, {12.f, -18.f});
    h.setOrigin({12.f, -9.f});
    h.setFillColor(sf::Color(120, 160, 255));
    return h;
}

// =====================================================
// Shop
// =====================================================
enum class ShopAction {
    None,
    P_AttackSpeed, P_Piercing, P_BulletSpeed, P_Homing,
    E_Slow, E_SpawnFaster,
    H_Buy, H_FireRate, H_Piercing, H_Homing,
    Continue
};
struct ShopItem {
    sf::RectangleShape rect;
    std::string label;
    int cost = 0;
    ShopAction action = ShopAction::None;
};

enum class GameState { Menu, Wave, Shop };

static ShopItem makeItem(float x, float y, float w, float h,
                         const std::string& label, int cost, ShopAction action,
                         sf::Color fill = sf::Color(25,25,40))
{
    ShopItem it;
    it.rect.setPosition({x, y});
    it.rect.setSize({w, h});
    it.rect.setFillColor(fill);
    it.rect.setOutlineThickness(2.f);
    it.rect.setOutlineColor(sf::Color(120,120,160));
    it.label = label;
    it.cost = cost;
    it.action = action;
    return it;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), "Zaidimas");
    window.setVerticalSyncEnabled(true);

    sf::View view(sf::FloatRect({0.f, 0.f}, sf::Vector2f(window.getSize())));
    window.setView(view);

    std::mt19937 rng(std::random_device{}());

    sf::Font font;
    const bool fontLoaded = font.openFromFile("resources/Roboto-Regular.ttf");

    // Menu buttons
    std::vector<MenuButton> buttons;
    buttons.reserve(3);
    {
        MenuButton a; a.label="1 - lengvas";  a.filename="lengvas.json";  a.rect.setSize({520.f, 86.f}); buttons.push_back(a);
        MenuButton b; b.label="2 - normalus"; b.filename="normalus.json"; b.rect.setSize({520.f, 86.f}); buttons.push_back(b);
        MenuButton c; c.label="3 - sunkus";   c.filename="sunkus.json";   c.rect.setSize({520.f, 86.f}); buttons.push_back(c);
    }
    for (auto& b : buttons) {
        b.rect.setFillColor(sf::Color(40, 40, 55));
        b.rect.setOutlineThickness(2.f);
        b.rect.setOutlineColor(sf::Color(120, 120, 160));
    }
    layoutMenu(window.getSize(), buttons);

    // Player
    sf::ConvexShape player(3);
    player.setPoint(0, {0.f,  0.f});
    player.setPoint(1, {40.f, 0.f});
    player.setPoint(2, {20.f, -30.f});
    player.setOrigin({20.f, -15.f});
    player.setFillColor(sf::Color(80, 240, 120));

    // Inventory icons
    sf::CircleShape iconPlayer(16.f), iconEnemy(16.f), iconHelper(16.f);
    iconPlayer.setOrigin({16.f,16.f});
    iconEnemy.setOrigin({16.f,16.f});
    iconHelper.setOrigin({16.f,16.f});
    iconPlayer.setFillColor(sf::Color(80,240,120));
    iconEnemy.setFillColor(sf::Color(240,80,80));
    iconHelper.setFillColor(sf::Color(120,160,255));

    // Game data
    LevelConfig lvl{};
    sf::Color bg = sf::Color(10,10,18);
    std::string lastError;

    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<HelperDrone> helpers;

    GameState state = GameState::Menu;

    float waveTimeLeft = 30.f;
    int waveNumber = 1;

    int killsProgress = 0;
    int playerLVL = 0;

    // HP
    int hpMax = 100;
    int hpCur = 100;

    // Upgrades
    float upP_AttackMult = 1.0f;
    int   upP_Piercing   = 0;
    float upP_BulletSpeedMult = 1.0f;
    bool  upP_Homing = false;

    float upE_SpeedMult = 1.0f;
    float upE_SpawnMult = 1.0f;

    const int MAX_HELPERS = 5;
    int helperCount = 0;
    float upH_FireRateMult = 1.0f;
    int   upH_Piercing = 0;
    bool  upH_Homing = false;

    // Costs
    const int C_P_ATK=1, C_P_PIER=1, C_P_BSPD=1, C_P_HOM=2;
    const int C_E_SLOW=1, C_E_SPAWN=1;
    const int C_H_BUY=3, C_H_RATE=1, C_H_PIER=1, C_H_HOM=2;

    // Gains
    const float G_P_ATK=1.15f, G_P_BSPD=1.15f;
    const int   G_P_PIER=1;
    const float G_E_SLOW=0.90f, G_E_SPAWN=0.90f;
    const float G_H_RATE=1.15f;
    const int   G_H_PIER=1;

    float spawnTimer = 0.f;
    float shootTimer = 0.f;

    sf::Clock clock;

    // HUD bars
    sf::RectangleShape lvlBarBg({360.f, 22.f});
    lvlBarBg.setFillColor(sf::Color(40, 40, 55));
    lvlBarBg.setOutlineThickness(2.f);
    lvlBarBg.setOutlineColor(sf::Color(120, 120, 160));
    sf::RectangleShape lvlBarFill;
    lvlBarFill.setFillColor(sf::Color(60, 200, 90));

    sf::RectangleShape hpBarBg({260.f, 20.f});
    hpBarBg.setFillColor(sf::Color(40, 40, 55));
    hpBarBg.setOutlineThickness(2.f);
    hpBarBg.setOutlineColor(sf::Color(120, 120, 160));
    sf::RectangleShape hpBarFill;
    hpBarFill.setFillColor(sf::Color(220, 60, 60));

    auto ensureHelpers = [&] {
        while ((int)helpers.size() < helperCount) {
            HelperDrone h;
            h.shape = makeHelperShape();
            h.x = player.getPosition().x;
            std::uniform_real_distribution<float> df(0.90f, 1.10f);
            h.speedFactor = df(rng);
            h.shootTimer = 0.f;
            helpers.push_back(h);
        }
        if ((int)helpers.size() > helperCount) helpers.resize(helperCount);
    };

    auto resetRun = [&] {
        bullets.clear();
        enemies.clear();
        spawnTimer = 0.f;
        shootTimer = 0.f;

        waveNumber = 1;
        waveTimeLeft = lvl.waveDuration;

        killsProgress = 0;
        playerLVL = 0;

        hpMax = lvl.playerMaxHP;
        hpCur = hpMax;

        upP_AttackMult = 1.0f;
        upP_Piercing = 0;
        upP_BulletSpeedMult = 1.0f;
        upP_Homing = false;

        upE_SpeedMult = 1.0f;
        upE_SpawnMult = 1.0f;

        helperCount = 0;
        upH_FireRateMult = 1.0f;
        upH_Piercing = 0;
        upH_Homing = false;

        helpers.clear();

        const auto ws = window.getSize();
        player.setPosition({ws.x * 0.5f, ws.y * 0.80f});
    };

    auto addKill = [&] {
        killsProgress++;
        while (killsProgress >= lvl.killsPerLevel) {
            killsProgress -= lvl.killsPerLevel;
            playerLVL += 1;
        }
    };

    auto startWave = [&] {
        state = GameState::Wave;
        bullets.clear();
        enemies.clear();
        spawnTimer = 0.f;
        shootTimer = 0.f;
        waveTimeLeft = lvl.waveDuration;

        const auto ws = window.getSize();
        player.setPosition({ws.x * 0.5f, ws.y * 0.80f});
        ensureHelpers();
    };

    auto enterShop = [&] {
        state = GameState::Shop;
        bullets.clear();
        enemies.clear();
    };

    auto spawnIntervalNow = [&] {
        float ramp = std::pow(lvl.enemySpawnRampPerWave, (float)std::max(0, waveNumber - 1));
        return lvl.enemySpawnBaseInterval * ramp * upE_SpawnMult;
    };

    auto applyPurchase = [&](ShopAction a) {
        auto spend = [&](int c)->bool { if (playerLVL < c) return false; playerLVL -= c; return true; };

        switch (a) {
            case ShopAction::P_AttackSpeed: if (spend(C_P_ATK))  upP_AttackMult *= G_P_ATK; break;
            case ShopAction::P_Piercing:    if (spend(C_P_PIER)) upP_Piercing += G_P_PIER; break;
            case ShopAction::P_BulletSpeed: if (spend(C_P_BSPD)) upP_BulletSpeedMult *= G_P_BSPD; break;
            case ShopAction::P_Homing:      if (!upP_Homing && spend(C_P_HOM)) upP_Homing = true; break;

            case ShopAction::E_Slow:        if (spend(C_E_SLOW))  upE_SpeedMult *= G_E_SLOW; break;
            case ShopAction::E_SpawnFaster: if (spend(C_E_SPAWN)) upE_SpawnMult *= G_E_SPAWN; break;

            case ShopAction::H_Buy:
                if (helperCount < MAX_HELPERS && spend(C_H_BUY)) { helperCount++; ensureHelpers(); }
                break;
            case ShopAction::H_FireRate:    if (spend(C_H_RATE)) upH_FireRateMult *= G_H_RATE; break;
            case ShopAction::H_Piercing:    if (spend(C_H_PIER)) upH_Piercing += G_H_PIER; break;
            case ShopAction::H_Homing:      if (!upH_Homing && spend(C_H_HOM)) upH_Homing = true; break;

            case ShopAction::Continue:
                waveNumber++;
                startWave();
                break;
            default: break;
        }
    };

    bool mouseHeld = false;

    // ===========================
    // Main loop
    // ===========================
    while (window.isOpen()) {

        // SFML 3 event loop style [1](https://www.sfml-dev.org/tutorials/3.0/window/events/)
        while (const std::optional ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();

            if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scancode::Escape) window.close();

                if (state == GameState::Menu) {
                    int idx = -1;
                    if (key->scancode == sf::Keyboard::Scancode::Num1) idx = 0;
                    if (key->scancode == sf::Keyboard::Scancode::Num2) idx = 1;
                    if (key->scancode == sf::Keyboard::Scancode::Num3) idx = 2;

                    if (idx != -1) {
                        LevelConfig loaded;
                        std::string err;
                        if (!loadLevelFromJsonFile(buttons[idx].filename, loaded, err)) {
                            lastError = err;
                        } else {
                            lastError.clear();
                            lvl = loaded;
                            bg = lvl.backgroundColor;
                            resetRun();
                            startWave();
                        }
                    }
                } else if (state == GameState::Shop) {
                    // keyboard shortcuts still available
                    if (key->scancode == sf::Keyboard::Scancode::Num1) applyPurchase(ShopAction::P_AttackSpeed);
                    if (key->scancode == sf::Keyboard::Scancode::Num2) applyPurchase(ShopAction::P_Piercing);
                    if (key->scancode == sf::Keyboard::Scancode::Num3) applyPurchase(ShopAction::P_BulletSpeed);
                    if (key->scancode == sf::Keyboard::Scancode::Num4) applyPurchase(ShopAction::E_Slow);
                    if (key->scancode == sf::Keyboard::Scancode::Num5) applyPurchase(ShopAction::E_SpawnFaster);
                    if (key->scancode == sf::Keyboard::Scancode::Num6) applyPurchase(ShopAction::H_Buy);
                    if (key->scancode == sf::Keyboard::Scancode::Num7) applyPurchase(ShopAction::H_FireRate);
                    if (key->scancode == sf::Keyboard::Scancode::Num8) applyPurchase(ShopAction::H_Piercing);
                    if (key->scancode == sf::Keyboard::Scancode::Num9) applyPurchase(ShopAction::Continue);
                } else if (state == GameState::Wave) {
                    if (key->scancode == sf::Keyboard::Scancode::Backspace) state = GameState::Menu;
                }
            }

            // mouse click in menu
            if (state == GameState::Menu) {
                if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mb->button == sf::Mouse::Button::Left) {
                        const sf::Vector2f mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                        for (auto& b : buttons) {
                            if (b.rect.getGlobalBounds().contains(mp)) {
                                LevelConfig loaded;
                                std::string err;
                                if (!loadLevelFromJsonFile(b.filename, loaded, err)) lastError = err;
                                else {
                                    lastError.clear();
                                    lvl = loaded;
                                    bg = lvl.backgroundColor;
                                    resetRun();
                                    startWave();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f);
        const auto ws = window.getSize();

        // =====================================================
        // UPDATE
        // =====================================================
        if (state == GameState::Wave) {
            // wave timer
            waveTimeLeft -= dt;
            if (waveTimeLeft <= 0.f) {
                waveTimeLeft = 0.f;
                enterShop();
            }

            // player movement (real-time input)
            sf::Vector2f move(0.f, 0.f);
            float ms = lvl.playerMoveSpeed;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Left))  move.x -= ms * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Right)) move.x += ms * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Up))    move.y -= ms * dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Down))  move.y += ms * dt;

            player.move(move);

            // clamp player
            {
                auto p = player.getPosition();
                p.x = std::clamp(p.x, 40.f, ws.x - 40.f);
                p.y = std::clamp(p.y, 80.f, ws.y - 80.f);
                player.setPosition(p);
            }

            // spawn enemies with scaling
            float interval = spawnIntervalNow();
            spawnTimer += dt;
            while (spawnTimer >= interval) {
                spawnTimer -= interval;
                std::uniform_real_distribution<float> xdist(40.f, ws.x - 40.f);
                enemies.push_back(makeEnemy({xdist(rng), -60.f}, lvl.enemySpeed * upE_SpeedMult));
            }

            // move enemies + apply escape damage
            for (auto& e : enemies) e.shape.move(e.vel * dt);
            for (int i = 0; i < (int)enemies.size();) {
                if (enemies[i].shape.getPosition().y > (float)ws.y + 80.f) {
                    enemies.erase(enemies.begin() + i);
                    hpCur = std::max(0, hpCur - lvl.enemyEscapeDamage);
                    continue;
                }
                ++i;
            }

            if (hpCur <= 0) {
                state = GameState::Menu;
            }

            // helpers partition movement
            ensureHelpers();
            int N = helperCount;
            if (N > 0) {
                float baseY = (float)ws.y - 30.f;
                for (int i = 0; i < N; ++i) {
                    float left = (ws.x * (float)i) / (float)N;
                    float right = (ws.x * (float)(i + 1)) / (float)N;
                    float mid = (left + right) * 0.5f;

                    // furthest-down enemy in this partition
                    int bestIdx = -1;
                    float bestY = -1e9f;
                    for (int ei = 0; ei < (int)enemies.size(); ++ei) {
                        float ex = enemies[ei].shape.getPosition().x;
                        float ey = enemies[ei].shape.getPosition().y;
                        if (ex >= left && ex < right) {
                            if (ey > bestY) { bestY = ey; bestIdx = ei; }
                        }
                    }

                    float targetX = (bestIdx != -1) ? enemies[bestIdx].shape.getPosition().x : mid;

                    float hs = (lvl.playerMoveSpeed * 0.5f) * helpers[i].speedFactor;
                    float dx = targetX - helpers[i].x;
                    float dir = (dx > 0.f) ? 1.f : (dx < 0.f ? -1.f : 0.f);
                    float step = hs * dt;
                    if (std::abs(dx) < step) helpers[i].x = targetX;
                    else helpers[i].x += dir * step;

                    helpers[i].x = std::clamp(helpers[i].x, left + 20.f, right - 20.f);
                    helpers[i].shape.setPosition({helpers[i].x, baseY});
                }
            }

            // player shooting (hold space)
            float sps = lvl.playerShotsPerSecond * upP_AttackMult;
            float shotInterval = 1.f / std::max(0.01f, sps);
            int playerPierce = 1 + upP_Piercing;
            float bs = lvl.bulletSpeed * upP_BulletSpeedMult;

            shootTimer += dt;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space)) {
                while (shootTimer >= shotInterval) {
                    shootTimer -= shotInterval;
                    bullets.push_back(makeBullet(
                        player.getPosition() + sf::Vector2f(0.f, -30.f),
                        bs,
                        upP_Homing,
                        playerPierce,
                        upP_Homing ? sf::Color(255,200,80) : sf::Color::Cyan
                    ));
                }
            }

            // helper shooting rules
            if (!helpers.empty()) {
                float baseHelperShots = std::max(0.2f, lvl.playerShotsPerSecond * 0.7f);
                float helperShots = baseHelperShots * upH_FireRateMult;
                float helperInterval = 1.f / std::max(0.01f, helperShots);
                int helperPierce = 1 + upH_Piercing;
                float helperBulletSpeed = lvl.bulletSpeed * 0.85f;

                int HN = (int)helpers.size();
                for (int i = 0; i < HN; ++i) {
                    float left = (ws.x * (float)i) / (float)HN;
                    float right = (ws.x * (float)(i + 1)) / (float)HN;

                    int bestIdx = -1;
                    float bestY = -1e9f;
                    for (int ei = 0; ei < (int)enemies.size(); ++ei) {
                        float ex = enemies[ei].shape.getPosition().x;
                        float ey = enemies[ei].shape.getPosition().y;
                        if (ex >= left && ex < right) {
                            if (ey > bestY) { bestY = ey; bestIdx = ei; }
                        }
                    }

                    bool canShoot = false;
                    if (upH_Homing) {
                        canShoot = true; // constant shooting
                    } else if (bestIdx != -1) {
                        float ex = enemies[bestIdx].shape.getPosition().x;
                        float hx = helpers[i].shape.getPosition().x;
                        canShoot = (std::abs(hx - ex) <= 18.f);
                    }

                    helpers[i].shootTimer += dt;

                    if (canShoot) {
                        while (helpers[i].shootTimer >= helperInterval) {
                            helpers[i].shootTimer -= helperInterval;
                            bullets.push_back(makeBullet(
                                helpers[i].shape.getPosition() + sf::Vector2f(0.f, -18.f),
                                helperBulletSpeed,
                                upH_Homing,
                                helperPierce,
                                sf::Color(120,160,255)
                            ));
                        }
                    } else {
                        helpers[i].shootTimer = std::min(helpers[i].shootTimer, helperInterval);
                    }
                }
            }

            // homing steering + bullet movement
            for (auto& b : bullets) {
                if (b.homing && !enemies.empty()) {
                    sf::Vector2f bp = b.shape.getPosition();
                    int best = 0;
                    float bestD2 = 1e30f;
                    for (int i = 0; i < (int)enemies.size(); ++i) {
                        sf::Vector2f ep = enemies[i].shape.getPosition();
                        sf::Vector2f d = ep - bp;
                        float d2 = d.x*d.x + d.y*d.y;
                        if (d2 < bestD2) { bestD2 = d2; best = i; }
                    }
                    sf::Vector2f dirV = normalize(enemies[best].shape.getPosition() - bp);
                    b.vel = dirV * b.speed;
                } else {
                    b.vel = {0.f, -b.speed};
                }
                b.shape.move(b.vel * dt);
            }

            // remove bullets off map (correct lambda)
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
                                         [&](const Bullet& b) {
                                             sf::Vector2f p = b.shape.getPosition();
                                             return (p.y < -120.f || p.y > (float)ws.y + 120.f ||
                                                     p.x < -120.f || p.x > (float)ws.x + 120.f);
                                         }),
                          bullets.end());

            // collisions + piercing
            for (size_t bi = 0; bi < bullets.size();) {
                bool removedBullet = false;
                for (size_t ei = 0; ei < enemies.size();) {
                    if (bullets[bi].shape.getGlobalBounds().findIntersection(enemies[ei].shape.getGlobalBounds())) {
                        enemies.erase(enemies.begin() + (long)ei);
                        addKill();

                        bullets[bi].pierce -= 1;
                        if (bullets[bi].pierce <= 0) {
                            bullets.erase(bullets.begin() + (long)bi);
                            removedBullet = true;
                            break;
                        }
                        continue;
                    }
                    ++ei;
                }
                if (!removedBullet) ++bi;
            }
        }

        // =====================================================
        // DRAW
        // =====================================================
        if (state == GameState::Wave) window.clear(bg);
        else window.clear(sf::Color(10,10,18));

        if (state == GameState::Menu) {
            const sf::Vector2f mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            if (fontLoaded) {
                drawCenteredText(window, font, "Select difficulty (JSON)", 34,
                                 {ws.x*0.5f, ws.y*0.18f}, sf::Color::White);
                drawCenteredText(window, font, "1/2/3 or click", 18,
                                 {ws.x*0.5f, ws.y*0.24f}, sf::Color(210,210,225));
            }
            for (auto& b : buttons) {
                bool hov = b.rect.getGlobalBounds().contains(mp);
                b.rect.setFillColor(hov ? sf::Color(60,60,85) : sf::Color(40,40,55));
                window.draw(b.rect);
                if (fontLoaded) drawCenteredText(window, font, b.label, 26, b.rect.getPosition(), sf::Color::White);
            }
        }

        if (state == GameState::Wave) {
            for (auto& e : enemies) window.draw(e.shape);
            for (auto& b : bullets) window.draw(b.shape);
            window.draw(player);
            for (auto& h : helpers) window.draw(h.shape);

            // LVL bar top middle
            const float barW=360.f, barH=22.f;
            const float barX=ws.x*0.5f - barW*0.5f, barY=10.f;
            lvlBarBg.setPosition({barX, barY});
            float fillRatio = (float)killsProgress / (float)lvl.killsPerLevel;
            fillRatio = std::clamp(fillRatio, 0.f, 1.f);
            lvlBarFill.setPosition({barX, barY});
            lvlBarFill.setSize({barW*fillRatio, barH});
            window.draw(lvlBarBg);
            window.draw(lvlBarFill);

            // HP bar top left
            const float hpX=12.f, hpY=12.f;
            hpBarBg.setPosition({hpX, hpY});
            float hpRatio = (float)hpCur / (float)hpMax;
            hpRatio = std::clamp(hpRatio, 0.f, 1.f);
            hpBarFill.setPosition({hpX, hpY});
            hpBarFill.setSize({260.f*hpRatio, 20.f});
            window.draw(hpBarBg);
            window.draw(hpBarFill);

            if (fontLoaded) {
                drawCenteredText(window, font, "LVL " + std::to_string(playerLVL), 18,
                                 {ws.x*0.5f, barY + barH*0.5f}, sf::Color::White);

                int secLeft = (int)std::ceil(waveTimeLeft);
                drawCenteredText(window, font, "Next shop in: " + std::to_string(secLeft) + "s", 18,
                                 {ws.x*0.5f, barY + 40.f}, sf::Color(220,220,230));

                drawCenteredText(window, font, std::to_string(hpCur)+"/"+std::to_string(hpMax), 14,
                                 {hpX+130.f, hpY+10.f}, sf::Color::White);

                sf::Text wt(font, "WAVE " + std::to_string(waveNumber), 18);
                wt.setFillColor(sf::Color(230,230,240));
                auto bb = wt.getLocalBounds();
                wt.setPosition({ws.x - bb.size.x - 14.f, 10.f});
                window.draw(wt);
            }
        }

        if (state == GameState::Shop) {
            // Layout: shop 4/5 top, inventory 1/5 bottom
            const float invHeight = ws.y * 0.2f;
            const float shopHeight = ws.y - invHeight;
            const float colW = ws.x / 3.f;

            sf::RectangleShape shopBg({(float)ws.x, shopHeight});
            shopBg.setFillColor(sf::Color(15,15,24));
            window.draw(shopBg);

            sf::RectangleShape invBg({(float)ws.x, invHeight});
            invBg.setPosition({0.f, shopHeight});
            invBg.setFillColor(sf::Color(12,12,20));
            window.draw(invBg);

            sf::RectangleShape divider({(float)ws.x, 2.f});
            divider.setPosition({0.f, shopHeight});
            divider.setFillColor(sf::Color(90,90,110));
            window.draw(divider);

            // Build boxed upgrades (same-size boxes)
            std::vector<ShopItem> items;
            items.reserve(24);

            const float padX = 14.f;
            const float boxW = colW - 2*padX;
            const float boxH = 46.f;
            const float startY = 110.f;
            const float gapY = 12.f;

            auto pushBox = [&](int col, int row, const std::string& label, int cost, ShopAction action) {
                float x = col * colW + padX;
                float y = startY + row * (boxH + gapY);
                items.push_back(makeItem(x, y, boxW, boxH, label, cost, action));
            };

            // Player column
            pushBox(0,0,"Attack speed", C_P_ATK, ShopAction::P_AttackSpeed);
            pushBox(0,1,"Piercing",     C_P_PIER, ShopAction::P_Piercing);
            pushBox(0,2,"Bullet speed", C_P_BSPD, ShopAction::P_BulletSpeed);
            pushBox(0,3, upP_Homing ? "Homing (UNLOCKED)" : "Homing", C_P_HOM, ShopAction::P_Homing);

            // Enemy column
            pushBox(1,0,"Reduce enemy speed", C_E_SLOW, ShopAction::E_Slow);
            pushBox(1,1,"Increase spawn rate", C_E_SPAWN, ShopAction::E_SpawnFaster);

            // Helper column
            pushBox(2,0,"Buy helper (" + std::to_string(helperCount) + "/5)", C_H_BUY, ShopAction::H_Buy);
            pushBox(2,1,"Helper fire rate", C_H_RATE, ShopAction::H_FireRate);
            pushBox(2,2,"Helper piercing",  C_H_PIER, ShopAction::H_Piercing);
            pushBox(2,3, upH_Homing ? "Helper homing (UNLOCKED)" : "Helper homing", C_H_HOM, ShopAction::H_Homing);

            // Continue button
            items.push_back(makeItem(ws.x*0.5f - 160.f, shopHeight - 70.f, 320.f, 50.f,
                                     "CONTINUE", 0, ShopAction::Continue, sf::Color(30,55,40)));

            // Hover & click
            const sf::Vector2f mp = window.mapPixelToCoords(sf::Mouse::getPosition(window));

            for (auto& it : items) {
                bool hov = it.rect.getGlobalBounds().contains(mp);
                it.rect.setOutlineColor(hov ? sf::Color(200,200,240) : sf::Color(120,120,160));
                window.draw(it.rect);

                if (fontLoaded) {
                    sf::Text tl(font, it.label, 16);
                    tl.setFillColor(sf::Color(235,235,245));
                    tl.setPosition({it.rect.getPosition().x + 10.f, it.rect.getPosition().y + 12.f});
                    window.draw(tl);

                    if (it.cost > 0) {
                        sf::Text tc(font, std::to_string(it.cost) + " LVL", 16);
                        tc.setFillColor(sf::Color(200,240,210));
                        auto bb = tc.getLocalBounds();
                        tc.setPosition({it.rect.getPosition().x + it.rect.getSize().x - bb.size.x - 10.f,
                                        it.rect.getPosition().y + 12.f});
                        window.draw(tc);
                    }
                }
            }

            bool nowDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
            if (!mouseHeld && nowDown) {
                mouseHeld = true;
                for (auto& it : items) {
                    if (it.rect.getGlobalBounds().contains(mp)) {
                        applyPurchase(it.action);
                        break;
                    }
                }
            }
            if (!nowDown) mouseHeld = false;

            // Inventory bottom: icons + multiline stats
            iconPlayer.setPosition({colW*0.5f, shopHeight + 30.f});
            iconEnemy.setPosition({colW*1.5f, shopHeight + 30.f});
            iconHelper.setPosition({colW*2.5f, shopHeight + 30.f});
            window.draw(iconPlayer); window.draw(iconEnemy); window.draw(iconHelper);

            if (fontLoaded) {
                float shots = lvl.playerShotsPerSecond * upP_AttackMult;
                std::string invPlayerText =
                    "Attack: " + std::to_string(shots) + "\n" +
                    "Pierce: " + std::to_string(1+upP_Piercing) + "\n" +
                    "BulletSpd: x" + std::to_string(upP_BulletSpeedMult) + "\n" +
                    "Homing: " + std::string(upP_Homing ? "YES" : "NO");

                std::string invEnemyText =
                    "SpeedMult: x" + std::to_string(upE_SpeedMult) + "\n" +
                    "SpawnMult: x" + std::to_string(upE_SpawnMult) + "\n" +
                    "CurSpawn: " + std::to_string(spawnIntervalNow());

                std::string invHelperText =
                    "Count: " + std::to_string(helperCount) + "/5\n" +
                    "FireMult: x" + std::to_string(upH_FireRateMult) + "\n" +
                    "Pierce: " + std::to_string(1+upH_Piercing) + "\n" +
                    "Homing: " + std::string(upH_Homing ? "YES" : "NO");

                sf::Text tp(font, invPlayerText, 14);
                tp.setFillColor(sf::Color(220,220,235));
                tp.setPosition({10.f, shopHeight + 52.f});
                window.draw(tp);

                sf::Text te(font, invEnemyText, 14);
                te.setFillColor(sf::Color(220,220,235));
                te.setPosition({colW + 10.f, shopHeight + 52.f});
                window.draw(te);

                sf::Text th(font, invHelperText, 14);
                th.setFillColor(sf::Color(220,220,235));
                th.setPosition({2*colW + 10.f, shopHeight + 52.f});
                window.draw(th);

                drawCenteredText(window, font, "LVL " + std::to_string(playerLVL), 18,
                                 {ws.x*0.5f, 40.f}, sf::Color(220,220,235));
            }
        }

        window.display();
    }

    return 0;
}
