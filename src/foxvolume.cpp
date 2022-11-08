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
                      size_t skip = 0,
                      bool is_little_endian = false) {



    file.seekg(skip, std::ios::beg);

    file.read(reinterpret_cast<char*>(image_data), element_count * sizeof(T));
    //    T voxel;
    //    T* p = image_data;
    //    not binary
    //    for (size_t i = 0; i < element_count; ++i) {
    //        file >> voxel;
    //        *p = voxel;
    //        p++;
    //    }

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
               uint8_t* normalized_data,
               size_t element_count) {
    T* p = reinterpret_cast<T*>(data);
    for(size_t i = 0; i < element_count; ++i) {
        normalized_data[i] = static_cast<uint8_t>(
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
    m_origin = {0., 0., 0.};
    m_datatype = DataType::Uint16;
    m_spacing = {0.1, 0.1, 0.1};
    m_size = {2000, 3072, 600};

    load_volume(filename);
}


FoxVolume::~FoxVolume() {}


void FoxVolume::uint8_normalized() {
    size_t element_count = std::get<0>(m_size) * std::get<1>(m_size) * std::get<2>(m_size);
    void* normalized_data = malloc(element_count);
    switch(m_datatype) {
    case FoxVolume::DataType::Uint8:
        normalize<uint8_t>(m_data, m_range,static_cast<uint8_t*>(normalized_data),element_count);
        break;
    case FoxVolume::DataType::Uint16:
        normalize<uint16_t>(m_data, m_range,static_cast<uint8_t*>(normalized_data),element_count);
        break;
    }

    delete m_data;
    m_data = static_cast<uint8_t*>(normalized_data);
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
            read_data<uint8_t>(file,voxel_count, m_data, m_range, 0);
            break;
        }
    {
    case FoxVolume::DataType::Uint16:
        {
            m_data = new uint8_t[voxel_count*2]();
            read_data<uint16_t>(file,voxel_count, m_data, m_range, 0);
            break;
        }
    }
    }

    file.close();
}


