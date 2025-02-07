/* https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#properties-reference */

#pragma once

#include "json/Parser.hh"
#include "adt/math.hh"
#include "adt/String.hh"

namespace gltf
{

/* matches gl macros */
enum class COMPONENT_TYPE
{
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    UNSIGNED_INT = 5125,
    FLOAT = 5126
};

/* matches gl macros */
enum class TARGET
{
    NONE = 0,
    ARRAY_BUFFER = 34962,
    ELEMENT_ARRAY_BUFFER = 34963
};

struct Scene
{
    adt::u32 nodeIdx;
};

/* A buffer represents a block of raw binary data, without an inherent structure or meaning.
 * This data is referred to by a buffer using its uri.
 * This URI may either point to an external file, or be a data URI that encodes the binary data directly in the JSON file. */
struct Buffer
{
    adt::u32 byteLength;
    adt::String sUri;
    adt::String sBin;
};

enum class ACCESSOR_TYPE
{
    SCALAR,
    VEC2,
    VEC3,
    VEC4,
    MAT2,
    MAT3,
    MAT4
};

union Type
{
    adt::f64 SCALAR;
    adt::math::V2 VEC2;
    adt::math::V3 VEC3;
    adt::math::V4 VEC4;
    adt::math::M2 MAT2;
    adt::math::M3 MAT3;
    adt::math::M4 MAT4;
};

/* An accessor object refers to a bufferView and contains properties 
 * that define the type and layout of the data of the bufferView.
 * The raw data of a buffer is structured using bufferView objects and is augmented with data type information using accessor objects.*/
struct Accessor
{
    adt::u32 bufferView;
    adt::u32 byteOffset; /* The offset relative to the start of the buffer view in bytes. This MUST be a multiple of the size of the component datatype. */
    enum COMPONENT_TYPE componentType; /* REQUIRED */
    adt::u32 count; /* REQUIRED The number of elements referenced by this accessor, not to be confused with the number of bytes or number of components. */
    union Type max;
    union Type min;
    enum ACCESSOR_TYPE type; /* REQUIRED */
};


/* Each node can contain an array called children that contains the indices of its child nodes.
 * So each node is one element of a hierarchy of nodes,
 * and together they define the structure of the scene as a scene graph. */
struct Node
{
    adt::String sName;
    adt::u32 camera;
    adt::VecBase<adt::u32> children;
    adt::math::M4 matrix = adt::math::M4Iden();
    adt::ssize mesh = adt::NPOS; /* The index of the mesh in this node. */
    adt::math::V3 translation {};
    adt::math::Qt rotation = adt::math::QtIden();
    adt::math::V3 scale {1.0f, 1.0f, 1.0f};

    Node() = default;
    Node(adt::IAllocator* p) : children(p) {}
};

struct Animation
{
    struct Channel
    {
        struct Target
        {
            enum class PATH_TYPE : adt::u8 { TRANSLATION, ROTATION, SCALE };

            /* The index of the node to animate. When undefined, the animated object MAY be defined by an extension. */
            int node = -1;
            /* REQUIRED. The name of the node’s TRS property to animate, or the "weights" of the Morph Targets it instantiates.
             * For the "translation" property, the values that are provided by the sampler are the translation along the X, Y, and Z axes.
             * For the "rotation" property, the values are a quaternion in the order (x, y, z, w), where w is the scalar.
             * For the "scale" property, the values are the scaling factors along the X, Y, and Z axes. */
            PATH_TYPE ePath {};
        };

        int sampler {}; /* REQUIRED. The index of a sampler in this animation used to compute the value for the target. */
        Target target {}; /* REQUIRED. The descriptor of the animated property. */
        
    };

    struct Sampler
    {
        enum class INTERPOLATION_TYPE : adt::u8 { LINEAR, STEP, CUBICSPLINE };

        int input {}; /* REQUIRED. The index of an accessor containing keyframe timestamps. */
        INTERPOLATION_TYPE interpolation = INTERPOLATION_TYPE::LINEAR;
        int output {}; /* REQUIRED. The index of an accessor, containing keyframe output values. */
    };

    /* REQUIRED. An array of animation channels. An animation channel combines an animation sampler with a target property being animated.
     * Different channels of the same animation MUST NOT have the same targets. */
    adt::VecBase<Channel> channels {};
    /* REQUIRED. An array of animation samplers.
     * An animation sampler combines timestamps with a sequence of output values and defines an interpolation algorithm. */
    adt::VecBase<Sampler> samplers {};
    adt::String sName {}; /* The user-defined name of this object. */
};

struct CameraPersp
{
    adt::f64 aspectRatio;
    adt::f64 yfov;
    adt::f64 zfar;
    adt::f64 znear;
};

struct CameraOrtho
{
    //
};

struct Camera
{
    union
    {
        CameraPersp perspective;
        CameraOrtho orthographic;
    } proj;
    enum
    {
        perspective,
        orthographic
    } type;
};

/* A bufferView represents a “slice” of the data of one buffer.
 * This slice is defined using an offset and a length, in bytes. */
struct BufferView
{
    adt::u32 buffer;
    adt::u32 byteOffset = 0; /* The offset into the buffer in bytes. */
    adt::u32 byteLength;
    adt::u32 byteStride = 0; /* The stride, in bytes, between vertex attributes. When this is not defined, data is tightly packed. */
    enum TARGET target;
};

struct Image
{
    adt::String sUri;
};

/* match real gl macros */
enum class PRIMITIVES
{
    POINTS = 0,
    LINES = 1,
    LINE_LOOP = 2,
    LINE_STRIP = 3,
    TRIANGLES = 4,
    TRIANGLE_STRIP = 5,
    TRIANGLE_FAN = 6
};

struct Primitive
{
    struct
    {
        adt::i32 NORMAL = adt::NPOS32;
        adt::i32 POSITION = adt::NPOS32;
        adt::i32 TEXCOORD_0 = adt::NPOS32;
        adt::i32 TANGENT = adt::NPOS32;
    } attributes; /* each value is the index of the accessor containing attribute’s data. */
    adt::i32 indices = adt::NPOS32; /* The index of the accessor that contains the vertex indices, drawElements() when defined and drawArrays() otherwise. */
    adt::i32 material = adt::NPOS32; /* The index of the material to apply to this primitive when rendering */
    enum PRIMITIVES mode = PRIMITIVES::TRIANGLES;
};

/* A mesh primitive defines the geometry data of the object using its attributes dictionary.
 * This geometry data is given by references to accessor objects that contain the data of vertex attributes. */
struct Mesh
{
    adt::VecBase<Primitive> aPrimitives; /* REQUIRED */
    adt::String sName {};
};

struct Texture
{
    adt::i32 source = adt::NPOS32; /* The index of the image used by this texture. */
    adt::i32 sampler = adt::NPOS32; /* The index of the sampler used by this texture. When undefined, a sampler with repeat wrapping and auto filtering SHOULD be used. */
};

struct TextureInfo
{
    adt::i32 index = adt::NPOS32; /* REQUIRED The index of the texture. */
};

struct NormalTextureInfo
{
    adt::i32 index = adt::NPOS32; /* REQUIRED */
    adt::f64 scale;
};

struct PbrMetallicRoughness
{
    TextureInfo baseColorTexture;
};

struct Material
{
    PbrMetallicRoughness pbrMetallicRoughness;
    NormalTextureInfo normalTexture;
};

struct Model
{
    struct
    {
        json::Object* pScene;
        json::Object* pScenes;
        json::Object* pNodes;
        json::Object* pMeshes;
        json::Object* pCameras;
        json::Object* pBuffers;
        json::Object* pBufferViews;
        json::Object* pAccessors;
        json::Object* pMaterials;
        json::Object* pTextures;
        json::Object* pImages;
        json::Object* pSamplers;
        json::Object* pSkins;
        json::Object* pAnimations;
    } m_jsonObjs {};
    adt::String m_sGenerator {};
    adt::String m_sVersion {};
    int m_defaultSceneIdx {};
    adt::VecBase<Scene> m_vScenes {};
    adt::VecBase<Buffer> m_vBuffers {};
    adt::VecBase<BufferView> m_vBufferViews {};
    adt::VecBase<Accessor> m_vAccessors {};
    adt::VecBase<Mesh> m_vMeshes {};
    adt::VecBase<Texture> m_vTextures {};
    adt::VecBase<Material> m_vMaterials {};
    adt::VecBase<Image> m_vImages {};
    adt::VecBase<Node> m_vNodes {};
    adt::VecBase<Animation> m_vAnimations {};

    adt::String m_sPath {};
    adt::String m_sFile {};

    /* */

    Model() = default;

    /* */

    bool read(adt::IAllocator* pAlloc, const json::Parser& parser, const adt::String svPath); /* clones uri */

    /* */

private:
    void procJSONObjs(adt::IAllocator* pAlloc, const json::Parser& parser);
    void procScenes(adt::IAllocator* pAlloc);
    void procBuffers(adt::IAllocator* pAlloc);
    void procBufferViews(adt::IAllocator* pAlloc);
    void procAccessors(adt::IAllocator* pAlloc);
    void procMeshes(adt::IAllocator* pAlloc);
    void procTexures(adt::IAllocator* pAlloc);
    void procMaterials(adt::IAllocator* pAlloc);
    void procImages(adt::IAllocator* pAlloc);
    void procNodes(adt::IAllocator* pAlloc);
    void procAnimations(adt::IAllocator* pAlloc);
};

inline adt::String
getComponentTypeString(enum COMPONENT_TYPE eType)
{
    switch (eType)
    {
        default:
        case COMPONENT_TYPE::BYTE:
            return "BYTE";
        case COMPONENT_TYPE::UNSIGNED_BYTE:
            return "UNSIGNED_BYTE";
        case COMPONENT_TYPE::SHORT:
            return "SHORT";
        case COMPONENT_TYPE::UNSIGNED_SHORT:
            return "UNSIGNED_SHORT";
        case COMPONENT_TYPE::UNSIGNED_INT:
            return "UNSIGNED_INT";
        case COMPONENT_TYPE::FLOAT:
            return "FLOAT";
    }
}

inline adt::String
getPrimitiveModeString(enum PRIMITIVES ePm)
{
    const char* ss[] {
        "POINTS", "LINES", "LINE_LOOP", "LINE_STRIP", "TRIANGLES", "TRIANGLE_STRIP", "TRIANGLE_FAN"
    };

    return ss[int(ePm)];
}

} /* namespace gltf */
