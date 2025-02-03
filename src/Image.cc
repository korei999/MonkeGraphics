#include "Image.hh"

#include "adt/utils.hh"

using namespace adt;

Image
Image::cloneToARGB(adt::IAllocator* pAlloc)
{
    Image nImg {
        .m_uData {},
        .m_width = m_width,
        .m_height = m_height,
        .m_eType = IMAGE_TYPE::ARGB,
    };

    nImg.m_uData.pARGB = pAlloc->zallocV<ImagePixelARGB>(m_width * m_height);

    switch (m_eType)
    {
        case IMAGE_TYPE::RGB:
        {
            for (ssize i = 0; i < m_width * m_height; ++i)
            {
                const auto& pix = m_uData.pRGB[i];

                nImg.m_uData.pARGB[i] = {
                    .b = pix.b,
                    .g = pix.g,
                    .r = pix.r,
                    .a = 255,
                };
            }
        }
        break;

        case IMAGE_TYPE::ARGB:
        {
            utils::copy(nImg.m_uData.pARGB, m_uData.pARGB, m_width * m_height);
        }
        break;
    }

    return nImg;
}
