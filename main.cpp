/* 
 * ========================================
 * Includes
 * ========================================
 */
// GLEW
#include <GL/glew.h>
// SDL
#include <SDL2/SDL.h>
// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// STL
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

/* 
 * ========================================
 * Class Declarations
 * ========================================
 */
class Shader;

/* 
 * ========================================
 * Shader Class
 * ========================================
 */
class Shader
{
public: 
  unsigned int id;

  Shader(const char* vertexPath, const char* fragmentPath)
  {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
      vShaderFile.open(vertexPath);
      fShaderFile.open(fragmentPath);
      std::stringstream vShaderStream, fShaderStream;

      vShaderStream << vShaderFile.rdbuf();
      fShaderStream << fShaderFile.rdbuf();

      vShaderFile.close();
      fShaderFile.close();

      vertexCode = vShaderStream.str();
      fragmentCode = fShaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    /*
     * Compile and link shaders
     */
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(vertex, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);

    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(fragment, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders
    id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);

    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
      glGetProgramInfoLog(id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
  }

  void use()
  {
    glUseProgram(id);
  }

  void setVec3(const std::string& name, float x, float y, float z)
  {
    glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
  }
};
/* 
 * ========================================
 * Constants
 * ========================================
 */
struct
{
  struct
  {
    const int WIDTH = 480;
    const int HEIGHT = 360;
    const char* TITLE = "Astro Astro";
  } WINDOW;
  struct
  {
    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;
  } GAME;
} CONSTANTS;

/* 
 * ========================================
 * Globals
 * ========================================
 */
struct
{
  struct
  {
    bool running = true;
    SDL_Window* window;
  } GAME;
  struct
  {
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
  } INPUT;
  struct
  {
    Shader* shader;
    struct
    {
      Shader* shader;
      float x = 0;
      float y = 5;
      float z = -20;
    } LIGHT;
    unsigned int VAO;
    unsigned int lightVAO;
  } GLOBJECTS;
  struct
  {
    float PI = 3.14159;
  } MATH;
} GLOBALS;

/* 
 * ========================================
 * Function Declarations
 * ========================================
 */
static void input();
static void update();
static void draw();

static void findNormals(std::vector<float>&);

/* 
 * ========================================
 * Player stuff
 * ========================================
 */
struct
{
  // Stats
  float x = 0;
  float y = 0;
  float tiltX = 0;
  float tiltY = 0;
  int health = 100;
  // The model
  // First three are positions, next three are colors, the last three are normals
  std::vector<float> p = {
    -1.000f,  0.000f,  0.000f, 0, 0, 0,
     1.000f,  0.000f,  0.000f, 0, 0, 0,
     0.000f, -0.500f, -0.250f, 0, 0, 0,
    -0.750f,  1.000f,  0.000f, 0, 0, 0,
     0.000f,  1.000f,  0.000f, 0, 0, 0,
     0.750f,  1.000f,  0.000f, 0, 0, 0,
     0.000f,  0.600f,  2.000f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 0, 0, 0,
    -2.000f,  0.000f,  0.000f, 0, 0, 0,
    -1.750f,  0.750f, -1.000f, 0, 0, 0,
    -1.500f,  4.000f,  2.000f, 0, 0, 0,
    -1.500f, -1.000f,  0.000f, 0, 0, 0,
    -4.000f, -1.250f,  5.000f, 0, 0, 0,
     2.000f,  0.000f,  0.000f, 0, 0, 0,
     1.750f,  0.750f, -1.000f, 0, 0, 0,
     1.500f,  4.000f,  2.000f, 0, 0, 0,
     1.500f, -1.000f,  0.000f, 0, 0, 0,
     4.000f, -1.250f,  5.000f, 0, 0, 0
  };
  std::vector<float> points = {
    -1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f, -0.500f, -0.250f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    -0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.600f,  2.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    
    -0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.600f,  2.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    
     0.000f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.600f,  2.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    
     0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.600f,  2.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    
    -1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
    -0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

    -0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

     0.750f,  1.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f, -0.500f, -0.250f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f, -0.500f, -0.250f, 1.00f, 1.00f, 1.00f, 0, 0, 0,
     0.000f,  0.250f, -6.000f, 1.00f, 1.00f, 1.00f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

    -1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
    -1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f,  4.000f,  2.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.750f,  0.750f, -1.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     4.000f, -1.250f,  5.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,

     1.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     2.000f,  0.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
     1.500f, -1.000f,  0.000f, 0.00f, 0.55f, 0.96f, 0, 0, 0,
  };
  // Model indices
  std::vector<unsigned int> indices = {
   0, 1, 2,
   0, 3, 6,
   3, 4, 6,
   4, 5, 6,
   5, 1, 6,
   0, 3, 7,
   3, 5, 7,
   5, 1, 7,
   0, 2, 7,
   1, 2, 7,
   0, 8, 10,
   8, 9, 10,
   9, 0, 10,
   0, 8, 9,
   0, 11, 12,
   0, 8, 12,
   8, 11, 12,
    0, 8, 11,
    1, 13, 15,
    13, 14, 15,
    14, 1, 15,
    1, 13, 14,
    1, 16, 17,
    1, 13, 17,
    13, 16, 17,
    1, 13, 16
  };
} player;

// Light source
struct
{
  std::vector<float> points {
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f
  };
  std::vector<unsigned int> indices {
    0, 1, 2,
    2, 1, 3,

    4, 0, 6,
    6, 0, 2,

    5, 1, 7,
    7, 1, 3,

    4, 5, 6,
    6, 5, 7,

    6, 7, 2,
    2, 7, 3,

    4, 5, 0,
    0, 5, 1
  };
} light;


/* 
 * ========================================
 * Main Function
 * ========================================
 */
int main(int argc, char* args[])
{
  /* 
   * ========================================
   * Initialize SDL and OpenGL
   * ========================================
   */
  // Init SDL
  if (SDL_Init(SDL_INIT_EVERYTHING != 0))
  {
    std::cout << "ERROR::SDL::INITIALIZATION_FAILED" << std::endl;
    return -1;
  }

  // Create the window, which is also the OpenGL context
  GLOBALS.GAME.window = SDL_CreateWindow(CONSTANTS.WINDOW.TITLE, 0, 0, CONSTANTS.WINDOW.WIDTH, CONSTANTS.WINDOW.HEIGHT, SDL_WINDOW_OPENGL);
  SDL_GLContext glContext = SDL_GL_CreateContext(GLOBALS.GAME.window);

  // Initialize GLEW
  if (glewInit() != GLEW_OK)
  {
    std::cout << "ERROR::GLEW::INITIALIZATION_FAILED" << std::endl;
    return -1;
  }

  // Enable/Set up some OpenGL stuff
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe mode
  glEnable(GL_DEPTH_TEST);
  GLOBALS.GLOBJECTS.shader = new Shader("res/shaders/vertex.glsl", "res/shaders/fragment.glsl");
  GLOBALS.GLOBJECTS.LIGHT.shader = new Shader("res/shaders/light_vertex.glsl", "res/shaders/light_fragment.glsl");
  
  /* 
   * ========================================
   * Load objects onto GPU
   * ========================================
   */
  // Player
  findNormals(player.points);
  unsigned int VBO;
  glGenVertexArrays(1, &GLOBALS.GLOBJECTS.VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(GLOBALS.GLOBJECTS.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * player.points.size(), player.points.data(), GL_STATIC_DRAW);

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  // Color
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Normal
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // Light
  unsigned int lightVBO, lightEBO;
  glGenVertexArrays(1, &GLOBALS.GLOBJECTS.lightVAO);
  glGenBuffers(1, &lightVBO);
  glGenBuffers(1, &lightEBO);

  glBindVertexArray(GLOBALS.GLOBJECTS.lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * light.points.size(), light.points.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * light.indices.size(), light.indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glEnableVertexAttribArray(0);

  GLOBALS.GLOBJECTS.shader->use();
  GLOBALS.GLOBJECTS.shader->setVec3("lightColor",  1.0f, 1.0f, 1.0f);
  GLOBALS.GLOBJECTS.shader->setVec3("lightPos", GLOBALS.GLOBJECTS.LIGHT.x, GLOBALS.GLOBJECTS.LIGHT.y, GLOBALS.GLOBJECTS.LIGHT.z);

  /* 
   * ========================================
   * Game Loop
   * ========================================
   */
  Uint32 frameStart;
  int frameTime;

  while (GLOBALS.GAME.running)
  {
    frameStart = SDL_GetTicks();

    input();
    update();
    draw();

    frameTime = SDL_GetTicks() - frameStart;

    while (CONSTANTS.GAME.FRAME_DELAY > frameTime)
    {
      input();
      frameTime = SDL_GetTicks() - frameStart;
    }
  }

  /* 
   * ========================================
   * Free up memory
   * ========================================
   */
  delete GLOBALS.GLOBJECTS.LIGHT.shader;
  delete GLOBALS.GLOBJECTS.shader;
  SDL_DestroyWindow(GLOBALS.GAME.window);
  SDL_Quit();

  return 0;
}

/* 
 * ========================================
 * Game Loop Function Implementations
 * ========================================
 */
void input()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    switch (e.type)
    {
      case SDL_QUIT:
      {
        GLOBALS.GAME.running = false;
        break;
      }
      case SDL_KEYDOWN:
      {
        switch (e.key.keysym.sym)
        {
          case SDLK_a:
          {
            GLOBALS.INPUT.left = true;
            break;
          }
          case SDLK_d:
          {
            GLOBALS.INPUT.right = true;
            break;
          }
          case SDLK_w:
          {
            GLOBALS.INPUT.up = true;
            break;
          }
          case SDLK_s:
          {
            GLOBALS.INPUT.down = true;
            break;
          }
        }
        break;
      }
      case SDL_KEYUP:
      {
        switch (e.key.keysym.sym)
        {
          case SDLK_a:
          {
            GLOBALS.INPUT.left = false;
            break;
          }
          case SDLK_d:
          {
            GLOBALS.INPUT.right = false;
            break;
          }
          case SDLK_w:
          {
            GLOBALS.INPUT.up = false;
            break;
          }
          case SDLK_s:
          {
            GLOBALS.INPUT.down = false;
            break;
          }
        }
        break;
      }
    }
  }
}

void update()
{
  if (GLOBALS.INPUT.left || GLOBALS.INPUT.right)
  {
    if (GLOBALS.INPUT.left)
    {
      if (player.x > -5)
      {
        player.x -= 0.1;
      }
      if (player.tiltX < GLOBALS.MATH.PI / 4)
      {
        if (player.tiltX < 0)
          player.tiltX += 0.05;
        else
          player.tiltX += 0.01;
      }
    }
    if (GLOBALS.INPUT.right)
    {
      if (player.x < 5)
      {
        player.x += 0.1;
      }
      if (player.tiltX > -GLOBALS.MATH.PI / 4)
      {
        if (player.tiltX > 0)
          player.tiltX -= 0.05;
        else
          player.tiltX -= 0.01;
      }
    }
  }
  else
  {
    if (player.tiltX > 0.1)
      player.tiltX -= 0.05;
    else if (player.tiltX < -0.1)
      player.tiltX += 0.05;
    else
      player.tiltX = 0;
  }

  if (GLOBALS.INPUT.up || GLOBALS.INPUT.down)
  {
    if (GLOBALS.INPUT.up)
    {
      if (player.tiltY < GLOBALS.MATH.PI / 4)
      {
        if (player.tiltY < 0)
          player.tiltY += 0.05;
        else
          player.tiltY += 0.01;
      }
    }
    if (GLOBALS.INPUT.down)
    {
      if (player.tiltY > -GLOBALS.MATH.PI / 4)
      {
        if (player.tiltY > 0)
          player.tiltY -= 0.05;
        else
          player.tiltY -= 0.01;
      }
    }
  }
  else
  {
    if (player.tiltY > 0.1)
      player.tiltY -= 0.05;
    else if (player.tiltY < -0.1)
      player.tiltY += 0.05;
    else
      player.tiltY = 0;
  }

  // Test: Move light
  //GLOBALS.GLOBJECTS.LIGHT.x = std::sin(4 * SDL_GetTicks());
  GLOBALS.GLOBJECTS.LIGHT.z = 20 * std::sin(.005 * SDL_GetTicks()) - 20;
  GLOBALS.GLOBJECTS.shader->use();
  GLOBALS.GLOBJECTS.shader->setVec3("lightColor",  1.0f, 1.0f, 1.0f);
  GLOBALS.GLOBJECTS.shader->setVec3("lightPos", GLOBALS.GLOBJECTS.LIGHT.x, GLOBALS.GLOBJECTS.LIGHT.y, GLOBALS.GLOBJECTS.LIGHT.z);
  printf("light pos %f, %f, %f\n", GLOBALS.GLOBJECTS.LIGHT.x, GLOBALS.GLOBJECTS.LIGHT.y, GLOBALS.GLOBJECTS.LIGHT.z);
}

void draw()
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the objects
  GLOBALS.GLOBJECTS.shader->use();

  // 3D Stuff
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(player.x, 0.0f, 0.0f));
  model = glm::rotate(model, player.tiltX, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::rotate(model, player.tiltY, glm::vec3(1.0f, 0.0f, 0.0f));
  
  glm::mat4 view = glm::mat4(1.0f);
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -20.0f));

  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float) CONSTANTS.WINDOW.WIDTH / CONSTANTS.WINDOW.HEIGHT, 0.1f, 100.0f);

  // Tell shader this stuff exists
  unsigned int modelLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.shader->id, "model");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  unsigned int viewLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.shader->id, "view");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

  unsigned int projectionLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.shader->id, "projection");
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glBindVertexArray(GLOBALS.GLOBJECTS.VAO);
  glDrawArrays(GL_TRIANGLES, 0, player.points.size());

  // Draw the light
  GLOBALS.GLOBJECTS.LIGHT.shader->use();

  // 3D Stuff
  glm::mat4 lightModel = glm::mat4(1.0f);
  lightModel = glm::translate(lightModel, glm::vec3(GLOBALS.GLOBJECTS.LIGHT.x, GLOBALS.GLOBJECTS.LIGHT.y, GLOBALS.GLOBJECTS.LIGHT.z));
  
  glm::mat4 lightView = glm::mat4(1.0f);

  glm::mat4 lightProjection = glm::perspective(glm::radians(45.0f), (float) CONSTANTS.WINDOW.WIDTH / CONSTANTS.WINDOW.HEIGHT, 0.1f, 100.0f);

  // Tell shader this stuff exists
  unsigned int lightModelLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.LIGHT.shader->id, "model");
  glUniformMatrix4fv(lightModelLoc, 1, GL_FALSE, glm::value_ptr(lightModel));

  unsigned int lightViewLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.LIGHT.shader->id, "view");
  glUniformMatrix4fv(lightViewLoc, 1, GL_FALSE, glm::value_ptr(lightView));

  unsigned int lightProjectionLoc = glGetUniformLocation(GLOBALS.GLOBJECTS.LIGHT.shader->id, "projection");
  glUniformMatrix4fv(lightProjectionLoc, 1, GL_FALSE, glm::value_ptr(lightProjection));

  glBindVertexArray(GLOBALS.GLOBJECTS.lightVAO);
  glDrawElements(GL_TRIANGLES, light.indices.size(), GL_UNSIGNED_INT, 0);

  SDL_GL_SwapWindow(GLOBALS.GAME.window); // Swap front and back buffers
}

/* 
 * ========================================
 * Find Normals Utility Function
 * ========================================
 */
void findNormals(std::vector<float>& data)
{
  for (int i = 0; i < player.points.size(); i += 9 * 3)
  {
    glm::vec3 p1(player.points[i], player.points[i + 1], player.points[i + 2]);
    glm::vec3 p2(player.points[i + 9], player.points[i + 10], player.points[i + 11]);
    glm::vec3 p3(player.points[i + 18], player.points[i + 19], player.points[i + 20]);

    glm::vec3 U = p2 - p1, V = p3 - p1;
    glm::vec3 N = U * V;

    player.points[i + 24] = player.points[i + 15] = player.points[i + 6] = U.y * V.z - U.z - V.y;
    player.points[i + 25] = player.points[i + 16] = player.points[i + 7] = U.z * V.x - U.x * V.z;
    player.points[i + 26] = player.points[i + 17] = player.points[i + 8] = U.x * V.y - U.y * V.x;
  }
}
