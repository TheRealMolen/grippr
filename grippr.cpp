// based from: https://lazyfoo.net/tutorials/SDL/51_SDL_and_modern_opengl/index.php

#include <string>
#include <iostream>
#include <chrono>

#include <SDL.h>
#include <SDL_opengl.h>
#include <gl\glu.h>


using namespace std;


static const int SCREEN_WIDTH = 800;
static const int SCREEN_HEIGHT = 600;

static const float BASE_WIDTH = 195.f;
static const float BASE_HEIGHT = 108.f;
static const float SHOULDER_HEIGHT = 72.f;
static const float ARM_OVERLAP = 25.f;
static const float ARM_LENGTH = 124.f;
static const float HAND_LENGTH = 192.f;

static const float PI = 3.141592f;
static const float PIBY180 = PI / 180.0f;
static const float TWOPI = PI * 2.0f;
static const float DEGTORAD = PIBY180;
static const float RADTODEG = 180.f / PI;


SDL_Window* gWindow = nullptr;
SDL_GLContext gContext;
GLUquadric* gQuadric = nullptr;

double gWallTime = 0;


class PushMatrixScope
{
public:
    PushMatrixScope()
    {
        glPushMatrix();
    }
    ~PushMatrixScope()
    {
        glPopMatrix();
    }
};


enum Bones
{
    BASE_ROT,
    SHOULDER,
    ELBOW,
    WRIST,

    NumBones,
};
float gRotations[NumBones] = { 0.f, -22.f, -65.f, -80.f };


void renderFloor(float size)
{
    size *= 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0.f, 1.f, 0.f);
    glVertex3f(-size, 0.f, -size);
    glVertex3f(size, 0.f, -size);
    glVertex3f(size, 0.f, size);
    glVertex3f(-size, 0.f, size);
    glEnd();
}

void renderBox(float width, float height, float depth)
{
    width *= 0.5f;
    depth *= 0.5f;

    glBegin(GL_QUADS);
    // top
    glNormal3f(0.f, 1.f, 0.f);
    glVertex3f(-width, height, -depth);
    glVertex3f(width, height, -depth);
    glVertex3f(width, height, depth);
    glVertex3f(-width, height, depth);
    // +z
    glNormal3f(0.f, 0.f, 1.f);
    glVertex3f(-width, height, depth);
    glVertex3f(width, height, depth);
    glVertex3f(width, 0.f, depth);
    glVertex3f(-width, 0.f, depth);
    // +x
    glNormal3f(1.f, 0.f, 0.f);
    glVertex3f(width, height, depth);
    glVertex3f(width, 0.f, depth);
    glVertex3f(width, 0.f, -depth);
    glVertex3f(width, height, -depth);
    // -z
    glNormal3f(0.f, 0.f, -1.f);
    glVertex3f(-width, height, -depth);
    glVertex3f(-width, 0.f, -depth);
    glVertex3f(width, 0.f, -depth);
    glVertex3f(width, height, -depth);
    // -x
    glNormal3f(-1.f, 0.f, 0.f);
    glVertex3f(-width, height, depth);
    glVertex3f(-width, height, -depth);
    glVertex3f(-width, 0.f, -depth);
    glVertex3f(-width, 0.f, depth);
    // btm
    glNormal3f(0.f, -1.f, 0.f);
    glVertex3f(-width, 0.f, -depth);
    glVertex3f(-width, 0.f, depth);
    glVertex3f(width, 0.f, depth);
    glVertex3f(width, 0.f, -depth);
    glEnd();
}

void renderBase(float radius)
{
    gluSphere(gQuadric, radius, 16, 16);
}

void renderArm()
{
    PushMatrixScope overlap;
    glTranslatef(0.f, -ARM_OVERLAP, 0.f);

    renderBox(55.f, ARM_LENGTH + 2 * ARM_OVERLAP, 40.f);
}

void renderHand()
{
    PushMatrixScope overlap;
    glTranslatef(0.f, -ARM_OVERLAP, 0.f);

    renderBox(57.f, 60.f + ARM_OVERLAP, 40.f);

    glColor3f(1.f, 0.9f, 0.7f);
    glTranslatef(0.f, 60.f + ARM_OVERLAP, 0.f);
    renderBox(90.f, 70.f, 18.f);

    {
        PushMatrixScope finger;
        glTranslatef(-20.f, 0.f, 0.f);
        renderBox(25.f, HAND_LENGTH - 60.f, 12.f);
    }
    {
        PushMatrixScope finger;
        glTranslatef(20.f, 0.f, 0.f);
        renderBox(25.f, HAND_LENGTH - 60.f, 12.f);
    }
}


void renderAxis()
{
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(1000.f, 0.f, 0.f);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 1000.f, 0.f);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.f, 0.f, 0.f);
    glVertex3f(0.f, 0.f, 1000.f);
    glEnd();
    glEnable(GL_LIGHTING);
}


void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    gluLookAt(600.f, 500.f, 500.f,
        200.f, 350.f, 200.f,
        0.f, 1.f, 0.f);

    //renderAxis();

    glColor3f(0.6f, 0.6f, 0.6f);
    renderFloor(500.f);

    glColor3f(0.9f, 0.9f, 0.9f);
    renderBox(BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH);
    {
        PushMatrixScope baseScope;
        glTranslatef(0.f, BASE_HEIGHT, 0.f);
        glRotatef(gRotations[BASE_ROT], -1.f, 0.f, 0.f);

        renderBase(SHOULDER_HEIGHT);
        {
            PushMatrixScope shoulderScope;
            glTranslatef(0.f, SHOULDER_HEIGHT, 0.f);
            glRotatef(gRotations[SHOULDER], -1.f, 0.f, 0.f);

            renderArm();
            {
                PushMatrixScope uArmScope;
                glTranslatef(0.f, ARM_LENGTH, 0.f);
                glRotatef(gRotations[ELBOW], -1.f, 0.f, 0.f);

                renderArm();
                {
                    PushMatrixScope lArmScope;
                    glTranslatef(0.f, ARM_LENGTH, 0.f);
                    glRotatef(gRotations[WRIST], -1.f, 0.f, 0.f);

                    renderHand();
                }
            }
        }
    }

    glPopMatrix();
}


void update(float deltaTime)
{
    gRotations[SHOULDER] = -15.0f + 20.f * (float)sin(gWallTime);
    gRotations[ELBOW] = -55.0f + 15.f * (float)cos(gWallTime);
}





bool initGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_FLAT);

    glClearColor(0.9f, 0.45f, 0.2f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 2.f, 2000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float lightDir[] = { -0.7f, 0.3f, 0.5f, 0.f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
    glEnable(GL_LIGHT0);

    gQuadric = gluNewQuadric();
    if (!gQuadric)
    {
        cerr << "couldn't allocate quadric" << endl;
        return false;
    }

    float ambientLevel[] = { 0.3f, 0.3f, 0.3f, 1.f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLevel);
  
    return true;
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
        return false;
    }

    //Create window
    gWindow = SDL_CreateWindow("grippr", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!gWindow)
    {
        cerr << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
        return false;
    }

    //Use OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    //Create context
    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == NULL)
    {
        cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << endl;
        return false;
    }

    return initGL();
}


void shutdown()
{
    //Destroy window	
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;

    //Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    cout << "warming up sdl & opengl..." << endl;
    if (!init())
    {
        cerr << "Failed to initialize! /tableflip" << endl;
        return 1;
    }

    bool quit = false;
    auto startTime = chrono::high_resolution_clock::now();
    auto lastFrameTime = startTime;
    float deltaTime = 0.f;
    while (!quit)
    {
        // process input
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
                quit = true;
        }

        // update time
        {
            auto now = chrono::high_resolution_clock::now();
            int64_t udelta = chrono::duration_cast<chrono::microseconds>(now - lastFrameTime).count();
            lastFrameTime = now;
            deltaTime = ((float)udelta) / 1'000'000.f;

            int64_t uwallTime = chrono::duration_cast<chrono::microseconds>(now - startTime).count();
            gWallTime = ((double)uwallTime) / 1'000'000.0;
        }

        update(deltaTime);
        render();

        SDL_GL_SwapWindow(gWindow);
    }

    shutdown();

    return 0;
}



