/* Smart Pro GLES probe: which sampler parameters and scene-buffer formats
 * the vendor driver (PowerVR Rogue GE8300, ES 3.2) actually accepts.
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
typedef void* (*get_string_fn)(unsigned);
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
#define EGL_NO_DISPLAY ((void*)0)
#define EGL_OPENGL_ES_API 0x30A2
#define EGL_OPENGL_ES3_BIT 0x0040
#define EGL_CONTEXT_MAJOR_VERSION 0x3098
#define EGL_CONTEXT_MINOR_VERSION 0x30FB
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_NO_CONTEXT ((void*)0)
#define EGL_NO_SURFACE ((void*)0)

/* GL constants */
#define GL_NO_ERROR 0
#define GL_INVALID_ENUM 0x0500
#define GL_INVALID_VALUE 0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_INVALID_FRAMEBUFFER_OPERATION 0x0506
#define GL_EXTENSIONS 0x1F03
#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01
#define GL_NUM_EXTENSIONS 0x821D
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_TEXTURE_WRAP_R 0x8072
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_LOD_BIAS 0x8501
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MIRROR_CLAMP_TO_EDGE 0x8743
#define GL_REPEAT 0x2901
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_SAMPLER_2D 0x8B5E
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
#define GL_RGBA16F 0x881A
#define GL_R32UI 0x8236
#define GL_DEPTH_COMPONENT32F 0x8CAC
#define GL_RGBA 0x1908
#define GL_FLOAT 0x1406
#define GL_HALF_FLOAT 0x140B
#define GL_RED_INTEGER 0x8D94
#define GL_UNSIGNED_INT 0x1405
#define GL_DEPTH_COMPONENT 0x1902
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_DRAW_BUFFER0 0x8825 /* GL_COLOR_ATTACHMENT0 alias use only */

typedef EGLDisplay (*egl_get_display_fn)(void*);
typedef EGLBoolean (*egl_initialize_fn)(EGLDisplay, EGLint*, EGLint*);
typedef EGLBoolean (*egl_bind_api_fn)(EGLenum);
typedef EGLBoolean (*egl_choose_config_fn)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
typedef EGLSurface (*egl_create_pbuffer_fn)(EGLDisplay, EGLConfig, const EGLint*);
typedef EGLContext (*egl_create_context_fn)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
typedef EGLBoolean (*egl_make_current_fn)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);

typedef void (*gl_gen_samplers_fn)(int, unsigned*);
typedef void (*gl_bind_sampler_fn)(unsigned, unsigned);
typedef void (*gl_sampler_param_i_fn)(unsigned, unsigned, int);
typedef void (*gl_sampler_param_f_fn)(unsigned, unsigned, float);
typedef unsigned (*gl_get_error_fn)(void);
typedef void* (*gl_get_string_fn)(unsigned);
typedef void* (*gl_get_stringi_fn)(unsigned, unsigned);
typedef void (*gl_get_integerv_fn)(unsigned, int*);
typedef void (*gl_gen_textures_fn)(int, unsigned*);
typedef void (*gl_bind_texture_fn)(unsigned, unsigned);
typedef void (*gl_tex_image_2d_fn)(unsigned, int, int, int, int, int, int, int, const void*);
typedef void (*gl_gen_framebuffers_fn)(int, unsigned*);
typedef void (*gl_bind_framebuffer_fn)(unsigned, unsigned);
typedef void (*gl_framebuffer_texture_fn)(unsigned, unsigned, unsigned, int);
typedef unsigned (*gl_check_framebuffer_status_fn)(unsigned);
typedef void (*gl_draw_buffers_fn)(int, const unsigned*);

static const char* errname(unsigned e)
{
	switch (e)
	{
	case GL_NO_ERROR: return "OK";
	case 0x0500: return "INVALID_ENUM";
	case 0x0501: return "INVALID_VALUE";
	case 0x0502: return "INVALID_OPERATION";
	case 0x0506: return "INVALID_FRAMEBUFFER_OPERATION";
	default: return "other";
	}
}

static unsigned drain(get_string_fn gs, gl_get_error_fn ge)
{
	(void)gs;
	unsigned e = ge(), n = 0;
	while (e != GL_NO_ERROR && n < 100) { e = ge(); n++; }
	return n ? e : GL_NO_ERROR;
}

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
	if (!p_get_display || !p_initialize || !p_choose_config || !p_create_context || !p_make_current)
	{ printf("EGL symbols missing\n"); return 1; }

	EGLDisplay dpy = p_get_display(EGL_DEFAULT_DISPLAY);
	EGLint maj = 0, min = 0;
	if (dpy == (void*)0 || !p_initialize(dpy, &maj, &min)) { printf("eglInitialize failed\n"); return 1; }
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
	egl_get_proc_fn gp = p_get_proc;
	#define GETGL(type, name) \
		type name = (type)(gp ? gp(#name) : NULL); \
		if (!name && gles) name = (type)dlsym(gles, #name); \
		if (!name) { printf("missing %s\n", #name); return 1; }

	GETGL(gl_gen_samplers_fn, glGenSamplers)
	GETGL(gl_bind_sampler_fn, glBindSampler)
	GETGL(gl_sampler_param_i_fn, glSamplerParameteri)
	GETGL(gl_sampler_param_f_fn, glSamplerParameterf)
	GETGL(gl_get_error_fn, glGetError)
	GETGL(gl_get_string_fn, glGetString)
	GETGL(gl_get_stringi_fn, glGetStringi)
	GETGL(gl_get_integerv_fn, glGetIntegerv)
	GETGL(gl_gen_textures_fn, glGenTextures)
	GETGL(gl_bind_texture_fn, glBindTexture)
	GETGL(gl_tex_image_2d_fn, glTexImage2D)
	GETGL(gl_gen_framebuffers_fn, glGenFramebuffers)
	GETGL(gl_bind_framebuffer_fn, glBindFramebuffer)
	GETGL(gl_framebuffer_texture_fn, glFramebufferTexture)
	GETGL(gl_check_framebuffer_status_fn, glCheckFramebufferStatus)
	GETGL(gl_draw_buffers_fn, glDrawBuffers)

	printf("GL_VERSION:  %s\n", (const char*)glGetString(GL_VERSION));
	printf("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));

	int num_ext = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	printf("glGetStringi ptr: %p; first calls:\n", (void*)glGetStringi);
	for (int i = 0; i < 3; i++)
	{
		const char* e = (const char*)glGetStringi(GL_EXTENSIONS, (unsigned)i);
		printf("  [%d] = %s\n", i, e ? e : "(NULL)");
	}
	int has_s3tc = 0, has_float_linear = 0, has_aniso = 0, has_mirror_clamp = 0, has_bufstorage = 0;
	for (int i = 0; i < num_ext; i++)
	{
		const char* e = (const char*)glGetStringi(GL_EXTENSIONS, (unsigned)i);
		if (!e) continue;
		if (strcmp(e, "GL_EXT_texture_compression_s3tc") == 0 || strcmp(e, "GL_WEBGL_compressed_texture_s3tc") == 0) has_s3tc = 1;
		if (strcmp(e, "GL_OES_texture_float_linear") == 0) has_float_linear = 1;
		if (strcmp(e, "GL_EXT_texture_filter_anisotropic") == 0) has_aniso = 1;
		if (strcmp(e, "GL_OES_texture_mirror_clamp") == 0 || strcmp(e, "GL_EXT_texture_mirror_clamp") == 0) has_mirror_clamp = 1;
		if (strcmp(e, "GL_EXT_buffer_storage") == 0) has_bufstorage = 1;
	}
	printf("extensions: %d (s3tc=%d float_linear=%d aniso=%d mirror_clamp_ext=%d buffer_storage=%d)\n",
		num_ext, has_s3tc, has_float_linear, has_aniso, has_mirror_clamp, has_bufstorage);

	while (glGetError() != GL_NO_ERROR) {}

	printf("\n-- sampler parameters --\n");
	unsigned s = 0;
	glGenSamplers(1, &s);
	unsigned e = glGetError();
	printf("glGenSamplers:        %s\n", errname(e));

	struct { const char* name; int is_f; unsigned param; int iv; float fv; } params[] = {
		{ "MIN_FILTER mipmap", 0, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR, 0 },
		{ "MAG_FILTER linear", 0, GL_TEXTURE_MAG_FILTER, GL_LINEAR, 0 },
		{ "WRAP_S REPEAT", 0, GL_TEXTURE_WRAP_S, GL_REPEAT, 0 },
		{ "WRAP_T REPEAT", 0, GL_TEXTURE_WRAP_T, GL_REPEAT, 0 },
		{ "WRAP_R REPEAT", 0, GL_TEXTURE_WRAP_R, GL_REPEAT, 0 },
		{ "WRAP_S MIRROR_CLAMP_TO_EDGE", 0, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE, 0 },
		{ "MIN_LOD 0", 1, GL_TEXTURE_MIN_LOD, 0, 0.0f },
		{ "LOD_BIAS 0", 1, GL_TEXTURE_LOD_BIAS, 0, 0.0f },
		{ "MAX_ANISOTROPY 8", 1, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0, 8.0f },
	};
	for (size_t i = 0; i < sizeof(params) / sizeof(params[0]); i++)
	{
		while (glGetError() != GL_NO_ERROR) {}
		if (params[i].is_f)
			glSamplerParameterf(s, params[i].param, params[i].fv);
		else
			glSamplerParameteri(s, params[i].param, params[i].iv);
		e = glGetError();
		printf("%-30s %s\n", params[i].name, errname(e));
	}
	while (glBindSampler(0, s), glGetError() != GL_NO_ERROR) {}
	printf("glBindSampler:        OK\n");

	printf("\n-- scene buffer formats --\n");
	struct { const char* name; int ifmt, fmt, type; int attachment; } texes[] = {
		{ "RGBA16F color", GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_COLOR_ATTACHMENT0 },
		{ "R32UI color", GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, GL_COLOR_ATTACHMENT1 },
		{ "D32F depth", GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, GL_DEPTH_ATTACHMENT },
		{ "RGBA16F msample-f", GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_COLOR_ATTACHMENT0 },
	};
	unsigned tex[4];
	glGenTextures(4, tex);
	for (size_t i = 0; i < 4; i++)
	{
		while (glGetError() != GL_NO_ERROR) {}
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, texes[i].ifmt, 64, 64, 0, texes[i].fmt, texes[i].type, NULL);
		e = glGetError();
		printf("%-20s glTexImage2D=%s", texes[i].name, errname(e));
		unsigned fbo = 0;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		while (glGetError() != GL_NO_ERROR) {}
		glFramebufferTexture(GL_FRAMEBUFFER, texes[i].attachment, tex[i], 0);
		e = glGetError();
		unsigned st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		printf(" attach=%s fbo=%s\n", errname(e), st == GL_FRAMEBUFFER_COMPLETE ? "complete" : "INCOMPLETE");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	printf("\n-- persistent mapping (GL_EXT_buffer_storage) --\n");
	{
		void* pbs = p_get_proc ? p_get_proc("glBufferStorage") : NULL;
		void* pmbr = p_get_proc ? p_get_proc("glMapBufferRange") : NULL;
		void* pfence = p_get_proc ? p_get_proc("glFenceSync") : NULL;
		void* pwait = p_get_proc ? p_get_proc("glClientWaitSync") : NULL;
		void* pdel = p_get_proc ? p_get_proc("glDeleteSync") : NULL;
		void* pgenbuf = p_get_proc ? p_get_proc("glGenBuffers") : NULL;
		void* pdelbuf = p_get_proc ? p_get_proc("glDeleteBuffers") : NULL;
		void* pbindbuf = p_get_proc ? p_get_proc("glBindBuffer") : NULL;
		printf("glBufferStorage via eglGetProcAddress: %s\n", pbs ? "resolved" : "NULL");
		if (pbs && !has_bufstorage) printf("  (extension string absent but entry point resolved)\n");
		if (pbs && pmbr && pfence && pwait && pdel && pgenbuf && pdelbuf && pbindbuf)
		{
			typedef void (*buf_storage_fn)(unsigned, long long, const void*, unsigned);
			typedef void* (*map_range_fn)(unsigned, long long, long long, unsigned);
			typedef void* (*fence_fn)(unsigned, long long);
			typedef unsigned (*wait_fn)(void*, unsigned long long, unsigned);
			typedef void (*del_fn)(void*);
			typedef void (*gen_buf_fn)(int, unsigned*);
			typedef void (*del_buf_fn)(int, const unsigned*);
			typedef void (*bind_buf_fn)(unsigned, unsigned);
			#define GL_MAP_PERSISTENT_BIT 0x0100
			#define GL_MAP_COHERENT_BIT 0x0200
			#define GL_MAP_WRITE_BIT 0x0001
			#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
			#define GL_ARRAY_BUFFER 0x8892
			#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
			#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
			#define GL_ALREADY_SIGNALED 0x911A
			#define GL_CONDITION_SATISFIED 0x911C
			buf_storage_fn p_storage = (buf_storage_fn)pbs;
			map_range_fn p_map = (map_range_fn)pmbr;
			fence_fn p_f = (fence_fn)pfence;
			wait_fn p_w = (wait_fn)pwait;
			del_fn p_ds = (del_fn)pdel;
			gen_buf_fn p_gb = (gen_buf_fn)pgenbuf;
			del_buf_fn p_db = (del_buf_fn)pdelbuf;
			bind_buf_fn p_bb = (bind_buf_fn)pbindbuf;

			unsigned vbo = 0;
			p_gb(1, &vbo);
			p_bb(GL_ARRAY_BUFFER, vbo);
			p_storage(GL_ARRAY_BUFFER, 4096, NULL, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
			unsigned e2 = glGetError();
			printf("glBufferStorage(PERSISTENT|COHERENT|WRITE, 4096): %s\n", errname(e2));
			if (e2 == GL_NO_ERROR)
			{
				void* m = p_map(GL_ARRAY_BUFFER, 0, 4096, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
				e2 = glGetError();
				printf("glMapBufferRange(persistent): %s ptr=%s\n", errname(e2), m ? "ok" : "NULL");
				if (m && e2 == GL_NO_ERROR)
				{
					for (int i = 0; i < 1024; i++) ((unsigned*)m)[i] = 0xdeadbeefu ^ (unsigned)i;
					void* f = p_f(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
					glGetError();
					unsigned r = p_w(f, 1000000000ull, GL_SYNC_FLUSH_COMMANDS_BIT);
					printf("fence wait: %s\n", (r == GL_ALREADY_SIGNALED || r == GL_CONDITION_SATISFIED) ? "signaled" : "timeout/other");
					int mismatches = 0;
					for (int i = 0; i < 1024; i++)
						if (((unsigned*)m)[i] != (0xdeadbeefu ^ (unsigned)i)) mismatches++;
					printf("coherent readback: %s\n", mismatches == 0 ? "OK (1024 dwords)" : "MISMATCH");
					if (p_ds && f) p_ds(f);
				}
			}
			if (vbo) p_db(1, &vbo);
		}
		while (glGetError() != GL_NO_ERROR) {}
	}

	printf("\n-- combined scene FBO (RGBA16F + R32UI + D32F, 2 draw buffers) --\n");
	unsigned fbo = 0;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	while (glGetError() != GL_NO_ERROR) {}
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex[0], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, tex[1], 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex[2], 0);
	unsigned bufs[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, bufs);
	e = glGetError();
	unsigned st = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	printf("drawBuffers(2)=%s status=%s\n", errname(e), st == GL_FRAMEBUFFER_COMPLETE ? "complete" : "INCOMPLETE");

	printf("\nprobe done\n");
	return 0;
}
