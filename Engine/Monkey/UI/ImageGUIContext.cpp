#include "Engine.h"
#include "ImageGUIContext.h"

#include "Application/GenericWindow.h"
#include "Application/GenericApplication.h"

#include "File/FileManager.h"

static uint32_t g__glsl_shader_vert_spv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
    0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
    0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
    0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
    0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
    0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
    0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
    0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
    0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
    0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
    0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
    0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
    0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
    0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
    0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
    0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
    0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
    0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
    0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
    0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
    0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
    0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
    0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
    0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
    0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
    0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
    0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
    0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
    0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
    0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
    0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
    0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
    0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
    0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
    0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
    0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
    0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
    0x0000002d,0x0000002c,0x000100fd,0x00010038
};

static uint32_t g__glsl_shader_frag_spv[] =
{
    0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
    0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
    0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
    0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
    0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
    0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
    0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
    0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
    0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
    0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
    0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
    0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
    0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
    0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
    0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
    0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
    0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
    0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
    0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
    0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
    0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
    0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
    0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
    0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
    0x00010038
};

ImageGUIContext::ImageGUIContext()
    : m_VulkanDevice(nullptr)
    , m_VertexCount(0)
    , m_IndexCount(0)
    , m_Subpass(0)
    , m_DescriptorPool(VK_NULL_HANDLE)
    , m_DescriptorSetLayout(VK_NULL_HANDLE)
    , m_DescriptorSet(VK_NULL_HANDLE)
    , m_PipelineLayout(VK_NULL_HANDLE)
    , m_Pipeline(VK_NULL_HANDLE)
    , m_FontMemory(VK_NULL_HANDLE)
    , m_FontImage(VK_NULL_HANDLE)
    , m_FontView(VK_NULL_HANDLE)
    , m_FontSampler(VK_NULL_HANDLE)
    , m_Visible(false)
    , m_Updated(false)
    , m_Scale(1.0f)
    , m_FontPath("")
{
    
}

ImageGUIContext::~ImageGUIContext()
{
    
}

void ImageGUIContext::Init(const std::string& font, VkPipelineCache pipelineCache, VkRenderPass renderPass)
{
	int32 windowWidth  = Engine::Get()->GetPlatformWindow()->GetWidth();
	int32 windowHeight = Engine::Get()->GetPlatformWindow()->GetHeight();
	int32 frameWidth   = Engine::Get()->GetVulkanRHI()->GetSwapChain()->GetWidth();
	int32 frameHeight  = Engine::Get()->GetVulkanRHI()->GetSwapChain()->GetHeight();

	m_Scale = frameWidth / windowWidth;
	m_FontPath = font;
	m_VulkanDevice = Engine::Get()->GetVulkanDevice();
	
	ImGui::CreateContext();
	ImGui::StyleColorsLight();

	CreateImageFont();
	CreateLayout();
	CreatePipeline(pipelineCache, renderPass);
}

void ImageGUIContext::CreatePipeline(VkPipelineCache pipelineCache, VkRenderPass renderPass)
{
	VkDevice device = m_VulkanDevice->GetInstanceHandle();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
	inputAssemblyState.flags    = 0;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineRasterizationStateCreateInfo rasterizationState;
	ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
	rasterizationState.polygonMode      = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode         = VK_CULL_MODE_NONE;
	rasterizationState.frontFace        = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags            = 0;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.lineWidth        = 1.0f;

	VkPipelineColorBlendAttachmentState blendAttachmentState = {};
	blendAttachmentState.blendEnable         = VK_TRUE;
	blendAttachmentState.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
	blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendState;
	ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments    = &blendAttachmentState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
	depthStencilState.depthTestEnable  = VK_FALSE;
	depthStencilState.depthWriteEnable = VK_FALSE;
	depthStencilState.depthCompareOp   = VK_COMPARE_OP_ALWAYS;
	depthStencilState.front            = depthStencilState.back;
	depthStencilState.back.compareOp   = VK_COMPARE_OP_ALWAYS;

	VkPipelineViewportStateCreateInfo viewportState;
	ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
	viewportState.viewportCount = 1;
	viewportState.scissorCount  = 1;
	viewportState.flags         = 0;

	VkPipelineMultisampleStateCreateInfo multisampleState;
	ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.flags                = 0;

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState;
	ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
	dynamicState.pDynamicStates    = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = 2;
	dynamicState.flags             = 0;

	VkShaderModule vertModule = VK_NULL_HANDLE;
	VkShaderModule fragModule = VK_NULL_HANDLE;

	VkShaderModuleCreateInfo moduleCreateInfo;
    ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	
    moduleCreateInfo.codeSize = sizeof(g__glsl_shader_vert_spv);
    moduleCreateInfo.pCode    = g__glsl_shader_vert_spv;
    VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &vertModule));

	moduleCreateInfo.codeSize = sizeof(g__glsl_shader_frag_spv);
    moduleCreateInfo.pCode    = g__glsl_shader_frag_spv;
    VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &fragModule));

	VkPipelineShaderStageCreateInfo shaderStages[2] = {};
	ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vertModule;
	shaderStages[0].pName  = "main";
	shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = fragModule;
	shaderStages[1].pName  = "main";

	VkVertexInputBindingDescription vertexInputBinding = {};
	vertexInputBinding.binding   = 0;
	vertexInputBinding.stride    = sizeof(ImDrawVert);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttributes[3] = {};
	vertexInputAttributes[0].location = 0;
	vertexInputAttributes[0].binding  = 0;
	vertexInputAttributes[0].format   = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributes[0].offset   = offsetof(ImDrawVert, pos);

	vertexInputAttributes[1].location = 1;
	vertexInputAttributes[1].binding  = 0;
	vertexInputAttributes[1].format   = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributes[1].offset   = offsetof(ImDrawVert, uv);

	vertexInputAttributes[2].location = 2;
	vertexInputAttributes[2].binding  = 0;
	vertexInputAttributes[2].format   = VK_FORMAT_R8G8B8A8_UNORM;
	vertexInputAttributes[2].offset   = offsetof(ImDrawVert, col);

	VkPipelineVertexInputStateCreateInfo vertexInputState;
	ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
	vertexInputState.vertexBindingDescriptionCount   = 1;
	vertexInputState.pVertexBindingDescriptions      = &vertexInputBinding;
	vertexInputState.vertexAttributeDescriptionCount = 3;
	vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributes;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo;
	ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	pipelineCreateInfo.layout              = m_PipelineLayout;
	pipelineCreateInfo.renderPass          = renderPass;
	pipelineCreateInfo.flags               = 0;
	pipelineCreateInfo.basePipelineIndex   = -1;
	pipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState    = &colorBlendState;
	pipelineCreateInfo.pMultisampleState   = &multisampleState;
	pipelineCreateInfo.pViewportState      = &viewportState;
	pipelineCreateInfo.pDepthStencilState  = &depthStencilState;
	pipelineCreateInfo.pDynamicState       = &dynamicState;
	pipelineCreateInfo.stageCount          = 2;
	pipelineCreateInfo.pStages             = shaderStages;
	pipelineCreateInfo.subpass             = m_Subpass;
	pipelineCreateInfo.pVertexInputState   = &vertexInputState;

	VERIFYVULKANRESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));	
}

void ImageGUIContext::CreateLayout()
{
	VkDevice device = m_VulkanDevice->GetInstanceHandle();

	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo descriptorPoolInfo;
	ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	descriptorPoolInfo.poolSizeCount = 1;
	descriptorPoolInfo.pPoolSizes    = &poolSize;
	descriptorPoolInfo.maxSets       = 2;
	VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

	VkDescriptorSetLayoutBinding setLayoutBinding = {};
	setLayoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setLayoutBinding.stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT;
	setLayoutBinding.binding         = 0;
	setLayoutBinding.descriptorCount = 1;

	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
	ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	setLayoutCreateInfo.pBindings    = &setLayoutBinding;
	setLayoutCreateInfo.bindingCount = 1;
	VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRange.offset     = 0;
	pushConstantRange.size       = sizeof(PushConstBlock);

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
	pipelineLayoutCreateInfo.setLayoutCount         = 1;
	pipelineLayoutCreateInfo.pSetLayouts            = &m_DescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;

	VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));

	VkDescriptorSetAllocateInfo setAllocateInfo;
	ZeroVulkanStruct(setAllocateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
	setAllocateInfo.descriptorPool     = m_DescriptorPool;
	setAllocateInfo.pSetLayouts        = &m_DescriptorSetLayout;
	setAllocateInfo.descriptorSetCount = 1;
	VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &setAllocateInfo, &m_DescriptorSet));

	VkDescriptorImageInfo fontDescriptor = {};
	fontDescriptor.sampler     = m_FontSampler;
	fontDescriptor.imageView   = m_FontView;
	fontDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet writeDescriptorSet;
	ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
	writeDescriptorSet.dstSet          = m_DescriptorSet;
	writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding      = 0;
	writeDescriptorSet.pImageInfo      = &fontDescriptor;
	writeDescriptorSet.descriptorCount = 1;
	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
}

void ImageGUIContext::Destroy()
{
    ImGui::DestroyContext();
}

void ImageGUIContext::CreateImageFont()
{
	VkDevice device = m_VulkanDevice->GetInstanceHandle();

	// font io
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = m_Scale;

	// load font file
	uint8* dataPtr  = nullptr;
    uint32 dataSize = 0;
    FileManager::ReadFile(m_FontPath, dataPtr, dataSize);
	io.Fonts->AddFontFromMemoryTTF(dataPtr, dataSize, 16.0f);

	// get font image data
	uint8* fontData = nullptr;
	int32 texWidth  = 0;
	int32 texHeight = 0;
	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
	VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(uint8);

	// mem alloc info
	uint32 memoryTypeIndex = 0;
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	// font image
	VkImageCreateInfo imageCreateInfo;
	ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType	  = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format		  = VK_FORMAT_R8G8B8A8_UNORM;
	imageCreateInfo.extent.width  = texWidth;
	imageCreateInfo.extent.height = texHeight;
	imageCreateInfo.extent.depth  = 1;
	imageCreateInfo.mipLevels	  = 1;
	imageCreateInfo.arrayLayers   = 1;
	imageCreateInfo.samples		  = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling		  = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage		  = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FontImage));

	// font memory
	vkGetImageMemoryRequirements(device, m_FontImage, &memReqs);
	m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize  = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_FontMemory));

	// bind memory to image
	VERIFYVULKANRESULT(vkBindImageMemory(device, m_FontImage, m_FontMemory, 0));

	// view info
	VkImageViewCreateInfo viewCreateInfo;
	ZeroVulkanStruct(viewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	viewCreateInfo.image    = m_FontImage;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format   = VK_FORMAT_R8G8B8A8_UNORM;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	VERIFYVULKANRESULT(vkCreateImageView(device, &viewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FontView));

	// sampler info
	VkSamplerCreateInfo samplerInfo;
	ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.magFilter     = VK_FILTER_LINEAR;
	samplerInfo.minFilter     = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.borderColor   = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &m_FontSampler));

	// staging info
	VkBuffer       stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

	// staging buffer
	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	bufferCreateInfo.size  = uploadSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &stagingBuffer));
    
	// staging memory
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);
	m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
	memAllocInfo.allocationSize  = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &stagingMemory));

	// bind staging buffer to memory
	VERIFYVULKANRESULT(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));
    
	// copy data to staging memory
	void* mappedPtr = nullptr;
	VERIFYVULKANRESULT(vkMapMemory(device, stagingMemory, 0, memAllocInfo.allocationSize, 0, &mappedPtr));
	std::memcpy(mappedPtr, fontData, uploadSize);
	vkUnmapMemory(device, stagingMemory);

	// prepare command
	VkCommandPool   commandPool = VK_NULL_HANDLE;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;

	// command pool
	VkCommandPoolCreateInfo cmdPoolInfo;
	ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	cmdPoolInfo.queueFamilyIndex = m_VulkanDevice->GetTransferQueue()->GetFamilyIndex();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &commandPool));

	// command buffer
	VkCommandBufferAllocateInfo cmdCreateInfo;
	ZeroVulkanStruct(cmdCreateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	cmdCreateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdCreateInfo.commandPool = commandPool;
	cmdCreateInfo.commandBufferCount = 1;
	VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &cmdCreateInfo, &cmdBuffer));

	// begin record
	VkCommandBufferBeginInfo cmdBeginInfo;
	ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	VERIFYVULKANRESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBeginInfo));

	VkImageMemoryBarrier imageMemoryBarrier;
	ZeroVulkanStruct(imageMemoryBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	
	VkImageSubresourceRange subresourceRange = {};

	// image barrier for transfer
	subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount   = 1;
	subresourceRange.layerCount   = 1;

	imageMemoryBarrier.image     = m_FontImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = 0;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_HOST_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
	);

	// copy buffer to image
	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width  = texWidth;
	bufferCopyRegion.imageExtent.height = texHeight;
	bufferCopyRegion.imageExtent.depth  = 1;

	vkCmdCopyBufferToImage(
		cmdBuffer,
		stagingBuffer,
		m_FontImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferCopyRegion
	);

	// image barrier for shader read
	subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount   = 1;
	subresourceRange.layerCount   = 1;

	imageMemoryBarrier.image     = m_FontImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
	);

	VERIFYVULKANRESULT(vkEndCommandBuffer(cmdBuffer));

	VkSubmitInfo submitInfo;
	ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers    = &cmdBuffer;

	VkFenceCreateInfo fenceCreateInfo;
	ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	fenceCreateInfo.flags = 0;

	VkFence fence = VK_NULL_HANDLE;
	VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &fence));

	VERIFYVULKANRESULT(vkQueueSubmit(m_VulkanDevice->GetTransferQueue()->GetHandle(), 1, &submitInfo, fence));
	VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_uint64));

	vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
	vkDestroyFence(device, fence, nullptr);
	vkDestroyCommandPool(device, commandPool, VULKAN_CPU_ALLOCATOR);
	vkDestroyBuffer(device, stagingBuffer, VULKAN_CPU_ALLOCATOR);
	vkFreeMemory(device, stagingMemory, VULKAN_CPU_ALLOCATOR);
}

void ImageGUIContext::Resize(uint32 width, uint32 height)
{
    ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)(width), (float)(height));
}

bool ImageGUIContext::Update()
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
	bool updateCmdBuffers  = false;

	if (!imDrawData) { 
		return false;
	};

	// Note: Alignment is done inside buffer creation
	VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	VkDeviceSize indexBufferSize  = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	if ((vertexBufferSize == 0) || (indexBufferSize == 0)) {
		return false;
	}
	
	// Vertex buffer
	if ((m_VertexBuffer.buffer == VK_NULL_HANDLE) || (m_VertexCount != imDrawData->TotalVtxCount)) {
		m_VertexCount = imDrawData->TotalVtxCount;
		m_VertexBuffer.Unmap();
		m_VertexBuffer.Destroy();
		CreateBuffer(m_VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
		m_VertexBuffer.Map();
		updateCmdBuffers = true;
	}

	// Index buffer
	VkDeviceSize indexSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);
	if ((m_IndexBuffer.buffer == VK_NULL_HANDLE) || (m_IndexCount < imDrawData->TotalIdxCount)) {
		m_IndexCount = imDrawData->TotalIdxCount;
		m_IndexBuffer.Unmap();
		m_IndexBuffer.Destroy();
		CreateBuffer(m_IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
		m_IndexBuffer.Map();
		updateCmdBuffers = true;
	}

	// Upload data
	ImDrawVert* vtxDst = (ImDrawVert*)m_VertexBuffer.mapped;
	ImDrawIdx* idxDst  = (ImDrawIdx*)m_IndexBuffer.mapped;

	for (int n = 0; n < imDrawData->CmdListsCount; n++) {
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];
		memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtxDst += cmd_list->VtxBuffer.Size;
		idxDst += cmd_list->IdxBuffer.Size;
	}

	m_VertexBuffer.Flush();
	m_IndexBuffer.Flush();

	return updateCmdBuffers;
}

void ImageGUIContext::CreateBuffer(UIBuffer& buffer, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size)
{
	VkDevice device = m_VulkanDevice->GetInstanceHandle();

	VkBufferCreateInfo bufferCreateInfo;
	ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
	bufferCreateInfo.size  = size;
	bufferCreateInfo.usage = usageFlags;
	VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &buffer.buffer));
    
	uint32 memoryTypeIndex = 0;
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAllocInfo;
	ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

	vkGetBufferMemoryRequirements(device, buffer.buffer, &memReqs);
	m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex);
	memAllocInfo.allocationSize  = memReqs.size;
	memAllocInfo.memoryTypeIndex = memoryTypeIndex;
	VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_FontMemory));

	VERIFYVULKANRESULT(vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0));

	buffer.device    = device;
	buffer.size      = memAllocInfo.allocationSize;
	buffer.alignment = memReqs.alignment;
}

void ImageGUIContext::Draw(const VkCommandBuffer commandBuffer)
{
    ImDrawData* imDrawData = ImGui::GetDrawData();
	int32_t vertexOffset   = 0;
	int32_t indexOffset    = 0;

	if ((!imDrawData) || (imDrawData->CmdListsCount == 0)) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();

	VkDeviceSize offsets[1] = { 0 };

	m_PushData.scale.Set(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
	m_PushData.translate.Set(-1.0f, -1.0f);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
	vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &m_PushData);
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &m_VertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

	for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
	{
		const ImDrawList* cmdList = imDrawData->CmdLists[i];

		for (int32_t j = 0; j < cmdList->CmdBuffer.Size; ++j)
		{
			const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];
			VkRect2D scissorRect;
			scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
			scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
			scissorRect.extent.width  = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
			scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
			vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += cmdList->VtxBuffer.Size;
	}
}

bool ImageGUIContext::Header(const char* caption)
{
    return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}

bool ImageGUIContext::CheckBox(const char* caption, bool& value)
{
    bool res = ImGui::Checkbox(caption, &value);
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::CheckBox(const char* caption, int32& value)
{
    bool val = value == 1;
	bool res = ImGui::Checkbox(caption, &val);
	value = val;
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::InputFloat(const char* caption, float& value, float step, uint32 precision)
{
	bool res = ImGui::InputFloat(caption, &value, step, step * 10.0f, precision);
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::SliderFloat(const char* caption, float& value, float min, float max)
{
    bool res = ImGui::SliderFloat(caption, &value, min, max);
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::SliderInt(const char* caption, int32& value, int32 min, int32 max)
{
	bool res = ImGui::SliderInt(caption, &value, min, max);
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::ComboBox(const char* caption, int32& itemindex, const std::vector<std::string>& items)
{
    if (items.empty()) {
		return false;
	}

	std::vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); i++) {
		charitems.push_back(items[i].c_str());
	}

	uint32_t itemCount = static_cast<uint32_t>(charitems.size());
	bool res = ImGui::Combo(caption, &itemindex, &charitems[0], itemCount, itemCount);
	if (res) { 
		m_Updated = true;
	}
	return res;
}

bool ImageGUIContext::Button(const char* caption)
{
    bool res = ImGui::Button(caption);
	if (res) { 
		m_Updated = true;
	};
	return res;
}

void ImageGUIContext::Text(const char* formatstr, ...)
{
    va_list args;
	va_start(args, formatstr);
	ImGui::TextV(formatstr, args);
	va_end(args);
}
