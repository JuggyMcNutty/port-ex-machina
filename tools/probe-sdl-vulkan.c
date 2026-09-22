/* Can SDL2 on this device hand out a Vulkan surface?
 *
 * Decisive for the engine port. The vendor SDL2 registers only "dummy" and
 * "mali" video drivers, and the mali driver has no MALI_Vulkan_* entry points
 * -- but the binary does contain PVR_Vulkan_CreateSurface, so the hooks may be
 * wired to the PowerVR implementations under the mali driver's name.
 *
 * If this works, Surreal Engine's existing Vulkan renderer can drive the
 * device and no new renderer has to be written.
 */
#include <SDL.h>
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("video driver: %s\n", SDL_GetCurrentVideoDriver());

    if (SDL_Vulkan_LoadLibrary(NULL) != 0) {
        printf("SDL_Vulkan_LoadLibrary FAILED: %s\n", SDL_GetError());
        SDL_Quit();
        return 2;
    }
    printf("SDL_Vulkan_LoadLibrary OK\n");

    SDL_Window *w = SDL_CreateWindow("vk", 0, 0, 1280, 720,
                                     SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
    if (!w) {
        printf("CreateWindow(SDL_WINDOW_VULKAN) FAILED: %s\n", SDL_GetError());
        SDL_Quit();
        return 3;
    }
    printf("window with SDL_WINDOW_VULKAN created\n");

    unsigned int n = 0;
    if (!SDL_Vulkan_GetInstanceExtensions(w, &n, NULL)) {
        printf("GetInstanceExtensions FAILED: %s\n", SDL_GetError());
    } else {
        const char **names = calloc(n ? n : 1, sizeof *names);
        SDL_Vulkan_GetInstanceExtensions(w, &n, names);
        printf("SDL wants %u instance extension(s):", n);
        for (unsigned i = 0; i < n; i++) printf(" %s", names[i]);
        printf("\n");

        VkApplicationInfo ai = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                 .apiVersion = VK_API_VERSION_1_0 };
        VkInstanceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                    .pApplicationInfo = &ai,
                                    .enabledExtensionCount = n,
                                    .ppEnabledExtensionNames = names };
        VkInstance inst = VK_NULL_HANDLE;
        VkResult rc = vkCreateInstance(&ci, NULL, &inst);
        if (rc != VK_SUCCESS) {
            printf("vkCreateInstance with SDL's extensions FAILED: %d\n", (int)rc);
        } else {
            printf("vkCreateInstance OK\n");
            VkSurfaceKHR surface = VK_NULL_HANDLE;
            if (SDL_Vulkan_CreateSurface(w, inst, &surface) == SDL_TRUE) {
                printf("*** SDL_Vulkan_CreateSurface OK -- surface=%p ***\n",
                       (void *)(uintptr_t)surface);
                int dw = 0, dh = 0;
                SDL_Vulkan_GetDrawableSize(w, &dw, &dh);
                printf("drawable size: %dx%d\n", dw, dh);

                /* This is the check SurrealGPU's device filter actually fails
                 * on: a graphics queue family that can also present to THIS
                 * surface. Report every family so we can see which. */
                uint32_t pdn = 0;
                vkEnumeratePhysicalDevices(inst, &pdn, NULL);
                VkPhysicalDevice *pds = calloc(pdn ? pdn : 1, sizeof *pds);
                vkEnumeratePhysicalDevices(inst, &pdn, pds);
                for (uint32_t d = 0; d < pdn; d++) {
                    uint32_t qn = 0;
                    vkGetPhysicalDeviceQueueFamilyProperties(pds[d], &qn, NULL);
                    VkQueueFamilyProperties *qs = calloc(qn ? qn : 1, sizeof *qs);
                    vkGetPhysicalDeviceQueueFamilyProperties(pds[d], &qn, qs);
                    printf("device %u: %u queue family(ies)\n", d, qn);
                    for (uint32_t q = 0; q < qn; q++) {
                        VkBool32 present = VK_FALSE;
                        VkResult pr = vkGetPhysicalDeviceSurfaceSupportKHR(pds[d], q, surface, &present);
                        printf("  family %u: count=%u graphics=%s compute=%s "
                               "transfer=%s | surfaceSupport rc=%d present=%s\n",
                               q, qs[q].queueCount,
                               (qs[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "yes" : "no",
                               (qs[q].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "yes" : "no",
                               (qs[q].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "yes" : "no",
                               (int)pr, present ? "YES" : "NO");
                    }
                    free(qs);
                }
                free(pds);
            } else {
                printf("SDL_Vulkan_CreateSurface FAILED: %s\n", SDL_GetError());
            }
            vkDestroyInstance(inst, NULL);
        }
        free((void *)names);
    }
    SDL_DestroyWindow(w);
    SDL_Quit();
    return 0;
}
