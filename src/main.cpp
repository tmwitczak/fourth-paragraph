// //////////////////////////////////////////////////////////// Includes //
#include "model.hpp"
#include "opengl-headers.hpp"
#include "shader.hpp"

#include <chrono>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

using sysclock = std::chrono::system_clock;
using sec = std::chrono::duration<float>;

// ////////////////////////////////////////////////////////////// Usings //
using glm::mat4;
using glm::perspective;
using glm::radians;
using glm::rotate;
using glm::scale;
using glm::value_ptr;
using glm::vec3;
using glm::normalize;
using glm::cross;

using std::array;
using std::begin;
using std::cerr;
using std::end;
using std::endl;
using std::exception;
using std::make_unique;
using std::make_shared;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::shared_ptr;
using std::vector;

template<typename T>
T clamp(T x, T min, T max) {
    return ((x < min) ? min : ((x > max) ? max : x));
}

template<typename T>
T lerp(T a, T b, float alpha) {
    return (1.0f - alpha) * a + alpha * b;
}

vec3 cameraPos(5.0f);
mat4 vp;

// /////////////////////////////////////////////////// Struct: GraphNode //
struct GraphNode {
    mat4 transform;
    shared_ptr<Renderable> model;
    GLuint overrideTexture;
    vector<shared_ptr<GraphNode>> children;

    GraphNode() : overrideTexture(0), model(nullptr) {}

    void render(mat4 const &vp = mat4(1.0f)) {
        mat4 renderTransform = vp * transform;

        if (model) {
            model->shader->use();
            model->shader->uniformMatrix4fv("transform",
                                            value_ptr(renderTransform));
            model->shader->uniformMatrix4fv("world",
                                            value_ptr(transform));
            model->shader->uniformMatrix4fv("vpMatrix",
                                            value_ptr(vp));
            model->shader->uniform3f("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

            model->render(model->shader, overrideTexture);
        }

//        for (auto const &child : children) {
//            child->render(renderTransform);
//        }
    }
};

// /////////////////////////////////////////////////////////// Constants //
int const WINDOW_WIDTH = 1589;
int const WINDOW_HEIGHT = 982;
char const *WINDOW_TITLE = "Tomasz Witczak 216920 - Zadanie 4";

// /////////////////////////////////////////////////////////// Variables //
// ----------------------------------------------------------- Window -- //
GLFWwindow *window = nullptr;

// ---------------------------------------------------------- Shaders -- //
shared_ptr<Shader> modelShader,
        sphereShader;

// --------------------------------------------------------- Textures -- //
GLuint plywoodTexture = 0,
        metalTexture = 0;

// ----------------------------------------------------------- Camera -- //
vec3 cameraFront(1.0f, 0.0f, 0.0f),
        cameraUp(0.0f, 1.0f, 0.0f);

vec3 cameraPosTarget = cameraPos;
vec3 cameraFrontTarget = cameraFront;

GLfloat pitch = 0.0f,
        yaw = 0.0f;

bool grabMouse = true;
bool isThisFirstIteration = true;

// ------------------------------------------------------------ Mouse -- //
GLfloat mousePositionLastX = WINDOW_WIDTH / 2.0f;
GLfloat mousePositionLastY = WINDOW_HEIGHT / 2.0f;
GLfloat mouseSensitivityFactor = 0.01f;

// ------------------------------------------------------ Scene graph -- //
GraphNode scene;

// --------------------------------------------------- Rendering mode -- //
bool wireframeMode = false;

// ----------------------------------------------------------- Models -- //
shared_ptr<Renderable> sphere, amplifier, guitar, orbit;

// //////////////////////////////////////////////////////////// Textures //
GLuint loadTextureFromFile(string const &filename) {
    // Generate OpenGL resource
    GLuint texture;
    glGenTextures(1, &texture);

    // Setup the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    {
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Load texture from file
        stbi_set_flip_vertically_on_load(true);

        int imageWidth, imageHeight, imageNumberOfChannels;
        unsigned char *textureData = stbi_load(
                filename.c_str(),
                &imageWidth, &imageHeight,
                &imageNumberOfChannels, 0);

        if (textureData == nullptr) {
            throw exception("Failed to load texture!");
        }

        // Pass image to OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     imageWidth, imageHeight, 0,
                     [&]() -> GLenum {
                         switch (imageNumberOfChannels) {
                             case 1:
                                 return GL_RED;
                             case 3:
                                 return GL_RGB;
                             case 4:
                                 return GL_RGBA;
                             default:
                                 return GL_RGB;
                         }
                     }(),
                     GL_UNSIGNED_BYTE, textureData);

        // Generate mipmap for loaded texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // After loading into OpenGL - release the raw resource
        stbi_image_free(textureData);
    }

    // Return texture's ID
    return texture;
}

// /////////////////////////////////////////////////////// Class: Sphere //
class Sphere : public Renderable {
public:
    static constexpr int SUBDIVISION_LEVEL_MIN = 1;
    static constexpr int SUBDIVISION_LEVEL_MAX = 7;

private:
    GLuint vao, vbo;
    GLuint texture;

    vec3 const point{0.0f, 0.0f, 0.0f};

public:
    static int subdivisionLevel;

    Sphere() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex), &point,
                     GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vec3), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        texture = loadTextureFromFile("res/textures/jupiter.jpg");
    }

    ~Sphere() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void render(shared_ptr<Shader> shader,
                GLuint const overrideTexture) const {
        shader->use();

        sphereShader->uniform1i("subdivisionLevelHorizontal",
                                subdivisionLevel + 2);
        sphereShader->uniform1i("subdivisionLevelVertical",
                                subdivisionLevel + 1);

        sphereShader->uniform1i("texture0", 0);

        glEnable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,
                      overrideTexture != 0 ? overrideTexture : texture);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, 1);
        glBindVertexArray(0);
    }
};

int Sphere::subdivisionLevel = Sphere::SUBDIVISION_LEVEL_MAX;


// ////////////////////////////////////////////////////// User interface //
void setupDearImGui() {
    constexpr char const *GLSL_VERSION = "#version 430";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GLSL_VERSION);

    ImGui::StyleColorsLight();
}

void prepareUserInterfaceWindow() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Zadanie 3");
    {
        ImGui::SliderInt("Szczegolowosc kuli",
                         &Sphere::subdivisionLevel,
                         Sphere::SUBDIVISION_LEVEL_MIN,
                         Sphere::SUBDIVISION_LEVEL_MAX);
        if (ImGui::Button("Tryb siatki")) {
            wireframeMode = !wireframeMode;
        }
        ImGui::SliderFloat("Kamera (x)", &cameraPos.x, 0.0f, 4.0f);
        ImGui::SliderFloat("Kamera (y)", &cameraPos.y, 0.0f, 4.0f);
        ImGui::SliderFloat("Kamera (z)", &cameraPos.z, 0.5f, 4.0f);

        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetWindowSize(ImVec2(400.0f, 150.0f));
    }
    ImGui::End();
    ImGui::Render();
}

// //////////////////////////////////////////////////////// Setup OpenGL //
void setupGLFW() {
    glfwSetErrorCallback(
            [](int const errorNumber,
               char const *description) {
                cerr << "GLFW;"
                     << "Error " << errorNumber
                     << "; "
                     << "Description: "
                     << description;
            });
    if (!glfwInit()) {
        throw exception("glfwInit error");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,
                   GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void createWindow() {
    window = glfwCreateWindow(WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              WINDOW_TITLE,
                              nullptr,
                              nullptr);
    if (window == nullptr) {
        throw exception("glfwCreateWindow error");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // Enable vertical synchronization
}

void initializeOpenGLLoader() {
    bool failedToInitializeOpenGL = false;
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    failedToInitializeOpenGL = (gl3wInit() != 0);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    failedToInitializeOpenGL = (glewInit() != GLEW_OK);
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    failedToInitializeOpenGL = !gladLoadGLLoader(
            (GLADloadproc) glfwGetProcAddress);
#endif
    if (failedToInitializeOpenGL) {
        throw exception(
                "Failed to initialize OpenGL loader!");
    }
}

void setupSceneGraph(float const deltaTime, float const displayWidth,
                     float const displayHeight) {
    static mat4 const identity = mat4(1.0f);
    static float angle = 0.0f;
    angle += glm::radians(45.0f) * deltaTime;

    // Scene elements
//    shared_ptr<GraphNode> gibson = make_shared<GraphNode>();
//    gibson->transform =
//            glm::rotate(identity, angle, vec3(0.0f, 1.0f, 0.0f)) *
//            glm::translate(identity, vec3(-1.0f, 0.0f, 0.0f)) *
//            glm::rotate(identity, -angle, vec3(0.5f, 0.25f, 0.0f)) *
//            glm::scale(identity, vec3(0.1f));
//    gibson->model = guitar;
//    gibson->overrideTexture = plywoodTexture;
//
//    shared_ptr<GraphNode> ball = make_shared<GraphNode>();
//    ball->transform =
//            glm::rotate(identity, angle, vec3(0.0f, 1.0f, 0.0f)) *
//            glm::translate(identity, vec3(1.0f, 0.0f, 0.0f)) *
//            glm::rotate(identity, 2.0f * angle, vec3(0.0f, 0.0f, 1.0f)) *
//            glm::scale(identity, vec3(0.1f));
//    ball->model = sphere;
//    ball->overrideTexture = plywoodTexture;
//
//    shared_ptr<GraphNode> secondOrbit = make_shared<GraphNode>();
//    secondOrbit->transform =
//            glm::rotate(identity, glm::radians(45.0f),
//                        vec3(1.0f, 0.0f, 0.0f)) *
//            glm::scale(identity, vec3(0.5f));
//    secondOrbit->model = orbit;
//    secondOrbit->overrideTexture = plywoodTexture;
//    secondOrbit->children.clear();
//    secondOrbit->children.push_back(ball);
//    secondOrbit->children.push_back(gibson);

//    shared_ptr<GraphNode> amp = make_shared<GraphNode>();
//    amp->transform =
//            glm::rotate(identity, 1.5f * angle, vec3(1.0f, 0.0f, 1.0f)) *
//            glm::scale(identity, vec3(0.004f));
//    amp->model = amplifier;
//    amp->overrideTexture = metalTexture;

//    shared_ptr<GraphNode> otherSystem = make_shared<GraphNode>();
//    otherSystem->transform =
//            glm::rotate(identity, angle, vec3(0.0f, 1.0f, 0.0f)) *
//            glm::translate(identity, vec3(-1.0f, 0.0f, 0.0f));
//    otherSystem->children.clear();
//    otherSystem->children.push_back(secondOrbit);
//    otherSystem->children.push_back(amp);
//
//    shared_ptr<GraphNode> jupiter = make_shared<GraphNode>();
//    jupiter->transform =
//            glm::rotate(identity, angle, vec3(0.0f, 1.0f, 0.0f)) *
//            glm::translate(identity, vec3(1.0f, 0.0f, 0.0f)) *
//            glm::rotate(identity, -angle, vec3(0.0f, 1.0f, 1.0f)) *
//            glm::scale(identity, vec3(0.3f));
//    jupiter->model = sphere;
//    jupiter->overrideTexture = metalTexture;
//
//    shared_ptr<GraphNode> firstOrbit = make_shared<GraphNode>();
//    firstOrbit->transform =
//            glm::rotate(identity, glm::radians(90.0f),
//                        vec3(1.0f, 0.0f, 0.0f)) *
//            glm::scale(identity, vec3(10.0f));
//    firstOrbit->model = orbit;
//    firstOrbit->overrideTexture = metalTexture;
//    firstOrbit->children.clear();
//    firstOrbit->children.push_back(jupiter);
//    firstOrbit->children.push_back(otherSystem);
//
//    shared_ptr<GraphNode> lonelyBlue = make_shared<GraphNode>();
    scene.transform =
            identity;
//            glm::translate(identity, vec3(1.25f, 0.5f, -0.5f)) *
//            glm::scale(identity, vec3(0.0125f));
    scene.model = amplifier;
//
//    shared_ptr<GraphNode> ball2 = make_shared<GraphNode>();
//    ball2->transform =
//            glm::translate(identity, vec3(0.75f, 0.0f, 0.75f)) *
//            glm::rotate(identity, glm::radians(90.0f),
//                        vec3(1.0f, 0.0f, 0.0f)) *
//            glm::scale(identity, vec3(0.6f));
//    ball2->model = sphere;
//
//    shared_ptr<GraphNode> notLonelyBlue = make_shared<GraphNode>();
//    notLonelyBlue->transform =
//            glm::translate(identity, vec3(-1.0f, 0.0f, 0.0f)) *
//            glm::rotate(identity, glm::radians(90.0f),
//                        vec3(1.0f, 0.0f, 0.0f)) *
//            glm::scale(identity, vec3(0.1f));
//    notLonelyBlue->model = guitar;
//    notLonelyBlue->children.clear();
//    notLonelyBlue->children.push_back(firstOrbit);

    // Scene
//    mat4 const projection = perspective(radians(60.0f),
//                                        ((float) displayWidth) /
//                                        ((float) displayHeight),
//                                        0.01f, 100.0f);
//
//    cameraPos = lerp(cameraPos, cameraPosTarget, 0.1f);
//    cameraFront = lerp(cameraFront, cameraFrontTarget, 0.1f);
//
//    mat4 const view = lookAt(cameraPos,
//                             cameraPos + cameraFront,
//                             cameraUp);
//
//    vp = projection * view;

//    scene.transform = projection * view;
    scene.children.clear();
//    scene.children.push_back(ball2);
//    scene.children.push_back(lonelyBlue);
//    scene.children.push_back(notLonelyBlue);
}

void mouseCallback(GLFWwindow *window, double x, double y) {
    // Remove first iteration's shutter
    if (isThisFirstIteration) {
        mousePositionLastX = x;
        mousePositionLastY = y;
        isThisFirstIteration = false;
    }

    // Calculate mouse offset and update its position
    GLfloat offsetX = mouseSensitivityFactor * (x - mousePositionLastX);
    GLfloat offsetY = mouseSensitivityFactor * (-(y - mousePositionLastY));

    mousePositionLastX = x;
    mousePositionLastY = y;

    // Update camera's Euler angles
    pitch = clamp(pitch + offsetY, radians(-89.0f), radians(89.0f));
    yaw += offsetX;

    // Update camera's front vector
    cameraFrontTarget = normalize(
            vec3(cos(yaw) * cos(pitch),
                 sin(pitch),
                 sin(yaw) * cos(pitch))
    );
}

void handleKeyboardInput(float const deltaTime) {
    float const CAMERA_SPEED = 2.0f;

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        cameraPosTarget +=
                deltaTime * CAMERA_SPEED * normalize(cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cameraPosTarget -=
                deltaTime * CAMERA_SPEED * normalize(cameraFront);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cameraPosTarget -= deltaTime * CAMERA_SPEED *
                           normalize(cross(cameraFront, cameraUp));
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        cameraPosTarget += deltaTime * CAMERA_SPEED *
                           normalize(cross(cameraFront, cameraUp));
    }

    static bool spacePressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS &&
        !spacePressed) {
        grabMouse = !grabMouse;
        spacePressed = true;

        if (grabMouse) {
            isThisFirstIteration = true;
        }

        glfwSetInputMode(window, GLFW_CURSOR,
                         grabMouse ? GLFW_CURSOR_DISABLED
                                   : GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window,
                                 grabMouse ? mouseCallback : nullptr);

    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        spacePressed = false;
    }
}

#include <iostream>
using namespace std;

void setupOpenGL() {
    setupGLFW();
    createWindow();
    initializeOpenGLLoader();

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    plywoodTexture = loadTextureFromFile(
            "res/textures/plywood.jpg");
    metalTexture = loadTextureFromFile("res/textures/metal.jpg");

//    amplifier = make_shared<Model>("res/models/orange-th30.obj");
    amplifier = make_shared<Model>("res/models/teapot.obj");
//    guitar = make_shared<Model>("res/models/gibson-es335.obj");
//    guitar = make_shared<Model>("res/models/gibson-es335.obj");
//    orbit = make_shared<Model>("res/models/orbit.obj");

    modelShader = make_shared<Shader>("res/shaders/model/vertex.glsl",
                                      "res/shaders/model/geometry.glsl",
                                      "res/shaders/model/fragment.glsl");

    sphereShader = make_shared<Shader>("res/shaders/sphere/vertex.glsl",
                                       "res/shaders/sphere/geometry.glsl",
                                       "res/shaders/sphere/fragment.glsl");

    sphere = make_shared<Sphere>();

    amplifier->shader = modelShader;
//    guitar->shader = modelShader;
//    orbit->shader = modelShader;
//    sphere->shader = sphereShader;

    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    sphere = nullptr;

    sphereShader = nullptr;
    modelShader = nullptr;

    orbit = nullptr;
    guitar = nullptr;
    amplifier = nullptr;

    glfwDestroyWindow(window);
    glfwTerminate();
}

// /////////////////////////////////////////////////////////// Main loop //
void performMainLoop() {
    auto previousStartTime = sysclock::now();

    while (!glfwWindowShouldClose(window)) {
        auto const startTime = sysclock::now();
        sec const deltaTime = startTime - previousStartTime;
        previousStartTime = startTime;

        // --------------------------------------------------- Events -- //
        glfwPollEvents();
        handleKeyboardInput(deltaTime.count());

        // ----------------------------------- Get current frame size -- //
        int displayWidth, displayHeight;
        glfwMakeContextCurrent(window);
        glfwGetFramebufferSize(window, &displayWidth,
                               &displayHeight);

        // ------------------------------------------- Clear viewport -- //
        glViewport(0, 0, displayWidth, displayHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --------------------------------------- Set rendering mode -- //
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK,
                      wireframeMode ? GL_LINE : GL_FILL);

        // --------------------------------------------- Render scene -- //
        setupSceneGraph(deltaTime.count(), displayWidth, displayHeight);
        mat4 const projection = perspective(radians(60.0f),
                                            ((float) displayWidth) /
                                            ((float) displayHeight),
                                            0.01f, 100.0f);

        cameraPos = lerp(cameraPos, cameraPosTarget, 0.1f);
        cameraFront = lerp(cameraFront, cameraFrontTarget, 0.1f);

        mat4 const view = lookAt(cameraPos,
                                 cameraPos + cameraFront,
                                 cameraUp);

        vp = projection * view;

        scene.render(vp);

        // ------------------------------------------------------- UI -- //
        prepareUserInterfaceWindow();
        ImGui_ImplOpenGL3_RenderDrawData(
                ImGui::GetDrawData());

        // -------------------------------------------- Update screen -- //
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
}


// //////////////////////////////////////////////////////////////// Main //
int main() {
    try {
        setupOpenGL();
        performMainLoop();
        cleanUp();
    } catch (exception const &exception) {
        cerr << exception.what();
        return 1;
    }
    return 0;
}

// ///////////////////////////////////////////////////////////////////// //
