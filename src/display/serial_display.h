#pragma once

#include "display_base.h"

class SerialDisplay : public DisplayBase {
public:
    explicit SerialDisplay(unsigned long baudRate = 115200);

    bool init() override;
    void showConnecting(const String& ssid) override;
    void showConnected(const String& ip) override;
    void showLoading() override;
    void showArrivals(const String& stopId, const String& systemTime, const std::vector<ArrivalPrediction>& arrivals) override;
    void showEmpty(const String& stopId, const String& systemTime) override;
    void showError(const String& errorMessage) override;

private:
    unsigned long _baudRate;
};
