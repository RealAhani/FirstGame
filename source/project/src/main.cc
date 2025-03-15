// goal: check an expresion in runtime if not android
// note: pass a true condition that you need like percent>100 fail
[[maybe_unused]]
void checkAtRuntime(bool faildCondition, std::string_view const & errMsg) noexcept
{
#ifdef DEBUG
    if (myproject::cmake::platform != "Android" && faildCondition)
    {
        std::cerr << errMsg << '\n';
        assert(!faildCondition);
    }
#endif  // DEBUG
}

// goal: calculate position and size of object based on desired percentage of parent
// cordinate
// note: x and y should be 0<n<100 note: widthPercent and heightPercent
// calculate based on reminding space of parent width and height so if you say 100 it
// fill reminding space of width
[[nodiscard]] [[maybe_unused]]
Rectangle placeRelative(Rectangle const & parentInfo,
                        uint8_t const     xPercent,
                        uint8_t const     yPercent,
                        uint8_t const     widthPercent,
                        uint8_t const     heightPercent) noexcept
{
    // bounds checking on input args (debug only)
    checkAtRuntime((xPercent > 100 || yPercent > 100 || widthPercent > 100 ||
                    heightPercent > 100),
                   "Placement Relative inputs should be 0<n<100 in percentage "
                   "(int)");

    float const newX = static_cast<float>(parentInfo.x + parentInfo.width) *
                       xPercent / 100.f;
    float const newY = static_cast<float>(parentInfo.y + parentInfo.height) *
                       yPercent / 100.f;
    float const newW = (static_cast<float>(parentInfo.width) - newX) * widthPercent / 100.f;
    float const newH = (static_cast<float>(parentInfo.height) - newY) *
                       heightPercent / 100.f;
    return Rectangle {.x = newX, .y = newY, .width = newW, .height = newH};
}

struct GridInfo
{
    Rectangle rect;
    Vector2   cellSize;
    uint8_t   columnCount;
    uint8_t   rowCount;
};

// struct InputInfo{
//     Vector2 cordinate;
//     float time;
// };

// struct InputRegister
// {
//     float delayRegister {};
//     bool  canRegister;
// };

// struct WindowInfo
// {
//     std::string_view name;
//     Rectangle winRect;
//     gridInfo grid;
//     float delatTime;
//     uint8_t fps;
//     bool isEnd;
// };

// goal: initilizer function for gridInfo object
// note: col and row should be bigger than 2
[[nodiscard]] [[maybe_unused]]
inline GridInfo createGridInfo(Rectangle const & gridRect,
                               uint8_t const     columnCount = 2,
                               uint8_t const     rowCount    = 2) noexcept
{
    // col and row should be bigger than 2X2
    // bounds checking on input args (debug only)
    checkAtRuntime((columnCount < 2 || rowCount < 2),
                   "column and row should be bigger than 2");
    return GridInfo {.rect        = gridRect,
                     .cellSize    = Vector2 {gridRect.width / columnCount,
                                          gridRect.height / rowCount},
                     .columnCount = columnCount,
                     .rowCount    = rowCount};
}

// goal: draw grid on screen in real time with grid info
// note: it can be slow if its a static grid use genGridTexture
[[maybe_unused]]
void drawGrid(GridInfo const & grid, Color const lineColor = WHITE) noexcept
{
    // draw in between lines based on col and row
    // drawing row lines
    float yOffset {grid.rect.y};
    for (unsigned int i {}; i <= grid.rowCount; ++i)
    {
        DrawLineV(Vector2 {grid.rect.x, yOffset},
                  Vector2 {grid.rect.width + grid.rect.x, yOffset},
                  lineColor);
        yOffset += grid.cellSize.y;
        // yOffset = std::clamp<float>(yOffset, 0, static_cast<float>(gridHeight));
    }
    // drawing column lines
    float xOffset {grid.rect.x};
    for (unsigned int i {}; i <= grid.columnCount; ++i)
    {
        DrawLineV(Vector2 {xOffset, grid.rect.y},
                  Vector2 {xOffset, grid.rect.height + grid.rect.y},
                  lineColor);
        xOffset += grid.cellSize.x;
        // xOffset = std::clamp<float>(xOffset, startPosX, static_cast<float>(gridWidth));
    }
}
// goal: crete a grid texture
// note: its more performance friendly for static grid
// TODO: make it lazy load
[[nodiscard]] [[maybe_unused]]
Texture2D genGridTexture(GridInfo const & grid,
                         Color const      lineColor       = WHITE,
                         Color const      backgroundColor = BLACK) noexcept
{
    Image img = GenImageColor(static_cast<int>(grid.rect.width),
                              static_cast<int>(grid.rect.height),
                              backgroundColor);
    // draw in between lines based on col and row
    // drawing row lines
    float yOffset {grid.rect.y};
    for (unsigned int i {}; i <= grid.rowCount; ++i)
    {
        ImageDrawLine(&img,
                      static_cast<int>(grid.rect.x),
                      static_cast<int>(yOffset),
                      static_cast<int>(grid.rect.width + grid.rect.x),
                      static_cast<int>(yOffset),
                      lineColor);
        yOffset += grid.cellSize.y;
    }
    // draw
    float xOffset {grid.rect.x};
    for (unsigned int i {}; i <= grid.columnCount; ++i)
    {
        ImageDrawLine(&img,
                      static_cast<int>(xOffset),
                      static_cast<int>(grid.rect.y),
                      static_cast<int>(xOffset),
                      static_cast<int>(grid.rect.height + grid.rect.y),
                      lineColor);
        xOffset += grid.cellSize.x;
        // xOffset = std::clamp<float>(xOffset, startPosX, static_cast<float>(gridWidth));
    }
    Texture2D gridTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    return gridTexture;
}

[[nodiscard]] [[maybe_unused]]
Rectangle whereClicked(float const     offsetX,
                       float const     offsetY,
                       Vector2 const & position,
                       bool const      debugDraw = false) noexcept
{
    int const tempRemX = (static_cast<int>(position.x) / static_cast<int>(offsetX));
    int       x1 {};
    if (tempRemX == 1)
    {
        x1 = static_cast<int>(offsetX);
    }
    else if (tempRemX < 1)
    {
        x1 = 0;
    }
    else  // its zero
    {
        x1 = tempRemX * static_cast<int>(offsetX);
    }

    int       y1 {};
    int const tempRemY = (static_cast<int>(position.y) / static_cast<int>(offsetY));
    if (tempRemY == 1)
    {
        y1 = static_cast<int>(offsetY);
    }
    else if (tempRemY < 1)
    {
        y1 = 0;
    }
    else
    {
        y1 = tempRemY * static_cast<int>(offsetY);
    }
    if (debugDraw)
    {
        DrawRectangleRec(Rectangle {x1 / 1.f, y1 / 1.f, offsetX, offsetY}, RAYWHITE);
        DrawCircleLinesV(Vector2 {x1 / 1.f, y1 / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {x1 / 1.f, (y1 + offsetY) / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {(x1 + offsetX) / 1.f, y1 / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {(x1 + offsetX) / 1.f, (y1 + offsetY) / 1.f}, 10, RED);
        DrawCircleLinesV(position, 5, YELLOW);
    }
    return Rectangle {x1 / 1.f, y1 / 1.f, offsetX, offsetY};
}

}  // namespace RA_Util
namespace RA_Anim
{
constexpr int const animationFPS = 60;
// class Animation2D{
// };

// TODO: it need a texture that support zBuffer
struct AnimData
{
    Texture2D textureAnim;
    Rectangle rect;
    int const length;  // actual length is length -1
    int       counter;
    int       currentFrame;
    int       currentSpeed;
    int const defaultSpeed;
    int const speedMAX;
    int const speedMIN;
};

[[nodiscard]] [[maybe_unused]]
AnimData initAnim(char const* fileName, int const animLenght, int const speed, int const speedMax)
{
    char const* path = TextFormat("resource/%s", fileName);
    if (FileExists(path))
    {
        Texture2D const temp = LoadTexture(path);
        return AnimData {.textureAnim  = temp,
                         .rect         = {.x = 0,
                                          .y = 0,
                                          .width = static_cast<float>(temp.width / animLenght),
                                          .height = static_cast<float>(temp.height)},
                         .length       = animLenght,
                         .counter      = 0,
                         .currentFrame = 0,
                         .currentSpeed = speed,
                         .defaultSpeed = speed,
                         .speedMAX     = speedMax,
                         .speedMIN     = 1};
    }
    std::cerr << "Error: " << fileName << " not found\n";
    std::terminate();
}
[[maybe_unused]]
void cleanAnim(AnimData & data) noexcept
{
    UnloadTexture(data.textureAnim);
    data.rect = Rectangle {0, 0, 0, 0};
}
// goal: animation speed currection
// note: it works as a input output param refrence
// warning: internall function usage
[[maybe_unused]]
void limitSpeedAnim(AnimData & inOutData) noexcept
{
    if (inOutData.currentSpeed > inOutData.speedMAX)
        inOutData.currentSpeed = inOutData.speedMAX;
    else if (inOutData.currentSpeed < inOutData.speedMIN)
        inOutData.currentSpeed = inOutData.speedMIN;
}
// note: it works as a input output param refrence
[[maybe_unused]]
inline void increaseAnimSpeed(AnimData & inOutData) noexcept
{
    inOutData.currentSpeed += 1;
    limitSpeedAnim(inOutData);
}
// note: it works as a input output param refrence
[[maybe_unused]]
inline void decreaseAnimSpeed(AnimData & inOutData) noexcept
{
    inOutData.currentSpeed -= 1;
    limitSpeedAnim(inOutData);
}

// note: it works as a input output param refrence
[[maybe_unused]]
inline void resetAnimSpeed(AnimData & inOutData) noexcept
{
    inOutData.currentSpeed = inOutData.defaultSpeed;
}
// note: it works as a input output param refrence
// warning: internall function usage for updateAnim
[[maybe_unused]]
void resetAnim(AnimData & inOutData) noexcept
{
    inOutData.currentFrame = 0;
}
// TODO: zBuffer needed
[[maybe_unused]]
void renderAnim(AnimData const & data, Vector2 const & pos, Color const tint = WHITE) noexcept
{
    DrawTextureRec(data.textureAnim, data.rect, pos, tint);
}
// goal: get ready AnimData for render them in gameloop
[[maybe_unused]]
void updateAnim(AnimData & data) noexcept
{
    data.counter++;
    if (data.counter >= animationFPS / data.currentSpeed)
    {
        data.counter = 0;
        // speed currection
        limitSpeedAnim(data);
        data.currentFrame++;

        if (data.currentFrame > (data.length - 1))  // start animation frame is zero
            resetAnim(data);

        data.rect.x = static_cast<float>(data.currentFrame) *
                      static_cast<float>(data.textureAnim.width) /
                      static_cast<float>(data.length);
    }
}

}  // namespace RA_Anim
// Define the operator== function outside the class
}  // namespace

[[maybe_unused]]
bool operator==(Rectangle const & lhs, Rectangle const & rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    InitWindow(0, 0, "Test");
    ToggleFullscreen();
    auto const height   = GetScreenHeight();
    auto const width    = GetScreenWidth();
    auto const fps      = GetMonitorRefreshRate(0);
    int const  row      = 100;
    int const  column   = 100;
    auto const offsety  = static_cast<float>(height) / row;
    auto const offsetx  = static_cast<float>(width) / column;
    auto const gridRect = RA_Util::placeRelative(Rectangle {0,
                                                            0,
                                                            static_cast<float>(width),
                                                            static_cast<float>(height)},
                                                 20,
                                                 10,
                                                 70,
                                                 80);
    auto       gridinfo = RA_Util::createGridInfo(gridRect, column, row);
    gridinfo.rect.x     = 0.f;
    gridinfo.rect.y     = 0.f;
    Texture2D const gridTexture {RA_Util::genGridTexture(gridinfo, WHITE, BLACK)};
    std::vector<Rectangle> rects {};
    rects.reserve(1000);
    Vector2 mousePos {};
    SetTargetFPS(fps);

    bool     isEnd {false};
    bool     canRegister {false};
    Camera2D camera;
    camera.offset   = Vector2 {};
    camera.target   = Vector2 {};
    camera.rotation = 0;
    camera.zoom     = 1.f;
    RA_Anim::AnimData scarfyAnim {RA_Anim::initAnim("scarfy.png", 6, 8, 15)};
    Vector2           playerPos {};
    while (!WindowShouldClose() && !isEnd)
    {
        if (IsKeyPressed(KEY_ESCAPE))
        {
            isEnd = true;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            canRegister = true;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            canRegister = false;
        }
        if (canRegister)
        {
            mousePos = GetTouchPosition(0);
            Rectangle const temp {RA_Util::whereClicked(offsetx, offsety, mousePos)};
            if (std::find(rects.begin(), rects.end(), temp) == rects.end())
            {
                rects.emplace(std::begin(rects), temp);
                if (rects.size() > 550)
                {
                    rects.pop_back();
                }
            }
        }
        RA_Anim::updateAnim(scarfyAnim);

        ClearBackground(WHITE);
        BeginDrawing();
        BeginMode2D(camera);
        // drawGrid(gridinfo, RED);
        DrawTexture(gridTexture,
                    static_cast<int>(gridRect.x),
                    static_cast<int>(gridRect.y),
                    RED);
        DrawText(TextFormat("height : %d \n width : %d", height, width),
                 width / 2,
                 height / 2,
                 22,
                 RAYWHITE);
        for (auto const & rect : rects)
        {
            DrawRectangleRec(rect, BLUE);
        }
        RA_Anim::renderAnim(scarfyAnim, playerPos);
        EndDrawing();
    }
    CloseWindow();
    RA_Anim::cleanAnim(scarfyAnim);
    return 0;
}