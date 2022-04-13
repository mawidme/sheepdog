#include <QtGui>
#include <QApplication>
#include <QMessageBox>
#include <QDesktopWidget>

#include "main_game.hpp"
#include "utils.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    gApplication = &app;

    // seed random generator with current time in milliseconds
    srand(QTime::currentTime().msec());

    QRect screenRect = app.desktop()->availableGeometry();
    qDebug("screen size: %d x %d", screenRect.width(), screenRect.height());

    // Screen rotation may be in portrait mode when starting, so set gScreenWidth to the
    // larger value found for screen resolution.
    if(screenRect.width() > screenRect.height()) {
        gScreenWidth = screenRect.width();
        gScreenHeight = screenRect.height();
    } else {
        gScreenWidth = screenRect.height();
        gScreenHeight = screenRect.width();
    }

    if(gScreenWidth < kMinScreenWidth || gScreenHeight < kMinScreenHeight) {
        QMessageBox msgBox;
        QString text = QString("Minimum screen size of %1x%2 required! Only detected %3x%4").
                       arg(kMinScreenWidth).arg(kMinScreenHeight).arg(gScreenWidth).arg(gScreenHeight);
        msgBox.setText(text);
        msgBox.exec();
        return -1;
    }

#ifdef MOBILE_VERSION
	gWindowWidth = gScreenWidth;
	gWindowHeight = gScreenHeight;
#else
    gWindowWidth = kDefaultWindowWidth;
    gWindowHeight = kDefaultWindowHeight;
#endif

    // Adjust info elements' dimensions according to screen resolution.
    setGlobals(true);

    // seed random generator with current time
    qsrand(QTime::currentTime().msec());

    MainGame game;
    if(!game.init())
        return -1;

    return app.exec();
}
