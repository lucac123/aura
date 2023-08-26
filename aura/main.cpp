#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include "Shader.h"
#include "Texture3D.h"
#include "FrameBuffer.h"
#include "ViewportCamera.h"
#include <algorithm>

const unsigned int WINDOW_DIM[] = { 1920,1080 };

const unsigned int GRID_DIM[] = { 128, 128, 128 };

float *field = nullptr;

//const float* field[GRID_DIM[0] * GRID_DIM[1] * GRID_DIM[2]];

Camera camera = Camera(glm::vec3(0, 0, 0), 3.0f, glm::radians(90.0f), 0.0f);
bool camera_pan = false;

float cursor_x, cursor_y;
float cursor_last_x, cursor_last_y;

float z_index = 0.0f;

void processInput(GLFWwindow* window, float delta_time);

void mouse_callback(GLFWwindow* window, double x, double y);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

float* generateScalarField(int width, int height, int depth, float scale);

int main() {
    /* GLFW WINDOW */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_DIM[0], WINDOW_DIM[1], "Aura", nullptr, nullptr);
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_CULL_FACE);

    /* GEOMETRY */
    float domain_vertex_data[] = {
        //position		          //texel
        -0.5, -0.5, -0.5,         0, 0, 0,              // BACK FACE
         0.5,  0.5, -0.5,         1,  1, 0,
         0.5, -0.5, -0.5,         1, 0, 0,
         0.5,  0.5, -0.5,         1,  1, 0,
        -0.5, -0.5, -0.5,         0, 0, 0,
        -0.5,  0.5, -0.5,         0,  1, 0,

        -0.5, -0.5,  0.5,         0, 0,  1,             // FRONT FACE
         0.5, -0.5,  0.5,         1, 0,  1,
         0.5,  0.5,  0.5,         1,  1,  1,
         0.5,  0.5,  0.5,         1,  1,  1,
        -0.5,  0.5,  0.5,         0,  1,  1,
        -0.5, -0.5,  0.5,         0, 0,  1,

        -0.5,  0.5,  0.5,         0,  1,  1,            // LEFT FACE
        -0.5,  0.5, -0.5,         0,  1, 0,
        -0.5, -0.5, -0.5,         0, 0, 0,
        -0.5, -0.5, -0.5,         0, 0, 0,
        -0.5, -0.5,  0.5,         0, 0,  1,
        -0.5,  0.5,  0.5,         0,  1,  1,

         0.5,  0.5,  0.5,         1,  1,  1,            // RIGHT FACE
         0.5, -0.5, -0.5,         1, 0, 0,
         0.5,  0.5, -0.5,         1,  1, 0,
         0.5, -0.5, -0.5,         1, 0, 0,
         0.5,  0.5,  0.5,         1,  1,  1,
         0.5, -0.5,  0.5,         1, 0,  1,

        -0.5, -0.5, -0.5,         0, 0, 0,              // BOTTOM FACE
         0.5, -0.5, -0.5,         1, 0, 0,
         0.5, -0.5,  0.5,         1, 0,  1,
         0.5, -0.5,  0.5,         1, 0,  1,
        -0.5, -0.5,  0.5,         0, 0,  1,
        -0.5, -0.5, -0.5,         0, 0, 0,

        -0.5,  0.5, -0.5,         0,  1, 0,             // TOP FACE
         0.5,  0.5,  0.5,         1,  1,  1,
         0.5,  0.5, -0.5,         1,  1, 0,
         0.5,  0.5,  0.5,         1,  1,  1,
        -0.5,  0.5, -0.5,         0,  1, 0,
        -0.5,  0.5,  0.5,         0,  1,  1
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

    render.setUniform("uGridDim", GRID_DIM[0], GRID_DIM[1], GRID_DIM[2]);
    render.setUniform("uBoxVector", 1, 1, 1);
    render.setUniform("uBoxLowerCorner", -0.5f, -0.5f, -0.5f);


    /* TEXTURE */
    field = generateScalarField(GRID_DIM[0], GRID_DIM[1], GRID_DIM[2], 1.0f);
    Texture3D density_field(field, GL_R16F, GRID_DIM[0], GRID_DIM[1], GRID_DIM[2]);

    density_field.bind();
    render.setUniform("uField", 0);

    float last_frame = static_cast<float>(glfwGetTime());
    float delta_time = 0;
    while (!glfwWindowShouldClose(window)) {
        // Timestep
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, delta_time);

        if (camera_pan) {
            camera.processPan(cursor_x - cursor_last_x, cursor_last_y-cursor_y);
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

        //render.setUniform("z", z_index);
        render.setUniform("uCameraPosition", camera.position);
        render.setUniform("uCameraFront", camera.front);

        glBindVertexArray(domain_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &domain_vbo);
    glDeleteVertexArrays(1, &domain_vao);

    delete[] field;

    return 0;
}

void processInput(GLFWwindow* window, float delta_time) {
    
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_E && action == GLFW_RELEASE) {
        int z = z_index * GRID_DIM[2];
        std::cout << "Slice for z = " << z << std::endl;

        /*for (int j = 0; j < GRID_DIM[1]; j++) {
            for (int i = 0; i < GRID_DIM[0]; i++)
                std::cout << field[GRID_DIM[0] * GRID_DIM[1] * z + GRID_DIM[0] * j + i] << ' ';

            std::cout << std::endl;
        }*/
    }
    if (key == GLFW_KEY_RIGHT && z_index <= 0.99f) {
        z_index += 0.01f;
    }
    if (key == GLFW_KEY_LEFT && z_index >= 0.01f) {
        z_index -= 0.01f;
    }
}

void mouse_callback(GLFWwindow* window, double x, double y) {
    cursor_x = x;
    cursor_y = y;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1) {
        if (action == GLFW_PRESS && mods == GLFW_MOD_ALT) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera_pan = true;
            cursor_last_x = cursor_x;
            cursor_last_y = cursor_y;
        }
        else if (action == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera_pan = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera_pan = true;
            cursor_last_x = cursor_x;
            cursor_last_y = cursor_y;
        }
        else if (action == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera_pan = false;
        }
    }
}

float* generateScalarField(int width, int height, int depth, float scale) {
	float* field = new float[height * width * depth];
    float threshold = 0.3f;
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
            for (int k = 0; k < depth; k++) {

                float x, y, z;
                int index = width * height * k + width * j + i;

                /* EQUATIONS */
                /* LINEAR
                x = scale * 0.1 * ((float)i / width);
                y = scale * 0.1 * ((float)j / height);
                z = scale * 0.1 * ((float)k / depth);
                field[index] = x + y + z;
                //*/

                /* INVERSE SPHERE
                x = scale * 0.8 * ((float)i / width - 0.5f);
                z = scale * 0.8 * ((float)k / width - 0.5f);
                y = scale * 0.8 * ((float)j / width - 0.5f);
                float radius_squared = x * x + y * y + z * z;
                field[index] = radius_squared;
                //*/

                //* MISC
                x = scale * 20.0f * ((float)i / width - 0.5f);
                z = scale * 20.0f * ((float)k / width - 0.5f);
                y = scale * 20.0f * ((float)j / width - 0.5f);
                float radius_squared = x * x + y * y + z * z;
                field[index] = (float)(pow(2.71828, -radius_squared)) * scale * (pow(sin(x * y * z), 2) < threshold) ? pow(sin(x * y * z), 2) : 0;
                field[index] += (float)(pow(2.71828, -radius_squared) * (1 / scale) * (pow(sin(scale * radius_squared), 2)));


            }
	return field;
}