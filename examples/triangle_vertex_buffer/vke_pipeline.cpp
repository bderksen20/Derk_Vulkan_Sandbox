#include "vke_pipeline.hpp"
#include "vke_model.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cassert>

namespace vke {

	// Constructor: Just call below fxns
	VkePipeline::VkePipeline(
		VkDerkDevice& device,
		const std::string& vertFilepath,
		const std::string& fragFilepath,
		const PipelineConfigInfo configInfo)
		: vkDerkDevice{ device } {

		createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
	}

	VkePipeline::~VkePipeline() {
		vkDestroyShaderModule(vkDerkDevice.device(), vertShaderModule, nullptr);
		vkDestroyShaderModule(vkDerkDevice.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(vkDerkDevice.device(), graphicsPipeline, nullptr);
	}

	// READ FILE FXN
	std::vector<char> VkePipeline::readFile(const std::string& filepath) {

		// Input Stream Obj: ate = when file open, move to eof & binary = read file in as binary to avoid text transforms
		std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file: " + filepath);
		}

		// Ate bit flag means that we are at end of file --> so tellg gives current position of file get pointer which is file size!
		size_t fileSize = static_cast<size_t>(file.tellg());
		// Initialize character buffer with size of file
		std::vector<char> buffer(fileSize);

		// Seek to front of file and read into buffer
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	// For now, just uses read file fxn to dump code into buffer
	void VkePipeline::createGraphicsPipeline(const std::string& vertFilepath, const std::string& fragFilepath, const PipelineConfigInfo configInfo) {

		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no pipelineLayout provided in configure");
		assert(configInfo.renderPass != VK_NULL_HANDLE && "Cannot create graphics pipeline:: no renderPass provided in configure");
		// Read in code
		auto vertCode = readFile(vertFilepath);
		auto fragCode = readFile(fragFilepath);

		// Print vertex & fragment shadert code sizes
		/*
			std::cout << "Vertex Shader Code Size: " << vertCode.size() << '\n';
			std::cout << "Fragment Shader Code Size: " << fragCode.size() << '\n';
		*/

		createShaderModule(vertCode, &vertShaderModule);
		createShaderModule(fragCode, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// UPDATED TO USE VERTEX BUFFERS
		auto bindingDescriptions = VkeModel::Vertex::getBindingDescriptions();
		auto attributeDescriptions = VkeModel::Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());	
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = &configInfo.viewport;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = &configInfo.scissor;

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;	// (vert+frag)
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pDynamicState = nullptr;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		// Associated with booting new pipelines from previous = maybe perf boosts. Deal with later
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		// TODO: understand / trace input variables better
		if (vkCreateGraphicsPipelines(vkDerkDevice.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline");
		}


	}

	void VkePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(vkDerkDevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	void VkePipeline::bind(VkCommandBuffer commandBuffer) {
		// Point graphics specifies that this pipeline is a GRAPHICS pipeline. Also compute & raytracing options.
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}

	void VkePipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo, uint32_t width, uint32_t height) {
		//PipelineConfigInfo configInfo{};

		// First stage of pipeline. Recieves input list and converts it to geometry.
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		// every three vertices are grouped into a seperate triangle: v1,v2,v3,v4,v5,v6 --> tri1(v1,v2,v3) & tr2(v4,v5,v6)
		// another option is the triangle strip: after initial tri, uses last two verts as base for next --> v1,v2,v3,v4,v5,v6 = tri1(v1,v2,v3), tr12(v2,v3,v4),etc..
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Configure viewport: viewport describes transformation from pipeline output to target image
		configInfo.viewport.x = 0.0f;
		configInfo.viewport.y = 0.0f;
		configInfo.viewport.width = static_cast<float>(width);
		configInfo.viewport.height = static_cast<float>(height);
		configInfo.viewport.minDepth = 0.0f;
		configInfo.viewport.maxDepth = 1.0f;

		// Configure Scissor: cuts image output accordingly
		configInfo.scissor.offset = { 0, 0 };
		configInfo.scissor.extent = { width, height };

		// RASTERIZATION STAGE
		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;			// ENABLED (requires gpu feature): forces z gl position to be between 0-1, clamps values outside of range
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;	// What pieces of triangles to draw (entire thing, edges, verts??)
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;			// IMPORTANT: culls faces that do not match frontal face direction
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;	// the front face of tri = if vertices are defined in clockwise manner: ex, top->bottom right->bottom left
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;				// alter depth biases
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;		//optional
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;					//optional
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;			//optional

		// Multisampling Info: relates to how rasterizer handles geometry edges
		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;					//optional
		configInfo.multisampleInfo.pSampleMask = nullptr;					//optional
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;		//optional
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;		//optional

		// Color blending: controls how colors combineed in frame buffer
		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE;		// enables color blending
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		// Depth testing config: depth buffer attatches to frame buffer --> stores value for every pixel like color attatchment
		// explanation: mountains & cloud analogy
		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;	//optional
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;	//optional
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};	//optional
		configInfo.depthStencilInfo.back = {};	//optional

		//return configInfo;
	}

}