#pragma once

class SignalBus;
class QwiicTwistService;

class PomodoroService
{
public:
    PomodoroService(QwiicTwistService* pTwistSvc);

    void setup();
    void loop(SignalBus& sb);

private:
    QwiicTwistService* m_pTwistSvc;
};
