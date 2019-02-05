clear all;

load('LL_withLLonly_lessthan1k_fail.mat');
t_test_tiny = all(t_c==t_d);   % I don't know why this wouldn't be true, but let's check
t_tiny = t_c*(1e6);            % time was in seconds, now it's in milliseconds
c_tiny = y_c;
d_tiny = y_d;
t_drop_3 = t_tiny(find(d_tiny<2,1));   % first time when data is less than 2, i.e. first drop in the data
t_tiny = t_tiny - t_drop_3;       % shift time so first drop corresponds to t=0

load('LL_withLLonly_1k_work.mat');
t_test_1 = all(t_c==t_d);
t_1 = t_c*(1e6);
c_1 = y_c;
d_1 = y_d;
t_drop_1 = t_1(find(d_1<2,1));
t_1 = t_1 - t_drop_1;

load('LL_withLLonly_5k_work.mat');
t_test_5 = all(t_c==t_d);
t_5 = t_c*(1e6);
c_5 = y_c;
d_5 = y_d;
t_drop_5 = t_5(find(d_5<2,1));
t_5 = t_5 - t_drop_5;

load('LL_withLLonly_opencircuit_work.mat');
t_test_OC = all(t_c==t_d);
t_OC = t_c*(1e6);
c_OC = y_c;
d_OC = y_d;
t_drop_OC = t_OC(find(d_OC<2,1));
t_OC = t_OC - t_drop_OC;

%% clip data so it has same beginning and end
t_min = max([min(t_tiny),min(t_1),min(t_5),min(t_OC)]);
t_max = min([max(t_tiny),max(t_1),max(t_5),max(t_OC)]);

ind_tiny = find(t_tiny>=t_min & t_tiny<=t_max);
t_tiny = t_tiny(ind_tiny);
c_tiny = c_tiny(ind_tiny);
d_tiny = d_tiny(ind_tiny);

ind_1 = find(t_1>=t_min & t_1<=t_max);
t_1 = t_1(ind_1);
c_1 = c_1(ind_1);
d_1 = d_1(ind_1);

ind_5 = find(t_5>=t_min & t_5<=t_max);
t_5 = t_5(ind_5);
c_5 = c_5(ind_5);
d_5 = d_5(ind_5);

ind_OC = find(t_OC>=t_min & t_OC<=t_max);
t_OC = t_OC(ind_OC);
c_OC = c_OC(ind_OC);
d_OC = d_OC(ind_OC);

%% plot
cm = lines;
t_mrg = 1;
y_mrg = 1;
sp1 = subplot(2,1,1);
plot(t_tiny,d_tiny,'Color',clr('matlab_red'));
hold on;
plot(t_1,d_1,'Color',clr('matlab_green'));
plot(t_5,d_5,'Color',clr('matlab_cyan'));
plot(t_OC,d_OC,'Color',clr('matlab_blue'));
xlabel('time (\mus)');
ylabel({'data','voltage (V)'},'FontSize',18);
legend('<1k\Omega (fail)','1k\Omega','5k\Omega','\inftyk\Omega');
sp1.XLim = [t_tiny(1),t_tiny(end)];
grid on;
%axis([t_min-t_mrg, t_max+t_mrg, d_1+
%ht = title('data');
%ht.Position = ht.Position - [0,1.5,0];
%h = text(-3,0,'data','FontSize',22);
%h.Rotation = 90;

sp2 = subplot(2,1,2);
plot(t_tiny,c_tiny,'Color',clr('matlab_red'));
hold on;
plot(t_1,c_1,'Color',clr('matlab_green'));
plot(t_5,c_5,'Color',clr('matlab_cyan'));
plot(t_OC,c_OC,'Color',clr('matlab_blue'));
xlabel('time (\mus)');
ylabel({'clock','voltage (V)'},'FontSize',18);
legend('<1k\Omega (fail)','1k\Omega','5k\Omega','\inftyk\Omega');
sp2.XLim = [t_tiny(1),t_tiny(end)];
grid on;
%ht = title('clock');
%ht.Position = ht.Position - [0,1.5,0];
%h = text(-3,0,'clock','FontSize',22);
%h.Rotation = 90;