/* https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#properties-reference */

#pragma once

#include "types.hh"

#include "json/Parser.hh"

namespace gltf
{

struct Model
{
    struct
    {
        json::Object* pAsset;
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
    Asset m_asset {}; /* REQUIRED */
    Scene m_rootScene {};
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
    bool procToplevelObjs(adt::IAllocator* pAlloc, const json::Parser& parser);
    bool procAsset(adt::IAllocator* pAlloc);
    bool procRootScene(adt::IAllocator* pAlloc);
    bool procScenes(adt::IAllocator* pAlloc);
    bool procBuffers(adt::IAllocator* pAlloc);
    bool procBufferViews(adt::IAllocator* pAlloc);
    bool procAccessors(adt::IAllocator* pAlloc);
    bool procMeshes(adt::IAllocator* pAlloc);
    bool procTexures(adt::IAllocator* pAlloc);
    bool procMaterials(adt::IAllocator* pAlloc);
    bool procImages(adt::IAllocator* pAlloc);
    bool procNodes(adt::IAllocator* pAlloc);
    bool procAnimations(adt::IAllocator* pAlloc);
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
