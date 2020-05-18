#pragma once

#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMessageBox>
#include <random>
#include "scene.h"

#define GoodBorders(x, y) (-1 < (x) && (x) < cR && -1 < (y) && (y) < cC)
#define Circle(R) QGraphicsEllipseItem(-(R),-(R), 2*(R), 2*(R))
#define SetColor(item, clr) static_cast<QGraphicsEllipseItem*>(item)->setBrush(clr);
#define SetWhite setRgb(240, 240, 240, 0)
#define SetBlack setRgb(  0,   0,   0)
#define SetRed   setRgb(255, 120, 120, 0)
#define SetBlue  setRgb(100, 100, 255, 0)
#define SetPink  setRgb(210,   0, 220, 0)
#define SetGreen setRgb( 20, 230,  20, 0)
#define SetCamera(x, y) setSceneRect(CenterX + 2*(x), CenterY + 2*(y), 500, 400);

const int MaxR = 64, MaxC = 64, MaxKeys = 5, FOG = 2;
const int STP[4][2] = {{ 0,-1},{-1, 0},{ 0,+1},{+1, 0}};
const char ROAD = '#', END = '%', KEY = '$', EYE = '0', VOID = ' ', GETBONUS = '-';

static char Map[MaxR][MaxC];
static int cR, cC, PassMazes, cFOG, nkeys;
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

            if (GoodBorders(cx, cy) && Map[cx][cy] != ROAD &&                           // If we can go
                DeadEnd(cx, cy))
            {
                Map[cx][cy] = ROAD;                                                     // Add cage

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
                rand() % 19 == 0)
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
}

Scene::Scene(QObject *parent)                                                           // New scene
  : QGraphicsScene(parent), m_activeItem(nullptr)
{
    srand(time(nullptr));

    cR = cC = 8;
    PassMazes = 0;
    cFOG = FOG;

    MakeNewMaze();
    MakeScene();
}

void Scene::NeedKeys()                                                                  // How many keys are left
{
    QPointF P(EndX * 20 - 7, EndY * 20 - 11);

    QGraphicsItem *crr = itemAt(P, QTransform());                                       // Erase old info(if can)
    if (crr != nullptr) removeItem(crr);

    QGraphicsTextItem *NKeys;                                                           // Write new
    NKeys = new QGraphicsTextItem(QString::number(nkeys));
    NKeys->setPos(P);

    addItem(NKeys);
}

void Scene::UpdateScene(int x, int y)                                                   // Show visible
{
    int Zone = cFOG + 1;                                                                // Show road(+remove old light)
    for(int i = -Zone; i <= Zone; i++)
    {
        for(int j = -Zone; j <= Zone; j++)
        {
            if (GoodBorders(x + i, y + j))
            {
                QColor clr;
                QList <QGraphicsItem*> L = items(QPointF((x + i) * 20, (y + j) * 20));
                int dist = abs(i) + abs(j);

                if (dist <= cFOG)
                {
                    if (dist <= cFOG / 2) dist = 0;
                    else
                    {
                        if (cFOG < FOG + 3) dist *= 50;
                        else dist *= 30;
                    }

                    if (L.size() == 1)                                                  // Road
                    {
                        clr.SetWhite; clr.setAlpha(255 - dist);
                        SetColor(L.back(), clr);
                    }
                    else if (L.size() == 2)                                             // Road + bonus|key
                    {
                        if (Map[x + i][y + j] == KEY) clr.SetBlue;
                        else if (Map[x + i][y + j] == EYE) clr.SetPink;
                        else if (i == j && i == 0) clr.SetGreen;

                        clr.setAlpha(255 - dist);
                        SetColor(*L.begin(), clr);

                        clr.SetWhite;
                        clr.setAlpha(255 - dist);
                        SetColor(L.back(), clr);
                    }
                    else if (L.size() == 3)                                             // Road + end + nkeys
                    {
                        clr.SetRed; clr.setAlpha(255 - dist);
                        SetColor(*++L.begin(), clr);
                    }
                }
                else
                {
                    clr.SetWhite;                                                       // Set void

                    if (L.size() == 1)
                    {
                        SetColor(L.back(), clr);
                    }
                    else if (L.size() == 2)
                    {
                        SetColor(*L.begin(), clr);
                        SetColor(L.back(), clr);
                    }
                    else if (L.size() == 3)
                    {
                        SetColor(*++L.begin(), clr);
                    }
                }
            }
        }
    }

    SetCamera(x, y);                                                                    // Move camera
}

void Scene::MakeScene()
{
    clear();

    int radius = 10;                                                                    // Set map
    for (int i = 0; i < cR; i++)
    {
        for (int j = 0; j < cC; j++)
        {
            if (Map[i][j] == VOID) continue;

            QGraphicsItem *crr = new Circle(radius);
            crr->setPos(i * 20, j * 20);

            addItem(crr);                                                               // Set road

            if (Map[i][j] == ROAD) continue;

            crr = new Circle(radius * (Map[i][j] == END ? 1 : 0.4));                    // End|key|bonus
            crr->setPos(i * 20, j * 20);

            addItem(crr);
        }
    }

    m_activeItem = new Circle(radius * 0.7);                                            // Set hero
    m_activeItem->setPos((cR / 2) * 20, (cC / 2) * 20);
    addItem(m_activeItem);

    NeedKeys();                                                                         // Output nkeys

    CenterX = -cR * 18 + PassMazes * 27;                                                // Center the scene
    CenterY = -cC * 14 + PassMazes * 23;
    UpdateScene(cR / 2, cC / 2);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    int KeyE = event->key(), io;                                                        // Check press
    if (KeyE == Qt::Key_W || KeyE == Qt::Key_Up) io = 0;
    else if (KeyE == Qt::Key_A || KeyE == Qt::Key_Left) io = 1;
    else if (KeyE == Qt::Key_S || KeyE == Qt::Key_Down) io = 2;
    else if (KeyE == Qt::Key_D || KeyE == Qt::Key_Right) io = 3;
    else return;

    int x = m_activeItem->x() / 20,                                                     // Hero coord
        y = m_activeItem->y() / 20;

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
            crr = itemAt((x - STP[io][0]) * 20, (y - STP[io][1]) * 20, QTransform());

            removeItem(crr);
        }

        UpdateScene(x, y);

        switch (Map[x][y])                                                              // Checking the cage
        {
        case END:
            if (nkeys == 0)
            {
                cR++; cC++;
                PassMazes++;

                if (PassMazes == 40)
                {
                    QMessageBox MWin;
                    MWin.setWindowTitle("Victory");
                    MWin.setText("YOU WIN!");
                    MWin.exec();

                    exit(0);
                }

                MakeNewMaze();
                MakeScene();
            }
            break;
        case EYE:
            if (cFOG < FOG + 3) cFOG++;
            Map[x][y] = GETBONUS;
            break;
        case KEY:
            nkeys--;
            NeedKeys();
            Map[x][y] = GETBONUS;
            break;
        }
    }
}
