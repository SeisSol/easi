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
#~ print('  map')
#~ model: !LayeredModel
  #~ map: !AffineMap
    #~ matrix:
      #~ z: [0.0, 0.0, 1.0]
    #~ translation:
      #~ z: 0
  #~ interpolation: linear
  #~ parameters: [rho]
  #~ nodes:
    #~ 0:     [2600, 1.04e10, 2.08e10]
    #~ -2000: [2700, 3.24e10, 3.24e10]
    #~ -6000: [3000, 4.80e10, 1.20e10]
    #~ -12000: [
    #~ -23000:
    #~ -.inf:
         #~ zLayers (1:6) = (/ 0d0,-2000d0, -6000d0, -12000d0, -23000d0,-600d6 /)
          #~ rhoLayers (1:6) = (/ 1000d0, 2720d0, 2860d0, 3050d0, 3300d0, 3375d0 /)
          #~ sigzz = 0d0
#~ 
          #~ DO k=2,nLayers
             #~ IF (zGP.GT.zLayers(k)) THEN
                #~ sigzz = sigzz + rhoLayers(k-1)*(zGP-zLayers(k-1))*g
                #~ EXIT
             #~ ELSE
                #~ sigzz = sigzz + rhoLayers(k-1)*(zLayers(k)-zLayers(k-1))*g
             #~ ENDIF
#~ ENDDO
