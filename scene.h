#ifndef SCENE_H
#define SCENE_H

#include <QGraphicsScene>

class Scene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit Scene(QObject *parent = nullptr);
    void MakeNewMaze();
    void Output();

    QGraphicsItem *m_activeItem;

protected:
    void keyPressEvent(QKeyEvent *event) override;

};

#endif // SCENE_H
