#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>

#include <go/mesh.h>
#include <random>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers> // C++20 for std::numbers::pi

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float PI = 3.1415926535f;


// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

class Jellyfish {
public:
    // Pointers to the shared mesh data.
    // We use pointers because many jellyfish instances will share the same mesh geometry.
    Mesh* headMesh;
    Mesh* bellMesh; // The "knot" part
    Mesh* tailMesh;

    // --- Per-instance properties ---
    glm::vec3 position;
    glm::vec3 scale;
    float rotationAngle;
    glm::vec3 rotationAxis;
    float time = 0.0f;
    std::vector<glm::vec3> tailOffsets;

    // Constructor to set up a jellyfish instance 
    Jellyfish(
        Mesh* head, Mesh* bell, 
        Mesh* tail,
		unsigned int numTails = 5,
        glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 scl = glm::vec3(1.0f),
        float angle = -90.0f,
        glm::vec3 rotAxis = glm::vec3(1.0f, 0.0f, 0.0f))
        : headMesh(head), bellMesh(bell), 
        tailMesh(tail),
        position(pos), scale(scl), rotationAngle(angle), rotationAxis(rotAxis)
    {
        // Here you can also define the relative offsets of the parts
        // For example, the bell might be slightly below the head.
        headOffset = glm::vec3(0.0f, 0.0f, 0.0f);
        bellOffset = glm::vec3(0.0f, 0.0f, 0.0f); // Position the bell below the head
        tailOffset = glm::vec3(0.0f, 0.0f, 0.0f); // Position the tails below the bell
        generateRandomTailOffsets(numTails, 0.6f);
    }

    // The core facade method: Draw the entire jellyfish.
    // It takes a shader so it can set the model matrix for each part.
    void Draw(Shader& shader, glm::mat4& baseModel) {
        // 1. Calculate the base model matrix for the entire jellyfish 
        baseModel = glm::translate(baseModel, position);
        baseModel = glm::rotate(baseModel, glm::radians(rotationAngle), rotationAxis);
        baseModel = glm::rotate(baseModel, glm::radians(10 * this->time), glm::vec3(0.0f, 0.0f, 1.0f));
        baseModel = glm::scale(baseModel, scale);

        // 2. Draw each part, applying its own local offset relative to the base matrix

        // Draw Head
        glm::mat4 headModel = glm::translate(baseModel, headOffset);
        shader.setMat4("model", headModel);
        headMesh->Draw();

        // Draw Bell (Knot)
        glm::mat4 bellModel = glm::translate(baseModel, bellOffset);
        shader.setMat4("model", bellModel);
        bellMesh->Draw();

        // Draw Tail
		// random start position and number of tails
        
        glm::mat4 tailModel;
        for (const auto& offset : tailOffsets) {
            tailModel = glm::rotate(baseModel, glm::radians(90.0f + offset[1]*5.0f), rotationAxis);
            tailModel = glm::translate(tailModel, offset);
            shader.setMat4("model", tailModel);
            tailMesh->Draw(); // Assuming tails are also one mesh
        }
    }

    void Animate(const std::vector<float>& vertices, float time) {
        tailMesh->updateVertices(vertices);
		this->time = time;
	}

private:
    // Relative positions of each part to the jellyfish's main position
    glm::vec3 headOffset;
    glm::vec3 bellOffset;
    glm::vec3 tailOffset;

    // A helper function to keep the constructor clean.
    void generateRandomTailOffsets(unsigned int numTails, float radius) {
        // Use modern C++ for better random number generation
        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
        std::uniform_real_distribution<> distrib(-radius, radius);

        tailOffsets.clear();
        for (unsigned int i = 0; i < numTails; ++i) {
            // Generate random X and Z offsets within a circle
            float x = distrib(gen);
            float z = distrib(gen);
            // The Y offset is constant, placing the top of the tail at the origin
            tailOffsets.push_back(glm::vec3(x, 0.01f, z));
        }
    }
};


// set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
float ground_vertices[] = {
    // positions          // normals           // texture coords
     5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  20.0f, 0.0f,
    -5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 20.0f,
     5.0f, -0.5f,  5.0f,  0.0f, 1.0f, 0.0f,  20.0f, 0.0f,
    -5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  0.0f, 20.0f,
     5.0f, -0.5f, -5.0f,  0.0f, 1.0f, 0.0f,  20.0f, 20.0f
};

void generateJellyfishHead(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // 1. Define resolution parameters
    const int u_segments = 32;
    const int v_segments = 128;

    // 2. Clear out any old data from the vectors
    vertices.clear();
    indices.clear();

    // 3. Reserve memory to avoid reallocations
    vertices.reserve((u_segments + 1) * (v_segments + 1) * 8); // 8 floats per vertex: X, Y, Z, NX, NY, NZ, U, V
    indices.reserve(u_segments * v_segments * 6);              // 6 indices per quad (2 triangles)

    // 4. Define the parametric equations as C++ lambdas
    auto x_func = [&](float u, float v) {
        return -sin(0.5f * PI * u) * -sin(2.0f * PI * v);
        };

    auto y_func = [&](float u, float v) {
        return -sin(0.5f * PI * u) * cos(2.0f * PI * v);
        };

    auto z_func = [&](float u, float v) {
        return cos(0.5f * PI * u);
        };

    // Helper function to calculate normal vector using partial derivatives
    auto calculateNormal = [&](float u, float v) -> glm::vec3 {
        float h = 0.001f; // Small step for numerical differentiation
        
        // Calculate partial derivatives numerically
        glm::vec3 p_center(x_func(u, v), y_func(u, v), z_func(u, v));
        glm::vec3 p_u(x_func(u + h, v), y_func(u + h, v), z_func(u + h, v));
        glm::vec3 p_v(x_func(u, v + h), y_func(u, v + h), z_func(u, v + h));
        
        glm::vec3 du = p_u - p_center;
        glm::vec3 dv = p_v - p_center;
        
        // Cross product gives the normal vector
        glm::vec3 normal = glm::cross(du, dv);
        return glm::normalize(normal);
    };

    // 5. Generate the vertex positions, normals, and texture coordinates
    for (int j = 0; j <= v_segments; ++j) {
        for (int i = 0; i <= u_segments; ++i) {
            // Normalize u and v to the range [0, 1]
            float u_norm = static_cast<float>(i) / u_segments;
            float v_norm = static_cast<float>(j) / v_segments;

            // Calculate vertex position
            vertices.push_back(x_func(u_norm, v_norm));
            vertices.push_back(y_func(u_norm, v_norm));
            vertices.push_back(z_func(u_norm, v_norm));

            // Calculate and add normal vector
            glm::vec3 normal = calculateNormal(u_norm, v_norm);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Add texture coordinates directly from the normalized u and v
            vertices.push_back(u_norm);
            vertices.push_back(v_norm);
        }
    }

    // 6. Generate the indices for the faces (as triangles)
    for (int j = 0; j < v_segments; ++j) {
        for (int i = 0; i < u_segments; ++i) {
            // Calculate the indices for the four corners of the current quad
            unsigned int topLeft = j * (u_segments + 1) + i;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (j + 1) * (u_segments + 1) + i;
            unsigned int bottomRight = bottomLeft + 1;

            // Create two triangles for the quad
            // Triangle 1: top-left, bottom-left, top-right
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2: top-right, bottom-left, bottom-right
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

// constructing the vertices and indice for VBO and EBO
void generateKnotTorus(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    // 1. Define resolution parameters
    const int u_segments = 100;
    const int v_segments = 64;

    // 2. Clear out any old data from the vectors
    vertices.clear();
    indices.clear();

    // 3. Reserve memory to avoid reallocations (good for performance)
    vertices.reserve((u_segments + 1) * (v_segments + 1) * 8); // XYZ + NXYZ + UV
    indices.reserve(u_segments * v_segments * 6);

    // 4. Define the parametric equations as C++ lambdas, broken down for clarity
    auto x_func = [&](float u, float v) {
        float term_u1 = (0.1f * cos(10.0f * PI * u)) + 0.25f;
        float term_u2 = 0.1f * sin(10.0f * PI * u);
        float term_u3 = cos(2.0f * PI * u);
        float term_u4 = sin(2.0f * PI * u);
        float term_v1 = cos(2.0f * PI * v);
        float term_v2 = sin(2.0f * PI * v);

        float inner_x = term_u1 * term_u3 - term_u2 * term_u4;
        float inner_y = term_u1 * term_u4 + term_u2 * term_u3;

        return (inner_x * term_v1 - inner_y * term_v2 + 1.0f) * (-term_v2);
    };

    auto y_func = [&](float u, float v) {
        float term_u1 = (0.1f * cos(10.0f * PI * u)) + 0.25f;
        float term_u2 = 0.1f * sin(10.0f * PI * u);
        float term_u3 = cos(2.0f * PI * u);
        float term_u4 = sin(2.0f * PI * u);
        float term_v1 = cos(2.0f * PI * v);
        float term_v2 = sin(2.0f * PI * v);

        float inner_x = term_u1 * term_u3 - term_u2 * term_u4;
        float inner_y = term_u1 * term_u4 + term_u2 * term_u3;

        return (inner_x * term_v1 - inner_y * term_v2 + 1.0f) * term_v1;
        };

    auto z_func = [&](float u, float v) {
        float term_u1 = (0.1f * cos(10.0f * PI * u)) + 0.25f;
        float term_u2 = 0.1f * sin(10.0f * PI * u);
        float term_u3 = cos(2.0f * PI * u);
        float term_u4 = sin(2.0f * PI * u);
        float term_v1 = cos(2.0f * PI * v);
        float term_v2 = sin(2.0f * PI * v);

        float inner_x = term_u1 * term_u3 - term_u2 * term_u4;
        float inner_y = term_u1 * term_u4 + term_u2 * term_u3;

        return inner_x * term_v2 + inner_y * term_v1;
        };

    // Helper function to calculate normal vector using partial derivatives
    auto calculateNormal = [&](float u, float v) -> glm::vec3 {
        float h = 0.001f; // Small step for numerical differentiation
        
        // Calculate partial derivatives numerically
        glm::vec3 p_center(x_func(u, v), y_func(u, v), z_func(u, v));
        glm::vec3 p_u(x_func(u + h, v), y_func(u + h, v), z_func(u + h, v));
        glm::vec3 p_v(x_func(u, v + h), y_func(u, v + h), z_func(u, v + h));
        
        glm::vec3 du = p_u - p_center;
        glm::vec3 dv = p_v - p_center;
        
        // Cross product gives the normal vector
        glm::vec3 normal = glm::cross(du, dv);
        return glm::normalize(normal);
    };

    // 5. Generate the vertex positions, normals, and texture coordinates
    for (int j = 0; j <= v_segments; ++j) {
        for (int i = 0; i <= u_segments; ++i) {
            float u_norm = static_cast<float>(i) / u_segments;
            float v_norm = static_cast<float>(j) / v_segments;

            // Add the x, y, z components
            vertices.push_back(x_func(u_norm, v_norm));
            vertices.push_back(y_func(u_norm, v_norm));
            vertices.push_back(z_func(u_norm, v_norm));

            // Calculate and add normal vector
            glm::vec3 normal = calculateNormal(u_norm, v_norm);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Add the texture coordinates (U, V)
            vertices.push_back(u_norm);
            vertices.push_back(v_norm);
        }
    }

    // 6. Generate the indices for the faces (as triangles)
    for (int j = 0; j < v_segments; ++j) {
        for (int i = 0; i < u_segments; ++i) {
            unsigned int topLeft = j * (u_segments + 1) + i;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (j + 1) * (u_segments + 1) + i;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

void generateAnimatedTail(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    float time,
    const float radius = 0.2f,
    unsigned int cross_section_resolution = 16,
    unsigned int circle_per_period = 8,
    float n_period = 2.0f,
    float shift_amplitude = 0.2f,
    float height = 1.5f
) {
    vertices.clear();
    //indices.clear();

    // 3. Reserve memory to avoid reallocations (good for performance)
    vertices.reserve(
        (circle_per_period * n_period + 1) * cross_section_resolution * 8 // circle vertices for the tail body: XYZ + NXYZ + UV
        // + cross_section_resolution * (cross_section_resolution / 4 - 2) + 1 // 1 for tip, resolution/4 - 2(tip and base) layer for arch
    );
    //indices.reserve(u_segments * v_segments * 6);

    auto x_offset = [&](float angle, float time) {
        float omega = -2 * PI;
        return shift_amplitude * sin(angle + omega * time);
        };

    auto z_position = [&](float r, float angle) {
        return r * sin(angle);
        };
    auto x_position = [&](float r, float angle) {
        return r * cos(angle);
        };

    float circle_y_offset = height / (circle_per_period * n_period); // the offset between each circle along the height 
    
    // Helper function to calculate normal for cylindrical geometry
    auto calculateCylinderNormal = [&](float angle, float oscillate_angle, float time) -> glm::vec3 {
        // For a cylinder, the normal points outward from the center axis
        // The normal is perpendicular to the cylinder surface
        float x_offset_val = x_offset(oscillate_angle, time);
        float x_pos = x_position(radius, angle) + x_offset_val;
        float z_pos = z_position(radius, angle);
        
        // Normalize the vector from center to surface point
        glm::vec3 normal = glm::normalize(glm::vec3(x_pos - x_offset_val, 0.0f, z_pos));
        return normal;
    };
    
    // iterate through each circle along the height
    for (unsigned int j = 0; j <= circle_per_period * n_period; ++j) {
        //for each cross section
        for (unsigned int i = 0; i < cross_section_resolution; ++i) {
            float u_norm = static_cast<float>(i) / cross_section_resolution;
            float v_norm = static_cast<float>(j) / (circle_per_period * n_period);
            
            // Calculate position
            float oscillate_angle = 2 * PI * j / circle_per_period; // oscillation along the height
            float angle = 2 * PI * i / cross_section_resolution;
            float x_pos = x_position(radius, angle) + x_offset(oscillate_angle, time);
            float y_pos = -circle_y_offset * j; // going down along the y axis
            float z_pos = z_position(radius, angle);
            
            // Add the x, y, z components
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            vertices.push_back(z_pos);

            // Calculate and add normal vector
            glm::vec3 normal = calculateCylinderNormal(angle, oscillate_angle, time);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Add the texture coordinates (U, V)
            vertices.push_back(u_norm);
            vertices.push_back(v_norm);
        }
    }

    // --- Index Generation (The crucial fix) ---
    for (unsigned int j = 0; j < circle_per_period * n_period; ++j) {
        for (unsigned int i = 0; i < cross_section_resolution; ++i) {
            unsigned int topLeft = j * cross_section_resolution + i;
            unsigned int topRight = topLeft + 1;
            // Wrap around for the last vertex in the circle
            if (i == cross_section_resolution - 1) {
                topRight = j * cross_section_resolution;
            }

            unsigned int bottomLeft = (j + 1) * cross_section_resolution + i;
            unsigned int bottomRight = bottomLeft + 1;
            // Wrap around for the last vertex in the circle
            if (i == cross_section_resolution - 1) {
                bottomRight = (j + 1) * cross_section_resolution;
            }

            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}

float vertices[] = {// this is for a cube which can be transform to make it scattered simply template
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
};
// positions all containers
glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};
// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

std::vector<VertexAttribute> texturedParametricLayout = {
    {0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0},                   // location 0: position
    {2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))}  // location 2: texCoord
};

std::vector<VertexAttribute> texturedParametricWithNormalsLayout = {
    {0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0},                   // location 0: position
    {1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))}, // location 1: normals
    {2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))}   // location 2: texCoord
};

std::vector<VertexAttribute> texture_n_ebo_Layout = {
    {0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0},                   // location 0: position
    {1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))},  // location 2: texCoord
    {2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))}
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "animated 3D jelly fish ", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("6.multiple_lights.vs", "6.multiple_lights.fs");
    Shader lightCubeShader("6.light_cube.vs", "6.light_cube.fs");

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO, VBO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // load textures (we now use a utility function to keep the code more organized)
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTexture(FileSystem::getPath("resources/textures/container2.png").c_str());
    unsigned int specularMap = loadTexture(FileSystem::getPath("resources/textures/container2_specular.png").c_str());
	unsigned int groundTexture = loadTexture(FileSystem::getPath("resources/textures/sand.jpg").c_str());
	unsigned int jellyfishMat = loadTexture(FileSystem::getPath("resources/textures/reflected_water.jpg").c_str());

    std::vector<float> JH_vertices;
    std::vector<unsigned int> JH_indices;
    generateJellyfishHead(JH_vertices, JH_indices);
    Mesh jellyfishHeadMesh(JH_vertices, JH_indices, texturedParametricWithNormalsLayout);

    std::vector<float> JC_vertices;
    std::vector<unsigned int> JC_indices;
    generateKnotTorus(JC_vertices, JC_indices);
    Mesh jellyfishCapMesh(JC_vertices, JC_indices, texturedParametricWithNormalsLayout);

    std::vector<float> tail_vertices;
    std::vector<unsigned int> tail_indices;
    generateAnimatedTail(tail_vertices, tail_indices, 0);
    Mesh animatedTailMesh(tail_vertices, tail_indices, texturedParametricWithNormalsLayout);

    Jellyfish Jellyfish_facade(&jellyfishHeadMesh, &jellyfishCapMesh, &animatedTailMesh);
    /// sizeof(float)
	Mesh ocean_ground_mesh(ground_vertices, sizeof(ground_vertices), texture_n_ebo_Layout);
    // shader configuration
    // --------------------
    lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);

        /*
           Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
           the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
           by defining light types as classes and set their values in there, or by using a more efficient uniform approach
           by using 'Uniform buffer objects', but that is something we'll discuss in the 'Advanced GLSL' tutorial.
        */
        // directional light
        lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        
        // This is the correct way to get the number of elements in a C-style array
        int numPointLights = sizeof(pointLightPositions) / sizeof(glm::vec3);

        for (int i = 0; i < numPointLights; i++) {
            // Create the base string for the current light, e.g., "pointLights[0]."
            std::string uniformName = "pointLights[" + std::to_string(i) + "]";

            // Set the uniforms using the constructed name
            lightingShader.setVec3(uniformName + ".position", pointLightPositions[i]);
            lightingShader.setVec3(uniformName + ".ambient", 0.05f, 0.05f, 0.05f);
            lightingShader.setVec3(uniformName + ".diffuse", 0.8f, 0.8f, 0.8f);
            lightingShader.setVec3(uniformName + ".specular", 1.0f, 1.0f, 1.0f);
            lightingShader.setFloat(uniformName + ".constant", 1.0f);
            lightingShader.setFloat(uniformName + ".linear", 0.09f);
            lightingShader.setFloat(uniformName + ".quadratic", 0.032f);
        }
        
        // spotLight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);
        lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLight.constant", 1.0f);
        lightingShader.setFloat("spotLight.linear", 0.09f);
        lightingShader.setFloat("spotLight.quadratic", 0.032f);
        lightingShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));     

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, jellyfishMat);
        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, jellyfishMat);

        //glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // render containers
        //glBindVertexArray(cubeVAO);

		unsigned int n_jellyfish = sizeof(cubePositions) / sizeof(glm::vec3);
        for (unsigned int i = 0; i < n_jellyfish; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

			Jellyfish_facade.Draw(lightingShader, model);
            generateAnimatedTail(tail_vertices, tail_indices, lastFrame);
			animatedTailMesh.updateVertices(tail_vertices);
			Jellyfish_facade.Animate(tail_vertices, lastFrame);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    
         // also draw the lamp object(s)
         lightCubeShader.use();
         lightCubeShader.setMat4("projection", projection);
         lightCubeShader.setMat4("view", view);
    
         // we now draw as many light bulbs as we have point lights.
         glBindVertexArray(lightCubeVAO);
         for (unsigned int i = 0; i < 4; i++)
         {
             model = glm::mat4(1.0f);
             model = glm::translate(model, pointLightPositions[i]);
             model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
             lightCubeShader.setMat4("model", model);
             glDrawArrays(GL_TRIANGLES, 0, 36);
         }

         lightingShader.use();

         // bind diffuse map
         glActiveTexture(GL_TEXTURE0);
         glBindTexture(GL_TEXTURE_2D, groundTexture);
         // bind specular map
         glActiveTexture(GL_TEXTURE1);
         glBindTexture(GL_TEXTURE_2D, groundTexture);

         //glBindBuffer(GL_ARRAY_BUFFER, VBO);
         //glBufferData(GL_ARRAY_BUFFER, sizeof(ground_vertices), ground_vertices, GL_STATIC_DRAW);
         // we now draw as many light bulbs as we have point lights.
         model = glm::mat4(1.0f);
         model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
         model = glm::scale(model, glm::vec3(50.0f)); // Make it a smaller cube
         model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.6f));
         lightingShader.setMat4("model", model);
		 ocean_ground_mesh.Draw();


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
