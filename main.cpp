#include "scene.cpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Start();

    Scene scene;
    QGraphicsView vw(&scene);

    QColor clr;
    clr.SetBlack;
    scene.setBackgroundBrush(clr);

    vw.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vw.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    vw.show();

    a.exec();
}
