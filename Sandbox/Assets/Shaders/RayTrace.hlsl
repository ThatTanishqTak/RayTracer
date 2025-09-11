cbuffer uParams : register(b0)
{
    int m_MaxDepth; // Maximum recursion depth
    int m_SamplesPerPixel; // Jittered samples per pixel
};

float3 uCameraPos; // Camera origin passed from CPU

// Material identifiers for shading choices.
static const uint MATERIAL_LAMBERTIAN = 0;
static const uint MATERIAL_METAL = 1;
static const uint MATERIAL_DIELECTRIC = 2;

// Material properties flattened alongside each primitive.
struct MaterialData
{
    float3 m_Albedo; // Base color
    float m_Fuzz; // Metal fuzziness or padding for Lambertian
    float m_RefIdx; // Index of refraction for dielectrics
    uint m_Type; // Material type selector
};

// GPU representation of a sphere primitive.
struct Sphere
{
    float3 m_Center; // Sphere center
    float m_Radius; // Sphere radius
    MaterialData m_Material; // Surface material parameters
};

// Axis aligned bounding box stored in the BVH nodes.
struct BVHFlatNode
{
    float3 m_BoundsMin; // Minimum corner of the node's bounds
    float3 m_BoundsMax; // Maximum corner of the node's bounds
    uint m_Left; // Index of the left child or primitive
    uint m_Right; // Index of the right child when not a leaf
    uint m_Primitive; // Primitive index for leaves
    uint m_IsLeaf; // Non-zero if this node is a leaf
};

struct Ray
{
    float3 m_Origin; // Ray origin in world space
    float3 m_Direction; // Normalized ray direction
};

struct HitRecord
{
    float3 m_Point; // Point of intersection
    float3 m_Normal; // Surface normal at the hit point
    float m_Target; // Distance along the ray
    bool m_FrontFace; // True when hitting the front face
    uint m_MaterialType; // Material selector
    float3 m_Albedo; // Surface color for shading
    float m_Fuzz; // Metal fuzziness
    float m_RefIdx; // Refractive index
};

StructuredBuffer<BVHFlatNode> g_Nodes : register(t0); // Flattened BVH nodes
StructuredBuffer<Sphere> g_Spheres : register(t1); // Primitive data
RWTexture2D<float4> g_Output : register(u0); // Render target

// --------------------------------------------------------------------------------------
// Random number generation utilities
// --------------------------------------------------------------------------------------
uint LCG(inout uint l_State)
{
    l_State = l_State * 1664525u + 1013904223u; // Linear congruential generator
    return l_State;
}

float RandomFloat(inout uint l_State)
{
    return (LCG(l_State) & 0x00FFFFFFu) / 16777216.0f;
}

float3 RandomInUnitSphere(inout uint l_State)
{
    // Rejection sampling inside the unit sphere
    float3 l_P;
    do
    {
        l_P = float3(RandomFloat(l_State) * 2.0f - 1.0f,
                     RandomFloat(l_State) * 2.0f - 1.0f,
                     RandomFloat(l_State) * 2.0f - 1.0f);
    }
    while (dot(l_P, l_P) >= 1.0f);

    return l_P;
}

float3 RandomUnitVector(inout uint l_State)
{
    return normalize(RandomInUnitSphere(l_State));
}

float Reflectance(float l_Cosine, float l_RefIdx)
{
    // Schlick's approximation for reflectance
    float l_R0 = (1.0f - l_RefIdx) / (1.0f + l_RefIdx);
    l_R0 = l_R0 * l_R0;
    return l_R0 + (1.0f - l_R0) * pow(1.0f - l_Cosine, 5.0f);
}

// --------------------------------------------------------------------------------------
// Intersection helpers
// --------------------------------------------------------------------------------------
bool HitAABB(float3 l_Min, float3 l_Max, Ray l_Ray, float l_tMin, float l_tMax)
{
    // Slab test across each axis
    for (int it_Axis = 0; it_Axis < 3; ++it_Axis)
    {
        float l_InvD = 1.0f / l_Ray.m_Direction[it_Axis];
        float l_T0 = (l_Min[it_Axis] - l_Ray.m_Origin[it_Axis]) * l_InvD;
        float l_T1 = (l_Max[it_Axis] - l_Ray.m_Origin[it_Axis]) * l_InvD;
        if (l_InvD < 0.0f)
        {
            float l_Temp = l_T0;
            l_T0 = l_T1;
            l_T1 = l_Temp;
        }
        l_tMin = max(l_T0, l_tMin);
        l_tMax = min(l_T1, l_tMax);
        if (l_tMax <= l_tMin)
        {
            return false;
        }
    }

    return true;
}

bool HitSphere(Ray l_Ray, Sphere l_Sphere, float l_tMin, float l_tMax, out HitRecord l_Record)
{
    l_Record = (HitRecord) 0;

    float3 l_OC = l_Ray.m_Origin - l_Sphere.m_Center;
    float l_A = dot(l_Ray.m_Direction, l_Ray.m_Direction);
    float l_HalfB = dot(l_OC, l_Ray.m_Direction);
    float l_C = dot(l_OC, l_OC) - l_Sphere.m_Radius * l_Sphere.m_Radius;
    float l_Discriminant = l_HalfB * l_HalfB - l_A * l_C;
    if (l_Discriminant < 0.0f)
    {
        return false;
    }

    float l_SqrtD = sqrt(l_Discriminant);
    float l_Root = (-l_HalfB - l_SqrtD) / l_A;
    if (l_Root < l_tMin || l_Root > l_tMax)
    {
        l_Root = (-l_HalfB + l_SqrtD) / l_A;
        if (l_Root < l_tMin || l_Root > l_tMax)
        {
            return false;
        }
    }

    l_Record.m_Target = l_Root;
    l_Record.m_Point = l_Ray.m_Origin + l_Root * l_Ray.m_Direction;
    float3 l_OutwardNormal = (l_Record.m_Point - l_Sphere.m_Center) / l_Sphere.m_Radius;
    l_Record.m_FrontFace = dot(l_Ray.m_Direction, l_OutwardNormal) < 0.0f;
    l_Record.m_Normal = l_Record.m_FrontFace ? l_OutwardNormal : -l_OutwardNormal;
    l_Record.m_MaterialType = l_Sphere.m_Material.m_Type;
    l_Record.m_Albedo = l_Sphere.m_Material.m_Albedo;
    l_Record.m_Fuzz = l_Sphere.m_Material.m_Fuzz;
    l_Record.m_RefIdx = l_Sphere.m_Material.m_RefIdx;

    return true;
}

// Traverse the flattened BVH to locate the closest intersection.
bool HitWorld(Ray l_Ray, out HitRecord l_Record)
{
    l_Record = (HitRecord) 0;
    bool l_Hit = false;
    float l_Closest = 1e30f;

    uint l_Stack[64]; // Fixed-size stack for iterative traversal
    int l_Top = 0;
    l_Stack[0] = 0; // Root node index

    while (l_Top >= 0)
    {
        uint l_NodeIndex = l_Stack[l_Top--];
        BVHFlatNode l_Node = g_Nodes[l_NodeIndex];

        if (!HitAABB(l_Node.m_BoundsMin, l_Node.m_BoundsMax, l_Ray, 0.001f, l_Closest))
        {
            continue; // Skip nodes not intersected
        }

        if (l_Node.m_IsLeaf != 0)
        {
            Sphere l_Sphere = g_Spheres[l_Node.m_Primitive];
            HitRecord l_TestRecord;
            if (HitSphere(l_Ray, l_Sphere, 0.001f, l_Closest, l_TestRecord))
            {
                l_Hit = true;
                l_Closest = l_TestRecord.m_Target;
                l_Record = l_TestRecord;
            }
        }
        else
        {
            // Push children indices for further processing
            l_Stack[++l_Top] = l_Node.m_Left;
            l_Stack[++l_Top] = l_Node.m_Right;
        }
    }

    return l_Hit;
}

// Compute scattering based on material properties.
bool Scatter(Ray l_Ray, HitRecord l_Record, inout uint l_State, out float3 l_Attenuation, out Ray l_Scattered)
{
    if (l_Record.m_MaterialType == MATERIAL_LAMBERTIAN)
    {
        float3 l_ScatterDir = l_Record.m_Normal + RandomUnitVector(l_State);
        if (dot(l_ScatterDir, l_ScatterDir) < 1e-8f)
        {
            l_ScatterDir = l_Record.m_Normal; // Handle degenerate scatter direction
        }
        l_Scattered.m_Origin = l_Record.m_Point;
        l_Scattered.m_Direction = l_ScatterDir;
        l_Attenuation = l_Record.m_Albedo;
        return true;
    }
    else if (l_Record.m_MaterialType == MATERIAL_METAL)
    {
        float3 l_Reflected = reflect(normalize(l_Ray.m_Direction), l_Record.m_Normal);
        float3 l_ScatterDir = l_Reflected + l_Record.m_Fuzz * RandomInUnitSphere(l_State);
        l_Scattered.m_Origin = l_Record.m_Point;
        l_Scattered.m_Direction = l_ScatterDir;
        l_Attenuation = l_Record.m_Albedo;
        return dot(l_ScatterDir, l_Record.m_Normal) > 0.0f;
    }
    else if (l_Record.m_MaterialType == MATERIAL_DIELECTRIC)
    {
        l_Attenuation = float3(1.0f, 1.0f, 1.0f);
        float l_RefractionRatio = l_Record.m_FrontFace ? (1.0f / l_Record.m_RefIdx) : l_Record.m_RefIdx;
        float3 l_UnitDir = normalize(l_Ray.m_Direction);
        float l_CosTheta = min(dot(-l_UnitDir, l_Record.m_Normal), 1.0f);
        float l_SinTheta = sqrt(1.0f - l_CosTheta * l_CosTheta);
        bool l_CannotRefract = l_RefractionRatio * l_SinTheta > 1.0f;
        float3 l_Direction;
        if (l_CannotRefract || Reflectance(l_CosTheta, l_RefractionRatio) > RandomFloat(l_State))
        {
            l_Direction = reflect(l_UnitDir, l_Record.m_Normal);
        }
        else
        {
            l_Direction = refract(l_UnitDir, l_Record.m_Normal, l_RefractionRatio);
        }
        l_Scattered.m_Origin = l_Record.m_Point;
        l_Scattered.m_Direction = l_Direction;
        return true;
    }

    // TODO: Support additional material types
    l_Attenuation = float3(0.0f, 0.0f, 0.0f);
    l_Scattered = l_Ray;
    return false;
}

// Iterative ray color calculation using a loop to avoid recursion.
float3 RayColor(Ray l_Ray, inout uint l_State)
{
    float3 l_Attenuation = float3(1.0f, 1.0f, 1.0f);
    Ray l_CurrentRay = l_Ray;
    for (int it_Depth = 0; it_Depth < m_MaxDepth; ++it_Depth)
    {
        HitRecord l_Record;
        if (HitWorld(l_CurrentRay, l_Record))
        {
            float3 l_StepAttenuation;
            Ray l_Scattered;
            if (Scatter(l_CurrentRay, l_Record, l_State, l_StepAttenuation, l_Scattered))
            {
                l_Attenuation *= l_StepAttenuation;
                l_CurrentRay = l_Scattered;
                continue; // Trace the scattered ray
            }
            return float3(0.0f, 0.0f, 0.0f); // Absorb when scattering fails
        }

        float3 l_UnitDirection = normalize(l_CurrentRay.m_Direction);
        float l_T = 0.5f * (l_UnitDirection.y + 1.0f);
        float3 l_Start = float3(1.0f, 1.0f, 1.0f);
        float3 l_End = float3(0.5f, 0.7f, 1.0f);
        float3 l_Background = lerp(l_Start, l_End, l_T);
        return l_Attenuation * l_Background;
    }

    return float3(0.0f, 0.0f, 0.0f);
}

// --------------------------------------------------------------------------------------
// Main compute shader entry point
// --------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void CSMain(uint3 l_DispatchThreadID : SV_DispatchThreadID)
{
    uint l_Width, l_Height;
    g_Output.GetDimensions(l_Width, l_Height); // Query render target size

    uint l_Seed = l_DispatchThreadID.x + l_DispatchThreadID.y * l_Width; // Per-thread RNG seed
    float3 l_AccumColor = float3(0.0f, 0.0f, 0.0f);

    // Multi-sample each pixel for basic anti-aliasing.
    for (int it_Sample = 0; it_Sample < m_SamplesPerPixel; ++it_Sample)
    {
        float2 l_Jitter = float2(RandomFloat(l_Seed), RandomFloat(l_Seed));
        float2 l_UV = (float2(l_DispatchThreadID.xy) + l_Jitter) / float2(l_Width, l_Height);
        float3 l_Dir = normalize(float3(l_UV * 2.0f - 1.0f, -1.0f));
        Ray l_Ray;
        l_Ray.m_Origin = uCameraPos;
        l_Ray.m_Direction = l_Dir;
        l_AccumColor += RayColor(l_Ray, l_Seed);
    }

    float3 l_Color = l_AccumColor / m_SamplesPerPixel;
    // TODO: Consider gamma correction and tone mapping
    g_Output[l_DispatchThreadID.xy] = float4(l_Color, 1.0f);
}

// TODO: Integrate advanced sampling strategies and more material models in the future.