namespace
{
using namespace std::string_literals;
using namespace std::string_view_literals;
namespace RA_Global
{
constexpr std::uint8_t const animationFPS = 60;
inline static constexpr std::string_view const
    texturePath = "resource/"sv;
}  // namespace RA_Global

namespace RA_Util
{
/*
 * @Goal: check an expresion in runtime if not android
 * @Note: pass a true condition that you need like percent>100 fail
 */
[[maybe_unused]]
auto checkAtRuntime(bool                   faildCondition,
                    std::string_view const errMsg) noexcept -> void
{
#ifdef DEBUG
    if (myproject::cmake::platform != "Android"sv && faildCondition)
    {
        std::cerr << errMsg << '\n';
        assert(!faildCondition);
    }
#endif  // DEBUG
}
/*
 *@Note: the parent x and y (origin) should be top-left e.x:(0,0)
 *@Note: widthPercent  and heightPercent is calculated based on parent w,h
 *@Warning: the width and height of the parent should be less than window size
 */
[[nodiscard]] [[maybe_unused]]
auto placeRelativeCenter(Rectangle const & parentInfo,
                         uint8_t const     widthPercent,
                         uint8_t const heightPercent) noexcept -> Rectangle
{

    // bounds checking on input args (debug only)
    checkAtRuntime((widthPercent > 100 || heightPercent > 100 ||
                    widthPercent == 0 || heightPercent == 0),
                   "Placement Relative inputs should be 1<n<100 "
                   "in percentage "
                   "(unsigned int)\n"sv);

    float const newX = parentInfo.x + (parentInfo.width / 2.f);
    float const newY = parentInfo.y + (parentInfo.height / 2.f);

    float const newW = parentInfo.width * widthPercent / 100.f;
    float const newH = parentInfo.height * heightPercent / 100.f;

    return Rectangle {.x      = newX - (newW / 2.f),
                      .y      = newY - (newH / 2.f),
                      .width  = newW,
                      .height = newH};
}
/*
 *@Goal: calculate position and size of object based on desired
 *percentage of parent cordinate note: x and y should be 0<n<100
 *@Note: widthPercent and heightPercent calculate based on
 *reminding space of parent width and height so if you say 100
 *it fill reminding space of width
 */
[[nodiscard]] [[maybe_unused]]
auto placeRelative(Rectangle const & parentInfo,
                   uint8_t const     xPercentOffset,
                   uint8_t const     yPercentOffset,
                   uint8_t const     wPercentReminded,
                   uint8_t const hPercentReminded) noexcept -> Rectangle
{
    // width and height percent reminded checking
    checkAtRuntime((wPercentReminded == 0 || hPercentReminded == 0),
                   "width and height percent should not be zero"
                   "in percentage "
                   "(unsigned int)\n"sv);

    // bounds checking on input args (debug only)
    checkAtRuntime((xPercentOffset > 100 || yPercentOffset > 100 ||
                    wPercentReminded > 100 || hPercentReminded > 100),
                   "Placement Relative inputs should be for \n x,y= "
                   "0<n<100 \nw,h= 1<n<100\n"
                   "in percentage "
                   "(unsigned int)\n"sv);

    float const newX = static_cast<float>(parentInfo.x + parentInfo.width) *
                       xPercentOffset / 100.f;
    float const newY = static_cast<float>(parentInfo.y + parentInfo.height) *
                       yPercentOffset / 100.f;
    return Rectangle {.x = newX,
                      .y = newY,
                      .width = (static_cast<float>(parentInfo.width) - newX) *
                               wPercentReminded / 100.f,
                      .height = (static_cast<float>(parentInfo.height) -
                                 newY) *
                                hPercentReminded / 100.f};
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

/*
 *
 *@Goal: initilizer function for gridInfo object
 *@Note: col and row should be bigger than 2
 */
[[nodiscard]] [[maybe_unused]]
inline auto createGridInfo(Rectangle const & gridRect,
                           uint8_t const     columnCount = 2,
                           uint8_t const rowCount = 2) noexcept -> GridInfo
{
    // col and row should be bigger than 2X2
    // bounds checking on input args (debug only)
    checkAtRuntime((columnCount < 2 || rowCount < 2),
                   "column and row should be bigger than 2\n"sv);
    return GridInfo {.rect = Rectangle {.x      = 0.f,
                                        .y      = 0.f,
                                        .width  = gridRect.width,
                                        .height = gridRect.height},
                     .cellSize = Vector2 {gridRect.width / columnCount,
                                          gridRect.height / rowCount},
                     .columnCount = columnCount,
                     .rowCount    = rowCount};
}
/*
 *@Goal: draw grid on screen in real time with grid info
 *@Note: it can be slow if its a static grid use genGridTexture
 */
[[maybe_unused]]
auto drawGrid(GridInfo const & grid, Color const lineColor = WHITE) noexcept
    -> void
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

/*
* @Goal: crete a grid texture
* @Note: its more performance friendly for static grid
 TODO: make it lazy load
*/
[[nodiscard]] [[maybe_unused]]
auto genGridTexture(GridInfo const & grid,
                    Color const      lineColor       = WHITE,
                    Color const      backgroundColor = BLACK) noexcept
    -> Texture2D
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
auto whereClicked(float const     offsetX,
                  float const     offsetY,
                  Vector2 const & position,
                  bool const debugDraw = false) noexcept -> Rectangle
{
    int const tempRemX = (static_cast<int>(position.x) /
                          static_cast<int>(offsetX));
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
    int const tempRemY = (static_cast<int>(position.y) /
                          static_cast<int>(offsetY));
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
        DrawRectangleRec(Rectangle {x1 / 1.f, y1 / 1.f, offsetX, offsetY},
                         RAYWHITE);
        DrawCircleLinesV(Vector2 {x1 / 1.f, y1 / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {x1 / 1.f, (y1 + offsetY) / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {(x1 + offsetX) / 1.f, y1 / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {(x1 + offsetX) / 1.f,
                                  (y1 + offsetY) / 1.f},
                         10,
                         RED);
        DrawCircleLinesV(position, 5, YELLOW);
    }
    return Rectangle {x1 / 1.f, y1 / 1.f, offsetX, offsetY};
}

}  // namespace RA_Util
namespace RA_Anim
{
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
auto initAnim(std::string_view const fileName,
              int const              animLenght,
              int const              speed,
              int const              speedMax) -> AnimData
{
    std::string path;
    path.reserve(RA_Global::texturePath.size() + fileName.size());
    path.append(RA_Global::texturePath);
    path.append(fileName);

    if (FileExists(path.c_str()))
    {
        Texture2D const temp = LoadTexture(path.c_str());
        return AnimData {.textureAnim  = temp,
                         .rect         = {.x     = 0,
                                          .y     = 0,
                                          .width = static_cast<float>(
                                      temp.width / animLenght),
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
auto cleanAnim(AnimData & outData) noexcept -> void
{
    UnloadTexture(outData.textureAnim);
    outData.rect = Rectangle {0, 0, 0, 0};
}

/*
 *@Goal: animation speed currection
 *@Note: it works as a input output param refrence
 *@Warning: internall function usage
 */
[[maybe_unused]]
auto limitSpeedAnim(AnimData & outData) noexcept -> void
{
    if (outData.currentSpeed > outData.speedMAX)
        outData.currentSpeed = outData.speedMAX;
    else if (outData.currentSpeed < outData.speedMIN)
        outData.currentSpeed = outData.speedMIN;
}

/*
 *@Note: it works as a input output param refrence
 */
[[maybe_unused]]
inline auto increaseAnimSpeed(AnimData & outData) noexcept -> void
{
    outData.currentSpeed += 1;
    limitSpeedAnim(outData);
}
/*
 *@Note: it works as a input output param refrence
 */
[[maybe_unused]]
inline auto decreaseAnimSpeed(AnimData & outData) noexcept -> void
{
    outData.currentSpeed -= 1;
    limitSpeedAnim(outData);
}

/*
 * @Note: it works as a input output param refrence
 */
[[maybe_unused]]
inline auto resetAnimSpeed(AnimData & outData) noexcept -> void
{
    outData.currentSpeed = outData.defaultSpeed;
}
/*
 * @Note: it works as a input output param refrence
 * @Warning: internall function usage for updateAnim
 */
[[maybe_unused]]
inline auto resetAnim(AnimData & outData) noexcept -> void
{
    outData.currentFrame = 0;
}
// TODO: zBuffer needed
[[maybe_unused]]
auto renderAnim(AnimData const & data,
                Vector2 const &  pos,
                Color const      tint = WHITE) noexcept -> void
{
    DrawTextureRec(data.textureAnim, data.rect, pos, tint);
}

/*
 *@Goal: get ready animation info for render them in gameloop
 */
[[maybe_unused]]
auto updateAnim(AnimData & data) noexcept -> void
{
    data.counter++;
    if (data.counter >= RA_Global::animationFPS / data.currentSpeed)
    {
        data.counter = 0;
        // speed currection
        limitSpeedAnim(data);
        data.currentFrame++;

        if (data.currentFrame >
            (data.length - 1))  // start animation frame is zero
            resetAnim(data);

        data.rect.x = static_cast<float>(data.currentFrame) *
                      static_cast<float>(data.textureAnim.width) /
                      static_cast<float>(data.length);
    }
}

}  // namespace RA_Anim
}  // namespace
// Define the operator== function outside the class
[[maybe_unused]]
auto operator==(Rectangle const & lhs, Rectangle const & rhs) noexcept
    -> bool
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int
{
    InitWindow(0, 0, "Test");
    // ToggleFullscreen();
    auto const height  = GetScreenHeight();
    auto const width   = GetScreenWidth();
    auto const fps     = GetMonitorRefreshRate(0);
    int const  row     = 100;
    int const  column  = 100;
    auto const offsety = static_cast<float>(height) / row;
    auto const offsetx = static_cast<float>(width) / column;
    // auto       gridRect = RA_Util::placeRelative(Rectangle {0,
    //                                                   0,
    //                                                   static_cast<float>(width),
    //                                                   static_cast<float>(
    //                                                       height)},
    //                                        49,
    //                                        49,
    //                                        100,
    //                                        100);
    auto gridRect = RA_Util::
        placeRelativeCenter(Rectangle {0.f,
                                       0.f,
                                       static_cast<float>(width),
                                       static_cast<float>(height)},
                            20,
                            50);

    auto gridinfo = RA_Util::createGridInfo(gridRect, column, row);
    Texture2D const gridTexture {
        RA_Util::genGridTexture(gridinfo, WHITE, BLACK)};
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
    RA_Anim::AnimData scarfyAnim {
        RA_Anim::initAnim("scarfy.png"sv, 6, 8, 15)};
    Vector2 playerPos {};
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
            Rectangle const temp {
                RA_Util::whereClicked(offsetx, offsety, mousePos)};
            if (std::find(rects.begin(), rects.end(), temp) ==
                rects.end())
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
        EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    RA_Anim::cleanAnim(scarfyAnim);
    return 0;
}