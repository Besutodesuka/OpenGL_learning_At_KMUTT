#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <array>
#include <iostream>
#include <cmath>
#include <numeric>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_s.h>
#include <stb_image.h>
#include <learnopengl/filesystem.h>

#ifndef PI
#define PI 3.1415926535f
#endif
const int N = 12;
const int RES = 32;
const float radius = 0.7f;
//const float PI = 3.14159265358979323846f;
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const float orbitRadius = 0.7f;
float centerX = 0 / 2.0f;
float centerY = 0 / 2.0f;

float global_omega = PI / 1024; // global angular velocity radians per second
float omega = 0.0f;
const float alpha = PI/1000000; // angular acceleration radians per second squared
float pressed_time = 0.0f; // time the mouse button has been pressed

// illuminate crystal ball when click glass shards particle created with sindwave lightling effect
// spining stars with mouse click and when release it travese and bouncing infinitely leaving the trailing behind with some texture
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
float sign(float val) { return (val < 0.0f) ? -1.0f : 1.0f; }
void get_self_orbit_vector(int index, float pressed_time);
std::array<float, 3> get_global_orbit_vector(int index, float pressed_time);
std::vector<std::array<float, 3>> star(
    float centerX,
    float centerY,
    float shape,
    float radius,
    float points
); 

std::vector<std::array<float, 3>> allStarVertices, center, center_vertex, orb;
// Firefly particle state
struct Particle { float x, y, vx, vy; };
std::vector<Particle> fireflies;

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "My Generative Art", NULL, NULL);

    if (!window)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    Shader ourShader("5.1.transform.vs", "5.1.transform.fs");

    // Enable alpha blending for PNGs
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Allow shaders to control point size (for fireflies)
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Load orb texture
    unsigned int orbTex = 0;
    glGenTextures(1, &orbTex);
    glBindTexture(GL_TEXTURE_2D, orbTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;

	orb = star(0.0f, 0.0f, 1.0f, orbitRadius/2.0f, 64);
    center = star(centerX, centerY, 1.0f, orbitRadius, N);
    for (int i = 0; i <= N; i++) {
        float angle = 2.0f * PI * (static_cast<float>(i) / N) + (PI / 4.0f);
        center_vertex.push_back({ angle, 0.0f, 0.0f });
    }
    // Generate vertices
    for (const auto& v : center) {
        std::cout << "Center vertex: (" << v[0] << ", " << v[1] << ", " << v[2] << ")\n";
        std::vector<std::array<float, 3>> smallStar = star(
            v[0],      // Use the x-coordinate of the central shape's vertex
            v[1],      // Use the y-coordinate of the central shape's vertex
            3.0f,      // Shape of the small star
            0.05f,      // Radius of the small star
            RES          // Resolution of the small star
        );
        allStarVertices.insert(
            allStarVertices.end(),      // Position to insert at (the end)
            smallStar.begin(),          // Start of the source range
            smallStar.end()             // End of the source range
        );
	}

    // Setup VAO + VBO for dynamic shapes (orb + stars)
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, allStarVertices.size() * sizeof(allStarVertices[0]), allStarVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(0);

    // Create background quad (two triangles covering the screen)
    float bgVertices[] = {
        -1.0f, -1.0f,  0.0f,
         1.0f, -1.0f,  0.0f,
         1.0f,  1.0f,  0.0f,
        -1.0f, -1.0f,  0.0f,
         1.0f,  1.0f,  0.0f,
        -1.0f,  1.0f,  0.0f
    };
    unsigned int bgVAO, bgVBO;
    glGenVertexArrays(1, &bgVAO);
    glGenBuffers(1, &bgVBO);
    glBindVertexArray(bgVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bgVertices), bgVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Load background texture
    unsigned int bgTex = 0;
    glGenTextures(1, &bgTex);
    glBindTexture(GL_TEXTURE_2D, bgTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    {
        int bgW = 0, bgH = 0, bgC = 0;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* bgData = stbi_load(FileSystem::getPath("resources/textures/twinlight.jpg").c_str(), &bgW, &bgH, &bgC, 0);
        if (bgData)
        {
            GLenum bgFormat = bgC == 4 ? GL_RGBA : (bgC == 3 ? GL_RGB : GL_RED);
            glTexImage2D(GL_TEXTURE_2D, 0, bgFormat, bgW, bgH, 0, bgFormat, GL_UNSIGNED_BYTE, bgData);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load background texture (twinlight.jpg)" << std::endl;
        }
        stbi_image_free(bgData);
    }

    ourShader.use();
    glUniform1i(glGetUniformLocation(ourShader.ID, "orbTexture"), 0);
    glUniform1i(glGetUniformLocation(ourShader.ID, "useTexture"), GL_FALSE);
    glUniform4f(glGetUniformLocation(ourShader.ID, "baseColor"), 1.0f, 0.8f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(ourShader.ID, "omegaMax"), 0.05f);
    // Dark purple target for stars; brightness increases with omega in shader
    glUniform4f(glGetUniformLocation(ourShader.ID, "starMaxColor"), 0.45f, 0.10f, 0.65f, 1.0f);
    glUniform1f(glGetUniformLocation(ourShader.ID, "pointSize"), 10.0f);
    glUniform1i(glGetUniformLocation(ourShader.ID, "drawFirefly"), GL_FALSE);

    // Render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 1) Draw background first
        glBindVertexArray(bgVAO);
        glm::mat4 bgTransform = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(bgTransform));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bgTex);
        glUniform1i(glGetUniformLocation(ourShader.ID, "useTexture"), GL_TRUE);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glUniform1i(glGetUniformLocation(ourShader.ID, "useTexture"), GL_FALSE);

        // 2) Draw orb and stars on top
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, orbTex);
        glUniform1i(glGetUniformLocation(ourShader.ID, "useTexture"), GL_TRUE);

		// draw magic orb
        glm::mat4 transform_still = glm::mat4(1.0f);
		transform_still = glm::scale(transform_still, glm::vec3(0.5f+omega*5, 0.5f + omega * 5, 0.0f));
        float angle = (float)glfwGetTime() * omega; // radians = angular velocity * time
        transform_still = glm::rotate(transform_still, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform_still));
        stbi_set_flip_vertically_on_load(true);

        unsigned char* data = stbi_load(FileSystem::getPath("resources/textures/pngtree-mystical-glowing-orb-design-png-image_14727587.png").c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = nrChannels == 4 ? GL_RGBA : (nrChannels == 3 ? GL_RGB : GL_RED);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load orb texture" << std::endl;
        }
        stbi_image_free(data);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, orb.size() * sizeof(orb[0]), orb.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(0);

        glBindVertexArray(VAO);

        glDrawArrays(GL_TRIANGLE_FAN, 0, orb.size());
        glUniform1i(glGetUniformLocation(ourShader.ID, "useTexture"), GL_FALSE);
        // Update star color intensity based on current omega each frame
        glUniform1f(glGetUniformLocation(ourShader.ID, "omega"), omega);
        glUniform4f(glGetUniformLocation(ourShader.ID, "baseColor"), 1.0f, 1.0f, 1.0f, 1.0f);

        std::array<float, 3>* allDataPtr = allStarVertices.data();
        for (int i = 0; i < N; i++) {
            glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            get_self_orbit_vector(i, (float)glfwGetTime());
            transform = glm::translate(transform, glm::vec3((float)center_vertex[i][1], (float)center_vertex[i][2], 0.0f));
            transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
            // Scale star based on current omega (grows with gradient)
            float tStar = glm::clamp((float)(omega / 0.05f), 0.0f, 1.0f);
            float s = 1.0f + tStar * 0.6f; // up to +60% larger
            transform = glm::scale(transform, glm::vec3(s, s, 1.0f));
			//std::array<float, 3> global_vec = get_global_orbit_vector(i, (float)glfwGetTime());
   //         transform = glm::translate(transform, glm::vec3((float)global_vec[0], (float)global_vec[1], 0.0f));
            // get their uniform location and set matrix (using glm::value_ptr)
            unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, ((allStarVertices.size() / N)) * sizeof(allStarVertices[0]), allDataPtr + (i * (RES+ 1)), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(0);

            glBindVertexArray(VAO);

            glDrawArrays(GL_TRIANGLE_FAN, 0, RES+1);
        }

        // 3) Fireflies
        if (fireflies.empty()) {
            const int NUM = 20;
            fireflies.resize(NUM);
            for (int i = 0; i < NUM; ++i) {
                float rx = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
                float ry = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
                float rvx = (((float)rand() / RAND_MAX) - 0.5f) * 0.01f;
                float rvy = (((float)rand() / RAND_MAX) - 0.5f) * 0.01f;
                fireflies[i] = { rx, ry, rvx, rvy };
            }
        }
        for (auto& p : fireflies) {
            p.x += p.vx;
            p.y += p.vy;
            if (p.x < -1.0f || p.x > 1.0f) p.vx = -p.vx;
            if (p.y < -1.0f || p.y > 1.0f) p.vy = -p.vy;
        }
        std::vector<float> pts;
        pts.reserve(fireflies.size() * 3);
        for (const auto& p : fireflies) {
            pts.push_back(p.x);
            pts.push_back(p.y);
            pts.push_back(0.0f);
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(float), pts.data(), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindVertexArray(VAO);
        glm::mat4 id = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "transform"), 1, GL_FALSE, glm::value_ptr(id));
        glUniform1f(glGetUniformLocation(ourShader.ID, "pointSize"), 8.0f);
        glUniform1i(glGetUniformLocation(ourShader.ID, "drawFirefly"), GL_TRUE);
        glDrawArrays(GL_POINTS, 0, (GLsizei)fireflies.size());
        glUniform1i(glGetUniformLocation(ourShader.ID, "drawFirefly"), GL_FALSE);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

void get_self_orbit_vector(int index, float pressed_time) {
    float theta0 = center_vertex[index][0];
	float theta = theta0 + omega* pressed_time; // update angle with angular velocity
	float Xprime = orbitRadius * cos(theta);
    float Yprime = orbitRadius * sin(theta);
	// update the respective vertex centroid and polar coordinate
    center_vertex[index][0] = theta;
    center_vertex[index][1] = Xprime - center[index][0];
    center_vertex[index][2] = Yprime - center[index][1];
	//update new centroid position
    center[index][0] = Xprime;
    center[index][1] = Yprime;
}

std::vector<std::array<float, 3>> star(
    float centerX,
    float centerY,
    float shape,
    float radius,
    float points
) {
    std::vector<std::array<float, 3>> vertices;
    vertices.reserve(points + 1.0f);
    for (int i = 0; i <= points; i++) {
        float angle = 2.0f * PI * (static_cast<float>(i) / points) + (PI / 4.0f);  // offset in need to prevent case where the cos(0)=1 and cos(2PI)=1 which first and last point are the same
        printf("angle: %f\n", angle);
        float cosine = cos(angle);
        float sine = sin(angle);

        float x = centerX + radius * pow(cosine, shape);// *sign(cosine);
        float y = centerY + radius * pow(sine, shape);// *sign(sine);
        printf("x,y: %f %f\n", x, y);
        vertices.push_back({ x, y, 0.0f });
    }
    return vertices;
};

std::array<float, 3> get_global_orbit_vector(int index, float pressed_time){
	float theta0 = acos(center_vertex[index][1] / orbitRadius);
    float theta = theta0 + global_omega * pressed_time; // update angle with angular velocity
    float Xprime = orbitRadius * cos(theta);
    float Yprime = orbitRadius * sin(theta);
    return { Xprime - center[index][0], Yprime - center[index][1], 0.0f};
}

void processInput(GLFWwindow* window)
{
    // get the frame per second
    double pressed_time = glfwGetTime();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		// increase angular velocity
        omega += alpha * pressed_time; // increase angular velocity
		pressed_time = 0.0f; // reset press time
		if (omega > 2.0f) omega = 2.0f; // cap the angular velocity
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
		// start go in the final vector direction with velocity = omega * radius
        // slow down the angular velocity
        if (omega > 0.0f) {
            omega -= alpha * pressed_time; // decrease angular velocity
            if (omega < 0.0f) omega = 0.0f; // cap the angular velocity
        }
        // trigger the bouncing physic and decrease star velocity -0.1f in scalar when bumbing
	}
    std::cout << "Angular velocity: " << omega << " radians/second\n";
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
