
#include "cvglContext.hpp"
#include "cvglCallbacks.hpp"

using namespace std;

/*
void cvglContext::set_time_uniform(float t)
{
    if( m_timeAttrib > -1 )
    {
        glUniform1fv(m_timeAttrib, 1, &t );
    }
}

void cvglContext::set_contrast_uniform(float x)
{
    if( m_contrastAttr > -1 )
    {
        glUniform1fv(m_contrastAttr, 1, &x );

    }
}
*/

void cvglContext::resize_callback(int w, int h)
{
    std::cout << "resize " << w << " " << h << std::endl;

    m_update_viewport = true;

    if( m_transformAttrib )
    {
        double aspect = (double)w / (double)h;
        double adjust = aspect /  m_aspectRatio ;
        
        std::cout << "new aspect " << aspect << " src aspect " << m_aspectRatio << " adjust Y " << adjust << std::endl;
        
        m_transform_matrix = glm::scale( glm::mat4(1.0f), glm::vec3(m_x_scale, m_y_scale * adjust, 1.0f) );


        glUniformMatrix4fv(m_transformAttrib, 1, GL_FALSE, &m_transform_matrix[0][0]);
        
    }

    m_wind_h = h;
    m_wind_w = w;

}

void cvglContext::flip( bool x, bool y)
{
    if( m_transformAttrib )
    {
        m_x_scale = x ? -1 : 1;
        m_y_scale = y ? -1 : 1;
        
        m_transform_matrix = glm::scale( m_transform_matrix, glm::vec3(m_x_scale, m_y_scale, 1.0f) );
        
        m_update_transform_matrix = true;
        
    }
    
}

cvglContext::cvglContext()
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        cout << "Failed to initialize GLFW" << endl;
        
        // throw exception here?
        return;
    }
    
}

cvglContext::~cvglContext()
{
    
    if( m_shaderProgram )
        glDeleteProgram(m_shaderProgram);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}



void context_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


void cvglContext::setupWindow(int width, int height )
{
    cout << "setting up window " << width <<  " " << height << endl;
    
    m_wind_h = height;
    m_wind_w = width;
    m_aspectRatio = (double)width / (double)height;
    
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GL_TRUE);

    glfwWindowHint(GLFW_DOUBLEBUFFER , 1);
  //  glfwWindowHint(GLFW_DEPTH_BITS, 24);
  //  glfwWindowHint(GLFW_STENCIL_BITS, 8);

   // std::string glsl_version = "";

#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    glsl_version = "#version 150";
    glfwWindowHint( // required on Mac OS
        GLFW_OPENGL_FORWARD_COMPAT,
        GL_TRUE
        );
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif __linux__
    // GL 3.2 + GLSL 150
   // glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#elif _WIN32
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif

    // Open a window and create its OpenGL context
    m_window = glfwCreateWindow( width, height, "cv gl", NULL, NULL);
    if( m_window == NULL ){
        cout <<"Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials." << endl;
        glfwTerminate();
        return;
    }
    
    float xscale, yscale;
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    glfwGetMonitorContentScale(monitor, &xscale, &yscale);

    cout << "\n\n\n monitor scaling " << xscale << " " << yscale << "\n\n\n" << endl;

    logWindow( m_window, this );
    
    glfwMakeContextCurrent(m_window);
    glfwSetWindowSizeCallback(m_window, global_resize_callback);
    

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
    }
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

    // hide cursor
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,  GL_NICEST);
    
    //glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSetErrorCallback(context_error_callback);
    
    glfwSwapInterval(1);
    
    cout << "initializing gl with size " << width << " " << height << " " << m_window << endl;
    
}

std::string slurp(std::ifstream& in) {
    std::stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

string getFileString(const char * file_path)
{
    ifstream in(file_path);
    stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int cvglContext::loadShaderFiles(const char * vertex_file_path, const char * fragment_file_path)
{
    string vertex_src = getFileString(vertex_file_path);
    string fragment_src = getFileString(fragment_file_path);
    
    if( !vertex_src.size() || !fragment_src.size() )
        return 0;
    
    if( loadShaders( vertex_src.c_str(), fragment_src.c_str() ) )
    {
        useShader();
        return 1;
    }
    
    return 0;
    
}

int cvglContext::loadShaderFiles(const string& vertex_file_path, const string& fragment_file_path)
{
    return loadShaderFiles(vertex_file_path.c_str(), fragment_file_path.c_str());
}

int cvglContext::loadShaders(const char * vertex_src, const char * fragment_src)
{
    
    // Create and compile the vertex shader
    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1, &vertex_src, NULL);
    glCompileShader(m_vertexShader);
    
    int  success;
    char infoLog[512];
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(m_vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << vertex_src << endl;
        return 0;
    }
    
    // Create and compile the fragment shader
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1, &fragment_src, NULL);
    glCompileShader(m_fragmentShader);
    
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(m_fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        std::cout << fragment_src << endl;
        return 0;
    }
    
    // Link the vertex and fragment shader into a shader program
    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, m_vertexShader);
    glAttachShader(m_shaderProgram, m_fragmentShader);
    glLinkProgram(m_shaderProgram);
    
    // check for linking errors
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return 0;
    }
    
    glUseProgram(m_shaderProgram);
    
    m_transformAttrib = glGetUniformLocation(m_shaderProgram, "transform_matrix");
    
    if( m_transformAttrib == -1 )
    {
        cout << "failed to find m_scaleAttrib " << endl;
        //    return 0;
    }
/*
    m_timeAttrib = glGetUniformLocation(m_shaderProgram, "time");
    if( m_timeAttrib == -1 )
    {
        cout << "failed to find m_timeAttrib " << endl;
        //    return 0;
    }
    
    m_contrastAttr = glGetUniformLocation(m_shaderProgram, "contrast");
    if( m_contrastAttr == -1 )
    {
        cout << "failed to find m_contrastAttr " << endl;
        //    return 0;
    }

    m_saturationAttr = glGetUniformLocation(m_shaderProgram, "saturation");
    if( m_saturationAttr == -1 )
    {
        cout << "failed to find m_saturationAttr " << endl;
        //    return 0;
    }
*/
    glBindFragDataLocation(m_shaderProgram, 0, "outColor");
    
    glUniformMatrix4fv(m_transformAttrib, 1, GL_FALSE, &m_transform_matrix[0][0]);
    
    glDetachShader(m_shaderProgram, m_vertexShader);
    glDetachShader(m_shaderProgram, m_fragmentShader);
    glDeleteShader(m_vertexShader);
    glDeleteShader(m_fragmentShader);
    
    return 1;
}

/*
 void cvglContext::genTexture()
 {
 //------
 // TEXTURE
 glGenTextures(1, &m_tex);
 glBindTexture(GL_TEXTURE_2D, m_tex);
 
 // wrap parameters
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
 
 // interpolation
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
 // Mipmap not sure if this is supposed to be used with the interpolation or not
 glGenerateMipmap(GL_TEXTURE_2D);
 }
 
 void cvglContext::setMatTexture(cv::Mat mat)
 {
 glTexImage2D(GL_TEXTURE_2D,     // Type of texture
 0,                 // Pyramid level (for mip-mapping) - 0 is the top level
 GL_RGB,            // Internal colour format to convert to
 mat.cols,          // Image width  i.e. 640 for Kinect in standard mode
 mat.rows,          // Image height i.e. 480 for Kinect in standard mode
 0,                 // Border width in pixels (can either be 1 or 0)
 GL_BGR, // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
 GL_UNSIGNED_BYTE,  // Image data type
 mat.ptr());        // The actual image data itself
 
 }
 
 */

/*
 
 GLuint loadShaders(const char * vertex_file_path, const char * fragment_file_path)
 {
 
 GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
 GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
 
 const char * vertex_src = getFileString(vertex_file_path);
 const char * fragment_src = getFileString(fragment_file_path);
 
 if( !vertex_src || !fragment_src )
 return 0;
 
 GLint Result = GL_FALSE;
 int InfoLogLength;
 
 // Compile Vertex Shader
 glShaderSource(vertexShaderID, 1, &vertex_src , NULL);
 glCompileShader(vertexShaderID);
 
 // Check Vertex Shader
 glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &Result);
 glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
 if ( InfoLogLength > 0 ){
 std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
 glGetShaderInfoLog(vertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
 cout << VertexShaderErrorMessage[0] << endl;
 }
 
 // Compile Fragment Shader
 glShaderSource(fragmentShaderID, 1, &fragment_src , NULL);
 glCompileShader(fragmentShaderID);
 
 // Check Fragment Shader
 glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &Result);
 glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
 if ( InfoLogLength > 0 ){
 std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
 glGetShaderInfoLog(fragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
 cout << FragmentShaderErrorMessage[0] << endl;
 }
 
 // Link the program
 GLuint programID = glCreateProgram();
 glAttachShader(programID, vertexShaderID);
 glAttachShader(programID, fragmentShaderID);
 glLinkProgram(programID);
 
 // Check the program
 glGetProgramiv(programID, GL_LINK_STATUS, &Result);
 glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
 if ( InfoLogLength > 0 ){
 std::vector<char> ProgramErrorMessage(InfoLogLength+1);
 glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
 printf("%s\n", &ProgramErrorMessage[0]);
 }
 
 
 glDetachShader(programID, vertexShaderID);
 glDetachShader(programID, fragmentShaderID);
 
 glDeleteShader(vertexShaderID);
 glDeleteShader(fragmentShaderID);
 
 return programID;
 }
 */
