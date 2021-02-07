// Instructions: run bat file to build for x64 VS2019, open .sln file and set as startup proj.

#include <vulkan/vulkan.h>
#include <assert.h>

#include <string.h>
VkInstance instance = VK_NULL_HANDLE;

// T2: Device Selection Helper Function
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

// T2: Device Selection Helper Function
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

// T4: Buffers Helper Function
// loop through physical device memory properties (t1)and find valid mem type
uint32_t findMemType(uint32_t type_filter, VkMemoryPropertyFlags properties, VkPhysicalDeviceMemoryProperties& chosen_physical_device_mem_properties) {

	for (uint32_t i = 0; i < chosen_physical_device_mem_properties.memoryTypeCount; i++) {

		// If bits are set at pos of type filter AND the memory type at i matches (meaning memory type is valid), return
		if (type_filter & (1 << i) && (chosen_physical_device_mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return -1;
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

	/* T4: Command Pool ------------------------------------------------------------------------------------------------------------------------------------------- */

		// facilitates sending commands to GPU, creating buffer, sending render commands, etc.

		// Command pool setup: pool info/params, creation using device + info, assert check success
		VkCommandPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.queueFamilyIndex = chosen_queue_family_index;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;		// fresh state

		VkCommandPool command_pool = VK_NULL_HANDLE;
		VkResult cmdpool_create = vkCreateCommandPool(
			device,
			&pool_info,
			nullptr,
			&command_pool
		);

		assert(cmdpool_create == VK_SUCCESS);

	/* T5: Buffers -------------------------------------------------------------------------------------------------------------------------------------------------- */

		// Generate array of test data to pass to GPU
		unsigned int example_input_data[64];
		for (unsigned int i = 0; i < 64; i++) {
			example_input_data[i] = i;

		}

		// Setup GPU mem allocation
		VkDeviceSize size = sizeof(unsigned int) * 64;		// should be 4 bytes

		// Set mem visibility: in this instance, just doing host (CPU) visible
		VkMemoryPropertyFlags buffer_mem_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VkBuffer test_buffer = VK_NULL_HANDLE;					// logical rep. of data and how it will be used
		VkDeviceMemory test_buffer_mem = VK_NULL_HANDLE;		// actual memory to bind to buffer

		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// Pass size of buffer. Even though buffer is not responsible for mem, still needs to know much mem it will read/occupy
		buffer_info.size = size;	
		buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;		// alt+f12 usage for diff buffers, this one is for basic general data
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// exclusive vs. concurrent: buffer is only allocated by 1 thread at a time vs. multithreading

		// Create buffer
		VkResult buffer_create = vkCreateBuffer(
			device,				// device
			&buffer_info,		// buffer info
			nullptr,			// mem callback
			&test_buffer		// buffer point
		);

		assert(buffer_create == VK_SUCCESS);

		// Buffer created, but now need to init memory
		VkMemoryRequirements mem_reqs;
		vkGetBufferMemoryRequirements(			// gathers mem requirements needed  for buffer
			device,
			test_buffer,
			&mem_reqs
		);

		// Pass memory type requirements bits, buffer memory properties, and device to check if valid memory type
		uint32_t memory_type = findMemType(
			mem_reqs.memoryTypeBits, 
			buffer_mem_properties, 
			chosen_physical_device_mem_properties
		);

		// Create memory allocation structure
		VkMemoryAllocateInfo malloc_info = {};
		malloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		malloc_info.allocationSize = size;
		malloc_info.memoryTypeIndex = memory_type;

		// Allocate memory
		VkResult malloc_result = vkAllocateMemory(
			device,				// device that memory will be allocated on
			&malloc_info,		// pointer to allocation info
			nullptr,
			&test_buffer_mem	// pointer to buffer memory
		);

		assert(malloc_result == VK_SUCCESS);

		// Bind buffer to memory
		VkResult buff_bind_result = vkBindBufferMemory(
			device,
			test_buffer,
			test_buffer_mem,
			0						// offset
		);

		assert(buff_bind_result == VK_SUCCESS);

		// Get pointer to memmory on GPU (MAP MEMORY)
		void* mapped_mem = nullptr;

		VkResult map_mem_result = vkMapMemory(
			device,					// device that mem is allocated on
			test_buffer_mem,		// buffer memory that we have bound to
			0,						// offset on buffer memory 
			size,					// how much memory to map
			0,						// flag
			&mapped_mem
		);

		assert(map_mem_result == VK_SUCCESS);

		// Copy data onto GPU
		memcpy(
			mapped_mem,			// destination
			example_input_data,
			size
		);

		// NOTE: at this point, data is just chilling on GPU and is not interactable after unmapping
		vkUnmapMemory(device, test_buffer_mem);

		// Now.... remap data
		map_mem_result = vkMapMemory(
			device,					// device that mem is allocated on
			test_buffer_mem,		// buffer memory that we have bound to
			0,						// offset on buffer memory 
			size,					// how much memory to map
			0,						// flag
			&mapped_mem
		);
		assert(map_mem_result == VK_SUCCESS);

		// Create new array & populate with data directly from GPU
		unsigned int example_output_data[64];
		memcpy(example_output_data, mapped_mem, size);

		// Summary of above process: create buffer, allocate memory on GPU, bind buffer to memory, map GPU memory, transfer data to GPU, unmap GPU memory, remap memory, retrieve data
		// TODO: either wrap fxns for easier use OR setup Vulkan memmory allocator


	// Destroy buffer + clear memory
	vkDestroyBuffer(device, test_buffer, nullptr);
	vkFreeMemory(device, test_buffer_mem, nullptr);

	// Destroy command pool
	vkDestroyCommandPool(device, command_pool, nullptr);

	// Destroy device
	vkDestroyDevice(device, nullptr);

	// Destroy instance
	vkDestroyInstance(instance, NULL);

	return 0;
}
