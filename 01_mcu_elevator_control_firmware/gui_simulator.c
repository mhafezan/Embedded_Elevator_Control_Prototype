#include <stdio.h>
#include <stdbool.h>
#include "raylib.h"
#include "elevator_controller.h"

/*
 * Professional GUI-based elevator simulator using raylib.
 *
 * This GUI keeps the embedded controller logic separated from the visualization.
 * The graphical interface only sends user events to Elevator_Update().
 *
 * Compile on Windows/MSYS2 after installing raylib:
 *   gcc gui_simulator.c elevator_controller.c -o gui_simulator.exe -lraylib -lopengl32 -lgdi32 -lwinmm
 *
 * Run:
 *   ./gui_simulator.exe
 */

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 820

#define UPDATE_PERIOD_SECONDS 0.75f

#define SHAFT_CARD_X 28
#define SHAFT_CARD_Y 70
#define SHAFT_CARD_W 405
#define SHAFT_CARD_H 655

#define SHAFT_X 145
#define SHAFT_Y 90
#define SHAFT_W 245
#define SHAFT_H 605

#define PANEL_X 460
#define PANEL_Y 70
#define PANEL_W 710

#define BUTTON_W 92
#define BUTTON_H 38
#define BUTTON_GAP 12

#define CONTROL_BUTTON_W 230
#define CONTROL_BUTTON_H 46

static const Color COLOR_BG = {245, 247, 251, 255};
static const Color COLOR_CARD = {255, 255, 255, 255};
static const Color COLOR_CARD_BORDER = {218, 225, 235, 255};
static const Color COLOR_TEXT = {17, 35, 64, 255};
static const Color COLOR_MUTED = {90, 105, 128, 255};
static const Color COLOR_BLUE = {31, 105, 230, 255};
static const Color COLOR_BLUE_DARK = {12, 58, 130, 255};
static const Color COLOR_BLUE_LIGHT = {229, 239, 255, 255};
static const Color COLOR_GREEN = {0, 135, 62, 255};
static const Color COLOR_GREEN_LIGHT = {225, 247, 234, 255};
static const Color COLOR_RED = {220, 30, 38, 255};
static const Color COLOR_RED_LIGHT = {255, 232, 232, 255};
static const Color COLOR_ORANGE = {245, 111, 30, 255};
static const Color COLOR_ORANGE_LIGHT = {255, 239, 226, 255};
static const Color COLOR_GRAY_LIGHT = {237, 241, 247, 255};
static const Color COLOR_SHAFT = {238, 242, 247, 255};
static const Color COLOR_SHAFT_LINE = {196, 205, 218, 255};

static Font g_ui_font;

/*
 * raylib's default font is pixel-styled. For a smoother professional UI,
 * this simulator tries to load Calibri from Windows. If it is not available,
 * it safely falls back to raylib's default font.
 */
static void LoadUIFont(void)
{
    g_ui_font = LoadFontEx("C:/Windows/Fonts/calibri.ttf", 14, NULL, 0);

    if (g_ui_font.texture.id == 0)
    {
        g_ui_font = LoadFontEx("C:/Windows/Fonts/arial.ttf", 14, NULL, 0);
    }

    if (g_ui_font.texture.id == 0)
    {
        g_ui_font = GetFontDefault();
    }
    else
    {
        SetTextureFilter(g_ui_font.texture, TEXTURE_FILTER_BILINEAR);
    }
}

static void UnloadUIFont(void)
{
    if (g_ui_font.texture.id != GetFontDefault().texture.id)
    {
        UnloadFont(g_ui_font);
    }
}

static void GuiDrawText(const char *text, int x, int y, int font_size, Color color)
{
    DrawTextEx(g_ui_font, text, (Vector2){(float)x, (float)y}, (float)font_size, 1.0f, color);
}

static int GuiMeasureText(const char *text, int font_size)
{
    Vector2 size = MeasureTextEx(g_ui_font, text, (float)font_size, 1.0f);
    return (int)size.x;
}


static bool IsButtonClicked(Rectangle rect)
{
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static bool IsButtonHovered(Rectangle rect)
{
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

static void DrawCard(Rectangle rect)
{
    DrawRectangleRounded((Rectangle){rect.x + 4, rect.y + 5, rect.width, rect.height}, 0.035f, 14, (Color){214, 222, 234, 80});
    DrawRectangleRounded(rect, 0.035f, 14, COLOR_CARD);
    DrawRectangleRoundedLines(rect, 0.035f, 14, COLOR_CARD_BORDER);
}

static void DrawSoftDivider(float x1, float y1, float x2, float y2)
{
    DrawLineEx((Vector2){x1, y1}, (Vector2){x2, y2}, 1.0f, COLOR_CARD_BORDER);
}

static void DrawTextRight(const char *text, int x, int y, int font_size, Color color)
{
    int w = GuiMeasureText(text, font_size);
    GuiDrawText(text, x - w, y, font_size, color);
}

static void DrawCenteredText(const char *text, Rectangle rect, int font_size, Color color)
{
    int w = GuiMeasureText(text, font_size);
    GuiDrawText(text,
             (int)(rect.x + rect.width / 2 - w / 2),
             (int)(rect.y + rect.height / 2 - font_size / 2),
             font_size,
             color);
}

static void DrawCircleBadge(int center_x, int center_y, int radius, Color fill, Color line)
{
    DrawCircle(center_x, center_y, (float)radius, fill);
    DrawCircleLines(center_x, center_y, (float)radius, line);
}

static void DrawElevatorIcon(int x, int y, int size, Color color)
{
    Rectangle box = {(float)x, (float)y, (float)size, (float)size};
    DrawRectangleRounded(box, 0.18f, 8, COLOR_BLUE_LIGHT);
    DrawRectangleRoundedLines(box, 0.18f, 8, color);

    DrawLine(x + size / 2, y + 9, x + size / 2, y + size - 9, color);

    DrawCircle(x + size / 4, y + size / 2 - 5, size / 11.0f, color);
    DrawRectangleRounded((Rectangle){x + size / 4 - 5, y + size / 2 + 4, 10, 16}, 0.4f, 6, color);

    DrawCircle(x + 3 * size / 4, y + size / 2 - 5, size / 11.0f, color);
    DrawRectangleRounded((Rectangle){x + 3 * size / 4 - 5, y + size / 2 + 4, 10, 16}, 0.4f, 6, color);

    DrawTriangle((Vector2){x + size / 2 - 11, y - 9},
                 (Vector2){x + size / 2 - 3, y - 22},
                 (Vector2){x + size / 2 + 5, y - 9},
                 color);
    DrawTriangle((Vector2){x + size / 2 + 10, y - 22},
                 (Vector2){x + size / 2 + 18, y - 9},
                 (Vector2){x + size / 2 + 26, y - 22},
                 color);
}

static void DrawShieldIcon(int x, int y, Color color)
{
    Vector2 pts[5] = {
        {(float)x, (float)y},
        {(float)x + 16, (float)y + 7},
        {(float)x + 13, (float)y + 29},
        {(float)x, (float)y + 38},
        {(float)x - 13, (float)y + 29}};
    DrawTriangle(pts[0], pts[1], pts[4], color);
    DrawTriangle(pts[1], pts[2], pts[4], color);
    DrawTriangle(pts[4], pts[2], pts[3], color);
    DrawLineEx((Vector2){x - 5, y + 19}, (Vector2){x - 1, y + 24}, 3.0f, WHITE);
    DrawLineEx((Vector2){x - 1, y + 24}, (Vector2){x + 8, y + 14}, 3.0f, WHITE);
}

static void DrawWarningIcon(int x, int y)
{
    DrawCircle(x, y, 20.0f, COLOR_RED_LIGHT);
    DrawCircle(x, y, 15.0f, COLOR_RED);
    DrawTriangle((Vector2){x, y - 10}, (Vector2){x - 11, y + 10}, (Vector2){x + 11, y + 10}, WHITE);
    GuiDrawText("!", x - 3, y - 6, 15, COLOR_RED);
}

static void DrawDoorIcon(int x, int y, Color color)
{
    DrawCircle(x, y, 20.0f, COLOR_GRAY_LIGHT);
    DrawRectangleLinesEx((Rectangle){x - 11, y - 15, 15, 30}, 3.0f, color);
    DrawLineEx((Vector2){x, y - 15}, (Vector2){x, y + 15}, 2.0f, color);
}

static void DrawUpIcon(int x, int y, Color color)
{
    DrawCircle(x, y, 20.0f, COLOR_GRAY_LIGHT);
    DrawLineEx((Vector2){x - 8, y + 6}, (Vector2){x, y - 4}, 3.0f, color);
    DrawLineEx((Vector2){x, y - 4}, (Vector2){x + 8, y + 6}, 3.0f, color);
    DrawLineEx((Vector2){x - 8, y - 6}, (Vector2){x, y - 16}, 3.0f, color);
    DrawLineEx((Vector2){x, y - 16}, (Vector2){x + 8, y - 6}, 3.0f, color);
}

static void DrawDownIcon(int x, int y, Color color)
{
    DrawCircle(x, y, 20.0f, COLOR_GRAY_LIGHT);
    DrawLineEx((Vector2){x - 8, y - 6}, (Vector2){x, y + 4}, 3.0f, color);
    DrawLineEx((Vector2){x, y + 4}, (Vector2){x + 8, y - 6}, 3.0f, color);
    DrawLineEx((Vector2){x - 8, y + 6}, (Vector2){x, y + 16}, 3.0f, color);
    DrawLineEx((Vector2){x, y + 16}, (Vector2){x + 8, y + 6}, 3.0f, color);
}

static void DrawMetricRow(int icon_x,
                          int y,
                          Color badge_color,
                          const char *label,
                          const char *value,
                          Color value_color)
{
    DrawCircleBadge(icon_x, y + 16, 21, badge_color, (Color){220, 226, 235, 255});
    GuiDrawText(label, icon_x + 38, y + 3, 14, COLOR_TEXT);
    GuiDrawText(value, icon_x + 215, y + 3, 15, value_color);
}

static void DrawRequestButton(Rectangle rect, const char *text, bool active)
{
    bool hover = IsButtonHovered(rect);

    Color fill = active ? COLOR_BLUE : (hover ? COLOR_BLUE_LIGHT : WHITE);
    Color border = active ? COLOR_BLUE_DARK : COLOR_BLUE;
    Color text_color = active ? WHITE : COLOR_BLUE_DARK;

    DrawRectangleRounded((Rectangle){rect.x + 2, rect.y + 3, rect.width, rect.height}, 0.12f, 12, (Color){160, 175, 200, 60});
    DrawRectangleRounded(rect, 0.12f, 12, fill);
    DrawRectangleRoundedLines(rect, 0.12f, 12, border);

    DrawCenteredText(text, rect, 14, text_color);
}

static void DrawControlButton(Rectangle rect, const char *text, int type, bool active)
{
    bool hover = IsButtonHovered(rect);
    Color fill = active ? COLOR_RED_LIGHT : (hover ? COLOR_GRAY_LIGHT : WHITE);
    Color border = active ? COLOR_RED : (Color){185, 197, 213, 255};

    DrawRectangleRounded((Rectangle){rect.x + 2, rect.y + 3, rect.width, rect.height}, 0.12f, 12, (Color){160, 175, 200, 45});
    DrawRectangleRounded(rect, 0.12f, 12, fill);
    DrawRectangleRoundedLines(rect, 0.12f, 12, border);

    int icon_x = (int)rect.x + 36;
    int icon_y = (int)rect.y + 28;

    if (type == 0)
        DrawWarningIcon(icon_x, icon_y);
    else if (type == 1)
        DrawDoorIcon(icon_x, icon_y, COLOR_MUTED);
    else if (type == 2)
        DrawUpIcon(icon_x, icon_y, COLOR_TEXT);
    else
        DrawDownIcon(icon_x, icon_y, COLOR_TEXT);

    GuiDrawText(text, (int)rect.x + 78, (int)rect.y + 18, 14, COLOR_TEXT);
}

static void DrawElevatorShaft(const ElevatorController *controller)
{
    DrawCard((Rectangle){SHAFT_CARD_X, SHAFT_CARD_Y, SHAFT_CARD_W, SHAFT_CARD_H});

    DrawRectangleRounded((Rectangle){SHAFT_X - 20, SHAFT_Y - 12, SHAFT_W + 40, SHAFT_H + 24}, 0.035f, 14, (Color){232, 237, 245, 255});
    DrawRectangleRoundedLines((Rectangle){SHAFT_X - 20, SHAFT_Y - 12, SHAFT_W + 40, SHAFT_H + 24}, 0.035f, 14, COLOR_CARD_BORDER);

    DrawRectangleGradientV(SHAFT_X, SHAFT_Y, SHAFT_W, SHAFT_H, (Color){250, 252, 255, 255}, COLOR_SHAFT);
    DrawRectangleLinesEx((Rectangle){SHAFT_X, SHAFT_Y, SHAFT_W, SHAFT_H}, 2.0f, (Color){80, 96, 115, 255});

    /* Elevator guide rails */
    DrawRectangle(SHAFT_X + 5, SHAFT_Y, 7, SHAFT_H, (Color){46, 57, 73, 255});
    DrawRectangle(SHAFT_X + 16, SHAFT_Y, 3, SHAFT_H, (Color){150, 160, 172, 255});
    DrawRectangle(SHAFT_X + SHAFT_W - 12, SHAFT_Y, 7, SHAFT_H, (Color){46, 57, 73, 255});
    DrawRectangle(SHAFT_X + SHAFT_W - 19, SHAFT_Y, 3, SHAFT_H, (Color){150, 160, 172, 255});

    int floor_count = MAX_FLOOR - MIN_FLOOR + 1;
    float floor_h = (float)SHAFT_H / floor_count;

    for (int floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
    {
        int visual_index = MAX_FLOOR - floor;
        float y = SHAFT_Y + visual_index * floor_h;

        DrawLine(SHAFT_X, (int)y, SHAFT_X + SHAFT_W, (int)y, COLOR_SHAFT_LINE);

        char label[32];
        snprintf(label, sizeof(label), "Floor %d", floor);

        Color label_color = (floor == controller->current_floor) ? COLOR_BLUE : COLOR_TEXT;
        DrawTextRight(label, SHAFT_X - 28, (int)(y + floor_h / 2 - 10), 14, label_color);

        DrawLineEx((Vector2){SHAFT_X - 16, y + floor_h / 2},
                   (Vector2){SHAFT_X - 2, y + floor_h / 2},
                   2.0f,
                   label_color);

        if (floor == controller->target_floor)
        {
            DrawRectangleRoundedLines((Rectangle){SHAFT_X + 22, y + 7, SHAFT_W - 44, floor_h - 14},
                                      0.08f,
                                      12,
                                      COLOR_ORANGE);
        }

        if (floor == controller->current_floor)
        {
            Rectangle cabin = {SHAFT_X + 36, y + 9, SHAFT_W - 72, floor_h - 18};

            DrawRectangleRounded((Rectangle){cabin.x + 3, cabin.y + 4, cabin.width, cabin.height}, 0.035f, 10, (Color){40, 60, 100, 90});
            DrawRectangleGradientV((int)cabin.x, (int)cabin.y, (int)cabin.width, (int)cabin.height, (Color){63, 169, 245, 255}, COLOR_BLUE);
            DrawRectangleRoundedLines(cabin, 0.035f, 10, COLOR_BLUE_DARK);

            if (controller->door_open)
            {
                DrawLineEx((Vector2){cabin.x + cabin.width / 2, cabin.y + 5},
                        (Vector2){cabin.x + cabin.width / 2, cabin.y + cabin.height - 5},
                        3.0f,
                        WHITE);
                DrawCenteredText("DOOR OPEN", cabin, 14, WHITE);
            }
            else
            {
                DrawCenteredText("CABIN", cabin, 14, WHITE);
            }
        }
    }

    DrawLine(SHAFT_X, SHAFT_Y + SHAFT_H, SHAFT_X + SHAFT_W, SHAFT_Y + SHAFT_H, COLOR_SHAFT_LINE);
}

static void DrawHeader(void)
{
    DrawElevatorIcon(PANEL_X + 30, PANEL_Y + 30, 52, COLOR_BLUE_DARK);
    GuiDrawText("Elevator GUI Simulator", PANEL_X + 105, PANEL_Y + 27, 30, COLOR_TEXT);
    GuiDrawText("Embedded controller visualization dashboard", PANEL_X + 108, PANEL_Y + 72, 15, COLOR_MUTED);
}

static void DrawStatusCard(const ElevatorController *controller)
{
    Rectangle card = {PANEL_X, PANEL_Y, PANEL_W, 250};
    DrawCard(card);
    DrawHeader();

    Rectangle metrics = {PANEL_X + 24, PANEL_Y + 112, PANEL_W - 48, 105};
    DrawRectangleRounded(metrics, 0.04f, 12, (Color){250, 252, 255, 255});
    DrawRectangleRoundedLines(metrics, 0.04f, 12, COLOR_CARD_BORDER);

    DrawMetricRow(PANEL_X + 55, PANEL_Y + 132, COLOR_BLUE_LIGHT, "Current Floor:", TextFormat("%d", controller->current_floor), COLOR_BLUE);
    DrawMetricRow(PANEL_X + 55, PANEL_Y + 177, COLOR_ORANGE_LIGHT, "Target Floor:", TextFormat("%d", controller->target_floor), COLOR_ORANGE);

    DrawSoftDivider(PANEL_X + 352, PANEL_Y + 124, PANEL_X + 352, PANEL_Y + 210);

    GuiDrawText("State:", PANEL_X + 382, PANEL_Y + 136, 14, COLOR_TEXT);
    GuiDrawText(Elevator_StateToString(controller->state), PANEL_X + 520, PANEL_Y + 136, 15, COLOR_BLUE_DARK);

    GuiDrawText("Motor:", PANEL_X + 382, PANEL_Y + 172, 14, COLOR_TEXT);
    GuiDrawText(Elevator_MotorToString(controller->motor),
             PANEL_X + 520,
             PANEL_Y + 172,
             22,
             controller->motor == MOTOR_STOP ? COLOR_RED : COLOR_GREEN);

    GuiDrawText("Door:", PANEL_X + 382, PANEL_Y + 208, 14, COLOR_TEXT);
    GuiDrawText(controller->door_open ? "OPEN" : "CLOSED",
             PANEL_X + 520,
             PANEL_Y + 208,
             22,
             controller->door_open ? COLOR_ORANGE : COLOR_GREEN);
}

static void DrawSafetyCard(bool emergency_stop,
                           bool door_obstruction,
                           bool upper_limit_switch,
                           bool lower_limit_switch)
{
    Rectangle card = {PANEL_X, PANEL_Y + 272, PANEL_W, 118};
    DrawCard(card);

    DrawShieldIcon(PANEL_X + 38, PANEL_Y + 298, COLOR_BLUE_DARK);
    GuiDrawText("Safety Inputs", PANEL_X + 70, PANEL_Y + 300, 14, COLOR_TEXT);

    const char *labels[4] = {"Emergency Stop", "Door Obstruction", "Upper Limit", "Lower Limit"};
    bool values[4] = {emergency_stop, door_obstruction, upper_limit_switch, lower_limit_switch};

    for (int i = 0; i < 4; i++)
    {
        int x = PANEL_X + 35 + i * 168;
        int y = PANEL_Y + 345;

        DrawShieldIcon(x + 14, y, values[i] ? COLOR_RED : COLOR_GREEN);
        GuiDrawText(labels[i], x + 40, y + 1, 14, COLOR_TEXT);
        GuiDrawText(values[i] ? "ACTIVE" : "OFF", x + 40, y + 27, 15, values[i] ? COLOR_RED : COLOR_GREEN);

        if (i < 3)
            DrawSoftDivider(x + 155, y, x + 155, y + 45);
    }
}

static void DrawFloorButtons(int *pending_request_floor)
{
    Rectangle card = {PANEL_X, PANEL_Y + 408, 410, 255};
    DrawCard(card);

    GuiDrawText("Floor Request Buttons", PANEL_X + 52, PANEL_Y + 430, 14, COLOR_TEXT);

    for (int i = 0; i < 4; i++)
    {
        int dot_x = PANEL_X + 25 + (i % 2) * 12;
        int dot_y = PANEL_Y + 433 + (i / 2) * 12;
        DrawRectangleRounded((Rectangle){dot_x, dot_y, 8, 8}, 0.3f, 4, COLOR_BLUE_DARK);
    }

    int max_buttons_per_row = 3;
    int start_x = PANEL_X + 28;
    int start_y = PANEL_Y + 472;

    for (int floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
    {
        int index = floor - MIN_FLOOR;
        int col = index % max_buttons_per_row;
        int row = index / max_buttons_per_row;

        Rectangle btn = {
            start_x + col * (BUTTON_W + BUTTON_GAP),
            start_y + row * (BUTTON_H + BUTTON_GAP),
            BUTTON_W,
            BUTTON_H};

        if (MAX_FLOOR == 10 && floor == 10)
        {
            btn.x = start_x + BUTTON_W + BUTTON_GAP;
        }

        bool active = (*pending_request_floor == floor);
        DrawRequestButton(btn, TextFormat("%d", floor), active);

        if (IsButtonClicked(btn))
            *pending_request_floor = floor;
    }
}

static void DrawActionButtons(bool *emergency_stop,
                              bool *door_obstruction,
                              bool *upper_limit_switch,
                              bool *lower_limit_switch)
{
    Rectangle card = {PANEL_X + 430, PANEL_Y + 408, 280, 255};
    DrawCard(card);

    Rectangle e = {PANEL_X + 448, PANEL_Y + 430, CONTROL_BUTTON_W, CONTROL_BUTTON_H};
    Rectangle d = {PANEL_X + 448, PANEL_Y + 492, CONTROL_BUTTON_W, CONTROL_BUTTON_H};
    Rectangle u = {PANEL_X + 448, PANEL_Y + 554, CONTROL_BUTTON_W, CONTROL_BUTTON_H};
    Rectangle l = {PANEL_X + 448, PANEL_Y + 616, CONTROL_BUTTON_W, CONTROL_BUTTON_H};

    DrawControlButton(e, *emergency_stop ? "Clear Emergency" : "Emergency Stop", 0, *emergency_stop);
    DrawControlButton(d, *door_obstruction ? "Clear Obstruction" : "Door Obstruction", 1, *door_obstruction);
    DrawControlButton(u, *upper_limit_switch ? "Clear Upper Limit" : "Upper Limit", 2, *upper_limit_switch);
    DrawControlButton(l, *lower_limit_switch ? "Clear Lower Limit" : "Lower Limit", 3, *lower_limit_switch);

    if (IsButtonClicked(e))
        *emergency_stop = !(*emergency_stop);

    if (IsButtonClicked(d))
        *door_obstruction = !(*door_obstruction);

    if (IsButtonClicked(u))
        *upper_limit_switch = !(*upper_limit_switch);

    if (IsButtonClicked(l))
        *lower_limit_switch = !(*lower_limit_switch);
}

static void DrawFooter(int pending_request_floor)
{
    Rectangle footer = {28, 750, 1144, 52};
    DrawCard(footer);

    DrawCircle(60, 776, 17.0f, COLOR_BLUE);
    GuiDrawText("i", 56, 765, 21, WHITE);

    if (pending_request_floor != 0)
    {
        GuiDrawText(TextFormat("Pending request: Floor %d will be sent on the next controller cycle.", pending_request_floor),
                 92,
                 766,
                 20,
                 COLOR_ORANGE);
    }
    else
    {
        GuiDrawText("Click a floor button to send a request. Toggle safety buttons to simulate sensor conditions.",
                 92,
                 766,
                 20,
                 COLOR_MUTED);
    }
}

int main(void)
{
    ElevatorController controller;
    ElevatorInputs inputs;

    int pending_request_floor = 0;
    float update_timer = 0.0f;

    bool emergency_stop = false;
    bool door_obstruction = false;
    bool upper_limit_switch = false;
    bool lower_limit_switch = false;

    Elevator_Init(&controller);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Embedded Elevator Controller - GUI Simulation");
    LoadUIFont();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        update_timer += GetFrameTime();

        if (update_timer >= UPDATE_PERIOD_SECONDS)
        {
            /*
             * This version matches the improved controller structure:
             * a GUI floor click is treated as a cabin request.
             * Hall UP/DOWN requests are not used in this GUI yet.
             */
            inputs.cabin_request_floor = pending_request_floor;
            inputs.hall_up_request_floor = 0;
            inputs.hall_down_request_floor = 0;

            inputs.door_obstruction = door_obstruction;
            inputs.emergency_stop = emergency_stop;
            inputs.upper_limit_switch = upper_limit_switch;
            inputs.lower_limit_switch = lower_limit_switch;

            Elevator_Update(&controller, inputs);

            pending_request_floor = 0;
            update_timer = 0.0f;
        }

        BeginDrawing();

        ClearBackground(COLOR_BG);

        DrawElevatorShaft(&controller);
        DrawStatusCard(&controller);
        DrawSafetyCard(emergency_stop, door_obstruction, upper_limit_switch, lower_limit_switch);
        DrawFloorButtons(&pending_request_floor);
        DrawActionButtons(&emergency_stop, &door_obstruction, &upper_limit_switch, &lower_limit_switch);
        DrawFooter(pending_request_floor);

        EndDrawing();
    }

    UnloadUIFont();
    CloseWindow();
    return 0;
}
