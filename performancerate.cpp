#include "performancerate.h"
#include <QDebug>

PerformanceRate::PerformanceRate()
{
    m_frameCount = 0;
    m_starttime = 0;
    m_fps = 0.0f;
    m_first = true;
}

void PerformanceRate::startTimer()
{
    m_time.start();
}

float PerformanceRate::getFPS()
{
   if (m_first) {
        m_frameCount = 0;
        m_starttime = m_time.elapsed();
        m_first = false;
        return -1.0;
   }

   m_frameCount++;

   if (m_time.elapsed() - m_starttime > 1000 && m_frameCount > 10) {
       m_fps = ((double) m_frameCount * 1000.0)/ (double)(m_time.elapsed() - m_starttime );
       qDebug() << "FPS: " << m_fps;
       m_starttime = m_time.elapsed();
       m_frameCount = 0;
       return m_fps;
   }

   return -1.0;
}
