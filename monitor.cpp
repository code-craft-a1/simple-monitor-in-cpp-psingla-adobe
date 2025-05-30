#include "./monitor.h"
#include <iostream>
#include <chrono>
#include <thread>

using std::cout, std::flush;
using std::chrono::seconds;
using std::this_thread::sleep_for;

enum VitalCondition {
    BELOW_RANGE,
    NEAR_LOW,
    WITHIN_RANGE,
    NEAR_HIGH,
    ABOVE_RANGE
};

struct VitalParameters {
    float lowerLimit;
    float upperLimit;
    float warningThresholdPercent;
    const char* vitalSign;
    const char* criticalAlert;
    const char* lowWarning;
    const char* highWarning;
};

float calculateTolerance(const VitalParameters& params) {
    return params.upperLimit * (params.warningThresholdPercent / 100.0f);
}

VitalCondition checkLowZone(float reading, float lowerLimit, float nearLowLimit) {
    if (reading < lowerLimit) return BELOW_RANGE;
    if (reading < nearLowLimit) return NEAR_LOW;
    return WITHIN_RANGE;
}

VitalCondition checkHighZone(float reading, float nearHighLimit, float upperLimit) {
    if (reading <= nearHighLimit) return WITHIN_RANGE;
    if (reading <= upperLimit) return NEAR_HIGH;
    return ABOVE_RANGE;
}

VitalCondition evaluateVital(float reading, const VitalParameters& params) {
    float tolerance = calculateTolerance(params);
    float nearLowLimit = params.lowerLimit + tolerance;
    float nearHighLimit = params.upperLimit - tolerance;

    if (reading < nearHighLimit) {
        return checkLowZone(reading, params.lowerLimit, nearLowLimit);
    } else {
        return checkHighZone(reading, nearHighLimit, params.upperLimit);
    }
}

void displayCriticalAlert(const char* message) {
    cout << message << "\n";
    for (int i = 0; i < 6; i++) {
        cout << "\r* " << flush;
        sleep_for(seconds(1));
        cout << "\r *" << flush;
        sleep_for(seconds(1));
    }
}

bool isCriticalLevel(VitalCondition condition) {
    return condition == BELOW_RANGE || condition == ABOVE_RANGE;
}

void displayWarning(VitalCondition condition, const VitalParameters& params) {
    if (condition == NEAR_LOW) {
        cout << params.vitalSign << ": " << params.lowWarning << "\n";
    } else if (condition == NEAR_HIGH) {
        cout << params.vitalSign << ": " << params.highWarning << "\n";
    }
}

bool verifyVital(float reading, const VitalParameters& params) {
    VitalCondition condition = evaluateVital(reading, params);
    if (isCriticalLevel(condition)) {
        displayCriticalAlert(params.criticalAlert);
        return false;
    }
    displayWarning(condition, params);
    return true;
}

int vitalsOk(float temperature, float pulseRate, float spo2) {
    VitalParameters tempParams = {
        95.0f, 102.0f, 1.5f,
        "Body Temperature", "Critical Alert: Temperature out of safe range!",
        "Caution: Temperature approaching low limit",
        "Caution: Temperature approaching high limit"
    };

    VitalParameters pulseParams = {
        60.0f, 100.0f, 1.5f,
        "Pulse Rate", "Critical Alert: Pulse rate abnormal!",
        "Caution: Pulse rate nearing bradycardia",
        "Caution: Pulse rate nearing tachycardia"
    };

    VitalParameters spo2Params = {
        90.0f, 100.0f, 1.5f,
        "Oxygen Saturation", "Critical Alert: Oxygen saturation below safe level!",
        "Caution: Oxygen saturation dropping",
        "Caution: Oxygen saturation nearing upper limit"
    };

    bool tempOk = verifyVital(temperature, tempParams);
    bool pulseOk = verifyVital(pulseRate, pulseParams);
    bool spo2Ok = verifyVital(spo2, spo2Params);

    return tempOk && pulseOk && spo2Ok;
}
