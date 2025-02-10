/* https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#properties-reference */

#pragma once

#include "adt/Vec.hh"
#include "adt/math.hh"
#include "adt/String.hh"

namespace gltf
{

/* REQUIRED */
struct Asset
{
    adt::String sCopyright {};
    adt::String sGenerator {};
    adt::String sVersion {}; /* REQUIRED. The glTF version in the form of <major>.<minor> that this asset targets. */
    adt::String sMinVersion {};
};

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

struct DefaultScene
{
    int nodeI {};
};

struct Scene
{
    adt::VecBase<int> vNodes {};
    adt::String sName {};
};

/* A buffer represents a block of raw binary data, without an inherent structure or meaning.
 * This data is referred to by a buffer using its uri.
 * This URI may either point to an external file, or be a data URI that encodes the binary data directly in the JSON file. */
struct Buffer
{
    int byteLength {};
    adt::String sUri {};
    adt::String sBin {};
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
    int bufferViewI {};
    int byteOffset {}; /* The offset relative to the start of the buffer view in bytes. This MUST be a multiple of the size of the component datatype. */
    enum COMPONENT_TYPE eComponentType {}; /* REQUIRED */
    int count {}; /* REQUIRED The number of elements referenced by this accessor, not to be confused with the number of bytes or number of components. */
    union Type max {};
    union Type min {};
    enum ACCESSOR_TYPE eType {}; /* REQUIRED */
};


/* Each node can contain an array called children that contains the indices of its child nodes.
 * So each node is one element of a hierarchy of nodes,
 * and together they define the structure of the scene as a scene graph. */
struct Node
{
    enum class TRANSFORMATION_TYPE : adt::u8 { NONE, MATRIX, ANIMATION };

    /* */

    adt::String sName {};
    int camera {};
    adt::VecBase<int> vChildren {};
    /* each node can have a local transform.
     * This transform can be given either by the matrix property of the node or by using the translation, rotation, and scale (TRS) properties. */
    TRANSFORMATION_TYPE eTransformationType {};
    union
    {
        adt::math::M4 matrix;
        struct
        {
            adt::math::V3 translation {};
            adt::math::Qt rotation = adt::math::QtIden();
            adt::math::V3 scale {1.0f, 1.0f, 1.0f};
        } animation {};
    } uTransformation {};
    int meshI = adt::NPOS; /* The index of the mesh in this node. */

    struct
    {
        adt::f32 currTime = 1.0f;
    } extras {}; /* Application-specific data. */
};

struct Animation
{
    struct Channel
    {
        struct Target
        {
            enum class PATH_TYPE : adt::u8 { TRANSLATION, ROTATION, SCALE, WEIGHTS };

            /* The index of the node to animate. When undefined, the animated object MAY be defined by an extension. */
            int nodeI = -1;
            /* REQUIRED. The name of the node’s TRS property to animate, or the "weights" of the Morph Targets it instantiates.
             * For the "translation" property, the values that are provided by the sampler are the translation along the X, Y, and Z axes.
             * For the "rotation" property, the values are a quaternion in the order (x, y, z, w), where w is the scalar.
             * For the "scale" property, the values are the scaling factors along the X, Y, and Z axes. */
            PATH_TYPE ePath {};
        };

        int samplerI {}; /* REQUIRED. The index of a sampler in this animation used to compute the value for the target. */
        Target target {}; /* REQUIRED. The descriptor of the animated property. */
        
    };

    struct Sampler
    {
        enum class INTERPOLATION_TYPE : adt::u8 { LINEAR, STEP, CUBICSPLINE };

        int inputI {}; /* REQUIRED. The index of an accessor containing keyframe timestamps. */
        INTERPOLATION_TYPE eInterpolation = INTERPOLATION_TYPE::LINEAR;
        int outputI {}; /* REQUIRED. The index of an accessor, containing keyframe output values. */
    };

    /* REQUIRED. An array of animation channels. An animation channel combines an animation sampler with a target property being animated.
     * Different channels of the same animation MUST NOT have the same targets. */
    adt::VecBase<Channel> vChannels {};
    /* REQUIRED. An array of animation samplers.
     * An animation sampler combines timestamps with a sequence of output values and defines an interpolation algorithm. */
    adt::VecBase<Sampler> vSamplers {};
    adt::String sName {}; /* The user-defined name of this object. */
};

struct CameraPersp
{
    adt::f64 aspectRatio {};
    adt::f64 yfov {};
    adt::f64 zfar {};
    adt::f64 znear {};
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
    } proj {};
    enum
    {
        perspective,
        orthographic
    } eType {};
};

/* A bufferView represents a “slice” of the data of one buffer.
 * This slice is defined using an offset and a length, in bytes. */
struct BufferView
{
    int bufferI {};
    int byteOffset {}; /* The offset into the buffer in bytes. */
    int byteLength {};
    int byteStride {}; /* The stride, in bytes, between vertex attributes. When this is not defined, data is tightly packed. */
    enum TARGET eTarget {};
};

struct Image
{
    adt::String sUri {};
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
        int NORMAL = adt::NPOS32;
        int POSITION = adt::NPOS32;
        int TEXCOORD_0 = adt::NPOS32;
        int TANGENT = adt::NPOS32;
    } attributes {}; /* each value is the index of the accessor containing attribute’s data. */
    int indicesI = adt::NPOS32; /* The index of the accessor that contains the vertex indices, drawElements() when defined and drawArrays() otherwise. */
    int materialI = adt::NPOS32; /* The index of the material to apply to this primitive when rendering */
    enum PRIMITIVES eMode = PRIMITIVES::TRIANGLES;
};

/* A mesh primitive defines the geometry data of the object using its attributes dictionary.
 * This geometry data is given by references to accessor objects that contain the data of vertex attributes. */
struct Mesh
{
    adt::VecBase<Primitive> vPrimitives {}; /* REQUIRED */
    adt::String sName {};
};

struct Texture
{
    int sourceI = adt::NPOS32; /* The index of the image used by this texture. */
    int samplerI = adt::NPOS32; /* The index of the sampler used by this texture. When undefined, a sampler with repeat wrapping and auto filtering SHOULD be used. */
};

struct TextureInfo
{
    int index = adt::NPOS32; /* REQUIRED The index of the texture. */
};

struct NormalTextureInfo
{
    int index = adt::NPOS32; /* REQUIRED */
    adt::f64 scale {};
};

struct PbrMetallicRoughness
{
    TextureInfo baseColorTexture {};
};

struct Material
{
    PbrMetallicRoughness pbrMetallicRoughness {};
    NormalTextureInfo normalTexture {};
};

} /* namespace gltf */

namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Animation::Channel::Target::PATH_TYPE x)
{
    constexpr adt::String aMap[] {
        "TRANSLATION", "ROTATION", "SCALE", "WEIGHTS"
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

} /* namespace adt::print */
