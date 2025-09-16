#version 430
// Converted from HLSL to GLSL for OpenGL/Vulkan style compute pipelines.
// Resource bindings mirror original HLSL register assignments for clarity.
// Future enhancement: add cross-compilation to support SPIR-V or DXC paths.
// TODO: Implement shader cross-compilation for multi-API support.

// Converted from HLSL constant buffers to GLSL uniform blocks.
// Binding points mirror the original register indices (b0 and b1).
layout(std140, binding = 0) uniform uParams
{
int m_MaxDepth; // Maximum recursion depth
int m_SamplesPerPixel; // Jittered samples per pixel
};

// Camera parameters passed each frame. Maintains 16-byte alignment for std140.
layout(std140, binding = 1) uniform uCamera
{
vec3 uCameraPos; // Camera origin supplied by the CPU
float m_Padding; // Padding for std140 alignment
};

// Material identifiers for shading choices.
const uint MATERIAL_LAMBERTIAN = 0;
const uint MATERIAL_METAL = 1;
const uint MATERIAL_DIELECTRIC = 2;
const uint MATERIAL_EMISSIVE = 3; // Light-emitting surface

// Plain-old-data material used by the GPU.
struct MaterialGPU
{
    vec3 m_Albedo; // Base color
    float m_Fuzz; // Metal fuzziness
    vec3 m_Emission; // Emissive radiance
    float m_RefIdx; // Index of refraction
    uint m_Type; // Material type selector
    vec3 m_Padding; // Alignment padding
    // TODO: Optimize packing once material parameters stabilize
};

// GPU representation of a sphere primitive.
struct Sphere
{
    vec3 m_Center; // Sphere center
    float m_Radius; // Sphere radius
    uint m_MaterialIndex; // Index into material buffer
    vec3 m_Padding; // Alignment padding
};

// Axis aligned bounding box stored in the BVH nodes.
struct BVHFlatNode
{
    vec3 m_BoundsMin; // Minimum corner of the node's bounds
    vec3 m_BoundsMax; // Maximum corner of the node's bounds
    uint m_Left; // Index of the left child or primitive
    uint m_Right; // Index of the right child when not a leaf
    uint m_Primitive; // Primitive index for leaves
    uint m_IsLeaf; // Non-zero if this node is a leaf
};

struct Ray
{
    vec3 m_Origin; // Ray origin in world space
    vec3 m_Direction; // Normalized ray direction
};

struct HitRecord
{
    vec3 m_Point; // Point of intersection
    vec3 m_Normal; // Surface normal at the hit point
    float m_Target; // Distance along the ray
    bool m_FrontFace; // True when hitting the front face
    uint m_MaterialType; // Material selector
    vec3 m_Albedo; // Surface color for shading
    float m_Fuzz; // Metal fuzziness
    vec3 m_Emission; // Emissive radiance
    float m_RefIdx; // Refractive index
};

// Helper used to create a zero-initialized hit record without relying on casts.
HitRecord CreateEmptyHitRecord()
{
    // All fields are explicitly set so the caller starts from a known state.
    HitRecord l_Record; // Local hit record with defined defaults
    l_Record.m_Point = vec3(0.0f);          // Intersection point
    l_Record.m_Normal = vec3(0.0f);         // Surface normal at hit
    l_Record.m_Target = 0.0f;               // Distance along the ray
    l_Record.m_FrontFace = false;           // Front-face test
    l_Record.m_MaterialType = 0u;           // Material identifier
    l_Record.m_Albedo = vec3(0.0f);         // Base color
    l_Record.m_Fuzz = 0.0f;                 // Metal fuzziness
    l_Record.m_Emission = vec3(0.0f);       // Emissive radiance
    l_Record.m_RefIdx = 0.0f;               // Index of refraction
    return l_Record;
}

// Structured buffers become shader storage buffers in GLSL.
layout(std430, binding = 0)
buffer NodesBuffer
{
    BVHFlatNode g_Nodes[]; // Flattened BVH nodes
};

layout(std430, binding = 1)
buffer SpheresBuffer
{
    Sphere g_Spheres[]; // Primitive data
};

layout(std430, binding = 2)
buffer MaterialsBuffer
{
    MaterialGPU g_Materials[]; // Material parameters
};

// UAV in HLSL translates to an image in GLSL. Uses rgba32f to match float4.
layout(binding = 0, rgba32f)
uniform image2D g_Output; // Render target

// --------------------------------------------------------------------------------------
// Random number generation utilities
// --------------------------------------------------------------------------------------
uint PCGHash(uint l_State)
{
    l_State = l_State * 747796405u + 2891336453u;
    uint l_Word = ((l_State >> ((l_State >> 28u) + 4u)) ^ l_State) * 277803737u;
    return (l_Word >> 22u) ^ l_Word;
}

float RandomFloat(inout uint l_State)
{
    l_State = PCGHash(l_State);
    return l_State / 4294967296.0f;
}

uint SeedFromPixel(uvec2 l_Pixel)
{
    uint l_State = l_Pixel.x * 1973u + l_Pixel.y * 9277u + 8917u;
    return PCGHash(l_State);
}


vec3 RandomInUnitSphere(inout uint l_State)
{
    // Rejection sampling inside the unit sphere
    vec3 l_P;
    do
    {
        l_P = vec3(RandomFloat(l_State) * 2.0f - 1.0f,
                     RandomFloat(l_State) * 2.0f - 1.0f,
                     RandomFloat(l_State) * 2.0f - 1.0f);
    }
    while (dot(l_P, l_P) >= 1.0f);

    return l_P;
}

vec3 RandomUnitVector(inout uint l_State)
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
bool HitAABB(vec3 l_Min, vec3 l_Max, Ray l_Ray, float l_tMin, float l_tMax)
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
    // Initialize the output record to a known zero state instead of casting
    // an integer literal. This prevents GPU compilers from flagging a type
    // mismatch and keeps the fields predictable.
    l_Record = CreateEmptyHitRecord();

    vec3 l_OC = l_Ray.m_Origin - l_Sphere.m_Center;
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
    vec3 l_OutwardNormal = (l_Record.m_Point - l_Sphere.m_Center) / l_Sphere.m_Radius;
    l_Record.m_FrontFace = dot(l_Ray.m_Direction, l_OutwardNormal) < 0.0f;
    l_Record.m_Normal = l_Record.m_FrontFace ? l_OutwardNormal : -l_OutwardNormal;
    // Fetch material properties using the referenced index.
    MaterialGPU l_Mat = g_Materials[l_Sphere.m_MaterialIndex];
    l_Record.m_MaterialType = l_Mat.m_Type;
    l_Record.m_Albedo = l_Mat.m_Albedo;
    l_Record.m_Fuzz = l_Mat.m_Fuzz;
    l_Record.m_Emission = l_Mat.m_Emission;
    l_Record.m_RefIdx = l_Mat.m_RefIdx;

    return true;
}

// Traverse the flattened BVH to locate the closest intersection.
bool HitWorld(Ray l_Ray, out HitRecord l_Record)
{
    // Clear the hit record so traversal starts with no prior hit data.
    l_Record = CreateEmptyHitRecord();

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
bool Scatter(Ray l_Ray, HitRecord l_Record, inout uint l_State, out vec3 l_Attenuation, out Ray l_Scattered)
{
    if (l_Record.m_MaterialType == MATERIAL_LAMBERTIAN)
    {
        vec3 l_ScatterDir = l_Record.m_Normal + RandomUnitVector(l_State);
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
        vec3 l_Reflected = reflect(normalize(l_Ray.m_Direction), l_Record.m_Normal);
        vec3 l_ScatterDir = l_Reflected + l_Record.m_Fuzz * RandomInUnitSphere(l_State);
        l_Scattered.m_Origin = l_Record.m_Point;
        l_Scattered.m_Direction = l_ScatterDir;
        l_Attenuation = l_Record.m_Albedo;
        return dot(l_ScatterDir, l_Record.m_Normal) > 0.0f;
    }
    else if (l_Record.m_MaterialType == MATERIAL_DIELECTRIC)
    {
        l_Attenuation = vec3(1.0f, 1.0f, 1.0f);
        float l_RefractionRatio = l_Record.m_FrontFace ? (1.0f / l_Record.m_RefIdx) : l_Record.m_RefIdx;
        vec3 l_UnitDir = normalize(l_Ray.m_Direction);
        float l_CosTheta = min(dot(-l_UnitDir, l_Record.m_Normal), 1.0f);
        float l_SinTheta = sqrt(1.0f - l_CosTheta * l_CosTheta);
        bool l_CannotRefract = l_RefractionRatio * l_SinTheta > 1.0f;
        vec3 l_Direction;
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

    else if (l_Record.m_MaterialType == MATERIAL_EMISSIVE)
    {
        l_Attenuation = l_Record.m_Emission; // Return emission and terminate
        return false;
    }

    // TODO: Add more sophisticated materials (e.g., subsurface, glossy).
    l_Attenuation = vec3(0.0f, 0.0f, 0.0f);
    l_Scattered = l_Ray;
    return false;
}

// Iterative ray color calculation using a loop to avoid recursion.
vec3 RayColor(Ray l_Ray, inout uint l_State)
{
    vec3 l_Attenuation = vec3(1.0f, 1.0f, 1.0f);
    Ray l_CurrentRay = l_Ray;
    // Respect the configured recursion limit supplied by the CPU via uParams.
    for (int it_Depth = 0; it_Depth < uParams.m_MaxDepth; ++it_Depth)
    {
        HitRecord l_Record;
        if (HitWorld(l_CurrentRay, l_Record))
        {
            vec3 l_StepAttenuation;
            Ray l_Scattered;
            if (Scatter(l_CurrentRay, l_Record, l_State, l_StepAttenuation, l_Scattered))
            {
                l_Attenuation *= l_StepAttenuation;
                l_CurrentRay = l_Scattered;
                continue; // Trace the scattered ray
            }
            return l_Attenuation * l_StepAttenuation; // Terminate with emission or absorption
        }

        vec3 l_UnitDirection = normalize(l_CurrentRay.m_Direction);
        float l_T = 0.5f * (l_UnitDirection.y + 1.0f);
        vec3 l_Start = vec3(1.0f, 1.0f, 1.0f);
        vec3 l_End = vec3(0.5f, 0.7f, 1.0f);
        vec3 l_Background = mix(l_Start, l_End, l_T);
        return l_Attenuation * l_Background;
    }

    return vec3(0.0f, 0.0f, 0.0f);
}

// --------------------------------------------------------------------------------------
// Main compute shader entry point
// --------------------------------------------------------------------------------------
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
    uvec3 l_DispatchThreadID = gl_GlobalInvocationID;
    ivec2 l_Size = imageSize(g_Output); // Query render target size
    int l_Width = l_Size.x;
    int l_Height = l_Size.y;

    uint l_Seed = SeedFromPixel(l_DispatchThreadID.xy); // Deterministic per-pixel seed
    vec3 l_AccumColor = vec3(0.0f, 0.0f, 0.0f);

    // Multi-sample each pixel for basic anti-aliasing.
    // Jitter multiple samples per pixel as instructed by uParams.
    for (int it_Sample = 0; it_Sample < uParams.m_SamplesPerPixel; ++it_Sample)
    {
        vec2 l_Jitter = vec2(RandomFloat(l_Seed), RandomFloat(l_Seed));
        vec2 l_UV = (vec2(l_DispatchThreadID.xy) + l_Jitter) / vec2(l_Width, l_Height);
        vec3 l_Dir = normalize(vec3(l_UV * 2.0f - 1.0f, -1.0f));
        Ray l_Ray;
        l_Ray.m_Origin = uCameraPos;
        l_Ray.m_Direction = l_Dir;
        l_AccumColor += RayColor(l_Ray, l_Seed);
    }

    // Average the accumulated color by the total samples per pixel from uParams.
    vec3 l_Color = l_AccumColor / uParams.m_SamplesPerPixel;
    // Apply simple Reinhard tone mapping to compress HDR values.
    vec3 l_ToneMapped = l_Color / (l_Color + vec3(1.0f));
    // Gamma correction approximating an sRGB display response.
    vec3 l_Gamma = pow(l_ToneMapped, vec3(1.0f / 2.2f));
    imageStore(g_Output, ivec2(l_DispatchThreadID.xy), vec4(l_Gamma, 1.0f));
    // TODO: Expose tone-mapping parameters for artistic control.
}

// TODO: Integrate advanced sampling strategies and more material models in the future.