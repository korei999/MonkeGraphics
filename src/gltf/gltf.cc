#include "gltf.hh"

#include "adt/file.hh"
#include "adt/logs.hh"

using namespace adt;

namespace gltf
{

enum class HASH_CODES : usize
{
    scene = hash::func("scene"),
    scenes = hash::func("scenes"),
    nodes = hash::func("nodes"),
    meshes = hash::func("meshes"),
    cameras = hash::func("cameras"),
    buffers = hash::func("buffers"),
    bufferViews = hash::func("bufferViews"),
    accessors = hash::func("accessors"),
    materials = hash::func("materials"),
    textures = hash::func("textures"),
    images = hash::func("images"),
    samplers = hash::func("samplers"),
    skins = hash::func("skins"),
    animations = hash::func("animations"),
    SCALAR = hash::func("SCALAR"),
    VEC2 = hash::func("VEC2"),
    VEC3 = hash::func("VEC3"),
    VEC4 = hash::func("VEC4"),
    MAT2 = hash::func("MAT2"),
    MAT3 = hash::func("MAT3"),
    MAT4 = hash::func("MAT4")
};

#ifdef D_GLTF

static String
getTargetString(enum TARGET t)
{
    switch (t)
    {
        default:
        case TARGET::NONE:
        return "NONE";

        case TARGET::ARRAY_BUFFER:
        return "ARRAY_BUFFER";

        case TARGET::ELEMENT_ARRAY_BUFFER:
        return "ELEMENT_ARRAY_BUFFER";
    }
}

static String
accessorTypeToString(enum ACCESSOR_TYPE t)
{
    const char* ss[] {
        "SCALAR", "VEC2", "VEC3", "VEC4", "MAT2", "MAT3", "MAT4"
    };
    return ss[(int)(t)];
}

#endif

static enum ACCESSOR_TYPE
stringToAccessorType(String sv)
{
    switch (hash::func(sv))
    {
        default:
        case usize(HASH_CODES::SCALAR):
        return ACCESSOR_TYPE::SCALAR;

        case usize(HASH_CODES::VEC2):
        return ACCESSOR_TYPE::VEC2;

        case usize(HASH_CODES::VEC3):
        return ACCESSOR_TYPE::VEC3;

        case usize(HASH_CODES::VEC4):
        return ACCESSOR_TYPE::VEC4;

        case usize(HASH_CODES::MAT2):
        return ACCESSOR_TYPE::MAT3;

        case usize(HASH_CODES::MAT3):
        return ACCESSOR_TYPE::MAT3;

        case usize(HASH_CODES::MAT4):
        return ACCESSOR_TYPE::MAT4;
    }
}

static union Type
assignUnionType(json::Object* obj, u32 n)
{
    auto& arr = json::getArray(obj);
    union Type type;

    for (u32 i = 0; i < n; i++)
        if (arr[i].tagVal.eTag == json::TAG::LONG)
            type.MAT4.d[i] = f32(json::getLong(&arr[i]));
        else
            type.MAT4.d[i] = f32(json::getDouble(&arr[i]));

    return type;
}

static union Type
accessorTypeToUnionType(enum ACCESSOR_TYPE t, json::Object* obj)
{
    union Type type;

    switch (t)
    {
        default:
        case ACCESSOR_TYPE::SCALAR:
        {
            auto& arr = json::getArray(obj);
            if (arr[0].tagVal.eTag == json::TAG::LONG)
                type.SCALAR = f64(json::getLong(&arr[0]));
            else type.SCALAR = f64(json::getDouble(&arr[0]));
        } break;

        case ACCESSOR_TYPE::VEC2:
        type = assignUnionType(obj, 2);
        break;

        case ACCESSOR_TYPE::VEC3:
        type = assignUnionType(obj, 3);
        break;

        case ACCESSOR_TYPE::VEC4:
        type = assignUnionType(obj, 4);
        break;

        case ACCESSOR_TYPE::MAT2:
        type = assignUnionType(obj, 2*2);
        break;

        case ACCESSOR_TYPE::MAT3:
        type = assignUnionType(obj, 3*3);
        break;

        case ACCESSOR_TYPE::MAT4:
        type = assignUnionType(obj, 4*4);
        break;
    }

    return type;
}

static Animation::Channel::Target::PATH_TYPE
AnimationChannelTargetPathTypeStringToPATH_TYPE(const String svPath)
{
    if (svPath == "translation")
        return Animation::Channel::Target::PATH_TYPE::TRANSLATION;
    else if (svPath == "rotation")
        return Animation::Channel::Target::PATH_TYPE::ROTATION;
    else if (svPath == "scale")
        return Animation::Channel::Target::PATH_TYPE::SCALE;

    LOG_BAD("failed to convert path string\n");
    return {};
}

bool
Model::read(IAllocator* pAlloc, const json::Parser& parser, const String svPath)
{
    m_sPath = svPath.clone(pAlloc);

    procJSONObjs(pAlloc, parser);
    m_defaultSceneIdx = json::getLong(m_jsonObjs.pScene);

    procScenes(pAlloc);
    procBuffers(pAlloc);
    procBufferViews(pAlloc);
    procAccessors(pAlloc);
    procMeshes(pAlloc);
    procTexures(pAlloc);
    procMaterials(pAlloc);
    procImages(pAlloc);
    procNodes(pAlloc);
    procAnimations(pAlloc);

    /* nullify potentially dangling pointers */
    m_jsonObjs = {};

    return true;
}

void
Model::procJSONObjs(IAllocator* pAlloc, const json::Parser& parser)
{
    /* collect all the top level objects */
    for (auto& node : parser.getRoot())
    {
        switch (hash::func(node.svKey))
        {
            default: break;

            case usize(HASH_CODES::scene):
            m_jsonObjs.pScene = &node;
            break;

            case usize(HASH_CODES::scenes):
            m_jsonObjs.pScenes = &node;
            break;

            case usize(HASH_CODES::nodes):
            m_jsonObjs.pNodes = &node;
            break;

            case usize(HASH_CODES::meshes):
            m_jsonObjs.pMeshes = &node;
            break;

            case usize(HASH_CODES::cameras):
            m_jsonObjs.pCameras = &node;
            break;

            case usize(HASH_CODES::buffers):
            m_jsonObjs.pBuffers = &node;
            break;

            case usize(HASH_CODES::bufferViews):
            m_jsonObjs.pBufferViews = &node;
            break;

            case usize(HASH_CODES::accessors):
            m_jsonObjs.pAccessors = &node;
            break;

            case usize(HASH_CODES::materials):
            m_jsonObjs.pMaterials = &node;
            break;

            case usize(HASH_CODES::textures):
            m_jsonObjs.pTextures = &node;
            break;

            case usize(HASH_CODES::images):
            m_jsonObjs.pImages = &node;
            break;

            case usize(HASH_CODES::samplers):
            m_jsonObjs.pSamplers = &node;
            break;

            case usize(HASH_CODES::skins):
            m_jsonObjs.pSkins = &node;
            break;

            case usize(HASH_CODES::animations):
            m_jsonObjs.pAnimations = &node;
            break;
        }
    }

#ifdef D_GLTF
    auto check = [](const char* nts, json::Object* p) -> void {
        String s = p ? p->sKey : "(null)";
        CERR("\t{}: '{}'\n", nts, s);
    };

    check("scene", m_jsonObjs.scene);
    check("scenes", m_jsonObjs.scenes);
    check("nodes", m_jsonObjs.nodes);
    check("meshes", m_jsonObjs.meshes);
    check("buffers", m_jsonObjs.buffers);
    check("bufferViews", m_jsonObjs.bufferViews);
    check("accessors", m_jsonObjs.accessors);
    check("materials", m_jsonObjs.materials);
    check("textures", m_jsonObjs.textures);
    check("images", m_jsonObjs.images);
    check("samplers", m_jsonObjs.samplers);
    check("skins", m_jsonObjs.skins);
    check("animations", m_jsonObjs.animations);
#endif
}

void
Model::procScenes(IAllocator* pAlloc)
{
    auto scenes = m_jsonObjs.pScenes;
    auto& arr = json::getArray(scenes);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
        auto pNodes = json::searchObject(obj, "nodes");
        if (pNodes)
        {
            auto& a = json::getArray(pNodes);
            for (auto& el : a)
                m_vScenes.push(pAlloc, {(u32)json::getLong(&el)});
        }
        else
        {
            m_vScenes.push(pAlloc, {0});
            break;
        }
    }
}

void
Model::procBuffers(IAllocator* pAlloc)
{
    auto buffers = m_jsonObjs.pBuffers;
    auto& arr = json::getArray(buffers);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
        auto pByteLength = json::searchObject(obj, "byteLength");
        auto pUri = json::searchObject(obj, "uri");
        if (!pByteLength) LOG_FATAL("'byteLength' field is required\n");

        String svUri;
        Opt<String> rsBin;
        String aBin;

        if (pUri)
        {
            svUri = json::getString(pUri).clone(pAlloc);
            auto sNewPath = file::replacePathEnding(pAlloc, m_sPath, svUri);

            rsBin = file::load(pAlloc, sNewPath);
            if (!rsBin)
            {
                LOG_WARN("error opening file: '{}'\n", sNewPath);
            }
            else
            {
                aBin = rsBin.value();
            }
        }

        m_vBuffers.push(pAlloc, {
            .byteLength = (u32)(json::getLong(pByteLength)),
            .sUri = svUri,
            .sBin = aBin
        });
    }
}

void
Model::procBufferViews(IAllocator* pAlloc)
{
    auto bufferViews = m_jsonObjs.pBufferViews;
    auto& arr = json::getArray(bufferViews);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);

        auto pBuffer = json::searchObject(obj, "buffer");
        if (!pBuffer) LOG_FATAL("'buffer' field is required\n");
        auto pByteOffset = json::searchObject(obj, "byteOffset");
        auto pByteLength = json::searchObject(obj, "byteLength");
        if (!pByteLength) LOG_FATAL("'byteLength' field is required\n");
        auto pByteStride = json::searchObject(obj, "byteStride");
        auto pTarget = json::searchObject(obj, "target");

        m_vBufferViews.push(pAlloc, {
            .buffer = (u32)(json::getLong(pBuffer)),
            .byteOffset = pByteOffset ? (u32)(json::getLong(pByteOffset)) : 0,
            .byteLength = (u32)(json::getLong(pByteLength)),
            .byteStride = pByteStride ? (u32)(json::getLong(pByteStride)) : 0,
            .target = pTarget ? (TARGET)(json::getLong(pTarget)) : TARGET::NONE
        });
    }
}

void
Model::procAccessors(IAllocator* pAlloc)
{
    auto accessors = m_jsonObjs.pAccessors;
    auto& arr = json::getArray(accessors);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
 
        auto pBufferView = json::searchObject(obj, "bufferView");
        auto pByteOffset = json::searchObject(obj, "byteOffset");
        auto pComponentType = json::searchObject(obj, "componentType");
        if (!pComponentType) LOG_FATAL("'componentType' field is required\n");
        auto pCount = json::searchObject(obj, "count");
        if (!pCount) LOG_FATAL("'count' field is required\n");
        auto pMax = json::searchObject(obj, "max");
        auto pMin = json::searchObject(obj, "min");
        auto pType = json::searchObject(obj, "type");
        if (!pType) LOG_FATAL("'type' field is required\n");
 
        enum ACCESSOR_TYPE type = stringToAccessorType(json::getString(pType));
 
        m_vAccessors.push(pAlloc, {
            .bufferView = pBufferView ? (u32)(json::getLong(pBufferView)) : 0,
            .byteOffset = pByteOffset ? (u32)(json::getLong(pByteOffset)) : 0,
            .componentType = (COMPONENT_TYPE)(json::getLong(pComponentType)),
            .count = (u32)(json::getLong(pCount)),
            .max = pMax ? accessorTypeToUnionType(type, pMax) : Type{},
            .min = pMin ? accessorTypeToUnionType(type, pMin) : Type{},
            .type = type
        });
    }
}

void
Model::procMeshes(IAllocator* pAlloc)
{
    auto meshes = m_jsonObjs.pMeshes;
    auto& arr = json::getArray(meshes);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
 
        auto pPrimitives = json::searchObject(obj, "primitives");
        if (!pPrimitives) LOG_FATAL("'primitives' field is required\n");
 
        VecBase<Primitive> aPrimitives(pAlloc);
        auto pName = json::searchObject(obj, "name");
        auto name = pName ? json::getString(pName).clone(pAlloc) : "";
 
        auto& aPrim = json::getArray(pPrimitives);
        for (auto& p : aPrim)
        {
            auto& op = json::getObject(&p);
 
            auto pAttributes = json::searchObject(op, "attributes");
            auto& oAttr = json::getObject(pAttributes);
            auto pNORMAL = json::searchObject(oAttr, "NORMAL");
            auto pTANGENT = json::searchObject(oAttr, "TANGENT");
            auto pPOSITION = json::searchObject(oAttr, "POSITION");
            auto pTEXCOORD_0 = json::searchObject(oAttr, "TEXCOORD_0");
 
            auto pIndices = json::searchObject(op, "indices");
            auto pMode = json::searchObject(op, "mode");
            auto pMaterial = json::searchObject(op, "material");
 
            aPrimitives.push(pAlloc, {
                .attributes {
                    .NORMAL = pNORMAL ? (i32)(json::getLong(pNORMAL)) : -1,
                    .POSITION = pPOSITION ? (i32)(json::getLong(pPOSITION)) : -1,
                    .TEXCOORD_0 = pTEXCOORD_0 ? (i32)(json::getLong(pTEXCOORD_0)) : -1,
                    .TANGENT = pTANGENT ? (i32)(json::getLong(pTANGENT)) : -1,
                },
                .indices = pIndices ? (i32)(json::getLong(pIndices)) : -1,
                .material = pMaterial ? (i32)(json::getLong(pMaterial)) : -1,
                .mode = pMode ? static_cast<decltype(Primitive::mode)>(json::getLong(pMode)) : PRIMITIVES::TRIANGLES,
            });
        }
 
        m_vMeshes.push(pAlloc, {.aPrimitives = aPrimitives, .sName = name});
    }
}

void
Model::procTexures(IAllocator* pAlloc)
{
    auto textures = m_jsonObjs.pTextures;
    if (!textures) return;

    auto& arr = json::getArray(textures);
    for (auto& tex : arr)
    {
        auto& obj = json::getObject(&tex);

        auto pSource = json::searchObject(obj, "source");
        auto pSampler = json::searchObject(obj, "sampler");

        m_vTextures.push(pAlloc, {
            .source = pSource ? (i32)(json::getLong(pSource)) : -1,
            .sampler = pSampler ? (i32)(json::getLong(pSampler)) : -1
        });
    }
}

void
Model::procMaterials(IAllocator* pAlloc)
{
    auto materials = m_jsonObjs.pMaterials;
    if (!materials) return;

    auto& arr = json::getArray(materials);
    for (auto& mat : arr)
    {
        auto& obj = json::getObject(&mat);

        TextureInfo texInfo {};

        auto pPbrMetallicRoughness = json::searchObject(obj, "pbrMetallicRoughness");
        if (pPbrMetallicRoughness)
        {
            auto& oPbr = json::getObject(pPbrMetallicRoughness);

            auto pBaseColorTexture = json::searchObject(oPbr, "baseColorTexture");
            if (pBaseColorTexture)
            {
                auto& objBct = json::getObject(pBaseColorTexture);

                auto pIndex = json::searchObject(objBct, "index");
                if (!pIndex) LOG_FATAL("index field is required\n");

                texInfo.index = json::getLong(pIndex);
            }
        }

        NormalTextureInfo normTexInfo {};

        auto pNormalTexture = json::searchObject(obj, "normalTexture");
        if (pNormalTexture)
        {
            auto& objNT = json::getObject(pNormalTexture);
            auto pIndex = json::searchObject(objNT, "index");
            if (!pIndex) LOG_FATAL("index filed is required\n");

            normTexInfo.index = json::getLong(pIndex);
        }

        m_vMaterials.push(pAlloc, {
            .pbrMetallicRoughness {
                .baseColorTexture = texInfo,
            },
            .normalTexture = normTexInfo
        });
    }
}

void
Model::procImages(IAllocator* pAlloc)
{
    auto imgs = m_jsonObjs.pImages;
    if (!imgs) return;

    auto& arr = json::getArray(imgs);
    for (auto& img : arr)
    {
        auto& obj = json::getObject(&img);

        auto pUri = json::searchObject(obj, "uri");
        if (pUri)
        {
            m_vImages.push(pAlloc,
                {json::getString(pUri).clone(pAlloc)}
            );
        }
    }
}

void
Model::procNodes(IAllocator* pAlloc)
{
    auto nodes = m_jsonObjs.pNodes;
    auto& arr = json::getArray(nodes);
    for (auto& node : arr)
    {
        auto& obj = json::getObject(&node);

        Node nNode(pAlloc);

        auto pName = json::searchObject(obj, "name");
        if (pName) nNode.sName = json::getString(pName);

        auto pCamera = json::searchObject(obj, "camera");
        if (pCamera) nNode.camera = (u32)(json::getLong(pCamera));

        auto pChildren = json::searchObject(obj, "children");
        if (pChildren)
        {
            auto& arrChil = json::getArray(pChildren);
            for (auto& c : arrChil)

            nNode.children.push(pAlloc, (u32)(json::getLong(&c)));
        }

        auto pMatrix = json::searchObject(obj, "matrix");
        if (pMatrix)
        {
            auto ut = assignUnionType(pMatrix, 4*4);
            nNode.matrix = ut.MAT4;
        }

        auto pMesh = json::searchObject(obj, "mesh");
        if (pMesh) nNode.mesh = (u32)(json::getLong(pMesh));

        auto pTranslation = json::searchObject(obj, "translation");
        if (pTranslation)
        {
            auto ut = assignUnionType(pTranslation, 3);
            nNode.translation = ut.VEC3;
        }

        auto pRotation = json::searchObject(obj, "rotation");
        if (pRotation)
        {
            auto ut = assignUnionType(pRotation, 4);
            nNode.rotation.base = ut.VEC4;
        }

        auto pScale = json::searchObject(obj, "scale");
        if (pScale)
        {
            auto ut = assignUnionType(pScale, 3);
            nNode.scale = ut.VEC3;
        }

        m_vNodes.push(pAlloc, nNode);
    }
}

void
Model::procAnimations(adt::IAllocator* pAlloc)
{
    if (!m_jsonObjs.pAnimations)
        return;

    const auto* pAnimations = m_jsonObjs.pAnimations;
    const auto& aAnimations = json::getArray(pAnimations); /* usually an array of one object */

    for (auto& animation : aAnimations)
    {
        auto& obj = json::getObject(&animation);

        Animation newAnim {};

        auto* pName = json::searchObject(obj, "name");
        newAnim.sName = json::getString(pName).clone(pAlloc);

        auto* pChannelsObj = json::searchObject(obj, "channels");
        if (!pChannelsObj)
        {
            LOG_BAD("'channels' object is required\n");
            return;
        }

        auto& arrChannels = json::getArray(pChannelsObj);
        for (const auto& channel : arrChannels)
        {
            auto& channelObj = json::getObject(&channel);

            Animation::Channel newChannel {};

            auto* pSampler = json::searchObject(channelObj, "sampler");
            if (!pSampler)
            {
                LOG_BAD("'sampler' object is required\n");
                return;
            }

            auto sampler = json::getLong(pSampler);
            newChannel.sampler = sampler;

            auto* pTarget = json::searchObject(channelObj, "target");
            if (!pTarget)
            {
                LOG_BAD("'target' object is required\n");
                return;
            }

            auto& targetObj = json::getObject(pTarget);

            Animation::Channel::Target newTarget {};

            auto pNode = json::searchObject(targetObj, "node");
            if (pNode)
                newTarget.node = static_cast<int>(json::getLong(pNode));

            auto pPath = json::searchObject(targetObj, "path");
            if (!pPath)
            {
                LOG_BAD("'path' object is required\n");
                return;
            }

            auto eType = AnimationChannelTargetPathTypeStringToPATH_TYPE(json::getString(pPath));
            newTarget.ePath = eType;

            newAnim.channels.push(pAlloc, newChannel);
        }

        m_vAnimations.push(pAlloc, newAnim);
    }
}

} /* namespace gltf */
