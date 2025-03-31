namespace
{
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace RA_Global
{
constexpr u8 const animationFPS = 60;

inline static constexpr str_v const texturePath = "resource/textures/"sv;

inline static constexpr str_v const fontPath = "resource/fonts/"sv;

enum class EFileType : u8
{
    Texture = 0,
    Font
};
constexpr Color const grey = Color {37, 37, 37, 255};

/*
 *@Goal: return the path to file based on fileType
 *@Note: put the file in
 *{currentProject}/resources/assets/resource/{fileType-folderName}/fileType
 */
[[nodiscard]] [[maybe_unused]]
auto pathToFile(str_v const fileName, EFileType const fileType) -> str
{
    str path;
    switch (fileType)
    {
        case EFileType::Font:
        {
            path.reserve(RA_Global::fontPath.size() + fileName.size());
            path.append(RA_Global::fontPath);
            break;
        }
        case EFileType::Texture:
        {
            path.reserve(RA_Global::texturePath.size() + fileName.size());
            path.append(RA_Global::texturePath);
            break;
        }
    }
    path.append(fileName);
    return path;
}

}  // namespace RA_Global

namespace RA_Util
{

class GRandom
{
public:

    GRandom()  = delete;
    ~GRandom() = default;
    explicit GRandom(f32 const min, f32 const max) noexcept :
    m_randDistro {min, max}
    {
    }

    [[nodiscard]] [[maybe_unused]]
    auto getRandom() const noexcept -> f32
    {
        return const_cast<GRandom &>(*this).getRandom();
    }

    [[nodiscard]] [[maybe_unused]]
    auto getRandom() noexcept -> f32
    {
        return m_randDistro(rand32);
    }

private:

    [[nodiscard]] [[maybe_unused]]
    static auto initRandWithSeed() noexcept -> std::mt19937 &
    {
        std::random_device rd {};
        std::seed_seq
                            seed {cast(std::mt19937::result_type,
                       std::chrono::steady_clock::now().time_since_epoch().count()),
                  cast(std::mt19937::result_type, rd())};
        static std::mt19937 rand {seed};
        return rand;
    }

    std::uniform_real_distribution<f32> m_randDistro;
    inline static std::mt19937          rand32 {initRandWithSeed()};
};

/*
 * @Goal: check an expresion in runtime if not android
 * @Note: pass a true condition that you need like percent>100 fail
 */
[[maybe_unused]]
auto checkAtRuntime(bool faildCondition, str_v const errMsg) noexcept -> void
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
                         u8 const          widthPercent,
                         u8 const          heightPercent) noexcept -> Rectangle
{

    // bounds checking on input args (debug only)
    checkAtRuntime((widthPercent > 100 || heightPercent > 100 ||
                    widthPercent == 0 || heightPercent == 0),
                   "Placement Relative inputs should be 1<n<100 "
                   "in percentage "
                   "(unsigned int)\n"sv);

    f32 const newX = parentInfo.x + (parentInfo.width / 2.f);
    f32 const newY = parentInfo.y + (parentInfo.height / 2.f);

    f32 const newW = parentInfo.width * widthPercent / 100.f;
    f32 const newH = parentInfo.height * heightPercent / 100.f;

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
                   u8 const          xPercentOffset,
                   u8 const          yPercentOffset,
                   u8 const          wPercentReminded,
                   u8 const          hPercentReminded) noexcept -> Rectangle
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

    f32 const newX = cast(f32, parentInfo.x + parentInfo.width) *
                     xPercentOffset / 100.f;
    f32 const newY = cast(f32, parentInfo.y + parentInfo.height) *
                     yPercentOffset / 100.f;
    return Rectangle {.x     = newX,
                      .y     = newY,
                      .width = (cast(f32, parentInfo.width) - newX) *
                               wPercentReminded / 100.f,
                      .height = (cast(f32, parentInfo.height) - newY) *
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
    u8        columnCount;
    u8        rowCount;
};

/*
 *
 *@Goal: initilizer function for gridInfo object
 *@Note: col and row should be bigger than 2
 */
[[nodiscard]] [[maybe_unused]]
inline auto createGridInfo(Rectangle const & gridRect,
                           u8 const          columnCount = 2,
                           u8 const          rowCount = 2) noexcept -> GridInfo
{
    // col and row should be bigger than 2X2
    // bounds checking on input args (debug only)
    checkAtRuntime((columnCount < 2 || rowCount < 2),
                   "column and row should be bigger than 2\n"sv);
    return GridInfo {.rect        = Rectangle {.x      = gridRect.x,
                                               .y      = gridRect.y,
                                               .width  = gridRect.width,
                                               .height = gridRect.height},
                     .cellSize    = Vector2 {gridRect.width / columnCount,
                                          gridRect.height / rowCount},
                     .columnCount = columnCount,
                     .rowCount    = rowCount};
}
/*
 *@Goal: draw grid on screen in real time with grid info
 *@Note: it can be slow if its a static grid use genGridTexture
 */
[[maybe_unused]]
auto drawGrid(GridInfo const & grid, Color const lineColor = WHITE) noexcept -> void
{
    // draw in between lines based on col and row
    // drawing row lines
    f32 yOffset {grid.rect.y};
    for (u16 i {}; i <= grid.rowCount; ++i)
    {
        DrawLineV(Vector2 {grid.rect.x, yOffset},
                  Vector2 {grid.rect.width + grid.rect.x, yOffset},
                  lineColor);
        yOffset += grid.cellSize.y;
    }
    // drawing column lines
    f32 xOffset {grid.rect.x};
    for (u16 i {}; i <= grid.columnCount; ++i)
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
                    i32 const        lineThickness = 10,
                    Color const      lineColor     = WHITE,
                    Color const backgroundColor = BLACK) noexcept -> Texture2D
{
    Image img = GenImageColor(cast(i32, grid.rect.width),
                              cast(i32, grid.rect.height),
                              backgroundColor);
    // draw in between lines based on col and row
    // drawing row lines
    f32 yOffset {grid.rect.y};
    for (u16 i {}; i <= grid.rowCount; ++i)
    {
        Vector2 const v0 {grid.rect.x, yOffset};
        Vector2 const v1 {grid.rect.width + grid.rect.x, yOffset};
        ImageDrawLineEx(&img, v0, v1, lineThickness, lineColor);
        yOffset += grid.cellSize.y;
    }
    // draw
    f32 xOffset {grid.rect.x};
    for (u16 i {}; i <= grid.columnCount; ++i)
    {
        Vector2 const v0 {xOffset, grid.rect.y};
        Vector2 const v1 {xOffset, grid.rect.height + grid.rect.y};
        ImageDrawLineEx(&img, v0, v1, lineThickness, lineColor);
        xOffset += grid.cellSize.x;
    }
    Texture2D gridTexture = LoadTextureFromImage(img);
    UnloadImage(img);
    return gridTexture;
}

/*
 * @Goal: check input-hit-position does inside the grid if so map it
 * to a sub-rectangle to that hit-pos based on grid-info and return that Cell as
 * a new rectangle (pos of new rect start from top-left)
 *
 * @Note: check the return velue before use it (nullopt)
 * @Warning: this function on his core does heavily depend on rounding integers

 * @Return: selected rectangle and index of it in grid(indexing start from
 bottom-right and go to left)
 * e.g for 3*3 grid index is:
 * 987
 * 654
 * 321
 */
[[nodiscard]] [[maybe_unused]]
auto mapHitToGridCell(GridInfo const & grid, Vector2 const & hitPos) noexcept
    -> std::optional<std::pair<Rectangle const, u8 const>>
{
    // sanity check for devide by zero
    checkAtRuntime((grid.cellSize.x == 0.f || grid.cellSize.y == 0.f ||
                    grid.columnCount == 0 || grid.rowCount == 0),
                   "grid cell size should not be zero"sv);

    // does hit is inside the grid
    if (hitPos.x < grid.rect.x || (hitPos.x - grid.rect.x) > grid.rect.width ||
        hitPos.y < grid.rect.y || (hitPos.y - grid.rect.y) > grid.rect.height)
        return std::nullopt;

    auto const tempRemX = cast(u8, (hitPos.x - grid.rect.x) / grid.cellSize.x);
    auto       x1       = cast(u16, (tempRemX * grid.cellSize.x) + grid.rect.x);
    if (tempRemX == 0)
        x1 = cast(u16, grid.rect.x);
    else if (tempRemX >= grid.columnCount)
        x1 = cast(u16, ((grid.columnCount - 1) * grid.cellSize.x) + grid.rect.x);

    auto const tempRemY = cast(u8, (hitPos.y - grid.rect.y) / grid.cellSize.y);
    auto       y1       = cast(u16, (tempRemY * grid.cellSize.y) + grid.rect.y);
    if (tempRemY == 0)
        y1 = cast(u16, grid.rect.y);
    else if (tempRemY >= grid.rowCount)
        y1 = cast(u16, ((grid.rowCount - 1) * grid.cellSize.y) + grid.rect.y);

// debuge draw
#if 0
    DrawRectangleRec(Rectangle {x1 / 1.f,
                                y1 / 1.f,
                                grid.cellSize.x,
                                grid.cellSize.y},
                     RAYWHITE);
    DrawCircleLinesV(Vector2 {x1 / 1.f, y1 / 1.f}, 10, RED);
    DrawCircleLinesV(Vector2 {x1 / 1.f, (y1 + grid.cellSize.y) / 1.f}, 10, RED);
    DrawCircleLinesV(Vector2 {(x1 + grid.cellSize.x) / 1.f, y1 / 1.f}, 10, RED);
    DrawCircleLinesV(Vector2 {(x1 + grid.cellSize.x) / 1.f,
                              (y1 + grid.cellSize.y) / 1.f},
                     10,
                     RED);
    DrawCircleLinesV(hitPos, 5, YELLOW);
#endif  // DEBUG

    // row* column = indexes
    // 3*3=9 is total indexes
    // 9 8 7
    // 6 5 4
    // 3 2 1
    u8 const totalLength {cast(u8, grid.columnCount * grid.rowCount)};

    u8 const currentIndex {
        cast(u8, (totalLength - ((tempRemX) + (tempRemY * grid.columnCount))))};

    // clang-format off
    return std::pair
    {
        Rectangle 
        {
            cast(f32, x1),
            cast(f32, y1),
            grid.cellSize.x,
            grid.cellSize.y
        },
        currentIndex
    };
    // clang-format on
}
/*
 * @Warning: this function just work with 3*3 grid
 * @Note : passed index should start from 1 to 3
 * @Goal: get and index of a rectangle on the grid and return center
 * point of that rectangle point of that rectangle
 * // TODO: reowrk to work with other dimensions
 */
[[maybe_unused]] [[nodiscard]]
auto index2PointOnGrid(u8 index, GridInfo const & grid) noexcept -> Vector2
{
    checkAtRuntime((index > (grid.columnCount * grid.rowCount)),
                   "index is not correct e.g:(1 to 9)"sv);

    i32 const ix = (index % grid.columnCount == 0)
                       ? 0
                       : (index % grid.columnCount > 1 ? 1 : grid.columnCount - 1);

    i32 const iy = (cast(f32, index) / cast(f32, grid.rowCount) > 2.f
                        ? 0
                        : (cast(f32, index) / cast(f32, grid.rowCount) > 1.f
                               ? 1
                               : grid.rowCount - 1));

    i32 const x = cast(i32,
                       grid.rect.x + (cast(f32, ix) * grid.cellSize.x) +
                           (grid.cellSize.x / 2.f));

    i32 const y = cast(i32,
                       grid.rect.y + (cast(f32, iy) * grid.cellSize.y) +
                           (grid.cellSize.y / 2.f));

    return {cast(f32, x), cast(f32, y)};
}
[[maybe_unused]]
auto moveTowards(Vector2 & p1, Vector2 const & p2, f32 step) noexcept -> void
{
    f32 const dx     = p2.x - p1.x;
    f32 const dy     = p2.y - p1.y;
    f32 const length = std::sqrt((dx * dx) + (dy * dy));

    if (length > 0.f && step < length)
    {  // Avoid overshooting
        p1.x += (dx / length) * step;
        p1.y += (dy / length) * step;
    }
    else
        p1 = p2;  // Snap to target if within step size
}
}  // namespace RA_Util

namespace RA_Anim
{

// TODO: it need a texture that support zBuffer
struct AnimData
{
    Texture2D textureAnim;
    Rectangle rect;
    i32       counter;
    i32       currentFrame;
    i32       currentSpeed;
    i32 const length;  // actual length is length -1
    i32 const defaultSpeed;
    i32 const speedMAX;
    i32 const speedMIN;
};

[[nodiscard]] [[maybe_unused]]
auto initAnim(str_v const fileName,
              i32 const   animLenght,
              i32 const   speed,
              i32 const   speedMax) -> AnimData
{
    // str path;
    // path.reserve(RA_Global::texturePath.size() + fileName.size());
    // path.append(RA_Global::texturePath);
    // path.append(fileName);
    str const path {RA_Global::pathToFile(fileName, RA_Global::EFileType::Texture)};

    if (FileExists(path.c_str()))
    {
        Texture2D const temp = LoadTexture(path.c_str());
        return AnimData {.textureAnim  = temp,
                         .rect         = {.x      = 0,
                                          .y      = 0,
                                          .width  = cast(f32, temp.width / animLenght),
                                          .height = cast(f32, temp.height)},
                         .counter      = 0,
                         .currentFrame = 0,
                         .currentSpeed = speed,
                         .length       = animLenght,
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

        if (data.currentFrame > (data.length - 1))  // start animation frame is zero
            resetAnim(data);

        data.rect.x = cast(f32, data.currentFrame) *
                      cast(f32, data.textureAnim.width) / cast(f32, data.length);
    }
}

}  // namespace RA_Anim
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
    auto operator==(ColoredRect const & rhs) const noexcept -> bool
    {
        return (rect.x == rhs.rect.x && rect.y == rhs.rect.y);
    }
};
// Define the operator== function outside the class
struct Particle
{
    Rectangle rect;
    b2BodyId  bodyID;
};
enum class GameState : u8
{
    none = 0,
    win,
    tie,
    end
};
struct Player
{
    std::bitset<9> moves;
    Color const    rectColor;
    str const      name;
    i32 const      id;
};

// game glob vars
i32                    gHeight {0};
i32                    gWidth {0};
u32                    winUIFramCounter {};
u32                    inputFramCounter {0};
bool                   canRegister {false};
bool                   canReset {false};
Vector2                uIPointAnimationWin {0.f, 0.f};
Vector2                mousePos {0.f, 0.f};
GameState              currentState {GameState::none};
RA_Util::GRandom const gRandom(0.f, 1400.f);
constexpr u16 const    fps {60};
constexpr u8 const     row    = 3;
constexpr u8 const     column = 3;
// clang-format off
// win condition should check this table to state the winner
inline static constexpr std::array<std::bitset<9>, 8> const winTable 
{
    0x007, 0x038,
    0x049, 0x054,
    0x092, 0x111, 
    0x124, 0x1c0
};
// clang-format on


namespace RA_Particle
{

[[nodiscard]] [[maybe_unused]]
auto initWorldOfBox2d() noexcept -> b2WorldId
{
    b2WorldDef   worldDef = {b2DefaultWorldDef()};
    b2Vec2 const gravity  = {0.f, -10.f};
    worldDef.gravity      = gravity;
    b2WorldId worldID     = {b2CreateWorld(&worldDef)};
    worldDef.enableSleep  = true;
    return worldID;
}

[[nodiscard]] [[maybe_unused]]
auto creatDynamicBody(Particle const & pr, b2WorldId const & worldID) noexcept
    -> b2BodyId
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
auto impulseParticles(std::vector<Particle> const & particles) noexcept -> void
{
    u16 i {};
    for (auto const pr : particles)
    {
        f32 force = 0.f;
        if (i < (particles.size() / 2))
            force = -500;
        else
            force = 500;
        b2Body_Enable(pr.bodyID);
        b2Body_ApplyForceToCenter(pr.bodyID,
                                  b2Vec2 {.x = force * gRandom.getRandom(),
                                          .y = force * gRandom.getRandom()},
                                  true);
        b2Body_ApplyTorque(pr.bodyID, force * gRandom.getRandom(), true);
        i++;
    }
}

[[maybe_unused]]
auto drawParticles(std::vector<Particle> const & particles, Color const color) noexcept
    -> void
{
    for (auto const pr : particles)
    {
        if (b2Body_GetPosition(pr.bodyID).y * -1 > cast(f32, gHeight) ||
            b2Body_GetPosition(pr.bodyID).x * -1 > cast(f32, gWidth) ||
            b2Body_GetPosition(pr.bodyID).x * -1 < 20.f)
        {
            b2Body_Disable(pr.bodyID);
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
                             b2Rot_GetAngle(b2Body_GetRotation(pr.bodyID)) * RAD2DEG,
                             color);
            // }
        }
    }
}
[[maybe_unused]]
auto resetParticles(std::vector<Particle> const & particles) noexcept -> void
{
    for (auto const pr : particles)
    {
        b2Body_SetTransform(pr.bodyID,
                            b2Vec2 {.x = -1.f * gRandom.getRandom(),
                                    .y = cast(f32, gHeight - 200)},
                            b2MakeRot(0.f));
        b2Body_Disable(pr.bodyID);
    }
}
}  // namespace RA_Particle

}  // namespace

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

    // clang-format off

    // font loading
    auto const font = LoadFont(
        RA_Global::pathToFile("NotoSans-VariableFont_wdth,wght.ttf"sv,
        RA_Global::EFileType::Font).c_str());

    // clang-format on

    // box2d init of the world of the game (box2d-related)
    // Simulating setting (box2d-related)
    b2WorldId const       worldID = RA_Particle::initWorldOfBox2d();
    constexpr f32 const   timeStep {1.f / 30.f};  // 30HZ
    constexpr u8 const    subStepCount {3};
    std::vector<Particle> particles {};
    particles.reserve(1000);
    // create dynamic bodies
    for (u16 i {}; i < 1000; ++i)
    {
        Particle pr {};
        pr.rect.x      = gRandom.getRandom();
        pr.rect.y      = cast(f32, (gHeight - 200) * -1);
        pr.rect.height = 15;
        pr.rect.width  = 15;
        pr.bodyID      = RA_Particle::creatDynamicBody(pr, worldID);
        particles.emplace_back(pr);
    }

    auto const gridRect = RA_Util::placeRelativeCenter(Rectangle {0.f,
                                                                  0.f,
                                                                  cast(f32, gWidth),
                                                                  cast(f32, gHeight)},
                                                       50,
                                                       80);

    auto const gridinfo = RA_Util::createGridInfo(gridRect, column, row);

    // center the texture of grid
    // we need gridinfo to be untouched for draw texture later
    auto gridTextureinfo   = gridinfo;
    gridTextureinfo.rect.x = -1.f;
    gridTextureinfo.rect.y = 0.f;

    Texture2D const gridTexture {
        RA_Util::genGridTexture(gridTextureinfo, 5, WHITE, RA_Global::grey)};

    std::vector<ColoredRect> rects;
    rects.reserve(row * column);
    // indexes of rects that caus win
    std::array<u8, 3> indexCausWin {};

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
                if (CheckCollisionPointRec(mousePos, resetBtn) && canRegister)
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
                        RA_Util::mapHitToGridCell(gridinfo, mousePos)};
                    // if player touch inside grid
                    if (filteredRect.has_value())
                    {
                        auto const [currentRect, indexRect] = filteredRect.value();
                        // create new rect inside the rect that touched with player color
                        ColoredRect newRect {};
                        // new rect should adjust size and coordinate inside the parent (touched rect)
                        newRect.rect = RA_Util::placeRelativeCenter(currentRect, 55, 55);
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
                                u8 counter {};
                                for (u32 i = 0; i < currentPlayer->moves.size(); ++i)
                                {
                                    // if both bit are 1
                                    if (currentPlayer->moves[i] && n[i])
                                    {
                                        // add this index it to index
                                        // buffer for win animation and drawing stuff
                                        indexCausWin[counter] = cast(u8, i + 1);
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
                        RA_Util::index2PointOnGrid(indexCausWin[2], gridinfo)};
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
                currentState = GameState::none;
                p1.moves.reset();
                p2.moves.reset();
                rects.clear();
                indexCausWin.fill(0);
                RA_Particle::resetParticles(particles);
                currentPlayer       = &p1;
                wonPlayer           = nullptr;
                canReset            = false;
                winUIFramCounter    = 0;
                uIPointAnimationWin = {};
            }
        }
        // draw game loop
        {
            ClearBackground(RA_Global::grey);
            BeginDrawing();
            BeginMode2D(camera);

            DrawTexture(gridTexture,
                        cast(i32, gridinfo.rect.x),
                        cast(i32, gridinfo.rect.y),
                        WHITE);
            // UI
            {
                DrawTextEx(font,
                           TextFormat("resulation : %d x %d", gWidth, gHeight),
                           Vector2 {50.f, 10.f},
                           cast(f32, font.baseSize),
                           2,
                           WHITE);
                DrawTextEx(font,
                           TextFormat("FPS: %d", GetFPS()),
                           Vector2 {50.f, 40.f},
                           cast(f32, font.baseSize),
                           2,
                           WHITE);

                // reset btn
                DrawRectangleRoundedLinesEx(resetBtn, .5f, 1, 5, WHITE);
                DrawTextEx(font,
                           "Reset",
                           Vector2 {resetBtn.x + (resetBtn.width / 2.f) - 31,
                                    resetBtn.y + (resetBtn.height / 2.f) - 11},
                           cast(f32, font.baseSize),
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
                               cast(f32, font.baseSize),
                               2,
                               currentPlayer->rectColor);
                    break;
                }
                case GameState::win:
                {
                    winUIFramCounter++;
                    DrawTextEx(font,
                               str {wonPlayer->name + " Won"s}.c_str(),
                               Vector2 {cast(f32, (gWidth / 2.f) - 200.f), 5.f},
                               cast(f32, font.baseSize * 2),
                               2,
                               wonPlayer->rectColor);

                    // animation of wining
                    auto const v {
                        RA_Util::index2PointOnGrid(indexCausWin[0], gridinfo)};
                    auto const v1 {
                        RA_Util::index2PointOnGrid(indexCausWin[1], gridinfo)};
                    auto const v2 {
                        RA_Util::index2PointOnGrid(indexCausWin[2], gridinfo)};
                    constexpr Color const col = WHITE;
                    if (winUIFramCounter >= 10)
                    {
                        DrawCircle(cast(i32, v.x), cast(i32, v.y), 25.f, col);
                    }
                    if (winUIFramCounter >= 20)
                    {
                        DrawCircle(cast(i32, v1.x), cast(i32, v1.y), 25.f, col);
                    }
                    if (winUIFramCounter >= 30)
                    {
                        DrawCircle(cast(i32, v2.x), cast(i32, v2.y), 25.f, col);
                    }
                    if (winUIFramCounter >= 40)
                    {
                        RA_Util::moveTowards(uIPointAnimationWin, v, 20);
                        DrawLineEx(uIPointAnimationWin, v2, 10.f, col);
                    }
                    if (winUIFramCounter >= 45)
                    {
                        RA_Particle::drawParticles(particles, wonPlayer->rectColor);
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
                               Vector2 {cast(f32, (gWidth / 2.f) - 100.f), 5.f},
                               cast(f32, font.baseSize),
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