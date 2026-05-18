#include "bgfx_backend.h"

#include "../internal/RenderCommands.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace OctalEngine::RendererBackend
{
    namespace
    {
        struct GpuVertex
        {
            float x = 0.0f;
            float y = 0.0f;
            float z = 0.0f;
            std::uint32_t abgr = 0xffffffff;
        };

        bgfx::VertexLayout makeVertexLayout()
        {
            bgfx::VertexLayout layout;
            layout.begin()
                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
                .end();
            return layout;
        }

        bool isValidVertexBuffer(bgfx::VertexBufferHandle handle)
        {
            return bgfx::isValid(handle);
        }

        bool isValidIndexBuffer(bgfx::IndexBufferHandle handle)
        {
            return bgfx::isValid(handle);
        }

        bool isValidProgram(bgfx::ProgramHandle handle)
        {
            return bgfx::isValid(handle);
        }

        std::uint8_t normalToColor(float value)
        {
            const float normalized = (value * 0.5f) + 0.5f;
            const float clamped = normalized < 0.0f ? 0.0f : (normalized > 1.0f ? 1.0f : normalized);
            return static_cast<std::uint8_t>(clamped * 255.0f);
        }

        std::uint32_t packAbgr(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        {
            return (static_cast<std::uint32_t>(a) << 24) |
                   (static_cast<std::uint32_t>(b) << 16) |
                   (static_cast<std::uint32_t>(g) << 8) |
                   static_cast<std::uint32_t>(r);
        }

        GpuVertex toGpuVertex(const Vertex& vertex)
        {
            return {
                vertex.x,
                vertex.y,
                vertex.z,
                packAbgr(normalToColor(vertex.x), normalToColor(vertex.y), normalToColor(vertex.z))};
        }

        std::vector<GpuVertex> toGpuVertices(const std::vector<Vertex>& vertices)
        {
            std::vector<GpuVertex> result;
            result.reserve(vertices.size());

            for (const Vertex& vertex : vertices)
            {
                result.push_back(toGpuVertex(vertex));
            }

            return result;
        }

        void setDefaultCamera(float aspect)
        {
            if (aspect <= 0.0f)
            {
                aspect = 1.0f;
            }

            const bx::Vec3 eye = {0.0f, 0.0f, -6.0f};
            const bx::Vec3 at = {0.0f, 0.0f, 0.0f};

            float view[16];
            float projection[16];
            bx::mtxLookAt(view, eye, at);
            bx::mtxProj(projection, 60.0f, aspect, 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
            bgfx::setViewTransform(0, view, projection);
        }

        std::vector<char> readBinaryFile(const std::string& path)
        {
            std::ifstream file(path, std::ios::binary);

            if (!file)
            {
                return {};
            }

            return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        }

        const char* shaderDirectory(bgfx::RendererType::Enum renderer)
        {
            switch (renderer)
            {
            case bgfx::RendererType::Direct3D11:
                return "dxbc";
            case bgfx::RendererType::Direct3D12:
                return "dxil";
            case bgfx::RendererType::OpenGLES:
                return "essl";
            case bgfx::RendererType::OpenGL:
                return "glsl";
            case bgfx::RendererType::Metal:
                return "metal";
            case bgfx::RendererType::Vulkan:
                return "spirv";
            case bgfx::RendererType::WebGPU:
                return "wgsl";
            default:
                return nullptr;
            }
        }

        bgfx::ShaderHandle loadShader(const char* directory, const char* name)
        {
            if (directory == nullptr)
            {
                return BGFX_INVALID_HANDLE;
            }

            const std::string path = std::string(OCTAL_BGFX_SHADER_DIR) + "/" + directory + "/" + name + ".bin";
            std::vector<char> data = readBinaryFile(path);

            if (data.empty())
            {
                return BGFX_INVALID_HANDLE;
            }

            const bgfx::Memory* memory = bgfx::copy(data.data(), static_cast<std::uint32_t>(data.size()));
            return bgfx::createShader(memory);
        }
    }

    struct BgfxBackend::Impl
    {
        struct GpuMesh
        {
            bgfx::VertexBufferHandle vertexBuffer = BGFX_INVALID_HANDLE;
            bgfx::IndexBufferHandle indexBuffer = BGFX_INVALID_HANDLE;
            std::uint32_t vertexCount = 0;
            std::uint32_t indexCount = 0;
        };

        bool initialized = false;
        bool headless = true;
        bgfx::VertexLayout vertexLayout{};
        bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;
        std::uint16_t width = 1280;
        std::uint16_t height = 720;
        std::unordered_map<std::uint64_t, GpuMesh> meshes;

        GpuMesh& uploadMesh(const Mesh& mesh)
        {
            auto found = meshes.find(mesh.id());
            if (found != meshes.end())
            {
                return found->second;
            }

            GpuMesh gpuMesh;
            gpuMesh.vertexCount = static_cast<std::uint32_t>(mesh.vertices().size());
            gpuMesh.indexCount = static_cast<std::uint32_t>(mesh.indices().size());

            const std::vector<GpuVertex> gpuVertices = toGpuVertices(mesh.vertices());
            const bgfx::Memory* vertexMemory = bgfx::copy(gpuVertices.data(),
                                                          static_cast<std::uint32_t>(gpuVertices.size() * sizeof(GpuVertex)));
            gpuMesh.vertexBuffer = bgfx::createVertexBuffer(vertexMemory, vertexLayout);

            if (!mesh.indices().empty())
            {
                const bgfx::Memory* indexMemory = bgfx::copy(mesh.indices().data(),
                                                             static_cast<std::uint32_t>(mesh.indices().size() * sizeof(std::uint16_t)));
                gpuMesh.indexBuffer = bgfx::createIndexBuffer(indexMemory);
            }

            auto [inserted, _] = meshes.emplace(mesh.id(), gpuMesh);
            return inserted->second;
        }

        void destroyMeshes()
        {
            for (auto& [_, mesh] : meshes)
            {
                if (isValidVertexBuffer(mesh.vertexBuffer))
                {
                    bgfx::destroy(mesh.vertexBuffer);
                }

                if (isValidIndexBuffer(mesh.indexBuffer))
                {
                    bgfx::destroy(mesh.indexBuffer);
                }
            }

            meshes.clear();
        }

        bool createDefaultProgram()
        {
            const char* directory = shaderDirectory(bgfx::getRendererType());
            bgfx::ShaderHandle vertexShader = loadShader(directory, "vs_cubes");
            bgfx::ShaderHandle fragmentShader = loadShader(directory, "fs_cubes");

            if (!bgfx::isValid(vertexShader) || !bgfx::isValid(fragmentShader))
            {
                if (bgfx::isValid(vertexShader))
                {
                    bgfx::destroy(vertexShader);
                }

                if (bgfx::isValid(fragmentShader))
                {
                    bgfx::destroy(fragmentShader);
                }

                return false;
            }

            program = bgfx::createProgram(vertexShader, fragmentShader, true);
            return isValidProgram(program);
        }
    };

    BgfxBackend::BgfxBackend()
        : impl(std::make_unique<Impl>())
    {
    }

    BgfxBackend::~BgfxBackend()
    {
        shutdown();
    }

    std::unique_ptr<IRendererBackend> createRendererBackend()
    {
        return std::make_unique<BgfxBackend>();
    }

    bool BgfxBackend::initialize(const BackendSettings& settings)
    {
        if (impl->initialized)
        {
            return true;
        }

        impl->headless = settings.headless;
        impl->width = static_cast<std::uint16_t>(settings.width > 0 ? settings.width : 1);
        impl->height = static_cast<std::uint16_t>(settings.height > 0 ? settings.height : 1);

        if (settings.headless)
        {
            impl->initialized = true;
            return true;
        }

        if (settings.nativeWindowHandle == nullptr)
        {
            return false;
        }

        bgfx::Init init;
#if defined(_WIN32)
        init.type = bgfx::RendererType::Direct3D11;
#else
        init.type = bgfx::RendererType::Count;
#endif
        init.platformData.nwh = settings.nativeWindowHandle;
        init.resolution.width = impl->width;
        init.resolution.height = impl->height;
        init.resolution.reset = BGFX_RESET_VSYNC;

        if (!bgfx::init(init))
        {
            return false;
        }

        impl->vertexLayout = makeVertexLayout();
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x101820ff, 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, impl->width, impl->height);
        setDefaultCamera(static_cast<float>(impl->width) / static_cast<float>(impl->height));

        if (!impl->createDefaultProgram())
        {
            bgfx::shutdown();
            return false;
        }

        impl->initialized = true;
        return true;
    }

    void BgfxBackend::shutdown()
    {
        if (!impl || !impl->initialized)
        {
            return;
        }

        if (!impl->headless)
        {
            impl->destroyMeshes();
            if (isValidProgram(impl->program))
            {
                bgfx::destroy(impl->program);
                impl->program = BGFX_INVALID_HANDLE;
            }
            bgfx::shutdown();
        }

        impl->initialized = false;
    }

    void BgfxBackend::execute(const RendererInternal::CommandBuffer& commandBuffer)
    {
        if (!impl->initialized)
        {
            return;
        }

        if (impl->headless)
        {
            return;
        }

        bgfx::touch(0);
        bgfx::setViewRect(0, 0, 0, impl->width, impl->height);
        setDefaultCamera(static_cast<float>(impl->width) / static_cast<float>(impl->height));

        for (const auto& command : commandBuffer.commands)
        {
            std::visit([this](const auto& concreteCommand) {
                using Command = std::decay_t<decltype(concreteCommand)>;

                if constexpr (std::is_same_v<Command, RendererInternal::DrawMeshCommand>)
                {
                    Impl::GpuMesh& mesh = impl->uploadMesh(concreteCommand.mesh);
                    bgfx::setTransform(concreteCommand.transform.values.data());
                    bgfx::setVertexBuffer(0, mesh.vertexBuffer);

                    if (isValidIndexBuffer(mesh.indexBuffer))
                    {
                        bgfx::setIndexBuffer(mesh.indexBuffer);
                    }

                    bgfx::setState(BGFX_STATE_WRITE_RGB |
                                   BGFX_STATE_WRITE_A |
                                   BGFX_STATE_WRITE_Z |
                                   BGFX_STATE_DEPTH_TEST_LESS);
                    bgfx::submit(0, impl->program);
                }
                else if constexpr (std::is_same_v<Command, RendererInternal::DrawVerticesCommand>)
                {
                    if (concreteCommand.vertices.empty())
                    {
                        return;
                    }

                    const std::vector<GpuVertex> gpuVertices = toGpuVertices(concreteCommand.vertices);
                    const bgfx::Memory* memory = bgfx::copy(gpuVertices.data(),
                                                            static_cast<std::uint32_t>(gpuVertices.size() * sizeof(GpuVertex)));
                    bgfx::VertexBufferHandle vertexBuffer = bgfx::createVertexBuffer(memory, impl->vertexLayout);
                    bgfx::setTransform(concreteCommand.transform.values.data());
                    bgfx::setVertexBuffer(0, vertexBuffer);
                    bgfx::setState(BGFX_STATE_WRITE_RGB |
                                   BGFX_STATE_WRITE_A |
                                   BGFX_STATE_WRITE_Z |
                                   BGFX_STATE_DEPTH_TEST_LESS);
                    bgfx::submit(0, impl->program);
                    bgfx::destroy(vertexBuffer);
                }
                else if constexpr (std::is_same_v<Command, RendererInternal::SetTransformCommand>)
                {
                    bgfx::setTransform(concreteCommand.transform.values.data());
                }
                else if constexpr (std::is_same_v<Command, RendererInternal::ResizeCommand>)
                {
                    impl->width = static_cast<std::uint16_t>(concreteCommand.width > 0 ? concreteCommand.width : 1);
                    impl->height = static_cast<std::uint16_t>(concreteCommand.height > 0 ? concreteCommand.height : 1);
                    bgfx::reset(impl->width, impl->height, BGFX_RESET_VSYNC);
                    bgfx::setViewRect(0, 0, 0, impl->width, impl->height);
                }
            },
                       command);
        }

        bgfx::frame();
    }
}
