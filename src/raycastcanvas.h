#pragma once

#include <functional>
#include <vector>

#include <QtMath>

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>

#include "mesh.h"
#include "raycastvolume.h"
#include "trackball.h"
#include "vtkvolume.h"

/*!
 * \brief Class for a raycasting canvas widget.
 */
class RayCastCanvas : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit RayCastCanvas(QWidget *parent = nullptr);
    ~RayCastCanvas();

    void setStepLength(const GLfloat step_length) {
        m_stepLength = step_length;
        update();
    }

    void setVolume(const QString& volume) {
        m_raycasting_volume->load_volume(volume);
        update();
    }

    void setThreshold(const double threshold) {
        auto range = m_raycasting_volume ? getRange() : std::pair<double, double>{0.0, 1.0};
        m_threshold = threshold / (range.second - range.first);
        update();
    }

    void setMode(const QString& mode) {
        m_active_mode = mode;
        update();
    }

    void setBackground(const QColor& colour) {
        m_background = colour;
        update();
    }

    std::vector<QString> getModes(void) {
        std::vector<QString> modes;
        for (const auto& [key, val] : m_modes) {
            modes.push_back(key);
        }
        return modes;
    }

    QColor getBackground(void) {
        return m_background;
    }

    std::pair<double, double> getRange(void) {
        return m_raycasting_volume->range();
    }

signals:
    // NOPE

public slots:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent * event);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

private:

    QMatrix4x4 m_viewMatrix;
    QMatrix4x4 m_modelViewProjectionMatrix;
    QMatrix3x3 m_normalMatrix;

    const GLfloat m_fov = 60.0f;                                          /*!< Vertical field of view. */
    const GLfloat m_focalLength = 1.0 / qTan(M_PI / 180.0 * m_fov / 2.0); /*!< Focal length. */
    GLfloat m_aspectRatio;                                                /*!< width / height */

    QVector2D m_viewportSize;
    QVector3D m_rayOrigin; /*!< Camera position in model space coordinates. */

    QVector3D m_lightPosition {3.0, 0.0, 3.0};    /*!< In camera coordinates. */
    QVector3D m_diffuseMaterial {1.0, 1.0, 1.0};  /*!< Material colour. */
    GLfloat m_stepLength;                         /*!< Step length for ray march. */
    GLfloat m_threshold;                          /*!< Isosurface intensity threshold. */
    QColor m_background;                          /*!< Viewport background colour. */

    const GLfloat m_gamma = 2.2f; /*!< Gamma correction parameter. */

    RayCastVolume *m_raycasting_volume;

    std::map<QString, QOpenGLShaderProgram*> m_shaders;
    std::map<QString, std::function<void(void)>> m_modes;
    QString m_active_mode;

    TrackBall m_trackBall {};       /*!< Trackball holding the model rotation. */
    TrackBall m_scene_trackBall {}; /*!< Trackball holding the scene rotation. */

    GLint m_distExp = -200;

    GLuint scaled_width();
    GLuint scaled_height();

    void raycasting(const QString& shader);

    QPointF pixel_pos_to_view_pos(const QPointF& p);
    void create_noise(void);
    void add_shader(const QString& name, const QString& vector, const QString& fragment);
};
