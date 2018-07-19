
      program evtest

      implicit none

      call wresp()

      end program evtest


      subroutine wresp()

      use iso_c_binding, only: c_loc, c_int, c_double
      implicit none

      interface
         integer(kind=c_int) function evresp(
     &        sta, cha, net, loc, datime, units, file,
     &        freq, npts, resp, rtype, vbs,
     &        start_stage, stop_stage, stdio_flag, sens_flag,
     &        b62_x, xml_flag) bind(C, name="evresp_1")
         use iso_c_binding, only: c_char, c_double, c_int, c_ptr
         character(kind=c_char) :: 
     &        sta(*), cha(*), net(*), loc(*),
     &        datime(*), units(*), file(*), rtype(*), vbs(*)
         integer(kind=c_int), value :: 
     &        npts, start_stage, stop_stage, stdio_flag, sens_flag,
     &        xml_flag
         type(c_ptr), value :: freq, resp
         real(kind=c_double), value :: b62_x
         end function evresp
      end interface

      character*1 :: sta = "*", net = "*", loc = "*"
      character*3 :: cha = "VMZ"
      character*20 :: datime = "2010,260,00:00:00.000"
      character*3 :: units = "VEL"
      character*23 :: file = "../c/data/station-1.xml"
      integer(kind=c_int), parameter :: npts = 100
      real(kind=c_double), target :: freq(npts), resp(2*npts)
      character*2 :: rtype = "CS"
      character*2 :: vbs = "-v"
      integer(kind=c_int) :: 
     &     start_stage = 1, stop_stage = 1, stdio_flag = 0, 
     &     sens_flag = 0, xml_flag = 1, iflag
      real(kind=c_double) :: b62_x = 3

      real(8), parameter :: flow = 0.0001, fhigh = 100
      real(8), parameter :: df = (log10(fhigh) - log10(flow)) / (npts-1)
      integer :: i
      real(8) :: rl, im, amp, phase, pi = 3.14159265358979

      do i = 1, npts
         freq(i) = 10**(log10(flow) + (i-1) * df)
      end do

      iflag = evresp(
     &     sta//char(0), cha//char(0), net//char(0), loc//char(0),
     &     datime//char(0), units//char(0), file//char(0), 
     &     c_loc(freq(1)), npts, c_loc(resp(1)), rtype//char(0), 
     &     vbs//char(0), start_stage, stop_stage, stdio_flag, sens_flag,
     &     b62_x, xml_flag)

      print *, "return value", iflag
      if (iflag .eq. 0) then
      
         open(1, file="evtest.out")
         do i = 1, npts
            rl = resp(2*i-1)
            im = resp(2*i)
            amp = sqrt(rl**2 + im**2)
            phase = atan2(im, rl) * 180. / pi
            write(1,'(3e15.6)') freq(i), amp, phase
         end do
         close(1)

      end if

      end subroutine wresp
