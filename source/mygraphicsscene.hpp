#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include "dog.hpp"
#include "bark_wave.hpp"
#include "utils.hpp"
#include "button_box.hpp"

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <QtGui/QKeyEvent>

class MyGraphicsScene : public QGraphicsScene
{
public:
    MyGraphicsScene();
    MyGraphicsScene(Dog *p_dog);
    ~MyGraphicsScene();

    void init(void *pLevel);
    void reset();

    void pauseGame(bool showRetryButton = true);
    void resumeGame();
    void timerEvent(QTimerEvent *event);

    void updateZoom(qreal zoomFactor);
    QPointF getLevelPos(QPointF scenePos);

    void dogBark(Dog *pDog);

    void drunkMode(bool activate);

    void showMessageBox(const QString &text, const QList<QString> *pButtonTextList,
                        void (*pButtonHandler)(ButtonContext*), void *pContext);

    bool startZooming();
    bool stopZooming();

#ifdef ANDROID_MOBILE_VERSION
    void closeApp(QCloseEvent *pEvent);
#endif

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent);

    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

    void keyPressEvent(QKeyEvent *p_event);
    void keyReleaseEvent(QKeyEvent *p_event);

#ifndef DEBUG_OUTPUT
    void focusOutEvent(QFocusEvent *pFocusEvent);
#endif

private:
    void moveDog();

    void handleDrunkMode();

    bool m_movingDog;
    bool m_draggingDog;

    void *p_level;

    QPointF m_clickPoint;

    int m_zoomTimer;
    int m_gameTimer;

    QList<BarkWave*> m_barkWaveList;

    bool m_inTimerHandler;

    bool m_drunkMode;

    QGraphicsPixmapItem *p_viewPixmap;

    // Button graphics
    MyButtonBox m_buttonBox;
};

#endif // MYGRAPHICSSCENE_H
