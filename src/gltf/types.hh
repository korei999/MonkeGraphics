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
enum class COMPONENT_TYPE : adt::u32
{
    BYTE = 5120,
    UNSIGNED_BYTE = 5121,
    SHORT = 5122,
    UNSIGNED_SHORT = 5123,
    INT = 5124,
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
    adt::Vec<int> vNodes {};
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

union Type
{
    adt::f32 SCALAR;
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
    enum class TYPE { SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4 };

    /* */

    int bufferViewI {}; /* The index of the bufferView. */
    int byteOffset {}; /* The offset relative to the start of the buffer view in bytes. This MUST be a multiple of the size of the component datatype. */
    COMPONENT_TYPE eComponentType {}; /* REQUIRED. The datatype of the accessor’s components. */
    int count {}; /* REQUIRED The number of elements referenced by this accessor, not to be confused with the number of bytes or number of components. */
    Type uMax {}; /* number [1-16]. Maximum value of each component in this accessor. */
    Type uMin {}; /* number [1-16]. Minimum value of each component in this accessor. */
    TYPE eType {}; /* REQUIRED. Specifies if the accessor’s elements are scalars, vectors, or matrices. */
};


/* Each node can contain an array called children that contains the indices of its child nodes.
 * So each node is one element of a hierarchy of nodes,
 * and together they define the structure of the scene as a scene graph. */
struct Node
{
    enum class TRANSFORMATION_TYPE : adt::u8 { NONE, MATRIX, ANIMATION };

    /* */

    adt::String sName {}; /* The user-defined name of this object. */
    int cameraI {}; /* The index of the camera referenced by this node. */
    adt::Vec<int> vChildren {}; /* The indices of this node’s children. */
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
    int meshI = -1; /* The index of the mesh in this node. */
    int skinI = -1; /* The index of the skin referenced by this node. */
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

    /* */

    /* REQUIRED. An array of animation channels. An animation channel combines an animation sampler with a target property being animated.
     * Different channels of the same animation MUST NOT have the same targets. */
    adt::Vec<Channel> vChannels {};
    /* REQUIRED. An array of animation samplers.
     * An animation sampler combines timestamps with a sequence of output values and defines an interpolation algorithm. */
    adt::Vec<Sampler> vSamplers {};
    adt::String sName {}; /* The user-defined name of this object. */
};

struct CameraPersp
{
    adt::f32 aspectRatio {};
    adt::f32 yfov {};
    adt::f32 zfar {};
    adt::f32 znear {};
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
    TARGET eTarget = TARGET::NONE;
};

struct Image
{
    adt::String sUri {};
};

/* When the node contains skin, all mesh.primitives MUST contain JOINTS_0 and WEIGHTS_0 attributes.  */
struct Primitive
{
    /* match real gl macros */
    enum class TYPE : adt::u8
    {
        POINTS = 0,
        LINES = 1,
        LINE_LOOP = 2,
        LINE_STRIP = 3,
        TRIANGLES = 4,
        TRIANGLE_STRIP = 5,
        TRIANGLE_FAN = 6
    };

    /* */

    struct
    {
        /* TODO: could be TEXCOORD_0, TEXCOORD_1..., JOINTS_*, WEIGHTS_* ... */
        int NORMAL = -1;
        int POSITION = -1;
        int TANGENT = -1;
        int TEXCOORD_0 = -1;
        int JOINTS_0 = -1;
        int WEIGHTS_0 = -1;
    } attributes {}; /* each value is the index of the accessor containing attribute’s data. */
    int indicesI = -1; /* The index of the accessor that contains the vertex indices, drawElements() when defined and drawArrays() otherwise. */
    int materialI = -1; /* The index of the material to apply to this primitive when rendering */
    TYPE eMode = TYPE::TRIANGLES;

    void* pData {};
};

/* A mesh primitive defines the geometry data of the object using its attributes dictionary.
 * This geometry data is given by references to accessor objects that contain the data of vertex attributes. */
struct Mesh
{
    adt::Vec<Primitive> vPrimitives {}; /* REQUIRED */
    adt::String sName {};
};

struct Texture
{
    int sourceI = -1; /* The index of the image used by this texture. */
    int samplerI = -1; /* The index of the sampler used by this texture. When undefined, a sampler with repeat wrapping and auto filtering SHOULD be used. */
};

struct TextureInfo
{
    int index = -1; /* REQUIRED The index of the texture. */
};

struct NormalTextureInfo
{
    int index = -1; /* REQUIRED */
    adt::f32 scale {};
};

struct PbrMetallicRoughness
{
    adt::math::V4 baseColorFactor {};
    TextureInfo baseColorTexture {};
};

struct Material
{
    adt::String sName {};
    PbrMetallicRoughness pbrMetallicRoughness {};
    NormalTextureInfo normalTexture {};
};

struct Skin
{
    adt::Vec<int> vJoints {}; /* REQUIRED. Indices of skeleton nodes, used as joints in this skin. */
    adt::String sName {}; /* The user-defined name of this object. */
    int inverseBindMatricesI = -1; /* The index of the accessor containing the floating-point 4x4 inverse-bind matrices. */
    int skeleton = -1; /* The index of the node used as a skeleton root. */
};

} /* namespace gltf */

namespace adt::print
{

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Animation::Channel::Target::PATH_TYPE x)
{
    constexpr adt::StringView aMap[] {
        "TRANSLATION", "ROTATION", "SCALE", "WEIGHTS",
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Animation::Sampler::INTERPOLATION_TYPE x)
{
    constexpr adt::StringView aMap[] {
        "LINEAR", "STEP", "CUBICSPLINE",
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Node::TRANSFORMATION_TYPE x)
{
    constexpr adt::StringView aMap[] {
        "NONE", "MATRIX", "ANIMATION",
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::COMPONENT_TYPE x)
{
    const char* nts;
    switch (x)
    {
        default: nts = "UNKNOWN"; break;

        case gltf::COMPONENT_TYPE::BYTE: nts = "BYTE"; break;
        case gltf::COMPONENT_TYPE::UNSIGNED_BYTE: nts = "UNSIGNED_BYTE"; break;
        case gltf::COMPONENT_TYPE::SHORT: nts = "SHORT"; break;
        case gltf::COMPONENT_TYPE::UNSIGNED_SHORT: nts = "UNSIGNED_SHORT"; break;
        case gltf::COMPONENT_TYPE::UNSIGNED_INT: nts = "UNSIGNED_INT"; break;
        case gltf::COMPONENT_TYPE::FLOAT: nts = "FLOAT"; break;
    }

    return formatToContext(ctx, fmtArgs, nts);
}


inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::TARGET x)
{
    const char* nts;
    switch (x)
    {
        default: nts = "UNKNOWN"; break;

        case gltf::TARGET::NONE: nts = "NONE"; break;
        case gltf::TARGET::ARRAY_BUFFER: nts = "ARRAY_BUFFER"; break;
        case gltf::TARGET::ELEMENT_ARRAY_BUFFER: nts = "ELEMENT_ARRAY_BUFFER"; break;
    }

    return formatToContext(ctx, fmtArgs, nts);
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Accessor::TYPE x)
{
    constexpr adt::StringView aMap[] {
        "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

inline ssize
formatToContext(Context ctx, FormatArgs fmtArgs, const gltf::Primitive::TYPE x)
{
    constexpr adt::StringView aMap[] {
        "POINTS", "LINES", "LINE_LOOP", "LINE_STRIP", "TRIANGLES", "TRIANGLE_STRIP", "TRIANGLE_FAN",
    };

    return formatToContext(ctx, fmtArgs, aMap[static_cast<int>(x)]);
}

} /* namespace adt::print */
