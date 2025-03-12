/* https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#properties-reference */

#pragma once

#include "types.hh"

#include "json/Parser.hh"

#include "adt/View.hh"

namespace gltf
{

struct Model
{
    adt::StringView m_sPath {};
    Asset m_asset {}; /* REQUIRED */
    int m_defaultSceneI = -1;
    adt::Vec<Scene> m_vScenes {};
    adt::Vec<Buffer> m_vBuffers {};
    adt::Vec<BufferView> m_vBufferViews {};
    adt::Vec<Accessor> m_vAccessors {};
    adt::Vec<Mesh> m_vMeshes {};
    adt::Vec<Texture> m_vTextures {};
    adt::Vec<Material> m_vMaterials {};
    adt::Vec<Image> m_vImages {};
    adt::Vec<Node> m_vNodes {};
    adt::Vec<Animation> m_vAnimations {};
    adt::Vec<Skin> m_vSkins {};

    /* */

    Model() = default;

    /* */

    bool read(adt::IAllocator* pAlloc, const json::Parser& parsed, const adt::StringView svPath); /* clones uri */

    /* NOTE: (unsafe) make sure T is the correct type, and accessorI isn't out of bounds. */
    template<typename T>
    adt::View<T>
    accessorView(int accessorI) const
    {
        auto& acc = m_vAccessors[accessorI];
        auto& view = m_vBufferViews[acc.bufferViewI];
        auto& buff = m_vBuffers[view.bufferI];
    
        return {reinterpret_cast<const T*>(&buff.sBin[acc.byteOffset + view.byteOffset]),
            acc.count, view.byteStride
        };
    }

    template<typename T>
    adt::Span<T>
    accessorSpan(int accessorI) const
    {
        auto& acc = m_vAccessors[accessorI];
        auto& view = m_vBufferViews[acc.bufferViewI];
        auto& buff = m_vBuffers[view.bufferI];
    
        return {reinterpret_cast<const T*>(&buff.sBin[acc.byteOffset + view.byteOffset]),
            acc.count
        };
    }

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
