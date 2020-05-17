#include "scene.cpp"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Scene scene;                                                                // Make new scene
    QColor clr;
    clr.SetBlack;
    scene.setBackgroundBrush(clr);

    QGraphicsView vw(&scene);                                                   // Show scene
    vw.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vw.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vw.setWindowTitle("Maze");
    vw.show();

    return a.exec();
}
