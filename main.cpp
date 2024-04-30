#include <iostream>
#include <cmath>
#include <raylib.h>

// Define the color
const Color lightBlue = Color{170, 204, 249, 255};

class Brick {
public:
    bool active; // Check if the ball has been hit, if true means visiable, collidable
    Rectangle rect;

    // Default constructor, avoid errors from Game() initialization
    Brick() : active(true), rect({0, 0, 0, 0}) {}

    Brick(float x, float y, float width, float height) : active(true), rect({x, y, width, height}) {}
};

class Ball {
public:
    Vector2 position;
    Vector2 speed;
    float radius;
    bool isVisiable;

    // Use to reset the ball
    Vector2 initialPosition;
    Vector2 initialSpeed;

    Ball(float x, float y, float vx, float vy, float r, bool b) 
    : position({x, y}), speed({vx, vy}), radius(r), isVisiable(b), initialPosition({x, y}), initialSpeed({vx, vy}) {}

    void move() {
        position.x += speed.x;
        position.y += speed.y;
    }

    void draw() {
        DrawCircle(static_cast<int>(position.x), static_cast<int>(position.y), radius, WHITE);
    }

    void reset() {
        // Change the position of the ball to default
        // This ensures that move()'s 'position' will chenge to default
        // e.g., resetting from 750 back to 400 (400 is the original position)
        position = initialPosition;
        speed = initialSpeed;
    }
};

class Paddle {
public:
    Rectangle rect;
    int speed;
    bool useMouseControl;  // Check whether the user wants to use mouse or keyboard
    

    Paddle(float x, float y, float width, float height, int paddleSpeed) 
    : rect({x, y, width, height}), speed(paddleSpeed), useMouseControl(true) {}

    void draw() {
        DrawRectangleRounded(rect, 0.8, 0, WHITE);
    }

    void move() {
        if (useMouseControl) {
            // Get the current X position of the mouse
            int mouseX = GetMouseX();
            // Set the center of the paddle to the mouse's position
            rect.x = mouseX - rect.width / 2;
        } else {  // When the key pressed, move the paddle
            if (IsKeyDown(KEY_LEFT)) {
                rect.x -= speed;
            }
            if (IsKeyDown(KEY_RIGHT)) {
                rect.x += speed;
            }
        }
        
        // Ensure the paddle doesn't move off the left side of the screen
        if (rect.x < 0) {
            rect.x = 0;
        } else if (rect.x + rect.width > GetScreenWidth()) {  // Ensure the paddle doesn't move off the right side of the screen; x is the starting position, adding width represents the full size of the paddle
            rect.x = GetScreenWidth() - rect.width;
        }
    }
};

class Game {
public:
    const int screenWidth = 800;
    const int screenHeight = 700;
    int scoreCounter = 0;
    int threshold = 5; // The score threshold to get the second ball
    
    static const int row = 9;
    static const int column = 7;

    const float margin = 15.0; 
    const float brickWidth = (screenWidth - (column + 1) * margin) / column;
    const float brickHeight = 20.0;

    bool gameOver = false;
    
    Ball ball, ball2;
    Brick bricks[row][column];
    Paddle paddle;

    Game() : ball(screenWidth / 2.0, screenHeight / 1.5, 5.0, -5.0, 15.0, true),  // x, y, speedX, speedY, radius
             ball2(screenWidth / 2.0, screenHeight / 1.5, 5.0, -5.0, 15.0, false),
             paddle(screenWidth / 2.0 - 80.0, screenHeight - 30.0, 160.0, 20.0, 18) {   // x, y, width, height, speed 
        initial();
    }

    void initial() {
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < column; j++) {
                bricks[i][j]= Brick(margin + j * (brickWidth + margin), 
                                     margin + i * (brickHeight + margin), 
                                     brickWidth, 
                                     brickHeight);  // x, y, width, height
            }
        }
    }

    void run() {
        InitWindow(screenWidth, screenHeight, "Brick Game");

        Image icon = LoadImage("./images/bricks.png");
        SetWindowIcon(icon);
        UnloadImage(icon);

        SetTargetFPS(60);

        while (!WindowShouldClose() && !IsKeyPressed(KEY_ENTER)) {
            if (!gameOver) { // If the game is running (not over), update the game
                update();
            }
            draw();

            if (gameOver) {
                DrawText("Game Over! Press Enter to exit the game", 80, screenHeight / 2 - 50, 30, BLACK);
                if (IsKeyPressed(KEY_ENTER)) {
                    break;  // Exit the game loop
                } else if (IsKeyPressed(KEY_R)) {
                    gameOver = false;
                    scoreCounter = 0;
                    ball.reset();
                    ball2.reset();
                    ball2.isVisiable = false;
                    initial();
                    update();
                }
            }
        }

        CloseWindow();
    }

    void checkCollision(Ball &ball) {
        // Collision with the walls
        if (ball.position.x + ball.radius >= screenWidth || ball.position.x - ball.radius <= 0) {
            ball.speed.x *= -1;
        }
        // Collision with the ceiling
        if (ball.position.y - ball.radius <= 0) {
            ball.speed.y *= -1;
        }
        // Collision with the bottom
        if (ball.position.y + ball.radius >= screenHeight) {
            ball.reset();
            // gameOver = true;
        }

        // Collision with bricks
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < column; j++) {
                // 'CheckCollisionCircleRec' checks for collision between a circle (ball) and a rectangle (brick)
                // 'ball.position' is the center of the ball, 'ball.radius' is the size of the ball,
                //  and 'bricks[i][j].rect' is the rectangle representing the current brick's position and size
                //  If both conditions are true -> the brick is active and there's a collision
                if (bricks[i][j].active && CheckCollisionCircleRec(ball.position, ball.radius, bricks[i][j].rect)) {  
                    ball.speed.y *= -1;
                    bricks[i][j].active = false;
                    scoreCounter++;
                }
            }
        }

        // Collision with the paddle
        if (CheckCollisionCircleRec(ball.position, ball.radius, paddle.rect)) {
            // ball.speed.y *= -1;

            // Calculate the difference between the ball's center and the paddle's center
            float difference = ball.position.x - (paddle.rect.x + paddle.rect.width / 2);

            // Normalize the difference to a percentage
            float percent = difference / (paddle.rect.width / 2);

            // Scale the percentage to radians for the angle (tweak 0.25 as needed)
            float radians = percent * 0.25;

            // Limit the maximum radians to prevent extreme angles
            float maxRadians = 0.4;  // Adjust this value as needed
            radians = std::max(std::min(radians, maxRadians), -maxRadians);

            // Calculate the new speed based on the angle and the initial speed
            float speedFactor = 10.0;  // Adjust this value as needed to control the overall speed
            ball.speed.x = ball.initialSpeed.x * radians * speedFactor;
            ball.speed.y = -abs(ball.speed.y); // Ensure the ball moves upward
        }

        // Update gameOver status
        bool allBricksGone = true;
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < column; j++) {
                if (bricks[i][j].active) {
                    allBricksGone = false;
                    break;
                }
            }
            if (!allBricksGone) break;
        }
        if (allBricksGone) {
            gameOver = true;
        }   
    }

    void update() {
        // Check if the player wants to switch control
        if (IsKeyPressed(KEY_M)) {
           paddle.useMouseControl = !paddle.useMouseControl;  // Switch control
        }
        
        ball.move();
        checkCollision(ball);

        if (scoreCounter >= threshold) {
            ball2.isVisiable = true;
            ball2.move();
            checkCollision(ball2);
        }


        paddle.move();
    }

    void draw() {
        BeginDrawing();
        ClearBackground(lightBlue);

        if (!gameOver) {
            for (int i = 0; i < row; i++) {
                for (int j = 0; j < column; j++) {
                    if (bricks[i][j].active) {   // If true (the brick has not been collided yet), draw the rectangle
                        DrawRectangleRec(bricks[i][j].rect, WHITE);  // (x, y, width, height, color)
                    }
                }
            }

            ball.draw();
            if (scoreCounter >= threshold && ball2.isVisiable == true) {
                ball2.draw();
            }
            paddle.draw();
        }            
        
        EndDrawing();
    }
};

int main() {
    Game game;
    game.run();

    return 0;
}
