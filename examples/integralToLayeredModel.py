#!/usr/bin/env python

# Original function computes
# sigma_zz = I_k + (z-z_k)*\rho_k*g
# where k is the smallest integer such that z > z_k and
# I_{k+1} = I_k + (z_{k+1}-z_k)*\rho_k*g.
#
# Linear interpolation with nodes z_k: I_k is equivalent, because
# sigma_zz = z*(I_k-I_{k+1})/(z_k-z_{k+1}) + (z_k*I_{k+1}-z_{k+1}*I_k)/(z_k-z_{k+1})
#          = I_k + (z-z_k)*\rho_k*g.
# The last z-coordinate 

layers = [0.0,   -2000.0, -6000.0, -12000.0, -23000.0, -100000.0,  float('inf')]
rho    = [1000.0, 2720.0,  2860.0,   3050.0,   3300.0, float('inf')]

integral = 0.0
print('!LayeredModel')
print('map: ??')
print('interpolation: linear')
print('parameters: [s_zz}')
print('nodes:')
for i in range(len(layers)-1):
  print('  {}: [{}]'.format(layers[i], integral))
  integral = integral + (layers[i+1]-layers[i]) * rho[i] * 9.8
