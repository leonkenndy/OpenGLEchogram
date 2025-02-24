// DpSounderCanvas.cpp
#include "DpSounderCanvas.h"

// Vertex shader
const char* vertShader = R"glsl(
#version 330 core
layout(location = 0) in vec2 position;
out vec2 texCoord;
uniform float scroll;

void main() {
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    // Convert position to texture coordinates with scrolling
    texCoord = vec2((position.x + 1.0) * 0.5 + scroll, 
                   1.0 - (position.y + 1.0) * 0.5); // Flip Y
}
)glsl";

// Fragment shader with LUT
const char* fragShader = R"glsl(
#version 330 core
in vec2 texCoord;
out vec4 outColor;
uniform sampler2D echogramTex;
uniform sampler1D colorLUT;

void main() {
    // Sample intensity and lookup color
    float intensity = texture(echogramTex, texCoord).r * 255.0;
    outColor = texture(colorLUT, intensity);
}
)glsl";

DpSounderCanvas::DpSounderCanvas(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition,
        wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
    m_glContext(nullptr) {
    m_glContext = new wxGLContext(this);
    SetCurrent(*m_glContext);

    glewExperimental = GL_TRUE;
    glewInit();
    InitGL();
}

DpSounderCanvas::~DpSounderCanvas() {
    SetCurrent(*m_glContext);

    glDeleteTextures(1, &m_echogramTex);
    glDeleteTextures(1, &m_lutTex);
    glDeleteProgram(m_shaderProgram);
    glDeleteVertexArrays(1, &m_vao);

    delete m_glContext;
}

void DpSounderCanvas::InitGL() {

    // Shader Program
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert, 1, &vertShader, nullptr);
    glCompileShader(vert);

    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag, 1, &fragShader, nullptr);
    glCompileShader(frag);

    m_shaderProgram = glCreateProgram();
    glAttachShader(m_shaderProgram, vert);
    glAttachShader(m_shaderProgram, frag);
    glLinkProgram(m_shaderProgram);
    glDeleteShader(vert);
    glDeleteShader(frag);

    // Full-screen quad
    float vertices[] = { -1, -1, 1, -1, -1, 1, 1, 1 };
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Echogram texture (single channel)
    glGenTextures(1, &m_echogramTex);
    glBindTexture(GL_TEXTURE_2D, m_echogramTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEX_WIDTH, TEX_HEIGHT, 0,
        GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Color LUT (example: grayscale to color)
    unsigned char lut[256 * 3];
    for (int i = 0; i < 256; i++) {
        lut[i * 3] = i;           // R
        lut[i * 3 + 1] = 255 - i;   // G
        lut[i * 3 + 2] = i / 2;       // B
    }

    glGenTextures(1, &m_lutTex);
    glBindTexture(GL_TEXTURE_1D, m_lutTex);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, lut);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void DpSounderCanvas::OnNewEchogram(const std::vector<uint8_t>& echogramData) {
    SetCurrent(*m_glContext);

    // Upload to texture
    glBindTexture(GL_TEXTURE_2D, m_echogramTex);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
        m_writePos, 0,
        m_pixelsPerPing, TEX_HEIGHT,
        GL_RED, GL_UNSIGNED_BYTE,
        echogramData.data());
    //std::vector<uint8_t> testData(TEX_HEIGHT, 255); // Full white line
    //glTexSubImage2D(GL_TEXTURE_2D, 0,
    //    m_writePos, 0,
    //    m_pixelsPerPing, TEX_HEIGHT,
    //    GL_RED, GL_UNSIGNED_BYTE,
    //    echogramData);

    // Update write position
   m_writePos = (m_writePos + m_pixelsPerPing) % TEX_WIDTH;

    Refresh(); // Trigger redraw
}

void DpSounderCanvas::OnPaint(wxPaintEvent& event) {
    SetCurrent(*m_glContext);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(m_shaderProgram);

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_echogramTex);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "echogramTex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, m_lutTex);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "colorLUT"), 1);

    // Calculate scroll (normalized offset)
    float scroll = static_cast<float>(m_writePos) / TEX_WIDTH;
    glUniform1f(glGetUniformLocation(m_shaderProgram, "scroll"), scroll);

    // Draw full-screen quad
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SwapBuffers();
}

void DpSounderCanvas::OnSize(wxSizeEvent& event) {
    SetCurrent(*m_glContext);
    wxSize size = GetClientSize();
    glViewport(0, 0, size.GetWidth(), size.GetHeight());
    event.Skip();
}

BEGIN_EVENT_TABLE(DpSounderCanvas, wxGLCanvas)
EVT_PAINT(DpSounderCanvas::OnPaint)
EVT_SIZE(DpSounderCanvas::OnSize)
END_EVENT_TABLE()