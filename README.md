# sxinit


`sxinit` starts `Xserver` and `~/.xinitrc` script. 
~~All `sxinit` arguments are passed to `Xserver` as is.~~


----

(misc)

	 I hardcoded the full path and arguments to Xorg to prevent users 
	 of placing another binary into the search path,
	 which would be executed with admin rights.
	 For the same reason, it shouldn't be possible to submit 
	 arbitrary arguments to xorg for users,
	 nor should Xorg itself be suid or executable by users.
	
	 xorg is a quite complex executable, so I believe it's better to have 
	 a small static binary like sxinit being suid,
	 which can be checked for security flaws,
	 with sametime known vulnerabilities of the xserver.
	
	 All arguments submitted to sxinit are now submitted to xinit.
	
	 The suid rights of sxinit are droppped, as soon the xserver runs.
	
	 (todo) When there's no /home/user/.xinitrc file present,
	 or the script exits with an error (>0), 
	 the default /etc/X11/xinitrc script is executed.
	
	 Wrote the configuration file to (optionally) compile sxinit with the devel branch of 
	 minilib (github.com/michael105/minilib) statically (2.9kB).


Still have some cleanup and testing to do.
