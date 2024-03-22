#include <raylib.h>
#include <string>

struct Pos {
    int x;
    int y;

    Pos(int x, int y) : x(x), y(y) {}

    Pos() : x(-1), y(-1) {}

    bool isValid() const {
        return x != -1 && y != -1;
    }
};

struct Vec2 {
    float x;
    float y;

    Vec2(float x, float y) : x(x), y(y) {}

    Vec2() : x(0), y(0) {}

    Vec2 operator-(const Vec2 &other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2 normalized() const {
        float length = std::sqrt(x * x + y * y);
        return Vec2(x / length, y / length);
    }

    Vec2 operator*(const float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator+(const Vec2 &other) const {
        return Vec2(x + other.x, y + other.y);
    }
};

Vec2 moveTowards(const Vec2& current, const Vec2& target, float maxDistanceDelta) {
    Vec2 directionVector = target - current;
    Vec2 normalizedDirection = directionVector.normalized();
    float distanceToMove = maxDistanceDelta;
    const Vec2 scaled = normalizedDirection * distanceToMove;
    return current + scaled;
}

float distance(const Vec2& l, const Vec2& r) {
    return sqrt((l.x - r.x) * (l.x - r.x) + (l.y - r.y) * (l.y - r.y));
}

struct Cell {
    int value;
    bool merged;
    bool isHidden;

    Cell(int value, bool merged) : value(value), merged(merged), isHidden(false) {}

    Cell() : value(0), merged(false), isHidden(false) {}

    bool isZero() const {
        return value == 0;
    }
};

struct Colors {
    Color background;
    Color text;
};

Color getColor(int value) {
    if (value <= 4) {
        return LIGHTGRAY;
    } else if (value <= 8) {
        return CLITERAL(Color){252, 170, 110, 255};
    } else if (value <= 16) {
        return CLITERAL(Color){255, 140, 84, 255};
    } else if (value <= 32) {
        return CLITERAL(Color){255, 115, 85, 255};
    } else if (value <= 64) {
        return CLITERAL(Color){255, 79, 45, 255};
    } else {
        return CLITERAL(Color){242, 190, 0, 255};
    }
}

struct Animation {
    Vec2 current;
    Vec2 target;
    std::string line;
    Color color;
    bool isActive;
    Pos pos;

    Animation()
            : current(), target(), line(), color(), isActive(false), pos() {}

    Animation(
            const Vec2 &current,
            const Vec2 &target,
            const std::string &line,
            Color color,
            Pos pos
    ) : current(current), target(target), line(line), color(color), isActive(true),
        pos(pos) {}
};

struct AppearAnimation {
    Pos pos;
    Vec2 coord;
    std::string line;
    float timeLeft;
    float delay;
    Color color;
    float duration;
    bool isActive;

    AppearAnimation(
            const Pos &pos,
            const Vec2 &coord,
            const std::string &line,
            float timeLeft,
            float delay,
            Color color
    )
            : pos(pos), coord(coord), line(line), timeLeft(timeLeft), delay(delay), color(color), duration(timeLeft),
              isActive(true) {}

    AppearAnimation() : pos(), coord(), line(), timeLeft(0), delay(0), color(), duration(0), isActive(false) {}
};

const int fieldSize = 4;

int score = 0;
bool gameOver = false;

int width = 800;
int height = 800;

Cell field[fieldSize][fieldSize];
Animation moveAnimations[fieldSize * fieldSize];
Animation joinAnimations[fieldSize * fieldSize];
AppearAnimation appearAnimations[fieldSize];

float moveSpeed = 2000;

void drawGrid() {
    int cellWidth = width / fieldSize;
    int cellHeight = height / fieldSize;
    for (int i = 0; i < fieldSize; i++) {
        DrawLine(cellWidth * i, 0, cellWidth * i, height, LIGHTGRAY);
    }
    for (int i = 0; i < fieldSize; i++) {
        DrawLine(0, cellHeight * i, width, cellHeight * i, LIGHTGRAY);
    }
}

Vec2 getCoord(std::string line, Pos pos) {
    int cellWidth = width / fieldSize;
    int cellHeight = height / fieldSize;
    int fontSize = cellWidth / 4;
    int charWidth = fontSize / 2;

    int cellCenterX = pos.x * cellWidth + cellWidth / 2 - (charWidth * line.size()) / 2 - 10;
    int cellCenterY = pos.y * cellHeight + cellHeight / 2 - fontSize / 2;

    return Vec2(cellCenterX, cellCenterY);
}

void drawField(float deltaTime) {
    int cellWidth = width / fieldSize;
    int fontSize = cellWidth / 4;

    for (int y = 0; y < fieldSize; y++) {
        for (int x = 0; x < fieldSize; x++) {
            Cell cell = field[x][y];
            if (cell.isZero() || cell.isHidden) {
                continue;
            }
            std::string line = std::to_string(cell.value);
            Vec2 coord = getCoord(line, {x, y});
            DrawText(line.c_str(), coord.x, coord.y, fontSize, getColor(cell.value));
        }
    }

    for (int i = 0; i < fieldSize * fieldSize; ++i) {
        Animation &animation = moveAnimations[i];
        if (!animation.isActive) {
            continue;
        }
        std::string &line = animation.line;
        DrawText(line.c_str(), animation.current.x, animation.current.y, fontSize, animation.color);
        animation.current = moveTowards(animation.current, animation.target, deltaTime * moveSpeed);
        if (distance(animation.current, animation.target) < cellWidth / 8) {
            animation.isActive = false;
            field[animation.pos.x][animation.pos.y].isHidden = false;
        }
    }

    for (int i = 0; i < fieldSize * fieldSize; ++i) {
        Animation &animation = joinAnimations[i];
        if (!animation.isActive) {
            continue;
        }
        std::string &line = animation.line;
        DrawText(line.c_str(), animation.current.x, animation.current.y, fontSize, animation.color);
        animation.current = moveTowards(animation.current, animation.target, deltaTime * moveSpeed);
        if (distance(animation.current, animation.target) < cellWidth / 8) {
            animation.isActive = false;
        }
    }

    for (int i = 0; i < fieldSize; ++i) {
        AppearAnimation &animation = appearAnimations[i];
        if (!animation.isActive) {
            continue;
        }
        if (animation.delay > 0) {
            animation.delay -= deltaTime;
            continue;
        }
        std::string &line = animation.line;
        int afontSize = fontSize + fontSize * sin(animation.timeLeft / animation.duration * PI) / 2;
        DrawText(line.c_str(), animation.coord.x, animation.coord.y, afontSize, animation.color);
        animation.timeLeft -= deltaTime;
        if (animation.timeLeft <= 0) {
            animation.isActive = false;
            field[animation.pos.x][animation.pos.y].isHidden = false;
        }
    }
}

Pos getFreePos() {
    Pos pos[fieldSize * fieldSize];
    int count = 0;
    for (int y = 0; y < fieldSize; y++) {
        for (int x = 0; x < fieldSize; x++) {
            Cell value = field[x][y];
            if (value.isZero()) {
                pos[count] = Pos(x, y);
                count++;
            }
        }
    }
    if (count == 0) {
        return {};
    }
    int index = rand() % count;
    return pos[index];
}

void checkIfGameOver() {
    for (int y = 0; y < fieldSize; y++) {
        for (int x = 0; x < fieldSize; x++) {
            if (x + 1 < fieldSize && field[x][y].value == field[x + 1][y].value) {
                return;
            }
            if (x - 1 > 0 && field[x][y].value == field[x - 1][y].value) {
                return;
            }
            if (y + 1 < fieldSize && field[x][y].value == field[x][y + 1].value) {
                return;
            }
            if (y - 1 > 0 && field[x][y].value == field[x][y - 1].value) {
                return;
            }
        }
    }
    gameOver = true;
}

void addMoveAnimation(Animation animation) {
    for (int i = 0; i < fieldSize * fieldSize; ++i) {
        if (moveAnimations[i].isActive) { continue; }
        moveAnimations[i] = animation;
        field[animation.pos.x][animation.pos.y].isHidden = true;
        return;
    }
}

void addJoinAnimation(Animation animation) {
    for (int i = 0; i < fieldSize * fieldSize; ++i) {
        if (joinAnimations[i].isActive) { continue; }
        joinAnimations[i] = animation;
        return;
    }
}

void addAppearAnimation(AppearAnimation animation) {
    for (int i = 0; i < fieldSize; ++i) {
        if (appearAnimations[i].isActive) { continue; }
        appearAnimations[i] = animation;
        return;
    }
}

void addRandomNumber() {
    Pos freePos = getFreePos();
    if (!freePos.isValid()) {
        gameOver = true;
        return;
    }
    int value = rand() % 2 == 0 ? 2 : 4;
    field[freePos.x][freePos.y].value = value;
    field[freePos.x][freePos.y].isHidden = true;
    std::string line = std::to_string(value);
    addAppearAnimation(AppearAnimation(freePos, getCoord(line, freePos), line, 0.3, 0.3, getColor(value)));
    if (!getFreePos().isValid()) {
        checkIfGameOver();
    }
}

bool moveCell(Pos pos, int offsetX, int offsetY, int value) {
    int moved = 0;
    Pos nextPos = pos;
    Animation animation;
    while (true) {
        Pos lastPos = nextPos;
        nextPos.x += offsetX;
        nextPos.y += offsetY;
        if (nextPos.y < 0 || nextPos.y >= fieldSize || nextPos.x < 0 || nextPos.x >= fieldSize) {
            break;
        }
        if (field[nextPos.x][nextPos.y].isZero()) {
            field[nextPos.x][nextPos.y].value = value;
            field[nextPos.x][nextPos.y].merged = field[lastPos.x][lastPos.y].merged;
            field[lastPos.x][lastPos.y].value = 0;
            std::string line = std::to_string(value);
            animation = Animation(
                    getCoord(line, pos), getCoord(line, nextPos), line, getColor(value), nextPos
            );
            moved = true;
            continue;
        } else if (!field[nextPos.x][nextPos.y].merged && !field[lastPos.x][lastPos.y].merged &&
                   field[nextPos.x][nextPos.y].value == value) {
            field[nextPos.x][nextPos.y].value = value * 2;
            field[lastPos.x][lastPos.y].value = 0;
            field[nextPos.x][nextPos.y].merged = true;
            score += value * 2;
            std::string line = std::to_string(value * 2);
            addJoinAnimation(
                    Animation(
                            getCoord(line, pos), getCoord(line, nextPos), line, getColor(value * 2), nextPos
                    )
            );
            return true;
        } else {
            break;
        }
    }
    if (animation.isActive) {
        addMoveAnimation(animation);
    }
    return moved;
}

bool move(int offsetX, int offsetY) {
    bool moved = false;
    int startY = offsetY > 0 ? fieldSize - 1 : 0;
    int startX = offsetX > 0 ? fieldSize - 1 : 0;
    int stepX = offsetX > 0 ? -1 : 1;
    int stepY = offsetY > 0 ? -1 : 1;
    for(int y = startY; offsetY > 0 ? y >= 0 : y < fieldSize; y += stepY) {
        for (int x = startX; offsetX > 0 ? x >= 0 : x < fieldSize; x += stepX) {
            Cell cell = field[x][y];
            if(cell.isZero()) {
                continue;
            }
            if(moveCell(Pos(x, y), offsetX, offsetY, cell.value)) {
                moved = true;
            }
        }
    }
    return moved;
}

void prepareForNextMove() {
    for (int y = 0; y < fieldSize; ++y) {
        for (int x = 0; x < fieldSize; ++x) {
            field[x][y].merged = false;
        }
    }
}

bool canMove() {
    for(int i = 0; i < fieldSize; i++) {
        if(appearAnimations[i].isActive) { return false; }
    }
    for(int i = 0; i < fieldSize * fieldSize; i++) {
        if(moveAnimations[i].isActive) { return false; }
    }
    for(int i = 0; i < fieldSize * fieldSize; i++) {
        if(joinAnimations[i].isActive) { return false; }
    }
    return true;
}

void update(int offsetX, int offsetY) {
    if(!canMove()) { return; }
    bool moved = false;
    for (int i = 0; i < fieldSize; ++i) {
        if (!move(offsetX, offsetY)) {
            break;
        }
        moved = true;
    }
    if (moved) {
        addRandomNumber();
    }
    prepareForNextMove();
}

void reset() {
    for (int y = 0; y < fieldSize; ++y) {
        for (int x = 0; x < fieldSize; ++x) {
            field[x][y].value = 0;
        }
    }
    gameOver = false;
    addRandomNumber();
    addRandomNumber();
}

int main() {
    srand(time(nullptr));
    InitWindow(width, height, "2048 GAME");
    SetWindowState(FLAG_VSYNC_HINT);
    SetTraceLogLevel(LOG_DEBUG);
    reset();
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        // DrawFPS(0, 0);
        float deltaTime = GetFrameTime();
        drawGrid();
        drawField(deltaTime);
        if (IsKeyPressed(KEY_R)) {
            reset();
        }
        if (gameOver) {
            DrawText("GAME OVER!", 165, height / 2 - 120, 70, YELLOW);
            DrawText("PRESS R TO RESTART", 150, height / 2, 40, YELLOW);
        } else {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                update(0, -1);
            }
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                update(0, 1);
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                update(-1, 0);
            }
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                update(1, 0);
            }
        }
        EndDrawing();
    }
    CloseWindow();
}