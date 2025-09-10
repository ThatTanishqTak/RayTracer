cbuffer RayTraceParams : register(b0)
{
    int uMaxDepth; // Maximum recursion depth
    int uSamplesPerPixel; // Number of jittered samples per pixel
};

struct Ray
{
    float3 m_Origin;
    float3 m_Direction;
};

struct HitRecord
{
    float3 m_Normal;
    float m_Target;
};

// Placeholder functions for scene intersection and material scattering.
// Future improvement: integrate BVH traversal and material system.
bool HitWorld(Ray l_Ray, out HitRecord l_Record)
{
    l_Record = (HitRecord) 0;
    
    return false;
}

bool Scatter(Ray l_Ray, HitRecord l_Record, out float3 l_Attenuation, out Ray l_Scattered)
{
    l_Attenuation = float3(1.0f, 1.0f, 1.0f);
    l_Scattered = l_Ray;
    
    return false;
}

// Iterative ray color calculation using a loop to avoid recursion.
float3 RayColor(Ray l_Ray)
{
    float3 l_Attenuation = float3(1.0f, 1.0f, 1.0f);
    Ray l_CurrentRay = l_Ray;
    for (int it_Depth = 0; it_Depth < uMaxDepth; ++it_Depth)
    {
        HitRecord l_Record;
        if (HitWorld(l_CurrentRay, l_Record))
        {
            float3 l_StepAttenuation;
            Ray l_Scattered;
            if (Scatter(l_CurrentRay, l_Record, l_StepAttenuation, l_Scattered))
            {
                l_Attenuation *= l_StepAttenuation;
                l_CurrentRay = l_Scattered;
            
                continue; // Continue tracing the scattered ray
            }
        
            return float3(0.0f, 0.0f, 0.0f); // Absorb when scattering fails
        }
        
        float3 l_UnitDirection = normalize(l_CurrentRay.m_Direction);
        float l_Target = 0.5f * (l_UnitDirection.y + 1.0f);
        float3 l_Start = float3(1.0f, 1.0f, 1.0f);
        float3 l_End = float3(0.5f, 0.7f, 1.0f);
        float3 l_Background = lerp(l_Start, l_End, l_Target);
        
        return l_Attenuation * l_Background; // Return accumulated color with background
    }
    
    return float3(0.0f, 0.0f, 0.0f);
}

[numthreads(8, 8, 1)]
void CSMain(uint3 l_DispatchThreadID : SV_DispatchThreadID)
{
    // Compute shader entry point. Future improvement: implement full ray tracing here.
    Ray l_Ray;
    l_Ray.m_Origin = float3(0.0f, 0.0f, 0.0f);
    l_Ray.m_Direction = float3(0.0f, 0.0f, -1.0f);
    
    float3 l_Color = RayColor(l_Ray);
    // TODO: Write l_Color to an output texture
}