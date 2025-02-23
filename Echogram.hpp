#pragma once
#include <GL/glew.h>        // OpenGL Extension Wrangler Library (GLEW)
#include <GL/gl.h>     
#include <wx/wx.h>          // wxWidgets main header
#include <wx/glcanvas.h>    // wxWidgets OpenGL support
     // OpenGL core functions
#include <vector>           // For handling echogram data
#include <cstring>          // For efficient memory operations (memmove)

class EchogramCanvas : public wxGLCanvas {
public:
    EchogramCanvas(wxFrame* parent);
    ~EchogramCanvas();

    void ScrollData(const std::vector<uint8_t>& newPing);
    void Render();

private:
    wxGLContext* m_context;
    GLuint shaderProgram;
    GLuint textureID;
    std::vector<uint8_t> echogramData;

    void InitGL();
    void CreateShaders();
    void CreateTexture();
    void OnPaint(wxPaintEvent& event);

    // Helper functions for error handling
    void CheckShaderCompile(GLuint shader);
    void CheckProgramLink(GLuint program);

    wxDECLARE_EVENT_TABLE();
};
