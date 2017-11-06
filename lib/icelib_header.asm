 define .icelib_header,space=ram
 define .icelib,space=ram
 segment .icelib_header
 .assume adl=1
 db 192,"ICELIB",0,1
 end

