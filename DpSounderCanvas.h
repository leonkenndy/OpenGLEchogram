// DpSounderCanvas.h
#pragma once
#include "GL/glew.h"
#include <wx/glcanvas.h>
#include <vector>

class DpSounderCanvas : public wxGLCanvas {
public:
	DpSounderCanvas(wxWindow* parent);
	virtual ~DpSounderCanvas();

	void OnNewEchogram(const std::vector<uint8_t>& pingData); // Call this with new data

private:
	void InitGL();
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

	wxGLContext* m_glContext;
	GLuint m_shaderProgram;
	GLuint m_vao;

	// Textures
	GLuint m_echogramTex;    // Main data texture (R8 format)
	GLuint m_lutTex;         // Color lookup texture

	// Texture parameters
	static const int TEX_WIDTH = 2048;
	static const int TEX_HEIGHT = 1024;
	int m_writePos = 0;      // Current write position in texture
	int m_pixelsPerPing = 1; // Columns per ping

	DECLARE_EVENT_TABLE()
};