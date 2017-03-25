#ifndef PERFORMANCERATE_H
#define PERFORMANCERATE_H

#include <QTime>
#include <QElapsedTimer>
#include <queue>

class PerformanceRate
{
public:
    PerformanceRate();

    void startTimer();

    float getFPS();

protected:
    // fps
    QTime                               m_time;

    int                                 m_frameCount;

    int                                 m_starttime;

    bool                                m_first;
    float                               m_fps;

    std::queue<float>                   m_windowFPS;
    float                               m_windowSUM;
};

#endif // PERFORMANCERATE_H
