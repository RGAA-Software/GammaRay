//
// Created by RGAA on 2023-12-26.
//

#include <QApplication>
#include <QSurfaceFormat>
#include <QFontDatabase>

#include "thunder_sdk.h"
#include "client_context.h"
#include "workspace.h"
#include "application.h"

using namespace tc;

int main(int argc, char** argv) {

#ifdef WIN32
    //CaptureDump();
#endif

#ifdef __APPLE__
    QSurfaceFormat fmt;
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setVersion(3, 3);
    fmt.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(fmt);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
#endif

    QApplication app(argc, argv);

    // font
#if 0
    auto id = QFontDatabase::addApplicationFont(":/resources/font/quixotic-1.otf");
    qDebug() << "font family : " << QFontDatabase::applicationFontFamilies(id) ;

    QFont font;
    font.setPointSize(10);
    qApp->setFont(font);
#endif

    auto ctx = std::make_shared<ClientContext>("ui");
    ctx->Init(false);
    Application application(ctx);
    application.show();

    return app.exec();
}