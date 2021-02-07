// Instructions: run bat file to build for x64 VS2019, open .sln file and set as startup proj.

#include <vulkan/vulkan.h>
#include <assert.h>

#include <string.h>
VkInstance instance = VK_NULL_HANDLE;

// Function to determine if devices have specified required extensions:
// - ex: does GPU have swapchain or raytracing capabilites???
bool hasReqExts(const VkPhysicalDevice& physical_device, const char** required_extensions, uint32_t extension_count) {

	uint32_t device_extension_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);	// get amount of extensions

	VkExtensionProperties* extensions = new VkExtensionProperties[device_extension_count];
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, extensions);	// populate extensions

	// Loop the extensions we want, and then loop through the available extensions to determine if they match up
	for (uint32_t i = 0; i < extension_count; i++) {
		bool extension_found = false;
		for (uint32_t j = 0; j < device_extension_count; j++) {

			if (strcmp(required_extensions[i], extensions[j].extensionName) == 0) {

				extension_found = true;
				break;
			}
		}

		if (!extension_found) {
			delete[] extensions;
			return false;
		}

	}

	delete[] extensions;
	return true;
}

// Does GPU have the queue familiy that we want? If so, return queue fam index (passed by reference in function def)!!!
bool getQueueFam(const VkPhysicalDevice& physical_device, VkQueueFlags flags, uint32_t& queue_family_index) {

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	VkQueueFamilyProperties* queue_families = new VkQueueFamilyProperties[queue_family_count];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	for (uint32_t i = 0; i < queue_family_count; i++) {

		if (queue_families[i].queueCount > 0) {

			// Do bitwise AND between available queue fams and what we want to verify compatability
			if ((queue_families[i].queueFlags & flags) == flags) {

				queue_family_index = i;		// set queue family inddex if has (for return)
				delete[] queue_families;
				return true;

			}
		}
	}

	delete[] queue_families;
	return false;
}

int main(int argc, char** argv)
{

	/* T1: INSTANCE CREATION ----------------------------------------------------------------------------------- */
	// TODO: make a nice function to do this

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

	/* T2: DEVICES ---------------------------------------------------------------------------------------------- */

		// Determine available graphics rendering devices on machine
		
		// Params: the instance, pointer to variable for population, and array of devices
		// NOTE: passing nullptr tells vulkan to just figure out the number of devices which then allows for init of array of that size
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);		

		// Init device array with device count and call fxn again
		VkPhysicalDevice* devices = new VkPhysicalDevice[device_count];
		vkEnumeratePhysicalDevices(instance, &device_count, devices);

		// Check if devices are compatible: gpu needs swapchain functionality for this proj.
		// Example of another: VK_NV_RAY_TRACING_EXTENSION_NAME
		const uint32_t extension_count = 1;
		const char* device_extensions[extension_count] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		// Chosen device variables
		VkPhysicalDevice chosen_device = VK_NULL_HANDLE;
		uint32_t chosen_queue_family_index = 0;
		VkPhysicalDeviceProperties chosen_physical_device_properties;
		VkPhysicalDeviceFeatures chosen_physical_device_features;
		VkPhysicalDeviceMemoryProperties chosen_physical_device_mem_properties;

		// Loop through devices to check if meets above compatability req.
		for (uint32_t i = 0; i < device_count; i++) {

			if (hasReqExts(devices[i], device_extensions, extension_count)){

				//queue families specialist stuffs: does GPU have both compute + graphics functionality (AND transfer possibly | VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
				uint32_t queue_family_index = 0;
				if (getQueueFam(devices[i], VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, queue_family_index)){

					// Retrieve device properties, features, and memory properties!!!

					VkPhysicalDeviceProperties physical_device_properties;	// alt+f12 to peek struct def
					vkGetPhysicalDeviceProperties(							// useful info: vertice drawing limits, etc.
						devices[i], 
						&physical_device_properties
					);

					VkPhysicalDeviceFeatures physical_device_features;
					vkGetPhysicalDeviceFeatures(
						devices[i],
						&physical_device_features
					);

					VkPhysicalDeviceMemoryProperties physical_device_mem_properties;
					vkGetPhysicalDeviceMemoryProperties(
						devices[i],
						&physical_device_mem_properties
					);

					// If GPU meets extension & queue fam reqs, set as chosen device. HOWEVER, also ensure the better GPU is selected by overwriting if a discrete/actual GPU is found
					if (chosen_device == VK_NULL_HANDLE || physical_device_properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
						chosen_device = devices[i];
						chosen_physical_device_properties = physical_device_properties;
						chosen_physical_device_features = physical_device_features;
						chosen_physical_device_mem_properties = physical_device_mem_properties;
						chosen_queue_family_index = queue_family_index;
					}
						
				}
			}
		}

		// Assertion check to verify if device was selected. If capable device was selected (meaning previous reqs were satisfied), this will be fine.
		assert(chosen_device != VK_NULL_HANDLE);

	/* T3: VK Device --------------------------------------------------------------------------------------------------------------------------------------------- */
		// previous device work gets VK representation of GPU. Now need to setup actual vulkan device

		static const float queue_priority = 1.0f;	// set level of priority between queues (ex: focus on render queue over misc data)
		VkDevice device = VK_NULL_HANDLE;
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = chosen_queue_family_index;
		// how many queues to be created? just one for now...
		queue_create_info.queueCount = 1;

		// Use t2 vars to arm device info
		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pQueueCreateInfos = &queue_create_info;
		device_create_info.queueCreateInfoCount = 1;
		device_create_info.pEnabledFeatures = &chosen_physical_device_features;
		device_create_info.enabledExtensionCount = extension_count;
		device_create_info.ppEnabledExtensionNames = device_extensions;

		// Create device
		VkResult device_create = vkCreateDevice(
			chosen_device,			//reference to chosen physical device
			&device_create_info,	//reference to device info
			nullptr,				//malloc callback
			&device					//reference to device we are creating
		);
		
		// Check if device created successfully
		assert(device_create == VK_SUCCESS);

		// Destroy device
		vkDestroyDevice(device, nullptr);

	// Destroy instance
	vkDestroyInstance(instance, NULL);

	return 0;
}
