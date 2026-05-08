#include "gamewindow.h"
#include <QPalette>
#include <QImage>
#include <QBrush>
#include <QResizeEvent>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QRandomGenerator>
#include <QtConcurrent>
#include <QKeyEvent>
#include <QFile>
#include <QTextStream>
#include <Windows.h>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , countdown(3)
    , gameTime(60)
    , dropSpeed(1000)
    , score(0)
    , moveLeft(false)
    , moveRight(false)
{
    setWindowTitle("游戏窗口");
    setFixedSize(1280, 720);
    setBackground();
    
    initCountdown();
}

GameWindow::~GameWindow()
{
}

void GameWindow::setBackground()
{
    QImage image("game_background.jpg");
    if (!image.isNull()) {
        QPalette palette;
        palette.setBrush(this->backgroundRole(), QBrush(image.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        this->setPalette(palette);
    }
}

void GameWindow::initCountdown()
{
    QWidget *darkOverlay = new QWidget(this);
    darkOverlay->setObjectName("countdownOverlay");
    darkOverlay->setFixedSize(1280, 720);
    darkOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.7);");
    darkOverlay->show();
    
    QLabel *countdownLabel = new QLabel(darkOverlay);
    countdownLabel->setStyleSheet("font-size: 120px; color: white; font-weight: bold;");
    countdownLabel->setAlignment(Qt::AlignCenter);
    countdownLabel->setGeometry(0, 0, 1280, 720);
    countdownLabel->show();
    
    QTimer *countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, [=]() mutable {
        if (countdown > 0) {
            countdownLabel->setText(QString::number(countdown));
            QtConcurrent::run([=]() {
                Beep(440, 150);
            });
            countdown--;
        } else {
            countdownTimer->stop();
            darkOverlay->deleteLater();
            startGame();
        }
    });
    countdownTimer->start(1000);
    
    QtConcurrent::run([=]() {
        Beep(440, 150);
    });
}

void GameWindow::startGame()
{
    QFont timerFont;
    timerFont.setFamily("Comic Sans MS");
    timerFont.setBold(true);
    timerFont.setPointSize(18);
    
    timerLabel = new QLabel(this);
    timerLabel->setGeometry(20, 20, 300, 50);
    timerLabel->setText("剩余时间: 60秒");
    timerLabel->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 0.5); border-radius: 10px; padding: 5px;");
    timerLabel->setFont(timerFont);
    timerLabel->show();
    
    scoreLabel = new QLabel(this);
    scoreLabel->setGeometry(950, 20, 200, 50);
    scoreLabel->setText("得分: 0");
    scoreLabel->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 0.5); border-radius: 10px; padding: 5px;");
    scoreLabel->setFont(timerFont);
    scoreLabel->show();
    
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, [=]() {
        gameTime--;
        if (gameTime > 0) {
            timerLabel->setText("剩余时间: " + QString::number(gameTime) + "秒");
        } else {
            gameTimer->stop();
            timerLabel->setText("时间到!");
            dropTimer->stop();
            moveTimer->stop();
            pauseAllRiceBalls();
            showGameOverDialog();
        }
    });
    gameTimer->start(1000);
    
    dropTimer = new QTimer(this);
    connect(dropTimer, &QTimer::timeout, this, &GameWindow::dropRiceBall);
    dropTimer->start(dropSpeed);
    
    jiyi = new QLabel(this);
    QPixmap jiyiPixmap("jiyi.png");
    if (!jiyiPixmap.isNull()) {
        jiyi->setPixmap(jiyiPixmap.scaled(100, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        jiyi->setText("🐱");
        jiyi->setStyleSheet("font-size: 100px;");
    }
    jiyi->setGeometry(590, 580, 100, 120);
    jiyi->show();
    
    moveTimer = new QTimer(this);
    connect(moveTimer, &QTimer::timeout, [=]() {
        if (moveLeft && jiyi->x() > 0) {
            jiyi->move(jiyi->x() - playerSpeed, jiyi->y());
        }
        if (moveRight && jiyi->x() < 1180) {
            jiyi->move(jiyi->x() + playerSpeed, jiyi->y());
        }
        checkCollision();
    });
    moveTimer->start(16);
    
    QPushButton *pauseBtn = new QPushButton(this);
    pauseBtn->setGeometry(1170, 10, 100, 80);
    pauseBtn->setText("▶");
    pauseBtn->setStyleSheet("QPushButton {"
                           "color: white;"
                           "background-color: transparent;"
                           "font-size: 60px;"
                           "border: none;"
                           "}"
                           "QPushButton:hover {"
                           "color: rgba(255, 255, 255, 0.8);"
                           "}");
    pauseBtn->show();
    
    connect(pauseBtn, &QPushButton::clicked, this, &GameWindow::showPauseDialog);
}

void GameWindow::dropRiceBall()
{
    QLabel *riceBall = new QLabel(this);
    QPixmap pixmap("fantuan.png");
    if (!pixmap.isNull()) {
        riceBall->setPixmap(pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        riceBall->setText("🍙");
        riceBall->setStyleSheet("font-size: 40px;");
        riceBall->setAlignment(Qt::AlignCenter);
    }
    
    int x = QRandomGenerator::global()->bounded(1200) + 40;
    riceBall->setGeometry(x, -60, 60, 60);
    riceBall->show();
    
    QTimer *moveTimer = new QTimer(this);
    moveTimers.append(moveTimer);
    moveTimer->setProperty("riceBall", QVariant::fromValue((void*)riceBall));
    int currentY = -60;
    
    int baseSpeed = 3;
    int speedIncrease = (60 - gameTime) / 10;
    int speed = baseSpeed + speedIncrease + QRandomGenerator::global()->bounded(2);
    
    connect(moveTimer, &QTimer::timeout, [=]() mutable {
        currentY += speed;
        riceBall->move(x, currentY);
        
        if (currentY > 720) {
            moveTimer->stop();
            moveTimers.removeOne(moveTimer);
            riceBall->deleteLater();
            moveTimer->deleteLater();
        }
    });
    
    moveTimer->start(16);
}

void GameWindow::showPauseDialog()
{
    gameTimer->stop();
    dropTimer->stop();
    moveTimer->stop();
    pauseAllRiceBalls();
    
    QWidget *darkOverlay = new QWidget(this);
    darkOverlay->setObjectName("pauseOverlay");
    darkOverlay->setFixedSize(1280, 720);
    darkOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    darkOverlay->show();
    
    QDialog *pauseDialog = new QDialog(this);
    pauseDialog->setWindowTitle("游戏暂停");
    pauseDialog->setFixedSize(500, 150);
    pauseDialog->setStyleSheet("background-color: rgba(255, 255, 255, 0.9); border-radius: 15px;");
    
    QPushButton *continueBtn = new QPushButton("继续游戏", pauseDialog);
    QPushButton *exitBtn = new QPushButton("返回首页", pauseDialog);
    
    QFont btnFont;
    btnFont.setFamily("Comic Sans MS");
    btnFont.setBold(true);
    btnFont.setPointSize(16);
    
    continueBtn->setFont(btnFont);
    exitBtn->setFont(btnFont);
    
    continueBtn->setStyleSheet("QPushButton {"
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
    btnLayout->addWidget(continueBtn);
    btnLayout->addWidget(exitBtn);
    btnLayout->setSpacing(30);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(pauseDialog);
    mainLayout->addStretch();
    mainLayout->addLayout(btnLayout);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    connect(continueBtn, &QPushButton::clicked, [=]() {
        pauseDialog->close();
        darkOverlay->deleteLater();
        initCountdown();
    });
    
    connect(exitBtn, &QPushButton::clicked, [=]() {
        pauseDialog->close();
        this->close();
    });
    
    pauseDialog->exec();
}

void GameWindow::playTone(int frequency, int duration)
{
    Beep(frequency, duration);
}

void GameWindow::pauseAllRiceBalls()
{
    foreach(QTimer *timer, moveTimers) {
        if (timer->isActive()) {
            timer->stop();
        }
    }
}

void GameWindow::resumeAllRiceBalls()
{
    foreach(QTimer *timer, moveTimers) {
        if (!timer->isActive()) {
            timer->start(16);
        }
    }
}

void GameWindow::updateScore()
{
    if (scoreLabel) {
        scoreLabel->setText("得分: " + QString::number(score));
    }
}

void GameWindow::checkCollision()
{
    if (!jiyi) return;
    
    QRect playerRect = jiyi->geometry();
    
    QList<QLabel*> ballsToRemove;
    
    foreach(QObject *child, children()) {
        QLabel *riceBall = qobject_cast<QLabel*>(child);
        if (riceBall && riceBall != jiyi && riceBall != timerLabel && riceBall != scoreLabel) {
            QRect ballRect = riceBall->geometry();
            if (playerRect.intersects(ballRect)) {
                score += 10;
                updateScore();
                playTone(600, 100);
                
                QTimer *timer = nullptr;
                foreach(QTimer *t, moveTimers) {
                    void *ballPtr = t->property("riceBall").value<void*>();
                    if (ballPtr == riceBall) {
                        timer = t;
                        break;
                    }
                }
                if (timer) {
                    timer->stop();
                    moveTimers.removeOne(timer);
                    timer->deleteLater();
                }
                
                ballsToRemove.append(riceBall);
            }
        }
    }
    
    foreach(QLabel *ball, ballsToRemove) {
        ball->deleteLater();
    }
}

void GameWindow::showGameOverDialog()
{
    QWidget *darkOverlay = new QWidget(this);
    darkOverlay->setObjectName("gameOverOverlay");
    darkOverlay->setFixedSize(1280, 720);
    darkOverlay->setStyleSheet("background-color: rgba(0, 0, 0, 0.5);");
    darkOverlay->show();
    
    QDialog *gameOverDialog = new QDialog(this);
    gameOverDialog->setWindowTitle("游戏结束");
    gameOverDialog->setFixedSize(700, 200);
    gameOverDialog->setStyleSheet("background-color: rgba(255, 255, 255, 0.9); border-radius: 15px;");
    
    int highScore = 0;
    QFile file("highscore.txt");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        file.close();
        bool ok;
        highScore = line.toInt(&ok);
        if (!ok) highScore = 0;
    }
    
    if (score > highScore) {
        highScore = score;
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << highScore;
            file.close();
        }
    }
    
    QLabel *scoreLabel = new QLabel("本次得分: " + QString::number(score) + "  |  历史最高: " + QString::number(highScore), gameOverDialog);
    scoreLabel->setStyleSheet("font-size: 24px; color: black; text-align: center;");
    scoreLabel->setAlignment(Qt::AlignCenter);
    scoreLabel->setGeometry(50, 30, 600, 50);
    scoreLabel->show();
    
    QPushButton *restartBtn = new QPushButton("再来一局", gameOverDialog);
    QPushButton *exitBtn = new QPushButton("返回首页", gameOverDialog);
    
    QFont btnFont;
    btnFont.setFamily("Comic Sans MS");
    btnFont.setBold(true);
    btnFont.setPointSize(16);
    
    restartBtn->setFont(btnFont);
    exitBtn->setFont(btnFont);
    
    restartBtn->setStyleSheet("QPushButton {"
                             "color: white;"
                             "background-color: rgba(255, 100, 150, 0.8);"
                             "border-radius: 20px;"
                             "padding: 12px 40px;"
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
                          "padding: 12px 40px;"
                          "border: none;"
                          "}"
                          "QPushButton:hover {"
                          "background-color: rgba(100, 150, 255, 1.0);"
                          "}"
                          "QPushButton:pressed {"
                          "background-color: rgba(80, 120, 200, 1.0);"
                          "}");
    
    restartBtn->setGeometry(90, 100, 260, 60);
    exitBtn->setGeometry(350, 100, 260, 60);
    restartBtn->show();
    exitBtn->show();
    
    connect(restartBtn, &QPushButton::clicked, [=]() {
        gameOverDialog->close();
        darkOverlay->deleteLater();
        
        foreach(QObject *child, children()) {
            QLabel *ball = qobject_cast<QLabel*>(child);
            if (ball && ball != jiyi && ball != timerLabel && ball != scoreLabel) {
                ball->deleteLater();
            }
        }
        
        foreach(QTimer *timer, moveTimers) {
            timer->stop();
            timer->deleteLater();
        }
        moveTimers.clear();
        
        if (jiyi) {
            jiyi->deleteLater();
            jiyi = nullptr;
        }
        
        if (timerLabel) {
            timerLabel->deleteLater();
            timerLabel = nullptr;
        }
        
        moveLeft = false;
        moveRight = false;
        
        score = 0;
        gameTime = 60;
        dropSpeed = 1000;
        updateScore();
        
        initCountdown();
    });
    
    connect(exitBtn, &QPushButton::clicked, [=]() {
        gameOverDialog->close();
        this->close();
    });
    
    gameOverDialog->exec();
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        moveLeft = true;
    } else if (event->key() == Qt::Key_Right) {
        moveRight = true;
    }
}

void GameWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left) {
        moveLeft = false;
    } else if (event->key() == Qt::Key_Right) {
        moveRight = false;
    }
}

void GameWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    setBackground();
}