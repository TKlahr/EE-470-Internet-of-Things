% LiPo Battery Discharge Data

data = readtable('Lion_Battery_Discharging_Clean.csv');

time = data.Time_min;
voltage = data.BatteryVoltage_V;

figure;

plot(time, voltage, 'LineWidth', 1.5);
hold on;

% Important measured values
startVoltage = 4.078;
shutdownVoltage = 2.720;
minimumVoltage = 0.067;

% Mark starting voltage
plot(0, startVoltage, 'o', 'LineWidth', 1.5);
text(5, 4.10, 'Starting Voltage = 4.078 V', ...
    'FontWeight', 'bold');

% Mark transition to rapid voltage drop
xline(195, '--', 'Rapid Voltage Drop', ...
    'LabelVerticalAlignment', 'middle');

% Mark shutdown voltage
plot(215, shutdownVoltage, 'o', 'LineWidth', 1.5);
text(165, 2.65, 'Shutdown = 2.720 V', ...
    'FontWeight', 'bold');

% Mark minimum measured voltage
plot(216, minimumVoltage, 'o', 'LineWidth', 1.5);
text(165, 0.20, 'Minimum = 0.067 V', ...
    'FontWeight', 'bold');

xlabel('Time (min)');
ylabel('Battery Voltage (V)');
title('LiPo Battery Discharge Voltage vs. Time');

grid on;
hold off;
