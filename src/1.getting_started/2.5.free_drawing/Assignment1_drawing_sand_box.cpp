#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <array>    // Use std::array for fixed-size arrays
//#include <limits>   // For std::numeric_limits

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
float Triangle_centroid(const std::array<float, 9>& triangle, char axis); // Updated signature
float Euclidian_distance(float x1, float y1, float x2, float y2);
int random_int(int min, int max);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
const char *fragmentShader1Source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";
const char *fragmentShader2Source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
    "}\n\0";

const char* fragmentShaderSource_Purple = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.937f, 0.0f, 1.0f, 1.0f);\n"
"}\n\0";

const char* fragmentShaderSource_Blue = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.0f, 0.06f, 1.0f, 1.0f);\n"
"}\n\0";

const char* fragmentShaderSource_LightBlue = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.498f, 0.8f, 1.0f, 1.0f);\n"
"}\n\0";

const char* fragmentShaderSource_Pink = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.572f, 0.96f, 1.0f);\n"
"}\n\0";

//struct Triangle {
//    float *vertices;
//	Triangle* next;
//};

//float firstTriangle[] = {
//        -0.5f, -0.5f, 0.0f,  // left 
//        -0.0f, -0.5f, 0.0f,  // right
//        -0.5f, 0.0f, 0.0f,  // top 
//};
//float secondTriangle[] = {
//    0.0f, -0.5f, 0.0f,  // left
//    0.9f, -0.5f, 0.0f,  // right
//    0.45f, 0.5f, 0.0f   // top 
//};

int selected_triangle_index = -1;
bool aKeyPressedLastFrame = false;

std::vector<std::array<float, 9>> allTriangles = {
        {{-0.5f, -0.5f, 0.0f,  -0.0f, -0.5f, 0.0f,  -0.5f, 0.0f, 0.0f}}, // First Triangle
        {{ 0.0f, -0.5f, 0.0f,   0.9f, -0.5f, 0.0f,   0.45f, 0.5f, 0.0f}}, // Second Triangle
        {{-0.9f, 0.5f, 0.0f,  -0.1f, 0.5f, 0.0f,  -0.5f, -0.5f, 0.0f}}   // A third triangle
};

std::vector<unsigned int> fragmentShaderlist;
std::vector<int> fragmentShadermap;


//TODO: read more about this
unsigned int VBO, VAO;

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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    // build and compile our shader program
    // ------------------------------------
    // we skipped compile log checks this time for readability (if you do encounter issues, add the compile-checks! see previous code samples)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER); // the first fragment shader that outputs the color orange
    unsigned int fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER); // the second fragment shader that outputs the color yellow
    unsigned int fragmentShaderPurple = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int fragmentShaderLightBlue = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int fragmentShaderBlue = glCreateShader(GL_FRAGMENT_SHADER);
    unsigned int fragmentShaderPink = glCreateShader(GL_FRAGMENT_SHADER);
    
    unsigned int shaderProgramOrange = glCreateProgram();
    unsigned int shaderProgramYellow = glCreateProgram(); // the second shader program
    unsigned int shaderProgramPurple = glCreateProgram();
    unsigned int shaderProgramLightBlue = glCreateProgram();
    unsigned int shaderProgramBlue = glCreateProgram();
    unsigned int shaderProgramPink = glCreateProgram();

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    glShaderSource(fragmentShaderOrange, 1, &fragmentShader1Source, NULL);
    glCompileShader(fragmentShaderOrange);
    glShaderSource(fragmentShaderYellow, 1, &fragmentShader2Source, NULL);
    glCompileShader(fragmentShaderYellow);
    glShaderSource(fragmentShaderPurple, 1, &fragmentShaderSource_Purple, NULL);
    glCompileShader(fragmentShaderPurple);
    glShaderSource(fragmentShaderLightBlue, 1, &fragmentShaderSource_LightBlue, NULL);
    glCompileShader(fragmentShaderLightBlue);
    glShaderSource(fragmentShaderBlue, 1, &fragmentShaderSource_Blue, NULL);
    glCompileShader(fragmentShaderBlue);
    glShaderSource(fragmentShaderPink, 1, &fragmentShaderSource_Pink, NULL);
    glCompileShader(fragmentShaderPink);

    // link the first program object
    glAttachShader(shaderProgramOrange, vertexShader);
    glAttachShader(shaderProgramOrange, fragmentShaderOrange);
    glLinkProgram(shaderProgramOrange);
    // then link the second program object using a different fragment shader (but same vertex shader)
    // this is perfectly allowed since the inputs and outputs of both the vertex and fragment shaders are equally matched.
    glAttachShader(shaderProgramYellow, vertexShader);
    glAttachShader(shaderProgramYellow, fragmentShaderYellow);
    glLinkProgram(shaderProgramYellow);

    glAttachShader(shaderProgramPurple, vertexShader);
    glAttachShader(shaderProgramPurple, fragmentShaderPurple);
    glLinkProgram(shaderProgramPurple);

    glAttachShader(shaderProgramLightBlue, vertexShader);
    glAttachShader(shaderProgramLightBlue, fragmentShaderLightBlue);
    glLinkProgram(shaderProgramLightBlue);

    glAttachShader(shaderProgramBlue, vertexShader);
    glAttachShader(shaderProgramBlue, fragmentShaderBlue);
    glLinkProgram(shaderProgramBlue);

    glAttachShader(shaderProgramPink, vertexShader);
    glAttachShader(shaderProgramPink, fragmentShaderPink);
    glLinkProgram(shaderProgramPink);

    fragmentShaderlist = {
        shaderProgramYellow,
        shaderProgramPink,
        shaderProgramPurple,
        shaderProgramLightBlue,
        shaderProgramBlue
    };

    fragmentShadermap = {
        0,1,2
    };

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    glGenVertexArrays(1, &VAO); // we can also generate multiple VAOs or buffers at the same time
    glGenBuffers(1, &VBO);

    // glBindVertexArray(0); // not really necessary as well, but beware of calls that could affect VAOs while this one is bound (like binding element buffer objects, or enabling/disabling vertex attributes)
    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // Allocate buffer memory. We'll upload data each frame, so usage is GL_DYNAMIC_DRAW.
        // We can initially allocate a large size to avoid re-allocating.
        glBufferData(GL_ARRAY_BUFFER, allTriangles.size() * sizeof(std::array<float, 9>), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // Bind the single VAO for all drawing
        glBindVertexArray(VAO);

        // Before drawing, upload the potentially modified CPU data to the GPU's VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, allTriangles.size() * sizeof(std::array<float, 9>), nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, allTriangles.size() * sizeof(std::array<float, 9>), allTriangles.data());
        // Draw all triangles
        for (int i = 0; i < allTriangles.size(); ++i)
        {
            // Use the yellow shader for the selected triangle, orange for others
            if (i == selected_triangle_index) {
                glUseProgram(shaderProgramOrange);
            }
            else {
				// TODO: use specific shader program for each vertices
                glUseProgram(
					fragmentShaderlist[fragmentShadermap[i]]
                );
            }

            // Draw one triangle (3 vertices), starting at the correct offset
            glDrawArrays(GL_TRIANGLES, i * 3, 3);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    /*glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteProgram(shaderProgramOrange);
    glDeleteProgram(shaderProgramYellow);*/
    
    // --- Cleanup ---
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgramOrange);
    glDeleteProgram(shaderProgramYellow);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

float Euclidian_distance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Updated to use std::array for type safety
float Triangle_centroid(const std::array<float, 9>& triangle, char axis) {
    if (axis == 'y')
        return (triangle[1] + triangle[4] + triangle[7]) / 3.0f;
    else if (axis == 'z')
        return (triangle[2] + triangle[5] + triangle[8]) / 3.0f;
    else
        return (triangle[0] + triangle[3] + triangle[6]) / 3.0f;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    // Convert window coordinates to normalized device coordinates (NDC)
    float ndcX = static_cast<float>((x / SCR_WIDTH) * 2.0 - 1.0);
    float ndcY = static_cast<float>(1.0 - (y / SCR_HEIGHT) * 2.0);
    bool aIsPressedNow = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    // Replace the incorrect lines in processInput with the following:
        glfwSetWindowShouldClose(window, true);
	// to selct VAO with mouse coordinates
    else if (aIsPressedNow && !aKeyPressedLastFrame) {
        std::array<float, 9> newTriangle =
        { {-0.5f, -0.5f, 0.0f, -0.0f, -0.5f, 0.0f,  -0.5f,  0.0f, 0.0f}};
		allTriangles.push_back(newTriangle);
        int newShader = random_int(0,fragmentShaderlist.size()-1);
		fragmentShadermap.push_back(newShader);
		// TODO: randomly select the shader program for the new triangle and add to vector
		std::cout << "Added new triangle. Total triangles: " << allTriangles.size() << std::endl;
	}
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        float min_dist = 9999;
        int closest_index = -1;
		// calculate triangle centroids; given 3 vertice with xyz coordinates

        for (int i = 0; i < allTriangles.size(); ++i)
        {
            float centroidX = Triangle_centroid(allTriangles[i], 'x');
            float centroidY = Triangle_centroid(allTriangles[i], 'y');
            float dist = Euclidian_distance(ndcX, ndcY, centroidX, centroidY);

            if (dist < min_dist) {
                min_dist = dist;
                closest_index = i;
            }
        }
        selected_triangle_index = closest_index;
		// calculate distance from mouse to triangle centroids
		// update selected_triangle pointer
	}
	// to change trinagle shape with w key
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		// loop check in 2D coordinates in firtst triangle
        float max_dist = 99999 ;
		int max_point = -1;
        for (int i = 0; i < 3; i++) {
			float dist = Euclidian_distance(ndcX, ndcY, allTriangles[selected_triangle_index][3 * i], allTriangles[selected_triangle_index][3 * i + 1]);
            if (dist < max_dist) {
				max_dist = dist;
				max_point = i;
            }
        }

        // Move the first vertex of the first triangle to the mouse position
        allTriangles[selected_triangle_index][3* max_point] = ndcX; // x
        allTriangles[selected_triangle_index][3* max_point + 1] = ndcY; // y
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (selected_triangle_index != -1) {
            allTriangles.erase(allTriangles.begin() + selected_triangle_index);
			fragmentShadermap.erase(fragmentShadermap.begin() + selected_triangle_index);
            selected_triangle_index = -1; // Deselect after deletion
            std::cout << "Deleted selected triangle. Total triangles: " << allTriangles.size() << std::endl;
        }
	}
    aKeyPressedLastFrame = aIsPressedNow;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

int random_int(int min, int max){
    return min + (rand() % (max - min + 1));
}