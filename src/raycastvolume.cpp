#include "raycastvolume.h"
#include "vtkvolume.h"
#include "foxvolume.h"

#include <QRegularExpression>

#include <algorithm>
#include <cmath>


/*!
 * \brief Create a two-unit cube mesh as the bounding box for the volume.
 */
RayCastVolume::RayCastVolume(void)
    : m_volume_texture {0}
    , m_noise_texture {0}
    , m_cube_vao {
          {
              -1.0f, -1.0f,  1.0f,
               1.0f, -1.0f,  1.0f,
               1.0f,  1.0f,  1.0f,
              -1.0f,  1.0f,  1.0f,
              -1.0f, -1.0f, -1.0f,
               1.0f, -1.0f, -1.0f,
               1.0f,  1.0f, -1.0f,
              -1.0f,  1.0f, -1.0f,
          },
          {
              // front
              0, 1, 2,
              0, 2, 3,
              // right
              1, 5, 6,
              1, 6, 2,
              // back
              5, 4, 7,
              5, 7, 6,
              // left
              4, 0, 3,
              4, 3, 7,
              // top
              2, 6, 7,
              2, 7, 3,
              // bottom
              4, 5, 1,
              4, 1, 0,
          }
      }
{
    initializeOpenGLFunctions();
}


/*!
 * \brief Destructor.
 */
RayCastVolume::~RayCastVolume()
{
    glDeleteTextures(1, &m_noise_texture);
}


/*!
 * \brief Load a volume from file.
 * \param File to be loaded.
 */
void RayCastVolume::load_volume(const QString& filename) {
    uint8_t* data;

    QRegularExpression re {"^.*\\.([^\\.]+)$"};
    QRegularExpressionMatch match = re.match(filename);

    if (!match.hasMatch()) {
        throw std::runtime_error("Cannot determine file extension.");
    }

    const std::string extension {match.captured(1).toLower().toStdString()};
    if ("vtk" == extension) {
        FoxVolume volume {filename.toStdString()};
//        volume.uint8_normalized();
        m_size = QVector3D(std::get<0>(volume.size()), std::get<1>(volume.size()), std::get<2>(volume.size()));
        m_origin = QVector3D(std::get<0>(volume.origin()), std::get<1>(volume.origin()), std::get<2>(volume.origin()));
        m_spacing = QVector3D(std::get<0>(volume.spacing()), std::get<1>(volume.spacing()), std::get<2>(volume.spacing()));
        m_range = volume.range();
        data = volume.data_ptr();
    }
    else {
        throw std::runtime_error("Unrecognised extension '" + extension + "'.");
    }

    glDeleteTextures(1, &m_volume_texture);
    glGenTextures(1, &m_volume_texture);
    glBindTexture(GL_TEXTURE_3D, m_volume_texture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // The array on the host has 1 byte alignment
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, m_size.x(), m_size.y(), m_size.z(), 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_3D, 0);
}


/*!
 * \brief Create a noise texture with the size of the viewport.
 */
void RayCastVolume::create_noise(void)
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    const int width = viewport[2];
    const int height = viewport[3];

    std::srand(std::time(NULL));

    auto noise_texture = new GLubyte[width * height]();

    for (auto *p = noise_texture; p <= noise_texture + width * height; ++p) {
        *p = std::rand() % 256;
    }

    glDeleteTextures(1, &m_noise_texture);
    glGenTextures(1, &m_noise_texture);
    glBindTexture(GL_TEXTURE_2D, m_noise_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, noise_texture);
    glBindTexture(GL_TEXTURE_2D, 0);
}


/*!
 * \brief Render the bounding box.
 */
void RayCastVolume::paint(void)
{
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_3D, m_volume_texture);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, m_noise_texture);

    m_cube_vao.paint();
}


/*!
 * \brief Range of the image, in intensity value.
 * \return A pair, holding <minimum, maximum>.
 */
std::pair<double, double> RayCastVolume::range() {
    return m_range;
}


/*!
 * \brief Scale factor to model space.
 *
 * Scale the bounding box such that the longest side equals 1.
 */
float RayCastVolume::scale_factor(void)
{
    auto e = m_size * m_spacing;
    return std::max({e.x(), e.y(), e.z()});
}
