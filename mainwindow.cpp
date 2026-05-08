#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "gamewindow.h"
#include <QPalette>
#include <QImage>
#include <QBrush>
#include <QResizeEvent>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QLabel>
#include <QFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("游戏窗口");
    setFixedSize(1280, 720);
    setBackground();
    
    initButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setBackground()
{
    QImage image("background.jpg");
    if (!image.isNull()) {
        QPalette palette;
        palette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        this->setPalette(palette);
    }
}

int MainWindow::loadHighScore()
{
    QFile file("highscore.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        file.close();
        bool ok;
        int score = line.toInt(&ok);
        return ok ? score : 0;
    }
    return 0;
}

void MainWindow::saveHighScore(int score)
{
    QFile file("highscore.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << score;
        file.close();
    }
}

void MainWindow::initButtons()
{
    QPushButton *startBtn = new QPushButton("开始游戏", this);
    QPushButton *exitBtn = new QPushButton("退出游戏", this);
    
    QFont btnFont;
    btnFont.setFamily("Comic Sans MS");
    btnFont.setBold(true);
    btnFont.setPointSize(16);
    
    startBtn->setFont(btnFont);
    exitBtn->setFont(btnFont);
    
    startBtn->setStyleSheet("QPushButton {"
                           "color: white;"
                           "background-color: rgba(255, 100, 150, 0.8);"
                           "border-radius: 20px;"
                           "padding: 12px 30px;"
                           "border: none;"
                           "}"
                           "QPushButton:hover {"
                           "background-color: rgba(255, 100, 150, 1.0);"
                           "}"
                           "QPushButton:pressed {"
                           "background-color: rgba(200, 80, 120, 1.0);"
                           "}");
    
    exitBtn->setStyleSheet("QPushButton {"
                          "color: white;"
                          "background-color: rgba(100, 150, 255, 0.8);"
                          "border-radius: 20px;"
                          "padding: 12px 30px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: rgba(100, 150, 255, 1.0);"
                          "}"
                          "QPushButton:pressed {"
                          "background-color: rgba(80, 120, 200, 1.0);"
                          "}");
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(startBtn);
    btnLayout->addWidget(exitBtn);
    btnLayout->setSpacing(100);
    
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addStretch(1);
    mainLayout->addLayout(btnLayout);
    mainLayout->setAlignment(btnLayout, Qt::AlignBottom | Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, -25);
    mainLayout->setSpacing(5);
    
    setCentralWidget(centralWidget);
    
    connect(startBtn, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(exitBtn, &QPushButton::clicked, this, &MainWindow::close);
}

void MainWindow::onStartGame()
{
    GameWindow *gameWindow = new GameWindow();
    gameWindow->show();
    this->hide();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    setBackground();
}