#include <cstdio>
#include <stdio.h>
#include <GL/glut.h>
#include <stdlib.h> 
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// SETTINGS (WINDOWS)
int windowWidth = 550, windowHeight = 700;

// USER MOVEMENT - CHANGES OF VALUE (X, Y)
float userX = 7.5f, userY = 0.0f;

// ENEMY SPREAD
const int enemyCount = 3;
float enemyX[enemyCount], enemyY[enemyCount];

// KEYBOARD (Start Jumping)
bool moveLeft = false, moveRight = false;

// USER JUMP 
bool isOnGround = true;
float gravity = 1.0f, velocityY = 0.0f, jumpStrength = 1.20f;

// PLATFORMS
const int platformCount = 3;
float platformX[platformCount] = { 6.0f, 11.0f, 16.0f };
float platformY[platformCount] = { 6.0f, 9.0f, 13.0f };
float platformHalfWidth[platformCount] = { 2.0f, 2.0f, 2.0f };

// AUTO MOVE PLATFORM
float platformSpeed[platformCount] = { 0.05f, 0.07f, 0.06f };
int platformDirection[platformCount] = { 1, -1, 1 };

// CLOSE PLATFORM

// LIVES, SCORING, STAGE & GAME OVER PROGRAM
int lives = 3; // STARTING LIVES
bool isGameOver = false;
int lastHitTime = 0;
const int hitDelay = 1000; // delay in ms (1 second)
int score = 0;
bool landedOnPlatform[platformCount] = { false, false, false };
bool switchOn = false;
int stage = 1;

// START, PAUSE, PLAY, & END (PLATFORM PROGRAM)
bool gameIntro = true; // GAME INTRODUCTION
bool isStart = false; // START GAME
bool isRestart = false; // RESTART GAME
bool isPaused = false; // PAUSE PROGRAM



GLuint loadTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to load texture: %s\n", filename);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Setup filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Pumili kung RGB or RGBA
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return textureID;
}




void renderBitmapString(float x, float y, const char* string)
{
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

// ============================ OBJECT FUNCTION ============================

// USER | USER SPRITES
GLuint userWalkLeft[3], userWalkRight[3];
int currentFrame = 0;
int frameDelay = 0;
int frameCounter = 0;

// USER | USER FUNCTION
void user()
{
    glEnable(GL_TEXTURE_2D);

    // GAME OVER PLATFORM PROGRAM
    if (!isGameOver) {

        // IMAGE FRAME PER FRAME
        if (moveRight) {   // ANIMATE RIGHT
            frameCounter++;
            if (frameCounter >= frameDelay) {
                currentFrame = (currentFrame + 1) % 3;
                frameCounter = 0;
            }
            glBindTexture(GL_TEXTURE_2D, userWalkRight[currentFrame]);
        }
        else if (moveLeft) {   // ANIMATE LEFT
            frameCounter++;
            if (frameCounter >= frameDelay) {
                currentFrame = (currentFrame + 1) % 3;
                frameCounter = 0;
            }
            glBindTexture(GL_TEXTURE_2D, userWalkLeft[currentFrame]);
        }
        else {
            // idle frame (default facing right)
            glBindTexture(GL_TEXTURE_2D, userWalkRight[0]);
        }
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(userX + 2.0f, userY + 2.0f); // BOTTOM LEFT
    glTexCoord2f(1.0f, 0.0f); glVertex2f(userX + 3.0f, userY + 2.0f); // BOTTOM RIGHT
    glTexCoord2f(1.0f, 1.0f); glVertex2f(userX + 3.0f, userY + 3.0f); // TOP RIGHT
    glTexCoord2f(0.0f, 1.0f); glVertex2f(userX + 2.0f, userY + 3.0f); // TOP LEFT
    glEnd();

    glDisable(GL_TEXTURE_2D); // RENDER OTHER OBJECTS NORMALLY
}

void ground()
{
    glBegin(GL_QUADS);
    glColor3f(0.396f, 0.263f, 0.129f); // DARK BROWN
    glVertex2f(0.0f, 0.0f); // BOTTOM LEFT
    glVertex2f(20.0f, 0.0f); // BOTTOM RIGHT
    glVertex2f(20.0f, 2.0f); // TOP RIGHT
    glVertex2f(0.0f, 2.0f); // TOP LEFT
    glEnd();
}

void middleGround()
{
    for (int i = 0; i < platformCount; i++) {
        glBegin(GL_QUADS);
        glColor3f(0.3f, 0.3f, 0.3f);
        glVertex2f(platformX[i] - platformHalfWidth[i], platformY[i]);
        glVertex2f(platformX[i] + platformHalfWidth[i], platformY[i]);
        glVertex2f(platformX[i] + platformHalfWidth[i], platformY[i] + 0.5f);
        glVertex2f(platformX[i] - platformHalfWidth[i], platformY[i] + 0.5f);
        glEnd();

        if (i == 2)
        {
            glBegin(GL_QUADS);
            glVertex2f(platformX[i] - 0.3f, platformY[i] + 0.5f); // BOTTOM LEFT
            glVertex2f(platformX[i] + 0.6f, platformY[i] + 0.5f); // BOTTOM RIGHT
            glVertex2f(platformX[i] + 0.6f, platformY[i] + 1.0f); // TOP RIGHT
            glVertex2f(platformX[i] - 0.3f, platformY[i] + 1.0f); // TOP LEFT
            glEnd();
        }
    }
}

// CLOSE PLATFORM
void closePlatform()
{
    glBegin(GL_QUADS);

    // Top (lighter blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(0.0f, 19.0f); // BOTTOM LEFT
    glVertex2f(20.0f, 19.0f); // BOTTOM RIGHT

    // Bottom (darker blue)
    glColor3f(0.0f, 0.0f, 0.5f);
    glVertex2f(20.0f, 20.0f); // TOP RIGHT
    glVertex2f(0.0f, 20.0f); // TOP LEFT
    glEnd();
}

void enemy()
{
    for (int i = 0; i < enemyCount; i++)
    {
        glBegin(GL_QUADS);
        glColor3f(0.396f, 0.263f, 0.129f); // DARK BROWN
        glVertex2f(enemyX[i], enemyY[i]);
        glVertex2f(enemyX[i] + 0.5, enemyY[i]);
        glVertex2f(enemyX[i] + 0.5, enemyY[i] + 0.5);
        glVertex2f(enemyX[i], enemyY[i] + 0.5);
        glEnd();
    }
}

void blackOverlay()
{
    // BLACK
    glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // OVERLAY
        glVertex2f(0.0f, 0.0f); // BOTTOM LEFT
        glVertex2f(20.0f, 0.0f); // BOTTOM RIGHT
        glVertex2f(20.0f, 20.0f); // TOP RIGHT
        glVertex2f(0.0f, 20.0f); // TOP LEFT
    glEnd();
}

void drawButton(float left, float bottom, float width, float height, const char* label)
{
    float right = left + width;
    float top = bottom + height;

    glBegin(GL_QUADS);
    glColor3f(0.8f, 0.1f, 0.1f);
    glVertex2f(left, bottom);
    glVertex2f(right, bottom);
    glVertex2f(right, top);
    glVertex2f(left, top);
    glEnd();

    // simple label placement (adjust offsets if needed)
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapString(left + width * 0.18f, bottom + height * 0.35f, label);
}

// --------------------------------------  PROGRAM: INTRODUCTION  --------------------------------------
void introductionOverlay()
{
    blackOverlay(); // OVERLAY

}

// --------------------------------------  SETTINGS: PAUSE & PLAY  -------------------------------------- 
void overlayPausePlay()
{
    blackOverlay(); // OVERLAY

    // BUTTON LAYOUT (centralized)
    float btnWidth = 3.5f;
    float btnHeight = 1.5f;
    float spacing = 1.0f; // space between buttons
    float totalWidth = 3 * btnWidth + 2 * spacing;
    float startX = (20.0f - totalWidth) / 2.0f; // center horizontally
    float btnY = 5.0f; // vertical position (bottom)

    drawButton(startX, btnY, btnWidth, btnHeight, "HOME");
    drawButton(startX + (btnWidth + spacing), btnY, btnWidth, btnHeight, "RESTART");
    drawButton(startX + 2 * (btnWidth + spacing), btnY, btnWidth, btnHeight, "RESUME");
}

// --------------------------------------  PROGRAM: GAME OVER  --------------------------------------
void gameOverOverlay()
{
    // BLACK OVERLAY
    glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.0f, 0.5f); // OVERLAY
        glVertex2f(0.0f, 0.0f); // BOTTOM LEFT
        glVertex2f(20.0f, 0.0f); // BOTTOM RIGHT
        glVertex2f(20.0f, 20.0f); // TOP RIGHT
        glVertex2f(0.0f, 20.0f); // TOP LEFT
    glEnd();

    // START BUTTON
    glBegin(GL_QUADS);
        glColor3f(0.8f, 0.1f, 0.1f);
        glVertex2f(8.0f, 8.0f); // BOTTOM LEFT
        glVertex2f(12.5f, 8.0f); // BOTTOM RIGHT
        glVertex2f(12.5f, 12.0f); // TOP RIGHT
        glVertex2f(8.0f, 12.0f); // TOP LEFT
    glEnd();

    // LABEL 
    glColor3f(1.0f, 1.0f, 1.0f);
    renderBitmapString(8.8f, 9.8f, "RESTART");
}

// ============================ OBJECT PROGRAM (FINAL) ============================
void objectProgram()
{
    if (!isStart) {
        overlayPausePlay();
    }

    // REVEAL USER, ENEMY, & GROUND (WHEN PROGRAM START)
    if (isStart) {
        // USER
        user();

        // ENEMY 
        enemy();

        // GROUND
        ground();

        // MIDDLE GROUND
        middleGround();

        // CLOSE PLATFORM
        closePlatform();
    }



    if (isPaused) {
        overlayPausePlay();
    }

    if (isGameOver) gameOverOverlay();
}

// --------------------- MAIN FUNCTION | ENEMY LOGIC ---------------------
void enemyLogic()
{
    for (int i = 0; i < enemyCount; i++) {
        enemyX[i] = rand() % 20;       // random x position
        enemyY[i] = (rand() % 20) + 5; // random y para di sabay-sabay
    }
}

// ============================ RESET GAME ============================
void resetGame() 
{
    // RESET STATES
    isGameOver = false;
    isStart = true;
    isPaused = false;

    // RESET POSITIONS
    userX = 7.5f;
    userY = 0.0f;
    velocityY = 0.0f;
    isOnGround = true;

    // RESET STATS
    lives = 3;
    score = 0;

    // RESET PLATFORMS (BACK TO THE ORIGINAL POSITIONS)
    float defaultPlatformX[platformCount] = { 6.0f, 11.0f, 16.0f };
    float defaultPlatformY[platformCount] = { 6.0f, 9.0f, 13.0f };

    for (int i = 0; i < platformCount; i++)
    {
        platformX[i] = defaultPlatformX[i];
        platformY[i] = defaultPlatformY[i];
        landedOnPlatform[i] = false;
    }

    // RESET ENEMY
    enemyLogic();
}
// ============================ MOUSE FUNCTION ============================
void mouse(int button, int state, int x, int y)
{
    if (isGameOver && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Convert mouse (screen coords) -> game coords (ortho 0–20)
        int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
        float mouseX = (float)x / windowWidth * 20.0f;
        float mouseY = (float)(windowHeight - y) / windowHeight * 20.0f;

        // Button bounds (same as sa overlay)
        float btnLeft = 8.0f, btnRight = 12.0f, btnBottom = 9.0f, btnTop = 11.0f;

        if (mouseX >= btnLeft && mouseX <= btnRight &&
            mouseY >= btnBottom && mouseY <= btnTop)
        {
            resetGame(); // restart game agad
        }
    }
}


// ============================ KEYBOARD FUNCTION ============================
void keyboard(unsigned char key, int x, int y)
{
    if (isStart) {

        switch (key)
        {
            // MOVE LEFT & RIGHT
        case 'a':
        case 'A':
            moveLeft = true;
            break;
        case 'd':
        case 'D':
            moveRight = true;
            break;
        case 32: // SPACE (jump)
            if (isOnGround || velocityY == 0.0f) {
                isOnGround = false;
                velocityY = jumpStrength;
            }
            break;
        case 27: // ESC (Pause & Play)
            isPaused = !isPaused ? true : false;
            break;
        }
    }

    // --------------- RESTART IF GAMEOVER ---------------
    if (isGameOver == true) {
        switch (key)
        {
        case 'S':
        case 's': 
            resetGame();
            break;
        default:
            break;
        }
    }

    switch (key)
    {
    case 9: // TAB key
        //isStart = !isStart ? true : false;
        isStart = true;
        break;
    }

    glutPostRedisplay();
}

void keyboardUp(unsigned char key, int x, int y)
{
    if (isStart) {
        switch (key)
        {
            // MOVE LEFT & RIGHT (STOP)
        case 'a':
        case 'A':
            moveLeft = false;
            break;
        case 'd':
        case 'D':
            moveRight = false;
            break;
        }
    }

    glutPostRedisplay();
}

// KEYBOARD FUNCTION | ARROW KEYS FUNCTION 
void specialKeys(int key, int x, int y)
{

    if (isStart) {
        switch (key)
        {
            // MOVE LEFT & RIGHT 
        case GLUT_KEY_LEFT:
            moveLeft = true;
            break;
        case GLUT_KEY_RIGHT:
            moveRight = true;
            break;
        }
    }

    glutPostRedisplay();
}

void specialKeysUp(int key, int x, int y)
{
    if (isStart) {
        switch (key)
        {
            // MOVE LEFT & RIGHT (STOP)
        case GLUT_KEY_LEFT:
            moveLeft = false;
            break;
        case GLUT_KEY_RIGHT:
            moveRight = false;
            break;
        }
    }
}




// ============================ UPDATE FUNCTION ============================

// UPDATE FUNCTION | APPLY GRAVITY
void applyGravity()
{
    // USER NOT ON GROUND
    if (!isOnGround)
    {
        velocityY -= gravity * 0.1f; printf("%f\n", velocityY);
        userY += velocityY; printf("%f\n", userY);
    }

    // USER ON GROUND
    if (userY <= 0.0f)
    {
        userY = 0.0f;
        velocityY = 0.0f;
        isOnGround = true;
    }
}

// MOVING PLATFORM
void movePlatforms() {
    for (int i = 0; i < platformCount; i++) {
        platformX[i] += platformSpeed[i] * platformDirection[i];

        // Kapag sumobra sa limit, magpalit ng direction
        if (platformX[i] > 18.0f || platformX[i] < 2.0f) {
            platformDirection[i] *= -1;
        }
    }
}

// PLATFORM COLLISION
void platformCollision()
{
    for (int i = 0; i < platformCount; i++)
    {
        // PLATFORM BOUNDS
        float platLeft = platformX[i] - platformHalfWidth[i];
        float platRight = platformX[i] + platformHalfWidth[i];
        float platTop = platformY[i] + 1.5f;
        float platBottom = platformY[i];

        // OBJECT BOUNDS
        float userLeft = userX + 2.0f;
        float userRight = userX + 3.0f;
        float userBottom = userY + 2.0f;
        float userTop = userY + 3.0f;

        if (
            userLeft < platRight &&
            userRight > platLeft &&
            userTop > platBottom &&
            userBottom < platTop
            )
        {
            // LANDING ON TOP
            if (userBottom <= platTop && velocityY < 0) {
                userY = platTop - 3.0f;
                velocityY = 0.0f;

                // MOVING WITH PLATFORM
                userX += platformSpeed[i] * platformDirection[i];

                // Only award score once per landing
                if (!landedOnPlatform[i]) {
                    score += 10;
                    landedOnPlatform[i] = true;
                }
            }

            // HITTING FROM BELOW
            else if (userTop >= platBottom && userBottom < platBottom && velocityY > 0)
            {
                userY = platBottom - 3.0f; printf("Legit %f\n", userY);
                velocityY = 0.0f;
            }

            // COLLISION LEFT
            else if (userRight > platLeft && userLeft < platLeft)
            {
                userX = platLeft - 3.0f; printf("Left Collision\n");
            }

            // COLLISION RIGHT
            else if (userLeft < platRight && userRight > platRight)
            {
                userX = platRight - 3.0f; printf("Right Collision\n");
            }



        }


    }


}

// UPDATE FUNCTION | ENEMY FUNCTION
void enemyFunction()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    for (int i = 0; i < enemyCount; i++)
    {
        enemyY[i] -= 0.1f;  // bagsak

        // CHECKING COLLISION WITH PLATFORMS
        for (int j = 0; j < platformCount; j++)
        {
            float platLeft = platformX[j] - platformHalfWidth[j];
            float platRight = platformX[j] + platformHalfWidth[j];
            float platTop = platformY[j] + 0.5f;

            // ENEMY OBJECT BOUNDS
            float enemyLeft = enemyX[i];
            float enemyRight = enemyX[i] + 0.5f;
            float enemyBottom = enemyY[i];
            float enemyTop = enemyY[i] + 0.5f;

            // USER OBJECT BOUNDS
            float userLeft = userX + 2.0f;
            float userRight = userX + 3.0f;
            float userBottom = userY + 2.0f;
            float userTop = userY + 3.0f;

            // Kapag tumuntong sa ibabaw ng platform
            if (enemyLeft < platRight && enemyRight > platLeft &&
                enemyBottom <= platTop && enemyTop > platformY[j])
            {
                enemyY[i] = 20.0f;
                enemyX[i] = rand() % 20;
            }

            // ENEMY COLLIDE TO THE USER
            if (
                userLeft < enemyRight &&
                userRight > enemyLeft &&
                userBottom < enemyTop &&
                userTop > enemyBottom
                ) {
                if (currentTime - lastHitTime > hitDelay)
                {
                    printf("Last Hit Time: %f\n", lastHitTime);
                    printf("Hit Delay: %f\n", hitDelay);
                    printf("Current Time: %f\n", currentTime);

                    lives--;
                    lastHitTime = currentTime; // RESET TIMER

                    // GAME OVER PROGRAM
                    if (lives <= 0) {
                        isGameOver = true;
                    }
                }
            }
        }

        // IF GROUND ON DOWN
        if (enemyY[i] < 2.0f)
        {
            enemyY[i] = 20.0f;
            enemyX[i] = rand() % 20;
        }
    }
}

void switchCollision()
{
    float switchLeft = platformX[2] - 0.3f;
    float switchRight = platformX[2] + 0.6f;
    float switchBottom = platformY[2] + 0.5f;
    float switchTop = platformY[2] + 1.0f;

    float userLeft = userX + 2.0f;
    float userRight = userX + 3.0f;
    float userBottom = userX + 2.0f;
    float userTop = userX + 3.0f;

    // Collision check
    if (userLeft < switchRight && userRight > switchLeft &&
        userBottom < switchTop && userTop > switchBottom) {
        switchOn = true;   // Switch activated
    }
}

void proceedToNextStage()
{
    stage++;

    for (int i = 0; i < platformCount; i++)
    {
        landedOnPlatform[i] = false;
    }
}

// --------------------------------------  UPDATE LOGIC: GAME OVER  --------------------------------------
void isGameOverLogicFunction()
{
    printf("Game Over: %s\n", isGameOver ? "true" : "false");
}



// UPDATE FUNCTION MAIN
void update(int value)
{
    // START PLATFORM GAME
    if (!isStart) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    // PAUSE
    if (isPaused) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    // GAME OVER
    if (isGameOver) {
        isGameOverLogicFunction();

        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }



    // MOVE LEFT & RIGHT
    if (moveLeft) userX -= 0.1f;
    if (moveRight) userX += 0.1f;

    // TESTING
    printf("User Y: ------- %f\n", userY);
    /*if (userY > 18.0f) {
        userX = 7.5f;
        userY = 0.0f;

        proceedToNextStage();
    }*/

    if (stage == 3) {
        glutPostRedisplay();
        glutTimerFunc(16, update, 0);
        return;
    }

    // APPLY GRAVITY
    applyGravity();

    // ENEMY FUNCTION
    enemyFunction();

    movePlatforms();

    // PLATFORM COLLISION
    platformCollision();

    // SWITCH COLLISION
    switchCollision();


    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ============================ DISPLAY FUNCTION ============================ 
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // CAMERA (follow only Y, but clamped so di mawawala si user)
    //float cameraY = userY - 5.0f;

    //// wag bumaba sa 0
    //if (cameraY < 0) cameraY = 0;

    //glOrtho(0, 20, 0 + cameraY, 20 + cameraY, -1, 1);

    glOrtho(0, 20, 0, 20, -1, 1);

    objectProgram();

    // WHEN THE GAME IS NOT YET STARTED - DISPLAY INTRO (START)
    // isStart = false
    if (isStart) {
        // DISPLAY LIVES
        char livesText[20];
        glColor3f(1.0f, 0.0f, 0.0f); // RED
        snprintf(livesText, sizeof(livesText), "Lives: %d", lives);
        renderBitmapString(2.0f, 1.0f, livesText);

        // DISPLAY SCORE
        char scoreText[20];
        glColor3f(0.0f, 0.0f, 0.0f); // BLACK
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
        renderBitmapString(2.0f, 19.0f, scoreText);
    }

    // WHEN START THE GAME - REMOVE DISPLAY INTRO (START)
    if (!isStart) {

        // DISPLAY INTRO (START)
        glColor3f(1.0f, 0.0f, 0.0f); // RED
        renderBitmapString(7.5f, 5.0f, "Click [Tab] to Start");
    }

    glFlush();
}


// ============================ MAIN FUNCTION ============================ 



int main(int arg, char** argv)
{
    glutInit(&arg, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

    // WINDOW CONFIGURATION (APPLICATION SETTINGS)
    int screenWidth = glutGet(GLUT_SCREEN_WIDTH);
    int screenHeight = glutGet(GLUT_SCREEN_HEIGHT);

    int posX = (screenWidth - windowWidth) / 2;
    int posY = (screenHeight - windowHeight) / 2;

    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(posX, posY);

    // APPLICATION TITLE
    glutCreateWindow("Application namin ni papi mosh");
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutKeyboardFunc(keyboard); // Keyboard Keys Manipulation
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glClearColor(0.678f, 0.847, 0.902f, 1.0f); // background

    // ENEMY LOGIC
    enemyLogic();

    // PICTURE LOADED TEXTURE
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /*userWalkRight[0] = loadTexture("C:\\Users\\Andrew\\source\\repos\\Midterm_Activity_Quiz_Exam\\Midterm_Activity_Quiz_Exam\\Debug\\Images\\User\\walkRightOne.png");
    userWalkRight[1] = loadTexture("C:\\Users\\Andrew\\source\\repos\\Midterm_Activity_Quiz_Exam\\Midterm_Activity_Quiz_Exam\\Debug\\Images\\User\\walkRightTwo.png");
    userWalkRight[2] = loadTexture("C:\\Users\\Andrew\\source\\repos\\Midterm_Activity_Quiz_Exam\\Midterm_Activity_Quiz_Exam\\Debug\\Images\\User\\rightStay.png");*/

    userWalkRight[0] = loadTexture("Images/User/walkRightOne.png");
    userWalkRight[1] = loadTexture("Images/User/walkRightTwo.png");
    userWalkRight[2] = loadTexture("Images/User/rightStay.png");


    glutMainLoop();

    return 0;
}


