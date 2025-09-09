#pragma once

#include <QOpenGLWidget>
#include <QOpenGLWindow>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLWindow>
#include <QResizeEvent>
#include <mutex>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QVBoxLayout>

#include "tc_message.pb.h"
#include "tc_client_sdk_new/gl/raw_image.h"
#include "client/front_render/ct_video_widget.h"

namespace tc
{
    class Data;
    class Sprite;
    class RawImage;
    class Director;
    class ClientContext;
    class ShaderProgram;
    class Statistics;
    class ThunderSdk;
    class Settings;

    class OpenGLVideoWidget : public QOpenGLWidget, public QOpenGLFunctions_3_3_Core, public VideoWidget {
    public:

        explicit OpenGLVideoWidget(const std::shared_ptr<ClientContext> &ctx, const std::shared_ptr<ThunderSdk> &sdk,
                                   int dup_idx, RawImageFormat format, QWidget *parent = nullptr);
        ~OpenGLVideoWidget() override;
        QWidget * AsWidget() override;
        void RefreshImage(const std::shared_ptr<RawImage> &image) override;
        void RefreshCursor(int x, int y, int tex_left, int text_right, int hpx, int hpy, const std::shared_ptr<RawImage> &cursor);
        void Exit();

    protected:
        void resizeEvent(QResizeEvent *event) override;
        void initializeGL() override;
        void paintGL() override;
        void resizeGL(int width, int height) override;
        void mouseMoveEvent(QMouseEvent *) override;
        void mousePressEvent(QMouseEvent *) override;
        void mouseReleaseEvent(QMouseEvent *) override;
        void mouseDoubleClickEvent(QMouseEvent *) override;
        void wheelEvent(QWheelEvent *event) override;
        void keyPressEvent(QKeyEvent *event) override;
        void keyReleaseEvent(QKeyEvent *event) override;
        void OnUpdate() override;

    private:
        void InitRGBATexture();
        void InitI420Texture();
        void InitI444Texture();

    private:
        Settings* settings_ = nullptr;
        std::shared_ptr<ClientContext> context_ = nullptr;
        glm::mat4 model_;
        std::shared_ptr<ShaderProgram> shader_program_ = nullptr;
        GLuint vao_{0};
        GLuint vbo_{0};
        GLuint ibo_{0};

        GLuint y_texture_id_ = 0;
        GLuint uv_texture_id_ = 0;
        GLuint rgb_texture_id_ = 0;
        GLuint u_texture_id_ = 0;
        GLuint v_texture_id_ = 0;

        std::shared_ptr<Director> director_ = nullptr;
        std::shared_ptr<Sprite> cursor_ = nullptr;
        std::shared_ptr<Sprite> logo_ = nullptr;

        int render_fps_ = 0;
        uint64_t last_update_fps_time_ = 0;
        
    };
}