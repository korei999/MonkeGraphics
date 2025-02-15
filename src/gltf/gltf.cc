#include "gltf.hh"

#include "adt/file.hh"
#include "adt/logs.hh"

using namespace adt;

namespace gltf
{

enum class HASH_CODE : usize
{
    asset = hash::func("asset"),
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
        case usize(HASH_CODE::SCALAR):
        return ACCESSOR_TYPE::SCALAR;

        case usize(HASH_CODE::VEC2):
        return ACCESSOR_TYPE::VEC2;

        case usize(HASH_CODE::VEC3):
        return ACCESSOR_TYPE::VEC3;

        case usize(HASH_CODE::VEC4):
        return ACCESSOR_TYPE::VEC4;

        case usize(HASH_CODE::MAT2):
        return ACCESSOR_TYPE::MAT3;

        case usize(HASH_CODE::MAT3):
        return ACCESSOR_TYPE::MAT3;

        case usize(HASH_CODE::MAT4):
        return ACCESSOR_TYPE::MAT4;
    }
}

static union Type
assignUnionType(json::Object* obj, int n)
{
    auto& arr = json::getArray(obj);
    union Type type;

    for (int i = 0; i < n; i++)
    {
        if (arr[i].tagVal.eTag == json::TAG::LONG)
            type.MAT4.d[i] = f32(json::getLong(&arr[i]));
        else type.MAT4.d[i] = f32(json::getDouble(&arr[i]));
    }

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
    else if (svPath == "weights")
        return Animation::Channel::Target::PATH_TYPE::WEIGHTS;

    LOG_BAD("failed to convert path string\n");
    return {};
}

static Animation::Sampler::INTERPOLATION_TYPE
AnimationSamplerStringToPATH_TYPE(const String svPath)
{
    if (svPath == "LINEAR")
        return Animation::Sampler::INTERPOLATION_TYPE::LINEAR;
    else if (svPath == "STEP")
        return Animation::Sampler::INTERPOLATION_TYPE::STEP;
    else if (svPath == "CUBICSPLINE")
        return Animation::Sampler::INTERPOLATION_TYPE::CUBICSPLINE;

    return Animation::Sampler::INTERPOLATION_TYPE::LINEAR;
}

bool
Model::read(IAllocator* pAlloc, const json::Parser& parsed, const String svPath)
{
    m_sPath = svPath.clone(pAlloc);

    procToplevelObjs(pAlloc, parsed);

    if (!procAsset(pAlloc)) return false;
    if (!procRootScene(pAlloc)) return false;
    if (!procScenes(pAlloc)) return false;
    if (!procBuffers(pAlloc)) return false;
    if (!procBufferViews(pAlloc)) return false;
    if (!procAccessors(pAlloc)) return false;
    if (!procMeshes(pAlloc)) return false;
    if (!procTexures(pAlloc)) return false;
    if (!procMaterials(pAlloc)) return false;
    if (!procImages(pAlloc)) return false;
    if (!procNodes(pAlloc)) return false;
    if (!procAnimations(pAlloc)) return false;

    /* nullify potentially dangling pointers */
    m_toplevelObjs = {};

    return true;
}

bool
Model::procToplevelObjs(IAllocator*, const json::Parser& parser)
{
    /* collect all the top level objects */
    for (auto& node : parser.getRoot())
    {
        switch (hash::func(node.svKey))
        {
            case usize(HASH_CODE::asset):
            m_toplevelObjs.pAsset = &node;
            break;

            case usize(HASH_CODE::scene):
            m_toplevelObjs.pScene = &node;
            break;

            case usize(HASH_CODE::scenes):
            m_toplevelObjs.pScenes = &node;
            break;

            case usize(HASH_CODE::nodes):
            m_toplevelObjs.pNodes = &node;
            break;

            case usize(HASH_CODE::meshes):
            m_toplevelObjs.pMeshes = &node;
            break;

            case usize(HASH_CODE::cameras):
            m_toplevelObjs.pCameras = &node;
            break;

            case usize(HASH_CODE::buffers):
            m_toplevelObjs.pBuffers = &node;
            break;

            case usize(HASH_CODE::bufferViews):
            m_toplevelObjs.pBufferViews = &node;
            break;

            case usize(HASH_CODE::accessors):
            m_toplevelObjs.pAccessors = &node;
            break;

            case usize(HASH_CODE::materials):
            m_toplevelObjs.pMaterials = &node;
            break;

            case usize(HASH_CODE::textures):
            m_toplevelObjs.pTextures = &node;
            break;

            case usize(HASH_CODE::images):
            m_toplevelObjs.pImages = &node;
            break;

            case usize(HASH_CODE::samplers):
            m_toplevelObjs.pSamplers = &node;
            break;

            case usize(HASH_CODE::skins):
            m_toplevelObjs.pSkins = &node;
            break;

            case usize(HASH_CODE::animations):
            m_toplevelObjs.pAnimations = &node;
            break;
        }
    }

    return true;
}

bool
Model::procAsset(adt::IAllocator* pAlloc)
{
    if (!m_toplevelObjs.pAsset)
        return false;

    const auto& assetObj = json::getObject(m_toplevelObjs.pAsset);
    auto* pVersion = json::searchObject(assetObj, "version");
    if (!pVersion)
    {
        LOG_BAD("'version' string is required\n");
        return false;
    }

    m_asset.sVersion = json::getString(pVersion).clone(pAlloc);

    auto* pCopyright = json::searchObject(assetObj, "copyright");
    if (pCopyright)
        m_asset.sCopyright = json::getString(pCopyright).clone(pAlloc);

    auto* pGenerator = json::searchObject(assetObj, "generator");
    if (pGenerator)
        m_asset.sGenerator = json::getString(pGenerator).clone(pAlloc);

    auto* pMinVersion = json::searchObject(assetObj, "minVersion");
    if (pMinVersion)
        m_asset.sMinVersion = json::getString(pMinVersion).clone(pAlloc);

    return true;
}

bool
Model::procRootScene(adt::IAllocator*)
{
    if (!m_toplevelObjs.pScene)
    {
        LOG_BAD("'scene' object not found\n");
        return false;
    }

    m_defaultScene.nodeI = json::getLong(m_toplevelObjs.pScene);

    return true;
}

bool
Model::procScenes(IAllocator* pAlloc)
{
    auto scenes = m_toplevelObjs.pScenes;
    auto& arr = json::getArray(scenes);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);

        Scene newScene {};

        auto pNodes = json::searchObject(obj, "nodes");
        if (pNodes)
        {
            auto& vNodes = json::getArray(pNodes);
            VecBase<int> vNewNodes {};
            for (auto& node : vNodes)
                vNewNodes.push(pAlloc, static_cast<int>(json::getLong(&node)));

            newScene.vNodes = vNewNodes;
        }

        auto pName = json::searchObject(obj, "name");
        if (pName)
            newScene.sName = json::getString(pName).clone(pAlloc);

        m_vScenes.push(pAlloc, newScene);
    }

    return true;
}

bool
Model::procBuffers(IAllocator* pAlloc)
{
    auto buffers = m_toplevelObjs.pBuffers;
    auto& arr = json::getArray(buffers);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
        auto pByteLength = json::searchObject(obj, "byteLength");
        auto pUri = json::searchObject(obj, "uri");
        if (!pByteLength)
        {
            LOG_BAD("'byteLength' is required\n");
            return false;
        }

        String svUri;
        Opt<String> rsBin;
        String aBin;

        if (pUri)
        {
            svUri = json::getString(pUri).clone(pAlloc);
            auto sNewPath = file::replacePathEnding(pAlloc, m_sPath, svUri);

            rsBin = file::load(pAlloc, sNewPath);
            if (!rsBin)
                LOG_WARN("error opening file: '{}'\n", sNewPath);
            else aBin = rsBin.value();
        }

        m_vBuffers.push(pAlloc, {
            .byteLength = static_cast<int>(json::getLong(pByteLength)),
            .sUri = svUri,
            .sBin = aBin
        });
    }

    return true;
}

bool
Model::procBufferViews(IAllocator* pAlloc)
{
    auto bufferViews = m_toplevelObjs.pBufferViews;
    auto& arr = json::getArray(bufferViews);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);

        BufferView newView {};

        auto pBuffer = json::searchObject(obj, "buffer");
        if (!pBuffer)
        {
            LOG_BAD("'buffer' field is required\n");
            return false;
        }

        auto pByteOffset = json::searchObject(obj, "byteOffset");
        auto pByteLength = json::searchObject(obj, "byteLength");

        if (!pByteLength)
        {
            LOG_BAD("'byteLength' field is required\n");
            return false;
        }

        auto pByteStride = json::searchObject(obj, "byteStride");
        auto pTarget = json::searchObject(obj, "target");

        m_vBufferViews.push(pAlloc, {
            .bufferI = static_cast<int>(json::getLong(pBuffer)),
            .byteOffset = pByteOffset ? static_cast<int>(json::getLong(pByteOffset)) : 0,
            .byteLength = static_cast<int>(json::getLong(pByteLength)),
            .byteStride = pByteStride ? static_cast<int>(json::getLong(pByteStride)) : 0,
            .eTarget = pTarget ? (TARGET)(json::getLong(pTarget)) : TARGET::NONE
        });
    }

    return true;
}

bool
Model::procAccessors(IAllocator* pAlloc)
{
    auto accessors = m_toplevelObjs.pAccessors;
    auto& arr = json::getArray(accessors);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
 
        auto pBufferView = json::searchObject(obj, "bufferView");
        auto pByteOffset = json::searchObject(obj, "byteOffset");
        auto pComponentType = json::searchObject(obj, "componentType");

        if (!pComponentType)
        {
            LOG_BAD("'componentType' field is required\n");
            return false;
        }

        auto pCount = json::searchObject(obj, "count");
        if (!pCount)
        {
            LOG_BAD("'count' field is required\n");
            return false;
        }

        auto pMax = json::searchObject(obj, "max");
        auto pMin = json::searchObject(obj, "min");
        auto pType = json::searchObject(obj, "type");

        if (!pType)
        {
            LOG_BAD("'type' field is required\n");
            return false;
        }
 
        enum ACCESSOR_TYPE type = stringToAccessorType(json::getString(pType));
 
        m_vAccessors.push(pAlloc, {
            .bufferViewI = pBufferView ? static_cast<int>(json::getLong(pBufferView)) : 0,
            .byteOffset = pByteOffset ? static_cast<int>(json::getLong(pByteOffset)) : 0,
            .eComponentType = (COMPONENT_TYPE)(json::getLong(pComponentType)),
            .count = static_cast<int>(json::getLong(pCount)),
            .max = pMax ? accessorTypeToUnionType(type, pMax) : Type{},
            .min = pMin ? accessorTypeToUnionType(type, pMin) : Type{},
            .eType = type
        });
    }

    return true;
}

bool
Model::procMeshes(IAllocator* pAlloc)
{
    auto meshes = m_toplevelObjs.pMeshes;
    auto& arr = json::getArray(meshes);
    for (auto& e : arr)
    {
        auto& obj = json::getObject(&e);
 
        auto pPrimitives = json::searchObject(obj, "primitives");
        if (!pPrimitives)
        {
            LOG_BAD("'primitives' field is required\n");
            return false;
        }
 
        VecBase<Primitive> aPrimitives(pAlloc);
        auto pName = json::searchObject(obj, "name");
        auto name = pName ? json::getString(pName).clone(pAlloc) : "";
 
        auto& aPrim = json::getArray(pPrimitives);
        for (auto& p : aPrim)
        {
            auto& op = json::getObject(&p);

            Primitive newPrimitive {};

            auto pIndices = json::searchObject(op, "indices");
            if (pIndices)
                newPrimitive.indicesI = static_cast<int>(json::getLong(pIndices));

            auto pMode = json::searchObject(op, "mode");
            if (pMode)
                newPrimitive.eMode = static_cast<Primitive::TYPE>(json::getLong(pMode));

            auto pMaterial = json::searchObject(op, "material");
            if (pMaterial)
                newPrimitive.materialI = static_cast<int>(json::getLong(pMaterial));
 
            auto pAttributes = json::searchObject(op, "attributes");
            if (!pAttributes)
            {
                LOG_BAD("'attributes' field is required\n");
                return false;
            }

            auto& oAttr = json::getObject(pAttributes);

            auto pNORMAL = json::searchObject(oAttr, "NORMAL");
            if (pNORMAL)
                newPrimitive.attributes.NORMAL = static_cast<int>(json::getLong(pNORMAL));
            auto pTANGENT = json::searchObject(oAttr, "TANGENT");

            if (pTANGENT)
                newPrimitive.attributes.TANGENT = static_cast<int>(json::getLong(pTANGENT));
            auto pPOSITION = json::searchObject(oAttr, "POSITION");

            if (pPOSITION)
                newPrimitive.attributes.POSITION = static_cast<int>(json::getLong(pPOSITION));

            auto pTEXCOORD_0 = json::searchObject(oAttr, "TEXCOORD_0");
            if (pTEXCOORD_0)
                newPrimitive.attributes.TEXCOORD_0 = static_cast<int>(json::getLong(pTEXCOORD_0));
 
            aPrimitives.push(pAlloc, newPrimitive);
        }
 
        m_vMeshes.push(pAlloc, {.vPrimitives = aPrimitives, .sName = name});
    }

    return true;
}

bool
Model::procTexures(IAllocator* pAlloc)
{
    auto textures = m_toplevelObjs.pTextures;
    if (!textures)
        return true;

    auto& arr = json::getArray(textures);
    for (auto& tex : arr)
    {
        auto& obj = json::getObject(&tex);

        auto pSource = json::searchObject(obj, "source");
        auto pSampler = json::searchObject(obj, "sampler");

        m_vTextures.push(pAlloc, {
            .sourceI = pSource ? (i32)(json::getLong(pSource)) : -1,
            .samplerI = pSampler ? (i32)(json::getLong(pSampler)) : -1
        });
    }

    return true;
}

bool
Model::procMaterials(IAllocator* pAlloc)
{
    auto* pMaterials = m_toplevelObjs.pMaterials;
    if (!pMaterials)
        return true;

    auto& arr = json::getArray(pMaterials);
    for (auto& mat : arr)
    {
        auto& obj = json::getObject(&mat);

        Material newMaterial {};

        auto pName = json::searchObject(obj, "name");
        if (pName)
            newMaterial.sName = json::getString(pName).clone(pAlloc);

        auto pPbrMetallicRoughness = json::searchObject(obj, "pbrMetallicRoughness");
        if (pPbrMetallicRoughness)
        {
            auto& oPbr = json::getObject(pPbrMetallicRoughness);

            {
                auto pBaseColorTexture = json::searchObject(oPbr, "baseColorTexture");
                if (pBaseColorTexture)
                {
                    auto& objBct = json::getObject(pBaseColorTexture);

                    auto pIndex = json::searchObject(objBct, "index");
                    if (!pIndex)
                    {
                        LOG_BAD("index field is required\n");
                        return false;
                    }

                    newMaterial.pbrMetallicRoughness.baseColorTexture.index = json::getLong(pIndex);
                }
            }

            {
                auto pBaseColorFactor = json::searchObject(oPbr, "baseColorFactor");
                if (pBaseColorFactor)
                    newMaterial.pbrMetallicRoughness.baseColorFactor = assignUnionType(pBaseColorFactor, 4).VEC4;
            }
        }

        auto pNormalTexture = json::searchObject(obj, "normalTexture");
        if (pNormalTexture)
        {
            auto& objNT = json::getObject(pNormalTexture);
            auto pIndex = json::searchObject(objNT, "index");
            if (!pIndex)
            {
                LOG_BAD("index filed is required\n");
                return false;
            }

            newMaterial.normalTexture.index = json::getLong(pIndex);
        }

        m_vMaterials.push(pAlloc, newMaterial);
    }

    return true;
}

bool
Model::procImages(IAllocator* pAlloc)
{
    auto imgs = m_toplevelObjs.pImages;
    if (!imgs)
        return true;

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

    return true;
}

bool
Model::procNodes(IAllocator* pAlloc)
{
    auto nodes = m_toplevelObjs.pNodes;
    auto& arr = json::getArray(nodes);
    for (auto& node : arr)
    {
        auto& obj = json::getObject(&node);

        Node newNode {};

        auto pName = json::searchObject(obj, "name");
        if (pName)
            newNode.sName = json::getString(pName);

        auto pCamera = json::searchObject(obj, "camera");
        if (pCamera) newNode.camera = static_cast<int>(json::getLong(pCamera));

        auto pChildren = json::searchObject(obj, "children");
        if (pChildren)
        {
            auto& arrChil = json::getArray(pChildren);
            for (auto& c : arrChil)
                newNode.vChildren.push(pAlloc, static_cast<int>(json::getLong(&c)));
        }

        auto pMatrix = json::searchObject(obj, "matrix");
        if (pMatrix)
        {
            newNode.uTransformation.matrix = assignUnionType(pMatrix, 4*4).MAT4;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::MATRIX;
        }

        auto pMesh = json::searchObject(obj, "mesh");
        if (pMesh) newNode.meshI = static_cast<int>(json::getLong(pMesh));

        auto pTranslation = json::searchObject(obj, "translation");
        if (pTranslation)
        {
            auto ut = assignUnionType(pTranslation, 3);
            newNode.uTransformation.animation.translation = ut.VEC3;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::ANIMATION;
        }

        auto pRotation = json::searchObject(obj, "rotation");
        if (pRotation)
        {
            auto ut = assignUnionType(pRotation, 4);
            newNode.uTransformation.animation.rotation.base = ut.VEC4;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::ANIMATION;
        }

        auto pScale = json::searchObject(obj, "scale");
        if (pScale)
        {
            auto ut = assignUnionType(pScale, 3);
            newNode.uTransformation.animation.scale = ut.VEC3;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::ANIMATION;
        }

        m_vNodes.push(pAlloc, newNode);
    }

    return true;
}

bool
Model::procAnimations(adt::IAllocator* pAlloc)
{
    if (!m_toplevelObjs.pAnimations)
        return true;

    const auto* pAnimations = m_toplevelObjs.pAnimations;
    const auto& aAnimations = json::getArray(pAnimations); /* usually an array of one object */

    for (auto& animation : aAnimations)
    {
        auto& obj = json::getObject(&animation);

        Animation newAnim {};

        auto* pName = json::searchObject(obj, "name");
        if (pName)
            newAnim.sName = json::getString(pName).clone(pAlloc);

        auto* pChannelsObj = json::searchObject(obj, "channels");
        if (!pChannelsObj)
        {
            LOG_BAD("'channels' object is required\n");
            return false;
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
                return false;
            }

            auto sampler = json::getLong(pSampler);
            newChannel.samplerI = sampler;

            auto* pTarget = json::searchObject(channelObj, "target");
            if (!pTarget)
            {
                LOG_BAD("'target' object is required\n");
                return false;
            }

            auto& targetObj = json::getObject(pTarget);

            Animation::Channel::Target newTarget {};

            auto pNode = json::searchObject(targetObj, "node");
            if (pNode)
            {
                newTarget.nodeI = static_cast<int>(json::getLong(pNode));
            }

            auto pPath = json::searchObject(targetObj, "path");
            if (!pPath)
            {
                LOG_BAD("'path' object is required\n");
                return false;
            }

            auto eType = AnimationChannelTargetPathTypeStringToPATH_TYPE(json::getString(pPath));
            newTarget.ePath = eType;

            newChannel.target = newTarget;
            newAnim.vChannels.push(pAlloc, newChannel);
        }

        auto* pSamplers = json::searchObject(obj, "samplers");
        if (!pSamplers)
        {
            LOG_BAD("'samplers' objects is required\n");
            return false;
        }

        auto& samplersArr = json::getArray(pSamplers);
        for (auto& sampler : samplersArr)
        {
            auto& samplerObj = json::getObject(&sampler);

            Animation::Sampler newSampler {};

            auto* pInput = json::searchObject(samplerObj, "input");
            if (!pInput)
            {
                LOG_BAD("'input' is required\n");
                return false;
            }

            newSampler.inputI = static_cast<int>(json::getLong(pInput));

            auto* pInterpolation = json::searchObject(samplerObj, "interpolation");
            if (pInterpolation)
                newSampler.eInterpolation = AnimationSamplerStringToPATH_TYPE(json::getString(pInterpolation));

            auto* pOutput = json::searchObject(samplerObj, "output");
            if (!pOutput)
            {
                LOG_BAD("'output' field is required\n");
                return false;
            }

            newSampler.outputI = static_cast<int>(json::getLong(pOutput));

            newAnim.vSamplers.push(pAlloc, newSampler);
        }

        m_vAnimations.push(pAlloc, newAnim);
    }
    
    return true;
}

} /* namespace gltf */
