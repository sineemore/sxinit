# sxinit


`sxinit` starts `Xserver` and `~/.xinitrc` script. 

~~All `sxinit` arguments are passed to `Xserver` as is.~~


----

(misc)

	 The full path and arguments to Xorg are hardcoded to prevent users 
	 of placing another binary into the search path,
	 which would be executed with admin rights.
	 For the same reason, it shouldn't be possible to submit 
	 arbitrary arguments to xorg for users,
	 nor should Xorg itself be suid or executable by users.
	
	 The xserver is a quite complex executable, so it is definitely more secure 
	 having a small static binary like sxinit being suid,
	 which can be checked for security flaws,
	 than having the xserver suid with known vulnerabilities.
	
	 All arguments submitted to sxinit are submitted to the script xinitrc,
	 which is parsed and executed by /bin/sh.
	
	 The suid rights of sxinit are droppped, as soon the xserver runs.
	
	 When there's no /home/user/.xinitrc file present,
	 the default /etc/X11/xinitrc script is executed.
	
	 'minilib.conf' is a configuration file to (optionally) compile sxinit  
	 statically linked with minilib (github.com/michael105/minilib) (2.9kB)
    via 'make with-minilib'


