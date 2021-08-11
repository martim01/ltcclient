#pragma once
#include <chrono>
#include <list>

class Offset
{
    public:
        Offset();
        std::pair<bool, double> Add(std::chrono::microseconds offset, unsigned char nFrame, double dFPS);
        void ClearData();

    private:
        void WorkoutLR();
        double GetAverage();

        void CrashTime(double dOffset);

        std::list<double> m_lstOffset;
        std::list<double> m_lstFrame;


        double m_dFPS;
        size_t m_nFrame;

        bool m_bSlewing;
};
