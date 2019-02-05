% clear all;
% plot(x,y);
% t_c = x; y_c = y; % for clock line
% t_d = x; y_d = y; % for data line

subplot(2,1,1);
plot(t_d,y_d);
subplot(2,1,2);
plot(t_c,y_c);

% save('LL_withLLonly_REPLACE.mat','t_c','y_c','t_d','y_d');
% save('PX_withPXonly_REPLACE.mat','t_c','y_c','t_d','y_d');
% load('LL_withLLonly_REPLACE.mat');

%{
subplot(2,1,1);
plot(t_d(500:2000),y_d(500:2000));
subplot(2,1,2);
plot(t_c(500:2000),y_c(500:2000));
%}

hold on;
subplot(2,1,1);
plot(t_d,y_d);
plot(t_d2,y_d2,'r');
subplot(2,1,2);
plot(t_c,y_c);
plot(t_c2,y_c2,'r');