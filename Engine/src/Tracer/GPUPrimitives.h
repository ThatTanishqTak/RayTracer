#pragma once

#include <raylib.h>
#include <cstdint>

namespace Engine
{
    /**
     * \brief Plain-old-data material representation for GPU upload.
     * Future expansion: support texture indices and additional BRDF parameters.
     */
    struct MaterialGPU
    {
        Vector3 m_Albedo{};       ///< Base color of the surface.
        float m_Fuzz{};           ///< Surface roughness for metal materials.
        Vector3 m_Emission{};     ///< Emissive color for light sources.
        float m_RefIdx{};         ///< Index of refraction for dielectrics.
        std::uint32_t m_Type{};   ///< Material type selector.
        Vector3 m_Padding{};      ///< Padding for 16-byte alignment.
        // TODO: Pack parameters more tightly once GPU layout is refactored.
    };

    /**
     * \brief Simplified sphere primitive used by the GPU.
     * Stores an index into a separate material buffer.
     */
    struct SphereGPU
    {
        Vector3 m_Center{};          ///< Sphere center position.
        float m_Radius{};            ///< Sphere radius.
        std::uint32_t m_MaterialIndex{}; ///< Index referencing MaterialGPU array.
        Vector3 m_Padding{};         ///< Padding for 16-byte alignment.
    };
}