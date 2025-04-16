#include "raylib.h"
namespace
{
using namespace std::string_literals;
using namespace std::string_view_literals;

namespace RA_Global
{
constexpr u8 const animationFPS = 60;
// all of path should start without / e.g= resources/...
inline static constexpr str_v const texturePath = "resource/textures/"sv;
inline static constexpr str_v const fontPath    = "resource/fonts/"sv;
inline static constexpr str_v const shadersPath = "resource/shaders/glsl"sv;

enum class EFileType : u8
{
    Texture = 0,
    Font,
    Shader
};
constexpr Color const gray = Color {37, 37, 37, 255};

/*
 *@Goal: return the path to file based on fileType
 *@Note: put the file in
 *{currentProject}/resources/assets/resource/{fileType-folderName}/fileType
 */
[[nodiscard]] [[maybe_unused]]
auto pathToFile(str_v const fileName, EFileType const fileType) -> str
{
    str path;
#if defined(PLATFORM_DESKTOP)  // finding abs path on deskto
    str const root = GetApplicationDirectory();
    path.append(root);
#endif
    switch (fileType)
    {
        case EFileType::Font:
        {
            path.append(RA_Global::fontPath);
            break;
        }
        case EFileType::Texture:
        {
            path.append(RA_Global::texturePath);
            break;
        }
        case EFileType::Shader:
        {
            // the 2 is the glsl define size
            path.append(RA_Global::shadersPath);
            path.append(TextFormat("%i/", GLSL_VERSION));
            break;
        }
    }
    path.append(fileName);
    return path;
}

}  // namespace RA_Global
namespace RA_Font
{

/*
 * @Goat: draw text as a sdf mode
 */
[[nodiscard]] [[maybe_unused]]
auto initSDFFont(str_v const fontFileName, i32 const fontSize, i32 const glyphCount)
    -> std::pair<Font, Shader>
{

    // Loading file to memory
    int            fileSize = 0;
    unsigned char* fileData = LoadFileData(RA_Global::pathToFile(fontFileName,
                                                                 RA_Global::EFileType::Font)
                                               .c_str(),
                                           &fileSize);
    Font           fontSDF {};
    fontSDF.baseSize   = fontSize;
    fontSDF.glyphCount = glyphCount;
    // Parameters > font size: 16, no glyphs array provided (0), glyphs count: 0 (defaults to 95)
    fontSDF.glyphs = LoadFontData(fileData, fileSize, fontSize, nullptr, 0, FONT_SDF);
    // Parameters > glyphs count: 95, font size: 16, glyphs padding in image: 0 px, pack method: 1 (Skyline algorythm)
    Image const atlas = GenImageFontAtlas(fontSDF.glyphs,
                                          &fontSDF.recs,
                                          glyphCount,
                                          fontSize,
                                          0,
                                          1);
    fontSDF.texture   = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(fileData);  // Free memory from loaded file
    // Load SDF required shader (we use default vertex shader)
    Shader const shader = LoadShader(nullptr,
                                     RA_Global::pathToFile("sdf.fs"sv,
                                                           RA_Global::EFileType::Shader)
                                         .c_str());
    SetTextureFilter(fontSDF.texture,
                     TEXTURE_FILTER_BILINEAR);  // Required for SDF font
    return std::pair {fontSDF, shader};
}

/*
 * @Goat: draw text as a sdf mode
 */
[[nodiscard]] [[maybe_unused]]
auto initFont(str_v const         fontFileName,
              i32 const           fontSize,
              TextureFilter const filterFlag) -> std::pair<Font, f32>
{
    auto font = LoadFontEx(RA_Global::pathToFile(fontFileName,
                                                 RA_Global::EFileType::Font)
                               .c_str(),
                           fontSize,
                           nullptr,
                           0);
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, filterFlag);
    return std::pair {font, cast(f32, font.baseSize)};
}

/*
 * @Goat: draw text as a sdf mode
 */
[[maybe_unused]]
auto drawTextSDF(Font const &    font,
                 f32 const       fontSize,
                 f32 const       fontSpacing,
                 str const &     msg,
                 Vector2 const & position,
                 Shader const &  shader,
                 Color const &   color) noexcept -> void
{
    BeginShaderMode(shader);
    DrawTextEx(font, msg.c_str(), position, fontSize, fontSpacing, color);
    EndShaderMode();
}
}  // namespace RA_Font
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
* @Note: if you want opeque picture zero out the alpha on backgroundColor
* @Note: Alpha is btw 0 and 255
 TODO: make it lazy load
*/
[[nodiscard]] [[maybe_unused]]
auto genGridTexture(GridInfo const & grid,
                    f32 const        resulationScale = 1.f,
                    f32 const        lineThickness   = 10.f,
                    Color const      lineColor       = WHITE,
                    Color const backgroundColor = BLACK) noexcept -> Texture2D
{
    Image img = GenImageColor(cast(i32, grid.rect.width * resulationScale),
                              cast(i32, grid.rect.height * resulationScale),
                              backgroundColor);
    // draw in between lines based on col and row
    // drawing row lines
    f64 yOffset {cast(f64, lineThickness) * 0.5 * resulationScale};
    for (u16 i {}; i <= grid.rowCount; ++i)
    {
        Vector2 const v0 {0.f, cast(f32, yOffset)};
        Vector2 const v1 {grid.rect.width * resulationScale, cast(f32, yOffset)};
        if (i != 0 && i != grid.rowCount)
            ImageDrawLineEx(&img,
                            v0,
                            v1,
                            cast(i32, lineThickness * resulationScale),
                            lineColor);
        yOffset += cast(f64,
                        (grid.cellSize.y - (lineThickness * 0.5f)) * resulationScale);
    }
    // draw column lines
    f64 xOffset {cast(f64, lineThickness) * 0.5 * resulationScale};
    for (u16 i {}; i <= grid.columnCount; ++i)
    {
        Vector2 const v0 {cast(f32, xOffset), 0.f};
        Vector2 const v1 {cast(f32, xOffset), grid.rect.height * resulationScale};
        if (i != 0 && i != grid.rowCount)
            ImageDrawLineEx(&img,
                            v0,
                            v1,
                            cast(i32, lineThickness * resulationScale),
                            lineColor);
        xOffset += cast(f64,
                        (grid.cellSize.x - (lineThickness * 0.5f)) * resulationScale);
    }
    Texture2D gridTexture = LoadTextureFromImage(img);
    // #if defined(DEBUG)
    // ExportImage(img, "myimg.png");
    // #endif
    UnloadImage(img);
    return gridTexture;
}

/*
 * @Goat: return the grid-cell(Rectangle) based on input point if is inside the grid
 */
[[nodiscard]] [[maybe_unused]]
auto point2RectOnGrid(Vector2 const & point, GridInfo const & grid) noexcept
    -> std::optional<Rectangle>
{
    // sanity check
    checkAtRuntime((grid.cellSize.x == 0.f || grid.cellSize.y == 0.f ||
                    grid.columnCount == 0 || grid.rowCount == 0),
                   "grid cell size or Row/Col count should not be zero"sv);
    checkAtRuntime((point.x < 0.f || point.y < 0.f),
                   "point should not have negative value"sv);

    // does this point is inside the grid
    if (point.x < grid.rect.x || (point.x - grid.rect.x) > grid.rect.width ||
        point.y < grid.rect.y || (point.y - grid.rect.y) > grid.rect.height)
        return std::nullopt;

    auto const tempReminderX = cast(u8, (point.x - grid.rect.x) / grid.cellSize.x);
    auto x1 = cast(u16, (tempReminderX * grid.cellSize.x) + grid.rect.x);

    if (tempReminderX == 0)
        x1 = cast(u16, grid.rect.x);
    else if (tempReminderX >= grid.columnCount)
        x1 = cast(u16, ((grid.columnCount - 1) * grid.cellSize.x) + grid.rect.x);

    auto const tempReminderY = cast(u8, (point.y - grid.rect.y) / grid.cellSize.y);
    auto y1 = cast(u16, (tempReminderY * grid.cellSize.y) + grid.rect.y);
    if (tempReminderY == 0)
        y1 = cast(u16, grid.rect.y);
    else if (tempReminderY >= grid.rowCount)
        y1 = cast(u16, ((grid.rowCount - 1) * grid.cellSize.y) + grid.rect.y);
    return Rectangle {cast(f32, x1),
                      cast(f32, y1),
                      grid.cellSize.x,
                      grid.cellSize.y};
}

/*
 * @Goat: return the index of grid cell based on the input point
 * @Warning:index should be checked by caller and should not be ZERO
 */
[[nodiscard]] [[maybe_unused]]
auto point2IndexOnGrid(Vector2 const & point, GridInfo const & grid) noexcept -> u16
{
    // sanity check
    checkAtRuntime((grid.cellSize.x == 0.f || grid.cellSize.y == 0.f ||
                    grid.columnCount == 0 || grid.rowCount == 0),
                   "grid cell size or Row/Col count should not be zero"sv);
    checkAtRuntime((point.x < 0.f || point.y < 0.f),
                   "input point should not have negative value"sv);

    // does this point is inside the grid
    if (point.x < grid.rect.x || (point.x - grid.rect.x) > grid.rect.width ||
        point.y < grid.rect.y || (point.y - grid.rect.y) > grid.rect.height)
        return 0;

    auto const tempReminderX = cast(u16, (point.x - grid.rect.x) / grid.cellSize.x);
    auto const tempReminderY = cast(u16, (point.y - grid.rect.y) / grid.cellSize.y);
    u16 const totalLength {cast(u16, grid.columnCount * grid.rowCount)};

    return cast(u16,
                (totalLength -
                 ((tempReminderX) + (tempReminderY * grid.columnCount))));
}

/*
 * @Goat: return the top-left corner point of the Rectangle(grid cell) inside
 * the grid based on input index(the output Point cordinate start from zero)
 * @Warning: index does start from 1 and should not be Zero
 * // TODO: this function just work with static col/row number 3*3
 */
[[nodiscard]] [[maybe_unused]]
auto index2PointOnGrid(u16 const index, GridInfo const & grid) noexcept -> Vector2
{
    checkAtRuntime((index == 0), "index should start from ONE not Zero"sv);
    checkAtRuntime((index > (grid.columnCount * grid.rowCount)),
                   "index is not correct e.g:(1 to row*col)"sv);

    i32        ix              = 0;
    auto const inBetweenCountX = grid.columnCount - 2;
    u16 const  offsetX         = ((grid.columnCount * grid.rowCount) - index) %
                        grid.columnCount;
    auto const endX           = grid.columnCount - 1;
    u8         tempInBetweenX = (offsetX < inBetweenCountX)
                                    ? cast(u8, inBetweenCountX - offsetX)
                                    : 0;
    for (u16 i = 0; i < grid.columnCount; i++)
    {
        if (offsetX == 0)  // start
        {
            ix = 0;
            break;
        }
        if (offsetX == endX)  // end
            ix = endX;
        else if (i < (endX) && i > 0 && (tempInBetweenX < inBetweenCountX))  // middle
        {
            ix++;
            tempInBetweenX++;
        }
    }

    i32        iy              = 0;
    auto const inBetweenCountY = grid.rowCount - 2;
    u16 const offsetY = ((grid.columnCount * grid.rowCount) - index) / grid.rowCount;
    u8         tempInBetweenY = (offsetY < inBetweenCountY)
                                    ? cast(u8, inBetweenCountY - offsetY)
                                    : 0;
    auto const endY           = grid.rowCount - 1;
    for (u16 i = 0; i < grid.rowCount; i++)
    {
        if (offsetY == (0))  // start
        {
            iy = 0;
            break;
        }
        if (offsetY == (endY))  // end
            iy = endY;
        else if (i < (endY) && i > 0 && (tempInBetweenY < inBetweenCountY))  // middle
        {
            iy++;
            tempInBetweenY++;
        }
    }
    i32 const x = cast(i32, grid.rect.x + (cast(f32, ix) * grid.cellSize.x));
    i32 const y = cast(i32, grid.rect.y + (cast(f32, iy) * grid.cellSize.y));
    return {cast(f32, x), cast(f32, y)};
}

/*
 * @Goat: return the center of the Rectangle(grid cell) inside the grid based on input index
 * @Warning: index does start from 1 and should not be Zero
 */
[[nodiscard]] [[maybe_unused]]
auto index2CenterPointOnGrid(u16 const index, GridInfo const & grid) noexcept
    -> Vector2
{
    auto temp = index2PointOnGrid(index, grid);
    return Vector2 {temp.x + (grid.cellSize.x / 2.f),
                    temp.y + (grid.cellSize.y / 2.f)};
}

/*
 * @Goat: return the Rectangle(grid cell) inside the grid based on input index
 * @Warning: index does start from 1 and should not be Zero
 */
[[nodiscard]] [[maybe_unused]]
auto index2RectOnGrid(u16 const index, GridInfo const & grid) noexcept -> Rectangle
{
    auto const point = index2PointOnGrid(index, grid);
    return Rectangle {.x      = point.x,
                      .y      = point.y,
                      .width  = grid.cellSize.x,
                      .height = grid.cellSize.y

    };
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

template <std::size_t size>
[[maybe_unused]]
auto defineCircles(RA_Util::GridInfo const &    gridInfo,
                   std::array<u8, size> const & indexesCausesWin)
    -> std::array<Vector2, size>
{
    std::array<Vector2, size> circles {};
    for (size_t i = 0; i < indexesCausesWin.size(); ++i)
    {
        circles[i] = RA_Util::index2CenterPointOnGrid(indexesCausesWin[i], gridInfo);
    }
    return circles;
}

template <std::size_t size>
[[maybe_unused]]
auto drawAnimCircles(u32 const                         currentFrame,
                     u32 &                             inFrameLimit,
                     u8 &                              inAnimState,
                     u32 const                         resetFrame,
                     u32 const                         jumpFrame,
                     u16 const                         jumpState,
                     std::array<Vector2, size> const & circles,
                     Color const                       color) -> void
{
    for (u16 i = 0; i < cast(u16, circles.size()); ++i)
    {
        if (currentFrame == inFrameLimit)
        {
            inFrameLimit += jumpFrame;
            inAnimState += jumpState;
        }
        if (i < inAnimState && i < cast(u16, circles.size()))
        {
            DrawCircle(cast(i32, circles[i].x), cast(i32, circles[i].y), 25.f, color);
        }
    }
    if (inFrameLimit > resetFrame)
        inFrameLimit = jumpFrame;
}


}  // namespace RA_Anim
namespace RA_UI
{
// struct UIicon
// {
//     Rectangle layout;

//     // *index start from 0
//     // *(index < 0) means not init
//     i32 const iconTextureIndx;

//     // Shader shader;
//     // animate

//     // private:

//     // *index of this item in owner array
//     // *index start from 0
//     // *(index < 0) means not init
//     i32 const id;
// };
struct UILable
{
    Vector2 position;
    Color   textColor;

    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    u32 const fontIndx;
    // lable allways has a text
    // an indexid to hash table
    u64 const textIndx;

    f32     fontSize;
    f32     fontSpacing;
    Vector2 textSize;
    // Shader shader;

    // private:

    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    i32 const id;
    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    i32 const parentInxd;
    bool      isHidden {false};
};

struct UIRouondLayout
{
    Rectangle rect;
    f32       borderRoundness;
    f32       lineThickness;
    Color     borderColor;


    // private:
    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    i32 const id;
    bool      isHidden {false};
};
// normat layout like rectangle without rounded corners
struct UILayout
{
    Rectangle rect;
    f32       rotation;
    Color     backgroundColor;

    // can be an array of lable or icon

    // private:

    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    i32 const id;
    bool      isHidden {false};
};

struct UIButton
{
    // *index of this item in owner array
    // *index start from 0
    // *(index < 0) means not init
    i32 const id;
    i32 const layoutIndx;
    i32 const lableIndx;
    // i32 const iconIndx;
    bool isHidden {false};
    // sound
    // shader
};
std::vector<UIRouondLayout> layoutArray;
std::vector<str>            lablesTextArray;
std::vector<UILable>        lablesArray;
// std::vector<UIicon>         iconsArray;
std::vector<UIButton> buttonsArray;
std::vector<Font>     fontsArray;

auto getFontID(Font const & font) -> u32
{
    static u32 fontID {0};
    fontsArray.emplace_back(font);
    return fontID++;
}

auto initRoundButton(i32 const  layoutIndx,
                     i32 const  lableIndx /*, i32 const iconIndx */,
                     bool const isHidden = false) -> i32
{
    static i32     btnID {0};
    UIButton const btn {.id         = btnID++,
                        .layoutIndx = layoutIndx,
                        .lableIndx  = lableIndx,
                        .isHidden   = isHidden};
    // TODO: add icon
    //  .iconIndx   = iconIndx};
    buttonsArray.emplace_back(btn);
    return (btnID - 1);
}

auto initRoundLayout(Rectangle const & rect,
                     f32 const         borderRoundness,
                     f32 const         lineThickness,
                     Color const &     borderColor,
                     bool const        isHidden = false) -> i32
{
    static i32           layID {0};
    UIRouondLayout const layout {.rect            = rect,
                                 .borderRoundness = borderRoundness,
                                 .lineThickness   = lineThickness,
                                 .borderColor     = borderColor,
                                 .id              = layID++,
                                 .isHidden        = isHidden};
    layoutArray.emplace_back(layout);
    return (layID - 1);
}

// position 0,0 is centerned by default
auto makeLable(char const*     text,
               Color const &   txtColor,
               u32 const       fontIndx         = 0,
               u16 const       fontSize         = 20,
               Vector2 const & position         = {0, 0},
               f32 const       fontSpace        = 2.f,
               bool const      isHidden         = false,
               i32 const       parentLayoutIndx = -1) -> i32
{
    RA_Util::checkAtRuntime((fontSize <= 0), "font scale should not be zero");
    RA_Util::checkAtRuntime((fontIndx < 0), "font index is not valid"sv);

    Font const currentFont = fontsArray[cast(u32, fontIndx)];
    f32 const currentFontSize = cast(f32, currentFont.baseSize) * (fontSize / 100.f);
    static i32 lblID {0};
    lablesTextArray.emplace_back(text);

    UILable lable {.position    = position,
                   .textColor   = txtColor,
                   .fontIndx    = fontIndx,
                   .textIndx    = (lablesTextArray.size() - 1),
                   .fontSize    = currentFontSize,
                   .fontSpacing = fontSpace,
                   .textSize = MeasureTextEx(currentFont, text, currentFontSize, fontSpace),
                   .id         = lblID++,
                   .parentInxd = parentLayoutIndx,
                   .isHidden   = isHidden};

    if (parentLayoutIndx >= 0)
    {
        Rectangle const parentLayout = layoutArray[cast(u32, parentLayoutIndx)].rect;
        lable.position.x = parentLayout.x + (parentLayout.width / 2.f) -
                           lable.textSize.x / 2.f;
        lable.position.y = parentLayout.y + (parentLayout.height / 2.f) -
                           lable.textSize.y / 2;
    }

    lablesArray.emplace_back(lable);
    return (lblID - 1);
}
auto updateLable(u32 const       lableID,
                 char const*     text,
                 Color const &   txtColor,
                 u16 const       fontSize = 20,
                 bool const      isHidden = false,
                 Vector2 const & position = {0, 0})
{
    lablesTextArray[lablesArray[lableID].textIndx] = text;
    lablesArray[lableID].textColor                 = txtColor;
    lablesArray[lableID].fontSize                  = fontSize;
    lablesArray[lableID].position                  = position;
    lablesArray[lableID].isHidden                  = isHidden;
}
auto drawRoundLayout(i32 const layoutID) -> void
{
    RA_Util::checkAtRuntime((layoutID < 0), "layout id is invalid");

    auto const lay = layoutArray[cast(u32, layoutID)];
    if (lay.isHidden)
        return;
    DrawRectangleRoundedLinesEx(lay.rect,
                                lay.borderRoundness,
                                1,
                                lay.lineThickness,
                                lay.borderColor);
}

// font scale should be bigger than 0.0
auto drawLable(u32 const lableID) -> void
{
    RA_Util::checkAtRuntime((lableID < 0), "lable id is invalid");
    UILable const & lable       = lablesArray[lableID];
    Font const &    currentFont = fontsArray[lable.fontIndx];
    auto const      text        = lablesTextArray[lable.textIndx].c_str();

    if (text == " " || lable.fontSize == 0 || lable.isHidden)
        return;

    DrawTextEx(currentFont,
               text,
               lable.position,
               lable.fontSize,
               lable.fontSpacing,
               lable.textColor);
}

// auto drawIcon(u32 const iconId)
// {
// }

auto drawRoundButton(u32 const btnID)
{
    RA_Util::checkAtRuntime((btnID < 0), "btn id is invalid");
    auto const currentBtn = buttonsArray[btnID];
    if (currentBtn.isHidden)
        return;
    if (currentBtn.layoutIndx >= 0)
        drawRoundLayout(currentBtn.layoutIndx);
    // if (currentBtn.iconIndx >= 0)
    //     drawIcon(currentBtn.iconIndx);
    if (currentBtn.lableIndx >= 0)
        drawLable(currentBtn.lableIndx);
}

auto makeRoundButton(Rectangle const & rect,
                     f32 const         borderRoundness,
                     f32 const         borderThickness,
                     Color const &     borderColor,
                     char const*       text,
                     Color const &     txtColor,
                     u16 const         fontSize,
                     bool const        isHidden,
                     u32 const         fontIndx = 0) -> i32
{
    i32 const layid = initRoundLayout(rect,
                                      borderRoundness,
                                      borderThickness,
                                      borderColor,
                                      isHidden);
    RA_Util::checkAtRuntime((layid < 0), "layout id is negative");
    i32 const lblid = makeLable(text, txtColor, fontIndx, fontSize, Vector2 {}, 2.f, isHidden, layid);
    RA_Util::checkAtRuntime((lblid < 0), "lable id is negative");
    i32 const btnid = initRoundButton(layid,
                                      lblid
                                      /* , i32 const iconIndx*/,
                                      isHidden);
    RA_Util::checkAtRuntime((btnid < 0), "btn id is negative");
    return btnid;
}

auto getBtnRect(u32 const btnID) -> Rectangle
{
    return layoutArray[cast(u32, buttonsArray[btnID].layoutIndx)].rect;
}
}  // namespace RA_UI

struct ColoredRect
{
    Rectangle rect;
    Color     color;
    u8        id;
    ColoredRect()  = default;
    ~ColoredRect() = default;
    auto operator=(ColoredRect const & rhs) -> ColoredRect &
    {
        if (this == &rhs)
            return *this;
        rect  = rhs.rect;
        color = rhs.color;
        id    = rhs.id;
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

// game glob vars
i32                    gHeight {0};
i32                    gWidth {0};
u32                    winUIFramCounter {0};
u32                    winFrameLimit {10};
u32                    winAnimResetFrame {40};
u8                     winAnimState {1};
u32                    inputFramCounter {0};
bool                   canRegister {false};
bool                   canReset {false};
Vector2                uIPointAnimationWin {0.f, 0.f};
Vector2                mousePos {0.f, 0.f};
GameState              currentState {GameState::none};
RA_Util::GRandom const gRandom(-1200.f, 1500.f);
// constexpr u16 const    fps {60};
constexpr u8 const row    = 4;
constexpr u8 const column = 4;
constexpr u8 const goal   = 4;
struct Player
{
    std::bitset<row * column> moves;
    Color const               rectColor;
    str const                 name;
    i32 const                 id;
};
// clang-format off
// win condition should check this table to state the winner
// win table for 3x3
// inline static constexpr std::array<std::bitset<row*column>, 8> const winTable 
// {
//     0x007, 0x038,
//     0x049, 0x054,
//     0x092, 0x111, 
//     0x124, 0x1c0
// };
// win table for 4x4
inline static constexpr std::array<std::bitset<row*column>, 10> const winTable 
{
    0b1111000000000000,0b0000111100000000,0b0000000011110000,0b0000000000001111,
    0b1000100010001000,0b0100010001000100,0b0010001000100010,0b0001000100010001,
    0b1000010000100001,0b0001001001001000
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
            force = -(800 + gRandom.getRandom());
        else
            force = 1600 + gRandom.getRandom();
        b2Body_Enable(pr.bodyID);
        b2Body_ApplyForceToCenter(pr.bodyID,
                                  b2Vec2 {.x = force * gRandom.getRandom(),
                                          .y = force * gRandom.getRandom()},
                                  true);
        i++;
    }
}

[[maybe_unused]]
auto impulseParticle(Particle const & pr) noexcept -> void
{
    f32 force = 0.f;
    if (cast(u16, gRandom.getRandom()) % 2 == 0)
        force = -(800 + gRandom.getRandom());
    else
        force = 1600 + gRandom.getRandom();
    b2Body_Enable(pr.bodyID);
    b2Body_ApplyForceToCenter(pr.bodyID,
                              b2Vec2 {.x = force * gRandom.getRandom(),
                                      .y = force * gRandom.getRandom()},
                              true);
}

auto isClippingForRender(Vector2 const & pos, Rectangle const & boundry) noexcept
    -> bool
{
    // Demorgan law
    return (pos.y > boundry.height || pos.x > boundry.width || pos.x < boundry.x);
}


[[maybe_unused]]
auto resetParticles(std::vector<Particle> const & particles) noexcept -> void
{
    for (auto const pr : particles)
    {
        b2Body_SetTransform(pr.bodyID,
                            b2Vec2 {.x = -1.f * gRandom.getRandom(),
                                    .y = (gHeight / 3.f)},
                            b2MakeRot(0.f));
        b2Body_Disable(pr.bodyID);
    }
}
[[maybe_unused]]
auto resetParticle(Particle const & pr) noexcept -> void
{
    b2Body_SetTransform(pr.bodyID,
                        b2Vec2 {.x = -1.f * gRandom.getRandom(),
                                .y = (gHeight / 3.f)},
                        b2MakeRot(0.f));
    b2Body_Disable(pr.bodyID);
}
[[maybe_unused]]
auto drawParticles(std::vector<Particle> const & particles,
                   Texture2D const &             texture,
                   Color                         color) noexcept -> void
{
    Rectangle const boundRect {.x      = -300.f,
                               .y      = 0,
                               .width  = cast(f32, gWidth),
                               .height = gHeight + 50.f};

    for (auto const pr : particles)
    {
        if (isClippingForRender(Vector2 {.x = (b2Body_GetPosition(pr.bodyID).x * -1),
                                         .y = (b2Body_GetPosition(pr.bodyID).y * -1)},
                                boundRect))
        {
            // b2Body_Disable(pr.bodyID);
            resetParticle(pr);
            impulseParticle(pr);
        }
        else
        {
            b2Vec2 const boxPos {b2Body_GetPosition(pr.bodyID)};
            DrawTextureEx(texture, Vector2 {-boxPos.x, -boxPos.y}, 0.f, 3.f, color);
        }
    }
}
}  // namespace RA_Particle

}  // namespace


auto main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) -> int
{

    // init
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_TOPMOST |
                   FLAG_WINDOW_RESIZABLE);

    InitWindow(0, 0, "XOXO");

    gHeight = GetMonitorHeight(GetCurrentMonitor());
    gWidth  = GetMonitorWidth(GetCurrentMonitor());
    if (gWidth < 1920 || gHeight < 1080)
    {
        gHeight = GetScreenHeight();
        gWidth  = GetScreenWidth();
    }
    SetWindowSize(gWidth, gHeight);
    SetConfigFlags(FLAG_WINDOW_UNFOCUSED);
    SetWindowFocused();
    auto const fps = GetMonitorRefreshRate(0);
    ToggleFullscreen();
    ToggleBorderlessWindowed();
    SetTargetFPS(fps);
    SetWindowFocused();

    // clang-format off
    auto const [font,fontSize] = RA_Font::initFont(
        "NotoSans-VariableFont_wdth,wght.ttf"sv,
        100,
        TEXTURE_FILTER_BILINEAR);
    auto const defaultFontID=        RA_UI::getFontID(font);
    // clang-format on

    // box2d init of the world of the game (box2d-related)
    // Simulating setting (box2d-related)
    b2WorldId const       worldID = RA_Particle::initWorldOfBox2d();
    constexpr f32 const   timeStep {1.f / 60.f};  // 30HZ
    constexpr u8 const    subStepCount {3};
    std::vector<Particle> particles {};
    particles.reserve(1000);
    // create dynamic bodies
    for (u16 i {}; i < 1000; ++i)
    {
        Particle pr {};
        pr.rect.x      = gRandom.getRandom();
        pr.rect.y      = ((gHeight / 3.f) * -1);
        pr.rect.height = 15;
        pr.rect.width  = 15;
        pr.bodyID      = RA_Particle::creatDynamicBody(pr, worldID);
        particles.emplace_back(pr);
    }

    auto const gridinfo = RA_Util::
        createGridInfo(RA_Util::placeRelativeCenter(Rectangle {0.f,
                                                               0.f,
                                                               cast(f32, gWidth),
                                                               cast(f32, gHeight)},
                                                    50,
                                                    80),
                       column,
                       row);

    Texture2D const gridTexture {
        RA_Util::genGridTexture(gridinfo, .1f, 5.0f, WHITE, Color {0, 0, 0, 0})};

    // touched rects
    std::vector<ColoredRect> rects;
    rects.reserve(row * column);
    // indexes of rects that caus win
    std::array<u8, goal>      indexCausWin {};
    std::array<Vector2, goal> circles {};
    Camera2D const            camera {.offset   = Vector2 {},
                                      .target   = Vector2 {},
                                      .rotation = 0.f,
                                      .zoom     = 1.0f};
    // player1
    Player p1 {.moves = {}, .rectColor = {200, 0, 0, 255}, .name = "Red"s, .id = 0};
    // player2
    Player p2 {.moves = {}, .rectColor = {0, 0, 230, 255}, .name = "Blue"s, .id = 1};

    // current player
    Player* currentPlayer = &p1;
    Player* wonPlayer {nullptr};

    // UI elements init
    // reset btn ui
    i32 const resetBtnID = RA_UI::
        makeRoundButton(Rectangle {.x = 50.f,
                                   .y = gridinfo.rect.y,
                                   .width = (100 * 400.f / gWidth) * 12.f,  // TODO: make this formula hide inside abstraction for all ui elemnt (btn ,lbl,etc ...)
                                   .height = (100 * 170.f / gHeight) * 7.f},
                        0.5f,
                        5.f,
                        WHITE,
                        "Reset",
                        WHITE,
                        (100 * 50 / fontSize),
                        false,
                        defaultFontID);
    // resulation lable
    RA_UI::makeLable(TextFormat("resulation : %d x %d", gWidth, gHeight),
                     WHITE,
                     defaultFontID,
                     25,
                     Vector2 {50.f, 10.f});
    // fps lable
    auto const fpsLblID = RA_UI::makeLable(TextFormat("FPS: %d", GetFPS()),
                                           WHITE,
                                           defaultFontID,
                                           25,
                                           Vector2 {50.f, 40.f});
    // turn lable
    // auto const turnLblID = RA_UI::makeLable("Turn",
    //                                         currentPlayer->rectColor,
    //                                         defaultFontID,
    //                                         fontSize,
    //                                         Vector2 {70.f, 350.f});
    // gameState lable
    auto const gameStateLblID = RA_UI::makeLable("",
                                                 currentPlayer->rectColor,
                                                 defaultFontID,
                                                 fontSize,
                                                 Vector2 {gWidth / 2.f, 5.f},
                                                 2.f,
                                                 true);

    // shader uniforms
    f32 const iRes[2] = {cast(f32, gWidth), cast(f32, gHeight)};
    f32       iTime {};

    // shader and render texture setup

    // particle render target and texture
    u8 const        size {64};
    Texture2D const particleTexture =
        {rlGetTextureIdDefault(), size, size, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RenderTexture2D const particleRenderTexture = LoadRenderTexture(size, size);

    // background texture and render texture setup
    Texture2D const shapeTexture = {rlGetTextureIdDefault(),
                                    cast(u16, gWidth * .5f),
                                    cast(u16, gHeight * .5f),
                                    1,
                                    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RenderTexture2D const
        backgourndRenderTexture = LoadRenderTexture(cast(u16, gWidth * .5f),
                                                    cast(u16, gHeight * .5f));


    // particle shader and render texture init
    Shader const particleShader = LoadShader(nullptr,
                                             RA_Global::pathToFile("particle.fs"sv,
                                                                   RA_Global::EFileType::Shader)
                                                 .c_str());
    i32 const    resLocParticle = GetShaderLocation(particleShader, "iRes");
    SetShaderValue(particleShader, resLocParticle, iRes, SHADER_UNIFORM_VEC2);

    // background shader
    f32 iColor[3] = {cast(f32, currentPlayer->rectColor.r),
                     cast(f32, currentPlayer->rectColor.g),
                     cast(f32, currentPlayer->rectColor.b)};

    Shader const
              backgroundShader  = LoadShader(nullptr,
                                      RA_Global::pathToFile("background.fs"sv,
                                                            RA_Global::EFileType::Shader)
                                          .c_str());
    i32 const resLocBackground  = GetShaderLocation(backgroundShader, "iRes");
    i32 const timeLocBackground = GetShaderLocation(backgroundShader, "iTime");
    i32 const colorLoc          = GetShaderLocation(backgroundShader, "iColor");
    SetShaderValue(backgroundShader, resLocBackground, iRes, SHADER_UNIFORM_VEC2);
    SetShaderValue(backgroundShader, timeLocBackground, &iTime, SHADER_UNIFORM_FLOAT);
    SetShaderValue(backgroundShader, colorLoc, iColor, SHADER_UNIFORM_VEC3);

    // Circular Cell Shader
    Shader const cellShader  = LoadShader(nullptr,
                                         RA_Global::pathToFile("CircleCell.fs"sv,
                                                               RA_Global::EFileType::Shader)
                                             .c_str());
    i32 const    resLocCell  = GetShaderLocation(cellShader, "iRes");
    i32 const    timeLocCell = GetShaderLocation(cellShader, "iTime");
    SetShaderValue(cellShader, resLocCell, iRes, SHADER_UNIFORM_VEC2);
    SetShaderValue(cellShader, timeLocCell, &iTime, SHADER_UNIFORM_FLOAT);

    // Cross Cell Shader
    Shader const crossShader      = LoadShader(nullptr,
                                          RA_Global::pathToFile("CrossCell.fs"sv,
                                                                RA_Global::EFileType::Shader)
                                              .c_str());
    i32 const    resLocCrossCell  = GetShaderLocation(crossShader, "iRes");
    i32 const    timeLocCrossCell = GetShaderLocation(crossShader, "iTime");
    SetShaderValue(crossShader, resLocCrossCell, iRes, SHADER_UNIFORM_VEC2);
    SetShaderValue(crossShader, timeLocCrossCell, &iTime, SHADER_UNIFORM_FLOAT);


    BeginTextureMode(particleRenderTexture);
    {
        ClearBackground(BLANK);
        {
            {
                BeginShaderMode(particleShader);
                DrawTexture(particleTexture, 0, 0, WHITE);
                EndShaderMode();
            }
        }
    }
    EndTextureMode();
    UnloadTexture(particleTexture);
    UnloadShader(particleShader);


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
                if (CheckCollisionPointRec(mousePos, RA_UI::getBtnRect(resetBtnID)) &&
                    canRegister)
                {
                    canReset = true;
                }
            }
        }
        // update
        {
            // update shader loc address
            float const tempTime = GetTime();
            float const period = 20.0f;  // full up+down cycle = 80 up + 80 down
            iTime              = 10.0f - fabs(fmod(tempTime, period) - 10.0f);
            // std::cout << iTime << '\n';

            SetShaderValue(backgroundShader,
                           timeLocBackground,
                           &iTime,
                           SHADER_UNIFORM_FLOAT);
            SetShaderValue(backgroundShader, colorLoc, iColor, SHADER_UNIFORM_VEC3);

            SetShaderValue(cellShader, timeLocCell, &iTime, SHADER_UNIFORM_FLOAT);

            SetShaderValue(crossShader, timeLocCrossCell, &iTime, SHADER_UNIFORM_FLOAT);

            // box2d Update world state (box2d-related)
            b2World_Step(worldID, timeStep, subStepCount);
            // update game state
            if (currentState == GameState::none)
            {
                // update game based on input
                if (canRegister)
                {
                    // what is the sub-rectangle on the grid + index of that rectangle in optional type
                    auto const selectedRect {
                        RA_Util::point2RectOnGrid(mousePos, gridinfo)};
                    // if player touch inside grid
                    if (selectedRect.has_value())
                    {
                        auto const indexRect = RA_Util::point2IndexOnGrid(mousePos,
                                                                          gridinfo);
                        // create new rect inside the rect that touched with player color
                        ColoredRect newRect {};
                        // new rect should adjust size and coordinate inside the parent (touched rect)
                        newRect.rect = RA_Util::placeRelativeCenter(selectedRect.value(),
                                                                    55,
                                                                    55);
                        // adjust color based on current player
                        newRect.color = currentPlayer->rectColor;
                        newRect.id    = currentPlayer->id;
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
                                if (counter == goal)
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
                                // update player color for background shader
                                iColor[0] = currentPlayer->rectColor.r;
                                iColor[1] = currentPlayer->rectColor.g;
                                iColor[2] = currentPlayer->rectColor.b;
                            }
                        }
                    }
                }
                // update win state
                if (currentState == GameState::win)
                {
                    wonPlayer = currentPlayer;
                    // index of rectangle to center point on that rectangle
                    uIPointAnimationWin = RA_Util::index2CenterPointOnGrid(indexCausWin[2],
                                                                           gridinfo);
                    circles = RA_Anim::defineCircles(gridinfo, indexCausWin);
                    uIPointAnimationWin = circles[goal - 1];
                    RA_Particle::impulseParticles(particles);
                    RA_UI::updateLable(gameStateLblID,
                                       str {wonPlayer->name + " Won"s}.c_str(),
                                       wonPlayer->rectColor,
                                       100,
                                       false,
                                       Vector2 {(gWidth / 2.f) -
                                                    (MeasureTextEx(font,
                                                                   str {
                                                                       wonPlayer->name + " Won"s}
                                                                       .c_str(),
                                                                   fontSize,
                                                                   2.f)
                                                         .x /
                                                     2),
                                                5.f});
                }
                // update tie state
                // touched rect buffer is full but no one won the game
                else if (rects.size() == (row * column) &&
                         currentState == GameState::none)
                {
                    currentState = GameState::tie;
                    iColor[0]    = 255;
                    iColor[1]    = 255;
                    iColor[2]    = 255;
                    RA_Particle::impulseParticles(particles);
                    RA_UI::updateLable(gameStateLblID,
                                       "Tie",
                                       WHITE,
                                       100,
                                       false,
                                       Vector2 {(gWidth / 2.f) -
                                                    (MeasureTextEx(font, "Tie", fontSize, 2.f)
                                                         .x /
                                                     2),
                                                5.f});
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
                winFrameLimit       = 10;
                winAnimState        = 1;
                uIPointAnimationWin = {};
                winAnimResetFrame   = {40};
                inputFramCounter    = {0};
                RA_UI::updateLable(gameStateLblID, "", WHITE, 100, true, Vector2 {});
                iColor[0] = currentPlayer->rectColor.r;
                iColor[1] = currentPlayer->rectColor.g;
                iColor[2] = currentPlayer->rectColor.b;
            }
            RA_UI::updateLable(fpsLblID,
                               TextFormat("FPS: %d", GetFPS()),
                               WHITE,
                               25,
                               false,
                               Vector2 {50.f, 40.f});

            // RA_UI::updateLable(turnLblID,
            //                    "Turn",
            //                    currentPlayer->rectColor,
            //                    fontSize,
            //                    (currentState == GameState::win ||
            //                     currentState == GameState::tie),
            //                    Vector2 {70.f, 350.f});
        }
        // draw game loop
        {
            ClearBackground(BLANK);
            BeginDrawing();
            BeginMode2D(camera);

            {
                BeginTextureMode(backgourndRenderTexture);
                {
                    ClearBackground(BLANK);
                    {
                        // writing background shader to render Target
                        {
                            BeginShaderMode(backgroundShader);
                            DrawTexture(shapeTexture, 0, 0, WHITE);
                            EndShaderMode();
                        }
                        {
                            // draw grid
                            DrawTextureEx(gridTexture,
                                          {gridinfo.rect.x -
                                               (gridinfo.rect.width * .25f),
                                           gridinfo.rect.y -
                                               (gridinfo.rect.height * 0.0625f)}, /* its (0.25f*0.25f)*/
                                          0.f,
                                          5.f, /*its aloso multiply by 2 when drawing on screen 5*2=10*/
                                          WHITE);
                        }
                        // writing cell shader to same render target
                        {
                            for (auto const & rect : rects)
                            {
                                if (rect.id == p1.id)
                                {
                                    BeginShaderMode(cellShader);
                                    DrawTexturePro(shapeTexture,
                                                   {.x      = 0.f,
                                                    .y      = 0.f,
                                                    .width  = gWidth * .5f,
                                                    .height = gHeight * .5f},
                                                   {.x = (rect.rect.x * .5f) -
                                                         (rect.rect.width * 0.25f),
                                                    .y = (gHeight * .5f) -
                                                         ((rect.rect.y * .5f) -
                                                          (rect.rect.height * .25f)) -
                                                         (rect.rect.height),
                                                    .width  = rect.rect.width,
                                                    .height = rect.rect.height},
                                                   {},
                                                   0.f,
                                                   WHITE);
                                    EndShaderMode();
                                }
                                else  // its p2
                                {
                                    BeginShaderMode(crossShader);
                                    DrawTexturePro(shapeTexture,
                                                   {.x      = 0.f,
                                                    .y      = 0.f,
                                                    .width  = gWidth * .5f,
                                                    .height = gHeight * .5f},
                                                   {.x = (rect.rect.x * .5f) -
                                                         (rect.rect.width * 0.25f),
                                                    .y = (gHeight * .5f) -
                                                         ((rect.rect.y * .5f) -
                                                          (rect.rect.height * .25f)) -
                                                         (rect.rect.height),
                                                    .width  = rect.rect.width,
                                                    .height = rect.rect.height},
                                                   {},
                                                   0.f,
                                                   WHITE);
                                    EndShaderMode();
                                }
                            }
                        }
                    }
                }
                EndTextureMode();
                // draw backgournd shader
                DrawTextureEx(backgourndRenderTexture.texture, {}, 0.f, 2.f, WHITE);
            }


            // state specific drawing animation and etc ...
            switch (currentState)
            {
                case GameState::none:
                {
                    break;
                }
                case GameState::win:
                {
                    winUIFramCounter++;
                    // animation of wining
                    constexpr Color const color = WHITE;
                    RA_Anim::drawAnimCircles(winUIFramCounter,
                                             winFrameLimit,
                                             winAnimState,
                                             winAnimResetFrame,
                                             10,
                                             1,
                                             circles,
                                             color);
                    if (winUIFramCounter >= 50)
                    {
                        RA_Util::moveTowards(uIPointAnimationWin, circles[0], 5);
                        DrawLineEx(uIPointAnimationWin, circles[goal - 1], 10.f, color);
                    }
                    if (winUIFramCounter >= 55)
                    {
                        RA_Particle::drawParticles(particles,
                                                   particleRenderTexture.texture,
                                                   wonPlayer->rectColor);
                    }
                    if (winUIFramCounter > 70)
                    {
                        winUIFramCounter = 55;
                    }
                    break;
                }
                case GameState::tie:
                {
                    RA_Particle::drawParticles(particles,
                                               particleRenderTexture.texture,
                                               WHITE);
                    break;
                }
                case GameState::end:
                    [[fallthrough]];

                default:
                    break;
            }
            // UI
            {
                // debug on screen write for path of the resource font
#if 0 
                            DrawText(TextFormat("path : %s ",
                                                RA_Global::pathToFile("NotoSans-"
                                                                      "VariableFont_wdth,"
                                                                      "wght.ttf",
                                                                      RA_Global::EFileType::Font)
                                                    .c_str()),
                                     200.f,
                                     gHeight / 2.f,
                                     20,
                                     GREEN);
#endif

                // draw lables
                for (auto const & lbl : RA_UI::lablesArray)
                    RA_UI::drawLable(cast(u32, lbl.id));

                // draw btns
                for (auto const & btn : RA_UI::buttonsArray)
                    RA_UI::drawRoundButton(cast(u32, btn.id));
            }
            // End UI
            EndMode2D();
            EndDrawing();
        }
    }
    // clean-up
    b2DestroyWorld(worldID);
    UnloadTexture(gridTexture);
    UnloadTexture(shapeTexture);
    UnloadRenderTexture(particleRenderTexture);
    UnloadRenderTexture(backgourndRenderTexture);
    UnloadFont(font);
    UnloadShader(backgroundShader);
    CloseWindow();
    return 0;
}