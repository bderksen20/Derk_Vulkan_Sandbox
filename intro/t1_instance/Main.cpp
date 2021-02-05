#include <vulkan/vulkan.h>
#include <assert.h>
VkInstance instance = VK_NULL_HANDLE;

int main(int argc, char** argv)
{
	// Define layer and extension arrays to say that we want standard debugging with project
	const char* instance_layers[] = { "VK_LAYER_LUNARG_standard_validation" };	
	const char* instance_extensions[] = { "VK_EXT_debug_report" };

	// Prepare app info and creation info to send into instance. Required.
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Instance";
	app_info.pEngineName = "My engine";
	app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.applicationVersion= VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion = VK_MAKE_VERSION(1, 1, 108);

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.enabledExtensionCount = 1;
	create_info.ppEnabledExtensionNames = instance_extensions;
	create_info.enabledLayerCount = 1;
	create_info.ppEnabledLayerNames = instance_layers;

	// Create instance
	VkResult result = vkCreateInstance(&create_info, NULL, &instance);

	// Asserts only compiled in debug mode: verify if instance successfully built
	assert(result == VK_SUCCESS);

	// Destroy instance
	vkDestroyInstance(instance, NULL);

	return 0;
}