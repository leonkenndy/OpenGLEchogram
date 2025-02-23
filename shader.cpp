#include "Echogram.hpp"
#include <cstring> // For memmove

const char* vertexShaderSrc = R"(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)";

const char* fragmentShaderSrc = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D echogramTexture;
    void main() {
        FragColor = texture(echogramTexture, TexCoord);
    }
)";

wxBEGIN_EVENT_TABLE(EchogramCanvas, wxGLCanvas)
EVT_PAINT(EchogramCanvas::OnPaint)
wxEND_EVENT_TABLE()

EchogramCanvas::EchogramCanvas(wxFrame* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr) {
    m_context = new wxGLContext(this);
    SetCurrent(*m_context);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        wxMessageBox("Failed to initialize GLEW!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    InitGL();
    CreateShaders();
    CreateTexture();
}

EchogramCanvas::~EchogramCanvas() {
    if (m_context) {
        SetCurrent(*m_context);
        glDeleteTextures(1, &textureID);
        glDeleteProgram(shaderProgram);
        delete m_context;
    }
}

void EchogramCanvas::InitGL() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void EchogramCanvas::CreateShaders() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);
    CheckShaderCompile(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);
    CheckShaderCompile(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckProgramLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void EchogramCanvas::CreateTexture() {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    echogramData.resize(1024 * 256, 0);  // Initialize empty echogram (1024 bins x 256 lines)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1024, 256, 0, GL_RED, GL_UNSIGNED_BYTE, echogramData.data());
}

void EchogramCanvas::ScrollData(const std::vector<uint8_t>& newPing) {
    // Efficient shifting using memmove
    for (int row = 0; row < 256; ++row) {
        std::memmove(&echogramData[row * 1024], &echogramData[row * 1024 + 1], 1023);
        echogramData[row * 1024 + 1023] = newPing[row];
    }

    // Update OpenGL texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 256, GL_RED, GL_UNSIGNED_BYTE, echogramData.data());
}

void EchogramCanvas::Render() {
    if (!m_context) return;

    SetCurrent(*m_context);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f, 1.0f);
    glEnd();

    SwapBuffers();
}

void EchogramCanvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    Render();
}

// Helper function to check shader compilation errors
void EchogramCanvas::CheckShaderCompile(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        wxMessageBox(wxString::Format("Shader Compilation Error:\n%s", infoLog), "Error", wxOK | wxICON_ERROR);
    }
}

// Helper function to check program linking errors
void EchogramCanvas::CheckProgramLink(GLuint program) {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        wxMessageBox(wxString::Format("Program Linking Error:\n%s", infoLog), "Error", wxOK | wxICON_ERROR);
    }
}
