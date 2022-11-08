#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

#include <QtWidgets>

#include "raycastcanvas.h"


/*!
 * \brief Convert a QColor to a QVector3D.
 * \return A QVector3D holding a RGB representation of the colour.
 */
QVector3D to_vector3d(const QColor& colour) {
    return QVector3D(colour.redF(), colour.greenF(), colour.blueF());
}


/*!
 * \brief Constructor for the canvas.
 * \param parent Parent widget.
 */
RayCastCanvas::RayCastCanvas(QWidget *parent)
    : QOpenGLWidget {parent}
    , m_raycasting_volume {nullptr}
{
    // Register the rendering modes here, so they are available to the UI when it is initialised
    m_modes["Isosurface"] = [&]() { RayCastCanvas::raycasting("Isosurface"); };
    m_modes["Alpha blending"] = [&]() { RayCastCanvas::raycasting("Alpha blending"); };
    m_modes["MIP"] = [&]() { RayCastCanvas::raycasting("MIP"); };
}


/*!
 * \brief Destructor.
 */
RayCastCanvas::~RayCastCanvas()
{
    for (auto& [key, val] : m_shaders) {
        delete val;
    }
    delete m_raycasting_volume;
}


/*!
 * \brief Initialise OpenGL-related state.
 */
void RayCastCanvas::initializeGL()
{
    initializeOpenGLFunctions();

    m_raycasting_volume = new RayCastVolume();
//    m_raycasting_volume->create_noise();

    add_shader("Isosurface", ":/shaders/isosurface.vert", ":/shaders/isosurface.frag");
    add_shader("Alpha blending", ":/shaders/alpha_blending.vert", ":/shaders/alpha_blending.frag");
    add_shader("MIP", ":/shaders/maximum_intensity_projection.vert", ":/shaders/maximum_intensity_projection.frag");
}


/*!
 * \brief Callback to handle canvas resizing.
 * \param w New width.
 * \param h New height.
 */
void RayCastCanvas::resizeGL(int w, int h)
{
    (void) w; (void) h;
    m_viewportSize = {(float) scaled_width(), (float) scaled_height()};
    m_aspectRatio = (float) scaled_width() / scaled_height();
    glViewport(0, 0, scaled_width(), scaled_height());
    //m_raycasting_volume->create_noise();
}


/*!
 * \brief Paint a frame on the canvas.
 */
void RayCastCanvas::paintGL()
{
    // Compute geometry
    m_viewMatrix.setToIdentity();
    m_viewMatrix.translate(0, 0, -4.0f * std::exp(m_distExp / 600.0f));
    m_viewMatrix.rotate(m_trackBall.rotation());

    m_modelViewProjectionMatrix.setToIdentity();
    m_modelViewProjectionMatrix.perspective(m_fov, (float)scaled_width()/scaled_height(), 0.1f, 100.0f);
    m_modelViewProjectionMatrix *= m_viewMatrix * m_raycasting_volume->modelMatrix();

    m_normalMatrix = (m_viewMatrix * m_raycasting_volume->modelMatrix()).normalMatrix();

    m_rayOrigin = m_viewMatrix.inverted() * QVector3D({0.0, 0.0, 0.0});

    // Perform raycasting
    m_modes[m_active_mode]();
    GLint memory_kb = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &memory_kb);
    GLint current_memory = 0;
    glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &current_memory);

    qDebug("%f/%f", (current_memory / float(1024 * 1024)), (memory_kb / float(1024 * 1024)));
}


/*!
 * \brief Width scaled by the pixel ratio (for HiDPI devices).
 */
GLuint RayCastCanvas::scaled_width()
{
    return devicePixelRatio() * width();
}


/*!
 * \brief Height scaled by the pixel ratio (for HiDPI devices).
 */
GLuint RayCastCanvas::scaled_height()
{
    return devicePixelRatio() * height();
}


/*!
 * \brief Perform isosurface raycasting.
 */
void RayCastCanvas::raycasting(const QString& shader)
{
    m_shaders[shader]->bind();
    {
        m_shaders[shader]->setUniformValue("ViewMatrix", m_viewMatrix);
        m_shaders[shader]->setUniformValue("ModelViewProjectionMatrix", m_modelViewProjectionMatrix);
        m_shaders[shader]->setUniformValue("NormalMatrix", m_normalMatrix);
        m_shaders[shader]->setUniformValue("aspect_ratio", m_aspectRatio);
        m_shaders[shader]->setUniformValue("focal_length", m_focalLength);
        m_shaders[shader]->setUniformValue("viewport_size", m_viewportSize);
        m_shaders[shader]->setUniformValue("ray_origin", m_rayOrigin);
        m_shaders[shader]->setUniformValue("top", m_raycasting_volume->top());
        m_shaders[shader]->setUniformValue("bottom", m_raycasting_volume->bottom());
        m_shaders[shader]->setUniformValue("background_colour", to_vector3d(m_background));
        m_shaders[shader]->setUniformValue("light_position", m_lightPosition);
        m_shaders[shader]->setUniformValue("material_colour", m_diffuseMaterial);
        m_shaders[shader]->setUniformValue("step_length", m_stepLength);
        m_shaders[shader]->setUniformValue("threshold", m_threshold);
        m_shaders[shader]->setUniformValue("gamma", m_gamma);
        m_shaders[shader]->setUniformValue("volume", 0);
        m_shaders[shader]->setUniformValue("jitter", 1);

        glClearColor(m_background.redF(), m_background.greenF(), m_background.blueF(), m_background.alphaF());
        glClear(GL_COLOR_BUFFER_BIT);

        m_raycasting_volume->paint();
    }
    m_shaders[shader]->release();
}


/*!
 * \brief Convert a mouse position into normalised canvas coordinates.
 * \param p Mouse position.
 * \return Normalised coordinates for the mouse position.
 */
QPointF RayCastCanvas::pixel_pos_to_view_pos(const QPointF& p)
{
    return QPointF(2.0 * float(p.x()) / width() - 1.0,
                   1.0 - 2.0 * float(p.y()) / height());
}


/*!
 * \brief Callback for mouse movement.
 */
void RayCastCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_trackBall.move(pixel_pos_to_view_pos(event->pos()), m_scene_trackBall.rotation().conjugated());
    } else {
        m_trackBall.release(pixel_pos_to_view_pos(event->pos()), m_scene_trackBall.rotation().conjugated());
    }
    update();
}


/*!
 * \brief Callback for mouse press.
 */
void RayCastCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_trackBall.push(pixel_pos_to_view_pos(event->pos()), m_scene_trackBall.rotation().conjugated());
    }
    update();
}


/*!
 * \brief Callback for mouse release.
 */
void RayCastCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_trackBall.release(pixel_pos_to_view_pos(event->pos()), m_scene_trackBall.rotation().conjugated());
    }
    update();
}


/*!
 * \brief Callback for mouse wheel.
 */
void RayCastCanvas::wheelEvent(QWheelEvent * event)
{
    m_distExp += event->delta();
    if (m_distExp < -1800)
        m_distExp = -1800;
    if (m_distExp > 600)
        m_distExp = 600;
    update();
}


/*!
 * \brief Add a shader.
 * \param name Name for the shader.
 * \param vertex Vertex shader source file.
 * \param fragment Fragment shader source file.
 */
void RayCastCanvas::add_shader(const QString& name, const QString& vertex, const QString& fragment)
{
    m_shaders[name] = new QOpenGLShaderProgram(this);
    m_shaders[name]->addShaderFromSourceFile(QOpenGLShader::Vertex, vertex);
    m_shaders[name]->addShaderFromSourceFile(QOpenGLShader::Fragment, fragment);
    m_shaders[name]->link();
}
