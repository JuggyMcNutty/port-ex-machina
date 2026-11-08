/* GLES scene probe: what the engine's GL device asks of an OpenGL ES driver
 * for its scene beyond the samplers (probe-gles-sampler.c):
 *   - a clip distance, GL_EXT_clip_cull_distance: a shader that writes
 *     gl_ClipDistance[0] compiles, and the draw is clipped only while
 *     GL_CLIP_DISTANCE0 is on (a mirror's plane; without one the device
 *     clips by a discard);
 *   - RGB10_A2 scene buffers made with glTexStorage2D, beside D32F depth,
 *     as the engine's scene is, single and multisampled -- 4x halved until
 *     the colour and depth attachments both take it, as the device chooses
 *     (GLRenderDevice::ResizeSceneBuffers) -- the samples resolved by
 *     a blit, sampled by a pass, and read back as
 *     RGBA/UNSIGNED_INT_2_10_10_10_REV (as the device reads a shot), with all
 *     ten bits kept, and as RGBA/UNSIGNED_BYTE.
 * No link-time GL deps: everything through dlopen/dlsym, like the launcher's
 * GPU probe. Cross-compile with the aarch64 toolchain, run on the device. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <stdint.h>

typedef void* EGLDisplay;
typedef void* EGLConfig;
typedef void* EGLSurface;
typedef void* EGLContext;
typedef unsigned EGLenum;
typedef int EGLint;
typedef unsigned EGLBoolean;
typedef void* (*egl_get_proc_fn)(const char*);

#define EGL_NONE 0x3038
#define EGL_PBUFFER_BIT 0x0001
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_SURFACE_TYPE 0x3033
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3058
#define EGL_DEFAULT_DISPLAY ((void*)0)
#define EGL_OPENGL_ES_API 0x30A0
#define EGL_OPENGL_ES3_BIT 0x0040
#define EGL_CONTEXT_MAJOR_VERSION 0x3098
#define EGL_CONTEXT_MINOR_VERSION 0x30FB
#define EGL_CONTEXT_CLIENT_VERSION 0x3098

#define GL_NO_ERROR 0
#define GL_EXTENSIONS 0x1F03
#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01
#define GL_NUM_EXTENSIONS 0x821D
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_LINEAR 0x2601
#define GL_NEAREST 0x2600
#define GL_RGBA8 0x8058
#define GL_RGB10_A2 0x8059
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_COLOR_BUFFER_BIT 0x4000
#define GL_SAMPLES 0x80A9
#define GL_NUM_SAMPLE_COUNTS 0x9380
#define GL_MAX_COLOR_TEXTURE_SAMPLES 0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES 0x910F
#define GL_IMPLEMENTATION_COLOR_READ_TYPE 0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B
#define GL_CLIP_DISTANCE0 0x3000
#define GL_MAX_CLIP_DISTANCES 0x0D32
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_TRIANGLE_STRIP 0x0005
#define GL_FALSE 0

typedef EGLDisplay (*egl_get_display_fn)(void*);
typedef EGLBoolean (*egl_initialize_fn)(EGLDisplay, EGLint*, EGLint*);
typedef EGLBoolean (*egl_bind_api_fn)(EGLenum);
typedef EGLBoolean (*egl_choose_config_fn)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
typedef EGLSurface (*egl_create_pbuffer_fn)(EGLDisplay, EGLConfig, const EGLint*);
typedef EGLContext (*egl_create_context_fn)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
typedef EGLBoolean (*egl_make_current_fn)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);

typedef unsigned (*gl_get_error_fn)(void);
typedef const unsigned char* (*gl_get_string_fn)(unsigned);
typedef const unsigned char* (*gl_get_stringi_fn)(unsigned, unsigned);
typedef void (*gl_get_integerv_fn)(unsigned, int*);
typedef void (*gl_get_internalformativ_fn)(unsigned, unsigned, unsigned, int, int*);
typedef void (*gl_gen_fn)(int, unsigned*);
typedef void (*gl_bind_fn)(unsigned, unsigned);
typedef void (*gl_tex_parameteri_fn)(unsigned, unsigned, int);
typedef void (*gl_tex_storage_2d_fn)(unsigned, int, unsigned, int, int);
typedef void (*gl_tex_storage_2d_ms_fn)(unsigned, int, unsigned, int, int, unsigned char);
typedef void (*gl_framebuffer_texture_fn)(unsigned, unsigned, unsigned, int);
typedef unsigned (*gl_check_framebuffer_status_fn)(unsigned);
typedef void (*gl_draw_buffers_fn)(int, const unsigned*);
typedef void (*gl_read_buffer_fn)(unsigned);
typedef void (*gl_blit_framebuffer_fn)(int, int, int, int, int, int, int, int, unsigned, unsigned);
typedef void (*gl_read_pixels_fn)(int, int, int, int, unsigned, unsigned, void*);
typedef void (*gl_viewport_fn)(int, int, int, int);
typedef void (*gl_clear_color_fn)(float, float, float, float);
typedef void (*gl_clear_fn)(unsigned);
typedef void (*gl_enable_fn)(unsigned);
typedef unsigned (*gl_create_shader_fn)(unsigned);
typedef void (*gl_shader_source_fn)(unsigned, int, const char* const*, const int*);
typedef void (*gl_compile_shader_fn)(unsigned);
typedef void (*gl_get_shaderiv_fn)(unsigned, unsigned, int*);
typedef void (*gl_get_info_log_fn)(unsigned, int, int*, char*);
typedef unsigned (*gl_create_program_fn)(void);
typedef void (*gl_attach_shader_fn)(unsigned, unsigned);
typedef void (*gl_link_program_fn)(unsigned);
typedef void (*gl_use_program_fn)(unsigned);
typedef void (*gl_draw_arrays_fn)(unsigned, int, int);
typedef void (*gl_active_texture_fn)(unsigned);

static gl_get_error_fn glGetError;
static gl_get_string_fn glGetString;
static gl_get_stringi_fn glGetStringi;
static gl_get_integerv_fn glGetIntegerv;
static gl_get_internalformativ_fn glGetInternalformativ;
static gl_gen_fn glGenTextures;
static gl_gen_fn glGenFramebuffers;
static gl_gen_fn glGenVertexArrays;
static gl_bind_fn glBindTexture;
static gl_bind_fn glBindFramebuffer;
static void (*glBindVertexArray)(unsigned);
static gl_tex_parameteri_fn glTexParameteri;
static gl_tex_storage_2d_fn glTexStorage2D;
static gl_tex_storage_2d_ms_fn glTexStorage2DMultisample;
static gl_framebuffer_texture_fn glFramebufferTexture;
static gl_check_framebuffer_status_fn glCheckFramebufferStatus;
static gl_draw_buffers_fn glDrawBuffers;
static gl_read_buffer_fn glReadBuffer;
static gl_blit_framebuffer_fn glBlitFramebuffer;
static gl_read_pixels_fn glReadPixels;
static gl_viewport_fn glViewport;
static gl_clear_color_fn glClearColor;
static gl_clear_fn glClear;
static gl_enable_fn glEnable;
static gl_enable_fn glDisable;
static gl_create_shader_fn glCreateShader;
static gl_shader_source_fn glShaderSource;
static gl_compile_shader_fn glCompileShader;
static gl_get_shaderiv_fn glGetShaderiv;
static gl_get_info_log_fn glGetShaderInfoLog;
static gl_create_program_fn glCreateProgram;
static gl_attach_shader_fn glAttachShader;
static gl_link_program_fn glLinkProgram;
static gl_get_shaderiv_fn glGetProgramiv;
static gl_get_info_log_fn glGetProgramInfoLog;
static gl_use_program_fn glUseProgram;
static gl_draw_arrays_fn glDrawArrays;
static gl_active_texture_fn glActiveTexture;

#define SIZE 64

static const char* errname(unsigned e)
{
	switch (e)
	{
	case GL_NO_ERROR: return "OK";
	case 0x0500: return "INVALID_ENUM";
	case 0x0501: return "INVALID_VALUE";
	case 0x0502: return "INVALID_OPERATION";
	case 0x0505: return "OUT_OF_MEMORY";
	case 0x0506: return "INVALID_FRAMEBUFFER_OPERATION";
	default: return "other";
	}
}

/* The first error since the last call, the rest drained. */
static unsigned first_error(void)
{
	unsigned first = glGetError(), e = first;
	for (int n = 0; e != GL_NO_ERROR && n < 100; n++)
		e = glGetError();
	return first;
}

static int has_extension(const char* want)
{
	int n = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	for (int i = 0; i < n; i++)
	{
		const char* e = (const char*)glGetStringi(GL_EXTENSIONS, (unsigned)i);
		if (e && strcmp(e, want) == 0)
			return 1;
	}
	return 0;
}

/* A shader from its parts: the version header, then the body. */
static unsigned compile(unsigned kind, const char* header, const char* body, const char* clip, const char* what)
{
	const char* parts[4] = { header, body, clip ? clip : "", clip ? "}\n" : "" };
	unsigned s = glCreateShader(kind);
	glShaderSource(s, 4, parts, NULL);
	glCompileShader(s);
	int ok = 0;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
	if (!ok)
	{
		char log[1024] = "";
		glGetShaderInfoLog(s, (int)sizeof(log), NULL, log);
		printf("%-44s compile FAILED: %s\n", what, log);
		return 0;
	}
	return s;
}

static unsigned link_program(unsigned vs, unsigned fs, const char* what)
{
	if (!vs || !fs)
		return 0;
	unsigned p = glCreateProgram();
	glAttachShader(p, vs);
	glAttachShader(p, fs);
	glLinkProgram(p);
	int ok = 0;
	glGetProgramiv(p, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		char log[1024] = "";
		glGetProgramInfoLog(p, (int)sizeof(log), NULL, log);
		printf("%-44s link FAILED: %s\n", what, log);
		return 0;
	}
	return p;
}

/* A texture made as the scene buffers are: immutable storage, one level. */
static unsigned storage(unsigned format, int samples, unsigned* err)
{
	unsigned t = 0;
	glGenTextures(1, &t);
	first_error();
	if (samples > 1)
	{
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, t);
		glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, SIZE, SIZE, GL_FALSE);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, t);
		glTexStorage2D(GL_TEXTURE_2D, 1, format, SIZE, SIZE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	*err = first_error();
	return t;
}

static unsigned framebuffer(unsigned color, unsigned depth, const char* what)
{
	unsigned fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color, 0);
	if (depth)
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth, 0);
	unsigned bufs[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, bufs);
	unsigned e = first_error();
	unsigned st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	printf("%-44s attach=%s fbo=%s\n", what, errname(e), st == GL_FRAMEBUFFER_COMPLETE ? "complete" : "INCOMPLETE");
	return st == GL_FRAMEBUFFER_COMPLETE ? fbo : 0;
}

/* Pixel (x, y) of the bound read framebuffer as 10-bit channels. */
static void read10(int x, int y, unsigned rgba[4], unsigned* err)
{
	uint32_t p = 0;
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	first_error();
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, &p);
	*err = first_error();
	rgba[0] = p & 1023u;
	rgba[1] = (p >> 10) & 1023u;
	rgba[2] = (p >> 20) & 1023u;
	rgba[3] = p >> 30;
}

static void read8(int x, int y, unsigned char rgba[4], unsigned* err)
{
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	first_error();
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
	*err = first_error();
}

/* A quad over the whole target, without buffers; its main() is closed by
 * compile() after the clip line it is given. */
static const char* quad_vs =
	"const vec2 corners[4] = vec2[4](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(1.0, 1.0));\n"
	"out vec2 uv;\n"
	"void main()\n"
	"{\n"
	"	vec2 p = corners[gl_VertexID];\n"
	"	uv = p * 0.5 + 0.5;\n"
	"	gl_Position = vec4(p, 0.0, 1.0);\n";

int main(void)
{
	void* egl = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
	if (!egl) egl = dlopen("libEGL.so.1", RTLD_NOW | RTLD_GLOBAL);
	if (!egl) { printf("no libEGL: %s\n", dlerror()); return 1; }

	egl_get_display_fn p_get_display = (egl_get_display_fn)dlsym(egl, "eglGetDisplay");
	egl_initialize_fn p_initialize = (egl_initialize_fn)dlsym(egl, "eglInitialize");
	egl_bind_api_fn p_bind_api = (egl_bind_api_fn)dlsym(egl, "eglBindAPI");
	egl_choose_config_fn p_choose_config = (egl_choose_config_fn)dlsym(egl, "eglChooseConfig");
	egl_create_pbuffer_fn p_create_pbuffer = (egl_create_pbuffer_fn)dlsym(egl, "eglCreatePbufferSurface");
	egl_create_context_fn p_create_context = (egl_create_context_fn)dlsym(egl, "eglCreateContext");
	egl_make_current_fn p_make_current = (egl_make_current_fn)dlsym(egl, "eglMakeCurrent");
	egl_get_proc_fn p_get_proc = (egl_get_proc_fn)dlsym(egl, "eglGetProcAddress");
	if (!p_get_display || !p_initialize || !p_choose_config || !p_create_pbuffer || !p_create_context || !p_make_current)
	{ printf("EGL symbols missing\n"); return 1; }

	EGLDisplay dpy = p_get_display(EGL_DEFAULT_DISPLAY);
	EGLint maj = 0, min = 0;
	if (!dpy || !p_initialize(dpy, &maj, &min)) { printf("eglInitialize failed\n"); return 1; }
	printf("EGL %d.%d\n", (int)maj, (int)min);
	if (p_bind_api) p_bind_api(EGL_OPENGL_ES_API);

	EGLint cfg_attrs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
		EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
		EGL_NONE
	};
	EGLConfig cfg; EGLint ncfg = 0;
	if (!p_choose_config(dpy, cfg_attrs, &cfg, 1, &ncfg) || ncfg < 1) { printf("no ES3 config\n"); return 1; }
	EGLint pb[] = { EGL_WIDTH, 16, EGL_HEIGHT, 16, EGL_NONE };
	EGLSurface surf = p_create_pbuffer(dpy, cfg, pb);
	EGLint ctx32[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 2, EGL_NONE };
	EGLContext ctx = p_create_context(dpy, cfg, (void*)0, ctx32);
	if (!ctx)
	{
		/* EGL 1.4 has no MAJOR/MINOR attrs; CLIENT_VERSION 3 is its way. */
		EGLint cv3[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
		ctx = p_create_context(dpy, cfg, (void*)0, cv3);
	}
	if (!ctx) { printf("ES context failed\n"); return 1; }
	if (!p_make_current(dpy, surf, surf, ctx)) { printf("make current failed\n"); return 1; }

	void* gles = dlopen("libGLESv2.so.2", RTLD_NOW | RTLD_GLOBAL);
	if (!gles) gles = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
	#define GETGL(name) \
		name = (__typeof__(name))(p_get_proc ? p_get_proc(#name) : NULL); \
		if (!name && gles) name = (__typeof__(name))dlsym(gles, #name); \
		if (!name) { printf("missing %s\n", #name); return 1; }

	GETGL(glGetError) GETGL(glGetString) GETGL(glGetStringi) GETGL(glGetIntegerv)
	GETGL(glGetInternalformativ) GETGL(glGenTextures) GETGL(glGenFramebuffers)
	GETGL(glGenVertexArrays) GETGL(glBindTexture) GETGL(glBindFramebuffer)
	GETGL(glBindVertexArray) GETGL(glTexParameteri) GETGL(glTexStorage2D)
	GETGL(glTexStorage2DMultisample) GETGL(glFramebufferTexture)
	GETGL(glCheckFramebufferStatus) GETGL(glDrawBuffers) GETGL(glReadBuffer)
	GETGL(glBlitFramebuffer) GETGL(glReadPixels) GETGL(glViewport) GETGL(glClearColor)
	GETGL(glClear) GETGL(glEnable) GETGL(glDisable) GETGL(glCreateShader)
	GETGL(glShaderSource) GETGL(glCompileShader) GETGL(glGetShaderiv)
	GETGL(glGetShaderInfoLog) GETGL(glCreateProgram) GETGL(glAttachShader)
	GETGL(glLinkProgram) GETGL(glGetProgramiv) GETGL(glGetProgramInfoLog)
	GETGL(glUseProgram) GETGL(glDrawArrays) GETGL(glActiveTexture)

	printf("GL_VERSION:  %s\n", (const char*)glGetString(GL_VERSION));
	printf("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
	first_error();

	unsigned vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glViewport(0, 0, SIZE, SIZE);

	/* The GL device's shaders on ES: #version 320 es, then highp. An
	 * extension's directive goes between the two, before any other token. */
	const char* es = "#version 320 es\nprecision highp float;\nprecision highp int;\n";
	const char* es_clip = "#version 320 es\n#extension GL_EXT_clip_cull_distance : enable\nprecision highp float;\nprecision highp int;\n";

	printf("\n-- clip distance --\n");
	int clip_ext = has_extension("GL_EXT_clip_cull_distance");
	printf("GL_EXT_clip_cull_distance:                   %s\n", clip_ext ? "yes" : "NO");
	int max_clip = 0;
	glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip);
	printf("GL_MAX_CLIP_DISTANCES:                       %d (%s)\n", max_clip, errname(first_error()));
	{
		unsigned err = 0;
		unsigned color = storage(GL_RGBA8, 1, &err);
		unsigned fbo = framebuffer(color, 0, "RGBA8 target");
		const char* fs_src = "out vec4 o;\nin vec2 uv;\nvoid main() { o = vec4(1.0, 0.0, 0.0, 1.0); }\n";
		unsigned vs = compile(GL_VERTEX_SHADER, es_clip, quad_vs, "	gl_ClipDistance[0] = p.x;\n", "vertex shader writing gl_ClipDistance[0]");
		unsigned fs = compile(GL_FRAGMENT_SHADER, es, fs_src, NULL, "fragment shader");
		unsigned prog = link_program(vs, fs, "clip program");
		if (vs)
			printf("%-44s compiled\n", "vertex shader writing gl_ClipDistance[0]");
		if (prog && fbo)
		{
			glUseProgram(prog);
			for (int on = 0; on < 2; on++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT);
				first_error();
				if (on)
					glEnable(GL_CLIP_DISTANCE0);
				unsigned e_enable = first_error();
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				unsigned e_draw = first_error();
				unsigned char left[4], right[4];
				read8(SIZE / 8, SIZE / 2, left, &err);
				read8(SIZE - SIZE / 8, SIZE / 2, right, &err);
				int drawn_left = left[0] > 128, drawn_right = right[0] > 128;
				const char* verdict = on
					? (!drawn_left && drawn_right ? "clipped: OK" : "NOT clipped as asked")
					: (drawn_left && drawn_right ? "whole: OK" : "clipped while off");
				printf("GL_CLIP_DISTANCE0 %-3s enable=%s draw=%s     x<0 %s, x>0 %s: %s\n",
					on ? "on" : "off", errname(e_enable), errname(e_draw),
					drawn_left ? "drawn" : "empty", drawn_right ? "drawn" : "empty", verdict);
				if (on)
					glDisable(GL_CLIP_DISTANCE0);
			}
		}
	}

	printf("\n-- RGB10_A2 scene buffers --\n");
	{
		int fmt = 0, type = 0;
		int max_color = 0, max_depth = 0;
		glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &max_color);
		glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &max_depth);
		printf("max samples: color %d, depth %d\n", max_color, max_depth);
		int counts = 0, samples[16] = { 0 };
		glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGB10_A2, GL_NUM_SAMPLE_COUNTS, 1, &counts);
		if (counts > 16) counts = 16;
		glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGB10_A2, GL_SAMPLES, counts, samples);
		printf("RGB10_A2 multisample counts (%s):", errname(first_error()));
		for (int i = 0; i < counts; i++)
			printf(" %d", samples[i]);
		printf("\n");

		/* The engine's choice: 4x, halved until the colour and the depth
		 * attachments both take it. */
		int ms = 4;
		while (ms > 1 && (ms > max_color || ms > max_depth))
			ms /= 2;

		unsigned err = 0;
		unsigned color1 = storage(GL_RGB10_A2, 1, &err);
		printf("%-44s %s\n", "glTexStorage2D(RGB10_A2)", errname(err));
		unsigned depth1 = storage(GL_DEPTH_COMPONENT32F, 1, &err);
		unsigned scene1 = framebuffer(color1, depth1, "RGB10_A2 + D32F");
		unsigned pp_color = storage(GL_RGB10_A2, 1, &err);
		unsigned pp = framebuffer(pp_color, 0, "RGB10_A2 alone (post-process)");
		unsigned out_color = storage(GL_RGB10_A2, 1, &err);
		unsigned out = framebuffer(out_color, 0, "RGB10_A2 alone (pass output)");

		unsigned scene_ms = 0;
		if (ms > 1)
		{
			char what[64];
			unsigned color_ms = storage(GL_RGB10_A2, ms, &err);
			snprintf(what, sizeof(what), "glTexStorage2DMultisample(RGB10_A2, %d)", ms);
			printf("%-44s %s\n", what, errname(err));
			unsigned depth_ms = storage(GL_DEPTH_COMPONENT32F, ms, &err);
			snprintf(what, sizeof(what), "%dx RGB10_A2 + D32F", ms);
			scene_ms = framebuffer(color_ms, depth_ms, what);
		}
		else
			printf("multisampling: the engine would take 1 sample here\n");

		/* A dark colour only ten bits tell apart: 2, 3 and 5 of 1023 are 0, 1
		 * and 1 of 255 (and 0, 4 and 4 of 1023 again). */
		const char* fs_src = "out vec4 o;\nin vec2 uv;\nvoid main() { o = vec4(2.0, 3.0, 5.0, 1023.0) / 1023.0; }\n";
		unsigned vs = compile(GL_VERTEX_SHADER, es, quad_vs, "", "quad vertex shader");
		unsigned fs = compile(GL_FRAGMENT_SHADER, es, fs_src, NULL, "dark fragment shader");
		unsigned prog = link_program(vs, fs, "dark program");
		const char* double_src = "out vec4 o;\nin vec2 uv;\nlayout(binding = 0) uniform sampler2D image;\nvoid main() { o = texture(image, uv) * vec4(2.0, 2.0, 2.0, 1.0); }\n";
		unsigned fs2 = compile(GL_FRAGMENT_SHADER, es, double_src, NULL, "sampling fragment shader");
		unsigned prog2 = link_program(vs, fs2, "sampling program");

		unsigned targets[2] = { scene1, scene_ms };
		for (int t = 0; t < 2 && prog && pp; t++)
		{
			if (!targets[t])
				continue;
			glBindFramebuffer(GL_FRAMEBUFFER, targets[t]);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(prog);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			unsigned e_draw = first_error();
			/* Scene to post-process, as the device's present does it: the
			 * same format on both sides, so a multisampled source resolves. */
			glBindFramebuffer(GL_READ_FRAMEBUFFER, targets[t]);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pp);
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, SIZE, SIZE, 0, 0, SIZE, SIZE, GL_COLOR_BUFFER_BIT, GL_NEAREST);
			unsigned e_blit = first_error();
			glBindFramebuffer(GL_FRAMEBUFFER, pp);
			unsigned v10[4], e10 = 0, e8 = 0;
			unsigned char v8[4];
			read10(SIZE / 2, SIZE / 2, v10, &e10);
			read8(SIZE / 2, SIZE / 2, v8, &e8);
			printf("%s: draw=%s %s=%s\n", t ? "multisampled scene" : "scene", errname(e_draw), t ? "resolve" : "blit", errname(e_blit));
			printf("  read as 2_10_10_10_REV=%s: %u %u %u %u (want 2 3 5 3)%s\n", errname(e10),
				v10[0], v10[1], v10[2], v10[3], v10[0] == 2 && v10[1] == 3 && v10[2] == 5 ? ": ten bits kept" : ": NOT ten bits");
			printf("  read as UNSIGNED_BYTE=%s: %u %u %u %u\n", errname(e8), v8[0], v8[1], v8[2], v8[3]);
		}
		if (pp)
		{
			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &fmt);
			glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
			printf("implementation read format/type: 0x%x/0x%x\n", (unsigned)fmt, (unsigned)type);
		}
		if (prog2 && pp && out)
		{
			/* A post-process pass: the 10-bit image sampled linearly, doubled,
			 * into another 10-bit image. */
			glBindFramebuffer(GL_FRAMEBUFFER, out);
			glUseProgram(prog2);
			glActiveTexture(0x84C0); /* GL_TEXTURE0 */
			glBindTexture(GL_TEXTURE_2D, pp_color);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			unsigned e_draw = first_error();
			unsigned v10[4], e10 = 0;
			read10(SIZE / 2, SIZE / 2, v10, &e10);
			printf("sampled pass: draw=%s read=%s: %u %u %u (want 4 6 10)%s\n", errname(e_draw), errname(e10),
				v10[0], v10[1], v10[2], v10[0] == 4 && v10[1] == 6 && v10[2] == 10 ? ": ten bits kept" : ": NOT ten bits");
		}
	}

	printf("\nprobe done\n");
	return 0;
}
