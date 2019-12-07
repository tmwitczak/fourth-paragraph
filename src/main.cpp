#include <iostream>

using namespace std;
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

vec3 ImVec4ToVec3(ImVec4 a) {
    return vec3(a.x, a.y, a.z);
}

ImVec4 Vec3ToImVec4(vec3 a) {
    return {a.x, a.y, a.z, 1.0};
}

enum LightType {
    LT_POINT,
    LT_DIRECTIONAL,
    LT_SPOT
};

struct LightParameters {
    string name;
    LightType type;
    float enable;
    ImVec4 direction;
    ImVec4 position;
    float angle;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    float ambientIntensity;
    ImVec4 ambientColor;
    float diffuseIntensity;
    ImVec4 diffuseColor;
    float specularIntensity;
    ImVec4 specularColor;
    float specularShininess;

    void setShaderParameters(shared_ptr<Shader> shader) {
        shader->uniform1f(name + ".enable", enable);

        shader->uniform3f(name + ".direction", ImVec4ToVec3(direction));
        shader->uniform3f(name + ".position", ImVec4ToVec3(position));
        shader->uniform1f(name + ".angle", angle);

        shader->uniform1f(name + ".attenuationConstant", attenuationConstant);
        shader->uniform1f(name + ".attenuationLinear", attenuationLinear);
        shader->uniform1f(name + ".attenuationQuadratic", attenuationQuadratic);

        shader->uniform1f(name + ".ambientIntensity", ambientIntensity);
        shader->uniform3f(name + ".ambientColor",
                          ImVec4ToVec3(ambientColor));
        shader->uniform1f(name + ".diffuseIntensity", diffuseIntensity);
        shader->uniform3f(name + ".diffuseColor",
                          ImVec4ToVec3(diffuseColor));
        shader->uniform1f(name + ".specularIntensity", specularIntensity);
        shader->uniform3f(name + ".specularColor",
                          ImVec4ToVec3(specularColor));
        shader->uniform1f(name + ".specularShininess", specularShininess);
    }
};

LightParameters lightDirectional = {
        "lightDirectional",
        LT_DIRECTIONAL,
        0.75,
        {1.0, -1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 1.0},
        radians(0.0),
        0.0, 0.0, 0.0,
        0.1, {1.0, 1.0, 1.0, 1.0},
        1.0, {1.0, 1.0, 1.0, 1.0},
        0.5, {1.0, 1.0, 1.0, 1.0}, 32.0
};
LightParameters lightPoint = {
        "lightPoint",
        LT_POINT,
        1.0,
        {0.0, 0.0, 0.0, 1.0},
        {0.0, 10.0, 0.0, 1.0},
        radians(0.0),
        0.0, 0.1, 0.001,
        0.0, {1.0, 1.0, 1.0, 1.0},
        1.0, {1.0, 0.57, 0.16, 1.0},
        1.0, {1.0, 1.0, 1.0, 1.0}, 32.0
};
LightParameters lightSpot1 = {
        "lightSpot1",
        LT_SPOT,
        0.8,
        {1.0, -1.0, 0.0, 1.0},
        {-10.0, 5.0, 0.0, 1.0},
        radians(60.0),
        0.5, 0.025, 0.0,
        0.35, {1.0, 0.0, 0.0, 1.0},
        0.5, {1.0, 0.0, 0.0, 1.0},
        1.0, {1.0, 1.0, 1.0, 1.0}, 128.0
};
LightParameters lightSpot2 = {
        "lightSpot2",
        LT_SPOT,
        0.35,
        {-1.0, -1.0, -1.0, 1.0},
        {0.0, 10.0, 20.0, 1.0},
        radians(45.0),
        1.0, 0.01, 0.0,
        0.2, {0.0, 0.75, 1.0, 1.0},
        1.0, {0.0, 0.75, 1.0, 1.0},
        1.0, {1.0, 1.0, 1.0, 1.0}, 2.0
};

// /////////////////////////////////////////////////// Struct: GraphNode //
struct GraphNode {
    vector<mat4> transform;
    vector<shared_ptr<Renderable>> model;
    GLuint overrideTexture;
    vector<shared_ptr<GraphNode>> children;

    GraphNode() : overrideTexture(0) {}

    void render(mat4 const &vp = mat4(1.0f)) {
        for (int i = 0; i < model.size(); i++) {
            mat4 renderTransform = vp * transform[i];

            if (model[i]) {
                model[i]->shader->use();
                model[i]->shader->uniformMatrix4fv("transform",
                                                   value_ptr(
                                                           renderTransform));
                model[i]->shader->uniformMatrix4fv("world",
                                                   value_ptr(
                                                           transform[i]));
                model[i]->shader->uniformMatrix4fv("vpMatrix",
                                                   value_ptr(vp));
                model[i]->shader->uniform3f("viewPos", cameraPos.x,
                                            cameraPos.y,
                                            cameraPos.z);

                lightDirectional.setShaderParameters(model[i]->shader);
                lightPoint.setShaderParameters(model[i]->shader);
                lightSpot1.setShaderParameters(model[i]->shader);
                lightSpot2.setShaderParameters(model[i]->shader);

                model[i]->render(model[i]->shader, overrideTexture);
            }
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
bool showLightDummies = true;

// ----------------------------------------------------------- Models -- //
shared_ptr<Renderable> ground, amplifier, guitar, lightbulb;

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

    ImGui::GetIO().Fonts->AddFontFromFileTTF("res/fonts/montserrat.ttf",
                                             12.0f, nullptr);
}

void constructTabForLight(LightParameters &light) {
    ImGui::SliderFloat("Enable", &light.enable, 0.0f, 1.0f);
    if (light.type != LT_POINT) {
        ImGui::SliderFloat3("Direction", (float *) &light.direction, -1.0f,
                            1.0f);
    }
    if (light.type != LT_DIRECTIONAL){
        ImGui::SliderFloat3("Position", (float *) &light.position, -100.0f,
                            100.0f);
    }
    if (light.type == LT_SPOT) {
        ImGui::SliderAngle("Angle", &light.angle, 0.0f, 90.0f);
    }
    ImGui::Separator();
    if (light.type != LT_DIRECTIONAL) {
        ImGui::Text("Attenuation");
        ImGui::SliderFloat("Constant", &light.attenuationConstant, 0.0f, 1.0f);
        ImGui::SliderFloat("Linear", &light.attenuationLinear, 0.0f, 1.0f);
        ImGui::SliderFloat("Quadratic", &light.attenuationQuadratic, 0.0f, 1.0f);
        ImGui::Separator();
    }

    ImGui::Text("Ambient");
    ImGui::SliderFloat("Intensity", &light.ambientIntensity, 0.0f, 1.0f);
    ImGui::ColorEdit3("Color", (float *) &light.ambientColor);
    ImGui::Separator();

    ImGui::Text("Diffuse");
    ImGui::SliderFloat("Intensity ", &light.diffuseIntensity, 0.0f, 1.0f);
    ImGui::ColorEdit3("Color ", (float *) &light.diffuseColor);
    ImGui::Separator();

    ImGui::Text("Specular");
    ImGui::SliderFloat("Intensity  ", &light.specularIntensity, 0.0f,
                       1.0f);
    ImGui::ColorEdit3("Color  ", (float *) &light.specularColor);
    ImGui::SliderFloat("Shininess", &light.specularShininess, 1.0f,
                       128.0f);
    ImGui::Separator();

    ImGui::EndTabItem();
}

void prepareUserInterfaceWindow() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Task 4", nullptr,
                 ImGuiWindowFlags_NoDecoration);
    {
        if (ImGui::Button("Toggle wireframe mode")) {
            wireframeMode = !wireframeMode;
        }
        if (ImGui::Button("Toggle light dummies")) {
            showLightDummies = !showLightDummies;
        }
        ImGui::NewLine();

        ImGui::BeginTabBar("Lights");

        if (ImGui::BeginTabItem("Directional")) {
            constructTabForLight(lightDirectional);
        }
        if (ImGui::BeginTabItem("Point")) {
            constructTabForLight(lightPoint);
        }
        if (ImGui::BeginTabItem("Spot 1")) {
            constructTabForLight(lightSpot1);
        }
        if (ImGui::BeginTabItem("Spot 2")) {
            constructTabForLight(lightSpot2);
        }

        ImGui::EndTabBar();

        ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetWindowSize(ImVec2(300.0f, WINDOW_HEIGHT));
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
    glfwWindowHint(GLFW_SAMPLES, 4);
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
    angle += glm::radians(30.0f) * deltaTime;

    lightPoint.position = Vec3ToImVec4(
            glm::rotate(identity, angle, vec3(0.0f, 1.0f, 0.0f)) *
            glm::vec4(25, 0, 0, 1));

    // Scene elements
    scene.transform.clear();
    scene.model.clear();

    scene.transform.push_back(identity);
    scene.model.push_back(ground);

//    scene.transform.push_back(identity);
//    scene.model.push_back(amplifier);
//
//    if (showLightDummies) {
//        scene.transform.push_back(glm::translate(mat4(1), ImVec4ToVec3( lightPoint.position)));
//        scene.model.push_back(lightbulb);
//
//        scene.transform.push_back(glm::translate(mat4(1), ImVec4ToVec3( lightSpot1.position)));
//        scene.model.push_back(lightbulb);
//
//        scene.transform.push_back(glm::translate(mat4(1), ImVec4ToVec3( lightSpot2.position)));
//        scene.model.push_back(lightbulb);
//    }
}

void mouseCallback(GLFWwindow *window, double x, double y) {
    // Remove first iteration's shutter
    if (isThisFirstIteration) {
        mousePositionLastX = x;
        mousePositionLastY = y;
        isThisFirstIteration = false;
    }

    // Calculate mouse offset and update its position
    GLfloat offsetX =
            mouseSensitivityFactor * (x - mousePositionLastX);
    GLfloat offsetY =
            mouseSensitivityFactor * (-(y - mousePositionLastY));

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
    float const CAMERA_SPEED = 8.0f;

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


void setupOpenGL() {
    setupGLFW();
    createWindow();
    initializeOpenGLLoader();

    glEnable(GL_MULTISAMPLE);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);

    plywoodTexture = loadTextureFromFile(
            "res/textures/light.jpg");
    metalTexture = loadTextureFromFile("res/textures/metal.jpg");

    ground = make_shared<Model>("res/models/ground.obj");
//    amplifier = make_shared<Model>("res/models/teapot.obj");
//    lightbulb = make_shared<Model>("res/models/light.obj");

    modelShader = make_shared<Shader>("res/shaders/model/vertex.glsl",
                                      "res/shaders/model/geometry.glsl",
                                      "res/shaders/model/fragment.glsl");

    sphereShader = make_shared<Shader>(
            "res/shaders/sphere/vertex.glsl",
            "res/shaders/sphere/geometry.glsl",
            "res/shaders/sphere/fragment.glsl");

    ground->shader = modelShader;
//    amplifier->shader = modelShader;
//    lightbulb->shader = sphereShader;

    setupDearImGui();
}

// //////////////////////////////////////////////////////////// Clean up //
void cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    sphereShader = nullptr;
    modelShader = nullptr;

    lightbulb = nullptr;
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
        cameraPos = lerp(cameraPos, cameraPosTarget, 0.1f);
        cameraFront = lerp(cameraFront, cameraFrontTarget, 0.1f);

        mat4 const projection = perspective(radians(60.0f),
                                            ((float) displayWidth) /
                                            ((float) displayHeight),
                                            0.01f, 100.0f);
        mat4 const view = lookAt(cameraPos,
                                 cameraPos + cameraFront,
                                 cameraUp);
        vp = projection * view;

        setupSceneGraph(deltaTime.count(), displayWidth,
                        displayHeight);
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
