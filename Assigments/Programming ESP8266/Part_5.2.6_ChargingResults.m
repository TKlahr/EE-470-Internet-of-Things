% Combined Charging Results

voltageData = readtable('Lion_Battery_Charging_Clean.csv');
currentData = readtable('Lion_Battery_Charging_Current.csv');

% Voltage data
timeV = voltageData.ElapsedTime_min;
voltage = voltageData.BatteryVoltage_V;

% Remove obvious erroneous voltage reading
valid = voltage > 3.7;
timeV = timeV(valid);
voltage = voltage(valid);

% Current data
timeI = currentData.Time_min;
current = currentData.USB_Current_A;

figure;

% Battery voltage - left axis
yyaxis left
plot(timeV, voltage, 'LineWidth', 1.5);
ylabel('Battery Voltage (V)');

% Charging current - right axis
yyaxis right
plot(timeI, current, 'o-', 'LineWidth', 1.5);
ylabel('USB Current (A)');

xlabel('Time (min)');
title('LiPo Battery Charging Voltage and Current vs. Time');
grid on;

yyaxis left

text(60, 4.12, 'CC Region', ...
    'FontSize', 12, 'FontWeight', 'bold');

text(350, 4.12, 'CV Region', ...
    'FontSize', 12, 'FontWeight', 'bold');
