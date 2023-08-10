
"""
Caileigh Bates ID 261114992
US76 model
Constants and equations from US Standard Atmosphere, 1976
Verified with Jim Hawley, Formulae and Code for U.S Standard Atmosphere, 2015
"""

"""
ASSUMPTIONS
Air is dry and altitude is below 86 km 
the atmosphere is assumed to be homogeneously mixed with a relative-volume composition leading to a CONSTANT MEAN MOLAR MASS 
Air is taken to be an ideal gas (use ideal gas law)
Assume hydrostatic equilibrium
valid up to 86 km
"""

import matplotlib.pyplot as plt
import math
import pandas as pd
import numpy as np
#DEFINE CONSTANTS#

L = [-6.5, 0, 1.0, 2.8, 0, -2.8, -2.0] #K/km
H = [0, 11, 20, 32, 47, 51, 71, 84.852] #km
g = -9.80665 #m/s^2
R = 0.00831432 #N*m/Kmol*K
molar_masses = [28.0134, 31.9988, 39.948, 44.00995, 20.183, 4.0026, 83.80, 131.30, 16.04303, 2.01594] 
volume_fractions = [0.7884, 0.209476, 0.00934, 0.000314, 0.00001818, 0.00000524, 0.00000114, 0.000000087, 0.000002, 0.0000005]
Po = 101325 #Pa
To = 288.15 #K

class us76():
    def __init__(self, L, H, g, R, molar_masses, volume_fractions, To, Po):
        self.L = L
        self.H = H
        self.g = g
        self.R = R
        self.molar_masses = molar_masses
        self.volume_fractions = volume_fractions 
        self.To = To
        self.Po = Po
        self.Mo = self.calculating_Mo()
        self.T = self.calculate_temp_points()
        self.P = self.calculate_pressure_points()

    '''''''''''''''''''INIT FUNCTIONS'''''''''''''''''''''''''''
    def calculating_Mo(self):
        fxm = 0 
        f = 0
        for i in range(len(self.molar_masses)):
            fxm += self.volume_fractions[i]*self.molar_masses[i]
            f += self.volume_fractions[i]
        Mo = fxm/(f*1000)
        return Mo #kg

    def calculate_temp_points(self):
        T = [self.To]
        for i in range(len(self.L)):
            t = self.calculate_temp( i, self.H[i+1], T)
            T.append(t)
        return T #K

    def calculate_pressure_points(self):
        P = [Po]
        for i in range(len(self.L)):
            p = self.calculate_pressure(i, self.H[i+1], P)
            P.append(p)
        print(P)
        return P
    ''''''''''''''''''''''''''''''''''EQUATIONS'''''''''''''''''''''''''''''''''''

    def calculate_pressure(self, b, h, P):
        if self.L[b] == 0:
            pressure = P[b]*math.e**(self.g*self.Mo*(h - self.H[b])/(self.R*self.T[b]))
        else:
            pressure = P[b]*(self.T[b]/(self.T[b] + self.L[b]*(h - self.H[b])))**(-self.g*self.Mo/(self.R*self.L[b]))
        return pressure

    def calculate_alt(self, b, p):
        if self.L[b] == 0:
            alt = (math.log(p/self.P[b])*self.R*self.T[b])/(self.g*self.Mo) + self.H[b]
        else:
            alt = (self.T[b]*(self.P[b]/p)**((self.R*self.L[b])/(-self.g*self.Mo)) - self.T[b])/self.L[b] + self.H[b]
        return alt
    
    def calculate_temp(self, b, h, T):
        temp = T[b] + self.L[b]*(h-self.H[b]) 
        return temp
    
    def calculate_rho(self, T, P):
        rho = P*self.Mo/(self.R*1000*T)
        return rho
    ''''''''''''''''''''''''''''''''''FIND VARIABLES'''''''''''''''''''''''''''''''''''

    def find_with_alt(self, h):
        b = 0 
        while h > self.H[b+1]:
            print(b)
            b += 1
        pressure = self.calculate_pressure(b, h, self.P)
        temp = self.calculate_temp(b, h, self.T)
        rho = self.calculate_rho(temp, pressure)
        print("index = ", b)
        return pressure, temp, rho
    
    def find_with_P(self, p):
        b = 1
        while p > self.P[-b]:
            b += 1
        b = len(self.P) - b 

        h = self.calculate_alt(b, p)
        temp = self.calculate_temp(b, h, self.T) 
        rho = self.calculate_rho(temp, p)
        return h, temp, rho

    ''''''''''''''''''''''''''''''''''GRAPHS AND DATASETS'''''''''''''''''''''''''''''''''''
    #csv file with time in the first column and pressure in the second column

    def P_2_alt_graph(self, file_name): 

        df = pd.read_csv(file_name, delimiter= ",")
        data = np.array(df)
        print(data)
        z = [self.find_with_P(elem)[0] for elem in data[:,1]]
        t = data[:, 0]/60
        font = {'fontname' : 'Times New Roman'}
        
        ax = plt.subplot()

        ax.grid(axis = 'y')
        ax.spines[['right', 'top']].set_visible(False)
        ax.plot(t, z, marker = '.', markersize=1, color = 'black')

        ax.set_ylim(0, max(z) - max(z)%5 +5)
        ax.set_xlim(0, max(t) - max(t)%20 + 20)


        plt.xlabel("Time (min.)", **font, size = 12)
        plt.ylabel("Altitude (km)", **font, size =12)
        plt.show()
        return z
    
    def P_2_temp_graph(self, file_name): 

        df = pd.read_csv(file_name, delimiter= ",")
        data = np.array(df)
        print(data)
        z = [self.find_with_P(elem)[1] for elem in data[:,1]]
        z = np.array(z) - 273.15
        t = data[:, 0]/60

        font = {'fontname' : 'Times New Roman'}
        
        ax = plt.subplot()

        ax.grid(axis = 'y')
        ax.spines[['right', 'top']].set_visible(False)
        ax.plot(t, z, marker = '.', markersize=1, color = 'black')

        ax.set_ylim(min(z)-10, max(z) - max(z)%10 +10)
        ax.set_xlim(0, max(t) - max(t)%20 + 20)

        plt.xlabel("Time (min.)", **font, size = 12)
        plt.ylabel("Temperature (C)", **font, size = 12)
        plt.show()
        return z
    



