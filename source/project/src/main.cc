// goal: check an expresion in runtime if not android
// note: pass a true condition that you need like percent>100 fail
void checkAtRuntime(bool faildCondition, std::string const & errMsg) noexcept
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
// cordinate note: x and y should be 0<n<100 note: widthPercent and heightPercent
// calculate based on reminding space of parent width and height so if you say 100 it
// fill reminding space of width
Rectangle placeRelative(Rectangle const & parentInfo,
                        uint8_t const     xPercent,
                        uint8_t const     yPercent,
                        uint8_t const     widthPercent,
                        uint8_t const     heightPercent) noexcept
{
    // bounds checking on input args
    // debug only
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

struct gridInfo
{
    Rectangle rect;
    Vector2   cellSize;
    uint8_t   columnCount;
    uint8_t   rowCount;
};

// goal: initilizer function for gridInfo object
// note: col and row should be bigger than 2
gridInfo createGrid(Rectangle const & gridRect,
                    uint8_t const     columnCount = 2,
                    uint8_t const     rowCount    = 2)
{
    // col and row should be bigger than 2X2
    checkAtRuntime((columnCount < 2 || rowCount < 2),
                   "column and row should be bigger than 2");
    return gridInfo {.rect        = gridRect,
                     .cellSize    = Vector2 {gridRect.width / columnCount,
                                          gridRect.height / rowCount},
                     .columnCount = columnCount,
                     .rowCount    = rowCount};
}

static void draw2DGrid(gridInfo const & grid, Color const lineColor) noexcept
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
    // its zero
    else
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
// Define the operator== function outside the class
bool operator==(Rectangle const & lhs, Rectangle const & rhs)
{
    return (lhs.x == rhs.x && lhs.y == rhs.y);
}
int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv)
{
    InitWindow(0, 0, "Test");
    ToggleFullscreen();
    auto const             height   = GetScreenHeight();
    auto const             width    = GetScreenWidth();
    auto const             fps      = GetMonitorRefreshRate(0);
    int const              row      = 60;
    int const              column   = 90;
    auto const             offsety  = static_cast<float>(height) / row;
    auto const             offsetx  = static_cast<float>(width) / column;
    auto const             gridRect = placeRelative(Rectangle {0,
                                                   0,
                                                   static_cast<float>(width),
                                                   static_cast<float>(height)},
                                        0,
                                        0,
                                        100,
                                        100);
    auto const             gridinfo = createGrid(gridRect, column, row);
    std::vector<Rectangle> rects {};
    rects.reserve(1000);
    Vector2 mouse_pos {};
    SetTargetFPS(fps);

    bool isEnd {false};
    bool canRegister {false};

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
            mouse_pos = GetTouchPosition(0);
            Rectangle const temp {whereClicked(offsetx, offsety, mouse_pos)};
            if (std::find(rects.begin(), rects.end(), temp) == rects.end())
            {
                rects.emplace(std::begin(rects), temp);
                if (rects.size() > 550)
                {
                    rects.pop_back();
                }
            }
        }

        ClearBackground(BLACK);
        BeginDrawing();
        draw2DGrid(gridinfo, RAYWHITE);
        DrawText(TextFormat("height : %d \n width : %d", height, width),
                 width / 2,
                 height / 2,
                 22,
                 RAYWHITE);
        for (auto const & rect : rects)
        {
            DrawRectangleRec(rect, BLUE);
        }
        EndDrawing();
    }

    return 0;
}