#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QTimer>
#include <QList>
#include <QLabel>

class GameWindow : public QMainWindow
{
    Q_OBJECT

signals:
    void returnToHome();

public:
    explicit GameWindow(QWidget *parent = nullptr);
    ~GameWindow() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    int countdown;
    int gameTime;
    int dropSpeed;
    int score;
    QTimer *dropTimer;
    QTimer *gameTimer;
    QTimer *moveTimer;
    QList<QTimer*> moveTimers;
    QLabel *scoreLabel;
    QLabel *timerLabel;
    QLabel *jiyi;
    bool moveLeft;
    bool moveRight;
    const int playerSpeed = 8;
    
    void setBackground();
    void initCountdown();
    void startGame();
    void dropRiceBall();
    void showPauseDialog();
    void playTone(int frequency, int duration);
    void pauseAllRiceBalls();
    void resumeAllRiceBalls();
    void updateScore();
    void checkCollision();
    void showGameOverDialog();
};

#endif // GAMEWINDOW_H