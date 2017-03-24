#ifndef PERFORMANCERATE_H
#define PERFORMANCERATE_H

#include <QTime>
#include <QElapsedTimer>

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
};

#endif // PERFORMANCERATE_H
