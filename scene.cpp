#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QKeyEvent>
#include <random>
#include "scene.h"

#define GoodBorders(x, y) (-1 < (x) && (x) < cR && -1 < (y) && (y) < cC)

#define SetWhite setRgb(240, 240, 240)
#define SetBlack setRgb( 70,  70,  70)
#define SetRed   setRgb(200, 100, 100)
#define SetBlue  setRgb(100, 100, 200)
#define SetPink  setRgb(210, 100, 220)
#define SetGreen setRgb(100, 240, 100)

#define SetCamera(x, y) Scene::setSceneRect(-150 + (x) * 8, -110 + (y) * 8, 500, 400);

const int MaxR = 64, MaxC = 64, FOG = 4;
const int STP[4][2] = {{ 0,-1},{-1, 0},{ 0,+1},{+1, 0}};
const char ROAD = '#', END = '%', KEY = '$', EYE = '0', VOID = ' ', GETBONUS = '-';

static char Map[MaxR][MaxC];
static int cR, cC, PassMazes;
static int nkeys, keys;
static bool HaveEye;

void Start()
{
    srand(time(NULL));

    cR = cC = 10;
    PassMazes = 0;
}

bool DeadEnd(int x, int y)                                                      // Is deadend
{
    bool nb = 0;
    for(int i = 0; i < 4; i++)
    {
        if (GoodBorders(x + STP[i][0], y + STP[i][1]) &&
            Map[x + STP[i][0]][y + STP[i][1]] != VOID)
        {
            if (nb) return 0;
            nb = 1;
        }
    }
    return nb;
}

void Scene::MakeNewMaze()
{
    QGraphicsScene::clear();                                                    // Cleaning
    for (int i = 0; i < cR; i++)
    {
        for (int j = 0; j < cC; j++) Map[i][j] = VOID;
    }

    int Qu[MaxR * MaxC][2], qb = 0, qe = 0;                                     // Maze (queue)
    int x = cR / 2, y = cC / 2;
    Map[x][y] = ROAD;
    Qu[qe][0] = x;
    Qu[qe][1] = y;
    qe++;
    while(qb < qe)
    {
        for(int rp = 0, i; rp < 4; rp++)
        {
            i = rand() % 4;
            int cx = Qu[qb][0] + STP[i][0],
                cy = Qu[qb][1] + STP[i][1];

            if (GoodBorders(cx, cy) && Map[cx][cy] != ROAD &&
                DeadEnd(cx, cy))
            {
                Map[cx][cy] = ROAD;

                Qu[qe][0] = cx;
                Qu[qe][1] = cy;
                qe++;
            }
        }
        qb++;
    }

    x = Qu[qb - 1][0];                                                          // End
    y = Qu[qb - 1][1];
    qb--;
    Map[x][y] = END;

    nkeys = 0;                                                                  // Keys and bonuses
    for( ; 0 < qb && nkeys != 3; qb--)
    {
        int cx = Qu[qb][0], cy = Qu[qb][1];
        if (Map[cx][cy] == ROAD)
        {
            if (DeadEnd(cx, cy) && rand() % 10 == 0)
            {
                Map[cx][cy] = KEY;
                nkeys++;
            }
            else
            {
                if (rand() % 491 == 0)
                {
                    Map[cx][cy] = EYE;
                }
            }
        }
    }
    keys = 0;
    HaveEye = 0;
}

void Scene::Output()
{
    int radius = 10;                                                            // Print map
    QColor color;
    for (int i = 0; i < cR; i++)
    {
        for (int j = 0; j < cC; j++)
        {
            QGraphicsItem *crr;
            crr = new QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius);

            switch (Map[i][j])
            {
            case ROAD: color.SetWhite; break;
            case END : color.SetRed;   break;
            case KEY : color.SetBlue;  break;
            case EYE : color.SetPink;  break;
            }

            if (Map[i][j] != VOID)
            {
                static_cast<QGraphicsEllipseItem*>(crr)->setBrush(color);       // Set color and pos
                crr->setPos(i * 20, j * 20);

                Scene::addItem(crr);                                            // Show
            }
        }
    }

    color.SetGreen;                                                             // Print hero
    Scene::m_activeItem = new QGraphicsEllipseItem(-radius, -radius, 2*radius, 2*radius);
    static_cast<QGraphicsEllipseItem*>(Scene::m_activeItem)->setBrush(color);

    Scene::addItem(Scene::m_activeItem);
    Scene::m_activeItem->setPos((cR / 2) * 20, (cC / 2) * 20);

    SetCamera(cR / 2, cC / 2);
}

Scene::Scene(QObject *parent)                                                   // Add scene
  : QGraphicsScene(parent),
    m_activeItem(nullptr)
{
    Scene::MakeNewMaze();
    Scene::Output();
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    int x = m_activeItem->x() / 20,                                             // Hero coord
        y = m_activeItem->y() / 20,
        i = -1;

    switch (event->key())                                                       // If we can go
    {
    case Qt::Key::Key_W:
        if (GoodBorders(x, y - 1) && Map[x][y - 1] != VOID) i = 0;
        break;
    case Qt::Key::Key_A:
        if (GoodBorders(x - 1, y) && Map[x - 1][y] != VOID) i = 1;
        break;
    case Qt::Key::Key_S:
        if (GoodBorders(x, y + 1) && Map[x][y + 1] != VOID) i = 2;
        break;
    case Qt::Key::Key_D:
        if (GoodBorders(x + 1, y) && Map[x + 1][y] != VOID) i = 3;
        break;
    }

    if (i != -1)
    {
        m_activeItem->moveBy(STP[i][0] * 20, STP[i][1] * 20);                   // Move hero
        if (Map[x][y] == GETBONUS)
        {
            Map[x][y] = ROAD;

            QGraphicsItem *crr;
            crr = Scene::itemAt(x * 20, y * 20, QTransform());

            QColor color;
            color.SetWhite;
            static_cast<QGraphicsEllipseItem*>(crr)->setBrush(color);
        }
        x += STP[i][0];
        y += STP[i][1];

        SetCamera(x, y);                                                        // Move camera

        switch (Map[x][y])                                                      // Checking the cage
        {
        case END:
            if (keys == nkeys)
            {
                cR++; cC++;
                PassMazes++;

                Scene::MakeNewMaze();
                Scene::Output();
            }
            break;
        case EYE:
            HaveEye = true;
            Map[x][y] = GETBONUS;
            break;
        case KEY:
            keys++;
            Map[x][y] = GETBONUS;
            break;
        }
    }
}
