#pragma once

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <random>
#include "scene.h"

#define GoodBorders(x, y) (-1 < (x) && (x) < cR && -1 < (y) && (y) < cC)
#define Circle(R) QGraphicsEllipseItem(-(R),-(R), 2*(R), 2*(R))
#define SetWhite setRgb(240, 240, 240, 0)
#define SetBlack setRgb(  0,   0,   0)
#define SetRed   setRgb(255, 120, 120, 0)
#define SetBlue  setRgb(100, 100, 255, 0)
#define SetPink  setRgb(210,   0, 220, 0)
#define SetGreen setRgb( 20, 230,  20, 0)
#define SetCamera(x, y) Scene::setSceneRect(CenterX + x, CenterY + y, 500, 400);

const int MaxR = 256, MaxC = 256, MaxKeys = 5, FOG = 2;
const int STP[4][2] = {{ 0,-1},{-1, 0},{ 0,+1},{+1, 0}};
const char ROAD = '#', END = '%', KEY = '$', EYE = '0', VOID = ' ', GETBONUS = '-';

static char Map[MaxR][MaxC];
static int cR, cC, PassMazes, cFOG, nkeys, keys;
static int CenterX, CenterY, EndX, EndY;

bool DeadEnd(int x, int y)                                                              // Is deadend
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

void MakeNewMaze()
{
    for (int i = 0; i < cR; i++)                                                        // Cleaning
    {
        for (int j = 0; j < cC; j++) Map[i][j] = VOID;
    }

    int Qu[MaxR * MaxC][2], qb = 0, qe = 0;                                             // Maze (queue)
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

    EndX = Qu[qb - 1][0];                                                               // Set end
    EndY = Qu[qb - 1][1];
    qb--;
    Map[EndX][EndY] = END;

    nkeys = 0;                                                                          // Keys and bonuses
    for( ; 0 < qb && nkeys < MaxKeys; qb--)
    {
        int cx = Qu[qb][0], cy = Qu[qb][1];
        if (Map[cx][cy] == ROAD)
        {
            if (DeadEnd(cx, cy) &&
                rand() % 7 == 0)
            {
                Map[cx][cy] = KEY;
                nkeys++;
            }
            else if (rand() % 997 == 0)
            {
                Map[cx][cy] = EYE;
            }
        }
    }
    keys = 0;
}

Scene::Scene(QObject *parent)                                                           // New scene
  : QGraphicsScene(parent), m_activeItem(nullptr)
{
    srand(time(NULL));

    cR = cC = 10;
    cFOG = FOG;
    PassMazes = 0;

    MakeNewMaze();
    MakeScene();
}

void Scene::NeedKeys()                                                                  // How many keys are left
{
    QPointF P(EndX * 20 - 6, EndY * 20 - 11);

    if (keys != 0)
    {
        QGraphicsItem *crr;
        crr = Scene::itemAt(P, QTransform());
        Scene::removeItem(crr);
    }

    QGraphicsTextItem *NKeys;
    NKeys = new QGraphicsTextItem(QString::number(nkeys - keys));
    NKeys->setPos(P);

    Scene::addItem(NKeys);
}

void Scene::UpdateScene(int x, int y)                                                   // Show visble
{
    int Zone = cFOG + 1;                                                                // Show road(+remove light)
    for(int i = -Zone; i <= Zone; i++)
    {
        for(int j = -Zone; j <= Zone; j++)
        {
            if (GoodBorders(x + i, y + j))
            {
                int dist = abs(i) + abs(j);
                if (dist <= cFOG)
                {
                    QColor clr;
                    QList <QGraphicsItem*> L;
                    L = Scene::items(QPointF((x + i) * 20, (y + j) * 20));

                    if (dist <= cFOG / 2) dist = 0;
                    else
                    {
                        if (cFOG < FOG + 3) dist *= 50;
                        else dist *= 30;
                    }

                    if (L.size() == 1)                                                  // Road
                    {
                        clr.SetWhite; clr.setAlpha(255 - dist);
                        static_cast<QGraphicsEllipseItem*>(L.back())->setBrush(clr);
                    }
                    else if (L.size() == 2)                                             // Road + bonus/key
                    {
                        if (Map[x + i][y + j] == KEY) clr.SetBlue;
                        else if (Map[x + i][y + j] == EYE) clr.SetPink;
                        else if (i == j && i == 0)
                        {
                            clr.SetGreen;
                        }

                        clr.setAlpha(255 - dist);
                        static_cast<QGraphicsEllipseItem*>(*L.begin())->setBrush(clr);

                        clr.SetWhite; clr.setAlpha(255 - dist);
                        static_cast<QGraphicsEllipseItem*>(L.back())->setBrush(clr);
                    }
                    else if (L.size() == 3)                                             // Road + end + nkeys
                    {
                        clr.SetRed; clr.setAlpha(255 - dist);
                        static_cast<QGraphicsEllipseItem*>((*++L.begin()))->setBrush(clr);
                    }
                }
                else
                {
                    QList <QGraphicsItem*> L;
                    L = Scene::items(QPointF((x + i) * 20, (y + j) * 20));

                    QColor clr; clr.SetWhite;

                    if (L.size() == 1)
                    {
                        static_cast<QGraphicsEllipseItem*>(L.back())->setBrush(clr);
                    }
                    else if (L.size() == 2)
                    {
                        static_cast<QGraphicsEllipseItem*>(*L.begin())->setBrush(clr);
                        static_cast<QGraphicsEllipseItem*>(L.back())->setBrush(clr);
                    }
                    else if (L.size() == 3)
                    {
                        static_cast<QGraphicsEllipseItem*>((*++L.begin()))->setBrush(clr);
                    }
                }
            }
        }
    }

    SetCamera(x, y);                                                                    // Move camera
}

void Scene::MakeScene()
{
    QGraphicsScene::clear();

    int radius = 10;                                                                    // Print map
    for (int i = 0; i < cR; i++)
    {
        for (int j = 0; j < cC; j++)
        {
            if (Map[i][j] == VOID) continue;

            QGraphicsItem *crr = new Circle(radius);
            crr->setPos(i * 20, j * 20);

            Scene::addItem(crr);                                                        // ---

            if (Map[i][j] == ROAD) continue;

            QGraphicsItem *crr1;
            crr1 = new Circle(radius * (Map[i][j] == END ? 1 : 0.4));
            crr1->setPos(i * 20, j * 20);

            Scene::addItem(crr1);
        }
    }

    Scene::m_activeItem = new Circle(radius * 0.7);                                     // Add hero
    Scene::m_activeItem->setPos((cR / 2) * 20, (cC / 2) * 20);
    Scene::addItem(Scene::m_activeItem);

    Scene::NeedKeys();                                                                  // Output nkeys

    CenterX = -cR * (12 - cR / 2);                                                      // Set camera
    CenterY = -cC * (10 - cC / 2);

    UpdateScene(cR / 2, cC / 2);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    int x = m_activeItem->x() / 20,                                                     // Hero coord and next step
        y = m_activeItem->y() / 20,
        io;

    int KeyE = event->key();                                                            // Check press
    if (KeyE == Qt::Key::Key_W || KeyE == Qt::Key::Key_Up) io = 0;
    else if (KeyE == Qt::Key::Key_A || KeyE == Qt::Key::Key_Left) io = 1;
    else if (KeyE == Qt::Key::Key_S || KeyE == Qt::Key::Key_Down) io = 2;
    else if (KeyE == Qt::Key::Key_D || KeyE == Qt::Key::Key_Right) io = 3;
    else return;

    if (GoodBorders(x + STP[io][0], y + STP[io][1]) &&                                  // If we can go
        Map[x + STP[io][0]][y + STP[io][1]] != VOID)
    {
        m_activeItem->moveBy(STP[io][0] * 20, STP[io][1] * 20);                         // Move hero

        x += STP[io][0];
        y += STP[io][1];

        if (Map[x - STP[io][0]][y - STP[io][1]] == GETBONUS)                            // Erase key or bonus
        {
            Map[x - STP[io][0]][y - STP[io][1]] = ROAD;

            QGraphicsItem *crr;
            crr = Scene::itemAt((x - STP[io][0]) * 20, (y - STP[io][1]) * 20, QTransform());

            Scene::removeItem(crr);
        }

        UpdateScene(x, y);

        switch (Map[x][y])                                                              // Checking the cage
        {
        case END:
            if (keys == nkeys)
            {
                cR++; cC++;
                PassMazes++;

                MakeNewMaze();
                MakeScene();
            }
            break;
        case EYE:
            if (cFOG < FOG + 3) cFOG++;
            Map[x][y] = GETBONUS;
            break;
        case KEY:
            keys++;
            Map[x][y] = GETBONUS;
            Scene::NeedKeys();
            break;
        }
    }
}
