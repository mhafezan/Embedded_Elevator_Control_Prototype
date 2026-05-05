#include <stdio.h>
#include <stdbool.h>
#include "raylib.h"
#include "elevator_controller.h"

/*
 * GUI-based elevator simulator using raylib.
 *
 * This file keeps the original elevator controller logic unchanged.
 * It only provides a graphical front-end for sending floor requests
 * and safety inputs to Elevator_Update().
 *
 * Build example on Windows with raylib:
 *   gcc gui_simulator.c elevator_controller.c -o gui_simulator.exe -lraylib -lopengl32 -lgdi32 -lwinmm
 */

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 720

#define SHAFT_X 80
#define SHAFT_Y 60
#define SHAFT_WIDTH 300
#define SHAFT_HEIGHT 600

#define PANEL_X 450
#define PANEL_Y 70

#define BUTTON_WIDTH 70
#define BUTTON_HEIGHT 38
#define BUTTON_GAP 10

#define UPDATE_PERIOD_SECONDS 0.8f

static bool IsButtonClicked(Rectangle rect)
{
    Vector2 mouse = GetMousePosition();
    return CheckCollisionPointRec(mouse, rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

static void DrawButton(Rectangle rect, const char *text)
{
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);

    DrawRectangleRec(rect, hover ? LIGHTGRAY : RAYWHITE);
    DrawRectangleLinesEx(rect, 2, DARKGRAY);

    int text_width = MeasureText(text, 18);
    DrawText(text,
             (int)(rect.x + rect.width / 2 - text_width / 2),
             (int)(rect.y + rect.height / 2 - 9),
             18,
             BLACK);
}

static void DrawElevatorShaft(const ElevatorController *controller)
{
    int floor_count = MAX_FLOOR - MIN_FLOOR + 1;
    float floor_height = (float)SHAFT_HEIGHT / floor_count;

    DrawRectangleLines(SHAFT_X, SHAFT_Y, SHAFT_WIDTH, SHAFT_HEIGHT, BLACK);

    for (int floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
    {
        int visual_index = MAX_FLOOR - floor;
        float y = SHAFT_Y + visual_index * floor_height;

        DrawLine(SHAFT_X, (int)y, SHAFT_X + SHAFT_WIDTH, (int)y, LIGHTGRAY);

        char floor_label[32];
        snprintf(floor_label, sizeof(floor_label), "Floor %d", floor);

        DrawText(floor_label, SHAFT_X - 70, (int)(y + floor_height / 2 - 10), 18, DARKGRAY);

        if (floor == controller->target_floor)
        {
            DrawRectangleLinesEx(
                (Rectangle){SHAFT_X + 5, y + 5, SHAFT_WIDTH - 10, floor_height - 10},
                3,
                ORANGE);
        }

        if (floor == controller->current_floor)
        {
            Rectangle elevator = {
                SHAFT_X + 55,
                y + 8,
                SHAFT_WIDTH - 110,
                floor_height - 16};

            DrawRectangleRec(elevator, SKYBLUE);
            DrawRectangleLinesEx(elevator, 3, BLUE);

            if (controller->door_open)
            {
                DrawLine((int)(elevator.x + elevator.width / 2),
                         (int)elevator.y,
                         (int)(elevator.x + elevator.width / 2),
                         (int)(elevator.y + elevator.height),
                         DARKBLUE);

                DrawText("OPEN",
                         (int)(elevator.x + elevator.width / 2 - 25),
                         (int)(elevator.y + elevator.height / 2 - 10),
                         18,
                         DARKBLUE);
            }
            else
            {
                DrawText("CABIN",
                         (int)(elevator.x + elevator.width / 2 - 30),
                         (int)(elevator.y + elevator.height / 2 - 10),
                         18,
                         DARKBLUE);
            }
        }
    }

    DrawLine(SHAFT_X, SHAFT_Y + SHAFT_HEIGHT, SHAFT_X + SHAFT_WIDTH, SHAFT_Y + SHAFT_HEIGHT, LIGHTGRAY);
}

static void DrawStatusPanel(const ElevatorController *controller,
                            bool emergency_stop,
                            bool door_obstruction,
                            bool upper_limit_switch,
                            bool lower_limit_switch)
{
    DrawText("Elevator GUI Simulator", PANEL_X, 30, 28, BLACK);

    DrawText(TextFormat("Current Floor: %d", controller->current_floor), PANEL_X, PANEL_Y, 22, BLACK);
    DrawText(TextFormat("Target Floor:  %d", controller->target_floor), PANEL_X, PANEL_Y + 35, 22, BLACK);
    DrawText(TextFormat("State: %s", Elevator_StateToString(controller->state)), PANEL_X, PANEL_Y + 70, 22, BLACK);
    DrawText(TextFormat("Motor: %s", Elevator_MotorToString(controller->motor)), PANEL_X, PANEL_Y + 105, 22, BLACK);
    DrawText(TextFormat("Door: %s", controller->door_open ? "OPEN" : "CLOSED"), PANEL_X, PANEL_Y + 140, 22, BLACK);

    DrawText("Safety Inputs", PANEL_X, PANEL_Y + 200, 24, BLACK);

    DrawText(TextFormat("Emergency Stop: %s", emergency_stop ? "ACTIVE" : "OFF"),
             PANEL_X, PANEL_Y + 240, 20, emergency_stop ? RED : DARKGREEN);

    DrawText(TextFormat("Door Obstruction: %s", door_obstruction ? "ACTIVE" : "OFF"),
             PANEL_X, PANEL_Y + 270, 20, door_obstruction ? RED : DARKGREEN);

    DrawText(TextFormat("Upper Limit: %s", upper_limit_switch ? "ACTIVE" : "OFF"),
             PANEL_X, PANEL_Y + 300, 20, upper_limit_switch ? RED : DARKGREEN);

    DrawText(TextFormat("Lower Limit: %s", lower_limit_switch ? "ACTIVE" : "OFF"),
             PANEL_X, PANEL_Y + 330, 20, lower_limit_switch ? RED : DARKGREEN);
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
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        update_timer += GetFrameTime();

        /*
         * Floor request buttons.
         * This GUI version sends one pending floor request to the controller.
         */
        int button_start_y = PANEL_Y + 390;

        for (int floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
        {
            int index = floor - MIN_FLOOR;
            int col = index % 3;
            int row = index / 3;

            Rectangle floor_button = {
                PANEL_X + col * (BUTTON_WIDTH + BUTTON_GAP),
                button_start_y + row * (BUTTON_HEIGHT + BUTTON_GAP),
                BUTTON_WIDTH,
                BUTTON_HEIGHT};

            if (IsButtonClicked(floor_button))
            {
                pending_request_floor = floor;
            }
        }

        Rectangle emergency_button = {PANEL_X + 320, button_start_y, 170, BUTTON_HEIGHT};
        Rectangle obstruction_button = {PANEL_X + 320, button_start_y + 50, 170, BUTTON_HEIGHT};
        Rectangle upper_limit_button = {PANEL_X + 320, button_start_y + 100, 170, BUTTON_HEIGHT};
        Rectangle lower_limit_button = {PANEL_X + 320, button_start_y + 150, 170, BUTTON_HEIGHT};

        if (IsButtonClicked(emergency_button))
            emergency_stop = !emergency_stop;

        if (IsButtonClicked(obstruction_button))
            door_obstruction = !door_obstruction;

        if (IsButtonClicked(upper_limit_button))
            upper_limit_switch = !upper_limit_switch;

        if (IsButtonClicked(lower_limit_button))
            lower_limit_switch = !lower_limit_switch;

        /*
         * Periodic controller update.
         * This simulates a real embedded control loop.
         */
        if (update_timer >= UPDATE_PERIOD_SECONDS)
        {
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
        ClearBackground(RAYWHITE);

        DrawElevatorShaft(&controller);
        DrawStatusPanel(&controller,
                        emergency_stop,
                        door_obstruction,
                        upper_limit_switch,
                        lower_limit_switch);

        DrawText("Floor Request Buttons", PANEL_X, button_start_y - 35, 22, BLACK);

        for (int floor = MIN_FLOOR; floor <= MAX_FLOOR; floor++)
        {
            int index = floor - MIN_FLOOR;
            int col = index % 3;
            int row = index / 3;

            Rectangle floor_button = {
                PANEL_X + col * (BUTTON_WIDTH + BUTTON_GAP),
                button_start_y + row * (BUTTON_HEIGHT + BUTTON_GAP),
                BUTTON_WIDTH,
                BUTTON_HEIGHT};

            DrawButton(floor_button, TextFormat("%d", floor));
        }

        DrawButton(emergency_button, emergency_stop ? "Clear E-Stop" : "Emergency");
        DrawButton(obstruction_button, door_obstruction ? "Clear Door Obs." : "Door Obs.");
        DrawButton(upper_limit_button, upper_limit_switch ? "Clear Upper" : "Upper Limit");
        DrawButton(lower_limit_button, lower_limit_switch ? "Clear Lower" : "Lower Limit");

        if (pending_request_floor != 0)
        {
            DrawText(TextFormat("Pending request: Floor %d", pending_request_floor),
                     PANEL_X,
                     WINDOW_HEIGHT - 40,
                     22,
                     ORANGE);
        }
        else
        {
            DrawText("Click a floor button to send a request.",
                     PANEL_X,
                     WINDOW_HEIGHT - 40,
                     22,
                     DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
