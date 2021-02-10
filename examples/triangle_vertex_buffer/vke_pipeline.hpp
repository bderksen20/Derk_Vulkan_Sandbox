#pragma once

#include <string>
#include <vector>
#include "vk_derk_device.hpp"

namespace vke{

	// Contain data specifying pipe config. 
	// Pulling out of pipe class so app layer code can configure pipe and share w/ other pipes
	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		//Has an issue, deleted: VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VkePipeline {

		public:

			//T2 Constructor: VkePipeline(const std::string& vertFilepath, const std::string& fragFilepath);
			VkePipeline(
				VkDerkDevice &device, 
				const std::string& vertFilepath, 
				const std::string& fragFilepath, 
				const PipelineConfigInfo configInfo
			);

			~VkePipeline();

			// Delete copy constructors. 
			VkePipeline(const VkePipeline&) = delete;
			void operator = (const VkePipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer);
			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height);
			
		private:
			static std::vector<char> readFile(const std::string& filepath);

			void createGraphicsPipeline(
				const std::string& vertFilepath, 
				const std::string& fragFilepath,
				const PipelineConfigInfo conigInfo
			);

			// Input vars: character vector and pointer to shader module (vkshadermodule is a pointer, so double pointer!)
			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			// Member var storing device reference
			// Could be memory unsafe: if device is freed before pipeline, this would be a dangling pointer (derefrence = crash)
			// HOWEVER: pipeline NEEDS a device to exist (implict dependency), so it is ok for now 
			VkDerkDevice& vkDerkDevice;

			VkPipeline graphicsPipeline;		
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
	};

}