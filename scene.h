#pragma once

#include <QGraphicsScene>
#include <QGraphicsView>

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Scene(QObject *parent = nullptr);
    void NeedKeys();
    void MakeScene();
    void UpdateScene(int, int);

    QGraphicsItem *m_activeItem;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};
