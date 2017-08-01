module special_mod
  interface STRESS_STR_DIP_SLIP_AM
    module procedure STRESS_STR_DIP_SLIP_AM
  end interface
contains
  subroutine STRESS_STR_DIP_SLIP_AM(mu_dy, mu_st, strike, dip, sigmazz, cohesion, R, bii) bind (c, name='STRESS_STR_DIP_SLIP_AM')
    use iso_c_binding, only: c_double
    implicit none
    real(kind=c_double), intent(in), value          :: mu_dy, mu_st, strike, dip, sigmazz, cohesion, R
    real(kind=c_double), intent(out), dimension(6)  :: bii
    real(kind=c_double)                             :: strike_rad, dip_rad, c2, s2, Phi, c2bis, ds, sm, phi_xyz, c, s
    real(kind=c_double)                             :: sii(3), Stress(3,3), R1(3,3), R2(3,3), Stress_cartesian_norm(3,3)
    real(kind=c_double), parameter                  :: pi = 3.141592653589793d0
    !most favorable direction (A4, AM2003)
    Phi = pi/4d0-0.5d0*atan(mu_st)
    s2=sin(2d0*Phi)
    c2=cos(2d0*Phi)
    strike_rad = strike*pi/180d0
    dip_rad = dip*pi/180d0

    c2bis = c2 - cos(2d0*(Phi-dip_rad))

    !ds (delta_sigma) is deduced from R (A5, Aochi and Madariaga 2003),
    !assuming that sig1 and sig3 are in the yz plane
    !sigzz and sigma_ini are then related by a phi+dip rotation (using A3, AM03)
    !sigmazz = sm  - ds * cos(2.*(Phi+dip_rad))
    !Now we have to assume that P = sm (not any more equal to sigmazz)
    !and we can obtain the new expression of ds:
    ds =  (mu_dy * sigmazz + R*(cohesion + (mu_st-mu_dy)*sigmazz)) / (s2 + mu_dy*c2bis + R*(mu_st-mu_dy)*c2bis)
    sm =  sigmazz + ds * cos(2d0*(Phi-dip_rad))

    sii(1)= sm + ds
    !could be any value between sig1 and sig3
    sii(2)= sm
    sii(3)= sm - ds

    Stress = transpose(reshape((/ sii(1), 0d0, 0d0, 0d0, sii(2), 0d0, 0d0, 0d0, sii(3) /), shape(Stress)))

    !first rotation: in xz plane
    phi_xyz=(Phi-dip_rad)
    c=cos(phi_xyz)
    s=-sin(phi_xyz)
    R1= transpose(reshape((/ c, 0d0, s, 0d0, 1d0, 0d0, -s, 0d0, c /), shape(R1)))

    !I cant explain the minus sign...
    c=cos(strike_rad)
    s=-sin(strike_rad)
    R2= transpose(reshape((/ c, -s, 0d0, s, c, 0d0, 0d0, 0d0, 1d0 /), shape(R2)))
    Stress_cartesian_norm = MATMUL(R2,MATMUL(R1,MATMUL(Stress,MATMUL(TRANSPOSE(R1),TRANSPOSE(R2)))))/sigmazz

    bii(1) = Stress_cartesian_norm(1,1)
    bii(2) = Stress_cartesian_norm(2,2)
    bii(3) = Stress_cartesian_norm(3,3)
    bii(4) = Stress_cartesian_norm(1,2)
    bii(5) = Stress_cartesian_norm(2,3)
    bii(6) = Stress_cartesian_norm(1,3)
  end subroutine
end module
