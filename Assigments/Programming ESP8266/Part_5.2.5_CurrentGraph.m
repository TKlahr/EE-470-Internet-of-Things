% USB Charging Current Data

time = [0 10 20 30 40 50 60 70 80 90 ...
        100 110 120 130 160 200 230 320 390 440 521];

current = [0.639 0.647 0.647 0.631 0.597 0.616 0.624 ...
           0.601 0.590 0.544 0.510 0.514 0.529 0.522 ...
           0.503 0.476 0.431 0.423 0.461 0.446 0.432];

figure;
plot(time, current, 'o-', 'LineWidth', 1.5);

xlabel('Time (min)');
ylabel('USB Current (A)');
title('USB Charging Current vs. Time');

grid on;
