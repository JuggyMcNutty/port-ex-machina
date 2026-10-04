/* M1 bring-up probe: does Mesa accept #version 420 shaders on a 3.2 core context? */
#define GL_GLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glcorearb.h>
#include <GL/glext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef const GLubyte* (*PFN_GetStringi)(GLenum, GLuint);

int main(int argc, char** argv)
{
	EGLint ctxMajor = argc > 1 ? atoi(argv[1]) : 3;
	EGLint ctxMinor = argc > 2 ? atoi(argv[2]) : 2;

	EGLDisplay dpy = eglGetPlatformDisplay(EGL_PLATFORM_X11_KHR, EGL_DEFAULT_DISPLAY, 0);
	if (dpy == EGL_NO_DISPLAY) { printf("no display\n"); return 1; }
	EGLint maj, min;
	if (!eglInitialize(dpy, &maj, &min)) { printf("eglInitialize failed\n"); return 1; }
	printf("EGL %d.%d\n", maj, min);
	if (!eglBindAPI(EGL_OPENGL_API)) { printf("bindapi failed\n"); return 1; }

	EGLint ctxAttrs[] = {
		EGL_CONTEXT_MAJOR_VERSION, ctxMajor,
		EGL_CONTEXT_MINOR_VERSION, ctxMinor,
		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};
	EGLint confAttrs[] = { EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_NONE };
	EGLConfig cfg; EGLint n = 0;
	if (!eglChooseConfig(dpy, confAttrs, &cfg, 1, &n) || n == 0) { printf("chooseconfig failed\n"); return 1; }

	EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttrs);
	if (ctx == EGL_NO_CONTEXT) { printf("context %d.%d failed: 0x%x\n", ctxMajor, ctxMinor, eglGetError()); return 1; }
	EGLint pbufAttrs[] = { EGL_WIDTH, 8, EGL_HEIGHT, 8, EGL_NONE };
	EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbufAttrs);
	if (!eglMakeCurrent(dpy, surf, surf, ctx)) { printf("makecurrent failed: 0x%x\n", eglGetError()); return 1; }

	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GLSL_VERSION: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	PFN_GetStringi GetStringi = (PFN_GetStringi)eglGetProcAddress("glGetStringi");
	if (GetStringi)
	{
		GLint num = 0;
		glGetIntegerv(GL_NUM_EXTENSIONS, &num); (void)num;
		GLint numShading = 0;
		glGetIntegerv(0x82FB /* GL_NUM_SHADING_LANGUAGE_VERSIONS */, &numShading);
		printf("GLSL versions supported on this context (%d):", numShading);
		for (GLint i = 0; i < numShading; i++)
			printf(" %s", GetStringi(GL_SHADING_LANGUAGE_VERSION, (GLuint)i));
		printf("\n");
	}

	const char* src = "#version 420\r\nout vec4 o; void main() { o = vec4(1.0); }";
	GLuint sh = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sh, 1, &src, 0);
	glCompileShader(sh);
	GLint ok = 0; glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
	char log[512] = { 0 };
	GLint len = 0; glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) glGetShaderInfoLog(sh, sizeof(log) - 1, 0, log);
	printf("compile '#version 420' fragment on %d.%d core context: %s%s\n",
		ctxMajor, ctxMinor, ok ? "OK" : "FAILED", log[0] ? " -- log: " : "");
	if (log[0]) printf("%s\n", log);

	return ok ? 0 : 2;
}
