#include "foxvolume.h"

#include <fstream>

/*!
 * \brief Invert the endianness of the input value.
 * \param p Pointer to the first byte of an input value of size `N`.
 */
template<size_t N>
void swap_byte_order(uint8_t* p)
{
    uint8_t tmp;
    for (size_t i = 0; i < N / 2; ++i) {
        tmp = p[i];
        p[i] = p[N-i-1];
        p[N-i-1] = tmp;
    }
}


template<typename T>
static void read_data(std::ifstream& file,
                      const size_t element_count,
                      uint8_t* image_data,
                      std::pair<double, double>& range,
                      bool is_little_endian = false) {


    T voxel;
    T* p = image_data;
    for (size_t i = 0; i < element_count; ++i) {
        file >> voxel;
        *p = voxel;
        p++;
    }

    // Find range
    range = {std::numeric_limits<double>::max(), std::numeric_limits<double>::min()};
    for (size_t i = 0; i < element_count; ++i) {
        if (image_data[i] > range.second) {
            range.second = image_data[i];
        }
        if (image_data[i] < range.first) {
            range.first = image_data[i];
        }
    }
}

template<typename T>
void normalize(uint8_t* data,
               std::pair<double,double>& range,
               size_t element_count) {
    T* p = reinterpret_cast<T*>(data);
    for(size_t i = 0; i < element_count; ++i) {
        p[i] = static_cast<uint8_t>(
                    255 *
                    (static_cast<double>(p[i]) - range.first) /
                    (range.second - range.first)
                    );
    }
}
FoxVolume::FoxVolume()
{

}

FoxVolume::FoxVolume(const std::string& filename) {
    m_data = nullptr;
    m_origin = {0.,0.,0.};
    m_datatype = DataType::Uint8;
    m_spacing = {0.1, 0.1, 0.1};
    m_size = {256,256,256};

    load_volume(filename);
}


FoxVolume::~FoxVolume() {}


void FoxVolume::uint8_normalized() {
    size_t element_count = std::get<0>(m_size) * std::get<1>(m_size) * std::get<2>(m_size);
    switch(m_datatype) {
    case FoxVolume::DataType::Uint8:
        normalize<uint8_t>(m_data, m_range,element_count);
        break;
    }
}


void FoxVolume::load_volume(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if(!file.is_open()) {
        throw FoxReadError("File is already open.");
    }

    size_t voxel_count = std::get<0>(m_size) * std::get<1>(m_size) * std::get<2>(m_size);
    switch(m_datatype) {
    case FoxVolume::DataType::Uint8:
    {
        m_data = new uint8_t[voxel_count]();
        read_data<uint8_t>(file,voxel_count, m_data, m_range);
        break;
    }
    }

    file.close();
}


