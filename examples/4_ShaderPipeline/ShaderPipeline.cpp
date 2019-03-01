#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VertexBuffer.h"
#include "Vulkan/IndexBuffer.h"
#include "Vulkan/VulkanMemory.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Loader/tiny_obj_loader.h"

#include "spirv_glsl.hpp"
#include "spirv_parser.hpp"
#include "spirv_reflect.hpp"

#include <vector>
#include <fstream>
#include <istream>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

using namespace spv;
using namespace spirv_cross;
using namespace std;

static void print_spec_constants(const Compiler &compiler)
{
    auto spec_constants = compiler.get_specialization_constants();
    fprintf(stderr, "Specialization constants\n");
    fprintf(stderr, "==================\n\n");
    for (auto &c : spec_constants)
        fprintf(stderr, "ID: %u, Spec ID: %u\n", c.id, c.constant_id);
    fprintf(stderr, "==================\n\n");
}

static void print_capabilities_and_extensions(const Compiler &compiler)
{
    fprintf(stderr, "Capabilities\n");
    fprintf(stderr, "============\n");
    for (auto &capability : compiler.get_declared_capabilities())
        fprintf(stderr, "Capability: %u\n", static_cast<unsigned>(capability));
    fprintf(stderr, "============\n\n");
    
    fprintf(stderr, "Extensions\n");
    fprintf(stderr, "============\n");
    for (auto &ext : compiler.get_declared_extensions())
        fprintf(stderr, "Extension: %s\n", ext.c_str());
    fprintf(stderr, "============\n\n");
}

static void print_resources(const Compiler &compiler, const char *tag, const vector<Resource> &resources)
{
    fprintf(stderr, "%s\n", tag);
    fprintf(stderr, "=============\n\n");
    bool print_ssbo = !strcmp(tag, "ssbos");
    
    for (auto &res : resources)
    {
        auto &type = compiler.get_type(res.type_id);
        
        if (print_ssbo && compiler.buffer_is_hlsl_counter_buffer(res.id))
            continue;
        
        // If we don't have a name, use the fallback for the type instead of the variable
        // for SSBOs and UBOs since those are the only meaningful names to use externally.
        // Push constant blocks are still accessed by name and not block name, even though they are technically Blocks.
        bool is_push_constant = compiler.get_storage_class(res.id) == StorageClassPushConstant;
        bool is_block = compiler.get_decoration_bitset(type.self).get(DecorationBlock) ||
        compiler.get_decoration_bitset(type.self).get(DecorationBufferBlock);
        bool is_sized_block = is_block && (compiler.get_storage_class(res.id) == StorageClassUniform ||
                                           compiler.get_storage_class(res.id) == StorageClassUniformConstant);
        uint32_t fallback_id = !is_push_constant && is_block ? res.base_type_id : res.id;
        
        uint32_t block_size = 0;
        uint32_t runtime_array_stride = 0;
        if (is_sized_block)
        {
            auto &base_type = compiler.get_type(res.base_type_id);
            block_size = uint32_t(compiler.get_declared_struct_size(base_type));
            runtime_array_stride = uint32_t(compiler.get_declared_struct_size_runtime_array(base_type, 1) -
                                            compiler.get_declared_struct_size_runtime_array(base_type, 0));
        }
        
        Bitset mask;
        if (print_ssbo)
            mask = compiler.get_buffer_block_flags(res.id);
        else
            mask = compiler.get_decoration_bitset(res.id);
        
        string array;
        for (auto arr : type.array)
            array = join("[", arr ? convert_to_string(arr) : "", "]") + array;
        
        fprintf(stderr, " ID %03u : %s%s", res.id,
                !res.name.empty() ? res.name.c_str() : compiler.get_fallback_name(fallback_id).c_str(), array.c_str());
        
        if (mask.get(DecorationLocation))
            fprintf(stderr, " (Location : %u)", compiler.get_decoration(res.id, DecorationLocation));
        if (mask.get(DecorationDescriptorSet))
            fprintf(stderr, " (Set : %u)", compiler.get_decoration(res.id, DecorationDescriptorSet));
        if (mask.get(DecorationBinding))
            fprintf(stderr, " (Binding : %u)", compiler.get_decoration(res.id, DecorationBinding));
        if (mask.get(DecorationInputAttachmentIndex))
            fprintf(stderr, " (Attachment : %u)", compiler.get_decoration(res.id, DecorationInputAttachmentIndex));
        if (mask.get(DecorationNonReadable))
            fprintf(stderr, " writeonly");
        if (mask.get(DecorationNonWritable))
            fprintf(stderr, " readonly");
        if (is_sized_block)
        {
            fprintf(stderr, " (BlockSize : %u bytes)", block_size);
            if (runtime_array_stride)
                fprintf(stderr, " (Unsized array stride: %u bytes)", runtime_array_stride);
        }
        
        uint32_t counter_id = 0;
        if (print_ssbo && compiler.buffer_get_hlsl_counter_buffer(res.id, counter_id))
            fprintf(stderr, " (HLSL counter buffer ID: %u)", counter_id);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "=============\n\n");
}

static const char *execution_model_to_str(spv::ExecutionModel model)
{
    switch (model)
    {
        case spv::ExecutionModelVertex:
            return "vertex";
        case spv::ExecutionModelTessellationControl:
            return "tessellation control";
        case ExecutionModelTessellationEvaluation:
            return "tessellation evaluation";
        case ExecutionModelGeometry:
            return "geometry";
        case ExecutionModelFragment:
            return "fragment";
        case ExecutionModelGLCompute:
            return "compute";
        default:
            return "???";
    }
}

static void print_resources(const Compiler &compiler, const ShaderResources &res)
{
    auto &modes = compiler.get_execution_mode_bitset();
    
    print_spec_constants(compiler);
    print_capabilities_and_extensions(compiler);
    
    fprintf(stderr, "Entry points:\n");
    auto entry_points = compiler.get_entry_points_and_stages();
    for (auto &e : entry_points)
        fprintf(stderr, "  %s (%s)\n", e.name.c_str(), execution_model_to_str(e.execution_model));
    fprintf(stderr, "\n");
    
    fprintf(stderr, "Execution modes:\n");
    modes.for_each_bit([&](uint32_t i) {
        auto mode = static_cast<ExecutionMode>(i);
        uint32_t arg0 = compiler.get_execution_mode_argument(mode, 0);
        uint32_t arg1 = compiler.get_execution_mode_argument(mode, 1);
        uint32_t arg2 = compiler.get_execution_mode_argument(mode, 2);
        
        switch (static_cast<ExecutionMode>(i))
        {
            case ExecutionModeInvocations:
                fprintf(stderr, "  Invocations: %u\n", arg0);
                break;
                
            case ExecutionModeLocalSize:
                fprintf(stderr, "  LocalSize: (%u, %u, %u)\n", arg0, arg1, arg2);
                break;
                
            case ExecutionModeOutputVertices:
                fprintf(stderr, "  OutputVertices: %u\n", arg0);
                break;
                
#define CHECK_MODE(m)                  \
case ExecutionMode##m:             \
fprintf(stderr, "  %s\n", #m); \
break
                CHECK_MODE(SpacingEqual);
                CHECK_MODE(SpacingFractionalEven);
                CHECK_MODE(SpacingFractionalOdd);
                CHECK_MODE(VertexOrderCw);
                CHECK_MODE(VertexOrderCcw);
                CHECK_MODE(PixelCenterInteger);
                CHECK_MODE(OriginUpperLeft);
                CHECK_MODE(OriginLowerLeft);
                CHECK_MODE(EarlyFragmentTests);
                CHECK_MODE(PointMode);
                CHECK_MODE(Xfb);
                CHECK_MODE(DepthReplacing);
                CHECK_MODE(DepthGreater);
                CHECK_MODE(DepthLess);
                CHECK_MODE(DepthUnchanged);
                CHECK_MODE(LocalSizeHint);
                CHECK_MODE(InputPoints);
                CHECK_MODE(InputLines);
                CHECK_MODE(InputLinesAdjacency);
                CHECK_MODE(Triangles);
                CHECK_MODE(InputTrianglesAdjacency);
                CHECK_MODE(Quads);
                CHECK_MODE(Isolines);
                CHECK_MODE(OutputPoints);
                CHECK_MODE(OutputLineStrip);
                CHECK_MODE(OutputTriangleStrip);
                CHECK_MODE(VecTypeHint);
                CHECK_MODE(ContractionOff);
                
            default:
                break;
        }
    });
    fprintf(stderr, "\n");
    
    print_resources(compiler, "subpass inputs", res.subpass_inputs);
    print_resources(compiler, "inputs", res.stage_inputs);
    print_resources(compiler, "outputs", res.stage_outputs);
    print_resources(compiler, "textures", res.sampled_images);
    print_resources(compiler, "separate images", res.separate_images);
    print_resources(compiler, "separate samplers", res.separate_samplers);
    print_resources(compiler, "images", res.storage_images);
    print_resources(compiler, "ssbos", res.storage_buffers);
    print_resources(compiler, "ubos", res.uniform_buffers);
    print_resources(compiler, "push", res.push_constant_buffers);
    print_resources(compiler, "counters", res.atomic_counters);
}

struct PLSArg
{
    PlsFormat format;
    string name;
};

struct Remap
{
    string src_name;
    string dst_name;
    unsigned components;
};

struct VariableTypeRemap
{
    string variable_name;
    string new_variable_type;
};

struct InterfaceVariableRename
{
    StorageClass storageClass;
    uint32_t location;
    string variable_name;
};

class OBJLoaderMode : public AppModeBase
{
public:
	OBJLoaderMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_CmdLine(cmdLine)
		, m_Ready(false)
	{

	}

	virtual ~OBJLoaderMode()
	{

	}

	virtual void PreInit() override
	{
		std::string assetsPath = m_CmdLine[0];
		int32 length = 0;
		for (size_t i = 0; i < assetsPath.size(); ++i)
		{
			if (assetsPath[i] == '\\')
			{
				assetsPath[i] = '/';
			}
		}
		for (size_t i = assetsPath.size() - 1; i >= 0; --i)
		{
			if (assetsPath[i] == '/')
			{
				break;
			}
			length += 1;
		}
		m_AssetsPath = assetsPath.substr(0, assetsPath.size() - length);
	}

	virtual void Init() override
	{
		m_VulkanRHI = GetVulkanRHI();
		m_Device = m_VulkanRHI->GetDevice()->GetInstanceHandle();

		LoadOBJ();
        CreateFences();
		CreateSemaphores();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		CreateDescriptorSet();
		CreatePipelines();
		SetupCommandBuffers();

		m_Ready = true;
	}

	virtual void Exist() override
	{
        DestroyFences();
		DestorySemaphores();
		DestroyDescriptorSetLayout();
		DestroyDescriptorPool();
		DestroyPipelines();
		DestroyUniformBuffers();
		UnLoadOBJ();
	}

	virtual void Loop() override
	{
		if (!m_Ready)
		{
			return;
		}
		Draw();
	}

private:

	struct GPUBuffer
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	typedef GPUBuffer UBOBuffer;

	struct UBOData
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	std::string GetRealFilePath(const std::string& filepath)
	{
		return m_AssetsPath + "../../../examples/" + filepath;;
	}

	bool ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize)
	{
		std::string finalPath = m_AssetsPath + "../../../examples/" + filepath;

		FILE* file = fopen(finalPath.c_str(), "rb");
		if (!file)
		{
			MLOGE("File not found :%s", filepath.c_str());
			return false;
		}

		fseek(file, 0, SEEK_END);
		dataSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (dataSize <= 0)
		{
			fclose(file);
			MLOGE("File has no data :%s", filepath.c_str());
			return false;
		}

		dataPtr = new uint8[dataSize];
		fread(dataPtr, 1, dataSize, file);
		fclose(file);
		
		return true;
	}

	VkShaderModule LoadSPIPVShader(std::string filepath)
	{
		uint32 dataSize = 0;
		uint8* dataPtr = nullptr;
		ReadFile(filepath, dataPtr, dataSize);
		
        std::vector<uint32_t> spirv_binary(dataSize / 4);
        std::memcpy(spirv_binary.data(), dataPtr, dataSize);
        
        spirv_cross::CompilerGLSL glsl(std::move(spirv_binary));
        spirv_cross::ShaderResources resources = glsl.get_shader_resources();
        print_resources(glsl, resources);
        
		VkShaderModuleCreateInfo moduleCreateInfo;
		ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		moduleCreateInfo.codeSize = dataSize;
		moduleCreateInfo.pCode = (uint32_t*)dataPtr;

		VkShaderModule shaderModule;
		VERIFYVULKANRESULT(vkCreateShaderModule(m_Device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
		delete[] dataPtr;

		return shaderModule;
	}

	void Draw()
	{
		UpdateUniformBuffers();

		VkPipelineStageFlags waitStageMask = m_VulkanRHI->GetStageMask();
		std::shared_ptr<VulkanQueue> gfxQueue = m_VulkanRHI->GetDevice()->GetGraphicsQueue();
		std::shared_ptr<VulkanQueue> presentQueue = m_VulkanRHI->GetDevice()->GetPresentQueue();
		std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
		std::shared_ptr<VulkanSwapChain> swapChain = m_VulkanRHI->GetSwapChain();

		VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
		m_CurrentBackBuffer = swapChain->AcquireImageIndex(&presentCompleteSemaphore);
        
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        fenceMgr.WaitForFence(m_Fences[m_CurrentBackBuffer], MAX_uint64);
        fenceMgr.ResetFence(m_Fences[m_CurrentBackBuffer]);
        
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitStageMask;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[m_CurrentBackBuffer];
		submitInfo.commandBufferCount = 1;
        
		VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]->GetHandle()));
        
		swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
	}
	
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = m_VulkanRHI->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = m_VulkanRHI->GetSwapChain()->GetWidth();
		renderPassBeginInfo.renderArea.extent.height = m_VulkanRHI->GetSwapChain()->GetHeight();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;

		std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
		std::vector<VkFramebuffer> frameBuffers = m_VulkanRHI->GetFrameBuffers();
		for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkViewport viewport = {};
			viewport.width = (float)renderPassBeginInfo.renderArea.extent.width;
			viewport.height = (float)renderPassBeginInfo.renderArea.extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.extent.width = (uint32)viewport.width;
			scissor.extent.height = (uint32)viewport.height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;

			VkDeviceSize offsets[1] = { 0 };

			VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_VertexBuffer->GetVKBuffers().data(), offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(drawCmdBuffers[i], m_IndexBuffer->GetIndexCount(), 1, 0, 0, 1);
			vkCmdEndRenderPass(drawCmdBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}

	void CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet));

		VkWriteDescriptorSet writeDescriptorSet;
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &m_MVPDescriptor;
		writeDescriptorSet.dstBinding = 0;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}

	void CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes = &poolSize;
		descriptorPoolInfo.maxSets = 1;

		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}

	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	// http://xiaopengyou.fun/article/31
	void CreatePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = (
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT
			);
		blendAttachmentState[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = m_VulkanRHI->GetSampleCount();
		multisampleState.pSampleMask = nullptr;

		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0; // Vertex Buffer 0
		vertexInputBinding.stride = 24; // Position + Color
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding = 0;
		vertexInputAttributs[0].location = 0; // triangle.vert : layout (location = 0)
		vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset = 0;
		// color
		vertexInputAttributs[1].binding = 0;
		vertexInputAttributs[1].location = 1; // triangle.vert : layout (location = 1)
		vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset = 12;

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("assets/shaders/3_OBJLoader/obj.vert.spv");
		shaderStages[0].pName = "main";
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("assets/shaders/3_OBJLoader/obj.frag.spv");
		shaderStages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = m_VulkanRHI->GetRenderPass();
		pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_VulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));

		vkDestroyShaderModule(m_Device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		vkDestroyPipeline(m_Device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
	}

	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 1;
		descSetLayoutInfo.pBindings = &layoutBinding;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}

	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	}

	void UpdateUniformBuffers()
	{
        // m_MVPData.model.AppendRotation(1.0f, Vector3::UpVector);
        
		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, 30.0f));
        m_MVPData.view.SetInverse();
        
		uint8_t *pData = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(m_Device, m_MVPBuffer.memory, 0, sizeof(UBOData), 0, (void**)&pData));
		std::memcpy(pData, &m_MVPData, sizeof(UBOData));
		vkUnmapMemory(m_Device, m_MVPBuffer.memory);
	}

	void CreateUniformBuffers()
	{
		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferInfo.size = sizeof(UBOData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &bufferInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.buffer));

		VkMemoryRequirements memReqInfo;
		vkGetBufferMemoryRequirements(m_Device, m_MVPBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		allocInfo.allocationSize = memReqInfo.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &allocInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.memory));

		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_MVPBuffer.buffer, m_MVPBuffer.memory, 0));
		m_MVPDescriptor.buffer = m_MVPBuffer.buffer;
		m_MVPDescriptor.offset = 0;
		m_MVPDescriptor.range = sizeof(UBOData);
        
        m_MVPData.model.SetIdentity();
        m_MVPData.view.SetIdentity();
        m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);
        
		UpdateUniformBuffers();
	}

	void DestroyUniformBuffers()
	{
		vkDestroyBuffer(m_Device, m_MVPBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_MVPBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void LoadOBJ()
	{
		
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		std::string err;
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, GetRealFilePath("assets/models/suzanne.obj").c_str());

		std::vector<float> vertices;
		std::vector<uint16> indices;

		for (size_t s = 0; s < shapes.size(); ++s) 
		{
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) 
			{
				int fv = shapes[s].mesh.num_face_vertices[f];
				for (size_t v = 0; v < fv; v++) 
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					//tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					//tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

					vertices.push_back(vx);
					vertices.push_back(-vy);
					vertices.push_back(vz);

					vertices.push_back(nx);
					vertices.push_back(-ny);
					vertices.push_back(nz);

					indices.push_back(indices.size());
				}
				index_offset += fv;
			}
		}
		
		uint8* vertStreamData = new uint8[vertices.size() * sizeof(float)];
		std::memcpy(vertStreamData, vertices.data(), vertices.size() * sizeof(float));

		VertexStreamInfo streamInfo;
		streamInfo.size = vertices.size() * sizeof(float);
		streamInfo.channelMask = 1 << (int32)VertexAttribute::Position | 1 << (int32)VertexAttribute::Color;

		std::vector<VertexChannelInfo> channels(2);
		channels[0].attribute = VertexAttribute::Position;
		channels[0].format = VertexElementType::VET_Float3;
		channels[0].stream = 0;
		channels[0].offset = 0;
		channels[1].attribute = VertexAttribute::Color;
		channels[1].format = VertexElementType::VET_Float3;
		channels[1].stream = 0;
		channels[1].offset = 12;

		m_VertexBuffer = new VertexBuffer();
		m_VertexBuffer->AddStream(streamInfo, channels, vertStreamData);
		m_VertexBuffer->Upload(GetVulkanRHI());

		// 索引数据
		uint32 indexStreamSize = indices.size() * sizeof(uint16);
		uint8* indexStreamData = new uint8[indexStreamSize];
		std::memcpy(indexStreamData, indices.data(), indexStreamSize);

		m_IndexBuffer = new IndexBuffer(indexStreamData, indexStreamSize, PrimitiveType::PT_TriangleList, VkIndexType::VK_INDEX_TYPE_UINT16);
		m_IndexBuffer->Upload(GetVulkanRHI());
	}

	void UnLoadOBJ()
	{
		m_VertexBuffer->Download(GetVulkanRHI());
		m_IndexBuffer->Download(GetVulkanRHI());

		delete m_VertexBuffer;
		delete m_IndexBuffer;

		m_VertexBuffer = nullptr;
		m_IndexBuffer = nullptr;
	}
    
    void CreateFences()
    {
        m_Fences.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            m_Fences[index] = fenceMgr.CreateFence(true);
        }
    }
    
    void DestroyFences()
    {
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            fenceMgr.WaitAndReleaseFence(m_Fences[index], MAX_int64);
        }
        m_Fences.clear();
    }

	void CreateSemaphores()
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(m_Device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
        
        GetVulkanRHI()->GetDevice()->GetFenceManager();
	}

	void DestorySemaphores()
	{
		vkDestroySemaphore(m_Device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
	}

	std::vector<std::string> m_CmdLine;
	std::string m_AssetsPath;
	bool m_Ready;

	VkDevice m_Device;
	std::shared_ptr<VulkanRHI> m_VulkanRHI;

	VertexBuffer* m_VertexBuffer;
	IndexBuffer* m_IndexBuffer;
	UBOBuffer m_MVPBuffer;
	UBOData m_MVPData;

	VkSemaphore m_RenderComplete;
    std::vector<VulkanFence*> m_Fences;

	VkDescriptorBufferInfo m_MVPDescriptor;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkDescriptorSet m_DescriptorSet;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_Pipeline;
	VkDescriptorPool m_DescriptorPool;

	uint32 m_CurrentBackBuffer;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new OBJLoaderMode(800, 600, "ShaderPipeline", cmdLine);
}