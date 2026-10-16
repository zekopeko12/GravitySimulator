#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "GL/freeglut_ext.h"
#include "GL/freeglut_std.h"
#include "ogldev_math_3d.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

#include "Pipeline.h"
#include "Camera.h"
#include "Ball.h"
#include "CBFunctions.h"
#include "UI.h"
#include "Shader.h"
#include "Trails.h"
#include "Gravity.h"
#include "Rendering.h"

GLuint VAO, VBO, IBO;
GLuint gShaderProgram = 0;
GLint gColorLocation;
GLint gWVPLocation;

static vector<vector<float> > gTrails;
static GLuint gTrailVAO = 0;
static GLuint gTrailVBO = 0;

Camera* pGameCamera = NULL;
float gCurrentFOV = 30.0f;
static bool gLookMode = false;

Ball gBall(32, 32, 1.0f);
Ball gAsteroidBall(6, 6, 1.0f);
vector<BallInstance> gBalls;
vector<BallInstance> gAsteroids;
static BallInstance gMoon;
static bool gMoonInitialized = false;

int gSelected = -1;
bool gFollowing = false;
float gLastFollowPos[3];

const float GravitationalConstant = 2.56e-4f;
float gTimeScale = 1.0f;

static const float kBallScale = 0.6f;
static const float kPlanetMassScale = 0.02f;
static const float kSelectedBallScale = 0.75f;
static const size_t kMaxTrailPoints = 1500;     // points kept per planet
static const float  kTrailInterval  = 0.03f;   // seconds between recorded points
static const float kTrailFraction = 0.35f;     // fraction of one orbit the trail covers

const float outlineColor[3] = { 0.0f, 1.0f, 1.0f };

static const char* VertexShaderText =
    "#version 330\n"
    "uniform mat4 gWVP;\n"
    "layout (location = 0) in vec3 Position;\n"
    "out vec4 Color;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = gWVP * vec4(Position, 1.0);\n"
    "    Color = vec4(Position * 0.5 + 0.5, 1.0);\n"
    "}\n";

static const char* FragmentShaderText =
    "#version 330\n"
    "uniform vec3 ballColor;\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(ballColor, 1.0);\n"
    "}\n";

int main(int argc, char** argv)
{
    srand((unsigned int)time(nullptr));
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("ImGui + FreeGLUT Test");

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    GLenum res = glewInit();

    GLint stencilBits = 0;
    glGetIntegerv(GL_STENCIL_BITS, &stencilBits);
    printf("Stencil bits: %d\n", stencilBits);

    if (res != GLEW_OK){
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(res) );
        return -1;
    }

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);
    glClearStencil(0);

    pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT,
                             Vector3f(0.0f, 30.0f, -60.0f),
                             Vector3f(0.0f, -30.0f, 60.8f),
                             Vector3f(0.0f, 1.0f, 0.0f));

    gBall.CreateBall();
    gAsteroidBall.CreateBall();
    InitTrails();

    SolarSystemDemo();
    CompileShaders(gShaderProgram, VertexShaderText, FragmentShaderText, gWVPLocation, gColorLocation);

    glutDisplayFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutKeyboardFunc(KeyboardCB);
    glutKeyboardUpFunc(KeyboardUpCB);
    glutPassiveMotionFunc(PassiveMouseCB);
    glutMotionFunc(MouseMotionCB);
    glutMouseFunc(MouseButtonCB);
    glutMouseWheelFunc(MouseWheelCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutSpecialUpFunc(SpecialKeyboardUpCB);

    glutMainLoop();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
