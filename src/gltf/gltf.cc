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

static Accessor::TYPE
stringToAccessorType(StringView sv)
{
    switch (hash::func(sv))
    {
        default:
        case usize(HASH_CODE::SCALAR):
        return Accessor::TYPE::SCALAR;

        case usize(HASH_CODE::VEC2):
        return Accessor::TYPE::VEC2;

        case usize(HASH_CODE::VEC3):
        return Accessor::TYPE::VEC3;

        case usize(HASH_CODE::VEC4):
        return Accessor::TYPE::VEC4;

        case usize(HASH_CODE::MAT2):
        return Accessor::TYPE::MAT3;

        case usize(HASH_CODE::MAT3):
        return Accessor::TYPE::MAT3;

        case usize(HASH_CODE::MAT4):
        return Accessor::TYPE::MAT4;
    }
}

static union Type
assignUnionType(json::Node* obj, int n)
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
accessorTypeToUnionType(Accessor::TYPE eType, json::Node* obj)
{
    union Type type;

    switch (eType)
    {
        default:
        case Accessor::TYPE::SCALAR:
        {
            auto& arr = json::getArray(obj);
            if (arr[0].tagVal.eTag == json::TAG::LONG)
                type.SCALAR = f64(json::getLong(&arr[0]));
            else type.SCALAR = f64(json::getDouble(&arr[0]));
        } break;

        case Accessor::TYPE::VEC2:
        type = assignUnionType(obj, 2);
        break;

        case Accessor::TYPE::VEC3:
        type = assignUnionType(obj, 3);
        break;

        case Accessor::TYPE::VEC4:
        type = assignUnionType(obj, 4);
        break;

        case Accessor::TYPE::MAT2:
        type = assignUnionType(obj, 2*2);
        break;

        case Accessor::TYPE::MAT3:
        type = assignUnionType(obj, 3*3);
        break;

        case Accessor::TYPE::MAT4:
        type = assignUnionType(obj, 4*4);
        break;
    }

    return type;
}

static Animation::Channel::Target::PATH_TYPE
AnimationChannelTargetPathTypeStringToPATH_TYPE(const StringView svPath)
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
AnimationSamplerStringToPATH_TYPE(const StringView svPath)
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
Model::read(IAllocator* pAlloc, const json::Parser& parsed, const StringView svPath)
{
    m_sPath = String(pAlloc, svPath);

    procToplevelObjs(pAlloc, parsed);

    if (!procAsset(pAlloc)) return false;
    if (!procRootScene(pAlloc)) return false;
    if (!procScenes(pAlloc)) return false;
    if (!procBuffers(pAlloc)) return false;
    if (!procBufferViews(pAlloc)) return false;
    if (!procAccessors(pAlloc)) return false;
    if (!procSkins(pAlloc)) return false;
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
Model::procAsset(IAllocator* pAlloc)
{
    if (!m_toplevelObjs.pAsset)
        return false;

    const auto& assetObj = json::getObject(m_toplevelObjs.pAsset);
    auto* pVersion = json::searchNode(assetObj, "version");
    if (!pVersion)
    {
        LOG_BAD("'version' string is required\n");
        return false;
    }

    m_asset.sVersion = String(pAlloc, json::getString(pVersion));

    auto* pCopyright = json::searchNode(assetObj, "copyright");
    if (pCopyright)
        m_asset.sCopyright = String(pAlloc, json::getString(pCopyright));

    auto* pGenerator = json::searchNode(assetObj, "generator");
    if (pGenerator)
        m_asset.sGenerator = String(pAlloc, json::getString(pGenerator));

    auto* pMinVersion = json::searchNode(assetObj, "minVersion");
    if (pMinVersion)
        m_asset.sMinVersion = String(pAlloc, json::getString(pMinVersion));

    return true;
}

bool
Model::procRootScene(IAllocator*)
{
    if (!m_toplevelObjs.pScene)
    {
        LOG_BAD("'scene' object not found\n");
        return false;
    }

    m_defaultSceneI = static_cast<int>(json::getLong(m_toplevelObjs.pScene));

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

        auto pNodes = json::searchNode(obj, "nodes");
        if (pNodes)
        {
            auto& vNodes = json::getArray(pNodes);
            Vec<int> vNewNodes {};
            for (auto& node : vNodes)
                vNewNodes.push(pAlloc, static_cast<int>(json::getLong(&node)));

            newScene.vNodes = vNewNodes;
        }

        auto pName = json::searchNode(obj, "name");
        if (pName)
            newScene.sName = String(pAlloc, json::getString(pName));

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
        auto pByteLength = json::searchNode(obj, "byteLength");
        auto pUri = json::searchNode(obj, "uri");
        if (!pByteLength)
        {
            LOG_BAD("'byteLength' is required\n");
            return false;
        }

        String svUri;
        String aBin;

        if (pUri)
        {
            svUri = String(pAlloc, json::getString(pUri));
            auto sNewPath = file::replacePathEnding(pAlloc, m_sPath, svUri);

            aBin = file::load(pAlloc, sNewPath);
            if (!aBin)
                LOG_WARN("error opening file: '{}'\n", sNewPath);
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

        auto pBuffer = json::searchNode(obj, "buffer");
        if (!pBuffer)
        {
            LOG_BAD("'buffer' field is required\n");
            return false;
        }

        auto pByteOffset = json::searchNode(obj, "byteOffset");
        auto pByteLength = json::searchNode(obj, "byteLength");

        if (!pByteLength)
        {
            LOG_BAD("'byteLength' field is required\n");
            return false;
        }

        auto pByteStride = json::searchNode(obj, "byteStride");
        auto pTarget = json::searchNode(obj, "target");

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
 
        auto pBufferView = json::searchNode(obj, "bufferView");
        auto pByteOffset = json::searchNode(obj, "byteOffset");
        auto pComponentType = json::searchNode(obj, "componentType");

        if (!pComponentType)
        {
            LOG_BAD("'componentType' field is required\n");
            return false;
        }

        auto pCount = json::searchNode(obj, "count");
        if (!pCount)
        {
            LOG_BAD("'count' field is required\n");
            return false;
        }

        auto pMax = json::searchNode(obj, "max");
        auto pMin = json::searchNode(obj, "min");
        auto pType = json::searchNode(obj, "type");

        if (!pType)
        {
            LOG_BAD("'type' field is required\n");
            return false;
        }
 
        Accessor::TYPE eType = stringToAccessorType(json::getString(pType));
 
        m_vAccessors.push(pAlloc, {
            .bufferViewI = pBufferView ? static_cast<int>(json::getLong(pBufferView)) : 0,
            .byteOffset = pByteOffset ? static_cast<int>(json::getLong(pByteOffset)) : 0,
            .eComponentType = (COMPONENT_TYPE)(json::getLong(pComponentType)),
            .count = static_cast<int>(json::getLong(pCount)),
            .uMax = pMax ? accessorTypeToUnionType(eType, pMax) : Type{},
            .uMin = pMin ? accessorTypeToUnionType(eType, pMin) : Type{},
            .eType = eType
        });
    }

    return true;
}

bool
Model::procSkins(IAllocator* pAlloc)
{
    if (!m_toplevelObjs.pSkins)
        return true;

    auto& skinsArray = json::getArray(m_toplevelObjs.pSkins);

    for (auto& skin : skinsArray)
    {
        auto& skinObj = json::getObject(&skin);

        Skin newSkin {};

        auto* pJoints = json::searchNode(skinObj, "joints");
        if (!pJoints)
        {
            LOG_BAD("'joints' are required\n");
            return false;
        }

        auto& jointsArr = json::getArray(pJoints);
        for (auto joint : jointsArr)
        {
            newSkin.vJoints.push(pAlloc,
                static_cast<int>(json::getLong(&joint))
            );
        }

        auto* pName = json::searchNode(skinObj, "name");
        if (pName)
            newSkin.sName = String(pAlloc, json::getString(pName));

        auto* pInverseBindMatrices = json::searchNode(skinObj, "inverseBindMatrices");
        if (pInverseBindMatrices)
            newSkin.inverseBindMatricesI = static_cast<int>(json::getLong(pInverseBindMatrices));

        auto* pSkeleton = json::searchNode(skinObj, "skeleton");
        if (pSkeleton)
            newSkin.skeleton = static_cast<int>(json::getLong(pSkeleton));

        m_vSkins.push(pAlloc, newSkin);
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
 
        auto pPrimitives = json::searchNode(obj, "primitives");
        if (!pPrimitives)
        {
            LOG_BAD("'primitives' field is required\n");
            return false;
        }
 
        Vec<Primitive> aPrimitives(pAlloc);
        auto pName = json::searchNode(obj, "name");
        String sName = pName ? String(pAlloc, json::getString(pName)) : String{};
 
        auto& aPrim = json::getArray(pPrimitives);
        for (auto& p : aPrim)
        {
            auto& op = json::getObject(&p);

            Primitive newPrimitive {};

            auto pIndices = json::searchNode(op, "indices");
            if (pIndices)
                newPrimitive.indicesI = static_cast<int>(json::getLong(pIndices));

            auto pMode = json::searchNode(op, "mode");
            if (pMode)
                newPrimitive.eMode = static_cast<Primitive::TYPE>(json::getLong(pMode));

            auto pMaterial = json::searchNode(op, "material");
            if (pMaterial)
                newPrimitive.materialI = static_cast<int>(json::getLong(pMaterial));
 
            auto pAttributes = json::searchNode(op, "attributes");
            if (!pAttributes)
            {
                LOG_BAD("'attributes' field is required\n");
                return false;
            }

            auto& oAttr = json::getObject(pAttributes);

            auto pNORMAL = json::searchNode(oAttr, "NORMAL");
            if (pNORMAL)
                newPrimitive.attributes.NORMAL = static_cast<int>(json::getLong(pNORMAL));

            auto pTANGENT = json::searchNode(oAttr, "TANGENT");
            if (pTANGENT)
                newPrimitive.attributes.TANGENT = static_cast<int>(json::getLong(pTANGENT));

            auto pPOSITION = json::searchNode(oAttr, "POSITION");
            if (pPOSITION)
                newPrimitive.attributes.POSITION = static_cast<int>(json::getLong(pPOSITION));

            auto pTEXCOORD_0 = json::searchNode(oAttr, "TEXCOORD_0");
            if (pTEXCOORD_0)
                newPrimitive.attributes.TEXCOORD_0 = static_cast<int>(json::getLong(pTEXCOORD_0));

            auto* pJOINTS_0 = json::searchNode(oAttr, "JOINTS_0");
            if (pJOINTS_0)
                newPrimitive.attributes.JOINTS_0 = static_cast<int>(json::getLong(pJOINTS_0));

            auto* pWEIGHTS_0 = json::searchNode(oAttr, "WEIGHTS_0");
            if (pWEIGHTS_0)
                newPrimitive.attributes.WEIGHTS_0 = static_cast<int>(json::getLong(pWEIGHTS_0));
 
            aPrimitives.push(pAlloc, newPrimitive);
        }
 
        m_vMeshes.push(pAlloc, {.vPrimitives = aPrimitives, .sName = sName});
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

        auto pSource = json::searchNode(obj, "source");
        auto pSampler = json::searchNode(obj, "sampler");

        m_vTextures.push(pAlloc, {
            .sourceI = pSource ? static_cast<i32>(json::getLong(pSource)) : -1,
            .samplerI = pSampler ? static_cast<i32>(json::getLong(pSampler)) : -1
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

        auto pName = json::searchNode(obj, "name");
        if (pName)
            newMaterial.sName = String(pAlloc, json::getString(pName));

        auto pPbrMetallicRoughness = json::searchNode(obj, "pbrMetallicRoughness");
        if (pPbrMetallicRoughness)
        {
            auto& oPbr = json::getObject(pPbrMetallicRoughness);

            {
                auto pBaseColorTexture = json::searchNode(oPbr, "baseColorTexture");
                if (pBaseColorTexture)
                {
                    auto& objBct = json::getObject(pBaseColorTexture);

                    auto pIndex = json::searchNode(objBct, "index");
                    if (!pIndex)
                    {
                        LOG_BAD("index field is required\n");
                        return false;
                    }

                    newMaterial.pbrMetallicRoughness.baseColorTexture.index = json::getLong(pIndex);
                }
            }

            {
                auto pBaseColorFactor = json::searchNode(oPbr, "baseColorFactor");
                if (pBaseColorFactor)
                    newMaterial.pbrMetallicRoughness.baseColorFactor = assignUnionType(pBaseColorFactor, 4).VEC4;
            }
        }

        auto pNormalTexture = json::searchNode(obj, "normalTexture");
        if (pNormalTexture)
        {
            auto& objNT = json::getObject(pNormalTexture);
            auto pIndex = json::searchNode(objNT, "index");
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

        auto pUri = json::searchNode(obj, "uri");
        if (pUri)
            m_vImages.push(pAlloc, {.sUri = String(pAlloc, json::getString(pUri))});
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

        auto pName = json::searchNode(obj, "name");
        if (pName)
            newNode.sName = String(pAlloc, json::getString(pName));

        auto pCamera = json::searchNode(obj, "camera");
        if (pCamera)
            newNode.cameraI = static_cast<int>(json::getLong(pCamera));

        auto pChildren = json::searchNode(obj, "children");
        if (pChildren)
        {
            auto& arrChil = json::getArray(pChildren);
            for (auto& c : arrChil)
                newNode.vChildren.push(pAlloc, static_cast<int>(json::getLong(&c)));
        }

        auto pMatrix = json::searchNode(obj, "matrix");
        if (pMatrix)
        {
            newNode.uTransformation.matrix = assignUnionType(pMatrix, 4*4).MAT4;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::MATRIX;
        }

        auto pMesh = json::searchNode(obj, "mesh");
        if (pMesh)
            newNode.meshI = static_cast<int>(json::getLong(pMesh));

        auto pSkin = json::searchNode(obj, "skin");
        if (pSkin)
            newNode.skinI = static_cast<int>(json::getLong(pSkin));

        auto pTranslation = json::searchNode(obj, "translation");
        if (pTranslation)
        {
            auto ut = assignUnionType(pTranslation, 3);
            newNode.uTransformation.animation.translation = ut.VEC3;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::ANIMATION;
        }

        auto pRotation = json::searchNode(obj, "rotation");
        if (pRotation)
        {
            auto ut = assignUnionType(pRotation, 4);
            newNode.uTransformation.animation.rotation.base = ut.VEC4;
            newNode.eTransformationType = Node::TRANSFORMATION_TYPE::ANIMATION;
        }

        auto pScale = json::searchNode(obj, "scale");
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
Model::procAnimations(IAllocator* pAlloc)
{
    if (!m_toplevelObjs.pAnimations)
        return true;

    const auto* pAnimations = m_toplevelObjs.pAnimations;
    const auto& aAnimations = json::getArray(pAnimations); /* usually an array of one object */

    for (auto& animation : aAnimations)
    {
        auto& obj = json::getObject(&animation);

        Animation newAnim {};

        auto* pName = json::searchNode(obj, "name");
        if (pName)
            newAnim.sName = String(pAlloc, json::getString(pName));

        auto* pChannelsObj = json::searchNode(obj, "channels");
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

            auto* pSampler = json::searchNode(channelObj, "sampler");
            if (!pSampler)
            {
                LOG_BAD("'sampler' object is required\n");
                return false;
            }

            auto sampler = json::getLong(pSampler);
            newChannel.samplerI = sampler;

            auto* pTarget = json::searchNode(channelObj, "target");
            if (!pTarget)
            {
                LOG_BAD("'target' object is required\n");
                return false;
            }

            auto& targetObj = json::getObject(pTarget);

            Animation::Channel::Target newTarget {};

            auto pNode = json::searchNode(targetObj, "node");
            if (pNode)
            {
                newTarget.nodeI = static_cast<int>(json::getLong(pNode));
            }

            auto pPath = json::searchNode(targetObj, "path");
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

        auto* pSamplers = json::searchNode(obj, "samplers");
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

            auto* pInput = json::searchNode(samplerObj, "input");
            if (!pInput)
            {
                LOG_BAD("'input' is required\n");
                return false;
            }

            newSampler.inputI = static_cast<int>(json::getLong(pInput));

            auto* pInterpolation = json::searchNode(samplerObj, "interpolation");
            if (pInterpolation)
                newSampler.eInterpolation = AnimationSamplerStringToPATH_TYPE(json::getString(pInterpolation));

            auto* pOutput = json::searchNode(samplerObj, "output");
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
