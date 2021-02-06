#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType mode = ShaderType::NONE;
    // Go through file line-by-line
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            // We find a shader section
            if (line.find("vertex") != std::string::npos)
            {
                // set the mode to 'vertex'
                mode = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {

                // set the mode to 'fragment'
                mode = ShaderType::FRAGMENT;
            }
        }
        else if(mode != ShaderType::NONE)
        {
            // Add the shader line to it's respective string string
            ss[(int)mode] << line << "\n";
        }
    }

    return ShaderProgramSource{ ss[0].str(), ss[1].str() };
}

static unsigned int ComplileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    // OpenGL expects a raw C-style string
    const char* src = source.c_str();
    // Pass the shader source code to OpenGL... the source string is compiled as a result of this call so it's safe to free source.
    glShaderSource(id, 1, &src, nullptr);
    // Compile the shader
    glCompileShader(id);
    // Check the shader compilation result
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* log = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, log);
        std::cout << "Failed to compile " 
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
            << std::endl;
        std::cout << log << std::endl;
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = ComplileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = ComplileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 640, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error!" << std::endl;
    }

    float positions[12] = 
    {
        -0.5f, -0.5f, // 0
        0.5f, -0.5f, // 1
        0.5f, 0.5f, // 2
        -0.5f, 0.5f // 3
    };

    unsigned int indices[6] = 
    {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffer;
    // Create a new buffer object and store a pointer to it in 'buffer'
    glGenBuffers(1, &buffer);
    // Bind the buffer to the the GL_ARRAY_BUFFER target
    // This target means the data written to 'buffer' will be used vertex data
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // Creates an initializes the data store for 'buffer'/GL_ARRAY_BUFFER with the triangle vertices in positions
    glBufferData(GL_ARRAY_BUFFER, (4 * 2) * sizeof(float), positions, GL_STATIC_DRAW);
    // Enables the generic attribute array at index '0', this will use will use the currently bound vertex array object above for operation
    glEnableVertexAttribArray(0);
    // This call will tell OpenGL about the location and format of the vertex data i.e at index 0 and consists of two floats that are two bytes wide each.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    
    // Create an element buffer object on the GPU
    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);
    
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;
    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the currently bound element buffer
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}