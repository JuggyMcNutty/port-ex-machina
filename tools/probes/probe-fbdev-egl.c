/* Framebuffer and EGL probe: what a video driver of our own needs to know to
 * draw on a device whose only display path is fbdev plus the GPU vendor's EGL
 * (the Smart Pro's: /dev/fb0 and the PowerVR stack, no DRM, no X11, no
 * Wayland), and what the pads look like without libudev.
 *
 *   probe-fbdev-egl                 everything below, in order
 *   probe-fbdev-egl --draw <s>      a window surface cleared and swapped for <s>
 *                                   seconds (kill it with SIGKILL mid-draw, then
 *                                   run the probe again to see the next is healthy)
 *   probe-fbdev-egl --beside <cmd>  run <cmd> through /bin/sh while a window
 *                                   surface is live, and draw again after it
 *
 * It answers:
 *   framebuffer  var and fix info, the pixel layout and line_length; whether
 *                FBIOPUT_VSCREENINFO takes a 2x and 3x yres_virtual (the mode
 *                is put back after); whether FBIOPAN_DISPLAY and
 *                FBIO_WAITFORVSYNC work, and what a vsync wait costs
 *   EGL          which of libEGL.so.1, libEGL.so, libGLESv2.so.2, libGLESv2.so
 *                exist; the window configs; eglCreateWindowSurface with a
 *                {ushort w, h}, an {int w, h} and a NULL native window, each
 *                drawn and read back from fb0 at the visible page; swap
 *                interval 0 and 1 timings
 *   lifecycle    five create-draw-destroy-eglTerminate cycles in one process
 *   display      the fbcon cursor's state and the consoles' KD modes; the cost
 *                of a 1280x720 memcpy into fb0, at whatever CPU state the
 *                device is in
 *   input        each /dev/input/event*: its name, ids and capability bits;
 *                whether libudev.so.1 exists
 *
 * No link-time GL deps: everything through dlopen/dlsym, like the launcher's
 * GPU probe. Pause the device's menu first: two programs on one framebuffer
 * fight. Cross-compile with the port's toolchain, run on the device. */
#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef FBIO_WAITFORVSYNC
#define FBIO_WAITFORVSYNC _IOW('F', 0x20, uint32_t)
#endif

typedef void *EGLDisplay, *EGLConfig, *EGLSurface, *EGLContext;
typedef int EGLint;
typedef unsigned EGLBoolean;

#define EGL_NONE                   0x3038
#define EGL_BUFFER_SIZE            0x3020
#define EGL_ALPHA_SIZE             0x3021
#define EGL_BLUE_SIZE              0x3022
#define EGL_GREEN_SIZE             0x3023
#define EGL_RED_SIZE               0x3024
#define EGL_DEPTH_SIZE             0x3025
#define EGL_STENCIL_SIZE           0x3026
#define EGL_CONFIG_ID              0x3028
#define EGL_SAMPLES                0x3031
#define EGL_SURFACE_TYPE           0x3033
#define EGL_NATIVE_VISUAL_ID       0x302E
#define EGL_RENDERABLE_TYPE        0x3040
#define EGL_WINDOW_BIT             0x0004
#define EGL_OPENGL_ES2_BIT         0x0004
#define EGL_OPENGL_ES_API          0x30A0
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_WIDTH                  0x3057
#define EGL_HEIGHT                 0x3058
#define EGL_VENDOR                 0x3053
#define EGL_VERSION                0x3054
#define EGL_EXTENSIONS             0x3055
#define EGL_CLIENT_APIS            0x308D
#define EGL_SUCCESS                0x3000
#define EGL_DEFAULT_DISPLAY        ((void *)0)
#define EGL_NO_CONTEXT             ((void *)0)
#define EGL_NO_SURFACE             ((void *)0)

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_RENDERER         0x1F01
#define GL_VERSION          0x1F02

static struct {
    void *egl, *gles;
    EGLDisplay (*GetDisplay)(void *);
    EGLBoolean (*Initialize)(EGLDisplay, EGLint *, EGLint *);
    EGLBoolean (*Terminate)(EGLDisplay);
    EGLBoolean (*ReleaseThread)(void);
    const char *(*QueryString)(EGLDisplay, EGLint);
    EGLBoolean (*BindAPI)(unsigned);
    EGLBoolean (*ChooseConfig)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *);
    EGLBoolean (*GetConfigAttrib)(EGLDisplay, EGLConfig, EGLint, EGLint *);
    EGLSurface (*CreateWindowSurface)(EGLDisplay, EGLConfig, void *, const EGLint *);
    EGLBoolean (*DestroySurface)(EGLDisplay, EGLSurface);
    EGLBoolean (*QuerySurface)(EGLDisplay, EGLSurface, EGLint, EGLint *);
    EGLContext (*CreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint *);
    EGLBoolean (*DestroyContext)(EGLDisplay, EGLContext);
    EGLBoolean (*MakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
    EGLBoolean (*SwapBuffers)(EGLDisplay, EGLSurface);
    EGLBoolean (*SwapInterval)(EGLDisplay, EGLint);
    EGLint (*GetError)(void);
    void (*ClearColor)(float, float, float, float);
    void (*Clear)(unsigned);
    void (*Finish)(void);
    const unsigned char *(*GetString)(unsigned);
} E;

/* The native windows tried, in order: the community mali-fbdev driver's
 * struct fbdev_window { unsigned short width, height; }, the same with ints,
 * and none. */
struct win_us { unsigned short w, h; };
struct win_int { int w, h; };

static int fb = -1;
static struct fb_var_screeninfo var0;
static struct fb_fix_screeninfo fix;
static uint8_t *fbmem;
static size_t fblen;

static double now_ms(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static void *sym(void *lib, const char *name) {
    void *p = lib ? dlsym(lib, name) : NULL;
    if (!p && E.egl) p = dlsym(E.egl, name);
    return p;
}

static int load_egl(void) {
    if (E.egl) return 0;
    const char *egl_names[] = { "libEGL.so.1", "libEGL.so" };
    const char *gles_names[] = { "libGLESv2.so.2", "libGLESv2.so" };
    for (size_t i = 0; i < 2 && !E.egl; i++) E.egl = dlopen(egl_names[i], RTLD_NOW | RTLD_GLOBAL);
    for (size_t i = 0; i < 2 && !E.gles; i++) E.gles = dlopen(gles_names[i], RTLD_NOW | RTLD_GLOBAL);
    if (!E.egl || !E.gles) { printf("  EGL or GLESv2 not loadable\n"); return -1; }
#define L(f, n) *(void **)&E.f = sym(E.egl, n)
    L(GetDisplay, "eglGetDisplay"); L(Initialize, "eglInitialize"); L(Terminate, "eglTerminate");
    L(ReleaseThread, "eglReleaseThread"); L(QueryString, "eglQueryString"); L(BindAPI, "eglBindAPI");
    L(ChooseConfig, "eglChooseConfig"); L(GetConfigAttrib, "eglGetConfigAttrib");
    L(CreateWindowSurface, "eglCreateWindowSurface"); L(DestroySurface, "eglDestroySurface");
    L(QuerySurface, "eglQuerySurface"); L(CreateContext, "eglCreateContext");
    L(DestroyContext, "eglDestroyContext"); L(MakeCurrent, "eglMakeCurrent");
    L(SwapBuffers, "eglSwapBuffers"); L(SwapInterval, "eglSwapInterval"); L(GetError, "eglGetError");
#undef L
#define G(f, n) *(void **)&E.f = sym(E.gles, n)
    G(ClearColor, "glClearColor"); G(Clear, "glClear"); G(Finish, "glFinish"); G(GetString, "glGetString");
#undef G
    if (!E.GetDisplay || !E.CreateWindowSurface || !E.ClearColor) {
        printf("  EGL or GLESv2 lacks an entry point\n");
        return -1;
    }
    return 0;
}

/* ---- framebuffer ---------------------------------------------------------- */

static void print_var(const char *what, const struct fb_var_screeninfo *v) {
    printf("  %s: %ux%u, virtual %ux%u, offset %u,%u, %u bpp, "
           "r %u/%u g %u/%u b %u/%u a %u/%u, %ux%u mm, pixclock %u\n",
           what, v->xres, v->yres, v->xres_virtual, v->yres_virtual, v->xoffset, v->yoffset,
           v->bits_per_pixel, v->red.offset, v->red.length, v->green.offset, v->green.length,
           v->blue.offset, v->blue.length, v->transp.offset, v->transp.length,
           v->width, v->height, v->pixclock);
}

static int fb_open(void) {
    fb = open("/dev/fb0", O_RDWR | O_CLOEXEC);
    if (fb < 0) { printf("  /dev/fb0: %s\n", strerror(errno)); return -1; }
    if (ioctl(fb, FBIOGET_VSCREENINFO, &var0) < 0 || ioctl(fb, FBIOGET_FSCREENINFO, &fix) < 0) {
        printf("  FBIOGET_*SCREENINFO: %s\n", strerror(errno));
        return -1;
    }
    fblen = fix.smem_len;
    fbmem = mmap(NULL, fblen, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbmem == MAP_FAILED) { fbmem = NULL; printf("  mmap: %s\n", strerror(errno)); }
    return 0;
}

/* The pixel at the centre of the page the display shows now. */
static uint32_t fb_centre(void) {
    struct fb_var_screeninfo v;
    if (!fbmem || ioctl(fb, FBIOGET_VSCREENINFO, &v) < 0) return 0;
    size_t off = (size_t)(v.yoffset + v.yres / 2) * fix.line_length +
                 (size_t)(v.xoffset + v.xres / 2) * (v.bits_per_pixel / 8);
    if (off + 4 > fblen) return 0;
    uint32_t p;
    memcpy(&p, fbmem + off, 4);
    return p;
}

static void probe_fb(void) {
    printf("== framebuffer\n");
    if (fb_open() < 0) return;
    printf("  fix: id \"%.16s\", smem_len %u (%.1f pages of the visible size), line_length %u, "
           "type %u, visual %u, ypanstep %u, ywrapstep %u, accel %u\n",
           fix.id, fix.smem_len, (double)fix.smem_len / ((double)fix.line_length * var0.yres),
           fix.line_length, fix.type, fix.visual, fix.ypanstep, fix.ywrapstep, fix.accel);
    print_var("var", &var0);

    for (unsigned k = 2; k <= 3; k++) {
        struct fb_var_screeninfo v = var0, got;
        v.yres_virtual = var0.yres * k;
        v.yoffset = 0;
        int rc = ioctl(fb, FBIOPUT_VSCREENINFO, &v);
        int e = errno;
        ioctl(fb, FBIOGET_VSCREENINFO, &got);
        printf("  FBIOPUT_VSCREENINFO yres_virtual %ux: %s, now virtual %ux%u\n", k,
               rc == 0 ? "ok" : strerror(e), got.xres_virtual, got.yres_virtual);
    }
    {
        struct fb_var_screeninfo v;
        ioctl(fb, FBIOGET_VSCREENINFO, &v);
        unsigned pages = v.yres_virtual / v.yres;
        for (unsigned p = 0; p < pages && p < 3; p++) {
            struct fb_var_screeninfo w = v;
            w.yoffset = p * v.yres;
            int rc = ioctl(fb, FBIOPAN_DISPLAY, &w);
            int e = errno;
            struct fb_var_screeninfo got;
            ioctl(fb, FBIOGET_VSCREENINFO, &got);
            printf("  FBIOPAN_DISPLAY to page %u (yoffset %u): %s, yoffset now %u\n", p, w.yoffset,
                   rc == 0 ? "ok" : strerror(e), got.yoffset);
        }
    }
    {
        uint32_t crtc = 0;
        int rc = ioctl(fb, FBIO_WAITFORVSYNC, &crtc);
        if (rc < 0) {
            printf("  FBIO_WAITFORVSYNC: %s\n", strerror(errno));
        } else {
            double t0 = now_ms();
            for (int i = 0; i < 30; i++) ioctl(fb, FBIO_WAITFORVSYNC, &crtc);
            printf("  FBIO_WAITFORVSYNC: ok, %.2f ms apart over 30\n", (now_ms() - t0) / 30);
        }
    }
    int rc = ioctl(fb, FBIOPUT_VSCREENINFO, &var0);
    printf("  mode put back: %s\n", rc == 0 ? "ok" : strerror(errno));
}

/* ---- EGL ------------------------------------------------------------------ */

static void probe_libs(void) {
    printf("== EGL libraries\n");
    const char *names[] = { "libEGL.so.1", "libEGL.so", "libGLESv2.so.2", "libGLESv2.so" };
    for (size_t i = 0; i < 4; i++) {
        void *h = dlopen(names[i], RTLD_NOW | RTLD_LOCAL);
        printf("  %-15s %s\n", names[i], h ? "loads" : dlerror());
        if (h) dlclose(h);
    }
    void *u = dlopen("libudev.so.1", RTLD_NOW | RTLD_LOCAL);
    printf("  %-15s %s\n", "libudev.so.1", u ? "loads" : "absent");
    if (u) dlclose(u);
}

static EGLDisplay dpy;
static EGLConfig cfg;

static int egl_up(int verbose) {
    dpy = E.GetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj = 0, min = 0;
    if (!dpy || !E.Initialize(dpy, &maj, &min)) {
        printf("  eglInitialize failed: 0x%x\n", E.GetError());
        return -1;
    }
    if (verbose) {
        printf("  EGL %d.%d, vendor \"%s\", version \"%s\", APIs \"%s\"\n", maj, min,
               E.QueryString(dpy, EGL_VENDOR), E.QueryString(dpy, EGL_VERSION),
               E.QueryString(dpy, EGL_CLIENT_APIS));
        printf("  extensions: %s\n", E.QueryString(dpy, EGL_EXTENSIONS));
    }
    E.BindAPI(EGL_OPENGL_ES_API);
    const EGLint want[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_NONE };
    EGLConfig all[64];
    EGLint n = 0;
    if (!E.ChooseConfig(dpy, want, all, 64, &n) || n == 0) {
        printf("  no window config: 0x%x\n", E.GetError());
        return -1;
    }
    if (verbose) {
        printf("  window configs (ES2, RGB888): %d\n", n);
        for (EGLint i = 0; i < n; i++) {
            EGLint id, r, g, b, a, d, s, ms, vis;
            E.GetConfigAttrib(dpy, all[i], EGL_CONFIG_ID, &id);
            E.GetConfigAttrib(dpy, all[i], EGL_RED_SIZE, &r);
            E.GetConfigAttrib(dpy, all[i], EGL_GREEN_SIZE, &g);
            E.GetConfigAttrib(dpy, all[i], EGL_BLUE_SIZE, &b);
            E.GetConfigAttrib(dpy, all[i], EGL_ALPHA_SIZE, &a);
            E.GetConfigAttrib(dpy, all[i], EGL_DEPTH_SIZE, &d);
            E.GetConfigAttrib(dpy, all[i], EGL_STENCIL_SIZE, &s);
            E.GetConfigAttrib(dpy, all[i], EGL_SAMPLES, &ms);
            E.GetConfigAttrib(dpy, all[i], EGL_NATIVE_VISUAL_ID, &vis);
            printf("    id %2d: rgba %d%d%d%d depth %2d stencil %d samples %d visual 0x%x\n",
                   id, r, g, b, a, d, s, ms, vis);
        }
    }
    cfg = all[0];
    return 0;
}

/* A window surface on <native>, a context, one red clear read back from fb0.
 * Returns the surface made current (the caller tears it down), or NULL. */
static EGLContext ctx;
static EGLSurface surf_make(const char *what, void *native, int report) {
    EGLSurface s = E.CreateWindowSurface(dpy, cfg, native, NULL);
    if (!s) {
        if (report) printf("  window %-14s eglCreateWindowSurface: 0x%x\n", what, E.GetError());
        return NULL;
    }
    const EGLint ca[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    ctx = E.CreateContext(dpy, cfg, EGL_NO_CONTEXT, ca);
    if (!ctx || !E.MakeCurrent(dpy, s, s, ctx)) {
        if (report) printf("  window %-14s context: 0x%x\n", what, E.GetError());
        if (ctx) E.DestroyContext(dpy, ctx);
        E.DestroySurface(dpy, s);
        return NULL;
    }
    EGLint w = -1, h = -1;
    if (!E.QuerySurface(dpy, s, EGL_WIDTH, &w)) w = -(EGLint)E.GetError();
    if (!E.QuerySurface(dpy, s, EGL_HEIGHT, &h)) h = -(EGLint)E.GetError();
    E.ClearColor(1, 0, 0, 1);
    E.Clear(GL_COLOR_BUFFER_BIT);
    E.SwapBuffers(dpy, s);
    E.Finish();
    usleep(50000);
    uint32_t px = fb_centre();
    struct fb_var_screeninfo v;
    ioctl(fb, FBIOGET_VSCREENINFO, &v);
    if (report)
        printf("  window %-14s surface %dx%d, \"%s\", fb0 centre after a red swap 0x%08x "
               "at yoffset %u\n", what, w, h, (const char *)E.GetString(GL_RENDERER), px, v.yoffset);
    return s;
}

static void surf_drop(EGLSurface s) {
    E.MakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    E.DestroyContext(dpy, ctx);
    E.DestroySurface(dpy, s);
}

static void swap_timing(EGLSurface s) {
    for (int iv = 0; iv <= 1; iv++) {
        EGLBoolean ok = E.SwapInterval(dpy, iv);
        double t0 = now_ms(), worst = 0, prev = t0;
        for (int i = 0; i < 60; i++) {
            E.ClearColor(0, (float)(i & 1), 0, 1);
            E.Clear(GL_COLOR_BUFFER_BIT);
            E.SwapBuffers(dpy, s);
            double t = now_ms();
            if (t - prev > worst) worst = t - prev;
            prev = t;
        }
        struct fb_var_screeninfo v;
        ioctl(fb, FBIOGET_VSCREENINFO, &v);
        printf("  swap interval %d (%s): %.2f ms a swap over 60, worst %.2f; yoffset now %u\n", iv,
               ok ? "accepted" : "refused", (now_ms() - t0) / 60, worst, v.yoffset);
    }
}

static struct win_us wus;
static struct win_int wint;
/* The native window the cycles and the two modes use: the first that drew. */
static const char *native_what = "NULL";
static void *native;

static void probe_egl(void) {
    printf("== EGL window surfaces\n");
    if (load_egl() < 0 || egl_up(1) < 0) return;
    wus.w = (unsigned short)var0.xres; wus.h = (unsigned short)var0.yres;
    wint.w = (int)var0.xres; wint.h = (int)var0.yres;
    struct { const char *what; void *native; } tries[] = {
        { "{ushort w,h}", &wus }, { "{int w,h}", &wint }, { "NULL", NULL },
    };
    EGLSurface keep = NULL;
    for (size_t i = 0; i < 3; i++) {
        EGLSurface s = surf_make(tries[i].what, tries[i].native, 1);
        if (!s) continue;
        if (!keep) {
            swap_timing(s);
            keep = s;
            native_what = tries[i].what;
            native = tries[i].native;
        }
        surf_drop(s);
    }
    E.Terminate(dpy);
    E.ReleaseThread();
}

static void probe_cycles(void) {
    printf("== lifecycle: five create-draw-destroy-eglTerminate cycles, window %s\n", native_what);
    if (load_egl() < 0) return;
    for (int c = 1; c <= 5; c++) {
        double t0 = now_ms();
        if (egl_up(0) < 0) { printf("  cycle %d: eglInitialize failed\n", c); return; }
        EGLSurface s = surf_make(native_what, native, 0);
        uint32_t px = fb_centre();
        if (s) surf_drop(s);
        E.Terminate(dpy);
        E.ReleaseThread();
        printf("  cycle %d: %s, fb0 centre 0x%08x, %.0f ms\n", c, s ? "drew" : "no surface", px,
               now_ms() - t0);
    }
}

/* ---- display -------------------------------------------------------------- */

static void cat1(const char *path) {
    char buf[128] = "";
    FILE *f = fopen(path, "r");
    if (f) { if (!fgets(buf, sizeof buf, f)) buf[0] = 0; fclose(f); }
    buf[strcspn(buf, "\n")] = 0;
    printf("  %s: %s\n", path, f ? buf : strerror(errno));
}

static void probe_display(void) {
    printf("== display\n");
    cat1("/sys/class/graphics/fbcon/cursor_blink");
    cat1("/sys/class/vtconsole/vtcon1/bind");
    DIR *g = opendir("/sys/class/graphics");
    if (g) {
        struct dirent *e;
        printf("  /sys/class/graphics:");
        while ((e = readdir(g))) if (e->d_name[0] != '.') printf(" %s", e->d_name);
        printf("\n");
        closedir(g);
    }
    const char *ttys[] = { "/dev/tty0", "/dev/tty1", "/dev/console" };
    for (size_t i = 0; i < 3; i++) {
        int t = open(ttys[i], O_RDWR | O_NOCTTY | O_CLOEXEC);
        if (t < 0) { printf("  %s: %s\n", ttys[i], strerror(errno)); continue; }
        int mode = -1;
        int rc = ioctl(t, KDGETMODE, &mode);
        printf("  %s: opens; KDGETMODE %s\n", ttys[i],
               rc < 0 ? strerror(errno) : mode == KD_TEXT ? "text" : mode == KD_GRAPHICS ? "graphics" : "?");
        close(t);
    }
    cat1("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
    cat1("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq");
    cat1("/sys/devices/system/cpu/online");
    if (!fbmem) return;
    size_t n = (size_t)1280 * 720 * 4;
    if (n > fblen) n = fblen;
    uint8_t *src = malloc(n);
    if (!src) return;
    memset(src, 0x40, n);
    double t0 = now_ms();
    for (int i = 0; i < 20; i++) memcpy(fbmem, src, n);
    printf("  1280x720x4 memcpy into fb0: %.2f ms (over 20)\n", (now_ms() - t0) / 20);
    uint8_t *dst = malloc(n);
    if (dst) {
        t0 = now_ms();
        for (int i = 0; i < 20; i++) memcpy(dst, src, n);
        printf("  the same memcpy within RAM: %.2f ms\n", (now_ms() - t0) / 20);
        free(dst);
    }
    free(src);
}

/* ---- input ---------------------------------------------------------------- */

#define NBITS(x) ((((x) - 1) / (8 * sizeof(unsigned long))) + 1)
#define TEST(b, a) (((a)[(b) / (8 * sizeof(unsigned long))] >> ((b) % (8 * sizeof(unsigned long)))) & 1)

static void probe_input(void) {
    printf("== input\n");
    DIR *d = opendir("/dev/input");
    if (!d) { printf("  /dev/input: %s\n", strerror(errno)); return; }
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strncmp(e->d_name, "event", 5) != 0) continue;
        char path[16 + sizeof e->d_name];
        snprintf(path, sizeof path, "/dev/input/%s", e->d_name);
        int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
        if (fd < 0) { printf("  %s: %s\n", path, strerror(errno)); continue; }
        char name[128] = "?";
        struct input_id id = { 0 };
        unsigned long ev[NBITS(EV_MAX)] = { 0 }, key[NBITS(KEY_MAX)] = { 0 }, abs_[NBITS(ABS_MAX)] = { 0 };
        ioctl(fd, EVIOCGNAME(sizeof name), name);
        ioctl(fd, EVIOCGID, &id);
        ioctl(fd, EVIOCGBIT(0, sizeof ev), ev);
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof key), key);
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof abs_), abs_);
        int nkeys = 0, btn_gamepad = TEST(BTN_GAMEPAD, key), btn_thumbl = TEST(BTN_THUMBL, key);
        for (int k = 0; k < KEY_MAX; k++) nkeys += (int)TEST(k, key);
        printf("  %s: \"%s\" bus 0x%x vendor 0x%04x product 0x%04x; ev 0x%lx; %d keys "
               "(BTN_GAMEPAD %d, BTN_THUMBL %d); abs", path, name, id.bustype, id.vendor,
               id.product, ev[0], nkeys, btn_gamepad, btn_thumbl);
        for (int a = 0; a < ABS_MAX; a++) {
            if (!TEST(a, abs_)) continue;
            struct input_absinfo ai;
            if (ioctl(fd, EVIOCGABS(a), &ai) == 0) printf(" %d[%d..%d]", a, ai.minimum, ai.maximum);
        }
        printf("\n");
        close(fd);
    }
    closedir(d);
}

/* ---- modes ---------------------------------------------------------------- */

/* A window surface on the first native window that takes one. */
static EGLSurface first_surface(void) {
    if (fb_open() < 0 || load_egl() < 0 || egl_up(0) < 0) return NULL;
    wus.w = (unsigned short)var0.xres; wus.h = (unsigned short)var0.yres;
    wint.w = (int)var0.xres; wint.h = (int)var0.yres;
    EGLSurface s = surf_make("{ushort w,h}", &wus, 1);
    if (!s) s = surf_make("{int w,h}", &wint, 1);
    if (!s) s = surf_make("NULL", NULL, 1);
    return s;
}

static int draw_for(int secs) {
    EGLSurface s = first_surface();
    if (!s) return 1;
    printf("  drawing for %d s, pid %d\n", secs, (int)getpid());
    fflush(stdout);
    double end = now_ms() + secs * 1000.0;
    long frames = 0;
    while (now_ms() < end) {
        E.ClearColor(0, 0, (float)(frames & 1), 1);
        E.Clear(GL_COLOR_BUFFER_BIT);
        E.SwapBuffers(dpy, s);
        frames++;
    }
    printf("  %ld frames\n", frames);
    surf_drop(s);
    E.Terminate(dpy);
    return 0;
}

static int beside(const char *cmd) {
    EGLSurface s = first_surface();
    if (!s) return 1;
    printf("== beside a live window surface: %s\n", cmd);
    fflush(stdout);
    int st = system(cmd);
    printf("  exit status %d\n", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
    E.ClearColor(0, 1, 0, 1);
    E.Clear(GL_COLOR_BUFFER_BIT);
    EGLBoolean ok = E.SwapBuffers(dpy, s);
    E.Finish();
    usleep(50000);
    printf("  a green swap after it: %s (0x%x), fb0 centre 0x%08x\n", ok ? "ok" : "failed",
           E.GetError(), fb_centre());
    surf_drop(s);
    E.Terminate(dpy);
    return 0;
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc == 3 && strcmp(argv[1], "--draw") == 0) return draw_for(atoi(argv[2]));
    if (argc == 3 && strcmp(argv[1], "--beside") == 0) return beside(argv[2]);
    if (argc != 1) {
        fprintf(stderr, "usage: %s [--draw <seconds> | --beside <command>]\n", argv[0]);
        return 2;
    }
    probe_libs();
    probe_fb();
    probe_egl();
    probe_cycles();
    probe_display();
    probe_input();
    return 0;
}
