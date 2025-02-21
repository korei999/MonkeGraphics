/* https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#properties-reference */

#pragma once

#include "types.hh"

#include "json/Parser.hh"

namespace gltf
{

struct Model
{
    adt::StringView m_sPath {};
    Asset m_asset {}; /* REQUIRED */
    DefaultScene m_defaultScene {};
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
    adt::VecBase<Skin> m_vSkins {};

    /* */

    Model() = default;

    /* */

    bool read(adt::IAllocator* pAlloc, const json::Parser& parsed, const adt::StringView svPath); /* clones uri */

    /* */

private:
    struct
    {
        json::Node* pAsset;
        json::Node* pScene;
        json::Node* pScenes;
        json::Node* pNodes;
        json::Node* pMeshes;
        json::Node* pCameras;
        json::Node* pBuffers;
        json::Node* pBufferViews;
        json::Node* pAccessors;
        json::Node* pMaterials;
        json::Node* pTextures;
        json::Node* pImages;
        json::Node* pSamplers;
        json::Node* pSkins;
        json::Node* pAnimations;
    } m_toplevelObjs {};

    bool procToplevelObjs(adt::IAllocator* pAlloc, const json::Parser& parser);
    bool procAsset(adt::IAllocator* pAlloc);
    bool procRootScene(adt::IAllocator* pAlloc);
    bool procScenes(adt::IAllocator* pAlloc);
    bool procBuffers(adt::IAllocator* pAlloc);
    bool procBufferViews(adt::IAllocator* pAlloc);
    bool procAccessors(adt::IAllocator* pAlloc);
    bool procSkins(adt::IAllocator* pAlloc);
    bool procMeshes(adt::IAllocator* pAlloc);
    bool procTexures(adt::IAllocator* pAlloc);
    bool procMaterials(adt::IAllocator* pAlloc);
    bool procImages(adt::IAllocator* pAlloc);
    bool procNodes(adt::IAllocator* pAlloc);
    bool procAnimations(adt::IAllocator* pAlloc);
};

} /* namespace gltf */
