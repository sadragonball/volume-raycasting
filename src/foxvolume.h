#pragma once

#include <iostream>
#include <vector>
#include <string>

class FoxReadError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class FoxVolume
{
public:
    FoxVolume();

    FoxVolume(const std::string& filename);

    virtual ~FoxVolume();

    void load_volume(const std::string& filename);

    /**
     * @brief uint8_normalized
     * Cast the data to uint8 and normalize it to [0,255]
     */
    void uint8_normalized();

    uint8_t* data_ptr() const {
        return m_data;
    }

    const std::pair<double, double> range() const{
        return m_range;
    }

    const std::tuple<size_t, size_t, size_t> size() const {
        return m_size;
    }

    const std::tuple<float , float, float> origin(void) const {
        return m_origin;
    }

    const std::tuple<float ,float ,float> spacing() const {
        return m_spacing;
    }

private:
    enum class DataType {Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64, Uint64, Float, Double};

    std::tuple<size_t, size_t, size_t> m_size; //number of voxels for each axis.
    std::tuple<float , float, float> m_origin; //Origin, in voxel coordinates.
    std::tuple<float, float, float> m_spacing; //Spacing between voxels.
    DataType m_datatype;
    std::pair<double, double> m_range;
    uint8_t* m_data;

//    void read_dimensions(const std::vector<std::string> &header);
//    void read_origin(const std::vector<std::string>& header);
//    void read_spacing(const std::vector<std::string>& header);
//    void read_data_type(const std::vector<std::string> &header);

};
