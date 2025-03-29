namespace
{
using namespace std::string_literals;
using namespace std::string_view_literals;
namespace RA_Global
{
constexpr std::uint8_t const animationFPS = 60;
inline static constexpr std::string_view const
    texturePath = "resource/"sv;
inline static constexpr std::string const fontPath = "resource/font/"s;
constexpr Color const Grey = Color {37, 37, 37, 255};
}  // namespace RA_Global

namespace RA_Util
{

class GRandom
{
public:

    GRandom()  = delete;
    ~GRandom() = default;
    explicit GRandom(float const min, float const max) noexcept :
    randDistro {min, max}
    {
    }
    float GetRandom() const noexcept
    {
        return const_cast<GRandom &>(*this).GetRandom();
    }
    float GetRandom() noexcept
    {
        return randDistro(rand32);
    }

private:

    [[nodiscard]] [[maybe_unused]]
    static auto initRandWithSeed() noexcept -> std::mt19937 &
    {
        std::random_device rd {};
        std::seed_seq seed {static_cast<std::mt19937::result_type>(
                                std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()),
                            static_cast<std::mt19937::result_type>(rd())};
        static std::mt19937 rand {seed};
        return rand;
    }

private:

    std::uniform_real_distribution<float> randDistro {};
    inline static std::mt19937            rand32 {initRandWithSeed()};
};

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

// grid index order row:3 column:3
// 9 8 7
// 6 5 4
// 3 2 1
struct GridInfo
{
    Rectangle rect;
    Vector2   cellSize;
    uint8_t   columnCount;
    uint8_t   rowCount;
};

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
                    int const        lineThickness   = 10,
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
        Vector2 const v0 {grid.rect.x, yOffset};
        Vector2 const v1 {grid.rect.width + grid.rect.x, yOffset};
        ImageDrawLineEx(&img, v0, v1, lineThickness, lineColor);
        yOffset += grid.cellSize.y;
    }
    // draw
    float xOffset {grid.rect.x};
    for (unsigned int i {}; i <= grid.columnCount; ++i)
    {
        Vector2 const v0 {xOffset, grid.rect.y};
        Vector2 const v1 {xOffset, grid.rect.height + grid.rect.y};
        ImageDrawLineEx(&img, v0, v1, lineThickness, lineColor);
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

 * @Return: selected rectangle and index of it in grid(indexing start from bottom-right and go to left)
 * e.g for 3*3 grid:
 * 987
 * 654
 * 321
 */
[[nodiscard]] [[maybe_unused]]
auto mapTouchToGridCell(GridInfo const & grid,
                        Vector2 const &  hitPos,
                        bool const       debugDraw = false) noexcept
    -> std::optional<std::pair<Rectangle const, std::size_t const>>
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

    auto const tempRemX = static_cast<std::size_t>(
        (hitPos.x - grid.rect.x) / grid.cellSize.x);

    auto x1 = static_cast<std::size_t>(
        (tempRemX * grid.cellSize.x) + grid.rect.x);

    if (tempRemX == 0)
        x1 = static_cast<std::size_t>(grid.rect.x);
    else if (tempRemX >= grid.columnCount)
        x1 = static_cast<std::size_t>(
            ((grid.columnCount - 1) * grid.cellSize.x) + grid.rect.x);

    auto const tempRemY = static_cast<std::size_t>(
        (hitPos.y - grid.rect.y) / grid.cellSize.y);

    auto y1 = static_cast<std::size_t>(
        (tempRemY * grid.cellSize.y) + grid.rect.y);
    if (tempRemY == 0)
        y1 = static_cast<std::size_t>(grid.rect.y);

    else if (tempRemY >= grid.rowCount)
        y1 = static_cast<std::size_t>(
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

    // row* column = indexes
    // 3*3=9 is total indexes
    // 9 8 7
    // 6 5 4
    // 3 2 1
    auto const totalLength {
        static_cast<std::size_t>(grid.columnCount * grid.rowCount)};

    auto const currentIndex {
        totalLength - ((tempRemX) + (tempRemY * grid.columnCount))};

    return std::pair {Rectangle {x1 / 1.f,
                                 y1 / 1.f,
                                 grid.cellSize.x,
                                 grid.cellSize.y},
                      currentIndex};
}
/*
 * @Warning: this function just work with 3*3 grid
 * @Note : passed index should start from 1 to 3
 * @Goal: get and index of a rectangle on the grid and return center
 * point of that rectangle point of that rectangle
 * // TODO: reowrk to work with other dimensions
 */
[[maybe_unused]] [[nodiscard]]
auto index2PointOnGrid(int index, GridInfo const & grid) noexcept
    -> Vector2
{
    checkAtRuntime((index <= 0 ||
                    index > (grid.columnCount * grid.rowCount)),
                   "index is not correct e.g:(1 to 9)"sv);
    int ix = (index % grid.columnCount == 0)
                 ? 0
                 : (index % grid.columnCount > 1 ? 1 : grid.columnCount - 1);

    int iy = ((float)index / (float)grid.rowCount > 2.f
                  ? 0
                  : ((float)index / (float)grid.rowCount > 1.f
                         ? 1
                         : grid.rowCount - 1));

    int x = grid.rect.x + (ix * grid.cellSize.x) +
            (grid.cellSize.x / 2.f);
    int y = grid.rect.y + (iy * grid.cellSize.y) +
            (grid.cellSize.y / 2.f);
    return {x / 1.f, y / 1.f};
}
[[maybe_unused]]
auto moveTowards(Vector2 & p1, Vector2 const & p2, double step) noexcept
    -> void
{
    float const dx     = p2.x - p1.x;
    float const dy     = p2.y - p1.y;
    float const length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.f && step < length)
    {  // Avoid overshooting
        p1.x += (dx / length) * step;
        p1.y += (dy / length) * step;
    }
    else
    {
        p1 = p2;  // Snap to target if within step size
    }
}
}  // namespace RA_Util

namespace RA_Anim
{

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
struct ColoredRect
{
    Rectangle rect;
    Color     color;
    ColoredRect()  = default;
    ~ColoredRect() = default;
    auto operator=(ColoredRect const & rhs) -> ColoredRect &
    {
        if (this == &rhs)
            return *this;
        rect  = rhs.rect;
        color = rhs.color;
        return *this;
    }
    [[maybe_unused]]
    auto operator==(ColoredRect const & rhs) noexcept -> bool
    {
        return (rect.x == rhs.rect.x && rect.y == rhs.rect.y);
    }
    [[maybe_unused]]
    auto operator==(ColoredRect const & rhs) const noexcept -> bool
    {
        return (const_cast<ColoredRect &>(*this) == rhs);
    }
};
// Define the operator== function outside the class
struct particle
{
    Rectangle rect;
    b2BodyId  bodyID;
};
enum class GameState : std::uint8_t
{
    none = 0,
    win,
    tie,
    end
};
struct Player
{
    std::bitset<9>    moves;
    Color const       rectColor;
    std::string const name;
    int const         id;
};

// game glob vars
constexpr unsigned int const fps {60};
constexpr int const          row    = 3;
constexpr int const          column = 3;
int                          gHeight {};
int                          gWidth {};
Vector2                      mousePos {};
bool                         canRegister = false;
bool                         canReset    = false;
GameState                    currentState {GameState::none};
unsigned int                 inputFramCounter {};
Vector2                      uIPointAnimationWin {};
unsigned int                 winUIFramCounter {};
// win condition should check this table to state the winner
inline static constexpr std::array<std::bitset<9>, 8> const
    winTable {0x007, 0x038, 0x049, 0x054, 0x092, 0x111, 0x124, 0x1c0};
RA_Util::GRandom const gRandom(0.f, 1400);


namespace RA_Particle
{


b2WorldId initWorldOfBox2d()
{
    b2WorldDef   worldDef = {b2DefaultWorldDef()};
    b2Vec2 const gravity  = {0.f, -10.f};
    worldDef.gravity      = gravity;
    b2WorldId worldID     = {b2CreateWorld(&worldDef)};
    worldDef.enableSleep  = true;
    return worldID;
}
b2BodyId creatDynamicBody(particle & pr, b2WorldId const & worldID)
{
    // Create a dynamic box (box2d-related)
    b2BodyDef boxDef          = {b2DefaultBodyDef()};
    boxDef.isEnabled          = false;
    boxDef.enableSleep        = true;
    boxDef.position           = b2Vec2 {-pr.rect.x, -pr.rect.y};
    boxDef.type               = b2_dynamicBody;
    boxDef.rotation           = b2MakeRot(30.f * DEG2RAD);
    b2BodyId const  boxBodyId = {b2CreateBody(worldID, &boxDef)};
    b2Polygon const boxShape  = {
        b2MakeBox(pr.rect.width / 2.f, pr.rect.height / 2.f)};
    b2ShapeDef boxShapeDef          = {b2DefaultShapeDef()};
    boxShapeDef.density             = 0.5f;
    boxShapeDef.friction            = 0.1f;
    boxShapeDef.filter.categoryBits = 0x0002;
    boxShapeDef.filter.maskBits     = 0xFFFF ^ 0x0002;
    b2CreatePolygonShape(boxBodyId, &boxShapeDef, &boxShape);
    return boxBodyId;
}

[[maybe_unused]]
auto impulseParticles(std::vector<particle> const & particles) noexcept
    -> void
{
    std::uint8_t i {};
    for (auto const pr : particles)
    {
        float force = 0.f;
        if (i < (particles.size() / 2))
            force = -500;
        else
            force = 500;
        b2Body_Enable(pr.bodyID);
        b2Body_ApplyForceToCenter(pr.bodyID,
                                  b2Vec2 {.x = force * gRandom.GetRandom(),
                                          .y = force * gRandom.GetRandom()},
                                  true);
        b2Body_ApplyTorque(pr.bodyID, force * gRandom.GetRandom(), true);
        i++;
    }
}

[[maybe_unused]]
auto drawParticles(std::vector<particle> const & particles,
                   Color const color) noexcept -> void
{
    for (auto const pr : particles)
    {
        if (b2Body_GetPosition(pr.bodyID).y * -1 >
                static_cast<float>(gHeight) ||
            b2Body_GetPosition(pr.bodyID).x * -1 >
                static_cast<float>(gWidth) ||
            b2Body_GetPosition(pr.bodyID).x * -1 < 20.f)
        {
            b2Body_Disable(pr.bodyID);
            continue;
        }
        else
        {
            // if (b2Body_IsEnabled(pr.bodyID))
            // {
            b2Vec2 const boxPos {b2Body_GetPosition(pr.bodyID)};
            DrawRectanglePro(Rectangle {.x      = -boxPos.x,
                                        .y      = -boxPos.y,
                                        .width  = pr.rect.width,
                                        .height = pr.rect.height},
                             Vector2 {.x = (pr.rect.width / 2.f),
                                      .y = (pr.rect.height / 2.f)},
                             b2Rot_GetAngle(b2Body_GetRotation(pr.bodyID)) *
                                 RAD2DEG,
                             color);
            // }
        }
    }
}
[[maybe_unused]]
auto resetParticles(std::vector<particle> const & particles) noexcept
    -> void
{
    for (auto const pr : particles)
    {
        b2Body_SetTransform(pr.bodyID,
                            b2Vec2 {.x = -1.f * gRandom.GetRandom(),
                                    .y = static_cast<float>(gHeight - 200)},
                            b2MakeRot(0.f));
        b2Body_Disable(pr.bodyID);
    }
}
}  // namespace RA_Particle
auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int
{

    // init
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_BORDERLESS_WINDOWED_MODE |
                   FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(0, 0, "FirstGame");
    gHeight = GetScreenHeight();
    gWidth  = GetScreenWidth();
    // auto const fps    = GetMonitorRefreshRate(0);
    SetTargetFPS(fps);

    // font loading
    auto const font = LoadFont(std::string {
        RA_Global::fontPath +
        "No"
        "to"
        "Sa"
        "ns"
        "-V"
        "ar"
        "ia"
        "bl"
        "eF"
        "on"
        "t_"
        "wd"
        "th"
        ",w"
        "gh"
        "t."
        "tt"
        "f"}
                                   .c_str());

    // box2d init of the world of the game (box2d-related)
    // Simulating setting (box2d-related)
    b2WorldId const        worldID = RA_Particle::initWorldOfBox2d();
    constexpr float const  timeStep {1.f / 30.f};  // 30HZ
    constexpr int8_t const subStepCount {3};
    std::vector<particle>  particles {};
    particles.reserve(1000);
    // create dynamic bodies
    for (size_t i {}; i < 1000; ++i)
    {
        particle pr {};
        pr.rect.x      = gRandom.GetRandom();
        pr.rect.y      = static_cast<float>((gHeight - 200) * -1);
        pr.rect.height = 15;
        pr.rect.width  = 15;
        pr.bodyID      = RA_Particle::creatDynamicBody(pr, worldID);
        particles.emplace_back(pr);
    }

    auto const gridRect = RA_Util::
        placeRelativeCenter(Rectangle {0.f,
                                       0.f,
                                       static_cast<float>(gWidth),
                                       static_cast<float>(gHeight)},
                            50,
                            80);

    auto const gridinfo = RA_Util::createGridInfo(gridRect, column, row);

    // center the texture of grid
    // we need gridinfo to be untouched for draw texture later
    auto gridTextureinfo   = gridinfo;
    gridTextureinfo.rect.x = 0.f;
    gridTextureinfo.rect.y = 0.f;

    Texture2D const gridTexture {
        RA_Util::genGridTexture(gridTextureinfo, 5, WHITE, RA_Global::Grey)};

    std::vector<ColoredRect> rects;
    rects.reserve(row * column);
    // indexes of rects that caus win
    std::array<int, 3> indexCausWin {};

    Camera2D const camera {.offset   = Vector2 {},
                           .target   = Vector2 {},
                           .rotation = 0.f,
                           .zoom     = 1.f};
    // player1
    Player p1 {.moves = {}, .rectColor = RED, .name = "Red"s, .id = 0};
    // player2
    Player p2 {.moves = {}, .rectColor = BLUE, .name = "Blue"s, .id = 1};

    // current player
    Player* currentPlayer = &p1;
    Player* wonPlayer {nullptr};
    // reset btn ui
    Rectangle const resetBtn {.x      = 50.f,
                              .y      = gridinfo.rect.y,
                              .width  = 300.f,
                              .height = 150.f};


    // game loop
    while (!WindowShouldClose() && currentState != GameState::end)
    {
        // input
        {
            // make input less responsive bc dont need every fram input
            // pulling => less cpu hogging || more proformance friendly
            inputFramCounter++;
            if (inputFramCounter >= 2)
            {
                mousePos         = GetTouchPosition(0);
                inputFramCounter = 0;
                canRegister      = false;
                if (IsKeyPressed(KEY_ESCAPE))
                    currentState = GameState::end;
                else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    canRegister = true;
                }
                // ui hit detection
                if (CheckCollisionPointRec(mousePos, resetBtn) &&
                    canRegister)
                {
                    canReset = true;
                }
            }
        }
        // update
        {
            // box2d Update world state (box2d-related)
            b2World_Step(worldID, timeStep, subStepCount);
            // update game state
            if (currentState == GameState::none)
            {
                // update game based on input
                if (canRegister)
                {
                    // what is the sub-rectangle on the grid + index of that rectangle in optional type
                    auto const filteredRect {
                        RA_Util::mapTouchToGridCell(gridinfo, mousePos)};
                    // if player touch inside grid
                    if (filteredRect.has_value())
                    {
                        auto const [currentRect,
                                    indexRect] = filteredRect.value();
                        // create new rect inside the rect that touched with player color
                        ColoredRect newRect {};
                        // new rect should adjust size and coordinate inside the parent (touched rect)
                        newRect.rect = RA_Util::placeRelativeCenter(currentRect,
                                                                    55,
                                                                    55);
                        // adjust color based on current player
                        newRect.color = currentPlayer->rectColor;
                        // if this new rect does not exist in buffer add it to buffer
                        if (std::ranges::find(std::cbegin(rects),
                                              std::cend(rects),
                                              newRect) == std::cend(rects))
                        {
                            rects.emplace_back(newRect);
                            // each rect index is 1 or 0 based on
                            // previouse touched on it bitset start
                            // from zero but rect index start from 1
                            // => so we should do: indexRect -1
                            currentPlayer->moves.set((indexRect)-1, true);
                            // check for win condition on LookUpTable bitsets => bit by bit
                            for (auto const n : winTable)
                            {
                                std::size_t counter {};
                                for (size_t i = 0;
                                     i < currentPlayer->moves.size();
                                     ++i)
                                {
                                    // if both bit are 1
                                    if (currentPlayer->moves[i] && n[i])
                                    {
                                        // add this index it to index
                                        // buffer for win animation and drawing stuff
                                        indexCausWin[counter] = static_cast<int>(
                                            i + 1);
                                        counter++;
                                    }
                                }
                                // if 3 bits is set it means you match one
                                // of the winTable numbers
                                // ToDo : this 3 should change based on row* col size
                                if (counter == 3)
                                {
                                    currentState = GameState::win;
                                    break;
                                }
                            }
                            // change current player to next player if the game is going on
                            if (currentState == GameState::none)
                            {
                                if (currentPlayer->id == p1.id)
                                {
                                    currentPlayer = &p2;
                                }
                                else
                                {
                                    currentPlayer = &p1;
                                }
                            }
                        }
                    }
                }
                // update win state
                if (currentState == GameState::win)
                {
                    wonPlayer = currentPlayer;
                    // index of rectangle to center point on that rectangle
                    uIPointAnimationWin = {
                        RA_Util::index2PointOnGrid(indexCausWin[2],
                                                   gridinfo)};
                    RA_Particle::impulseParticles(particles);
                }
                // update tie state
                // touched rect buffer is full but no one won the game
                else if (rects.size() == (row * column) &&
                         currentState == GameState::none)
                {
                    currentState = GameState::tie;
                    RA_Particle::impulseParticles(particles);
                }
            }
            // update ui
            // reset button clicked
            if (canReset)
            {
                currentState  = GameState::none;
                p1.moves      = {};
                p2.moves      = {};
                currentPlayer = &p1;
                wonPlayer     = nullptr;
                rects.clear();
                indexCausWin.fill(0);
                canReset            = false;
                winUIFramCounter    = 0;
                uIPointAnimationWin = {};
                // reset particles trandform
                RA_Particle::resetParticles(particles);
            }
        }
        // draw game loop
        {
            ClearBackground(RA_Global::Grey);
            BeginDrawing();
            BeginMode2D(camera);

            DrawTexture(gridTexture,
                        static_cast<int>(gridinfo.rect.x),
                        static_cast<int>(gridinfo.rect.y),
                        WHITE);
            // UI
            {
                DrawTextEx(font,
                           TextFormat("resulation : %d x %d", gWidth, gHeight),
                           Vector2 {50.f, 10.f},
                           (float)font.baseSize,
                           2,
                           WHITE);
                DrawTextEx(font,
                           TextFormat("FPS: %d", GetFPS()),
                           Vector2 {50.f, 40.f},
                           (float)font.baseSize,
                           2,
                           WHITE);

                // reset btn
                DrawRectangleRoundedLinesEx(resetBtn, .5f, 1, 5, WHITE);
                DrawTextEx(font,
                           "Reset",
                           Vector2 {resetBtn.x + (resetBtn.width / 2.f) - 31,
                                    resetBtn.y +
                                        (resetBtn.height / 2.f) - 11},
                           (float)font.baseSize,
                           2,
                           WHITE);
                // reset btn
            }
            // End UI

            // touched rect buffer drawing
            for (auto const & rect : rects)
            {
                DrawRectangleRec(rect.rect, rect.color);
            }
            // win drawing animation and etc ...
            switch (currentState)
            {
                case GameState::none:
                {
                    DrawTextEx(font,
                               "Turn",
                               Vector2 {70.f, 350.f},
                               (float)font.baseSize,
                               2,
                               currentPlayer->rectColor);
                    break;
                }
                case GameState::win:
                {
                    winUIFramCounter++;
                    DrawTextEx(font,
                               std::string {wonPlayer->name + " Won"s}.c_str(),
                               Vector2 {static_cast<float>(
                                            gWidth / 2.f - 200.f),
                                        5.f},
                               (float)font.baseSize * 2,
                               2,
                               wonPlayer->rectColor);

                    // animation of wining
                    auto const v {
                        RA_Util::index2PointOnGrid(indexCausWin[0],
                                                   gridinfo)};
                    auto const v1 {
                        RA_Util::index2PointOnGrid(indexCausWin[1],
                                                   gridinfo)};
                    auto const v2 {
                        RA_Util::index2PointOnGrid(indexCausWin[2],
                                                   gridinfo)};
                    constexpr Color const col = WHITE;
                    if (winUIFramCounter >= 10)
                    {
                        DrawCircle(v.x, v.y, 25.f, col);
                    }
                    if (winUIFramCounter >= 20)
                    {
                        DrawCircle(v1.x, v1.y, 25.f, col);
                    }
                    if (winUIFramCounter >= 30)
                    {
                        DrawCircle(v2.x, v2.y, 25.f, col);
                    }
                    if (winUIFramCounter >= 40)
                    {
                        RA_Util::moveTowards(uIPointAnimationWin, v, 20);
                        DrawLineEx(uIPointAnimationWin, v2, 10.f, col);
                    }
                    if (winUIFramCounter >= 45)
                    {
                        RA_Particle::drawParticles(particles,
                                                   wonPlayer->rectColor);
                    }
                    if (winUIFramCounter > 700)
                    {
                        winUIFramCounter = 45;
                    }
                    break;
                }
                case GameState::tie:
                {
                    DrawTextEx(font,
                               "Tie",
                               Vector2 {static_cast<float>(
                                            gWidth / 2.f - 100.f),
                                        5.f},
                               (float)font.baseSize,
                               2.f,
                               WHITE);

                    RA_Particle::drawParticles(particles, WHITE);
                    break;
                }
                case GameState::end:
                    [[fallthrough]];

                default:
                    break;
            }
            EndMode2D();
            EndDrawing();
        }
    }
    // clean-up
    b2DestroyWorld(worldID);
    // worldID = b2_nullWorldId;
    UnloadFont(font);
    CloseWindow();
    return 0;
}