#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>


class VTKReadError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};


/*!
 * \brief Represent a VTK volume.
 *
 * This class allows to load a VTK volume from file.
 */
class VTKVolume {

public:

    /*!
     * \brief Create an empty volume.
     */
    VTKVolume(void);

    /*!
     * \brief Create a volume from file.
     * \param filename File to be loaded.
     */
    VTKVolume(const std::string& filename);

    /*!
     * \brief Destructor.
     */
    virtual ~VTKVolume();

    /*!
     * \brief Load data from file, replacing any current data.
     * \param filename File to be loaded.
     */
    void load_volume(const std::string& filename);

    /*!
     * \brief Cast the data to `unsigned char` and normalise it to [0, 255];
     */
    void uint8_normalised(void);

    /*!
     * \brief Pointer to the data.
     */
    unsigned char * data_ptr(void) {
        return m_data.data();
    }

    /*!
     * \brief Get a copy of the data.
     */
    std::vector<unsigned char> data(void) {
        return m_data;
    }

    /*!
     * \brief Range of the image, in intensity value.
     * \return A pair, holding <minimum, maximum>.
     */
    const std::pair<double, double> range(void) const {
        return m_range;
    }

    /*!
     * \brief Range of the image, in intensity value.
     * \return A pair, holding <minimum, maximum>.
     */
    const std::tuple<size_t, size_t, size_t> size(void) const {
        return m_size;
    }

    /*!
     * \brief Range of the image, in intensity value.
     * \return A pair, holding <minimum, maximum>.
     */
    const std::tuple<float, float, float> origin(void) const {
        return m_origin;
    }

    /*!
     * \brief Range of the image, in intensity value.
     * \return A pair, holding <minimum, maximum>.
     */
    const std::tuple<float, float, float> spacing(void) const {
        return m_spacing;
    }

private:
    enum class DataType {Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64, Uint64, Float, Double};

    std::tuple<size_t, size_t, size_t> m_size;    /*!< Number of voxels for each axis. */
    std::tuple<float, float, float> m_origin;  /*!< Origin, in voxel coordinates. */
    std::tuple<float, float, float> m_spacing; /*!< Spacing between voxels. */
    DataType m_datatype;                          /*!< Data type. */
    std::pair<double, double> m_range;            /*!< (min, max) of the original intensities, before normalisation. */
    std::vector<unsigned char> m_data;            /*!< Volume data, casted to `unsigned char` and normalised to [0, 255]. */

    void read_dimensions(const std::vector<std::string> &header);
    void read_origin(const std::vector<std::string> &header);
    void read_spacing(const std::vector<std::string> &header);
    void read_data_type(const std::vector<std::string> &header);
};
