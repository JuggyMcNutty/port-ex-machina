/* Is Vulkan usable on this device?
 *
 * Decisive for any renderer that needs more than GLES: the A133's PowerVR
 * GE8300 ships libVK_IMG.so (the IMG Vulkan driver) and a libvulkan loader,
 * but no ICD manifest -- so the loader finds nothing unless one is supplied.
 * This enumerates instance + physical devices and prints what it gets.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *dev_type(VkPhysicalDeviceType t) {
    switch (t) {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: return "integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:   return "discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:    return "virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:            return "CPU";
    default:                                     return "other";
    }
}

int main(void) {
    /* The device's libvulkan.so.1 is IMG's own 24KB dispatch shim exporting
     * only Vulkan 1.0 core, so vkEnumerateInstanceVersion (a 1.1 entry point)
     * is not there to call. */
    uint32_t n = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &n, NULL);
    printf("instance extensions: %u\n", n);
    if (n) {
        VkExtensionProperties *ext = calloc(n, sizeof *ext);
        vkEnumerateInstanceExtensionProperties(NULL, &n, ext);
        for (uint32_t i = 0; i < n; i++) printf("  %s\n", ext[i].extensionName);
        free(ext);
    }

    VkApplicationInfo ai = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "dxl vulkan probe",
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &ai,
    };
    VkInstance inst = VK_NULL_HANDLE;
    VkResult rc = vkCreateInstance(&ci, NULL, &inst);
    if (rc != VK_SUCCESS) {
        printf("vkCreateInstance FAILED: %d\n", (int)rc);
        if (rc == VK_ERROR_INCOMPATIBLE_DRIVER)
            printf("  (no usable ICD -- the loader found no driver manifest)\n");
        return 1;
    }
    printf("vkCreateInstance OK\n");

    uint32_t count = 0;
    vkEnumeratePhysicalDevices(inst, &count, NULL);
    printf("physical devices: %u\n", count);
    if (count) {
        VkPhysicalDevice *devs = calloc(count, sizeof *devs);
        vkEnumeratePhysicalDevices(inst, &count, devs);
        for (uint32_t i = 0; i < count; i++) {
            VkPhysicalDeviceProperties p;
            vkGetPhysicalDeviceProperties(devs[i], &p);
            printf("  [%u] %s -- %s, API %u.%u.%u, driver 0x%x\n", i, p.deviceName,
                   dev_type(p.deviceType),
                   VK_VERSION_MAJOR(p.apiVersion),
                   VK_VERSION_MINOR(p.apiVersion),
                   VK_VERSION_PATCH(p.apiVersion), p.driverVersion);
            printf("       maxImageDimension2D=%u  maxMemoryAllocationCount=%u\n",
                   p.limits.maxImageDimension2D, p.limits.maxMemoryAllocationCount);
        }
        free(devs);
    }
    vkDestroyInstance(inst, NULL);
    return count ? 0 : 2;
}
