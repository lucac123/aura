#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include "Shader.h"
#include "Texture3D.h"
#include "FrameBuffer.h"
#include "ViewportCamera.h"

const unsigned int WINDOW_DIM[] = { 1920,1080 };

const unsigned int GRID_DIM[] = { 512, 512, 512 };

glm::vec3 position = glm::vec3(2, 2, 2);

Camera camera = Camera(position);
bool camera_pan = false;

float cursor_x, cursor_y;
float cursor_last_x, cursor_last_y;

void processInput(GLFWwindow* window, float delta_time);

void mouse_callback(GLFWwindow* window, double x, double y);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

float* generateScalarField(int width, int height, int depth);

int main() {
    /* GLFW WINDOW */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_DIM[0], WINDOW_DIM[1], "Neptune3D", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "average rice window creator" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, cursor);


    /* GLAD */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "didn't get glad got mad" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glDisable(GL_CULL_FACE);

    /* GEOMETRY */
    float domain_vertex_data[] = {
        //position		        //texel
        -0.5, -0.5, -0.5,       0, 0, 0,
         0.5, -0.5, -0.5,       1, 0, 0,
         0.5,  0.5, -0.5,       1, 1, 0,
         0.5,  0.5, -0.5,       1, 1, 0,
        -0.5,  0.5, -0.5,       0, 1, 0,
        -0.5, -0.5, -0.5,       0, 0, 0,

        -0.5, -0.5,  0.5,       0, 0, 1,
         0.5, -0.5,  0.5,       1, 0, 1,
         0.5,  0.5,  0.5,       1, 1, 1,
         0.5,  0.5,  0.5,       1, 1, 1,
        -0.5,  0.5,  0.5,       0, 1, 1,
        -0.5, -0.5,  0.5,       0, 0, 1,

        -0.5,  0.5,  0.5,       0, 1, 1,
        -0.5,  0.5, -0.5,       0, 1, 0,
        -0.5, -0.5, -0.5,       0, 0, 0,
        -0.5, -0.5, -0.5,       0, 0, 0,
        -0.5, -0.5,  0.5,       0, 0, 1,
        -0.5,  0.5,  0.5,       0, 1, 1,

         0.5,  0.5,  0.5,       1, 1, 1,
         0.5,  0.5, -0.5,       1, 1, 0,
         0.5, -0.5, -0.5,       1, 0, 0,
         0.5, -0.5, -0.5,       1, 0, 0,
         0.5, -0.5,  0.5,       1, 0, 1,
         0.5,  0.5,  0.5,       1, 1, 1,

        -0.5, -0.5, -0.5,       0, 1, 0,
         0.5, -0.5, -0.5,       1, 1, 0,
         0.5, -0.5,  0.5,       1, 1, 1,
         0.5, -0.5,  0.5,       1, 1, 1,
        -0.5, -0.5,  0.5,       0, 1, 1,
        -0.5, -0.5, -0.5,       0, 1, 0,

        -0.5,  0.5, -0.5,       0, 1, 0,
         0.5,  0.5, -0.5,       1, 1, 0,
         0.5,  0.5,  0.5,       1, 1, 1,
         0.5,  0.5,  0.5,       1, 1, 1,
        -0.5,  0.5,  0.5,       0, 1, 1,
        -0.5,  0.5, -0.5,       0, 1, 0
    };

    unsigned int domain_vbo, domain_vao;
    glGenBuffers(1, &domain_vbo);
    glGenVertexArrays(1, &domain_vao);

    glBindVertexArray(domain_vao);

    glBindBuffer(GL_ARRAY_BUFFER, domain_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(domain_vertex_data), domain_vertex_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* SHADER */
    Shader render("render.vert", "render.frag");
    render.use();

    /* FRAMEBUFFER */
    FrameBuffer data_renderer(1);

    /* TEXTURE */
    Texture3D velocity0(GL_RGB16F, GRID_DIM[0], GRID_DIM[1], GRID_DIM[2]);


    float last_frame = static_cast<float>(glfwGetTime());
    float delta_time = 0;
    while (!glfwWindowShouldClose(window)) {
        // Timestep
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, delta_time);

        if (camera_pan) {
            camera.processPan(cursor_last_x - cursor_x, cursor_y - cursor_last_y);
            cursor_last_x = cursor_x;
            cursor_last_y = cursor_y;
        }

        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render.use();

        glm::mat4 model = glm::mat4(1.0f);
        render.setUniform("uModel", model);

        glm::mat4 view = camera.getViewMatrix();
        render.setUniform("uView", view);

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)WINDOW_DIM[0] / (float)WINDOW_DIM[1], 0.1f, 100.0f);
        render.setUniform("uProjection", projection);


        glBindVertexArray(domain_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &domain_vbo);
    glDeleteVertexArrays(1, &domain_vao);

    return 0;
}

void processInput(GLFWwindow* window, float delta_time) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.processMovement(FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.processMovement(BACKWARD, delta_time);
    /*if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.processMovement(RIGHT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.processMovement(LEFT, delta_time);*/
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_E)
        std::cout << glm::to_string(camera.position) << std::endl << glm::to_string(camera.up) << std::endl << glm::to_string(camera.front) << std::endl;
}

void mouse_callback(GLFWwindow* window, double x, double y) {
    cursor_x = x;
    cursor_y = y;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1) {
        if (action == GLFW_PRESS && mods == GLFW_MOD_ALT) {
            camera_pan = true;
            cursor_last_x = cursor_x;
            cursor_last_y = cursor_y;
        }
        else if (action == GLFW_RELEASE) {
            camera_pan = false;
        }
    }
}

float* generateScalarField(int width, int height, int depth) {
	float* field = new float[height * width * depth];
	for (int x = 0; x < width; x++)
		for (int y = 0; y < height; y++)
			for (int z = 0; z < depth; z++)
				field[width*height*z+width*y+x] = (float)pow(2.71828, -pow(x-width/2, 2) - pow(y-height/2, 2) - pow(z-depth/2, 2));
	return field;
}