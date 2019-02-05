clear all;

load('PX_withPXonly_3k_fail.mat');
t_test_3 = all(t_c==t_d);   % I don't know why this wouldn't be true, but let's check
t_3 = t_c*(1e6);            % time was in seconds, now it's in milliseconds
c_3 = y_c;
d_3 = y_d;
t_drop_3 = t_3(find(d_3<2,1));   % first time when data is less than 2, i.e. first drop in the data
t_3 = t_3 - t_drop_3;       % shift time so first drop corresponds to t=0

load('PX_withPXonly_6k_work.mat');
t_test_9 = all(t_c==t_d);
t_9 = t_c*(1e6);
c_9 = y_c;
d_9 = y_d;
t_drop_9 = t_9(find(d_9<2,1));
t_9 = t_9 - t_drop_9;

load('PX_withPXonly_11k_work.mat');
t_test_17 = all(t_c==t_d);
t_17 = t_c*(1e6);
c_17 = y_c;
d_17 = y_d;
t_drop_17 = t_17(find(d_17<2,1));
t_17 = t_17 - t_drop_17;

load('PX_withPXonly_25k_fail.mat');
t_test_25 = all(t_c==t_d);
t_25 = t_c*(1e6);
c_25 = y_c;
d_25 = y_d;
t_drop_25 = t_25(find(d_25<2,1));
t_25 = t_25 - t_drop_25;

%% clip data so it has same beginning and end
t_min = max([min(t_3),min(t_9),min(t_17),min(t_25)]);
t_max = min([max(t_3),max(t_9),max(t_17),max(t_25)]);

ind_3 = find(t_3>=t_min & t_3<=t_max);
t_3 = t_3(ind_3);
c_3 = c_3(ind_3);
d_3 = d_3(ind_3);

ind_9 = find(t_9>=t_min & t_9<=t_max);
t_9 = t_9(ind_9);
c_9 = c_9(ind_9);
d_9 = d_9(ind_9);

ind_17 = find(t_17>=t_min & t_17<=t_max);
t_17 = t_17(ind_17);
c_17 = c_17(ind_17);
d_17 = d_17(ind_17);

ind_25 = find(t_25>=t_min & t_25<=t_max);
t_25 = t_25(ind_25);
c_25 = c_25(ind_25);
d_25 = d_25(ind_25);

%% plot
sp1 = subplot(2,1,1);
plot(t_3,d_3,'Color',clr('matlab_red'));
hold on;
plot(t_9,d_9,'Color',clr('matlab_green'));
plot(t_17,d_17,'Color',clr('matlab_cyan'));
plot(t_25,d_25,'Color',clr('matlab_blue'));
xlabel('time (\mus)');
ylabel({'data','voltage (V)'},'FontSize',18);
legend('3k\Omega (fail)','6k\Omega','11k\Omega','25k\Omega (fail)');
sp1.XLim = [t_3(1),t_3(end)];
grid on;
%ht = title('data');
%ht.Position = ht.Position - [0,1.5,0];
%h = text(-3,0,'data','FontSize',22);
%h.Rotation = 90;

sp2 = subplot(2,1,2);
plot(t_3,c_3,'Color',clr('matlab_red'));
hold on;
plot(t_9,c_9,'Color',clr('matlab_green'));
plot(t_17,c_17,'Color',clr('matlab_cyan'));
plot(t_25,c_25,'Color',clr('matlab_blue'));
xlabel('time (\mus)');
ylabel({'clock','voltage (V)'},'FontSize',18);
legend('3k\Omega (fail)','6k\Omega','11k\Omega','25k\Omega (fail)');
sp2.XLim = [t_3(1),t_3(end)];
grid on;
%ht = title('clock');
%ht.Position = ht.Position - [0,1.5,0];
%h = text(-3,0,'clock','FontSize',22);
%h.Rotation = 90;