/* Does vkGetPhysicalDeviceFeatures2 work on this driver?
 *
 * SurrealGPU creates its instance at API 1.2 when it can, and then reads
 * device features through vkGetPhysicalDeviceFeatures2 with a pNext chain.
 * The device's libvulkan.so.1 is IMG's ~24KB shim exporting only Vulkan 1.0
 * core, so the question is whether the 1.1+ entry point resolves through
 * vkGetInstanceProcAddr and whether it actually fills the chain.
 *
 * If Features2 reports the four required features as false while the plain 1.0
 * query reports them true, that is the whole bug.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define YN(x) ((x) ? "yes" : "NO")

static void report(const char *label, const VkPhysicalDeviceFeatures *f) {
    printf("  %-22s samplerAnisotropy=%-3s fragmentStoresAndAtomics=%-3s "
           "multiDrawIndirect=%-3s independentBlend=%-3s\n",
           label, YN(f->samplerAnisotropy), YN(f->fragmentStoresAndAtomics),
           YN(f->multiDrawIndirect), YN(f->independentBlend));
}

static void try_api(uint32_t api, const char *name) {
    printf("--- instance at API %s ---\n", name);
    VkApplicationInfo ai = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = api };
    VkInstanceCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                .pApplicationInfo = &ai };
    VkInstance inst = VK_NULL_HANDLE;
    VkResult rc = vkCreateInstance(&ci, NULL, &inst);
    if (rc != VK_SUCCESS) { printf("  vkCreateInstance failed: %d\n", (int)rc); return; }

    uint32_t n = 0;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    VkPhysicalDevice *devs = calloc(n ? n : 1, sizeof *devs);
    vkEnumeratePhysicalDevices(inst, &n, devs);

    PFN_vkGetPhysicalDeviceFeatures2 pF2 =
        (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(inst, "vkGetPhysicalDeviceFeatures2");
    PFN_vkGetPhysicalDeviceFeatures2 pF2KHR =
        (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(inst, "vkGetPhysicalDeviceFeatures2KHR");
    printf("  vkGetPhysicalDeviceFeatures2    = %s\n", pF2 ? "resolved" : "NULL");
    printf("  vkGetPhysicalDeviceFeatures2KHR = %s\n", pF2KHR ? "resolved" : "NULL");

    for (uint32_t i = 0; i < n; i++) {
        VkPhysicalDeviceFeatures f10;
        memset(&f10, 0, sizeof f10);
        vkGetPhysicalDeviceFeatures(devs[i], &f10);
        report("1.0 query:", &f10);

        PFN_vkGetPhysicalDeviceFeatures2 fn = pF2 ? pF2 : pF2KHR;
        if (fn) {
            /* Mirror what SurrealGPU does: a chained query. */
            VkPhysicalDeviceDescriptorIndexingFeatures di;
            memset(&di, 0, sizeof di);
            di.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

            VkPhysicalDeviceFeatures2 f2;
            memset(&f2, 0, sizeof f2);
            f2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            f2.pNext = &di;
            fn(devs[i], &f2);
            report("Features2 (chained):", &f2.features);

            VkPhysicalDeviceFeatures2 bare;
            memset(&bare, 0, sizeof bare);
            bare.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            fn(devs[i], &bare);
            report("Features2 (bare):", &bare.features);
            printf("    descriptorIndexing: runtimeDescriptorArray=%s partiallyBound=%s\n",
                   YN(di.runtimeDescriptorArray), YN(di.descriptorBindingPartiallyBound));
        }
    }
    free(devs);
    vkDestroyInstance(inst, NULL);
}

int main(void) {
    try_api(VK_API_VERSION_1_0, "1.0");
    try_api(VK_MAKE_VERSION(1,1,0), "1.1");
    try_api(VK_MAKE_VERSION(1,2,0), "1.2");
    return 0;
}
