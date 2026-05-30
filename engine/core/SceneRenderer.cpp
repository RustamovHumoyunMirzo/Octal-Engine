#include "SceneRenderer.h"
#include "Scene.h"
#include "Renderer.h"
#include "RendererMath.h"
#include "Mesh.h"

#include <algorithm>
#include <vector>
#include <cmath>

namespace OctalEngine
{
    namespace
    {
        constexpr float pi = 3.14159265358979323846f;

        std::vector<Vertex> cubeVertices()
        {
            return {
                {-1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
                {1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f},
                {1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f},
                {-1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                {1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                {1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
            };
        }

        std::vector<std::uint16_t> cubeIndices()
        {
            return {
                0, 1, 2, 2, 3, 0,
                4, 6, 5, 6, 4, 7,
                0, 4, 5, 5, 1, 0,
                3, 2, 6, 6, 7, 3,
                1, 5, 6, 6, 2, 1,
                0, 3, 7, 7, 4, 0,
            };
        }

        const Mesh& cubeMesh()
        {
            static Mesh mesh(cubeVertices(), cubeIndices());
            return mesh;
        }

        const Mesh& planeMesh()
        {
            static Mesh mesh(
                {
                    {-1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
                    {1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f},
                    {1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f},
                    {-1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
                },
                {0, 1, 2, 2, 3, 0});
            return mesh;
        }

        const Mesh& sphereMesh()
        {
            static Mesh mesh = []() {
                constexpr int rings = 12;
                constexpr int segments = 24;
                std::vector<Vertex> vertices;
                std::vector<std::uint16_t> indices;

                for (int ring = 0; ring <= rings; ++ring)
                {
                    const float v = static_cast<float>(ring) / static_cast<float>(rings);
                    const float theta = v * pi;
                    const float y = std::cos(theta);
                    const float radius = std::sin(theta);

                    for (int segment = 0; segment <= segments; ++segment)
                    {
                        const float u = static_cast<float>(segment) / static_cast<float>(segments);
                        const float phi = u * pi * 2.0f;
                        const float x = std::cos(phi) * radius;
                        const float z = std::sin(phi) * radius;
                        vertices.push_back({x, y, z, x, y, z, u, v});
                    }
                }

                for (int ring = 0; ring < rings; ++ring)
                {
                    for (int segment = 0; segment < segments; ++segment)
                    {
                        const std::uint16_t a = static_cast<std::uint16_t>(ring * (segments + 1) + segment);
                        const std::uint16_t b = static_cast<std::uint16_t>(a + segments + 1);
                        indices.insert(indices.end(), {a, b, static_cast<std::uint16_t>(a + 1),
                                                       static_cast<std::uint16_t>(a + 1), b, static_cast<std::uint16_t>(b + 1)});
                    }
                }

                return Mesh(vertices, indices);
            }();

            return mesh;
        }

        const Mesh* meshFor(PrimitiveType primitive)
        {
            switch (primitive)
            {
            case PrimitiveType::Cube:
                return &cubeMesh();
            case PrimitiveType::Sphere:
                return &sphereMesh();
            case PrimitiveType::Plane:
                return &planeMesh();
            case PrimitiveType::Capsule:
            case PrimitiveType::Cylinder:
                return &sphereMesh();
            case PrimitiveType::Custom:
                return nullptr;
            default:
                return nullptr;
            }
        }

        Mat4 transformMatrix(const TransformComponent& transform)
        {
            const Quaternion& q = transform.rotation;
            const Vec3& s = transform.scale;
            const Vec3& t = transform.position;

            const float xx = q.x * q.x;
            const float yy = q.y * q.y;
            const float zz = q.z * q.z;
            const float xy = q.x * q.y;
            const float xz = q.x * q.z;
            const float yz = q.y * q.z;
            const float wx = q.w * q.x;
            const float wy = q.w * q.y;
            const float wz = q.w * q.z;

            Mat4 matrix;
            matrix.values = {
                (1.0f - 2.0f * (yy + zz)) * s.x,
                (2.0f * (xy + wz)) * s.x,
                (2.0f * (xz - wy)) * s.x,
                0.0f,

                (2.0f * (xy - wz)) * s.y,
                (1.0f - 2.0f * (xx + zz)) * s.y,
                (2.0f * (yz + wx)) * s.y,
                0.0f,

                (2.0f * (xz + wy)) * s.z,
                (2.0f * (yz - wx)) * s.z,
                (1.0f - 2.0f * (xx + yy)) * s.z,
                0.0f,

                t.x,
                t.y,
                t.z,
                1.0f};
            return matrix;
        }

        Mat4 multiply(const Mat4& a, const Mat4& b)
        {
            Mat4 result;

            for (int column = 0; column < 4; ++column)
            {
                for (int row = 0; row < 4; ++row)
                {
                    result.values[column * 4 + row] =
                        a.values[0 * 4 + row] * b.values[column * 4 + 0] +
                        a.values[1 * 4 + row] * b.values[column * 4 + 1] +
                        a.values[2 * 4 + row] * b.values[column * 4 + 2] +
                        a.values[3 * 4 + row] * b.values[column * 4 + 3];
                }
            }

            return result;
        }

        Vec3 normalize(Vec3 value)
        {
            const float length = std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
            if (length <= 0.0001f)
            {
                return {0.35f, -1.0f, 0.35f};
            }

            return {value.x / length, value.y / length, value.z / length};
        }

        Mat4 shadowProjectionMatrix(const Vec3& lightDirection, float planeY)
        {
            Vec3 direction = normalize(lightDirection);
            if (std::abs(direction.y) <= 0.0001f)
            {
                direction.y = -1.0f;
                direction = normalize(direction);
            }

            const float dxOverDy = direction.x / direction.y;
            const float dzOverDy = direction.z / direction.y;

            Mat4 matrix;
            matrix.values = {
                1.0f, 0.0f, 0.0f, 0.0f,
                -dxOverDy, 0.0f, -dzOverDy, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                dxOverDy * planeY, planeY, dzOverDy * planeY, 1.0f};
            return matrix;
        }
    }

    void SceneRenderer::render(Scene* scene, Renderer* internalRenderer)
    {
        if (scene == nullptr || internalRenderer == nullptr)
        {
            return;
        }

        internalRenderer->beginFrame();

        Object cameraObject = scene->primaryCamera();
        if (cameraObject.valid())
        {
            const auto* camera = cameraObject.getComponent<CameraComponent>();
            if (camera != nullptr)
            {
                const TransformComponent cameraTransform = cameraObject.worldTransform();
                const Vec3 forward = cameraObject.forward();
                internalRenderer->setCamera({
                    cameraTransform.position.x,
                    cameraTransform.position.y,
                    cameraTransform.position.z,
                    cameraTransform.position.x - forward.x,
                    cameraTransform.position.y - forward.y,
                    cameraTransform.position.z - forward.z,
                    0.0f,
                    1.0f,
                    0.0f,
                    camera->fov,
                    camera->nearPlane,
                    camera->farPlane,
                    camera->isOrthographic});
            }
        }

        bool hasShadowLight = false;
        Vec3 shadowDirection{0.35f, -1.0f, 0.35f};

        scene->each<TransformComponent, LightComponent>(
            [&](Object lightObject, const TransformComponent&, const LightComponent& light) {
                if (hasShadowLight || light.type != LightType::Directional || !light.castShadows)
                {
                    return;
                }

                const Vec3 forward = lightObject.forward();
                shadowDirection = normalize({-forward.x, -std::abs(forward.y) - 0.25f, -forward.z});
                hasShadowLight = true;
            });

        struct Renderable
        {
            Object object;
            const Mesh* mesh = nullptr;
            bool castShadows = false;
            bool receiveShadows = true;
            int sortingOrder = 0;
            uint32_t renderLayer = 0;
        };

        std::vector<Renderable> renderables;

        scene->each<TransformComponent, MeshGeometry, MeshRendererComponent>(
            [&](Object object, const TransformComponent&, const MeshGeometry& geometry, const MeshRendererComponent& rendererComponent) {
                if (!rendererComponent.visible)
                {
                    return;
                }

                const Mesh* mesh = meshFor(geometry.primitive);
                if (mesh == nullptr)
                {
                    return;
                }

                renderables.push_back({
                    object,
                    mesh,
                    rendererComponent.castShadows,
                    rendererComponent.receiveShadows,
                    rendererComponent.sortingOrder,
                    rendererComponent.renderLayer
                });
            });

        std::sort(renderables.begin(), renderables.end(),
            [](const Renderable& a, const Renderable& b) {
                if (a.renderLayer != b.renderLayer)
                    return a.renderLayer < b.renderLayer;
                return a.sortingOrder < b.sortingOrder;
            });

        for (const Renderable& renderable : renderables)
        {
            internalRenderer->drawMesh(*renderable.mesh, transformMatrix(renderable.object.worldTransform()));
        }

        if (hasShadowLight)
        {
            const Mat4 projection = shadowProjectionMatrix(shadowDirection, -0.96f);
            const RenderColor shadowColor{0.03f, 0.03f, 0.03f, 0.65f};

            for (const Renderable& renderable : renderables)
            {
                if (!renderable.castShadows)
                {
                    continue;
                }

                internalRenderer->drawMesh(
                    *renderable.mesh,
                    multiply(projection, transformMatrix(renderable.object.worldTransform())),
                    shadowColor);
            }
        }

        internalRenderer->endFrame();
    }
}
