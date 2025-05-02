clear; clc;

% Initialization section
% Physical parameters
rv = 0.04; % [m]  Aerodynamic arm effective length
S = 72e-4; % Calculated fin surface area [m^2]

rwdivrv = 0.25; % Ratio of (cog) distance and fin aerodynamic center cog distance 
rw = rv*rwdivrv; % [m] Counterweight distance from shaft
mv = 1/100; % [kg] Wind vane weight
uv = mv/S; % [kg/m^2] Weight to fin area ratio
J = mv * rv^2 * (1 + rwdivrv); % Inertia of vane [kg*m^2]

% Aerodynamic parameters
rho = 1.225; % [kg/m^3] Standard condition air density
av = 6.28; % Torque parameter, brutal approximation for small angles
u = 20.73; % [m/s] Relative wind speed

% Effect due to finite fin neglected
L = 0.05; % Decay distance ratio 
alfamin = 0.1 / 360 * 2 * pi; % Minimum desired readable angle in radians

% Friction parameter
Dm = 20e-6; % [Nms/rad] For a single bearing

q = 0.5 * rho * u^2; % Dynamic pressure [Pa]
Fvdivbeta = 6.28 * q * S; % [N/rad]
N = rv * Fvdivbeta; % [Nm/rad] Wind vane torque coefficient
D = rv * N / u; % [Nm/rad s] Aerodynamic damping
D0 = sqrt(N * 4 * J); 

% Undamped oscillation period
wn = sqrt(N / J); % Natural frequency [rad/s]
wd = sqrt((N / J) - (D / (2 * J))^2); 
zeta = rv * N / u / D0; % Classical defined damping of the system
wdsecond = wn * sqrt(1 - zeta^2); % Redundant calculation

Kv = av / (uv * (1 + rwdivrv)); % Vane quality factor
zetadamping = (0.63 * Dm * sqrt(Kv)) / (u * av * S * rv^(1.5)); % Damping contribution due to friction on the shaft

% Stick to the reference
zetar = 0.395 * rho / 1.25 * sqrt((av * rv^3 * S) / J);
utl = (7.37 * log(1/L) * J) / (av * rv^2 * S); % Decay distance

% Output results
fprintf('Vane parameters\n');
fprintf('Windvane weight grams %.2f\n', mv * 1e3);
fprintf('rv length %.2f mm\n', rv * 1e3);
fprintf('rw length %.2f mm\n\n', rv * rwdivrv * 1e3);

fprintf('Vane calculated parameters\n');
fprintf('Fin surface %.2f mm^2\n', S * 1e6);
fprintf('Vane inertia %.6f kg*m^2\n', J);
fprintf('Natural frequency %.2f rad/s\n', wn);
fprintf('Damped frequency %.2f rad/s \n', wd);
fprintf('Damping ratio zeta %.3f\n', zetar);
fprintf('Decay distance %.1f [m]\n\n', utl);

fprintf('Shaft at test condition with no friction\n');
fprintf('Relative wind speed %.2f m/s\n', u);
fprintf('Alfa value %.2f degrees\n', alfamin / (2*pi) * 360);
fprintf('Aerodynamic Torque %.2e Nm \n\n', N * alfamin);

fprintf('Shaft at test condition with viscous friction\n');
fprintf('Viscous friction term Dm %.3e Nms/rad\n', Dm);
fprintf('Damping ratio increase due to viscous friction %.3f \n', zetadamping);

fprintf('Total damping ratio %.3f\n', zetar + zetadamping);

% Time vector
tspan = [0 1]; % seconds

% Initial angles in degrees
initial_angles_deg = [10, 20, 30, 45];
colors = lines(length(initial_angles_deg)); % For distinct plot colors

figure;
hold on;

for i = 1:length(initial_angles_deg)
    theta0 = deg2rad(initial_angles_deg(i)); % Convert to radians
    omega0 = 0;
    y0 = [theta0; omega0];

    % ODE function
    odefun = @(t, y) [y(2); -(D/J)*y(2) - (N/J)*y(1)];

    % Solve
    [t, y] = ode45(odefun, tspan, y0);
    theta_deg = rad2deg(y(:,1)); % Convert to degrees

    % Plot
    plot(t, theta_deg, 'LineWidth', 2, 'Color', colors(i,:));
end

xlabel('Time (s)');
ylabel('Angle (degrees)');
title('Time vs Angle of Wind Vane for Various Initial Angles');
legend(arrayfun(@(x) sprintf('%.0f°', x), initial_angles_deg, 'UniformOutput', false));
grid on;
hold off;

