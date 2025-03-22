#include "raylib.h"
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
    return GridInfo {.rect = Rectangle {.x      = gridRect.x,
                                        .y      = gridRect.y,
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
    }
    // drawing column lines
    float xOffset {grid.rect.x};
    for (unsigned int i {}; i <= grid.columnCount; ++i)
    {
        DrawLineV(Vector2 {xOffset, grid.rect.y},
                  Vector2 {xOffset, grid.rect.height + grid.rect.y},
                  lineColor);
        xOffset += grid.cellSize.x;
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

/*
 * @Goal: check input-hit-position does inside the grid if so map it
 * to a sub-rectangle to that hit-pos based on grid-info and return that Cell
 * @Note: check the return velue before use it (nullopt)
 * @Warning: this function on his core does heavily depend on rounding integers
 */
[[nodiscard]] [[maybe_unused]]
auto mapTouchToGridCell(GridInfo const & grid,
                        Vector2 const &  hitPos,
                        bool const       debugDraw = false) noexcept
    -> std::optional<Rectangle const>
{
    // sanity check for devide by zero
    checkAtRuntime((grid.cellSize.x == 0.f || grid.cellSize.y == 0.f),
                   "grid cell size should not be zero"sv);

    // does hit is inside the grid
    if (hitPos.x < grid.rect.x ||
        (hitPos.x - grid.rect.x) > grid.rect.width ||
        hitPos.y < grid.rect.y ||
        (hitPos.y - grid.rect.y) > grid.rect.height)
        return std::nullopt;

    int const tempRemX = static_cast<int>(
        (hitPos.x - grid.rect.x) / grid.cellSize.x);

    int x1 = static_cast<int>((tempRemX * grid.cellSize.x) + grid.rect.x);

    if (tempRemX == 0)
        x1 = static_cast<int>(grid.rect.x);
    else if (tempRemX >= grid.columnCount)
        x1 = static_cast<int>(
            ((grid.columnCount - 1) * grid.cellSize.x) + grid.rect.x);

    int const tempRemY = static_cast<int>(
        (hitPos.y - grid.rect.y) / grid.cellSize.y);

    int y1 = static_cast<int>((tempRemY * grid.cellSize.y) + grid.rect.y);
    if (tempRemY == 0)
        y1 = static_cast<int>(grid.rect.y);

    else if (tempRemY >= grid.rowCount)
        y1 = static_cast<int>(
            ((grid.rowCount - 1) * grid.cellSize.y) + grid.rect.y);

#ifdef DEBUG
    if (debugDraw)
    {
        DrawRectangleRec(Rectangle {x1 / 1.f,
                                    y1 / 1.f,
                                    grid.cellSize.x,
                                    grid.cellSize.y},
                         RAYWHITE);
        DrawCircleLinesV(Vector2 {x1 / 1.f, y1 / 1.f}, 10, RED);
        DrawCircleLinesV(Vector2 {x1 / 1.f, (y1 + grid.cellSize.y) / 1.f},
                         10,
                         RED);
        DrawCircleLinesV(Vector2 {(x1 + grid.cellSize.x) / 1.f, y1 / 1.f},
                         10,
                         RED);
        DrawCircleLinesV(Vector2 {(x1 + grid.cellSize.x) / 1.f,
                                  (y1 + grid.cellSize.y) / 1.f},
                         10,
                         RED);
        DrawCircleLinesV(hitPos, 5, YELLOW);
    }
#endif  // DEBUG
    return Rectangle {x1 / 1.f,
                      y1 / 1.f,
                      grid.cellSize.x,
                      grid.cellSize.y};
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
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_BORDERLESS_WINDOWED_MODE |
                   FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(0, 0, "Test");
    auto const height   = GetScreenHeight();
    auto const width    = GetScreenWidth();
    auto const fps      = GetMonitorRefreshRate(0);
    int const  row      = 3;
    int const  column   = 3;
    auto       gridRect = RA_Util::
        placeRelativeCenter(Rectangle {0.f,
                                       0.f,
                                       static_cast<float>(width),
                                       static_cast<float>(height)},
                            20,
                            20);

    auto gridinfo = RA_Util::createGridInfo(gridRect, column, row);

    // center the texture of grid
    auto gridTextureinfo   = gridinfo;
    gridTextureinfo.rect.x = 0.f;
    gridTextureinfo.rect.y = 0.f;

    Texture2D const gridTexture {
        RA_Util::genGridTexture(gridTextureinfo, WHITE, BLACK)};

    std::vector<Rectangle> rects;
    rects.reserve(1000);
    Vector2 mousePos {};
    SetTargetFPS(fps);

    bool isEnd {false};
    bool canRegister {false};

    Camera2D camera;
    camera.offset   = Vector2 {};
    camera.target   = Vector2 {};
    camera.rotation = 0;
    camera.zoom     = 1.f;

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

            std::optional<Rectangle> filteredRect {
                RA_Util::mapTouchToGridCell(gridinfo, mousePos)};

            if (filteredRect.has_value())
            {
                if (std::find(rects.begin(),
                              rects.end(),
                              filteredRect.value()) == rects.end())
                {
                    rects.emplace(std::begin(rects),
                                  filteredRect.value());
                    if (rects.size() > ((row * column) - 2))
                    {
                        rects.pop_back();
                    }
                }
            }
        }

        ClearBackground(WHITE);
        BeginDrawing();
        BeginMode2D(camera);
        DrawTexture(gridTexture,
                    static_cast<int>(gridinfo.rect.x),
                    static_cast<int>(gridinfo.rect.y),
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
        EndMode2D();
        EndDrawing();
    }
    CloseWindow();
    return 0;
}