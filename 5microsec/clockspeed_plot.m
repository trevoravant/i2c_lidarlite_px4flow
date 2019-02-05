clear all;

%% 400 kHz clock speed
load('LL_withLLonly_20k_work.mat');
t_test_400kHz = all(t_c==t_d);   % I don't know why this wouldn't be true, but let's check
t_400kHz = t_c*(1e6);            % time was in seconds, now it's in milliseconds
c_400kHz = y_c;
d_400kHz = y_d;
t_drop_400kHz = t_400kHz(find(d_400kHz<2,1));   % first time when data is less than 2, i.e. first drop in the data
t_400kHz = t_400kHz - t_drop_400kHz;       % shift time so first drop corresponds to t=0

ind_after_fall_400kHz = find(c_400kHz<.5,1);   % index of first time data has first gone below .5
c_after_fall_400kHz = c_400kHz(ind_after_fall_400kHz:end);
ind_rise_400kHz = find(c_after_fall_400kHz>.5,1);
ind_first_rise_400kHz = ind_after_fall_400kHz(1) + ind_rise_400kHz;
t_rise_400kHz = t_400kHz(ind_first_rise_400kHz);
t_400kHz_c_compare = t_400kHz - t_rise_400kHz;

ind_after_fall_400kHz = find(d_400kHz<.5,1);   % index of first time data has first gone below .5
d_after_fall_400kHz = d_400kHz(ind_after_fall_400kHz:end);
ind_rise_400kHz = find(d_after_fall_400kHz>.5,1);
ind_first_rise_400kHz = ind_after_fall_400kHz(1) + ind_rise_400kHz;
t_rise_400kHz = t_400kHz(ind_first_rise_400kHz);
t_400kHz_d_compare = t_400kHz - t_rise_400kHz;

%% 100 kHz clock speed
load('LL_withLLonly_20k_work_100k_clockspeed_5microsec.mat');
t_test_100kHz = all(t_c==t_d);
t_100kHz = t_c*(1e6);
c_100kHz = y_c;
d_100kHz = y_d;
t_drop_100kHz = t_100kHz(find(d_100kHz<2,1));
t_100kHz = t_100kHz - t_drop_100kHz;

load('LL_withLLonly_20k_work_100k_clockspeed_25microsec.mat');
t_test_100kHz_25ms = all(t_c==t_d);
t_100kHz_25ms = t_c*(1e6);
c_100kHz_25ms = y_c;
d_100kHz_25ms = y_d;
t_drop_100kHz_25ms = t_100kHz_25ms(find(d_100kHz_25ms<2,1));
t_100kHz_25ms = t_100kHz_25ms - t_drop_100kHz_25ms;

%% clock and data fall times
ind_after_fall_100kHz = find(c_100kHz<.5,1);   % index times after data has first gone below .5
c_after_fall_100kHz = c_100kHz(ind_after_fall_100kHz:end);
ind_rise_100kHz = find(c_after_fall_100kHz>.5,1);
ind_first_rise_100kHz = ind_after_fall_100kHz(1) + ind_rise_100kHz;
t_rise_100kHz = t_100kHz(ind_first_rise_100kHz);
t_100kHz_c_compare = t_100kHz - t_rise_100kHz;

ind_after_fall_100kHz = find(d_100kHz<.5,1);   % index times after data has first gone below .5
d_after_fall_100kHz = d_100kHz(ind_after_fall_100kHz:end);
ind_rise_100kHz = find(d_after_fall_100kHz>.5,1);
ind_first_rise_100kHz = ind_after_fall_100kHz(1) + ind_rise_100kHz;
t_rise_100kHz = t_100kHz(ind_first_rise_100kHz);
t_100kHz_d_compare = t_100kHz - t_rise_100kHz;

%{
t_400kHz = t_400kHz_clock_compare;
t_100kHz = t_100kHz_clock_compare;

t_400kHz = t_400kHz_data_compare;
t_100kHz = t_100kHz_data_compare;
%}

%% overlap plot
%{
cm = lines;
subplot(2,1,1);
plot(t_400kHz,d_400kHz,'Color',clr('matlab_black'));
hold on;
plot(t_100kHz,d_100kHz,'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel('voltage (V)');
legend('20k\Omega, 400kHz clockspeed','20k\Omega, 100kHz clockspeed');

subplot(2,1,2);
plot(t_400kHz,c_400kHz,'Color',clr('matlab_black'));
hold on;
plot(t_100kHz,c_100kHz,'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel('voltage (V)');
legend('20k\Omega, 400kHz clockspeed','20k\Omega, 100kHz clockspeed');
%}

%% shift plot
t_shift = 40;
t_space = 5;    % time units of space around x-axis
t_beg_400kHz = 500;
t_end_400kHz = 2050;
t_beg_100kHz = 800;
t_end_100kHz = 1850;

sp1 = subplot(2,1,1);
plot(t_400kHz(t_beg_400kHz:t_end_400kHz),...
    d_400kHz(t_beg_400kHz:t_end_400kHz),'Color',clr('matlab_black'));
hold on;
plot(t_100kHz_25ms(t_beg_100kHz:t_end_100kHz)+t_shift,...
    d_100kHz_25ms(t_beg_100kHz:t_end_100kHz),'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel({'data','voltage (V)'},'FontSize',18);
legend('20k\Omega, 400kHz (fail)','20k\Omega, 100kHz');
sp1.XLim = [t_400kHz(t_beg_400kHz) - t_space,...
    t_100kHz_25ms(t_end_100kHz) + t_shift + t_space];
grid on;

sp2 = subplot(2,1,2);
plot(t_400kHz(t_beg_400kHz:t_end_400kHz),...
    c_400kHz(t_beg_400kHz:t_end_400kHz),'Color',clr('matlab_black'));
hold on;
plot(t_100kHz_25ms(t_beg_100kHz:t_end_100kHz)+t_shift,...
    c_100kHz_25ms(t_beg_100kHz:t_end_100kHz),'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel({'clock','voltage (V)'},'FontSize',18);
legend('20k\Omega, 400kHz (fail)','20k\Omega, 100kHz');
sp2.XLim = [t_400kHz(t_beg_400kHz) - t_space,...
    t_100kHz_25ms(t_end_100kHz) + t_shift + t_space];
grid on;

%% plot clock and data comparisons
figure;
% [ha, pos] = tight_subplot(Nh, Nw, gap [height width], marg_h [lower upper], marg_w [left right])
%ha = tight_subplot(2,1,[.3 .03],[.2 .1],[.05 .05]);
sp1 = subplot(2,1,1);
%axes(ha(1))
t_fst = -5;
t_lst = 25;
plt_ind_400kHz = find(t_400kHz_d_compare>t_fst & t_400kHz_d_compare<t_lst);
plt_ind_100kHz = find(t_100kHz_d_compare>t_fst & t_100kHz_d_compare<t_lst);
plot(t_400kHz_d_compare(plt_ind_400kHz),d_400kHz(plt_ind_400kHz),'Color',clr('matlab_black'));
hold on;
plot(t_100kHz_d_compare(plt_ind_100kHz),d_100kHz(plt_ind_100kHz),'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel({'data','voltage (V)'},'FontSize',18);
legend('20k\Omega, 400kHz (fail)','20k\Omega, 100kHz');
t_min = t_400kHz_d_compare(plt_ind_400kHz(1));
t_max = t_400kHz_d_compare(plt_ind_400kHz(end));
sp1.XLim = [t_min,t_max];
grid on;
%{
h = text(t_min-(t_max-t_min)/20-5,0,'data','FontSize',22);
h.Rotation = 90;
%}

sp2 = subplot(2,1,2);
%axes(ha(2))
t_fst = -3;
t_lst = 8;
plt_ind_400kHz = find(t_400kHz_c_compare>t_fst & t_400kHz_c_compare<t_lst);
plt_ind_100kHz = find(t_100kHz_c_compare>t_fst & t_100kHz_c_compare<t_lst);
plot(t_400kHz_c_compare(plt_ind_400kHz),c_400kHz(plt_ind_400kHz),'Color',clr('matlab_black'));
hold on;
plot(t_100kHz_c_compare(plt_ind_100kHz),c_100kHz(plt_ind_100kHz),'Color',clr('matlab_magenta'));
xlabel('time (\mus)');
ylabel({'clock','voltage (V)'},'FontSize',18);
legend('20k\Omega, 400kHz (fail)','20k\Omega, 100kHz');
t_min = t_100kHz_c_compare(plt_ind_100kHz(1));
t_max = t_100kHz_c_compare(plt_ind_100kHz(end));
sp2.XLim = [t_min,t_max];
grid on;
%{
h = text(t_min-(t_max-t_min)/20,0,'clock','FontSize',22);
h.Rotation = 90;
%}
%{
% set figure properties
hf = gcf;
hf.Units = 'normalized';
hf.Position = [0 0 .6 .5];
%}