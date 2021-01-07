%COMPENG 2DP4
%Final Project- serial communication
%Brooklyn Schmidt
%400065772

%This program takes input from the ADC and converts all data into integers
%The data is then plotted onto a figure in real time using the plot
%function 
%There are two figure created from this program: one plots the data in real
%time, showing the data in 20 second intervals, and the other shows a graph
%of all of the points on one axes

s = serial('COM4');%open serial communication on port 4
set(s,'BaudRate',19200);%set the baud rate
fopen(s);
s.ReadAsyncMode = 'continuous';

inf = 60;%This value determines how long you are sampling the data for in seconds

output = zeros(inf,1);%intializes an array to store the output
time = zeros(inf,1);%this array is used for the x axis of the second figure

fprintf(s,'*IDN?')
out = fscanf(s);

t = 0;%this value is used to plot the x axis in the first figure
x = 0; %placeholder to expand axes after 10 seconds

figure
hold on
ylim([0 90])%set y axis to the desired range of angles
xlabel('Time (s)')
ylabel('Angle (degrees)')
title('Measured Angle of Accelerometer- Brooklyn Schmidt')

for i = 1:inf
    out = fscanf(s);%takes output from ADC as a string
    output(i) = str2double(out);%converts data to an integer
    disp(out)%displays value on the script
             %this is important for when the push button is pressed so 
             %the user can read that the device is on standby
    pause(1);%one second delay
    xlim([0+x 20+x])%x axes are reset in every loop
    plot(t, output(i),'o')
    t=t+1;
    if(i>20)
        x= x+1;%the value of x is increased to increase the x axis
    end
    time(i) = i;
end
hold off;

pause(5);
figure
plot(time,output)%plotting the second figure
ylim([0 90])
xlabel("Time (s)")
ylabel("Angle (degrees)")


fclose(s)%close serial communciation
delete(s)
clear s